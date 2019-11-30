#ifdef _WIN32
#include <excpt.h>
#include <windows.h>
#elif( defined _ANDROID )
#include <alloca.h>
#endif

extern "C"
{
	#include "lua.h"
	#include "lauxlib.h"
	#include "lstate.h"
	#include "lualib.h"
}

#include "common/CodeCvs.h"
#include "core/GammaScriptX.h"

#include "CTypeLua.h"
#include "CDebugLua.h"
#include "CScriptLua.h"
#include "CCallLua.h"
#include "core/CClassRegistInfo.h"

namespace Gamma
{
    //====================================================================================
    // CScriptLua
    //====================================================================================
	void* CScriptLua::ms_pGlobObjectTableKey	= (void*)"__global_object_table";
	void* CScriptLua::ms_pGlobReferenceTableKey	= (void*)"__global_reference_table";
	void* CScriptLua::ms_pRegistScriptLuaKey	= (void*)"__regist_cscript_lua";
	void* CScriptLua::ms_pErrorHandlerKey		= (void*)"__error_handler";
	void* CScriptLua::ms_pClassInfoKey			= (void*)"__class_info";

    CScriptLua::CScriptLua( uint16 nDebugPort )
        : m_pAllAllocBlock( NULL )
	{
		memset( m_aryBlock, 0, sizeof(m_aryBlock) );
		lua_State* pL = lua_newstate( &CScriptLua::Realloc, this );
		if( !pL )
			pL = luaL_newstate();
		m_vecLuaState.push_back( pL );
		luaL_openlibs( pL );

		m_pDebugger = new CDebugLua( this, nDebugPort );
#ifdef _DEBUG
		SetDebugLine();
#endif

		lua_atpanic( pL, &CScriptLua::Panic );
		//Redirect2Console( stdin, stdout, stderr );

		const char* szDefClass =
			// 新加的类派生给子类
			"local function __derive_to_child( child, key, value, orgFun )\n"
			"	if rawget( rawget( child, \"__index\" ), key ) ~= orgFun then\n"
			"		return;\n"
			"	end\n"
			"	rawset( rawget( child, \"__index\" ), key, value )\n"
			"	for i = 1, #child.__derive_list do\n"
			"		__derive_to_child( child.__derive_list[i], key, value, orgFun )\n"
			"	end\n"
			"end\n"

			// child从base继承函数
			"local function __inherit_from_base( child, base )\n"
			"	for k, v in pairs( rawget( base, \"__index\" ) ) do\n"
			"		if not rawget( rawget( child, \"__index\" ), k ) then\n"
			"			rawset( rawget( child, \"__index\" ), k, v )\n"
			"		end\n"
			"	end\n"
			"	for i = 1, #base.__base_list do\n"
			"		__inherit_from_base( child, base.__base_list[i] )\n"
			"	end\n"
			"end\n"

			//在继承树上检查check_node是否cur_node的基类
			"local function SearchClassNode( cur_node, check_node )\n"
			"	if( cur_node == check_node )then\n"
			"	    return true\n"
			"	end\n"

			"	local base_list = rawget( cur_node, \"__base_list\")\n"
			"	for i = 1, #base_list do\n"
			"	    if( SearchClassNode( base_list[i], check_node ) ) then\n"
			"	        return true\n"
			"	    end\n"
			"	end\n"

			"	return false\n"
			"end\n"

			//创建一个类
            "function class( ... )\n"
            "	local NewClass = {}\n"
			"	local virtual_table = {}\n"
			"	NewClass.__base_list = {}\n"
			"	NewClass.__derive_list = {}\n"			
			"	NewClass.__index = virtual_table\n" 

			"	local nIndex = 1\n"
			"	while true do\n"
			"		local v = select( nIndex, ... );\n"
			"		if not v then\n"
			"			break;\n"
			"		end\n"
			"		table.insert( NewClass.__base_list, v )\n"
			"		nIndex = nIndex + 1\n"
			"	end\n"

            "	NewClass.ctor = function( self, Instance )\n"
            "	    for i = 1, #self.__base_list do\n"
            "	        local base_class = self.__base_list[i]\n"
            "	        base_class.ctor( base_class, Instance )\n"
            "	    end\n"
            "	end\n"

            "	NewClass.new = function( self, ... )\n"
            "	    local NewInstance = {}\n"
            "	    setmetatable( NewInstance, self )\n"
            "	    self.ctor( self, NewInstance )\n"
			"	    if( self.Ctor )then\n"
			"			self.Ctor( NewInstance, ... )\n"
            "	    end\n"
            "	    return NewInstance\n"
            "	end\n"

			"	NewClass.GetSuperClass = function( n )\n"
			"		return NewClass.__base_list[n or 1];\n"
			"	end\n"

			"	function virtual_table.IsInheritFrom( self, class )\n"
			"	    return SearchClassNode( NewClass, class )\n"
			"	end\n"

			"	function virtual_table.GetClass( self )\n"
			"	    return NewClass\n"
			"	end\n"

			"	virtual_table.class = NewClass\n"

			"	for i = 1, #NewClass.__base_list do\n"
			"	    table.insert( NewClass.__base_list[i].__derive_list, NewClass )\n"
			"		__inherit_from_base( NewClass, NewClass.__base_list[i] )\n"
			"	end\n"

			// class的metatable
			"	local class_metatable = {}\n"
			"	class_metatable.__index = rawget( NewClass, \"__index\" )\n"
			"	class_metatable.__newindex = function( class, key, value )\n"
			"		__derive_to_child( class, key, value, rawget( rawget( class, \"__index\" ), key ) )\n"
			"	end\n"

            "	setmetatable( NewClass, class_metatable )\n"
            "	return NewClass\n"
            "end\n"

            //在继承树上检查check_node是否cur_node的基类，不是则不能进行类型转换
            //如果是，还要检查在cur_node继承树上是否有和check_node内存不连续的基类
            //如果有，也不能进行类型转换
            //返回结果：
            //first：大于0表示可以转换，其余结果表示不可转换
            //second：表示最接近的c++基类
            //注：内存不连续指lua类分别继承于两个以上的c++类，它的实例生成时，
            //    实例所使用的内存块不是一块连续内存
            "local function CheckClassNode( cur_node, check_node )\n"
			"    if( cur_node == check_node ) then\n"
			"        return 1, nil\n"
			"    end\n"

			"    local IsCurCppClass = rawget( cur_node, \"_info\")\n"
			"    local BaseList = rawget( cur_node, \"__base_list\")\n"
			"    local FoundCount = 0\n"
			"    local BaseCppClass = nil\n"

            "	for i = 1, #BaseList do\n"
            "		local IsBaseCppClass = rawget( BaseList[i], \"_info\")\n"
            "		local re, base = CheckClassNode( BaseList[i], check_node )\n"
            //不能进行类型转换
            "		if( re == -1 ) then\n"
            "			return -1, nil\n"
            "		elseif( re == 0 ) then\n"
            //有不连续内存
            "		if( not IsCurCppClass and IsBaseCppClass ) then\n"
            "			return -1, nil\n"
            "		end\n"
            "		else\n"
            //不是唯一基类
            "		if( FoundCount > 0 ) then\n"
            "			return -1, nil\n"
            "			end\n"
            "			FoundCount = 1\n"
            "			BaseCppClass = IsCurCppClass and cur_node or base\n"
            "		end\n"
            "	end\n"

            "	return FoundCount, BaseCppClass\n"
            "end\n"

			//类型转换
            "function ClassCast( class, obj, ... )\n"
            "	local obj_class = getmetatable( obj )\n"
            "	if( obj_class == nil )then\n"
            "		return nil\n"
            "	end\n"

            "	if( SearchClassNode( obj_class, class ) ) then\n"
            "		return obj\n"
            "	end\n"

            "	local re, base = CheckClassNode( class, obj_class )\n"
            "	if( re > 0 ) then\n"
            //有c++基类要先进行c++转换
            "		if( base ) then\n"
            "			__cpp_cast( obj, base )\n"
            "		end\n"
			"		setmetatable( obj, class )\n"
			"		if( class.Ctor )then\n"
			"			class.Ctor( obj, ... )\n"
			"	    end\n"
            "		return obj\n"
            "	end\n"

            "	return nil\n"
            "end\n";

        const char* szDebugPrint = 
			"function DebugPrint( n, ... )\n"

			"	local nIndex = 1\n"
			"	while true do\n"
			"		local v = select( nIndex, ... );\n"
			"		if not v then\n"
			"			break;\n"
			"		end\n"
			"		n = n[v]\n"
			"		nIndex = nIndex + 1\n"
			"	end\n"

            "	if( type( n ) == \"table\") then\n"
            "		print( n, \"\\n{\")\n"
            "		for k, v in pairs( n ) do\n"
            "			print( \"\", k, \"=\", v )\n"
            "		end\n"
            "		print( \"}\")\n"
            "	else\n"
            "		print( n )\n"
            "	end\n"
			"end\n";


		lua_pushlightuserdata( pL, ms_pRegistScriptLuaKey );
		lua_pushlightuserdata( pL, this );
		lua_rawset( pL, LUA_REGISTRYINDEX );

		//生成 CScriptLua::ms_szGlobObjectTable
		lua_pushlightuserdata( pL, CScriptLua::ms_pGlobObjectTableKey );
        lua_newtable( pL );
        lua_newtable( pL );
        lua_pushstring( pL, "v");
        lua_setfield( pL, -2, "__mode");
        lua_setmetatable( pL, -2 );
		lua_rawset( pL, LUA_REGISTRYINDEX );

		lua_pushlightuserdata( pL, ms_pGlobReferenceTableKey );
		lua_newtable( pL );
		lua_rawset( pL, LUA_REGISTRYINDEX );    

		lua_pushlightuserdata( pL, ms_pErrorHandlerKey );
		lua_pushcfunction( pL, &CScriptLua::ErrorHandler );
		lua_rawset( pL, LUA_REGISTRYINDEX );

		RunString( szDefClass );
		RunString( szDebugPrint );

        lua_register( pL, "__cpp_cast",	&CScriptLua::ClassCast );
        lua_register( pL, "gsd",		&CScriptLua::DebugBreak );
		lua_register( pL, "BTrace",		&CScriptLua::BackTrace );

        lua_register( pL, "uint32",		&CScriptLua::ToUint32 );
        lua_register( pL, "int32",		&CScriptLua::ToInt32 );
        lua_register( pL, "uint16",		&CScriptLua::ToUint16 );
        lua_register( pL, "int16",		&CScriptLua::ToInt16 );
        lua_register( pL, "uint8",		&CScriptLua::ToUint8 );
        lua_register( pL, "int8",		&CScriptLua::ToInt8 );
        lua_register( pL, "c2n",		&CScriptLua::ToChar );
        lua_register( pL, "BitAnd",		&CScriptLua::BitAnd );
		lua_register( pL, "BitOr",		&CScriptLua::BitOr );
		lua_register( pL, "BitNot",		&CScriptLua::BitNot );
		lua_register( pL, "BitXor",		&CScriptLua::BitXor );
		lua_register( pL, "LeftShift",	&CScriptLua::LeftShift );
		lua_register( pL, "RightShift",	&CScriptLua::RightShift );

		AddLoader();
		IO_Replace();

		CLuaBuffer::RegistClass( this );
		lua_register( pL, "print",		&CScriptLua::Print );

		BuildRegisterInfo();
    }

