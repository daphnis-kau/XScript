#include "CDebugLua.h"
#include "CScriptLua.h"
#include <list>
#include <vector>
#include <sstream>
#include <algorithm>
#include "common/Help.h"

extern "C"
{
	#include "lua.h"
	#include "lauxlib.h"
	#include "lstate.h"
	#include "lualib.h"
}

namespace Gamma
{
	#define LOCAL_VARIABLE_ID 1
	static void* s_szValue2ID = (void*)"v2i";
	static void* s_szID2Value = (void*)"i2v";

    //-----------------------------------------------------
    // 当脚本执行中行跳转时将会调用此函数
    //-----------------------------------------------------
    CDebugLua::CDebugLua( CScriptBase* pBase, uint16 nDebugPort )
        : CDebugBase( pBase, nDebugPort )
		, m_pState(NULL)
        , m_nRunningStackLevel(-1)
        , m_nBreakStackLevel(-1)
        , m_bInCoroutine(false)
		, m_bStop(false)
		, m_nValueID( LOCAL_VARIABLE_ID )
	{
		CScriptLua* pScript = static_cast<CScriptLua*>( pBase );
		lua_State* pL = pScript->GetLuaState();

		lua_pushlightuserdata( pL, s_szValue2ID );
		lua_newtable( pL );
		lua_newtable( pL );
		lua_pushstring( pL, "k");
		lua_setfield( pL, -2, "__mode");
		lua_setmetatable( pL, -2 );
		lua_rawset( pL, LUA_REGISTRYINDEX );

		lua_pushlightuserdata( pL, s_szID2Value );
		lua_newtable( pL );
		lua_newtable( pL );
		lua_pushstring( pL, "v");
		lua_setfield( pL, -2, "__mode");
		lua_setmetatable( pL, -2 );
		lua_rawset( pL, LUA_REGISTRYINDEX );
    }

    CDebugLua::~CDebugLua(void)
    {
	}

	void CDebugLua::SetCurState( lua_State* pL )
	{
		m_pState = pL;
	}

	uint32 CDebugLua::GetFrameCount()
	{
		lua_Debug ld;
		uint32 nDepth = 0;
		while( lua_getstack ( m_pState, nDepth, &ld ) )
			nDepth++;
		return nDepth;
	}

	bool CDebugLua::GetFrameInfo( int32 nFrame, int32* nLine, 
		const char** szFunction, const char** szSource )
	{
		lua_Debug ld;
		if( !lua_getstack ( m_pState, nFrame, &ld ) )
			return false;

		if( szFunction )
		{
			lua_getinfo ( m_pState, "n", &ld );
			*szFunction = ld.name;
		}

		if( szSource )
		{
			lua_getinfo ( m_pState, "S", &ld );
			*szSource = ld.source + 1;
		}

		if( nLine )
		{
			lua_getinfo ( m_pState, "l", &ld );
			*nLine = ld.currentline;
		}

		return true;
	}

	void CDebugLua::ReadFile( std::string& strBuffer, const char* szFileName )
	{
		static string szKey = "GammaScriptStringTrunk";
		if( szKey == string( szFileName, szKey.size() ) )
		{
			uintptr_t address = 0;
			std::stringstream( szFileName + szKey.size() ) >> address;
			if( !address )
				return;
			strBuffer.assign( (const char*)address );
			return;
		}

		CDebugBase::ReadFile( strBuffer, szFileName );
		if( strBuffer.empty() )
			return;
		if( strBuffer[0] != '#' && strBuffer[0] != LUA_SIGNATURE[0] )
			return;
		strBuffer.clear();
	}

	uint32 CDebugLua::GenBreakPointID( const char* szFileName, int32 nLine )
	{
		static uint32 s_nBreakPointID = 1;
		return s_nBreakPointID++;
	}

    void CDebugLua::HookProc( lua_State *pState, lua_Debug* pDebug )
    {
		CScriptLua* pScriptLua = CScriptLua::GetScript( pState );
		CDebugBase* pDebugger = pScriptLua->GetDebugger();
		static_cast<CDebugLua*>( pDebugger )->LineHook( pState, pDebug );
    }

