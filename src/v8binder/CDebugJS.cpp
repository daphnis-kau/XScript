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

#define DEBUG_LOG( msg )
//#define DEBUG_LOG( msg )	m_pBase->Output( msg, -1 )

namespace Gamma
{
	std::string ReadIDFromJson( CJson* pJson )
	{
		if (!pJson)
			return "";
		std::string strID = pJson->As<std::string>();
		for (int32 i = (int32)strID.size() - 1; i >= 0; --i)
		{
			if (strID[i] != '\"')
				continue;
			strID.insert(i, 1, '\\');
		}
		return strID;
	}

	void CDebugJS::SLocation::ReadFromJson(CJson* pJson)
	{
		if (!pJson)
			return;
		nScriptId = pJson->At<int32>("scriptId");
		nLineNum = pJson->At<int32>("lineNumber");
		nColumnNum = pJson->At<int32>("columnNumber");
	};

	void CDebugJS::SObjectInfo::ReadFromJson(CJson* pJson)
	{
		if (!pJson)
			return;
		strType = pJson->At<std::string>("type");
		strClassName = pJson->At<std::string>("className");
		strDesc = pJson->At<std::string>("description");
		strValue = pJson->At<std::string>("value");
		strID = ReadIDFromJson( pJson->GetChild("objectId") );
	};

	void CDebugJS::SScopeInfo::ReadFromJson(CJson* pJson)
	{
		if (!pJson)
			return;
		strType = pJson->At<std::string>("type");
		ObjectInfo = new SObjectInfo;
		ObjectInfo->ReadFromJson(pJson->GetChild("object"));
		StartLocation.ReadFromJson(pJson->GetChild("startLocation"));
		EndLocation.ReadFromJson(pJson->GetChild("endLocation"));
	};

