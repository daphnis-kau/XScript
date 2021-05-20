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
	#include "lstate.h"
	#include "lualib.h"
	#include "lvm.h"
}

namespace XS
{
	enum EPreDefinedVariableID
	{
		ePDVID_Scopes	= 1,
		ePDVID_Global	= 2,
		ePDVID_Local	= 3,
		ePDVID_UpValue	= 4,
		ePDVID_Tempory  = 5,
		ePDVID_Count	= 6,
	};

	#define LUA_MASKALL (LUA_MASKCALL|LUA_MASKRET|LUA_MASKLINE)
	static void* s_szValue2ID = (void*)"v2i";
	static void* s_szID2Value = ( void* )"i2v";

    CDebugLua::CDebugLua( CScriptBase* pBase, const char* strDebugHost, uint16 nDebugPort )
        : CDebugBase( pBase, strDebugHost, nDebugPort )
		, m_pState( nullptr )
		, m_pPreState( nullptr )
        , m_nBreakFrame( -1 )
		, m_nValueID( ePDVID_Count )
	{
    }

	CDebugLua::~CDebugLua( void )
    {
	}

	void CDebugLua::SetCurState( lua_State* pL )
	{
		m_pState = pL;
		m_pPreState = nullptr;
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
			std::set<void*> setReached;
			CallInfo* pCallInfo = m_pState->base_ci + ld.i_ci;
			lua_pushlightuserdata( m_pState, s_szID2Value );
			lua_rawget( m_pState, LUA_REGISTRYINDEX );				//1[ID2Value]
			for( int32 i = ePDVID_Scopes; !*szFunction && i < ePDVID_Tempory; i++ )
			{
				if( !lua_istable( m_pState, -1 ) )
					continue;
				lua_pushnumber( m_pState, i );						//2[ID2Value,id]
				lua_rawget( m_pState, -2 );   						//2[ID2Value,value]
				void* pTarget = pCallInfo->func->value.gc;
				int tableIndex = lua_gettop( m_pState );
				TValue* vTable = m_pState->base + tableIndex - 1;
				if( *szFunction = GetFuncName( pTarget, vTable, setReached ) )
				{
					if( !**szFunction )
						*szFunction = nullptr;
					lua_pop( m_pState, 1 );
					break;
				}
				lua_pop( m_pState, 1 );
			}
			lua_pop( m_pState, 1 );
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

	uint32 CDebugLua::GenBreakPointID( const char* szFileName, int32 nLine )
	{
		static uint32 s_nBreakPointID = 1;
		return s_nBreakPointID++;
	}

	const char* CDebugLua::GetFuncName( void* pGC, lua_TValue* vTable, std::set<void*>& setReached )
	{		
		if( ttype( vTable ) != LUA_TTABLE ||
			setReached.find( vTable->value.gc ) != setReached.end() )
			return nullptr;
		setReached.insert( vTable->value.gc );
		Table* pTable = hvalue( vTable );

		const char* szName = nullptr;
		for( int32 i = 0; !szName && i < pTable->sizearray; i++ )
		{
			if( ttisnil( &pTable->array[i] ) )
				continue;
			if( ttistable( &pTable->array[i] ) )
				szName = GetFuncName( pGC, &pTable->array[i], setReached );
			if( pTable->array[i].tt != LUA_TFUNCTION ||
				pTable->array[i].value.gc != pGC )
				continue;
			char_stream( m_szFunctionName ) << i + 1;
			return m_szFunctionName.c_str();
		}

		for( int32 i = 0; !szName && i < sizenode( pTable ); i++ )
		{  
			if( ttisnil( &pTable->node[i].i_val ) )
				continue;
			if( ttistable( &pTable->node[i].i_val ) )
				szName = GetFuncName( pGC, &pTable->node[i].i_val, setReached );
			if( pTable->node[i].i_val.tt != LUA_TFUNCTION ||
				pTable->node[i].i_val.value.gc != pGC )
				continue;
			if( pTable->node[i].i_key.tvk.tt == LUA_TNUMBER )
			{
				char_stream( m_szFunctionName ) << pTable->node[i].i_key.tvk.value.n;
				return m_szFunctionName.c_str();
			}

			if( pTable->node[i].i_key.tvk.tt == LUA_TSTRING )
			{
				char_stream( m_szFunctionName ) << pTable->node[i].i_key.tvk.value.n;
				return svalue( &pTable->node[i].i_key.tvk );
			}

			return "";
		}
		return szName;
	}

	void CDebugLua::DebugHook( lua_State* pState, lua_Debug* pDebug )
    {
		auto pScriptLua = CScriptLua::GetScript( pState );
		auto pDebugger = static_cast<CDebugLua*>( pScriptLua->GetDebugger() );

		// always stop while step in or meet the breakpoint
		if( pDebugger->m_nBreakFrame == MAX_INT32 ||
			( lua_getinfo( pState, "lS", pDebug ) &&
				pDebugger->GetBreakPoint( pDebug->source, pDebug->currentline ) ) )
			return pDebugger->Debug( pState );

		// no any frame is expected to be break
		if( pDebugger->m_nBreakFrame < 0 )
			return;

		// corroutine changed
		if( pState != pDebugger->m_pState )
		{
			if( pDebug->event == LUA_HOOKRET && 
				pDebugger->m_pPreState == pDebugger->m_pState )
				return pDebugger->Debug( pState );
			pDebugger->m_pPreState = pState;
			return;
		}

		// ignore the return event in same corroutine
		if( pDebug->event == LUA_HOOKRET )
			return;

		// check break frame
		if( (int32)pDebugger->GetFrameCount() > pDebugger->m_nBreakFrame )
			return;

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
		CScriptLua* pScriptLua = static_cast<CScriptLua*>( GetScriptBase() );
		lua_State* pState = pScriptLua->GetLuaState();
		if( HaveBreakPoint() )
			lua_sethook( pState, &CDebugLua::DebugHook, LUA_MASKALL, 0 );
		return nID;
	}

	void CDebugLua::DelBreakPoint( uint32 nBreakPointID )
	{
		CDebugBase::DelBreakPoint( nBreakPointID );
		CScriptLua* pScriptLua = static_cast<CScriptLua*>( GetScriptBase() );
		lua_State* pState = pScriptLua->GetLuaState();
		if( HaveBreakPoint() || m_nBreakFrame >= 0 )
			return;
		lua_sethook( pState, &CDebugLua::DebugHook, 0, 0 );
	}

	void CDebugLua::Stop()
	{
		CScriptLua* pScriptLua = static_cast<CScriptLua*>( GetScriptBase() );
		SetCurState( pScriptLua->GetLuaState() );
		StepIn();
	}

	void CDebugLua::Continue()
	{
		if( !HaveBreakPoint() )
			return;
		lua_sethook( m_pState, &CDebugLua::DebugHook, LUA_MASKALL, 0 );
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
        lua_sethook( m_pState, &CDebugLua::DebugHook, LUA_MASKALL, 0 );
		m_nBreakFrame = MAX_INT32;
		m_pPreState = m_pState;
    }

    void CDebugLua::StepOut()
    {
        lua_sethook( m_pState, &CDebugLua::DebugHook, LUA_MASKLINE|LUA_MASKRET, 0 );
		m_nBreakFrame = (int32)GetFrameCount() - 1;
		m_pPreState = m_pState;
    }

	int32 CDebugLua::SwitchFrame( int32 nCurFrame )
	{
		lua_Debug ld;
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
		m_mapVariable.Insert( *pNode );

		// global value
		m_nValueID = ePDVID_Scopes;
		lua_getglobal( m_pState, "_G" );
		TouchVariable( "Global", ePDVID_Scopes );

		// local value
		lua_newtable( m_pState );
		const char* name = nullptr;
		for( int n = 1; ( name = lua_getlocal( m_pState, &ld, n ) ) != nullptr; n++ )
		{
			if( lua_equal( m_pState, -1, -2 ) )
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
			return static_cast<SVariableNode*>( pField )->m_nVariableID;

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
		mapFields.Insert( *pNode );
		m_mapVariable.Insert( *pNode );
		return nID;
	}

	uint32 CDebugLua::GetScopeChainID( int32 nCurFrame )
	{
		return ePDVID_Scopes;
	}

	XS::SValueInfo CDebugLua::GetVariable( uint32 nID )
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
			lua_remove( m_pState, -2 );
			CScriptLua::ToString( m_pState );
			const char* s = lua_tostring( m_pState, -1 );
			Info.strValue = s ? s : "nil";
			lua_pop( m_pState, 1 );
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
		CScriptLua* pScriptLua = static_cast<CScriptLua*>( GetScriptBase() );

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
					pScriptLua->ToString( m_pState );
					const char* szName = lua_tostring( m_pState, -1 );
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