    void CDebugLua::LineHook( lua_State* pState, lua_Debug* pDebug )
    {
		bool bStop = false;
		if( m_bStop && ( pDebug->event == LUA_HOOKLINE || pDebug->event == LUA_HOOKRET ) )
		{
			m_bStop = false;
			bStop = true;
		}

		if( !bStop )
		{
			lua_getinfo ( pState, "S", pDebug );
			lua_getinfo ( pState, "l", pDebug );
			if( GetBreakPoint( pDebug->source, pDebug->currentline ) )
				bStop = true;
		}

        if( pState != m_pState && !bStop )
        {
            if( m_bInCoroutine )
                return;

            //转换了coroutine
            switch( pDebug->event )
            {
            case LUA_HOOKCALL:
                //resume
                if( m_nBreakStackLevel <= m_nRunningStackLevel )
                {
                    //打开这个开关，在coroutine中运行的代码都不会触发任何调试器的状态变化
                    m_bInCoroutine = true;
                    return;
                }
                //step in
                //fall down
            case LUA_HOOKRET:
                //yield or dead
                m_pState = pState;
                StepIn();
                break;
            default:
                {
                    ostringstream strm;
                    strm << "Invalid hook event "<< pDebug->event << " when switching coroutine.";
                    throw( strm.str() );
                }
            }
			return;
        }

		if( m_nBreakStackLevel != -1 )
		{
			//单步执行是打开的
			switch( pDebug->event )
			{
			case LUA_HOOKCALL:
				//并不是每一次LUA_HOOKCALL都会增加堆栈深度，所以这里不得不每次遍历堆栈来获得深度，是不是安装了coco的原因呢
				m_nRunningStackLevel = GetFrameCount();
				/*if(GetFrameCount()!=++m_nRunningStackLevel)
				DebugBreak();*/
				if( m_nBreakStackLevel < m_nRunningStackLevel )	//step out或者step over
					lua_sethook( pState, &CDebugLua::HookProc, LUA_MASKCALL | LUA_MASKRET | (HaveBreakPoint()?LUA_MASKLINE:0), 0 );
				return;
			case LUA_HOOKRET:
				if(m_bInCoroutine)
					m_bInCoroutine=false;
				//并不是每一次LUA_HOOKRETURN都会减少堆栈深度，所以这里不得不每次遍历堆栈来获得深度，是不是安装了coco的原因呢
				m_nRunningStackLevel = GetFrameCount()-1;
				/*if(GetFrameCount()!=m_nRunningStackLevel--)
				DebugBreak();*/
				if( m_nBreakStackLevel>=m_nRunningStackLevel )
					lua_sethook( pState, &CDebugLua::HookProc, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 );
				return;
			case LUA_HOOKTAILRET:
				break;
			case LUA_HOOKLINE:
				break;
			default:
				throw( L"Invalid event in lua hook function" );
			}

			if(  (!bStop) && (m_nBreakStackLevel>=0) && (m_nBreakStackLevel<m_nRunningStackLevel)  )
				return;

			m_nRunningStackLevel=m_nBreakStackLevel=-1;//关闭所有各类单步执行
		}
		else
		{
			if( !bStop || ( pDebug->event != LUA_HOOKLINE && pDebug->event != LUA_HOOKRET ) )
				return;
		}

		m_pState = pState;
        lua_sethook( pState, &CDebugLua::HookProc, 0, 0 );
        Debug();
	}

	uint32 CDebugLua::AddBreakPoint( const char* szFileName, int32 nLine )
	{
		uint32 nID = CDebugBase::AddBreakPoint( szFileName, nLine );
		CScriptLua* pScriptLua = static_cast<CScriptLua*>( GetScriptBase() );
		lua_State* pState = pScriptLua->GetLuaState();
		if( !HaveBreakPoint() )
			lua_sethook( pState, &CDebugLua::HookProc, 0, 0 );
		else
			lua_sethook( pState, &CDebugLua::HookProc, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 );
		return nID;
	}

	void CDebugLua::DelBreakPoint( uint32 nBreakPointID )
	{
		CDebugBase::DelBreakPoint( nBreakPointID );
		CScriptLua* pScriptLua = static_cast<CScriptLua*>( GetScriptBase() );
		lua_State* pState = pScriptLua->GetLuaState();
		if( !HaveBreakPoint() )
			lua_sethook( pState, &CDebugLua::HookProc, 0, 0 );
		else
			lua_sethook( pState, &CDebugLua::HookProc, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 );
	}

	void CDebugLua::Stop()
	{
		m_bStop = true;
		CScriptLua* pScriptLua = static_cast<CScriptLua*>( GetScriptBase() );
		lua_State* pState = pScriptLua->GetLuaState();
		lua_sethook( pState, &CDebugLua::HookProc, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 );
	}

	void CDebugLua::Continue()
	{
		if( !HaveBreakPoint() )
			return;
		lua_sethook( m_pState, &CDebugLua::HookProc, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 );
	}

    void CDebugLua::StepNext()
    {
        lua_sethook( m_pState, &CDebugLua::HookProc, LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET, 0 );
        m_nRunningStackLevel = GetFrameCount();
        m_nBreakStackLevel = m_nRunningStackLevel;    //栈级别必须与当前执行深度相同
    }

    void CDebugLua::StepIn()
    {
        lua_sethook( m_pState, &CDebugLua::HookProc, LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET, 0 );
        m_nRunningStackLevel = GetFrameCount();
        m_nBreakStackLevel = INT_MAX;                    //无堆栈级别需求，任何情况都可以断
    }

    void CDebugLua::StepOut()
    {
        lua_sethook( m_pState, &CDebugLua::HookProc, LUA_MASKCALL | LUA_MASKRET | (HaveBreakPoint()?LUA_MASKLINE:0), 0 );
        m_nRunningStackLevel = GetFrameCount();
        m_nBreakStackLevel = m_nRunningStackLevel - 1;    //堆栈级别必须比当前执行深度小1
    }