	//-----------------------------------------------------
	// CDebugJS
	//-----------------------------------------------------
	CDebugJS::CDebugJS(CScriptBase* pBase, uint16 nDebugPort)
		: CDebugBase(pBase, nDebugPort)
		, m_nDebugPort(nDebugPort)
		, m_eProtocol( ePT_Unknow )
		, m_nMessageID( 1 )
	{
		CScriptJS* pScript = (CScriptJS*)m_pBase;
		SV8Context& Context = pScript->GetV8Context();
		v8::Isolate* isolate = Context.m_pIsolate;
		v8::HandleScope handle_scope(isolate);
		v8::TryCatch try_catch(isolate);

		// Enter the context for compiling and running the hello world script.
		v8::Local<v8::Context> context = Context.m_Context.Get(isolate);
		v8::Context::Scope context_scope(context);

		// create a v8 inspector instance.
		m_Inspector = v8_inspector::V8Inspector::create(isolate, this);

		// create a v8 channel. 
		// ChannelImpl : public v8_inspector::V8Inspector::Channel
		const char* szView = "{}";
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

	CDebugJS::~CDebugJS(void)
	{
	}

	//=====================================================================
	// v8_inspector协议
	//=====================================================================
	bool CDebugJS::ReciveRemoteData(char(&szBuffer)[2048], int32 nCurSize)
	{
		if( nCurSize < 0 )
			return false;
		//GammaLog << szBuffer << endl;
		// 确定是否V8协议（websocket)
		if( !memcmp( szBuffer, "Content-Length", 14 ) )
		{
			m_eProtocol = ePT_VSCode;
			bool bResult = CDebugBase::ReciveRemoteData(szBuffer, nCurSize);
			m_eProtocol = ePT_Unknow;
			return bResult;
		}

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

		m_eProtocol = ePT_Chrome;
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

		m_eProtocol = ePT_Unknow;
		return false;
	}

	bool CDebugJS::CheckRemoteCmd()
	{
		if (m_nRemoteConnecter == -1 || m_eProtocol == ePT_Unknow)
			return false;

		if (m_eProtocol == ePT_VSCode)
		{
			char szCommand[256];
			gammasstream(szCommand) << "{\"id\":" << m_nMessageID++
				<< ",\"method\":\"Debugger.enable\"}";
			v8_inspector::StringView view((const uint8_t*)szCommand, strlen(szCommand));
			m_strUtf8Buffer.clear();
			m_Session->dispatchProtocolMessage(view);
			return CDebugBase::CheckRemoteCmd();
		}

		while (RemoteCmdValid())
		{
			CmdLock();
			std::unique_ptr<CDebugCmd> pCmd( m_listDebugCmd.GetFirst() );
			pCmd->CDebugNode::Remove();
			CmdUnLock();
			if (!pCmd->GetName())
				continue;

			const char* szContent = pCmd->GetContent();
			uint32 nSize = pCmd->GetContentLen();
			if (*pCmd->GetName() == 'p')
			{
				SendWebSocketData(eWS_Pong, szContent, nSize);
				continue;
			}

			DEBUG_LOG("*********************cmd begin**********************\n");
			DEBUG_LOG(szContent); DEBUG_LOG("\n");
			DEBUG_LOG("**********************cmd end***********************\n");
			v8_inspector::StringView view((const uint8_t*)szContent, nSize);
			m_Session->dispatchProtocolMessage(view);
		}
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
		if (m_eProtocol == ePT_VSCode)
			return CDebugBase::Debug();

		m_bLoopOnPause = true;
		while (m_bLoopOnPause)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			CheckRemoteCmd();
		}
		ClearVariables();
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

		DEBUG_LOG( "*********************res begin**********************\n" );
		DEBUG_LOG( m_strUtf8Buffer.c_str() ); DEBUG_LOG( "\n" );
		DEBUG_LOG( "**********************res end***********************\n" );
		if (m_nRemoteConnecter == INVALID_SOCKET || m_eProtocol == ePT_VSCode)
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

		DEBUG_LOG( "*********************ntf begin**********************\n" );
		DEBUG_LOG( m_strUtf8Buffer.c_str() ); DEBUG_LOG( "\n" );
		DEBUG_LOG( "**********************ntf end***********************\n" );
		if (m_nRemoteConnecter != INVALID_SOCKET && m_eProtocol == ePT_Chrome)
			return SendWebSocketData(eWS_Text, szBuffer, nSize);

		CJson Json;
		Json.Load(szBuffer, nSize);
		std::string strMethod = Json.At<std::string>("method");
		if (strMethod == "Debugger.paused")
		{
			ClearVariables();
			CJson* params = Json.GetChild("params");
			if (!params)
				return;
			CJson* pCallFrames = params->GetChild("callFrames");
			if( !pCallFrames || !pCallFrames->GetChildCount() )
				return;

			int32 nFrameIndex = 0;
			m_aryFrame.resize( pCallFrames->GetChildCount() );
			for( CJson* pFrame = pCallFrames->GetChild( (uint32)0 );
				pFrame; pFrame = pFrame->GetNext(), nFrameIndex++ )
			{
				SFrameInfo& Frame = m_aryFrame[nFrameIndex];
				Frame.ThisInfo = new SObjectInfo;
				Frame.strCallFrameID = ReadIDFromJson(pFrame->GetChild( "callFrameId" ));
				Frame.strFunctionName = pFrame->At<std::string>( "functionName" );
				Frame.strScriptUrl = pFrame->At<std::string>( "url" );
				Frame.FunctionLocation.ReadFromJson( pFrame->GetChild( "functionLocation" ) );
				Frame.PauseLocation.ReadFromJson( pFrame->GetChild( "location" ) );
				Frame.ThisInfo->ReadFromJson(pFrame->GetChild("this"));
				AddFrameObject(Frame, *Frame.ThisInfo, "this");

				if (!memcmp(Frame.strScriptUrl.c_str(), "file://", 7))
					Frame.strScriptUrl.erase(0, 7 + (Frame.strScriptUrl[9] == ':'));
				else if (!memcmp(Frame.strScriptUrl.c_str(), "memory:///", 10))
					Frame.strScriptUrl.erase(0, 10);
				else if (Frame.strScriptUrl.empty())
					Frame.strScriptUrl = m_mapScriptInfo[Frame.PauseLocation.nScriptId];

				CJson* pScopeChain = pFrame->GetChild( "scopeChain" );
				if( !pScopeChain || !pScopeChain->GetChildCount() )
					continue;
				int32 nScopeIndex = 0;
				Frame.vecScope.resize( pScopeChain->GetChildCount() );
				for (CJson* pScope = pScopeChain->GetChild((uint32)0);
					pScope; pScope = pScope->GetNext(), nScopeIndex++)
				{
					Frame.vecScope[nScopeIndex].ReadFromJson(pScope);
					SObjectInfo* pObjInfo = Frame.vecScope[nScopeIndex].ObjectInfo;
					AddFrameObject(Frame, *pObjInfo, Frame.vecScope[nScopeIndex].strType.c_str());
				}
			}
		}
		else if (strMethod == "Debugger.scriptParsed")
		{
			CJson* params = Json.GetChild("params");
			if (!params)
				return;
			uint32 nID = params->At<uint32>("scriptId");
			const char* szUrl = params->At<const char*>("url");
			if (!nID || !szUrl || !szUrl[0])
				return;
			AddScriptInfo(nID, szUrl);
		}
	}

