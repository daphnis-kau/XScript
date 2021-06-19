#include "common/Help.h"
#include "common/CJson.h"
#include "core/CDebugBase.h"
#include <algorithm>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <io.h>
#include <stdio.h>
#include <signal.h>
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#define StrCaseCmp stricmp
#else
#include <alloca.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cerrno>
#include <netdb.h>
#include <fcntl.h>
#define INVALID_SOCKET		-1
#define closesocket			close
#define ioctlsocket			ioctl
typedef int32				SOCKET;
typedef struct linger		LINGER;
#define SOCKET_ERROR		-1
#define SD_SEND				SHUT_WR
#define StrCaseCmp			strcasecmp
#endif

#undef min
#undef max

//#define LOG_REMOTE_COMMAND
#define LINE_COUNT_ON_SHOW 16

namespace XS
{
	//-----------------------------------------------------
	// CBreakPoint
	//-----------------------------------------------------
    CBreakPoint::CBreakPoint( uint32 nID, 
		const char* szFileName, bool bRef, uint32 nLineNum )
		: const_string( szFileName, bRef )
		, m_nBreakPointID( nID )
		, m_nLineNum( nLineNum )
		, m_nFileNameStart( 0 )
	{
		assert( szFileName );
		for( const char* pStart = szFileName; *pStart; pStart++ )
			if( *pStart == '\\' || *pStart == '/' )
				m_nFileNameStart = (int32)( pStart - szFileName + 1 );
    }

    bool CBreakPoint::operator < ( const CBreakPoint& ano )const
    {
        if( m_nLineNum < ano.m_nLineNum )
            return true;
        if( m_nLineNum > ano.m_nLineNum )
            return false;
		return strcmp( GetModuleName(), ano.GetModuleName() ) < 0;
    }
	
    bool CBreakPoint::operator == ( const CBreakPoint& ano ) const
    {
        return m_nLineNum == ano.m_nLineNum && ( strcmp( GetModuleName(), ano.GetModuleName() ) ) == 0;
    }

	//-----------------------------------------------------
	// CBreakPoint
	//-----------------------------------------------------
    CDebugBase::CDebugBase( IDebugHandler* pHandler,
		const char* strDebugHost, uint16 nDebugPort )
		: m_pHandler( pHandler )
		, m_nRemoteListener( INVALID_SOCKET )
		, m_nRemoteConnecter( INVALID_SOCKET )
		, m_bAllExceptionsBreak( false )
		, m_bUncaughtExceptionsBreak( false )
		, m_bPrintFrame( true )
		, m_pBuf( nullptr )
		, m_bLoopOnPause( false )
		, m_bEnterDebug( false )
		, m_eAttachType( eAT_Detach )
		, m_nExceptionID( 1 )
		, m_bExpectStep( false )
		, m_bQuit( false )
		, m_nCurFrame( 0 )
    {
		if( !nDebugPort )
			return;
		ListenRemote(strDebugHost, nDebugPort );
    }

    CDebugBase::~CDebugBase(void)
	{
		m_bQuit = true;
		if( m_hThread.joinable() )
			m_hThread.join();
		if( m_nRemoteConnecter != INVALID_SOCKET )
			closesocket( m_nRemoteConnecter );
		if( m_nRemoteListener != INVALID_SOCKET )
			closesocket( m_nRemoteListener );
		while( m_listDebugCmd.GetFirst() )
			delete m_listDebugCmd.GetFirst();
	}

	bool CDebugBase::RemoteDebugEnable() const
	{
		return m_nRemoteListener != INVALID_SOCKET;
	}

	bool CDebugBase::Error( const char* szException, bool bBeCaught )
	{
		m_pHandler->Output( "Error :", -1, true );
		m_pHandler->Output( szException, -1, true );
		m_pHandler->Output( "\n", -1, true );
		if( !m_bAllExceptionsBreak &&
			( bBeCaught || !m_bUncaughtExceptionsBreak ) )
		{
			BTrace( INVALID_32BITID );
			return false;
		}

		m_bEnterDebug = true;
		SException Exception = { szException, bBeCaught };

		if( m_nRemoteConnecter == INVALID_SOCKET )
			ConsoleDebug( &Exception );
		else
			RemoteDebug( &Exception );
		m_bEnterDebug = false;
		return true;
	}

	void CDebugBase::Debug()
	{
		m_bEnterDebug = true;
		m_nCurFrame = SwitchFrame( 0 );
		if( m_nRemoteConnecter == INVALID_SOCKET )
			ConsoleDebug( nullptr );
		else
			RemoteDebug( nullptr );
		m_bEnterDebug = false;
	}

	void CDebugBase::BTrace( int32 nFrameCount )
	{
		int32 nCurFrame = 0;
		int32 nCurLine = 0;
		const char* szFunction = nullptr;
		const char* szSource = nullptr;
		while( (uint32)nCurFrame < (uint32)nFrameCount )
		{ 
			if( !GetFrameInfo( nCurFrame, &nCurLine, &szFunction, &szSource ) )
				break;
			PrintFrame( nCurFrame++, szFunction, szSource, nCurLine );
		}
	}

	void CDebugBase::AddFileContent( const char* szSource, const char* szData )
	{
		CFileLines& vecLines = m_mapFileBuffer[szSource];
		if( !szData || !szData[0] )
			return;
		const char* pCur = szData;
		const char* pStart = pCur;
		while( *pCur )
		{
			if( *pCur == '\n' || *pCur == '\r' )
			{
				vecLines.push_back( std::string( pStart, pCur - pStart ) );
				if( *pCur == '\r' && pCur[1] == '\n' )
					pCur++;
				pStart = pCur + 1;
			}
			*pCur++;
		}
		vecLines.push_back( std::string( pStart, pCur - pStart ) );
	}

