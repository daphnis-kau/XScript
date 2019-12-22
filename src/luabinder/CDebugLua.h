#ifndef __LUA_DEBUG_H__
#define __LUA_DEBUG_H__
//=====================================================================
// CLuaDebug.h 
// 为lua定义的调试器接口
// 柯达昭
// 2007-10-16
//=====================================================================

#include "core/CDebugBase.h"
#include <vector>

struct lua_State;
struct lua_Debug;

namespace Gamma
{
    class CDebugLua : public CDebugBase
	{
        lua_State*			m_pState;
		lua_State*			m_pPreState;
        int32				m_nBreakFrame;
		uint32				m_nValueID;

        static void			DebugHook( lua_State *pState, lua_Debug* pDebug );
		void				Debug( lua_State* pState );

		uint32				GetVariableField( const char* szField );
		virtual void		ReadFile( std::string& strBuffer, const char* szFileName );
		virtual uint32		GenBreakPointID( const char* szFileName, int32 nLine );
    public:
        CDebugLua( CScriptBase* pBase, uint16 nDebugPort );
        ~CDebugLua(void);

		void				SetCurState( lua_State* pL );
		
		virtual uint32		AddBreakPoint( const char* szFileName, int32 nLine );
		virtual void		DelBreakPoint( uint32 nBreakPointID );
		virtual uint32		GetFrameCount();
		virtual bool		GetFrameInfo( int32 nFrame, int32* nLine, 
								const char** szFunction, const char** szSource );
		virtual int32		SwitchFrame( int32 nCurFrame );
		virtual uint32		GetVariableID( int32 nCurFrame, const char* szName );
		virtual uint32		GetChildrenID( uint32 nParentID, bool bIndex, 
								uint32 nStart, uint32* aryChild, uint32 nCount );
		virtual SValueInfo	GetVariable( uint32 nID );
		virtual void		Stop();
		virtual void		Continue();
		virtual void		StepIn();
		virtual void		StepNext();
		virtual void		StepOut();
    };

}

#endif