	int32 CDebugLua::SwitchFrame( int32 nCurFrame )
	{
		lua_Debug ld;
		return lua_getstack( m_pState, nCurFrame, &ld ) ? nCurFrame : -1;
	}

	uint32 CDebugLua::GetVariableField( const char* szField )
	{
		if( szField && szField[0] )
		{
			lua_pushlightuserdata( m_pState, CScriptLua::ms_pErrorHandlerKey );
			lua_rawget( m_pState, LUA_REGISTRYINDEX );
			int32 nErrFunIndex = lua_gettop( m_pState );
			const char* szFun = "return function( a ) return a%s end";
			char szFuncBuf[256];
			sprintf( szFuncBuf, szFun, szField );
			if( luaL_loadstring( m_pState, szFuncBuf ) )
			{
				lua_pop( m_pState, 2 );
				return INVALID_32BITID;
			}
			lua_pcall( m_pState, 0, 1, 0 );
			lua_pushvalue( m_pState, -3 );
			lua_pcall( m_pState, 1, 1, nErrFunIndex );
			lua_remove( m_pState, -2 );
			lua_remove( m_pState, -2 );
		}

		lua_pushlightuserdata( m_pState, s_szValue2ID );
		lua_rawget( m_pState, LUA_REGISTRYINDEX );	
		lua_pushvalue( m_pState, -2 );	
		lua_rawget( m_pState, -2 ); 
		uint32 nID = (uint32)lua_tonumber( m_pState, -1 );
		if( nID )
		{
			lua_pop( m_pState, 3 );
			return nID;
		}

		lua_pop( m_pState, 1 );

		// 加入s_szValue2ID
		nID = ++m_nValueID;
		lua_pushvalue( m_pState, -2 );
		lua_pushnumber( m_pState, nID );
		lua_rawset( m_pState, -3 );   
		lua_pop( m_pState, 1 );

		// 加入s_szID2Value
		lua_pushlightuserdata( m_pState, s_szID2Value );
		lua_rawget( m_pState, LUA_REGISTRYINDEX );	
		lua_pushnumber( m_pState, nID );
		lua_pushvalue( m_pState, -3 );
		lua_rawset( m_pState, -3 );   
		lua_pop( m_pState, 2 );
		return nID;
	}

	uint32 CDebugLua::GetVariableID( int32 nCurFrame, const char* szName )
	{
		if( szName == NULL )
			return LOCAL_VARIABLE_ID;

		if( !IsWordChar( szName[0] ) )
			return INVALID_32BITID;
		uint32 nIndex = 0;
		string strValue;
		while( szName[nIndex] == '_' ||
			IsWordChar( szName[nIndex] ) || 
			IsNumber( szName[nIndex] ) )
			strValue.push_back( szName[nIndex++] );

		lua_Debug _ar;
		if( !lua_getstack ( m_pState, nCurFrame, &_ar ) )
			return INVALID_32BITID;

		int n = 1;
		const char* name = NULL;
		while( ( name = lua_getlocal( m_pState, &_ar, n++ ) ) != NULL ) 
		{
			if( strValue == name )
				return GetVariableField( szName + nIndex );
			lua_pop( m_pState, 1 ); 
		}			

		n = 1;
		lua_getinfo( m_pState, "f", &_ar );
		while( ( name = lua_getupvalue( m_pState, -1, n++ ) ) != NULL ) 
		{
			if( strValue == name )
				return GetVariableField( szName + nIndex );
			lua_pop( m_pState, 1 ); 
		}			

		lua_getglobal( m_pState, strValue.c_str() );
		if( ( lua_type( m_pState, -1 ) != LUA_TNIL ) )
			return GetVariableField( szName + nIndex );
		lua_pop( m_pState, 1 ); 
		return INVALID_32BITID;
	}

	Gamma::SValueInfo CDebugLua::GetVariable( uint32 nID )
	{
		SValueInfo Info;
		Info.nID = nID;
		if( nID == INVALID_32BITID )
		{
			Info.strValue = "nil";
			return Info;
		}

		// 加入s_szID2Value
		lua_pushlightuserdata( m_pState, s_szID2Value );
		lua_rawget( m_pState, LUA_REGISTRYINDEX );	
		lua_pushnumber( m_pState, nID );
		lua_rawget( m_pState, -2 );     
		lua_remove( m_pState, -2 );  

		lua_getglobal( m_pState, "tostring");
		lua_pushvalue( m_pState, -2 );
		lua_call( m_pState, 1, 1 );
		const char* s = lua_tostring( m_pState, -1 );  /* get result */
		Info.strValue = s ? s : "nil";
		lua_pop( m_pState, 1 );

		// [todo 获取属性]
		lua_pop( m_pState, 1 );
		return Info;
	}

	uint32 CDebugLua::GetChildrenID( uint32 nParentID, bool bIndex, 
		uint32 nStart, uint32* aryChild, uint32 nCount )
	{
		return 0;
	}
}
