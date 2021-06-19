/**@file  		CLuaDebug.h
* @brief		LUA debugger interface
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
*/

#ifndef __LUA_DEBUG_H__
#define __LUA_DEBUG_H__
#include "common/TRBTree.h"
#include "core/CDebugBase.h"
#include <vector>

struct lua_State;
struct lua_Debug;
struct lua_TValue;

namespace XS
{
    class CDebugLua : public CDebugBase
	{
		struct SFieldInfo;
		struct SVariableInfo;
		typedef TRBTree<SFieldInfo> CFieldMap;
		typedef TRBTree<SVariableInfo> CVariableMap;

		struct SFieldInfo : public CFieldMap::CRBTreeNode
		{
			const_string	m_strField;
			uint32			m_nRegisterID;
			operator const const_string&( ) const { return m_strField; }
			bool operator < ( const const_string& strKey ) { return m_strField < strKey; }
		};

		struct SVariableInfo : public CVariableMap::CRBTreeNode
		{
			uint32			m_nParentID;
			uint32			m_nVariableID;
			CFieldMap		m_mapFields[2];
			operator const uint32( ) const { return m_nVariableID; }
			bool operator < ( uint32 nKey ) { return (uint32)*this < nKey; }
		};

		struct SVariableNode : public SFieldInfo, public SVariableInfo {};

        lua_State*			m_pState;
		lua_State*			m_pPreState;
        int32				m_nBreakFrame;

		uint32				m_nValueID;
		CVariableMap		m_mapVariable;
		std::string			m_szFunctionName;
		std::string			m_strLastSorece;
		int32				m_nLastLine;

        static void			DebugHook( lua_State *pState, lua_Debug* pDebug );
		static CDebugLua*	GetDebugger( lua_State* pState );
		void				Debug( lua_State* pState );

		void				ClearVariables();
		uint32				TouchVariable( const char* szField, uint32 nParentID );
		virtual uint32		GenBreakPointID( const char* szFileName, int32 nLine );
    public:
        CDebugLua( IDebugHandler* pHandler, const char* strDebugHost, uint16 nDebugPort );
        ~CDebugLua(void);

		void				SetCurState( lua_State* pL );
		
		virtual uint32		AddBreakPoint( const char* szFileName, int32 nLine );
		virtual void		DelBreakPoint( uint32 nBreakPointID );
		virtual uint32		GetFrameCount();
		virtual bool		GetFrameInfo( int32 nFrame, int32* nLine, 
								const char** szFunction, const char** szSource );
		virtual int32		SwitchFrame( int32 nCurFrame );
		virtual uint32		EvaluateExpression( int32 nCurFrame, const char* szExpression );
		virtual uint32		GetScopeChainID( int32 nCurFrame );
		virtual uint32		GetChildrenID( uint32 nParentID, bool bIndex, uint32 nStart, 
								uint32* aryChild = nullptr, uint32 nCount = INVALID_32BITID );
		virtual SValueInfo	GetVariable( uint32 nID );
		virtual void		Stop();
		virtual void		Continue();
		virtual void		StepIn();
		virtual void		StepNext();
		virtual void		StepOut();
    };
}

#endif