	void CDebugJS::flushProtocolNotifications()
	{
	}

	//=====================================================================
	// 默认协议
	//=====================================================================
	void CDebugJS::AddScriptInfo(int32 nID, const char* szFileName)
	{
		m_mapScriptInfo[nID] = szFileName;
	}

	void CDebugJS::ClearVariables()
	{
		m_aryFrame.clear();
		while (m_mapObjects.GetFirst())
			delete m_mapObjects.GetFirst();
	}

	uint32 CDebugJS::AddFrameObject( SFrameInfo& FrameInfo, SObjectInfo& ObjInfo,
		std::string strField, std::string strParentID /*= ""*/ )
	{
		if(!ObjInfo.IsInTree())
			m_mapObjects.Insert( ObjInfo );

		SObjectRefInfo& RefInfo = FrameInfo.mapObjRefs[++FrameInfo.nVariableID];
		RefInfo.pObjectInfo = &ObjInfo;
		RefInfo.strFieldName = strField;
		if (strParentID.empty())
		{
			FrameInfo.nMaxScopeID = FrameInfo.nVariableID;
			return FrameInfo.nVariableID;
		}

		SObjectInfo* pParent = m_mapObjects.Find(strParentID);
		if (pParent == nullptr)
			return FrameInfo.nVariableID;
		if(strField.empty() || strField[0] < '0' || strField[0] > '9')
			pParent->vecName.push_back(FrameInfo.nVariableID);
		else
			pParent->vecIndex.push_back(FrameInfo.nVariableID);
		return FrameInfo.nVariableID;
	}

	void CDebugJS::FetchChildren( SObjectInfo& ObjInfo )
	{
		if (m_nCurFrame >= (int32)GetFrameCount())
			return;
		SFrameInfo& CurFrame = m_aryFrame[m_nCurFrame];
		if (ObjInfo.bChildrenFetched)
			return;
		ObjInfo.bChildrenFetched = true;
		char szCommand[4096];
		uint32 nMessageID = m_nMessageID++;
		gammasstream(szCommand)
			<< "{\"id\":" << nMessageID << ","
			<< "\"method\":\"Runtime.getProperties\","
			<< "\"params\":{\"objectId\":\"" << ObjInfo.strID
			<< "\",\"ownProperties\":true}}";
		v8_inspector::StringView view((const uint8_t*)szCommand, strlen(szCommand));
		m_strUtf8Buffer.clear();
		m_Session->dispatchProtocolMessage(view);
		if (m_strUtf8Buffer.empty())
			return;
		CJson Propertys;
		Propertys.Load(m_strUtf8Buffer.c_str(), (uint32)strlen(m_strUtf8Buffer.c_str()));
		CJson* pResult = Propertys.GetChild("result");
		if (!pResult)
			return;
		pResult = pResult->GetChild("result");
		if (!pResult)
			return;
		for (auto pProp = pResult->GetChild((uint32)0);
			pProp; pProp = pProp->GetNext())
		{
			const char* szName = pProp->At<const char*>("name");
			SObjectInfo* ObjectInfo = new SObjectInfo;
			ObjectInfo->ReadFromJson(pProp->GetChild("value"));
			AddFrameObject(CurFrame, *ObjectInfo, szName, ObjInfo.strID);
		}
	}

