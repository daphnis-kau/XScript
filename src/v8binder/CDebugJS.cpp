#include "common/Http.h"
#include "common/CodeCvs.h"
#include "common/TStrStream.h"
#include "CDebugJS.h"
#include "CScriptJS.h"
#include "V8Context.h"
#include <memory>

#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <io.h>
#include <stdio.h>
#include <signal.h>
#define NE_EWOULDBLOCK	WSAEWOULDBLOCK
#define NE_EINPROGRESS	WSAEINPROGRESS
#define LastError		WSAGetLastError()
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#else
#include <alloca.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cerrno>
#include <netdb.h>
#define INVALID_SOCKET		-1
#define closesocket			close
#define ioctlsocket			ioctl
typedef int32				SOCKET;
typedef struct linger		LINGER;
#define SOCKET_ERROR		-1
#define SD_SEND				SHUT_WR
#define LastError			errno
#define NE_EWOULDBLOCK		EWOULDBLOCK	
#define NE_EINPROGRESS		EINPROGRESS	
#endif

namespace Gamma
{
	//-----------------------------------------------------
	// CDebugJS
	//-----------------------------------------------------
	CDebugJS::CDebugJS(CScriptBase* pBase, uint16 nDebugPort, bool bV8Protocal)
		: CDebugBase(pBase, nDebugPort)
		, m_nDebugPort(nDebugPort)
		, m_bV8Protocal( bV8Protocal )
		, m_nMessageID( 1 )
	{
		if (!nDebugPort)
			return;
		CheckSession();
	}

	CDebugJS::~CDebugJS(void)
	{
	}

	void CDebugJS::CheckSession()
	{
		if (m_Session.get())
			return;

		CScriptJS* pScript = (CScriptJS*)m_pBase;
		SV8Context& Context = pScript->GetV8Context();
		v8::Isolate* isolate = Context.m_pIsolate;

		// create a v8 inspector instance.
		m_Inspector = v8_inspector::V8Inspector::create(isolate, this);

		// create a v8 channel. 
		// ChannelImpl : public v8_inspector::V8Inspector::Channel
		const char* szView = "{\"Debugger\":{\"debuggerEnabled\":true}}";
		v8_inspector::StringView view((const uint8_t*)szView, strlen(szView));

		// Create a debugging session by connecting the V8Inspector
		// instance to the channel
		m_Session = m_Inspector->connect(1, this, view);

		const char* szName = "GammaJavaScriptDebugger";
		v8_inspector::StringView ContextName((const uint8_t*)szName, strlen(szName));
		// make sure you register Context objects in the V8Inspector.
		// ctx_name will be shown in CDT/console. Call this for each context 
		// your app creates. Normally just one btw.
		v8_inspector::V8ContextInfo Info(isolate->GetCurrentContext(), 1, ContextName);
		m_Inspector->contextCreated(Info);
	}