    CScriptLua::~CScriptLua(void)
    {
		lua_close( GetLuaState() );
	}

	lua_State* CScriptLua::GetLuaState()
	{
		assert( !m_vecLuaState.empty() );
		return *m_vecLuaState.rbegin();
	}

	void CScriptLua::PushLuaState( lua_State* pL )
	{
		m_vecLuaState.push_back( pL );
	}

	void CScriptLua::PopLuaState()
	{
		m_vecLuaState.pop_back();
	}

	int32 CScriptLua::Panic( lua_State* pL ) 
	{
		std::cout << "PANIC: unprotected error in call to Lua API : " << lua_tostring( pL, -1 ) << endl;
		return 0;
	}

	void* CScriptLua::Realloc( void* pContex, void* pPreBuff, size_t nOldSize, size_t nNewSize )
	{
		if( nOldSize == nNewSize )
			return pPreBuff;
		
		static int64 s_nTotalSize = 0;
		s_nTotalSize += nNewSize;
		s_nTotalSize -= nOldSize;

		//static map<uint32, int64> s_mapSize;
		//s_mapSize[nNewSize] += nNewSize;
		//s_mapSize[nOldSize] -= nOldSize;

		CScriptLua* pThis = (CScriptLua*)pContex;
		void* pNewBuf = NULL;
		const uint32 nMaxManage = eMaxManageMemoryCount*eMemoryStep;
		if( nNewSize > nMaxManage )
			pNewBuf = nNewSize ? new tbyte[nNewSize] : NULL;
		else if( nNewSize )
		{
			uint16 nIdx = (uint16)( ( nNewSize - 1 )/eMemoryStep );
			SMemoryBlock* pNew = pThis->m_aryBlock[nIdx];
			if( !pNew )
			{
				uint32 nBufferSize = nMaxManage*63;
				tbyte* pBuffer = new tbyte[nBufferSize + sizeof(SMemoryBlock)];
				SMemoryBlock* pBlocksHead = (SMemoryBlock*)pBuffer;
				pBlocksHead->m_pNext = pThis->m_pAllAllocBlock;
				pThis->m_pAllAllocBlock = pBlocksHead;
				tbyte* pBufferStart = (tbyte*)( pBlocksHead + 1 );
				uint32 nBlockSize = ( nIdx + 1 ) * eMemoryStep;
				uint32 nCount = nBufferSize/nBlockSize;
				SMemoryBlock* pPreBlock = NULL;
				for( uint32 i = 0; i < nCount; i++ )
				{
					SMemoryBlock* pBlock = (SMemoryBlock*)( pBufferStart + nBlockSize*i );
					pBlock->m_pNext = pPreBlock;
					pPreBlock = pBlock;
				}
				pNew = pPreBlock;
			}

			pThis->m_aryBlock[nIdx] = pNew->m_pNext;
			pNewBuf = pNew;
		}

		if( pNewBuf && nOldSize )
			memcpy( pNewBuf, pPreBuff, Min( nOldSize, nNewSize ) );

		if( nOldSize > nMaxManage )
			delete [] (tbyte*)pPreBuff;
		else if( nOldSize )
		{
			uint16 nIdx = (uint16)( ( nOldSize - 1 )/eMemoryStep );
			SMemoryBlock* pFree = (SMemoryBlock*)pPreBuff;
			pFree->m_pNext = pThis->m_aryBlock[nIdx];
			pThis->m_aryBlock[nIdx] = pFree;
		}

		return pNewBuf;
	}
    
