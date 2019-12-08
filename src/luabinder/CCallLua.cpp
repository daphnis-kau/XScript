#include <sstream>
#include <iostream>

extern "C"
{
	#include "lua.h"
	#include "lauxlib.h"
	#include "lstate.h"
	#include "lualib.h"
}

#ifdef _WIN32
#include <excpt.h>
#elif( defined _ANDROID )
#include <alloca.h>
#endif

#include "common/Help.h"
#include "CTypeLua.h"
#include "CCallLua.h"
#include "CDebugLua.h"
#include "CScriptLua.h"

namespace Gamma
{
	CLuaTypeBase* s_aryLuaType[eDT_count] =
	{
		0,
		&CLuaChar::GetInst(),
		&CLuaInt8::GetInst(),
		&CLuaInt16::GetInst(),
		&CLuaInt32::GetInst(),
		&CLuaInt64::GetInst(),
		&CLuaLong::GetInst(),
		&CLuaUint8::GetInst(),
		&CLuaUint16::GetInst(),
		&CLuaUint32::GetInst(),
		&CLuaUint64::GetInst(),
		&CLuaUlong::GetInst(),
		&CLuaWChar::GetInst(),
		&CLuaBool::GetInst(),
		&CLuaFloat::GetInst(),
		&CLuaDouble::GetInst(),
		&CLuaString::GetInst(),
		&CLuaWString::GetInst(),
		&CLuaBuffer::GetInst()
	};

    //=====================================================================
    // Lua脚本调用C++的接口
    //=====================================================================
    int32 CByScriptLua::CallByLua( lua_State* pL )
	{
		CByScriptBase* pCallBase = (CByScriptBase*)lua_touserdata( pL, lua_upvalueindex(1) );
		uint32 nTop = lua_gettop( pL );

		CScriptLua* pScript = CScriptLua::GetScript(pL);
		pScript->CheckUnlinkCppObj();
		pScript->PushLuaState( pL );

		try
		{
			auto& listParam = pCallBase->GetParamList();
			size_t nParamCount = listParam.size();
			const DataType* aryParam = &listParam[0];
			size_t* aryParamSize = (size_t*)alloca( sizeof(size_t)*nParamCount );
			size_t nParamSize = CalBufferSize( aryParam, nParamCount, aryParamSize );
			DataType nResultType = pCallBase->GetResultType();
			size_t nReturnSize = nResultType ? GetAligenSizeOfType(nResultType) : sizeof(int64);
			size_t nArgSize = pCallBase->GetParamCount()*sizeof(void*);
			char* pDataBuf = (char*)alloca( nParamSize + nReturnSize + nArgSize );
			char* pResultBuf = pDataBuf + nParamSize;
			void** pArgArray = (void**)( pResultBuf + nReturnSize );

			int32 nStkId = 1;
			//Lua函数最右边的参数，在Lua stack的栈顶,         
			//放在m_listParam的第一个成员中
			for( size_t nArgIndex = 0; nArgIndex < nParamCount; nArgIndex++ )
			{
				DataType nType = aryParam[nArgIndex];
				CLuaTypeBase* pParamType = GetTypeBase( nType );
				pParamType->GetFromVM( nType, pL, pDataBuf, nStkId++ );
				pArgArray[nArgIndex] = pDataBuf;
				pDataBuf += aryParamSize[nArgIndex];
			}
			lua_settop( pL, 0 );

			if( pCallBase->GetFunctionIndex() == eCT_MemberFunction )
			{
				pCallBase->Call( nTop > 1 ? NULL : pResultBuf, pArgArray, *pScript );
				pScript->CheckUnlinkCppObj();
				if( nResultType && nTop <= 1 )
					GetTypeBase(nResultType)->PushToVM( nResultType, pL, pResultBuf );
			}
			else
			{
				pCallBase->Call( pResultBuf, pArgArray, *pScript );
				pScript->CheckUnlinkCppObj();
				if(nResultType)
					GetTypeBase(nResultType)->PushToVM( nResultType, pL, pResultBuf );
			}
			pScript->PopLuaState();
			return 1;
		}
        catch( std::exception& exp )
		{
			char szBuf[256];
			sprintf( szBuf, "An unknow exception occur on calling %s\n", pCallBase->GetFunctionName().c_str() );
			std::cout << szBuf << endl;
            luaL_error( pL, exp.what() );
        }
		catch( ... )
        {
			char szBuf[256];
			sprintf( szBuf, "An unknow exception occur on calling %s\n", pCallBase->GetFunctionName().c_str() );
            luaL_error( pL, szBuf );
        }

		pScript->PopLuaState();
        return 0;
    }

    //=====================================================================
    // C++调用Lua脚本的接口
	//=====================================================================
	bool CCallBackLua::CallVM( CScriptLua* pScript,	
		CCallScriptBase* pCallBase, void* pRetBuf, void** pArgArray )
	{	
		lua_State* pL = pScript->GetLuaState();

		lua_pushlightuserdata( pL, CScriptLua::ms_pErrorHandlerKey );
		lua_rawget( pL, LUA_REGISTRYINDEX );
		int32 nErrFunIndex = lua_gettop( pL );		// 1

		lua_pushlightuserdata( pL, CScriptLua::ms_pGlobObjectTableKey );
		lua_rawget( pL, LUA_REGISTRYINDEX );		// 2	

		lua_pushlightuserdata( pL, *(void**)pArgArray[0] );
		lua_gettable( pL, -2 );						// 3

		if( lua_isnil( pL, -1 ) )
		{
			lua_pop( pL, 3 );            //Error occur
			return false;                //*******表不存在时，此代码有问题************
		}

		lua_getfield( pL, -1, pCallBase->GetFunctionName().c_str() ); // 4

		if( lua_tocfunction( pL, -1 ) == &CByScriptLua::CallByLua )
		{
			lua_getupvalue( pL, -1, 1 );
			if( pCallBase == lua_touserdata( pL, -1 ) )
			{
				// call self
				lua_pop( pL, 5 );
				return false;
			}
		}
		else if( lua_isnil( pL, -1 ) )
		{
			//Error occur
			lua_pop( pL, 4 );
			return false;
		}

		lua_insert(pL, -2);
		auto& listParam = pCallBase->GetParamList();
		DataType nResultType = pCallBase->GetResultType();
		for( size_t nArgIndex = 1; nArgIndex < listParam.size(); nArgIndex++ )
		{
			DataType nType = listParam[nArgIndex];
			CLuaTypeBase* pParamType = GetTypeBase( nType );
			pParamType->PushToVM( nType, pL, (char*)pArgArray[nArgIndex] );
		}

		int32 nArg = (int32)( listParam.size() );
		lua_pcall( pL, nArg, nResultType ? 1 : 0, nErrFunIndex );
		if(nResultType)
			GetTypeBase(nResultType)->GetFromVM( nResultType, pL, (char*)pRetBuf, -1 );
		lua_settop( pL, nErrFunIndex - 1 );
		return true;
    }

	void CCallBackLua::DestrucVM( CScriptLua* pScript,
		CCallScriptBase* pCallBase, SVirtualObj* pObject )
	{
	}
};