	std::string CDebugBase::ReadEntirFile( const char* szFileName )
	{
		std::string strBuffer;
		void* pContext = m_pHandler->OpenFile( szFileName );
		if( !pContext )
			return strBuffer;
		char szBuffer[1024];
		int32 nReadSize = 0;
		while( (nReadSize = m_pHandler->ReadFile( pContext, szBuffer, 1024 )) > 0 )
			strBuffer.append( szBuffer, nReadSize );
		m_pHandler->CloseFile( pContext );
		return strBuffer;
	}

	const char* CDebugBase::ReadFileLine( const char* szSource, int32 nLine )
	{
		CFileMap::iterator it = m_mapFileBuffer.find( szSource );
		if( m_mapFileBuffer.end() == it || it->second.empty() )
		{
			std::string strBuffer = ReadEntirFile( szSource );
			AddFileContent( szSource, strBuffer.c_str() );
			it = m_mapFileBuffer.find( szSource );
		}

		if( nLine <= 0 || nLine > (int32)it->second.size() )
			return nullptr;
		return it->second[nLine - 1].c_str();
	}

	//=================================================================
	// RemoteDebug
	//=================================================================
	void CDebugBase::ListenRemote(const char* strDebugHost, uint16 nDebugPort )
	{
#ifdef _WIN32
		WORD wVersion;
		WSADATA wsaData;
		wVersion = MAKEWORD( 2, 2 );
		if( WSAStartup( wVersion, &wsaData ) )
			return;
#endif

		//创建socket
		m_nRemoteListener = (uint32)socket( AF_INET, SOCK_STREAM, 0 );
		if( m_nRemoteListener == INVALID_SOCKET )
			return;

#ifdef _WIN32
		u_long mode = 0;
		if (0 != ioctlsocket(m_nRemoteListener, FIONBIO, &mode))
			return;
#else
		int flags = fcntl(m_nRemoteListener, F_GETFL, 0);
		if (-1 == flags)
		{
			return;
		}
		flags &= ~O_NONBLOCK;
		if (-1 == fcntl(m_nRemoteListener, F_SETFL, flags))
		{
			return;
		}
#endif // !_WIN32


		int32 nVal = 1;
		if( SOCKET_ERROR == setsockopt( m_nRemoteListener, SOL_SOCKET, 
			SO_REUSEADDR, (const char*)(&nVal), sizeof(nVal) ) )
		{
			closesocket( m_nRemoteListener );
			m_nRemoteListener = INVALID_SOCKET;
			return;
		}

		sockaddr_in Address;
		memset( &Address, 0, sizeof(Address) );
		Address.sin_addr.s_addr = strDebugHost ? inet_addr(strDebugHost) : 0;
		Address.sin_port = htons( nDebugPort );
		Address.sin_family = AF_INET;

		if( ::bind( m_nRemoteListener, (sockaddr*)(&Address), sizeof(sockaddr) ) )
		{
			closesocket( m_nRemoteListener );
			m_nRemoteListener = INVALID_SOCKET;
			return;
		}

		if( listen( m_nRemoteListener, INVALID_16BITID ) )
		{
			closesocket( m_nRemoteListener );
			m_nRemoteListener = INVALID_SOCKET;
			return;
		}

		struct _{  static void Run( CDebugBase* pThis ) { pThis->Run(); } };
		m_hThread = std::thread(&_::Run, this);
	}

	void CDebugBase::CmdLock()
	{
		m_hCmdLock.lock();
	}

	void CDebugBase::CmdUnLock()
	{
		m_hCmdLock.unlock();
	}

	void CDebugBase::Run()
	{
		while( !m_bQuit )
		{
			fd_set fdValid;
			FD_ZERO( &fdValid );
			FD_SET( m_nRemoteListener, &fdValid );

			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 100000;
			if( !select( (int32)(m_nRemoteListener + 1), &fdValid, NULL, NULL, &tv ) )
				continue;

			sockaddr_in Address;
			socklen_t nSize = sizeof( sockaddr_in );
			m_nRemoteConnecter = accept( m_nRemoteListener, (sockaddr*)&Address, &nSize );
			if( m_nRemoteConnecter == INVALID_SOCKET )
				continue;
			m_eAttachType = eAT_Waiting;
			m_bExpectStep = false;
			char szBuffer[2048];
			int32 nResult = (int32)recv(m_nRemoteConnecter, szBuffer, 2048, 0);
			if (ReciveRemoteData(szBuffer, nResult))
			{
				TeminateRemote(nullptr);
				continue;
			}
			closesocket(m_nRemoteConnecter);
			m_nRemoteConnecter = INVALID_SOCKET;
			m_szStringSend.clear();
		}
	}