    CScriptLua* CScriptLua::GetScript( lua_State* pL )
    {
        lua_pushlightuserdata( pL, ms_pRegistScriptLuaKey );
        lua_rawget( pL, LUA_REGISTRYINDEX );
        CScriptLua* pScriptLua = (CScriptLua*)lua_touserdata( pL, -1 );
        lua_pop( pL, 1 );
        return pScriptLua;
	}

	bool CScriptLua::CallVM( CCallScriptBase* pCallBase, 
		SVirtualObj* pObject, void* pRetBuf, void** pArgArray )
	{
		return CCallBackLua::CallVM( this, pCallBase, pObject, pRetBuf, pArgArray );
	}

	void CScriptLua::DestrucVM( CCallScriptBase* pCallBase, SVirtualObj* pObject )
	{
		return CCallBackLua::DestrucVM( this, pCallBase, pObject );
	}

    //-------------------------------------------------------------------------------
    // 通用函数
    //--------------------------------------------------------------------------------
    // Lua stack 堆栈必须只有一个值，类（表,在栈底）.调用后， stack top = 1, 对象对应的表在栈顶
    void CScriptLua::NewLuaObj( lua_State* pL, const CClassRegistInfo* pInfo, void* pSrc )
    {
		lua_pushstring( pL, pInfo->GetObjectIndex().c_str() );
        void* pObj = lua_newuserdata( pL, pInfo->GetClassSize() );

		//设置垃圾回收器
		lua_newtable( pL );                    //mt压栈
		lua_pushlightuserdata( pL, ms_pClassInfoKey );
		lua_pushlightuserdata( pL, (void*)pInfo );
		lua_rawset( pL, -3 );
		lua_pushcfunction( pL, Delete );
		lua_setfield( pL, -2, "__gc");
		lua_setmetatable( pL, -2 );            //setmetatable( userdata, mt )

		// Obj.ClassName_hObject = userdata;
		lua_rawset( pL, -3 );

		CScriptLua* pScriptLua = GetScript(pL);
		pScriptLua->PushLuaState( pL );
		pInfo->Create( pObj );                //userdata为对象指针的指针
		if( pSrc )
			pInfo->Assign( pObj, pSrc );
		pScriptLua->CheckUnlinkCppObj();
		pScriptLua->PopLuaState();
		
		//stack top = 2, 对象指针在栈顶,保存对象的表在下面
		RegisterObject( pL, pInfo, pObj, true ); 
    }

    void CScriptLua::RegistToLua( lua_State* pL, const CClassRegistInfo* pInfo, void* pObj, int32 nObjTable, int32 nObj )
    {                                            //__addTableOfUserdata, 把对象表，挂在 CScriptLua::ms_szGlobObjectTable
        lua_pushlightuserdata( pL, pObj );
        lua_pushvalue( pL, nObj );        //返回Obj在堆栈的栈底
		lua_settable( pL, nObjTable );

        for( size_t i = 0; i < pInfo->BaseRegist().size(); i++ )
        {
			void* pChild = ( (char*)pObj ) + pInfo->BaseRegist()[i].m_nBaseOff;
			const CClassRegistInfo* pChildInfo = pInfo->BaseRegist()[i].m_pBaseInfo;
			RegistToLua( pL, pChildInfo, pChild, nObjTable, nObj );

			// 只能处理基类的ObjectIndex，因为最终实例有可能是userdata，
			// 而不是lightuserdata，这里没法区分
			lua_pushstring( pL, pChildInfo->GetObjectIndex().c_str() );
			lua_pushlightuserdata( pL, pChild );
			lua_rawset( pL, nObj );
        }
	}

	void CScriptLua::RemoveFromLua( lua_State* pL, const CClassRegistInfo* pInfo, void* pObj, int32 nObjTable, int32 nObj )
	{
		lua_pushlightuserdata( pL, pObj );
		lua_pushnil( pL );
		lua_settable( pL, nObjTable );

		for( size_t i = 0; i < pInfo->BaseRegist().size(); i++ )
		{
			void* pChild = ( (char*)pObj ) + pInfo->BaseRegist()[i].m_nBaseOff;
			const CClassRegistInfo* pChildInfo = pInfo->BaseRegist()[i].m_pBaseInfo;

			RemoveFromLua( pL, pChildInfo, pChild, nObjTable, nObj );

			// 只能处理基类的ObjectIndex，因为最终实例有可能是userdata，
			// 而不是lightuserdata，这里没法区分
			lua_pushstring( pL, pChildInfo->GetObjectIndex().c_str() );
			lua_pushnil( pL );
			lua_rawset( pL, nObj );
		}
	}

	void CScriptLua::RegisterObject( lua_State* L, const CClassRegistInfo* pInfo, void* pObj, bool bGC )
    {                                        
		//In C++, stack top = 1, 返回Obj, 留在堆栈里
		if( pInfo->IsCallBack() )
			pInfo->ReplaceVirtualTable( GetScript(L), pObj, bGC, 0 );

        int32 nObj = lua_gettop( L );
		//设置全局对象表 CScriptLua::ms_szGlobObjectTable    
		lua_pushlightuserdata( L, CScriptLua::ms_pGlobObjectTableKey );
		lua_rawget( L, LUA_REGISTRYINDEX );						
        RegistToLua( L, pInfo, pObj, nObj + 1, nObj );
        lua_pop( L, 1 );        //弹出CScriptLua::ms_szGlobObjectTable    
    }

//==================================================================================================================================//
//                                                        对Lua提供的功能性函数                                                        //
//==================================================================================================================================//


    //=========================================================================
    // 构造和析构                                            
    //=========================================================================
    int32 CScriptLua::Delete( lua_State* pL )
    {
		lua_getmetatable( pL, -1 );
		lua_pushlightuserdata( pL, ms_pClassInfoKey );
        lua_rawget( pL, -2 );
		const CClassRegistInfo* pInfo = (const CClassRegistInfo*)lua_touserdata( pL, -1 );
		void* pObject = (void*)lua_touserdata( pL, -3 );
		// 不需要调用UnRegisterObject，仅仅恢复虚表即可，
		// 因为已经被回收，所以不存在还有任何地方会引用到此对象
		// 调用UnRegisterObject反而会导致gc问题（table[obj] = nil 会crash）
		CScriptLua* pScriptLua = GetScript(pL);
		pInfo->RecoverVirtualTable( pScriptLua, pObject );
		pInfo->Release( pObject );
		pScriptLua->CheckUnlinkCppObj();
        lua_pop( pL, 3 );
        return 0;
    }