	//=====================================================================
	// v8_inspector协议
	//=====================================================================
	bool CDebugJS::CheckRemoteSocket(char(&szBuffer)[2048], int32 nCurSize)
	{
		//GammaLog << szBuffer << endl;
		// 确定是否V8协议（websocket)
		if (!m_bV8Protocal)
			return CDebugBase::CheckRemoteSocket(szBuffer, nCurSize);

		if (nCurSize < 0 || memcmp(szBuffer, "GET /", 5))
			return false;

		CHttpRequestState State;
		EHttpReadState eState = State.CheckHttpBuffer(szBuffer, nCurSize);
		if (eState != eHRS_Ok)
			return false;
		std::string strPage = std::string(State.GetPageStart(), State.GetPageLength());
		if (strPage == "json/version")
		{
			const char* szContent =
				"HTTP/1.0 200 OK\r\n"
				"Content-Type: application/json; charset=UTF-8\r\n"
				"Cache-Control: no-cache\r\n"
				"Content-Length: 53\r\n\r\n"
				"{\"Browser\": \"gamma/v0.1\",\"Protocol-Version\" : \"1.1\" }";
			static uint32 nLen = (uint32)strlen(szContent);
			send(m_nRemoteConnecter, szContent, nLen, 0);
			return false;
		}

		if (strPage == "json" || strPage == "json/list" )
		{
			char szParam[256];
			const char* szKey = "9F2FF3E5-BE28-4C06-BADC-83688C4AFFB7";
			gammasstream( szParam ) << "experiments=true&v8only=true&ws=127.0.0.1:"
				<< m_nDebugPort << "/" << szKey;
			char szContent[1024];
			gammasstream(szContent) <<
				"[{"
					"\"description\": \"gamma instance\","
					"\"devtoolsFrontendUrl\": "
					"\"chrome-devtools://devtools/bundled/"
						"inspector.html?" << szParam << "\","
					"\"id\": \"" << szKey << "\","
					"\"title\": \"gamma\","
					"\"type\": \"node\","
					"\"webSocketDebuggerUrl\": "
						"\"ws://127.0.0.1:" << m_nDebugPort << "/" << szKey << "\""
				"}]";

			uint32 nContentLen = (uint32)strlen(szContent);
			char szTotal[2048];
			gammasstream(szTotal) <<
				"HTTP/1.0 200 OK\r\n"
				"Content-Type: application/json; charset=UTF-8\r\n"
				"Cache-Control: no-cache\r\n"
				"Content-Length: " << nContentLen << "\r\n\r\n"
				<< szContent;
			send(m_nRemoteConnecter, szTotal, (int32)strlen(szTotal), 0);
			return false;
		}

		std::string strBuffer;
		strBuffer.append( szBuffer, nCurSize );
		while( true )
		{
			const char* szKey;
			uint32 nKeyLen;
			uint32 nReadCount = WebSocketShakeHandCheck(
				strBuffer.c_str(), strBuffer.size(), true, szKey, nKeyLen );
			if( nReadCount == INVALID_32BITID )
				return false;
			if( !nReadCount )
			{
				nCurSize = (int32)recv( m_nRemoteConnecter, szBuffer, 2048, 0 );
				if( nCurSize < 0 )
					return false;
				strBuffer.append( szBuffer, nCurSize );
				continue;
			}
			
			char szSendBuffer[256];
			uint32 nSendSize = MakeWebSocketServerShakeHandResponese( 
				szSendBuffer, ELEM_COUNT(szSendBuffer), szKey, nKeyLen );
			int32 nSend = send( m_nRemoteConnecter, szSendBuffer, nSendSize, 0 );
			if( (uint32)nSend != nSendSize )
				return false;
			strBuffer.erase( 0, nReadCount );
			break;
		}

		std::string strCmd;
		while( m_eAttachType )
		{
			while( strBuffer.size() >= sizeof(SWebSocketProtocal) )
			{
				SWebSocketProtocal Protocal = *(SWebSocketProtocal*)&strBuffer[0];
				char* pExtraBuffer = &strBuffer[0] + sizeof(SWebSocketProtocal);
				uint64 nExtraSize = strBuffer.size() - sizeof(SWebSocketProtocal);
				uint64 nDecode = DecodeWebSocketProtocol( &Protocal, pExtraBuffer, nExtraSize );
				if( nDecode == INVALID_64BITID )
					break;
				strCmd.append( pExtraBuffer, (uint32)nExtraSize );
				strBuffer.erase( 0, (uint32)nDecode + sizeof(SWebSocketProtocal) );
				if( !Protocal.m_bFinished )
					continue;
				if( Protocal.m_nId == eWS_Text ||
					Protocal.m_nId == eWS_Binary ||
					Protocal.m_nId == eWS_Ping )
				{
					CDebugCmd* pCmd = new CDebugCmd;
					const char* name = Protocal.m_nId == eWS_Ping ? "p" : "d";
					*(CJson*)pCmd = CJson( name, strCmd.c_str() );
					OnNetData( pCmd );
				}
				strCmd.clear();
			}

			nCurSize = (int32)recv( m_nRemoteConnecter, szBuffer, 2048, 0 );
			if (nCurSize >= 0)
			{
				strBuffer.append(szBuffer, nCurSize);
				continue;
			}

			int32 nError = LastError;
			if (nError == NE_EWOULDBLOCK)
				continue;
			if( nError == NE_EINPROGRESS)
				continue;
			break;
		}
		return false;
	}