	bool CDebugBase::ReciveRemoteData(char(&szBuffer)[2048], int32 nCurSize)
	{
		std::string strBuffer;
		while ( m_eAttachType && nCurSize > 0 && !m_bQuit )
		{
			strBuffer.append(szBuffer, nCurSize);
			while ( !m_bQuit )
			{
				std::string::size_type nFlagPos = strBuffer.find("Content-Length");
				if (nFlagPos == std::string::npos)
					break;
				std::string::size_type nStartPos = strBuffer.find(':', nFlagPos);
				if (nStartPos == std::string::npos)
					break;
				std::string::size_type nEndPos = strBuffer.find("\r\n\r\n", nStartPos);
				if (nEndPos == std::string::npos)
					break;
				const char* szStr = strBuffer.c_str() + nStartPos + 1;
				uint32 nDataSize = (uint32)strtol( szStr, NULL, 0 );
				if (strBuffer.size() < nEndPos + 4 + nDataSize)
					break;
				CDebugCmd* pCmd = new CDebugCmd;
				pCmd->Load(strBuffer.c_str() + nEndPos + 4, nDataSize);
				OnNetData(pCmd);
				strBuffer.erase(0, nEndPos + 4 + nDataSize);
			}

			while( !m_bQuit )
			{
				fd_set fdValid;
				FD_ZERO( &fdValid );
				FD_SET( m_nRemoteConnecter, &fdValid );

				struct timeval tv;
				tv.tv_sec = 0;
				tv.tv_usec = 100000;
				if( !select( (int32)(m_nRemoteConnecter + 1), &fdValid, NULL, NULL, &tv ) )
					continue;
				nCurSize = (int32)recv( m_nRemoteConnecter, szBuffer, 2048, 0 );
				break;
			}
		}
		return true;
	}

	void CDebugBase::TeminateRemote(const char* szSequence)
	{
		m_eAttachType = eAT_Detach;
		if( szSequence )
			SendRespone( nullptr, szSequence, true, "terminate" );

		CDebugCmd* pCmd = new CDebugCmd;
		pCmd->AddChild( "seq", "0" );
		pCmd->AddChild( "type", "request" );
		pCmd->AddChild( "command", "_clearall" );
		OnNetData( pCmd );
		closesocket( m_nRemoteConnecter );
		m_nRemoteConnecter = INVALID_SOCKET;
		m_szStringSend.clear();
	}

	void CDebugBase::SendNetData( CJson* pJson )
	{
		if( m_nRemoteConnecter != INVALID_SOCKET )
		{
			std::stringstream ss;
			pJson->Save( ss, INVALID_32BITID );
			const std::string& strResult = ss.str();
			uint32 nLength = (uint32)strResult.size();
			char szBuffer[256];
			snprintf( szBuffer, ELEM_COUNT(szBuffer) - 1,"Content-Length:%d\r\n\r\n", nLength);
			m_szStringSend.append( szBuffer );
			m_szStringSend.append( strResult );
		}
		delete pJson;
	}

	void CDebugBase::SendEvent( CJson* pBody, const char* szEvent )
	{
		CJson* pEvent = new CJson;
		pEvent->AddChild( "type", "event" );
		pEvent->AddChild( "event", szEvent );
		if( pBody )
			pEvent->AddChild( pBody );

#ifdef LOG_REMOTE_COMMAND
		std::stringstream oss;
		pEvent->Save(oss);
		m_pHandler->Output( "\n----------------------event begin----------------------\n", -1 );		
		m_pHandler->Output( szEvent, -1 ); m_pHandler->Output( "\n", -1 );
		m_pHandler->Output( oss.str().c_str(), -1 );
		m_pHandler->Output( "\n-----------------------event end-----------------------\n", -1 );
#endif
		SendNetData( pEvent );
	}

	void CDebugBase::SendRespone( CJson* pBody, const char* szSequence, 
		bool bSucceeded, const char* szCommand, const char* szMsg )
	{
		CJson* pRespone = new CJson;
		pRespone->AddChild( "type", "response" );
		pRespone->AddChild( "request_seq", szSequence );
		pRespone->AddChild( "success", bSucceeded ? "true" : "false" );
		pRespone->AddChild( "command", szCommand );
		pRespone->AddChild( "message", szMsg );
		if( pBody )
			pRespone->AddChild( pBody );

#ifdef LOG_REMOTE_COMMAND
		std::stringstream oss;
		pRespone->Save(oss);
		m_pHandler->Output( "\n----------------------response begin----------------------\n", -1 );
		m_pHandler->Output( szCommand, -1 );
		m_pHandler->Output( ":", -1 );
		m_pHandler->Output( szSequence, -1 );
		m_pHandler->Output( ",", -1 );
		m_pHandler->Output(oss.str().c_str(), -1 );
		m_pHandler->Output( "\n-----------------------response end-----------------------\n", -1 );
#endif
		SendNetData(pRespone);
	}

	void CDebugBase::OnNetData( CDebugCmd* pCmd )
	{
		CmdLock();
		m_listDebugCmd.PushBack( *pCmd );
		CmdUnLock();
	}

	bool CDebugBase::CheckEnterRemoteDebug()
	{
		if( m_nRemoteConnecter == -1 )
			return false;
		if( m_bEnterDebug )
			return false;
		CheckRemoteCmd();
		return true;
	}

	int32 CDebugBase::GetDebuggerState()
	{
		if( m_nRemoteListener == INVALID_SOCKET )
			return 1;
		if( !m_hThread.joinable() )
			return 2;
		if( m_nRemoteConnecter == INVALID_SOCKET )
			return 3;
		return 0;
	}

	bool CDebugBase::CheckRemoteCmd()
	{
		if( m_nRemoteConnecter == -1 )
			return false;

		bool bContinue = true;
		while( RemoteCmdValid() && bContinue )
		{
			CmdLock();
			CDebugCmd* pCmd = m_listDebugCmd.GetFirst();
			pCmd->CDebugNode::Remove();
			CmdUnLock();
			if( !pCmd )
				break;
			if( !ProcessCommand( pCmd ) )
				bContinue = false;
			delete pCmd;
		}

		if( m_szStringSend.empty() )
			return bContinue;
		const char* szData = m_szStringSend.c_str();
		uint32 nLen = (uint32)m_szStringSend.size();
		int32 nSend = send( m_nRemoteConnecter, szData, nLen, 0 );
		if( nSend <= 0 )
			return bContinue;
		m_szStringSend.erase( 0, nSend );
		return bContinue;
	}