    int32 CScriptLua::Construct( lua_State* pL )
    {
        lua_getfield( pL, -2, "_info" );        //得到c++类属性
		const CClassRegistInfo* pInfo = (const CClassRegistInfo*)lua_touserdata( pL, -1 );
		lua_pop( pL, 1 );
		lua_remove( pL, -2 );
		NewLuaObj( pL, pInfo, NULL );                //stack top = 1, 返回Obj
		return 1;                            //stack top = 1, 返回Obj, 留在堆栈里
	}

	//=========================================================================
	// 错误                                                
	//=========================================================================
	int CScriptLua::ErrorHandler( lua_State *pState )
	{
		const char* szWhat = lua_tostring( pState, -1 );
		lua_pop( pState, 1 );
		CScriptLua* pScript = GetScript( pState );
		CDebugBase* pDebugger = pScript->GetDebugger();
		static_cast<CDebugLua*>( pDebugger )->SetCurState( pState );
		static_cast<CDebugLua*>( pDebugger )->Error( szWhat, true );
		return 0;
	}

    //=========================================================================
    // 调试中断                                                
    //=========================================================================
    int32 CScriptLua::DebugBreak( lua_State* pState )
	{
		CScriptLua* pScript = GetScript( pState );
		CDebugBase* pDebugger = pScript->GetDebugger();
		static_cast<CDebugLua*>( pDebugger )->SetCurState( pState );
		static_cast<CDebugLua*>( pDebugger )->StepOut();
        return 0;
	}

	//=========================================================================
	// 调试中断                                                
	//=========================================================================
	int32 CScriptLua::BackTrace( lua_State* pState )
	{
		uint32 n = lua_isnil( pState, -1 ) 
			? INVALID_32BITID : (uint32)lua_tonumber( pState, -1 );
		CScriptLua* pScript = GetScript( pState );
		CDebugBase* pDebugger = pScript->GetDebugger();
		static_cast<CDebugLua*>( pDebugger )->SetCurState( pState );
		static_cast<CDebugLua*>( pDebugger )->BTrace( n );
		return 0;
	}

    //=========================================================================
    // 类型转换                                                
    //=========================================================================
    int32 CScriptLua::ClassCast( lua_State* pL )
    {
        lua_getfield( pL, -1, "_info" );
		const CClassRegistInfo* pNewInfo = (const CClassRegistInfo*)lua_touserdata( pL, -1 );
        lua_pop( pL, 1 );

        const char* szNewName = pNewInfo->GetObjectIndex().c_str();
        lua_getfield( pL, -2, szNewName );
        if( !lua_isnil( pL, -1 ) )
        {
            lua_pop( pL, 2 );
            return 1;
        }
        lua_pop( pL, 1 );

		lua_getfield( pL, -2, "class" );
        lua_getfield( pL, -1, "_info" );
		const CClassRegistInfo* pOrgInfo = (const CClassRegistInfo*)lua_touserdata( pL, -1 );
        lua_pop( pL, 2 );

        int32 nOffset = pNewInfo->GetBaseOffset( pOrgInfo );
        if( nOffset >= 0 )
            nOffset = -nOffset;
        else
        {
            nOffset = pOrgInfo->GetBaseOffset( pNewInfo );
            if( nOffset < 0 )
            {
                lua_pop( pL, 2 );
                lua_pushnil( pL );
                return 1;
            }
		}

		lua_setmetatable( pL, -2 );

        const char* szOldName = pOrgInfo->GetObjectIndex().c_str();
        lua_getfield( pL, -1, szOldName );
		void* pObj = (void*)lua_touserdata( pL, -1 );
		
		// 不需要调用UnRegisterObject，仅仅恢复虚表即可，
		// 弱表索引会被RegisterObject自动覆盖
		// UnRegisterObject( L, pOrgInfo, *ppObj );
		CScriptLua* pScriptLua = GetScript(pL);
		pOrgInfo->RecoverVirtualTable( pScriptLua, pObj );

		if( nOffset )
		{
			pObj = ( (char*)pObj ) + nOffset;
			lua_pop( pL, 1 );
			lua_pushlightuserdata( pL, pObj );
		}

        lua_setfield( pL, -2, szNewName );
        RegisterObject( pL, pNewInfo, pObj, false );
        return 1;
    }

    //=========================================================================
    // 数值类型转换                                                
    //=========================================================================
    int32 CScriptLua::NewUcs2String( lua_State* L )
    {
		size_t nLen;
		const char* szStr = lua_tolstring( L, -1, &nLen );
		if( !szStr )
			return 0;
		if( nLen >= 2 && szStr[ nLen - 1 ] == 0 && szStr[ nLen - 2 ] == 0 )
			return 1;

		CScriptLua* pScript = GetScript( L );
		if( pScript->m_szTempUcs2.size() < nLen + 1 )
			pScript->m_szTempUcs2.resize( nLen + 1 );

		nLen = Utf8ToUcs( &pScript->m_szTempUcs2[0], (uint32)pScript->m_szTempUcs2.size(), szStr, (uint32)nLen );
        lua_pushlstring( L, (const char*)pScript->m_szTempUcs2.c_str(), ( nLen + 1 )*sizeof(wchar_t) );
        return 1;
    }

	int32 CScriptLua::NewUtf8String( lua_State* L )
	{
		size_t nLen;
		const char* szStr = lua_tolstring( L, -1, &nLen );
		if( !szStr )
			return 0;

		if( nLen >= 2 && szStr[ nLen - 1 ] == 0 && szStr[ nLen - 2 ] == 0 )
		{
			CScriptLua* pScript = GetScript( L );
			size_t nMaxLen = ( nLen/sizeof(wchar_t) - 1 )*3 + 1;
			if( pScript->m_szTempUtf8.size() < nMaxLen )
				pScript->m_szTempUtf8.resize( nMaxLen );

			nLen = UcsToUtf8( &pScript->m_szTempUtf8[0], (uint32)pScript->m_szTempUtf8.size(), (const wchar_t*)szStr, (uint32)( nLen/sizeof(wchar_t) ) - 1 );
			lua_pushlstring( L, pScript->m_szTempUtf8.c_str(), nLen );
		}

		return 1;
	}

    int32 CScriptLua::ToUint32( lua_State* L )
    {
        double n = GetNumFromLua( L, -1 );
        lua_pushnumber( L, (double)( (uint32)n ) );
        return 1;
    }

    int32 CScriptLua::ToInt32( lua_State* L )
    {
        double n = GetNumFromLua( L, -1 );
        lua_pushnumber( L, (int32)(uint32)n );
        return 1;
    }

    int32 CScriptLua::ToUint16( lua_State* L )
    {
        double n = GetNumFromLua( L, -1 );
        lua_pushnumber( L, (double)( (uint16)(uint32)n ) );
        return 1;
    }

    int32 CScriptLua::ToInt16( lua_State* L )
    {
        double n = GetNumFromLua( L, -1 );
        lua_pushnumber( L, (int16)(uint32)n );
        return 1;
    }

    int32 CScriptLua::ToUint8( lua_State* L )
    {
        double n = GetNumFromLua( L, -1 );
        lua_pushnumber( L, (double)( (uint8)(uint32)n ) );
        return 1;
    }

