#include "CLuaDebugger.h"
#include "../src/luabinder/CDebugLua.h"

extern "C"
{
	#include "../third_party/lua-5.1.5/src/lauxlib.h"
}

class CDebugHandler : public XS::IDebugHandler
{
	lua_State*		m_pMainState;
	XS::CDebugLua	m_Debugger;

public:
	CDebugHandler( lua_State* pState, const char* strDebugHost, uint16 nDebugPort, bool bWaitDebug );

	XS::CDebugLua& GetDebugger() { return m_Debugger; }
	virtual void* GetVM() { return m_pMainState; }
	virtual const char* GetFunctionByStack( void* pFunObject, void* pContext );
	int32				ToString( lua_State* pL );

	virtual int32		Input( char* szBuffer, int nCount );
	virtual int32		Output( const char* szBuffer, int nCount, bool bError = false );
	virtual void*		OpenFile( const char* szFileName );
	virtual int32		ReadFile( void* pContext, char* szBuffer, int32 nCount );
	virtual void		CloseFile( void* pContext );
	virtual bool		RunFile( const char* szFileName, bool bForce = false );
};

int32 CDebugHandler::ToString( lua_State* pL )
{
	int32 nTop = lua_gettop( pL );
	luaL_checkany( pL, -1 );
	if( luaL_callmeta( pL, -1, "__tostring" ) )
	{
		lua_remove( pL, -2 );
		assert( nTop == lua_gettop( pL ) );
		return 1;
	}

	lua_settop( pL, nTop );
	char szDouble[256];
	int type = lua_type( pL, -1 );
	const char* s = nullptr;
	if( type == LUA_TNUMBER )
	{

		double n = lua_tonumber( pL, -1 );
		if( n != (double)((int64)n) )
		{
			sprintf( szDouble, "%lf", n );
			s = szDouble;
		}
		else
			s = lua_tostring( pL, -1 );
	}
	else if( type == LUA_TSTRING )
		return 1;
	else if( type == LUA_TBOOLEAN )
		s = lua_toboolean( pL, -1 ) ? "true" : "false";
	else if( type == LUA_TNIL )
		s = "nil";

	if( s )
	{
		lua_pop( pL, 1 );
		lua_pushstring( pL, s );
		assert( nTop == lua_gettop( pL ) );
		return 1;
	}

	const void* ptr = lua_topointer( pL, -1 );
	const char* name = lua_typename( pL, lua_type( pL, -1 ) );
	if( type != LUA_TTABLE )
	{
		lua_pop( pL, 1 );
		lua_pushfstring( pL, "%s: %p", name, ptr );
		assert( nTop == lua_gettop( pL ) );
		return 1;
	}

	lua_pop( pL, 1 );
	lua_pushfstring( pL, "table: %p", ptr );
	assert( nTop == lua_gettop( pL ) );
	return 1;
}

CDebugHandler::CDebugHandler( lua_State* pState,
	const char* strDebugHost, uint16 nDebugPort, bool bWaitDebug )
	: m_pMainState( pState )
	, m_Debugger( this, strDebugHost, nDebugPort )
{
	while( bWaitDebug && !m_Debugger.CheckEnterRemoteDebug() )
		std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
}

const char* CDebugHandler::GetFunctionByStack( void* pFunObject, void* pContext )
{
	return nullptr;
}

int32 CDebugHandler::Input( char* szBuffer, int nCount )
{
	return 0;
}

int32 CDebugHandler::Output( const char* szBuffer, int nCount, bool bError /*= false */ )
{
	return 0;
}

void* CDebugHandler::OpenFile( const char* szFileName )
{
	return 0;
}

int32 CDebugHandler::ReadFile( void* pContext, char* szBuffer, int32 nCount )
{
	return 0;
}

void CDebugHandler::CloseFile( void* pContext )
{

}

bool CDebugHandler::RunFile( const char* szFileName, bool bForce /*= false */ )
{
	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

DEBUG_API void debugger_hook_wrap(lua_State* pState, lua_Debug* pDebug)
{
	//XS::CDebugLua::DebugHook(pState, pDebug);
}

DEBUG_API void* new_debugger(lua_State* pState, const char* strDebugHost, uint16 nDebugPort, bool bWaitDebug)
{
	return new CDebugHandler(pState, strDebugHost, nDebugPort, bWaitDebug);
}

DEBUG_API void del_debugger(void* _pDebugger )
{
	delete (CDebugHandler*)_pDebugger;
}

DEBUG_API int check_debug_cmd(void* _pDebugger)
{
	auto pDebugger = (CDebugHandler*)_pDebugger;
	if (!pDebugger)
		return -1;

	if (pDebugger->GetDebugger().CheckEnterRemoteDebug())
		return 0;

	return pDebugger->GetDebugger().GetDebuggerState();
}

#ifdef __cplusplus
};
#endif