	uint32 CDebugJS::GenBreakPointID(const char* szFileName, int32 nLine)
	{
		auto itFile = m_mapScriptInfo.begin();
		while( itFile != m_mapScriptInfo.end() &&
			itFile->second.find( szFileName ) == std::string::npos )
			++itFile;
		if( itFile == m_mapScriptInfo.end() || nLine <= 0 )
			return 0;

		char szCommand[4096];
		uint32 nMessageID = m_nMessageID++;
		gammasstream( szCommand )
			<< "{\"id\":" << nMessageID << ","
			<< "\"method\":\"Debugger.setBreakpoint\","
			<< "\"params\":{\"location\":{\"scriptId\":\"" 
			<< itFile->first << "\",\"lineNumber\":" << nLine - 1 << "}}}";
		v8_inspector::StringView view( (const uint8_t*)szCommand, strlen( szCommand ) );
		m_strUtf8Buffer.clear();
		m_Session->dispatchProtocolMessage( view );
		if( m_strUtf8Buffer.empty() )
			return 0;
		CJson BreakPoint;
		BreakPoint.Load( m_strUtf8Buffer.c_str(), (uint32)strlen( m_strUtf8Buffer.c_str() ) );
		CJson* pResult = BreakPoint.GetChild( "result" );
		if( !pResult )
			return 0;
		m_mapBreakPoint[nMessageID] = pResult->At<std::string>("breakpointId");
		return nMessageID;
	}