    int32 CScriptLua::ToInt8( lua_State* L )
    {
        double n = GetNumFromLua( L, -1 );
        lua_pushnumber( L, (int8)(uint32)n );
        return 1;
    }

    int32 CScriptLua::ToChar( lua_State* L )
    {
        const char* szBuf = lua_tostring( L, -1 );
        lua_pushnumber( L, szBuf ? szBuf[0] : 0 );
        return 1;
	}

	void CScriptLua::IO_Replace() 
	{
#ifdef _WIN32
		lua_State* pL = GetLuaState();
		const char* szStr = 
			"local arg = { ... } \n"
			"local io_input = io.input;\n"
			"local io_output = io.output;\n"
			"local io_lines = io.lines;\n"
			"local io_open = io.open;\n"
			"io.input = function( n ) return io_input( arg[1]( n ) ); end;\n"
			"io.output = function( n ) return io_output( arg[1]( n ) ); end;\n"
			"io.lines = function( n ) return io_lines( arg[1]( n ) ); end;\n"
			"io.open = function( n, m ) return io_open( arg[1]( n ), m ); end;\n";
		
		struct SIO_Replace
		{
			static int32 IO_Utf2A( lua_State* pL ) 
			{
				if( lua_type( pL, 1 ) != LUA_TSTRING )
					return 1;
				const char* szFileName = luaL_checkstring( pL, 1 );
				if( szFileName == NULL || szFileName[0] == 0 )
					return 1;
				wstring strName = Utf8ToUcs( szFileName );
				assert( strName.size() < 1024 );
				char szAcsName[4096];
				WideCharToMultiByte( CP_ACP, NULL, strName.c_str(), -1, 
					szAcsName, ELEM_COUNT(szAcsName), NULL, FALSE );
				lua_settop( pL, 0 );
				lua_pushstring( pL, szAcsName );
				return 1;
			}

			static const char* ReadString( lua_State* , void* pContext, size_t* pSize )
			{
				const char* szStr = *(const char**)pContext;
				*pSize = strlen( szStr );
				*(const char**)pContext = szStr + *pSize;
				return szStr;
			}
		};

		lua_pushlightuserdata( pL, CScriptLua::ms_pErrorHandlerKey );
		lua_rawget( pL, LUA_REGISTRYINDEX ); //1
		int32 nErrFunIndex = lua_gettop( pL );

		if( lua_load( pL, &SIO_Replace::ReadString, &szStr, NULL ) )
			throw( "Invalid string!!!!" );
		lua_pushcfunction( pL, &SIO_Replace::IO_Utf2A );
		lua_pcall( pL, 1, 0, nErrFunIndex );
		lua_remove( pL, nErrFunIndex );
#endif
	}

	int32 CScriptLua::Print( lua_State* pL )
	{
		int n = lua_gettop( pL );  /* number of arguments */
		int i;
		lua_getglobal( pL, "tostring");
		for( i = 1; i <= n; i++ )
		{
			const char *s;
			lua_pushvalue( pL, -1 );  /* function to be called */
			lua_pushvalue( pL, i );   /* value to print */
			lua_call( pL, 1, 1 );
			s = lua_tostring( pL, -1 );  /* get result */
			if( s == NULL )
				return luaL_error( pL, LUA_QL("tostring") " must return a string to " LUA_QL("print") );
			if( i > 1 )
				std::cout << "\t";
			std::cout << s;
			lua_pop( pL, 1 );  /* pop result */
		}
		std::cout << endl;
		return 0;
	}

    //=========================================================================
    // 位操作                                                
    //=========================================================================
    int32 CScriptLua::BitAnd( lua_State* L )
    {
        uint64 n = INVALID_64BITID;
        int32 nTop = lua_gettop( L );
        for( int32 i = 1; i <= nTop; i++ )
		{
			double d = GetNumFromLua( L, i );
			n &= ( d < 0 ? (uint64)(int64)d : (uint64)d );
		}
        lua_pop( L, nTop );
        lua_pushnumber( L, (double)n );
        return 1;
    }

    int32 CScriptLua::BitOr( lua_State* L )
    {
        uint64 n = 0;
        int32 nTop = lua_gettop( L );
        for( int32 i = 1; i <= nTop; i++ )
		{
			double d = GetNumFromLua( L, i );
            n |= ( d < 0 ? (uint64)(int64)d : (uint64)d );
		}
        lua_pop( L, nTop );
        lua_pushnumber( L, (double)n );
        return 1;
    }

    int32 CScriptLua::BitNot( lua_State* L )
    {
        double d = GetNumFromLua( L, -1 );
        lua_pop( L, 1 );
        uint64 n = ~( d < 0 ? (uint64)(int64)d : (uint64)d );
        lua_pushnumber( L, (double)( n&0xfffffffffffffLL ) );
        return 1;
    }

	int32 CScriptLua::BitXor( lua_State* L )
	{
		uint64 n = 0;
		int32 nTop = lua_gettop( L );
		for( int32 i = 1; i <= nTop; i++ )
		{
			double d = GetNumFromLua( L, i );
			n ^= ( d < 0 ? (uint64)(int64)d : (uint64)d );
		}
		lua_pop( L, nTop );
		lua_pushnumber( L, (double)n );
		return 1;
	}

	int32 CScriptLua::LeftShift( lua_State* L )
	{
		double d = GetNumFromLua( L, 1 );
		uint8  b = (int64)GetNumFromLua( L, 2 );
		lua_pop( L, 1 );
		uint64 n = ( d < 0 ? (uint64)(int64)d : (uint64)d ) << b;
		lua_pushnumber( L, (double)n );
		return 1;
	}

	int32 CScriptLua::RightShift( lua_State* L )
	{
		double d = GetNumFromLua( L, 1 );
		uint8  b = (int64)GetNumFromLua( L, 2 );
		lua_pop( L, 1 );
		uint64 n = ( d < 0 ? (uint64)(int64)d : (uint64)d ) >> b;
		lua_pushnumber( L, (double)n );
		return 1;
	}

    //==================================================================================================================================//
    //                                                        对内部提供的功能性函数                                                        //
    //==================================================================================================================================//
    void CScriptLua::AddLoader()
    {
		lua_State* pL = GetLuaState();
        lua_getglobal( pL, "package");
        lua_getfield( pL, -1, "loaders");
        assert( lua_istable( pL, -1 ) );
        lua_pushcfunction( pL, &CScriptLua::LoadFile );
        int32 i = 1; 
        for( ; ; i++ )
        {
            lua_rawgeti( pL, -2, i );
            if( lua_isnil( pL, -1 ) )
            {
                lua_pop( pL, 1 );
                break;
            }
            lua_insert( pL, -2 );
            lua_rawseti( pL, -3, i );
        }
        lua_rawseti( pL, -2, i );
		lua_pop( pL, 2 );
		lua_register( pL, "dofile", &CScriptLua::DoFile );
		lua_register( pL, "loadfile", &CScriptLua::LoadFile );
	}

	const char* _ReadString( lua_State* /*pL*/, void* pContext, size_t* pSize )
	{
		const char* szStr = *(const char**)pContext;
		if( !szStr || !szStr[0] )
			return NULL;

		*pSize = strlen( szStr );
		*(const char**)pContext = szStr + *pSize;
		return szStr;
	}

