#ifndef __DEBUG_BASE_H__
#define __DEBUG_BASE_H__
//=====================================================================
// CDebugBase.h
// 定义基本的调试器接口
// 柯达昭
// 2007-10-16
//=====================================================================

#include "core/GammaScriptDef.h"
#include "common/TConstString.h"
#include "common/TList.h"
#include "common/CThread.h"
#include "common/CJson.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>

namespace Gamma
{
	class CDebugCmd;
	class CScriptBase;
	class CBreakPoint;
	typedef std::set<CBreakPoint> CBreakPointList;
	typedef std::vector<std::string> CFileLines;
	typedef std::map<gammacstring, CFileLines> CFileMap;
	typedef TList<CDebugCmd> CDebugCmdList;
	typedef CDebugCmdList::CListNode CDebugNode;
	class CDebugCmd : public CDebugNode, public CJson{};

	struct SException 
	{ 
		const char* szException; 
		bool bBeCaught; 
	};

	struct SValueInfo 
	{ 
		SValueInfo( const char* szValue = "" ) 
			: nID( 0 )
			, nNameValues( 0 )
			, nIndexValues( 0 )
			, strValue( szValue ? szValue : "" )
		{}
		uint32		nID;
		std::string strName;
		std::string strValue; 
		uint32		nNameValues;
		uint32		nIndexValues;
	};

    class CBreakPoint : public gammacstring
	{
		uint32				m_nBreakPointID;
		uint32				m_nFileNameStart;
		uint32				m_nLineNum;
	public:
        CBreakPoint( uint32 nID, const char* szFileName, bool bRef, uint32 uLineNum );
        bool operator < ( const CBreakPoint& ano ) const;
		bool operator == ( const CBreakPoint& ano ) const;
		const char* GetModuleName() const { return c_str() + m_nFileNameStart; }
		uint32 GetBreakPointID() const { return m_nBreakPointID; }
		uint32 GetLineNum() const { return m_nLineNum; }
	};

    class CDebugBase
	{
	protected:
		enum EAttachType
		{ 
			eAT_Detach, 
			eAT_Waiting, 
			eAT_Launch, 
			eAT_Attach 
		};

		CScriptBase*		m_pBase;
		HTHREAD				m_hThread;
		HSEMAPHORE			m_hSemaphore;
		HLOCK				m_hCmdLock;
		intptr_t			m_nRemoteListener;
		intptr_t			m_nRemoteConnecter;
		CDebugCmdList		m_listDebugCmd;

		char*				m_pBuf;
		char				m_szBuffer[1024];
		CBreakPointList		m_setBreakPoint;
		CFileMap			m_mapFileBuffer;

		EAttachType			m_eAttachType;
		bool				m_bRemoteCmdValid;
		bool				m_bAllExceptionsBreak;
		bool				m_bUncaughtExceptionsBreak;
		bool				m_bPrintFrame;
		bool				m_bEnterDebug;
		bool				m_bExpectStep;
		uint32				m_nExceptionID;
		std::string			m_szException;
		std::string			m_szStringSend;
		
		int32				m_nCurFrame;
		int32				m_nCurLine;
		std::string			m_strLastVarName;

		//=================================================================
		// ConsoleDebug
		//=================================================================
		void				ConsoleDebug( SException* pException );
		const char*			ReadWord( bool bNewLine = false );
		bool				PrintLine( int32 nFrame, const char* szSource, int32 nLine, bool bIsCurLine );
		void				PrintFrame( int32 nFrame, const char* szFun, const char* szSource, int32 nLine );
		const char*			ReadFileLine( const char* szSource, int32 nLine );

		void				AddBreakPoint( const char* szBuf );
		void				PrintBreakInfo();
		void				ShowFileLines( int32 nCurLine );

		//=================================================================
		// RemoteDebug
		// https://microsoft.github.io/debug-adapter-protocol/
		//=================================================================
		void				RemoteDebug( SException* pException );
		void				CmdLock();
		void				CmdUnLock();
		void				ListenRemote( uint16 nDebugPort );
		void				TeminateRemote( const char* szSequence );
		void				Run();
		virtual bool		CheckRemoteSocket( char(&szBuffer)[2048], int32 nCurSize );

		void				SendEvent( CJson* pBody, const char* szEvent );
		void				SendRespone( CJson* pBody, const char* szSequence, 
								bool bSucceeded, const char* szCommand, const char* szMsg = "" );
		void				SendNetData( CJson* pJson );
		void				OnNetData( CDebugCmd* pCmd );
		virtual bool		ProcessCommand( CDebugCmd* pCmd );

	protected:
		CScriptBase*		GetScriptBase() const { return m_pBase; }
		virtual void		ReadFile( std::string& strBuffer, const char* szFileName );
		virtual uint32		GenBreakPointID( const char* szFileName, int32 nLine ) = 0;
		bool				HaveBreakPoint() const { return !m_setBreakPoint.empty(); }

    public:
        CDebugBase( CScriptBase* pBase, uint16 nDebugPort );
		virtual ~CDebugBase(void);

		void				Debug();
		bool				Error( const char* szException, bool bBeCaught );
		void				BTrace( int32 nFrameCount );
		void				AddFileContent( const char* szSource, const char* szData );
		bool				RemoteDebugEnable() const;
		bool				RemoteCmdValid() const { return m_bRemoteCmdValid; }
		bool				CheckRemoteCmd();

		virtual uint32		AddBreakPoint( const char* szFileName, int32 nLine );
		virtual void		DelBreakPoint( uint32 nBreakPointID );
		const CBreakPoint*	GetBreakPoint( const char* szSource, int32 nLine );

		virtual uint32		GetFrameCount() = 0;
		virtual bool		GetFrameInfo( int32 nFrame, int32* nLine, 
								const char** szFunction, const char** szSource ) = 0;
		virtual int32		SwitchFrame( int32 nCurFrame ) = 0;
		virtual uint32		GetVariableID( int32 nCurFrame, const char* szName ) = 0;
		virtual uint32		GetChildrenID( uint32 nParentID, bool bIndex, 
								uint32 nStart, uint32* aryChild, uint32 nCount ) = 0;
		virtual SValueInfo	GetVariable( uint32 nID ) = 0;
		virtual void		Stop() = 0;
		virtual void		Continue() = 0;
		virtual void		StepIn() = 0;
		virtual void		StepNext() = 0;
		virtual void		StepOut() = 0;
    };
}

#endif