	bool CDebugJS::ProcessCommand( CDebugCmd* pCmd )
	{
		if( !m_bV8Protocal )
			return CDebugBase::ProcessCommand( pCmd );

		if (!pCmd->GetName())
			return true;

		const char* szContent = pCmd->GetContent();
		uint32 nSize = pCmd->GetContentLen();
		if( *pCmd->GetName() == 'p' )
		{
			SendWebSocketData( eWS_Pong, szContent, nSize );
			return true;
		}

		//GammaLog << "************************begin*************************" << endl;
		//GammaLog << szContent << endl;
		//GammaLog << "*************************end**************************" << endl;
		v8_inspector::StringView view( (const uint8_t*)szContent, nSize );
		m_Session->dispatchProtocolMessage(view);
		return true;
	}

	void CDebugJS::SendWebSocketData( uint8 nId, const char* pData, uint32 nSize )
	{
		SWebSocketProtocal MsgHead = {0};
		MsgHead.m_nId = nId;
		MsgHead.m_bMask = false;
		EncodeWebSocketProtocol( MsgHead, (char*)pData, nSize );
		send( m_nRemoteConnecter, (const char*)&MsgHead, sizeof(MsgHead), 0 );

		union { uint64 u64; uint8 u8[sizeof(uint64)]; } Size;
		Size.u64 = (uint32)nSize;
		if( Size.u64 >= 65536 )
		{
			uint8 aryBigEnd64[] = { Size.u8[7], Size.u8[6], Size.u8[5],
				Size.u8[4], Size.u8[3], Size.u8[2], Size.u8[1], Size.u8[0] };
			send( m_nRemoteConnecter, (const char*)aryBigEnd64, sizeof(aryBigEnd64), 0 );
		}
		else if (Size.u64 >= 126)
		{
			uint8 aryBigEnd16[] = { Size.u8[1], Size.u8[0] };
			send( m_nRemoteConnecter, (const char*)aryBigEnd16, sizeof(aryBigEnd16), 0 );
		}
		
		send( m_nRemoteConnecter, (char*)pData, nSize, 0 );
	}

	void CDebugJS::runMessageLoopOnPause(int contextGroupId)
	{
		if (!m_bV8Protocal)
			return CDebugBase::Debug();

		m_bLoopOnPause = true;
		while (m_bLoopOnPause)
		{
			GammaSleep(10);
			CheckRemoteCmd();
		}
	}

	void CDebugJS::quitMessageLoopOnPause()
	{
		m_bLoopOnPause = false;
	}

	void CDebugJS::runIfWaitingForDebugger(int contextGroupId)
	{
		CheckRemoteCmd();
	}

	v8::Local<v8::Context> CDebugJS::ensureDefaultContextInGroup(int context_group_id)
	{
		CScriptJS* pScript = (CScriptJS*)GetScriptBase();
		SV8Context& Context = pScript->GetV8Context();
		return Context.m_pIsolate->GetCurrentContext();
	}

	double CDebugJS::currentTimeMS()
	{
		CScriptJS* pScript = (CScriptJS*)GetScriptBase();
		SV8Context& Context = pScript->GetV8Context();
		return Context.m_platform->CurrentClockTimeMillis();
	}