	bool CScriptLua::RunString( lua_State* pL, const char* szStr )
	{
		CheckUnlinkCppObj();
		lua_pushlightuserdata( pL, CScriptLua::ms_pErrorHandlerKey );
		lua_rawget( pL, LUA_REGISTRYINDEX ); //1
		int32 nErrFunIndex = lua_gettop( pL );

		static set<string> s_setRuningString;
		pair< set<string>::iterator, bool > ib = s_setRuningString.insert( szStr );
		std::stringstream name;
		name << "@GammaScriptStringTrunk"<< (uintptr_t)(void*)( ib.first->c_str() );
		string strName = name.str();
		const char* szName = strName.c_str();

		// 通过_ReadString装载字符串的代码块
		if( GetGlobObject( pL, szName ) ||
			( !lua_load( pL, &_ReadString, &szStr, szName ) && SetGlobObject( pL, szName ) ) )
		{
			bool re = !lua_pcall( pL, 0, LUA_MULTRET, nErrFunIndex );
			lua_remove( pL, nErrFunIndex );
			return re;
		}
		else
		{
			// 装载失败时移走错误函数，并且从s_setRuningString删除字符串
			lua_remove( pL, nErrFunIndex );
			if( ib.second )
				s_setRuningString.erase( ib.first );

			const char* szError = lua_tostring( pL, -1 );
			if( szError )
			{
				std::cout << szError << endl;
				lua_remove( pL, 1 );
			}

			return false;
		}

	}

	struct SFileLoadInfo
	{
		SFileLoadInfo(const char* szFileName)
		{
			FILE* fp = fopen( szFileName, "rb" );
			if( NULL == fp )
				return;
			fseek( fp, 0, SEEK_END );
			fileBuff.resize( ftell( fp ) );
			fseek( fp, 0, SEEK_SET );
			fread( &fileBuff[0], fileBuff.size(), 1, fp );
			fclose( fp );
		}
		string fileBuff;
		bool bFinished;
	};

	const char* _ReadFile( lua_State* pL, void* pContext, size_t* pSize )
	{
		SFileLoadInfo* pLoadInfo = (SFileLoadInfo*)pContext;
		if( pLoadInfo->bFinished )
			return NULL;

		pLoadInfo->bFinished = true;
		uint32 nSize = (uint32)(pLoadInfo->fileBuff.size());
		const tbyte* pBuffer = (const tbyte*)pLoadInfo->fileBuff.c_str();
		if( pBuffer[0] == 0xef && 
			pBuffer[1] == 0xbb && 
			pBuffer[2] == 0xbf )
		{
			nSize -= 3;
			pBuffer += 3;
		}
		*pSize = nSize;
		return (const char*)pBuffer;
	}

	struct SReadWithPackage
	{
		vector<tbyte>* pBuffer;
		bool bFinished;
	};

	const char* _ReadFileWithPackageFile( lua_State* pL, void* pContext, size_t* pSize )
	{
		SReadWithPackage* pInfo = (SReadWithPackage*)pContext;
		if( pInfo->bFinished )
			return NULL;

		pInfo->bFinished = true;
		*pSize = pInfo->pBuffer->size();
		return (const char* )( &( (*pInfo->pBuffer)[0] ) );
	}

    bool CScriptLua::LoadFile( lua_State* pL, const char* szFileName, bool bReload )
    {
		if( !szFileName )
			return false;

		if( szFileName[0] == '/' || ::strchr( szFileName, ':' ) )
			return LoadSingleFile( pL, szFileName, bReload ) > 0;

		for( list<string>::iterator it = m_listSearchPath.begin(); it != m_listSearchPath.end(); ++it )
		{
			int32 nResult;
			string sFileName = *it + szFileName;
			if( !GetFileExtend( szFileName ) )
				sFileName.append( ".lua" );
			if( 0 == ( nResult = LoadSingleFile( pL, sFileName.c_str(), bReload ) ) )
				continue;
			return nResult > 0;
        }
        return false;
    }
    
    int32 CScriptLua::LoadFile( lua_State* pL )
    {
        const char* szFileName = lua_tostring( pL, 1 );
        lua_pop( pL, 1 );
        if( CScriptLua::GetScript( pL )->LoadFile( pL, szFileName, true ) )
            return 1;
        string szError = string( "Cannot find the file ") + szFileName;
        lua_pushstring( pL, szError.c_str() );
        return 1;
    }

	int32 CScriptLua::LoadSingleFile( lua_State* pL, const char* szFileName, bool bReload )
	{
		char szBuf[1024] = "@";
		strcat2array_safe( szBuf, szFileName );
		if( !bReload && GetGlobObject( pL, szFileName ) )
			return 1;

		SFileLoadInfo LoadInfo(szFileName);
		if( !LoadInfo.fileBuff.size() )
			return 0;

		LoadInfo.bFinished = false;
		if( !lua_load( pL, &_ReadFile, &LoadInfo, szBuf ) )
		{
			SetGlobObject( pL, szFileName );
			if( GetDebugger() && GetDebugger()->RemoteDebugEnable() )
			{
				const char* szBuffer = LoadInfo.fileBuff.c_str();
				GetDebugger()->AddFileContent( szFileName, szBuffer );
			}
			return 1;
		}

		const char* szError = lua_tostring( pL, -1 );
		if( szError )
		{
			std::cout << szError << std::endl;
			lua_remove( pL, 1 );
		}
		return -1;
	}

	 int32 CScriptLua::DoFile( lua_State* pL )
	 {
		 const char* szFileName = luaL_optstring( pL, 1, NULL );
		 int n = lua_gettop( pL );
		 if( !CScriptLua::GetScript( pL )->LoadFile( pL, szFileName, true ) ) 
			 lua_error( pL );
		 lua_call( pL, 0, LUA_MULTRET );
		 return lua_gettop( pL ) - n;
	 }

	 bool CScriptLua::SetGlobObject( lua_State* pL, const char* szKey )
	 {
		 lua_pushlightuserdata( pL, CScriptLua::ms_pGlobObjectTableKey );
		 lua_rawget( pL, LUA_REGISTRYINDEX );						
		 lua_pushstring( pL, szKey );
		 lua_pushvalue( pL, -3 );
		 lua_rawset( pL, -3 );   
		 lua_pop( pL, 1 );
		 return true;   
	 }

	 bool CScriptLua::GetGlobObject( lua_State* pL, const char* szKey )
	 {
		 lua_pushlightuserdata( pL, CScriptLua::ms_pGlobObjectTableKey );
		 lua_rawget( pL, LUA_REGISTRYINDEX );						
		 lua_pushstring( pL, szKey );
		 lua_rawget( pL, -2 );      
		 lua_remove( pL, -2 );
		 if( !lua_isnil( pL, -1 ) )
			 return true;   
		 lua_pop( pL, 1 );
		 return false;
	 }

