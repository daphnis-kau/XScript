/**@file  		Http.h
* @brief		Implement simple http protocal(include websocket)
* @author		Daphnis Kau
* @date			2020-01-17
* @version		V1.0
*/
#ifndef _XS_HTTP_H_
#define _XS_HTTP_H_

#include "common/CommonType.h"

namespace XS
{
	#define HTTP_REQUEST_HEAD_SIZE 256

	enum EHttpReadState
	{
		eHRS_Ok,
		eHRS_NeedMore,
		eHRS_Error,
		eHRS_Unknow,
	};

	enum EWebSocketID
	{
		eWS_Empty = 0x00,
		eWS_Text = 0x01,
		eWS_Binary = 0x02,
		eWS_Close = 0x08,
		eWS_Ping = 0x09,
		eWS_Pong = 0x0a,
	};

	/**
	 *	@struct WebSocket Protocal
	 *  @brief RFC 6455
	 *   0                   1                   2                   3
	 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 *  +-+-+-+-+-------+-+-------------+-------------------------------+
	 *  |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	 *  |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
	 *  |N|V|V|V|       |S|             |   (if payload len==126/127)   |
	 *  | |1|2|3|       |K|             |                               |
	 *  +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	 *  |     Extended payload length continued, if payload len == 127  |
	 *  + - - - - - - - - - - - - - - - +-------------------------------+
	 *  |                               |Masking-key, if MASK set to 1  |
	 *  +-------------------------------+-------------------------------+
	 *  | Masking-key (continued)       |          Payload Data         |
	 *  +-------------------------------- - - - - - - - - - - - - - - - +
	 *  :                     Payload Data continued ...                :
	 *  + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	 *  |                     Payload Data continued ...                |
	 *  +---------------------------------------------------------------+
	 */

#pragma pack(push,1)
	struct SWebSocketProtocal
	{
		uint8 m_nId : 4;
		uint8 m_nReserved : 3;
		uint8 m_bFinished : 1;
		uint8 m_nLen : 7;
		uint8 m_bMask : 1;
	};
#pragma pack(pop)

	struct SUrlInfo
	{ 
		const char*		szHost; 
		uint32			nHostLen; 
		uint16			nPort; 
		bool			bHttps; 
	};

	class CHttpRecvState
	{
		uint32			m_nHttpLength;
	public:
		CHttpRecvState();
		~CHttpRecvState();

		EHttpReadState	CheckHttpBuffer( char* szBuffer, uint32& nBufferSize );
		uint32			GetDataSize() const;
		void			Reset();
	}; 

	class CHttpRequestState
	{
		uint32			m_nKeepAlive;
		const char*		m_szDataStart;
		uint32			m_nDataLength;
		const char*		m_szPageStart;
		uint32			m_nPageLength;
		const char*		m_szParamStart;
		uint32			m_nParamLength;
		bool			m_bGetMethod;
	public:
		CHttpRequestState();
		~CHttpRequestState();

		EHttpReadState	CheckHttpBuffer( const char* szBuffer, uint32 nBufferSize );
		uint32			GetKeepAlive() const { return m_nKeepAlive; }
		const char*		GetDataStart() const { return m_szDataStart; }
		uint32			GetDataLength() const { return m_nDataLength; }
		const char*		GetPageStart() const { return m_szPageStart; }
		uint32			GetPageLength() const { return m_nPageLength; }
		const char*		GetParamStart() const { return m_szParamStart; }
		uint32			GetParamLength() const { return m_nParamLength; }
		bool			IsGetMethod() const { return m_bGetMethod; }
		void			Reset();
	}; 

	/**
	* @brief Get host and port from url
	* @return host and port
	*/
	SUrlInfo GetHostAndPortFromUrl( const char* szUrl );

