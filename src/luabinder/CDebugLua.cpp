#include "common/Help.h"
#include "common/TStrStream.h"
#include "CDebugLua.h"
#include "CScriptLua.h"
#include <list>
#include <vector>
#include <sstream>
#include <algorithm>

extern "C"
{
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

namespace XS
{
	struct HackLuaDebug : public lua_Debug { char szBuffer[128]; };

	enum EPreDefinedVariableID
	{
		ePDVID_Scopes	= 1,
		ePDVID_Global	= 2,
		ePDVID_Local	= 3,
		ePDVID_UpValue	= 4,
		ePDVID_Tempory  = 5,
		ePDVID_Count	= 6,
	};

	#define STEP_DEBUG_MASK (LUA_MASKRET|LUA_MASKLINE)

	static void* s_szValue2ID = (void*)"v2i";
	static void* s_szID2Value = ( void* )"i2v";
	static void* s_szDebuger = (void*)"CDebugLua";

    CDebugLua::CDebugLua( IDebugHandler* pHandler, 
		const char* strDebugHost, uint16 nDebugPort )
        : CDebugBase( pHandler, strDebugHost, nDebugPort )
		, m_pState( nullptr )
		, m_pPreState( nullptr )
        , m_nBreakFrame( -1 )
		, m_nValueID( ePDVID_Count )
	{
		assert( pHandler );
		lua_State* pL = (lua_State*)pHandler->GetVM();
		assert( pL );
		lua_pushlightuserdata( pL, s_szDebuger );
		lua_pushlightuserdata( pL, this );
		lua_rawset( pL, LUA_REGISTRYINDEX );
    }

	CDebugLua::~CDebugLua( void )
	{
		lua_State* pL = (lua_State*)GetDebugHandler()->GetVM();
		lua_sethook( pL, &CDebugLua::DebugHook, 0, 0 );
	}

	CDebugLua* CDebugLua::GetDebugger( lua_State* pState )
	{
		lua_pushlightuserdata( pState, s_szDebuger );
		lua_rawget( pState, LUA_REGISTRYINDEX );
		void* pDebuger = lua_touserdata( pState, -1 );
		lua_pop( pState, 1 );
		return (CDebugLua*)pDebuger;
	}

	void CDebugLua::SetCurState( lua_State* pL )
	{
		m_pState = pL;
		m_pPreState = nullptr;
	}

	uint32 CDebugLua::GetFrameCount()
	{
		HackLuaDebug ld = {};
		uint32 nDepth = 0;
		while( lua_getstack ( m_pState, nDepth, &ld ) )
			nDepth++;
		return nDepth;
	}

	bool CDebugLua::GetFrameInfo( int32 nFrame, int32* nLine, 
		const char** szFunction, const char** szSource )
	{
		HackLuaDebug ld = {};
		if( !lua_getstack ( m_pState, nFrame, &ld ) )
			return false;

		lua_getinfo(m_pState, "Sln", &ld);
		if( szFunction )
		{
			*szFunction = ld.name && ld.name[0] == '?' ? nullptr : ld.name;
		}

		if( szSource )
		{
			*szSource = ld.source + 1;
		}

		if( nLine )
		{
			*nLine = ld.currentline;
		}

		return true;
	}

	uint32 CDebugLua::GenBreakPointID( const char* szFileName, int32 nLine )
	{
		static uint32 s_nBreakPointID = 1;
		return s_nBreakPointID++;
	}

	void CDebugLua::DebugHook( lua_State* pState, lua_Debug* pDebug )
    {
		auto pDebugger = GetDebugger( pState );

		if (pDebugger->m_nBreakFrame == MAX_INT32)
		{
			// skip invalid source while step in
			if (lua_getinfo(pState, "lS", pDebug) && pDebug->currentline > 0 && 
				(pDebug->currentline != pDebugger->m_nLastLine || 
				strcmp(pDebugger->m_strLastSorece.c_str(), pDebug->source) != 0))
			{
				pDebugger->m_strLastSorece = pDebug->source;
				pDebugger->m_nLastLine = pDebug->currentline;
				return pDebugger->Debug(pState);
			}
			return;
		}

		// always stop while step in or meet the breakpoint
		if (lua_getinfo(pState, "lS", pDebug) && 
			pDebugger->GetBreakPoint(pDebug->source, pDebug->currentline))
		{
			pDebugger->m_strLastSorece = pDebug->source;
			pDebugger->m_nLastLine = pDebug->currentline;
			return pDebugger->Debug(pState);
		}

		// no any frame is expected to be break
		if( pDebugger->m_nBreakFrame < 0 )
			return;

		// corroutine changed
		if( pState != pDebugger->m_pState )
		{
			if( pDebug->event == LUA_HOOKRET &&
				pDebugger->m_pPreState == pDebugger->m_pState )
			{
				pDebugger->m_strLastSorece = pDebug->source;
				pDebugger->m_nLastLine = pDebug->currentline;
				return pDebugger->Debug( pState );
			}
			pDebugger->m_pPreState = pState;
			return;
		}

		// ignore the return event in same corroutine
		if( pDebug->event == LUA_HOOKRET )
			return;

		// check break frame
		if( (int32)pDebugger->GetFrameCount() > pDebugger->m_nBreakFrame )
			return;

		pDebugger->m_strLastSorece = pDebug->source;
		pDebugger->m_nLastLine = pDebug->currentline;
		pDebugger->Debug( pState );
	}

	void CDebugLua::ClearVariables()
	{
		while( m_mapVariable.GetFirst() )
		{
			auto pNode = static_cast<SVariableNode*>( m_mapVariable.GetFirst() );
			while( pNode->m_mapFields[0].GetFirst() )
				pNode->m_mapFields[0].GetFirst()->Remove();
			while( pNode->m_mapFields[1].GetFirst() )
				pNode->m_mapFields[1].GetFirst()->Remove();
			delete pNode;
		}
	}

	void CDebugLua::Debug( lua_State* pState )
	{
		m_pState = pState;
		lua_sethook( pState, &CDebugLua::DebugHook, 0, 0 );
		CDebugBase::Debug();

		ClearVariables();

		lua_pushlightuserdata( m_pState, s_szValue2ID );
		lua_pushnil( m_pState );
		lua_rawset( m_pState, LUA_REGISTRYINDEX );

		lua_pushlightuserdata( m_pState, s_szID2Value );
		lua_pushnil( m_pState );
		lua_rawset( m_pState, LUA_REGISTRYINDEX );
	}

	uint32 CDebugLua::AddBreakPoint( const char* szFileName, int32 nLine )
	{
		uint32 nID = CDebugBase::AddBreakPoint( szFileName, nLine );
		lua_State* pState = (lua_State*)(GetDebugHandler()->GetVM());
		if( HaveBreakPoint() )
			lua_sethook( pState, &CDebugLua::DebugHook, STEP_DEBUG_MASK, 0 );
		return nID;
	}

	void CDebugLua::DelBreakPoint( uint32 nBreakPointID )
	{
		CDebugBase::DelBreakPoint( nBreakPointID );
		lua_State* pState = (lua_State*)(GetDebugHandler()->GetVM());
		if( HaveBreakPoint() || m_nBreakFrame >= 0 )
			return;
		lua_sethook( pState, &CDebugLua::DebugHook, 0, 0 );
	}

	void CDebugLua::Stop()
	{
		SetCurState( (lua_State*)(GetDebugHandler()->GetVM()) );
		StepIn();
	}

	void CDebugLua::Continue()
	{
		if( !HaveBreakPoint() )
			return;
		lua_sethook( m_pState, &CDebugLua::DebugHook, STEP_DEBUG_MASK, 0 );
		m_nBreakFrame = -1;
		m_pPreState = m_pState;
	}

    void CDebugLua::StepNext()
    {
        lua_sethook( m_pState, &CDebugLua::DebugHook, LUA_MASKLINE, 0 );
		m_nBreakFrame = GetFrameCount();
		m_pPreState = m_pState;
    }

    void CDebugLua::StepIn()
    {
        lua_sethook( m_pState, &CDebugLua::DebugHook, STEP_DEBUG_MASK, 0 );
		m_nBreakFrame = MAX_INT32;
		m_pPreState = m_pState;
    }

    void CDebugLua::StepOut()
    {
        lua_sethook( m_pState, &CDebugLua::DebugHook, STEP_DEBUG_MASK, 0 );
		m_nBreakFrame = (int32)GetFrameCount() - 1;
		m_pPreState = m_pState;
    }

	int32 CDebugLua::SwitchFrame( int32 nCurFrame )
	{
		HackLuaDebug ld = {};
		if( !lua_getstack( m_pState, nCurFrame, &ld ) )
			return -1;

		ClearVariables();

		lua_pushlightuserdata( m_pState, s_szValue2ID );
		lua_newtable( m_pState );
		lua_rawset( m_pState, LUA_REGISTRYINDEX );

		lua_pushlightuserdata( m_pState, s_szID2Value );
		lua_newtable( m_pState );
		lua_rawset( m_pState, LUA_REGISTRYINDEX );

		SVariableNode* pNode = new SVariableNode;
		pNode->m_strField = "Scope";
		pNode->m_nRegisterID = ePDVID_Scopes;
		pNode->m_nVariableID = ePDVID_Scopes;
		pNode->m_nParentID = -1;
		m_mapVariable.Insert( *pNode );

		// global value
		m_nValueID = ePDVID_Scopes;
		lua_getglobal( m_pState, "_G" );
		TouchVariable( "Global", ePDVID_Scopes );

		// local value
		lua_newtable( m_pState );
		const char* name = nullptr;
		int32 nLocalIndex = lua_gettop( m_pState );

		#ifndef lua_equal
		#define lua_equal lua_rawequal
		#endif
		for( int n = 1; ( name = lua_getlocal( m_pState, &ld, n ) ) != nullptr; n++ )
		{
			if( lua_equal( m_pState, nLocalIndex, -1 ) )
				lua_pop( m_pState, 1 );
			else
				lua_setfield( m_pState, -2, name[0] ? name : "(anonymous local)" );
		}
		TouchVariable( "Local", ePDVID_Scopes );

		// up value
		lua_newtable(m_pState);
		int32 nCurCount = GetFrameCount() - 1;
		while( nCurCount >= 0 )
		{
			if( !lua_getstack( m_pState, nCurCount, &ld ) )
				break;
			lua_getinfo( m_pState, "f", &ld );
			for( int n = 1; ( name = lua_getupvalue( m_pState, -1, n ) ) != nullptr; n++ )
				lua_setfield( m_pState, -3, name[0] ? name : "(anonymous upvalue)" );
			lua_pop( m_pState, 1 );
			nCurCount--;
		}

		TouchVariable( "UpValue", ePDVID_Scopes );

		// tempory value
		lua_newtable( m_pState );
		TouchVariable( "Tempory", ePDVID_Scopes );
		return nCurFrame;
	}

	uint32 CDebugLua::TouchVariable( const char* szField, uint32 nParentID )
	{
		bool bIndex = szField[0] >= '0' && szField[0] <= '9';
		SVariableInfo* pInfo = m_mapVariable.Find( nParentID );
		if( pInfo == nullptr )
		{
			lua_pop( m_pState, 1 );
			return INVALID_32BITID;
		}

		CFieldMap& mapFields = pInfo->m_mapFields[bIndex];
		SFieldInfo* pField = mapFields.Find( const_string( szField, true ) );
		if( pField )
		{
			lua_pop( m_pState, 1 );
			return static_cast<SVariableNode*>(pField)->m_nVariableID;
		}

		lua_pushlightuserdata( m_pState, s_szValue2ID );
		lua_rawget( m_pState, LUA_REGISTRYINDEX );			//2[value,value2ID]
		lua_pushvalue( m_pState, -2 );						//3[value,value2ID,value]
		lua_rawget( m_pState, -2 );							//3[value,value2ID,id]
		uint32 nRegisterID = (uint32)lua_tonumber( m_pState, -1 );
		if( nRegisterID )
		{
			lua_pop( m_pState, 3 );
			SVariableNode* pNode = new SVariableNode;
			pNode->m_strField = szField;
			pNode->m_nRegisterID = nRegisterID;
			pNode->m_nVariableID = ++m_nValueID;
			pNode->m_nParentID = nParentID;
			mapFields.Insert( *pNode );
			m_mapVariable.Insert( *pNode );
			return pNode->m_nVariableID;
		}
		lua_pop( m_pState, 1 );								//2[value,value2ID]

		// add to s_szValue2ID
		uint32 nID = ++m_nValueID;
		lua_pushvalue( m_pState, -2 );						//3[value,value2ID,value]
		lua_pushnumber( m_pState, nID );					//4[value,value2ID,value,id]
		lua_rawset( m_pState, -3 );   						//2[value,value2ID]
		lua_pop( m_pState, 1 );   							//1[value]

		// add to s_szID2Value
		lua_pushlightuserdata( m_pState, s_szID2Value );
		lua_rawget( m_pState, LUA_REGISTRYINDEX );			//2[value,ID2Value]
		lua_pushnumber( m_pState, nID );					//3[value,ID2Value,id]
		lua_pushvalue( m_pState, -3 );						//4[value,ID2Value,id,value]
		lua_rawset( m_pState, -3 );   						//2[value,ID2Value]
		lua_pop( m_pState, 2 );

		SVariableNode* pNode = new SVariableNode;
		pNode->m_strField = szField;
		pNode->m_nRegisterID = nID;
		pNode->m_nVariableID = nID;
		pNode->m_nParentID = nParentID;
		mapFields.Insert( *pNode );
		m_mapVariable.Insert( *pNode );
		return nID;
	}

	uint32 CDebugLua::GetScopeChainID( int32 nCurFrame )
	{
		return ePDVID_Scopes;
	}

	SValueInfo CDebugLua::GetVariable( uint32 nID )
	{
		SValueInfo Info;
		Info.nID = nID;
		if( nID == ePDVID_Scopes )
		{
			Info.strName = "scope";
			Info.nNameValues = 3;
			return Info;
		}

		SVariableInfo* pInfo = m_mapVariable.Find( nID );
		if (pInfo == nullptr)
			return SValueInfo();

		auto pNode = static_cast<SVariableNode*>( pInfo );
		Info.strName = pNode->m_strField.c_str();
		Info.strValue = Info.strName;

		if( nID >= ePDVID_Count )
		{
			lua_pushlightuserdata( m_pState, s_szID2Value );
			lua_rawget( m_pState, LUA_REGISTRYINDEX );
			lua_pushnumber( m_pState, pNode->m_nRegisterID );
			lua_rawget( m_pState, -2 );
			void* nObjectIndex = (void*)(ptrdiff_t)lua_gettop( m_pState );
			const char* s = GetDebugHandler()->PresentValue( nObjectIndex, m_pState );
			Info.strValue = s ? s : "nil";
			lua_pop( m_pState, 2 );
		}

		Info.nIndexValues = GetChildrenID( nID, true, 0 );
		Info.nNameValues = GetChildrenID( nID, false, 0 );
		return Info;
	}

	uint32 CDebugLua::GetChildrenID( uint32 nParentID, bool bIndex, 
		uint32 nStart, uint32* aryChild, uint32 nCount )
	{
		if( nParentID == ePDVID_Scopes )
		{
			if( bIndex )
				return 0;
			if( aryChild )
			{
				aryChild[0] = ePDVID_Global;
				aryChild[1] = ePDVID_UpValue;
				aryChild[2] = ePDVID_Local;
			}
			return 3;
		}

		SVariableInfo* pInfo = m_mapVariable.Find( nParentID );
		if( pInfo == nullptr )
			return 0;
		auto pNode = static_cast<SVariableNode*>( pInfo );
		CFieldMap& mapFields = pInfo->m_mapFields[bIndex];

		if( pInfo->m_mapFields[0].IsEmpty() && pInfo->m_mapFields[1].IsEmpty() )
		{
			int32 nTop = lua_gettop( m_pState );
			lua_pushlightuserdata( m_pState, s_szID2Value );
			lua_rawget( m_pState, LUA_REGISTRYINDEX );
			lua_pushnumber( m_pState, pNode->m_nRegisterID );
			lua_rawget( m_pState, -2 );

			if( lua_istable( m_pState, -1 ) )
			{
				lua_pushnil( m_pState );
				while( lua_next( m_pState, -2 ) )
				{
					lua_pushvalue( m_pState, -2 );
					void* nObjectIndex = (void*)(ptrdiff_t)lua_gettop( m_pState );
					const char* szName = GetDebugHandler()->PresentValue( nObjectIndex, m_pState );
					lua_pop( m_pState, 1 );
					TouchVariable( szName, nParentID );
				}
			}

			int nType = lua_type( m_pState, -1 );
			if( ( nType == LUA_TTABLE || nType == LUA_TUSERDATA ) && 
				lua_getmetatable( m_pState, -1 ) )
				TouchVariable( "(metatable)", nParentID );

			lua_settop( m_pState, nTop );
			assert( nTop == lua_gettop( m_pState ) );
		}

		uint32 nIndex = 0, nCurCount = 0;
		for( auto pField = mapFields.GetFirst(); pField;
			pField = pField->GetNext(), nIndex++ )
		{
			auto pNode = static_cast<SVariableNode*>( pField );
			if( nIndex < nStart )
				continue;
			if( nCurCount >= nCount )
				break;
			if( aryChild )
				aryChild[nCurCount] = pNode->m_nVariableID;
			nCurCount++;
		}
		return nCurCount;
	}

	uint32 CDebugLua::EvaluateExpression(int32 nCurFrame, const char* szName)
	{
		if (szName == nullptr)
			return INVALID_32BITID;
		const_string strKey(szName, true);

		uint32 aryID[] = { ePDVID_Local, ePDVID_UpValue, ePDVID_Global };
		for (uint32 i = 0; i < ELEM_COUNT(aryID); i++)
		{
			SVariableInfo* pInfo = m_mapVariable.Find(aryID[i]);
			if (!pInfo)
				continue;
			GetChildrenID(aryID[i], false, 0);
			GetChildrenID(aryID[i], true, 0);
			SFieldInfo* pField = pInfo->m_mapFields[0].Find(strKey);
			if (!pField)
				continue;
			return static_cast<SVariableNode*>(pField)->m_nVariableID;
		}

		return INVALID_32BITID;
	}
}