	 void CScriptLua::BuildRegisterInfo()
	 {
		lua_State* pL = GetLuaState();
		 const CTypeIDNameMap& mapRegisterInfo = CClassRegistInfo::GetAllRegisterInfo();
		 for( auto pInfo = mapRegisterInfo.GetFirst(); pInfo; pInfo = pInfo->GetNext() )
		 {
			 if( pInfo->IsEnum() )
				 continue;

			 const CCallBaseMap& mapFunction = pInfo->GetRegistFunction();
			 const char* szClass = pInfo->GetClassName().c_str();
			 if( szClass && szClass[0] )
			 {
				 //调用完毕之后，lua stack 还有一个值，新的class
				 lua_getglobal( pL, "class" );
				 assert( !lua_isnil( pL, -1 ) );            //"class"没被注册

				 int nClassIdx = lua_gettop( pL );
				 lua_getglobal( pL, szClass );
				 assert( lua_isnil( pL, -1 ) );            //szClass已被注册
				 lua_pop( pL, 1 );                        //1

				 //生成全局类名
				 for( size_t i = 0; i < pInfo->BaseRegist().size(); i++ )
				 {
					 auto pBaseInfo = pInfo->BaseRegist()[i].m_pBaseInfo;
					 assert( pBaseInfo != NULL );
					 lua_getglobal( pL, pBaseInfo->GetClassName().c_str() );
					 assert( !lua_isnil( pL, -1 ) );            //Base class do not exsit.
				 }

				 lua_call( pL, (int32)pInfo->BaseRegist().size(), 1 );            //top = 1, the new class 
				 lua_pushvalue( pL, -1 );
				 lua_setglobal( pL, szClass );            //top = 1, the new class

				 //给类设置类属性结构
				 lua_pushstring( pL, "_info" );
				 lua_pushlightuserdata( pL, pInfo );
				 lua_rawset( pL, nClassIdx );            //top = 1

				 //设置垃圾回收函数
				 lua_pushstring( pL, "__gc" );
				 lua_pushcfunction( pL, Delete );
				 lua_rawset( pL, nClassIdx );            //top = 1

				 //将默认的new替换成新的new
				 lua_pushstring( pL, "ctor" );
				 lua_pushcfunction( pL, Construct );
				 lua_rawset( pL, nClassIdx );
			 }
			 else
			 {
				lua_getglobal( pL, "_G" );
			 }

			 assert( !lua_isnil( pL, -1 ) );

			 for( auto pCall = mapFunction.GetFirst(); pCall; pCall = pCall->GetNext() )
			 {
				 lua_pushlightuserdata( pL, pCall );
				 lua_pushcclosure( pL, CByScriptLua::CallByLua, 1 );
				 lua_setfield( pL, -2, pCall->GetFunctionName().c_str() );
			 }
			 lua_pop( pL, 1 );
		 }
	 }

#ifdef _DEBUG
	 uint32 g_nIndex = 0;
	 pair<const char*, uint32> g_aryLog[1024];

	 void CScriptLua::DebugHookProc( lua_State *pState, lua_Debug* pDebug )
	 {
		 static bool bOut = false;
		 if( bOut )
		 {
			 lua_getinfo ( pState, "S", pDebug );
			 if( strncmp( "@GammaScriptStringTrunk", pDebug->source, 13 ) )
			 {
				 lua_getinfo ( pState, "l", pDebug );
				 uint32 nIndex = ( g_nIndex++ )%1024;
				 g_aryLog[nIndex].first = pDebug->source;
				 g_aryLog[nIndex].second = pDebug->currentline;
			 }
		 }
		 lua_sethook( pState, &DebugHookProc, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 );
	 }

	 void CScriptLua::SetDebugLine()
	 {
		 lua_sethook( GetLuaState(), &DebugHookProc, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 );
	 }
#endif

    //==================================================================================================================================//
    //                                                        对C++提供的功能性函数                                                     //
    //==================================================================================================================================//
    bool CScriptLua::RunFile( const char* szFileName, bool bReload )
	{
		CheckUnlinkCppObj();
		lua_State* pL = GetLuaState();
		lua_pushlightuserdata( pL, CScriptLua::ms_pErrorHandlerKey );
		lua_rawget( pL, LUA_REGISTRYINDEX ); //1
		int32 nErrFunIndex = lua_gettop( pL );
		if( !LoadFile( pL, szFileName, false ) )
		{
			lua_pop( pL, 1 );
			return false;
		}

		bool re = !lua_pcall( pL, 0, LUA_MULTRET, nErrFunIndex );
		lua_remove( pL, nErrFunIndex );
        return re;
	}

	const char* _ReadBuffer( lua_State* /*pL*/, void* pContext, size_t* pSize )
	{
		pair<const void*, size_t>* pairBuffer = ( pair<const void*, size_t>* )pContext;
		*pSize = pairBuffer->second;
		pairBuffer->second = 0;
		return *pSize ? (const char*)pairBuffer->first : NULL;
	}

	bool CScriptLua::RunBuffer( const void* pBuffer, size_t nSize )
	{
		CheckUnlinkCppObj();
		lua_State* pL = GetLuaState();
		lua_pushlightuserdata( pL, CScriptLua::ms_pErrorHandlerKey );
		lua_rawget( pL, LUA_REGISTRYINDEX ); //1
		int32 nErrFunIndex = lua_gettop( pL );

		char szBuf[1024];
		static uint32 s_nBufferIndex = 0;
		sprintf( szBuf, "@GammaScriptBufferTrunk%x", s_nBufferIndex );
		pair<const void*, size_t> context( pBuffer, nSize );

		// 通过_ReadString装载字符串的代码块
		if( !lua_load( pL, &_ReadBuffer, &context, szBuf ) )
		{
			bool re = !lua_pcall( pL, 0, LUA_MULTRET, nErrFunIndex );
			lua_remove( pL, nErrFunIndex );
			return re;
		}

		// 装载失败时移走错误函数，并且从s_setRuningString删除字符串
		lua_remove( pL, nErrFunIndex );

		const char* szError = lua_tostring( pL, -1 );
		if( szError )
		{
			std::cout << szError << endl;
			lua_remove( pL, 1 );
		}

		return false;
	}

    bool CScriptLua::RunString( const char* szString )
	{
        return RunString( GetLuaState(), szString );
	}
	
	bool CScriptLua::RunFunction( const STypeInfoArray& aryTypeInfo, void* pResultBuf, const char* szFunction, void** aryArg )
	{
		CheckUnlinkCppObj();
		lua_State* pL = GetLuaState();
		const CCallBase* pCallBase = CClassRegistInfo::GetGlobalCallBase( aryTypeInfo );
		lua_pushlightuserdata( pL, ms_pErrorHandlerKey );
		lua_rawget( pL, LUA_REGISTRYINDEX );
		int32 nErrFunIndex = lua_gettop( pL );

		const char* szFun = "return %s";
		char szFuncBuf[256];
		sprintf(szFuncBuf, szFun, szFunction);
		if( GetGlobObject( pL, szFuncBuf ) || ( !luaL_loadstring( pL, szFuncBuf ) && SetGlobObject( pL, szFuncBuf ) ) )
			lua_pcall( pL, 0, LUA_MULTRET, 0 );

		uint32 nParamCount = (uint32)pCallBase->GetParamCount();
		const vector<DataType>& listParam = pCallBase->GetParamList();
		for( uint32 nArgIndex = 0; nArgIndex < nParamCount; nArgIndex++ )
		{
			DataType nType = listParam[nArgIndex];
			CLuaTypeBase* pParamType = GetTypeBase(nType);
			pParamType->PushToVM(nType, pL, (char*)aryArg[nArgIndex]);
		}

		DataType nResultType = pCallBase->GetResultType();
		lua_pcall( pL, nParamCount, nResultType && pResultBuf, nErrFunIndex );

		if( nResultType && pResultBuf )
		{
			GetTypeBase(nResultType)->GetFromVM( nResultType, pL, (char*)pResultBuf, -1 );
			lua_pop( pL, 1 );
		}

		lua_pop( pL, 1 );
		return true;
	}