	/**
	* @brief Make HTTP request buffer
	* @param [out] szBuffer Request buffer
	* @param [in] nBufferSize Request buffer size
	* @param [in] bPost Whether use as POST method
	* @param [in] szUrl Request URL
	* @param [in] pData Data for POST
	* @param [in] nDataSize Size of input POST data(pData)
	* @note nBufferSize must greater than \n
	*  HTTP_REQUEST_HEAD_SIZE + strlen( szUrl ) + nDataSize
	* @return used size of szBuffer
	*/
	uint32 MakeHttpRequest( char* szBuffer, uint32 nBufferSize,
		bool bPost, const char* szUrl, const void* pData, uint32 nDataSize);

	/**
	* @brief Make WebSocket request buffer(client side)
	* @param [out] szBuffer Request buffer
	* @param [in] nBufferSize Request buffer size
	* @param [in] aryBinKey Shake Hand Key
	* @param [in] szUrl Request URL
	* @note nBufferSize must greater than \n
	*  HTTP_REQUEST_HEAD_SIZE + strlen( szUrl )
	* @return used size of szBuffer
	*/
	uint32 MakeWebSocketShakeHand( char* szBuffer, 
		uint32 nBufferSize, uint8 (&aryBinKey)[16], const char* szUrl );

	/**
	* @brief Check WebSocket shake hand(server&client side)
	* @param [in] szBuffer Request buffer
	* @param [in] nBufferSize Request buffer size
	* @param [in] bServer whether check on server side
	* @param [out] szWebSocketKey Shake Hand Key
	* @param [out] nWebSocketKeyLen Shake Hand Key size
	* @note nBufferSize must greater than HTTP_REQUEST_HEAD_SIZE + strlen( szUrl )
	* @return Check result\n
	*	0:buffer not enough; 
	*	INVALID_32BITID:error, szWebSocketKey output message; 
	*	other:succeeded, used size of szBuffer
	*/
	uint32 WebSocketShakeHandCheck( const char* pBuffer, size_t nSize, 
		bool bServer, const char*& szWebSocketKey, uint32& nWebSocketKeyLen );

	/**
	* @brief Make shake hand response of websocket server(server side)
	* @param [out] szBuffer Request buffer
	* @param [in] nBufferSize Request buffer size
	* @param [in] szWebSocketKey Shake Hand Key
	* @param [in] nWebSocketKeyLen Shake Hand Key size
	* @note nBufferSize must greater than HTTP_REQUEST_HEAD_SIZE
	* @return used size of szBuffer
	*/
	uint32 MakeWebSocketServerShakeHandResponese( char* szBuffer,
		uint32 nBufferSize, const char* szWebSocketKey, uint32 nWebSocketKeyLen);

	/**
	* @brief Get whole message size(server&client side)
	* @param [in] pProtocol Message head
	* @param [in] nSize Size of protocol buffer( may be followed by next protocol)
	* @return if buffer is not enough, return INVALID_64BITID, otherwise return message size
	*/
	uint64 GetWebSocketProtocolLen( 
		const SWebSocketProtocal* pProtocol, uint64 nSize );

	/**
	* @brief Decode websocket protocol(server&client side)
	* @param [in] pProtocol Message head
	* @param [in/out] pExtraBuffer Protocol extra buffer 
	* @param [in/out] nSize Size of protocol extra buffer
	* @return if buffer is not enough, return INVALID_64BITID, otherwise return message size
	*/
	uint64 DecodeWebSocketProtocol( 
		const SWebSocketProtocal* pProtocol, char*& pExtraBuffer, uint64& nSize );

	/**
	* @brief Encode websocket protocol(server&client side)
	* @param [in] pProtocol Message head
	* @param [in] pExtraBuffer Protocol extra buffer
	* @param [in] nSize Size of protocol extra buffer
	* @return Encode mask when Protocol.m_bMask = true
	*/
	uint32 EncodeWebSocketProtocol( 
		SWebSocketProtocal& Protocol, char* pExtraBuffer, uint64 nSize );

}

#endif 