	void CDebugBase::RemoteDebug( SException* pException )
	{
		CJson* pBody = new CJson( "body" );
		pBody->AddChild( "threadId", "1" );
		if( pException )
		{
			m_nExceptionID++;
			m_szException = pException->szException;
			pBody->AddChild( "reason", "exception" );
		}
		else
		{
			m_szException = "";
			const char* szName[] = { "breakpoint", "step" };
			pBody->AddChild( "reason", szName[m_bExpectStep] );
		}
		SendEvent( pBody, "stopped" );

		m_bLoopOnPause = true;
		while( m_bLoopOnPause )
		{
			if (!CheckRemoteCmd())
				m_bLoopOnPause = false;
			else
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	bool CDebugBase::ProcessCommand( CDebugCmd* pCmd )
	{
		char szBuf[256];
		const char* szCommand = pCmd->At<const char*>( "command" );
		const char* szSequence = pCmd->At<const char*>( "seq" );
#ifdef LOG_REMOTE_COMMAND
		std::stringstream oss;
		pCmd->Save(oss);
		m_pHandler->Output( "\n----------------------process begin----------------------\n", -1 );
		m_pHandler->Output( szCommand, -1 );
		m_pHandler->Output( ":", -1 );
		m_pHandler->Output( szSequence, -1 ); m_pHandler->Output( "\n", -1 );
		m_pHandler->Output( oss.str().c_str(), -1 );
		m_pHandler->Output( "\n-----------------------process end-----------------------\n", -1 );
#endif

		if( !StrCaseCmp( szCommand, "initialize" ) )
		{
			CJson* pBody = new CJson( "body" );
			pBody->AddChild( "supportsEvaluateForHovers", "true" );
			//pBody->AddChild( "supportsExceptionInfoRequest", "true" );
			//pBody->AddChild( "supportsExceptionOptions", "true" );
			SendRespone( pBody, szSequence, true, szCommand );
			SendEvent( nullptr, "initialized" );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "launch" ) )
		{
			CJson* pArg = pCmd->GetChild( "arguments" );
			bool bNoDebug = pArg->At<bool>( "noDebug" );
			m_eAttachType = bNoDebug ? eAT_Detach : eAT_Launch;
			SendRespone( nullptr, szSequence, true, szCommand );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "attach" ) )
		{
			m_eAttachType = eAT_Attach;
			CJson* pArg = pCmd->GetChild("arguments");
			m_strCWD = pArg->At<std::string>("cwd");
			if (m_strCWD == "local")
			{
				m_strCWD.clear();
			}
			SendRespone( nullptr, szSequence, true, szCommand );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "loadedSources" ) )
		{
			CJson* pBody = new CJson( "body" );
			CJson* pSourceArray = pBody->AddChild( "sources" );
			CFileMap::iterator it = m_mapFileBuffer.begin();
			for( ; it != m_mapFileBuffer.end(); ++it )
			{
				CJson* pSource = pSourceArray->AddChild( "" );
				std::string strpath = it->first.c_str();
				if (m_strCWD.length() > 0)
				{
					const char* pstr = strstr(strpath.c_str(), "/Lua/");
					if (pstr)
					{
						strpath = pstr + 4;
						strpath = m_strCWD + strpath;
					}
				}
				pSource->AddChild( "path", strpath.c_str() );
				pSource->AddChild( "name", GetFileNameFromPath( it->first.c_str() ) );
				if( it->first.c_str()[0] != '/' && it->first.find(':') == INVALID_32BITID )
				{
					pSource->AddChild( "presentationHint", "normal" );
					pSource->AddChild( "sourceReference", "1" );
				}
			}
			SendRespone( pBody, szSequence, true, szCommand );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "terminate" ) ||
			!StrCaseCmp( szCommand, "disconnect" ) )
		{
			uint8 nAttachType = m_eAttachType;
			TeminateRemote( szSequence );
			if( nAttachType != eAT_Launch )
				return true;
#ifdef _WIN32
			TerminateProcess( GetCurrentProcess(), 0 );
#else
			exit(0);
#endif
			return true;
		}
		else if( !StrCaseCmp( szCommand, "setBreakpoints" ) )
		{
			CJson* pArg = pCmd->GetChild( "arguments" );
			CJson* pSource = pArg->GetChild( "source" );
			const char* szPath = pSource->At<const char*>( "path" );
			if( !szPath )
				szPath = pSource->At<const char*>( "reference" );
			const char* szFileName = GetFileNameFromPath( szPath );
			CJson* pBreakpoints = pArg->GetChild( "breakpoints" );
			CBreakPoint bp( 0, szFileName, false, 0 );
			CBreakPointList::iterator it = m_setBreakPoint.lower_bound( bp );
			while( it != m_setBreakPoint.end() )
			{
				if (!strcmp((*it).GetModuleName(), szFileName))
					DelBreakPoint((it++)->GetBreakPointID());
				else
					it++;
			}

			CJson* pBody = new CJson( "body" );
			CJson* pBreakPointArray = pBody->AddChild( "breakpoints" );
			CJson* pChild = pBreakpoints->GetChild( (uint32)0 );
			while( pChild )
			{
				uint32 nLine = pChild->At<uint32>( "line" );
				uint32 nID = AddBreakPoint( szPath, nLine );
				pChild = pChild->GetNext();

				CJson* pBody = new CJson( "body" );
				CJson* pNew = pBody->AddChild( new CJson( "breakpoint" ) );
				pNew->AddChild( "id", nID );
				pNew->AddChild( "verified", "true" );
				pNew->AddChild( new CJson( *pSource ) );
				pNew->AddChild( "line", nLine );
				pBody->AddChild( "reason", "changed" );

				pBreakPointArray->AddChild( new CJson( *pNew ) );
				SendEvent( pBody, "breakpoint" );
			}
			SendRespone( pBody, szSequence, true, szCommand );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "threads" ) )
		{
			CJson* pBody = new CJson( "body" );
			CJson* pThreadArray = pBody->AddChild( "threads" );
			CJson* pSource = pThreadArray->AddChild( "" );
			pSource->AddChild( "name", "thread 1" );
			pSource->AddChild( "id", "1" );
			SendRespone( pBody, szSequence, true, szCommand );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "pause" ) )
		{
			if( !m_bEnterDebug )
				Stop();
			SendRespone( nullptr, szSequence, true, szCommand );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "setExceptionBreakpoints" ) )
		{
			m_bAllExceptionsBreak = false;
			m_bUncaughtExceptionsBreak = false;
			CJson* pArg = pCmd->GetChild( "arguments" );
			CJson* pFilter = pArg->GetChild( "filters" );
			CJson* pName = pFilter->GetChild( (uint32)0 );
			while( pName )
			{
				const char* szName = pName->As<const char*>();
				if( !StrCaseCmp( szName, "all" ) )
					m_bAllExceptionsBreak = true;
				else if( !StrCaseCmp( szName, "uncaught" ) )
					m_bUncaughtExceptionsBreak = true;
				pName = pName->GetNext();
			}

			SendRespone( nullptr, szSequence, true, szCommand );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "_clearall" ) )
		{
			while( !m_setBreakPoint.empty() )
				DelBreakPoint( m_setBreakPoint.begin()->GetBreakPointID() );
			return true;
		}

		if( !m_bEnterDebug )
		{
			SendRespone( nullptr, szSequence, true, szCommand );
			return true;
		}

		if( !StrCaseCmp( szCommand, "stackTrace" ) )
		{
			CJson* pArg = pCmd->GetChild( "arguments" );
			int32 nStartFrame = pArg->At<int32>( "startFrame", -1 );
			int32 nFrameCount = pArg->At<int32>( "levels", -1 );
			int32 nMaxFrameEnd = (int32)GetFrameCount() - 1;
			int32 nEndFrame = std::min( nStartFrame + nFrameCount, nMaxFrameEnd );
			nStartFrame = std::min( nStartFrame, nMaxFrameEnd );
			snprintf( szBuf, ELEM_COUNT(szBuf) - 1, "%d", nMaxFrameEnd + 1);

			CJson* pBody = new CJson( "body" );
			pBody->AddChild( "totalFrames", szBuf );
			CJson* pFrameArray = pBody->AddChild( "stackFrames" );
			for( int32 i = nStartFrame; i <= nEndFrame; i++ )
			{
				int32 nLine;
				const char* szSource = nullptr;
				const char* szFunction = nullptr;
				GetFrameInfo( i, &nLine, &szFunction, &szSource );
				CJson* pFrame = pFrameArray->AddChild( "" );
				pFrame->AddChild( "id", i );
				pFrame->AddChild( "name", szFunction ? szFunction : "<unknow>" );
				pFrame->AddChild( "line", nLine );
				pFrame->AddChild( "column", "0" );
				CJson* pSource = pFrame->AddChild( "source" );

				szSource = szSource ? szSource : "<now valid source>";
				std::string strpath = szSource;
				if (m_strCWD.length() > 0)
				{
					const char* pstr = strstr(strpath.c_str(), "/Lua/");
					if (pstr)
					{
						strpath = pstr + 4;
						strpath = m_strCWD + strpath;
					}
				}
				pSource->AddChild( "path", strpath);
				pSource->AddChild( "name", GetFileNameFromPath( szSource ) );
				if( szSource[0] != '/' && !::strchr( szSource, ':' ) )
				{
					pSource->AddChild( "presentationHint", "normal" );
					pSource->AddChild( "sourceReference", "1" );
				}
			}
			SendRespone( pBody, szSequence, true, szCommand );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "scopes" ) )
		{
			CJson* pArg = pCmd->GetChild( "arguments" );
			int32 nFrame = pArg->At<int32>( "frameId", 0 );
			CJson* pBody = new CJson( "body" );
			CJson* pScopes = pBody->AddChild( "scopes" );
			SValueInfo Value = GetVariable( GetScopeChainID( nFrame ) );
			uint32 nCount = Value.nNameValues;
			uint32* aryChild = (uint32*)alloca(sizeof(uint32) * nCount );
			GetChildrenID( Value.nID, false, 0, aryChild, nCount );
			for( uint32 i = 0; i < nCount; i++ )
			{
				SValueInfo Scope = GetVariable( aryChild[i] );
				CJson* pLocal = pScopes->AddChild( "" );
				pLocal->AddChild( "name", Scope.strName.c_str() );
				pLocal->AddChild( "variablesReference", Scope.nID );
				pLocal->AddChild( "namedVariables", Scope.nNameValues );
				pLocal->AddChild( "indexedVariables", Scope.nIndexValues );
				pLocal->AddChild( "expensive", false );
			}
			SendRespone( pBody, szSequence, true, szCommand );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "evaluate" ) )
		{
			CJson* pArg = pCmd->GetChild( "arguments" );
			int32 nFrame = pArg->At<int32>( "frameId", 0 );
			const char* szExpression = pArg->At<const char*>( "expression" );
			//CJson* pFormat = pArg->GetChild( "format" );
			//bool bHex = pFormat && pFormat->At<bool>( "hex" );
			uint32 nID = EvaluateExpression( nFrame, szExpression );
			SValueInfo Value = nID == INVALID_32BITID ? SValueInfo() : GetVariable( nID );
			CJson* pBody = new CJson( "body" );
			const char* szValue = Value.strValue.c_str();
			if( !Value.nNameValues && !Value.nIndexValues )
				Value.nID = 0;

			pBody->AddChild( "result", szValue )->ForceString( true );
			pBody->AddChild( "variablesReference", Value.nID );
			pBody->AddChild( "namedVariables", Value.nNameValues );
			pBody->AddChild( "indexedVariables", Value.nIndexValues );
			SendRespone( pBody, szSequence, true, szCommand );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "variables" ) )
		{
			CJson* pArg = pCmd->GetChild( "arguments" );
			uint32 nParentID = pArg->At<uint32>( "variablesReference" );
			bool bIndex = pArg->At<std::string>( "filter" ) == "indexed";
			uint32 nStart = pArg->At<uint32>( "start" );
			uint32 nCount = pArg->At<uint32>( "count" );
			if( nCount == 0 )
			{
				SValueInfo Info = GetVariable( nParentID );
				nCount = bIndex ? Info.nIndexValues : Info.nNameValues;
			}

			uint32* aryChild = (uint32*)alloca( sizeof(uint32)*nCount );
			uint32 nResult = GetChildrenID( nParentID, bIndex, nStart, aryChild, nCount );

			CJson* pBody = new CJson( "body" );
			CJson* pVariableArray = pBody->AddChild( "variables" );
			for( uint32 i = 0; i < std::max( nCount, nResult ); i++ )
			{
				SValueInfo Info = GetVariable( i < nResult ? aryChild[i] : 0 );
				if( !Info.nNameValues && !Info.nIndexValues )
					Info.nID = 0;
				CJson* pVariable = pVariableArray->AddChild( "" );
				pVariable->AddChild( "variablesReference", Info.nID );
				pVariable->AddChild( "name", Info.strName )->ForceString( true );
				pVariable->AddChild( "value", Info.strValue )->ForceString( true );
				pVariable->AddChild( "namedVariables", Info.nNameValues );
				pVariable->AddChild( "indexedVariables", Info.nIndexValues );
			}
			SendRespone( pBody, szSequence, true, szCommand );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "exceptionInfo" ) )
		{
			CJson* pBody = new CJson( "body" );
			snprintf( szBuf, ELEM_COUNT( szBuf ) - 1, "e%d", m_nExceptionID);
			pBody->AddChild( "exceptionId", szBuf );
			pBody->AddChild( "description", m_szException.c_str() );
			pBody->AddChild( "breakMode", "always" );
			CJson* pDetails = pBody->AddChild( "details" );
			pDetails->AddChild( "message", m_szException.c_str() );
			SendRespone( pBody, szSequence, true, szCommand );
			return true;
		}
		else if( !StrCaseCmp( szCommand, "continue" ) ) 
		{
			Continue();
			CJson* pBody = new CJson( "body" );
			pBody->AddChild( "allThreadsContinued", "true" );
			SendRespone( pBody, szSequence, true, szCommand );
			m_bExpectStep = false;
			return false;
		}
		else if( !StrCaseCmp( szCommand, "stepIn" ) )
		{
			StepIn();
			SendRespone( nullptr, szSequence, true, szCommand );
			m_bExpectStep = true;
			return false;
		}
		else if( !StrCaseCmp( szCommand, "stepOut" ) )
		{
			StepOut();
			SendRespone( nullptr, szSequence, true, szCommand );
			m_bExpectStep = true;
			return false;
		}
		else if( !StrCaseCmp( szCommand, "next" ) )
		{
			StepNext();
			SendRespone( nullptr, szSequence, true, szCommand );
			m_bExpectStep = true;
			return false;
		}

		SendRespone( nullptr, szSequence, true, szCommand );
		return true;
	}

	//=================================================================
	// ConsoleDebug
	//=================================================================
	const char* CDebugBase::ReadWord( bool bNewLine )
	{
		if( bNewLine )
			m_pBuf = nullptr;

		if( !m_pBuf )
		{
			if( !bNewLine )
				return nullptr;

			m_pHandler->Output( "(gdb) ", -1 );
			char szBuf[ sizeof(m_szBuffer) ];
			if( m_pHandler->Input( szBuf, sizeof(szBuf) ) && szBuf[0] != '\n' )
				memcpy( m_szBuffer, szBuf, sizeof(szBuf) );
			m_pBuf = m_szBuffer;
		}

		while( *m_pBuf == ' ' || *m_pBuf == '\t' )
			m_pBuf++;

		char* pCur = m_pBuf;
		while( *m_pBuf != ' ' && *m_pBuf != '\t' && *m_pBuf != '\n' && *m_pBuf != 0 )
			m_pBuf++;                

		while( *m_pBuf == ' ' || *m_pBuf == '\t' )
		{
			*m_pBuf = 0;
			m_pBuf++;
		}

		if( *m_pBuf == 0 || *m_pBuf == '\n' )
		{
			*m_pBuf = 0;
			m_pBuf = nullptr;
		}

		return pCur;
	}

	bool CDebugBase::PrintLine( int32 nFrame, 
		const char* szSource, int32 nLine, bool bIsCurLine )
	{
		if( nLine <= 0 || szSource == nullptr )
		{
			m_pHandler->Output( "Source not available.\n", -1 );
			return false;
		}

		CBreakPoint bp( 0, GetFileNameFromPath( szSource ), true, nLine );
		bool bIsBreakLine = m_setBreakPoint.find( bp ) != m_setBreakPoint.end();
		const char* szLine = ReadFileLine( szSource, nLine );
		if( !szLine )
			return false;
		char szLineNumber[32];
		sprintf( szLineNumber, "%d %s%s\t", nLine,
			( bIsBreakLine ? "B" : " " ), ( bIsCurLine ? ">>" : "" ) );
		m_pHandler->Output( szLineNumber, -1 );
		m_pHandler->Output( szLine, -1 );
		m_pHandler->Output( "\n", -1 );
		return true;
	}

	void CDebugBase::PrintFrame( int32 nFrame, 
		const char* szFun, const char* szSource, int32 nLine )
	{
		char szFrameInfo[1024];
		sprintf( szFrameInfo, "#%d  %s ", nFrame, ( szFun ? szFun : "(unknown)" ) );
		m_pHandler->Output( szFrameInfo, -1 );
		m_pHandler->Output( szSource ? szSource : "(no source)", -1 );
		sprintf( szFrameInfo, ":%d\n", nLine );
		m_pHandler->Output( szFrameInfo, -1 );
	}

	void CDebugBase::ConsoleDebug( SException* pException )
	{
		const char* szBuf;
		const char* szCurFunction = nullptr;
		const char* szCurSource = nullptr;
		if( pException )
		{
			m_pHandler->Output( "Exception : ", -1 );
			m_pHandler->Output( pException->szException, -1 );
			m_pHandler->Output( "\n", -1 );
		}

		GetFrameInfo( m_nCurFrame, &m_nCurLine, &szCurFunction, &szCurSource );
		if( m_bPrintFrame )
			PrintFrame( m_nCurFrame, szCurFunction, szCurSource, m_nCurLine );
		PrintLine( m_nCurFrame, szCurSource, m_nCurLine, true );
		m_nShowLine = m_nCurLine - LINE_COUNT_ON_SHOW/2;
		m_bPrintFrame = true; 
		m_bLoopOnPause = true;

		while( m_bLoopOnPause )
		{
			szBuf = ReadWord( true );
			if( !szBuf )
				return;

			if( !strcmp( szBuf, "help") )
			{
				const char* szHelp =
					"backtrace/bt                      back trace stack\n"
					"break/b file:line                 set break point on line in file\n"
					"continue/c                        continue\n"
					"del n                             delete break point\n"
					"disable                           disable all break point\n"
					"enable                            enable all break point\n"
					"frame/f n                         locate the stack frame\n"
					"help                              print this infomation\n"
					"info                              list break point\n"
					"list/l                            list current source\n"
					"load/lo file                      load the file\n"
					"next/n                            step next\n"
					"out/o                             step out\n"
					"print/p v                         print valur of expression\n"
					"step/s                            step in\n"
					;
				m_pHandler->Output( szHelp, -1 );
			}
			else if( !strcmp( szBuf, "continue" ) || !strcmp( szBuf, "c" ) )
			{
				Continue();
				m_bLoopOnPause = false;
			}
			else if( !strcmp( szBuf, "del" ) )
			{
				szBuf = ReadWord();        
				DelBreakPoint( szBuf && IsNumber( *szBuf ) ? atoi( szBuf ) : INVALID_32BITID );
			}
			else if( !strcmp( szBuf, "break" ) || !strcmp( szBuf, "b" ) )
			{
				szBuf = ReadWord();
				AddBreakPoint( szBuf );
			}
			else if( !strcmp( szBuf, "info" ) )
			{
				PrintBreakInfo();
			}
			else if( !strcmp( szBuf, "load" ) || !strcmp( szBuf, "lo") )
			{
				szBuf = ReadWord();
				if( !szBuf )
					GetFrameInfo( m_nCurFrame, nullptr, nullptr, &szBuf );
				m_pHandler->RunFile( szBuf );
			}
			else if( !strcmp( szBuf, "next" ) || !strcmp( szBuf, "n" ) )
			{
				StepNext();
				m_bPrintFrame = false;
				m_bLoopOnPause = false;
			}
			else if( !strcmp( szBuf, "step") || !strcmp( szBuf, "s") )
			{
				StepIn();
				m_bLoopOnPause = false;
			}
			else if( !strcmp( szBuf, "out") || !strcmp( szBuf, "o") )
			{
				StepOut();
				m_bLoopOnPause = false;
			}
			else if( !strcmp( szBuf, "list") || !strcmp( szBuf, "l") )
			{
				szBuf = ReadWord();
				uint32 nLine = szBuf && IsNumber( *szBuf ) ? atoi( szBuf ) : 0;
				if( !nLine )
					nLine = LINE_COUNT_ON_SHOW;
				ShowFileLines( nLine );
			}
			else if( !strcmp( szBuf, "backtrace" ) || !strcmp( szBuf, "bt" ) )
			{
				BTrace( INVALID_32BITID );
			}
			else if( !strcmp( szBuf, "frame") || !strcmp( szBuf, "f" ) )
			{
				szBuf = ReadWord();
				int32 nFrame = -1;
				if( !szBuf || !IsNumber( *szBuf ) || 
					( nFrame = SwitchFrame( atoi( szBuf ) ) ) < 0 )
				{
					m_pHandler->Output( "Invalid stack index.\n", -1 );
					continue;
				}
				m_nCurFrame = nFrame;
				GetFrameInfo( nFrame, &m_nCurLine, &szCurFunction, &szCurSource );
				PrintFrame( m_nCurFrame, szCurFunction, szCurSource, m_nCurLine );
				PrintLine( m_nCurFrame, szCurSource, m_nCurLine, true );
				m_nShowLine = m_nCurLine - LINE_COUNT_ON_SHOW/2;
			}
			else if( !strcmp( szBuf, "print") || !strcmp( szBuf, "p" ) )
			{
				szBuf = ReadWord();
				if( ( !szBuf || !szBuf[0] ) && m_strLastVarName.empty() )
				{
					m_pHandler->Output( "Variable not specified.\n", -1 );
					continue;
				}
				szBuf = szBuf && szBuf[0] ? szBuf : m_strLastVarName.c_str();
				m_strLastVarName = szBuf;
				SValueInfo Info = GetVariable( EvaluateExpression( m_nCurFrame, szBuf ) );
				m_pHandler->Output( Info.strValue.c_str(), -1 );
				m_pHandler->Output( "\n", -1 );
			}
			else if( !strcmp( szBuf, "enable" ) )
			{
				szBuf = ReadWord();
				szBuf = szBuf && szBuf[0] ? szBuf : "a";
				if( szBuf[0] == 'a' )
					m_bAllExceptionsBreak = true;
				else if( szBuf[0] == 'u' )
					m_bUncaughtExceptionsBreak = true;
			}
			else if( !strcmp( szBuf, "disable" ) )
			{
				szBuf = ReadWord();
				szBuf = szBuf && szBuf[0] ? szBuf : "a";
				if( szBuf[0] == 'a' )
					m_bAllExceptionsBreak = false;
				else if( szBuf[0] == 'u' )
					m_bUncaughtExceptionsBreak = false;
			}
			else if( strlen ( szBuf ) )
			{
				m_pHandler->Output( "Invalid command!\n", -1 );
			}
		}
	}

	void CDebugBase::AddBreakPoint( const char* szBuf )
	{
		int32 nBreakLine = m_nCurLine;
		const char* szSource = nullptr;
		char* pColon = nullptr;

		if( !szBuf )
		{
			nBreakLine = m_nCurLine;
			GetFrameInfo( m_nCurFrame, nullptr, nullptr, &szSource );
		}
		else if( IsNumber( *szBuf ) )
		{
			GetFrameInfo( m_nCurFrame, nullptr, nullptr, &szSource );
			nBreakLine = atoi( szBuf );
		}
		else if( ( pColon = (char*)strchr( szBuf, ':' ) ) == nullptr )
		{
			m_pHandler->Output( "Filename and line number must be provided.\n", -1 );
			return;
		}
		else if( !IsNumber( *( pColon + 1 ) ) )
		{
			m_pHandler->Output( "Line number must be provided.\n", -1 );
			return;
		}
		else
		{
			*pColon = 0;
			szSource = szBuf;
			nBreakLine = atoi( pColon + 1 );
		}

		if( AddBreakPoint( szSource, nBreakLine ) != INVALID_32BITID )
			return;
		m_pHandler->Output( "Break location not found.\n", -1 );
	}

	void CDebugBase::PrintBreakInfo()
	{
		CBreakPointList::iterator it = m_setBreakPoint.begin();
		while( it != m_setBreakPoint.end() )
		{
			const CBreakPoint& BreakPoint = *it++;
			char szBreakInfo[1024];
			sprintf( szBreakInfo, "%d\t%s:%d\n", BreakPoint.GetBreakPointID(),
				BreakPoint.GetModuleName(), BreakPoint.GetLineNum() );
			m_pHandler->Output( szBreakInfo, -1 );
		}
	}

	void CDebugBase::ShowFileLines( int32 nLineCount )
	{
		if( m_nShowLine < 1 )
			m_nShowLine = 1;

		int32 nLine = 0;
		const char* szCurSource = nullptr;
		if( !GetFrameInfo( m_nCurFrame, &nLine, nullptr, &szCurSource ) )
			return;

		int32 nShowEndLine = m_nShowLine + LINE_COUNT_ON_SHOW;
		while( m_nShowLine < nShowEndLine &&
			PrintLine( m_nCurFrame, szCurSource, m_nShowLine, m_nShowLine == nLine ) )
			m_nShowLine++;
	}

	//===============================================================
	// 导出的接口
	//===============================================================
	uint32 CDebugBase::AddBreakPoint( const char* szFileName, int32 nLine )
	{
		szFileName = GetFileNameFromPath( szFileName );
		uint32 nID = GenBreakPointID( szFileName, nLine );
		CBreakPoint BreakPoint = CBreakPoint( nID, szFileName, false, nLine );
		m_setBreakPoint.insert( BreakPoint );
		return BreakPoint.GetBreakPointID();
	}

	void CDebugBase::DelBreakPoint( uint32 nBreakPointID )
	{
		CBreakPointList::iterator it = m_setBreakPoint.begin();
		for( ; it != m_setBreakPoint.end(); ++it )
		{
			const CBreakPoint& BreakPoint = *it;
			if( nBreakPointID != BreakPoint.GetBreakPointID() )
				continue;
			m_setBreakPoint.erase( it );
			return;
		}
	}

	const CBreakPoint* CDebugBase::GetBreakPoint( const char* szSource, int32 nLine )
	{
		if( !HaveBreakPoint() )
			return 0;
		CBreakPoint bp( 0, szSource, true, nLine );
		CBreakPointList::iterator it = m_setBreakPoint.find( bp );
        return it == m_setBreakPoint.end() ? nullptr : &*it;
	}

	bool CDebugBase::HasLoadFile(const char* szFile)
	{
		if (m_mapFileBuffer.find(szFile) != m_mapFileBuffer.end())
		{
			return true;
		}
		return false;
	}
}
