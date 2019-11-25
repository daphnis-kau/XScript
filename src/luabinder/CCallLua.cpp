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

    //=====================================================================
    // Lua脚本调用C++的接口
    //=====================================================================
    void CByScriptLua::GetParam( lua_State* pL, int32 nStartIndex, 
		const list<CTypeBase*>& listParam, char* pDataBuf, void** pArgArray )
    {
        int32 nStkId = nStartIndex;
		int32 nArgIndex = 0;

		//Lua函数最右边的参数，在Lua stack的栈顶,         
        //放在m_listParam的第一个成员中
        for( list<CTypeBase*>::const_iterator it = listParam.begin();
			it != listParam.end(); ++it, nStkId++ )
		{
			CLuaTypeBase* pParamType = static_cast<CLuaTypeBase*>(*it);
            pParamType->GetFromVM( pL, pDataBuf, nStkId, true ); 
			pArgArray[nArgIndex++] = pDataBuf;
			pDataBuf += AligenUp( pParamType->GetLen(), sizeof(void*) );
		}
	}

    int32 CByScriptLua::CallByLua( lua_State* pL )
	{
		CByScriptBase* pScript = (CByScriptBase*)lua_touserdata( pL, lua_upvalueindex(1) );
		uint32 nTop = lua_gettop( pL );

		CScriptLua* pScriptLua = (CScriptLua*)pScript->GetScript();
		pScriptLua->CheckUnlinkCppObj();
		pScriptLua->PushLuaState( pL );

		try
		{
			const list<CTypeBase*>& listParam = pScript->GetParamList();
			CLuaTypeBase* pResultType = static_cast<CLuaTypeBase*>( pScript->GetResultType() );
			uint32 nParamSize = AligenUp( pScript->GetParamSize(), sizeof(void*) );
			uint32 nReturnSize = AligenUp( pResultType ? pResultType->GetLen() : sizeof(int64), sizeof(void*) );
			uint32 nArgSize = AligenUp( pScript->GetParamCount()*sizeof(void*), sizeof(void*) );
			char* pDataBuf = (char*)alloca( nParamSize + nReturnSize + nArgSize );
			char* pResultBuf = pDataBuf + nParamSize;
			void** pArgArray = (void**)( pResultBuf + nReturnSize );
			void* pObject = NULL;

			int32 nStkId = 1;
			if( pScript->GetThisType() )				
			{
				CLuaTypeBase* pThis = static_cast<CLuaTypeBase*>( pScript->GetThisType() );
				pThis->GetFromVM( pL, (char*)&pObject, nStkId++, false ); 
				assert( pObject );
			}

			memset( pDataBuf, 0, pScript->GetParamSize() );
			if( pScript->GetFunctionIndex() == eCT_MemberFunction )
			{
				if( nTop > 1 )
					GetParam( pL, nStkId, listParam, pDataBuf, pArgArray );
				lua_settop( pL, 0 );
				pScript->Call(pObject, nTop > 1 ? NULL : pResultBuf, pArgArray);
				pScriptLua->CheckUnlinkCppObj();
				if( pResultType && nTop <= 1 )
					pResultType->PushToVM( pL, pResultBuf );
			}
			else
			{
				GetParam( pL, nStkId, listParam, pDataBuf, pArgArray );
				lua_settop( pL, 0 );
				pScript->Call(pObject, pResultBuf, pArgArray);
				pScriptLua->CheckUnlinkCppObj();
				if( pResultType )
					pResultType->PushToVM( pL, pResultBuf );
			}
			pScriptLua->PopLuaState();
			return 1;
		}
        catch( std::exception& exp )
		{
			char szBuf[256];
			sprintf( szBuf, "An unknow exception occur on calling %s\n", pScript->GetFunctionName().c_str() );
			std::cout << szBuf << endl;
            luaL_error( pL, exp.what() );
        }
		catch( ... )
        {
			char szBuf[256];
			sprintf( szBuf, "An unknow exception occur on calling %s\n", pScript->GetFunctionName().c_str() );
            luaL_error( pL, szBuf );
        }

		pScriptLua->PopLuaState();
        return 0;
    }

    //=====================================================================
    // C++调用Lua脚本的接口
	//=====================================================================
    bool CCallBackLua::CallVM( SVirtualObj* pObject, void* pRetBuf, void** pArgArray )
	{	
		CScriptLua* pScriptLua = static_cast<CScriptLua*>( m_pScript );
		lua_State* pL = pScriptLua->GetLuaState();

		lua_pushlightuserdata( pL, CScriptLua::ms_pErrorHandlerKey );
		lua_rawget( pL, LUA_REGISTRYINDEX );
		int32 nErrFunIndex = lua_gettop( pL );		// 1

		lua_pushlightuserdata( pL, CScriptLua::ms_pGlobObjectTableKey );
		lua_rawget( pL, LUA_REGISTRYINDEX );		// 2	

		lua_pushlightuserdata( pL, pObject );
		lua_gettable( pL, -2 );						// 3

		if( lua_isnil( pL, -1 ) )
		{
			lua_pop( pL, 3 );            //Error occur
			return false;                //*******表不存在时，此代码有问题************
		}

		lua_getfield( pL, -1, m_sFunName.c_str() ); // 4

		if( lua_tocfunction( pL, -1 ) == &CByScriptLua::CallByLua )
		{
			lua_getupvalue( pL, -1, 1 );
			if( this == lua_touserdata( pL, -1 ) )
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

		lua_insert( pL, -2 );
		PushParam2VM( pL, pArgArray );
		int32 nArg = (int32)( m_listParam.size() + 1 );
		lua_pcall( pL, nArg, m_pResult ? 1 : 0, nErrFunIndex );
		if( m_pResult )
			static_cast<CLuaTypeBase*>( m_pResult )->GetFromVM( pL, (char*)pRetBuf, -1, true );
		lua_settop( pL, nErrFunIndex - 1 );
		return true;
    }

    void CCallBackLua::PushParam2VM( void* pVM, void** pArgArray )
    {
		uint32 i = 0;
        for( list<CTypeBase*>::iterator it = m_listParam.begin(); it != m_listParam.end(); ++it )
            static_cast<CLuaTypeBase*>(*it)->PushToVM( (lua_State*)pVM, (char*)pArgArray[i++] );
	}   
};