    void CScriptLua::RefScriptObj( void* pObj )
	{
		lua_State* pL = GetLuaState();
		// 从全局对象表查找pObj对应的table，table必须存在
		lua_pushlightuserdata( pL, CScriptLua::ms_pGlobObjectTableKey );
		lua_rawget( pL, LUA_REGISTRYINDEX );						           // nStk = 1
        assert( !lua_isnil( pL, -1 ) );    

        lua_pushlightuserdata( pL, pObj );                                // nStk = 2
        lua_gettable( pL, -2 );                                        // nStk = 2
        assert( !lua_isnil( pL, -1 ) );

		// 根据table在全局对象引用计数表中查询对象的引用计数值，使其增加1
		lua_pushlightuserdata( pL, ms_pGlobReferenceTableKey );
        lua_rawget( pL, LUA_REGISTRYINDEX );								// nStk = 3
        lua_pushvalue( pL, -2 );                                        // nStk = 4
        lua_rawget( pL, -2 );                                            // nStk = 4
        int32 nRef = lua_isnil( pL, -1 ) ? 0 : (int32)lua_tointeger( pL, -1 );
        lua_pop( pL, 1 );                                                // nStk = 3
        lua_pushvalue( pL, -2 );                                        // nStk = 4
        lua_pushinteger( pL, nRef + 1 );                                // nStk = 5
        lua_rawset( pL, -3 );                                            // nStk = 3
        lua_pop( pL, 3 );                                                // nStk = 0
    }

    void CScriptLua::UnrefScriptObj( void* pObj )
	{
		lua_State* pL = GetLuaState();
		// 从全局对象表查找pObj对应的table，table必须存在
		lua_pushlightuserdata( pL, CScriptLua::ms_pGlobObjectTableKey );
		lua_rawget( pL, LUA_REGISTRYINDEX );					      // nStk = 1
        assert( !lua_isnil( pL, -1 ) );    

        lua_pushlightuserdata( pL, pObj );                            // nStk = 2
        lua_gettable( pL, -2 );                                    // nStk = 2
        assert( !lua_isnil( pL, -1 ) );

        // 根据table在全局对象引用计数表中查询对象的引用计数值，
        // 引用必须存在，引用值而且必须大于1，然后使引用值减1，
		// 如果减到0，则在全局对象引用计数表中删除该引用
		lua_pushlightuserdata( pL, ms_pGlobReferenceTableKey );
		lua_rawget( pL, LUA_REGISTRYINDEX );						   // nStk = 3
        lua_pushvalue( pL, -2 );                                    // nStk = 4
        lua_rawget( pL, -2 );                                        // nStk = 4
        assert( !lua_isnil( pL, -1 ) );
        int32 nRef = (int32)lua_tointeger( pL, -1 );
        assert( nRef );
        lua_pop( pL, 1 );                                            // nStk = 3
        lua_pushvalue( pL, -2 );                                    // nStk = 4
        if( nRef > 1 )
            lua_pushinteger( pL, nRef - 1 );                        // nStk = 5
        else
            lua_pushnil( pL );                                        // nStk = 5
        lua_rawset( pL, -3 );                                        // nStk = 3
        lua_pop( pL, 3 );                                            // nStk = 0
    }

    void CScriptLua::UnlinkCppObjFromScript( void* pObj )
	{
		lua_State* pL = GetLuaState();
		int32 nTop = lua_gettop( pL );
		// 从全局对象表查找pObj对应的table，table必须存在
		lua_pushlightuserdata( pL, CScriptLua::ms_pGlobObjectTableKey );
		lua_rawget( pL, LUA_REGISTRYINDEX );		    // nStk = 1
		assert( !lua_isnil( pL, -1 ) );  

		lua_pushlightuserdata( pL, pObj );              // nStk = 2
		lua_gettable( pL, -2 );							// nStk = 2

		if( lua_isnil( pL, -1 ) )
		{
			lua_settop( pL, nTop );
			return;
		}

		lua_getmetatable( pL, -1 );						// nStk = 3
		if( !lua_isnil( pL, -1 ) )
		{
			struct SClearClassInfo
			{
				static void Run( lua_State* pL, int32 nObj )
				{
					lua_getfield( pL, -1, "_info" );				// nStk = 4
					void* pData = lua_touserdata( pL, -1 );		// nStk = 4
					if( pData )
					{
						const CClassRegistInfo* pInfo = (const CClassRegistInfo*)pData;
						lua_pushstring( pL, pInfo->GetObjectIndex().c_str() );	// nStk = 5
						lua_rawget( pL, nObj );			
						void* pObj = lua_touserdata( pL, -1 );
						if( pObj )
						{
							int32 nTop = lua_gettop( pL );
							// 从全局对象表查找pObj对应的table，table必须存在
							lua_pushlightuserdata( pL, CScriptLua::ms_pGlobObjectTableKey );
							lua_rawget( pL, LUA_REGISTRYINDEX );		    // nStk = 1
							assert( !lua_isnil( pL, -1 ) );  

							lua_pushlightuserdata( pL, pObj );              // nStk = 2
							lua_gettable( pL, -2 );							// nStk = 2

							// 这里不需要恢复虚表，因为当基类析构时虚表自然恢复了，
							// 而且恢复虚表可能会发生内存越界，因为pObj已经释放了
							RemoveFromLua( pL, pInfo, pObj, nTop + 1, nTop + 2 );

							lua_pushnil( pL );
							lua_setmetatable( pL, nTop + 2 );
							lua_settop( pL, nTop );
						}
						lua_pop( pL, 2 );
					}
					else
					{
						lua_pop( pL, 1 );
						lua_pushstring( pL, "__base_list" );
						lua_rawget( pL, -2 );
						lua_rawgeti( pL, -1, 1 );
						for( int i = 1; !lua_isnil( pL, -1 ); lua_rawgeti( pL, -1, ++i ) )
						{
							SClearClassInfo::Run( pL, nObj );
							lua_pop( pL, 1 );
						}
						lua_pop( pL, 2 );
					}
				}
			};

			SClearClassInfo::Run( pL, nTop + 2 );
		}

		lua_settop( pL, nTop );
	}

	void CScriptLua::GC()
	{
		lua_gc( GetLuaState(), LUA_GCSTEP, 0 );
	}

	void CScriptLua::GCAll()
	{
		lua_State* pL = GetLuaState();
		lua_gc( pL, LUA_GCCOLLECT, 0 );
		lua_gc( pL, LUA_GCCOLLECT, 0 );
	}
};