	void CDebugJS::sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message)
	{
		const v8_inspector::StringView& buffer = message->string();
		const char* szBuffer = (const char*)buffer.characters8();
		uint32 nSize = (uint32)buffer.length();
		m_strUtf8Buffer.assign( szBuffer, nSize );
		if( !buffer.is8Bit() )
		{
			m_strUtf8Buffer.resize( nSize * 6 );
			nSize = Ucs2ToUtf8( &m_strUtf8Buffer[0], nSize*6, buffer.characters16(), nSize );
			szBuffer = m_strUtf8Buffer.c_str();
		}

		if (m_nRemoteConnecter == INVALID_SOCKET || !m_bV8Protocal)
			return;
		SendWebSocketData( eWS_Text, m_strUtf8Buffer.c_str(), nSize );
	}

	void CDebugJS::sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message)
	{
		const v8_inspector::StringView& buffer = message->string();
		const char* szBuffer = (const char*)buffer.characters8();
		uint32 nSize = (uint32)buffer.length();
		if (!buffer.is8Bit())
		{
			m_strUtf8Buffer.resize(nSize * 6);
			nSize = Ucs2ToUtf8(&m_strUtf8Buffer[0], nSize*6, buffer.characters16(), nSize);
			szBuffer = m_strUtf8Buffer.c_str();
		}

		if (m_nRemoteConnecter != INVALID_SOCKET && m_bV8Protocal)
			return SendWebSocketData(eWS_Text, szBuffer, nSize);

		CJson Json;
		Json.Load(szBuffer, nSize);
		std::string strMethod = Json.At<std::string>("method");
		if (strMethod == "Debugger.paused")
		{
			m_aryFrame.clear();
			CJson* params = Json.GetChild("params");
			if (!params)
				return;
			CJson* pCallFrames = params->GetChild("callFrames");
			if( !pCallFrames || !pCallFrames->GetChildCount() )
				return;

			auto ReadLocation = [&](CJson* pLocation)->SLocation
			{
				SLocation Location;
				if( !pLocation )
					return Location;
				Location.nScriptId = pLocation->At<int32>( "scriptId" );
				Location.nLineNum = pLocation->At<int32>( "lineNumber" );
				Location.nColumnNum = pLocation->At<int32>( "columnNumber" );
				return Location;
			};

			auto ReadObject = [&]( CJson* pObject )->SObjectInfo
			{
				SObjectInfo ObjectInfo;
				if( !pObject )
					return ObjectInfo;
				ObjectInfo.strType = pObject->At<std::string>( "type" );
				ObjectInfo.strClassName = pObject->At<std::string>( "className" );
				ObjectInfo.strDesc = pObject->At<std::string>( "description" );
				ObjectInfo.strID = pObject->At<std::string>( "objectId" );
				return ObjectInfo;
			};

			auto ReadScope = [&]( CJson* pScope )->SScopeInfo
			{
				SScopeInfo VariableInfo;
				if( !pScope )
					return VariableInfo;
				VariableInfo.strType = pScope->At<std::string>( "type" );
				VariableInfo.ObjectInfo = ReadObject( pScope->GetChild( "object" ) );
				VariableInfo.StartLocation = ReadLocation( pScope->GetChild( "startLocation" ) );
				VariableInfo.EndLocation = ReadLocation( pScope->GetChild( "endLocation" ) );
				return VariableInfo;
			};

			int32 nFrameIndex = 0;
			m_aryFrame.resize( pCallFrames->GetChildCount() );
			for( CJson* pFrame = pCallFrames->GetChild( (uint32)0 );
				pFrame; pFrame = pFrame->GetNext(), nFrameIndex++ )
			{
				SFrameInfo& Frame = m_aryFrame[nFrameIndex];
				Frame.strCallFrameID = pFrame->At<std::string>( "callFrameId" );
				Frame.strFunctionName = pFrame->At<std::string>( "functionName" );
				Frame.strScriptUrl = pFrame->At<std::string>( "url" );
				Frame.FunctionLocation = ReadLocation( pFrame->GetChild( "functionLocation" ) );
				Frame.PauseLocation = ReadLocation( pFrame->GetChild( "location" ) );
				Frame.ThisInfo = ReadObject( pFrame->GetChild( "this" ) );
				CJson* pScopeChain = pFrame->GetChild( "scopeChain" );
				if( !pScopeChain || !pScopeChain->GetChildCount() )
					continue;
				int32 nScopeIndex = 0;
				Frame.vecScope.resize( pScopeChain->GetChildCount() );
				for( CJson* pScope = pScopeChain->GetChild( (uint32)0 );
					pScope; pScope = pScope->GetNext(), nScopeIndex++ )
					Frame.vecScope[nScopeIndex] = ReadScope( pScope );
			}
		}
	}

	void CDebugJS::flushProtocolNotifications()
	{
	}

	//=====================================================================
	// 默认 协议
	//=====================================================================
	uint32 CDebugJS::GenBreakPointID(const char* szFileName, int32 nLine)
	{
		auto itFile = m_mapFileBuffer.begin();
		while( itFile != m_mapFileBuffer.end() &&
			itFile->first.find( szFileName ) == std::string::npos )
			++itFile;
		if( itFile == m_mapFileBuffer.end() )
			return 0;
		char szCommand[4096];
		uint32 nMessageID = m_nMessageID++;
		gammasstream( szCommand )
			<< "{\"id\":" << nMessageID << ","
			<< "\"method\":\"Debugger.setBreakpointByUrl\","
			<< "\"params\":{\"url\":\"" << itFile->first.c_str() << "\","
			<< "\"lineNumber\":" << ( nLine ? nLine - 1 : 0 ) << "}}";
		v8_inspector::StringView view( (const uint8_t*)szCommand, strlen( szCommand ) );
		m_strUtf8Buffer.clear();
		m_Session->dispatchProtocolMessage( view );
		if( m_strUtf8Buffer.empty() )
			return 0;
		CJson BreakPoint;
		BreakPoint.Load( m_strUtf8Buffer.c_str(), strlen( m_strUtf8Buffer.c_str() ) );
		CJson* pResult = BreakPoint.GetChild( "result" );
		if( !pResult )
			return 0;
		std::string strID = pResult->At<std::string>( "breakpointId" );
		auto itBreakPoint = m_mapBreakPoint.find( strID );
		if( itBreakPoint != m_mapBreakPoint.end() )
			return itBreakPoint->second;
		m_mapBreakPoint[strID] = nMessageID;
		return nMessageID;
	}

	void CDebugJS::DelBreakPoint( uint32 nBreakPointID )
	{
		for(auto it = m_mapBreakPoint.begin(); it != m_mapBreakPoint.end(); ++it)
		{
			if(it->second != nBreakPointID)
				continue;
			char szCommand[4096];
			gammasstream( szCommand )
				<< "{\"id\":" << m_nMessageID++ << ","
				<< "\"method\":\"Debugger.removeBreakpoint\","
				<< "\"params\":{\"breakpointId\":\"" << it->first << "\"}}";
			v8_inspector::StringView view( (const uint8_t*)szCommand, strlen( szCommand ) );
			m_mapBreakPoint.erase(it);
			m_Session->dispatchProtocolMessage( view );
			return;
		}
	}

	uint32 CDebugJS::GetFrameCount()
	{
		return (uint32)m_aryFrame.size();
	}

	bool CDebugJS::GetFrameInfo(int32 nFrame, int32* nLine, const char** szFunction, const char** szSource)
	{
		if( nFrame >= (int32)GetFrameCount() )
			return false;
		SFrameInfo& CurFrame = m_aryFrame[nFrame];
		if( nLine )
			*nLine = CurFrame.PauseLocation.nLineNum + 1;
		if( szFunction )
			*szFunction = CurFrame.strFunctionName.c_str();
		if( szSource )
			*szSource = CurFrame.strScriptUrl.c_str();
		return true;
	}

	int32 CDebugJS::SwitchFrame(int32 nCurFrame)
	{
		int32 nFrameCount = GetFrameCount();
		if( nCurFrame >= nFrameCount )
			nCurFrame = nFrameCount - 1;
		return nCurFrame;
	}

	uint32 CDebugJS::GetVariableID(int32 nCurFrame, const char* szName)
	{
		return 0;
	}

	uint32 CDebugJS::GetChildrenID(uint32 nParentID, bool bIndex, uint32 nStart, uint32* aryChild, uint32 nCount)
	{
		return 0;
	}

	Gamma::SValueInfo CDebugJS::GetVariable(uint32 nID)
	{
		return Gamma::SValueInfo();
	}

	void CDebugJS::Stop()
	{
		CheckSession();
		v8_inspector::StringView breakReason((const uint8_t*)"break", 5);
		v8_inspector::StringView breakDetails((const uint8_t*)"{}", 2);
		m_Session->breakProgram(breakReason, breakDetails);
	}

	void CDebugJS::Continue()
	{
		m_Session->resume();
	}

	void CDebugJS::StepIn()
	{
		char szCommand[256];
		gammasstream(szCommand) << "{\"id\":" << m_nMessageID++ 
			<< ",\"method\":\"Debugger.stepInto\"}";
		v8_inspector::StringView view((const uint8_t*)szCommand, strlen(szCommand));
		m_Session->dispatchProtocolMessage(view);
	}

	void CDebugJS::StepNext()
	{
		char szCommand[256];
		gammasstream(szCommand) << "{\"id\":" << m_nMessageID++
			<< ",\"method\":\"Debugger.stepOver\"}";
		v8_inspector::StringView view((const uint8_t*)szCommand, strlen(szCommand));
		m_Session->dispatchProtocolMessage(view);
	}

	void CDebugJS::StepOut()
	{
		char szCommand[256];
		gammasstream(szCommand) << "{\"id\":" << m_nMessageID++
			<< ",\"method\":\"Debugger.stepOut\"}";
		v8_inspector::StringView view((const uint8_t*)szCommand, strlen(szCommand));
		m_Session->dispatchProtocolMessage(view);
	}
}