	void CDebugJS::DelBreakPoint( uint32 nBreakPointID )
	{
		CDebugBase::DelBreakPoint( nBreakPointID );
		auto it = m_mapBreakPoint.find(nBreakPointID);
		if (it == m_mapBreakPoint.end())
			return;
		char szCommand[4096];
		gammasstream(szCommand)
			<< "{\"id\":" << m_nMessageID++ << ","
			<< "\"method\":\"Debugger.removeBreakpoint\","
			<< "\"params\":{\"breakpointId\":\"" << it->second << "\"}}";
		v8_inspector::StringView view((const uint8_t*)szCommand, strlen(szCommand));
		m_mapBreakPoint.erase(it);
		m_Session->dispatchProtocolMessage(view);
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

	uint32 CDebugJS::GetChildrenID( uint32 nParentID, bool bIndex,
		uint32 nStart, uint32* aryChild, uint32 nCount)
	{
		if (m_nCurFrame >= (int32)GetFrameCount())
			return 0;

		SFrameInfo& CurFrame = m_aryFrame[m_nCurFrame];
		if( nParentID == eScopeID )
		{
			if( bIndex )
				return 0;
			if( aryChild == nullptr )
				return (uint32)CurFrame.vecScope.size() + 1;
			for( uint32 i = 0; i < CurFrame.vecScope.size() + 1; i++ )
				aryChild[i] = eScopeID + 1 + i;
			return (uint32)CurFrame.vecScope.size() + 1;
		}

		auto it = CurFrame.mapObjRefs.find(nParentID);
		if (it == CurFrame.mapObjRefs.end())
			return 0;
	
		SObjectInfo* pInfo = it->second.pObjectInfo;
		FetchChildren( *pInfo );

		auto& vecChild = bIndex ? pInfo->vecIndex : pInfo->vecName;
		if (nStart >= vecChild.size())
			return 0;
		if (nCount == 0 || nCount > vecChild.size() - nStart)
			nCount = (uint32)vecChild.size() - nStart;
		if (aryChild)
			memcpy(aryChild, &vecChild[nStart], sizeof(uint32) * nCount);
		return nCount;
	}

	SValueInfo CDebugJS::GetVariable( uint32 nID )
	{
		if (m_nCurFrame >= (int32)GetFrameCount())
			return Gamma::SValueInfo();

		SFrameInfo& CurFrame = m_aryFrame[m_nCurFrame];
		SValueInfo Info;
		Info.nID = nID;
		if (nID == eScopeID)
		{
			Info.strName = "Scopes";
			Info.nNameValues = (uint32)CurFrame.vecScope.size() + 1;
			return Info;
		}

		auto itRef = CurFrame.mapObjRefs.find(nID);
		if(itRef == CurFrame.mapObjRefs.end())
			return Gamma::SValueInfo();

		auto pInfo = itRef->second.pObjectInfo;
		FetchChildren(*pInfo);

		Info.strName = itRef->second.strFieldName;
		Info.nNameValues = (uint32)(pInfo->vecName.size());
		Info.nIndexValues = (uint32)(pInfo->vecIndex.size());
		Info.strValue = pInfo->strValue;
		return Info;
	}

	void CDebugJS::Stop()
	{
		char szCommand[256];
		gammasstream( szCommand ) << "{\"id\":" << m_nMessageID++
			<< ",\"method\":\"Debugger.enable\"}";
		v8_inspector::StringView view( (const uint8_t*)szCommand, strlen( szCommand ) );
		m_Session->dispatchProtocolMessage( view );

		v8_inspector::StringView breakReason((const uint8_t*)"break", 5);
		v8_inspector::StringView breakDetails((const uint8_t*)"{}", 2);
		m_Session->schedulePauseOnNextStatement(breakReason, breakDetails);
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
		m_Session->stepOver();
	}

	void CDebugJS::StepOut()
	{
		char szCommand[256];
		gammasstream(szCommand) << "{\"id\":" << m_nMessageID++
			<< ",\"method\":\"Debugger.stepOut\"}";
		v8_inspector::StringView view((const uint8_t*)szCommand, strlen(szCommand));
		m_Session->dispatchProtocolMessage(view);
	}

	uint32 CDebugJS::EvaluateExpression(int32 nCurFrame, const char* szExpression)
	{
		if (m_nCurFrame >= (int32)GetFrameCount())
			return INVALID_32BITID;
		SFrameInfo& CurFrame = m_aryFrame[m_nCurFrame];
		char szCommand[4096];
		uint32 nMessageID = m_nMessageID++;
		gammasstream(szCommand)
			<< "{\"id\":" << nMessageID << ","
			<< "\"method\":\"Debugger.evaluateOnCallFrame\",\"params\":{"
			<< "\"callFrameId\":\"" << CurFrame.strCallFrameID << "\","
			<< "\"expression\":\"" << szExpression << "\","
			<< "\"silent\":true}}";
		v8_inspector::StringView view((const uint8_t*)szCommand, strlen(szCommand));
		m_strUtf8Buffer.clear();
		m_Session->dispatchProtocolMessage(view);
		if (m_strUtf8Buffer.empty())
			return INVALID_32BITID;
		CJson Value;
		Value.Load(m_strUtf8Buffer.c_str(), (uint32)strlen(m_strUtf8Buffer.c_str()));
		CJson* pResult = Value.GetChild("result");
		if (!pResult || !pResult->GetChild("result"))
			return INVALID_32BITID;
		SObjectInfo* ObjectInfo = new SObjectInfo;
		ObjectInfo->ReadFromJson(pResult->GetChild("result"));
		return AddFrameObject(CurFrame, *ObjectInfo, "");
	}
}
