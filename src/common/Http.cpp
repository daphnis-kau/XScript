#include "common/Http.h"
#include "common/Help.h"
#include "common/SHA1.h"
#include "common/TStrStream.h"

namespace XS
{
#define INVALID_DATA_SIZE ( (uint32)( INVALID_32BITID - 1 ) )

	///< Base64 Encode
	static int32 Base64Encode(
		char* pBase64Buffer, uint32 nOutLen, const void* pSrcBuffer, uint32 nSrcLen )
	{
		static const char aryTable64[64] =
		{
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
			'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
			'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
			'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
			'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
			'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
			'w', 'x', 'y', 'z', '0', '1', '2', '3',
			'4', '5', '6', '7', '8', '9', '+', '/',
		};

		const char cEnd = '=';
		if( nOutLen < (uint32)AligenUp( nSrcLen, 3 )*4/3 )
			return -1;

		const uint8* pSrc = (const uint8*)pSrcBuffer;
		uint32 nSrc = 0;
		uint32 nDes = 0;
		while( nSrc < nSrcLen )
		{
			uint32 c = pSrc[nSrc++];
			pBase64Buffer[nDes++] = aryTable64[c>>2];
			c = c & 0x3;

			if( nSrc < nSrcLen )
			{
				c = ( c << 8 )|pSrc[nSrc++];
				pBase64Buffer[nDes++] = aryTable64[c>>4];
				c = c&0xf;

				if( nSrc < nSrcLen )
				{
					c = ( c << 8 )|pSrc[nSrc++];
					pBase64Buffer[nDes++] = aryTable64[c>>6];
					pBase64Buffer[nDes++] = aryTable64[c&0x3f];
				}
				else
				{
					pBase64Buffer[nDes++] = aryTable64[c<<2];
					pBase64Buffer[nDes++] = cEnd;
				}
			}
			else
			{
				pBase64Buffer[nDes++] = aryTable64[c<<4];
				pBase64Buffer[nDes++] = cEnd;
				pBase64Buffer[nDes++] = cEnd;
			}
		}

		if( (int32)nDes < (int32)nOutLen )
			pBase64Buffer[nDes] = 0;
		return nDes;
	}

	///< Read integer value from buffer
	uint32 GetUnsignedInt( const char* szBuffer, size_t& nLinePos, size_t nSize )
	{
		// skip invalid character
		while( nLinePos < nSize && 
			!IsNumber( szBuffer[nLinePos] ) && 
			!( szBuffer[nLinePos] >= 'A' && szBuffer[nLinePos] <= 'F' ) && 
			!( szBuffer[nLinePos] >= 'a' && szBuffer[nLinePos] <= 'f' ) )
		{
			if( szBuffer[nLinePos] == '\n' )
				return 0;
			nLinePos++;
		}

		uint32 nValue = 0;
		while( nLinePos < nSize )
		{
			if( !IsHexNumber( szBuffer[nLinePos] ) )
				break;
			nValue = nValue*16 + ValueFromHexNumber( szBuffer[nLinePos++] );
		}

		return nValue;
	}

	///< CHttpRecvState::CHttpRecvState
	CHttpRecvState::CHttpRecvState(void)
		: m_nHttpLength( INVALID_32BITID )
	{
	}

	CHttpRecvState::~CHttpRecvState(void)
	{
	}

	EHttpReadState CHttpRecvState::CheckHttpBuffer( 
		char* szBuffer, uint32& nBufferSize )
	{
		if( m_nHttpLength == INVALID_DATA_SIZE )
			return eHRS_NeedMore;

		uint32 nSize = nBufferSize;
		if( m_nHttpLength == INVALID_32BITID && nSize < 4 )
			return eHRS_NeedMore;

		if( m_nHttpLength != INVALID_32BITID )
		{
			if( nSize < m_nHttpLength )
				return eHRS_NeedMore;
			nBufferSize = m_nHttpLength;
			return eHRS_Ok;
		}

		const char* szMsg = szBuffer;
		if( *(uint32*)szMsg != MAKE_DWORD( 'H', 'T', 'T', 'P' ) )
		{
			m_nHttpLength = INVALID_DATA_SIZE;
			return eHRS_NeedMore;
		}

		///< skip version
		size_t nLinePos = 4;
		while( nLinePos < nSize && szMsg[nLinePos] != ' ' )
			nLinePos++;

		///< need more data
		if( nLinePos == nSize )
			return eHRS_NeedMore;

		///< skip space
		while( nLinePos < nSize && szMsg[nLinePos] == ' ')
			nLinePos++;

		///< need more data
		if( nLinePos == nSize )
			return eHRS_NeedMore;

		///< check http status
		if( szMsg[nLinePos] != '2' )
			return eHRS_Error;

		const char* szTransferType = nullptr;
		uint32 nContentLength = INVALID_32BITID;
		while( true )
		{
			///< need more data
			if( nLinePos >= nSize )
				return eHRS_NeedMore;

			if( szMsg[nLinePos++] != '\n' )
				continue;

			if( szMsg[nLinePos] == '\n' || *(uint16*)( szMsg + nLinePos ) == MAKE_UINT16( '\r', '\n' ) )
			{
				nLinePos += szMsg[nLinePos] == '\n' ? 1 : 2;
				break;
			}

			static const char* szContentLengthTag = "Content-Length";
			static const size_t nContentLengthTagLen = strlen( szContentLengthTag );

			static const char* szTransferEncoding = "Transfer-Encoding";
			static const size_t nTransferEncodingLen = strlen( szTransferEncoding );

			if( nLinePos + nContentLengthTagLen <= nSize && 
				!memcmp( szMsg + nLinePos, szContentLengthTag, nContentLengthTagLen ) )
			{
				nLinePos += (uint32)nContentLengthTagLen;

				// skip invalid character
				while( nLinePos < nSize && !IsNumber( szMsg[nLinePos] ) )
					nLinePos++;

				nContentLength = 0;
				while( nLinePos < nSize && IsNumber( szMsg[nLinePos] ) )
					nContentLength = nContentLength*10 + szMsg[nLinePos++] - '0';

				///< need more data
				if( nLinePos >= nSize )
					return eHRS_NeedMore;

				if( nContentLength == 0 )
				{
					m_nHttpLength = 0;
					nBufferSize = 0;
					return eHRS_Ok;
				}
			}
			else if( nLinePos + nTransferEncodingLen <= nSize && 
				!memcmp( szMsg + nLinePos, szTransferEncoding, nTransferEncodingLen ) )
			{
				nLinePos += nTransferEncodingLen;
				nContentLength = INVALID_DATA_SIZE;
				szTransferType = szMsg + nLinePos;
				while( *szTransferType != ':' )
					szTransferType++;
				szTransferType++;
				while( IsBlank( *szTransferType ) )
					szTransferType++;
			}
		}

		///< need more data
		if( nLinePos >= nSize )
			return eHRS_NeedMore;

		if( nContentLength == INVALID_32BITID )
		{
			m_nHttpLength = INVALID_DATA_SIZE;
			return eHRS_NeedMore;
		}

		///< Content-Length exist
		if( nContentLength < INVALID_DATA_SIZE )
		{
			m_nHttpLength = nContentLength;
			nBufferSize -= (uint32)nLinePos;
			memmove( &szBuffer[0], &szBuffer[nLinePos], nBufferSize );
			if( nBufferSize > m_nHttpLength )
				nBufferSize = m_nHttpLength;
			return nBufferSize == m_nHttpLength ? eHRS_Ok : eHRS_NeedMore;
		}

		static const char* szChunkedTypeTag = "chunked";
		static const size_t nChunkedTypeTagLen = strlen( szChunkedTypeTag );
		if( memcmp( szChunkedTypeTag, szTransferType, nChunkedTypeTagLen ) )
			return eHRS_Error;

		size_t nStart = nLinePos;

		///< data's integrity check
		while( true )
		{
			nContentLength = GetUnsignedInt( szMsg, nLinePos, nSize );
			///< need more data
			if( szMsg[nLinePos] != '\r' )
				return eHRS_NeedMore;

			///< skip newline character
			while( nLinePos < nSize && szMsg[nLinePos] != '\n' )
				nLinePos++;
			nLinePos++;

			///< need more data
			if( nLinePos + nContentLength >= nSize )
				return eHRS_NeedMore;

			if( nContentLength == 0 )
				break;

			nLinePos += nContentLength;

			///< skip newline character
			while( nLinePos < nSize && szMsg[nLinePos] != '\n' )
				nLinePos++;
			nLinePos++;
		}

		///< Read extra data
		nLinePos = nStart;
		nBufferSize = 0;
		while( true )
		{
			nContentLength = GetUnsignedInt( szMsg, nLinePos, nSize );
			///< skip newline character
			while( nLinePos < nSize && szMsg[nLinePos] != '\n' )
				nLinePos++;
			nLinePos++;
			if( nContentLength == 0 )
				break;

			memmove( &szBuffer[nBufferSize], szMsg + nLinePos, nContentLength );
			nBufferSize += nContentLength;
			nLinePos += nContentLength;

			///< skip newline character
			while( nLinePos < nSize && szMsg[nLinePos] != '\n' )
				nLinePos++;
			nLinePos++;
		}

		return eHRS_Ok;
	}

	uint32 CHttpRecvState::GetDataSize() const
	{
		if( m_nHttpLength >= INVALID_DATA_SIZE )
			return 0;
		return m_nHttpLength;
	}

	void CHttpRecvState::Reset()
	{
		m_nHttpLength = INVALID_32BITID;
	}

	///< CHttpRequestState::CHttpRequestState
	CHttpRequestState::CHttpRequestState()
		: m_nKeepAlive( INVALID_32BITID )
		, m_szDataStart( nullptr )
		, m_nDataLength( INVALID_32BITID )
		, m_bGetMethod( true )
		, m_szPageStart( nullptr )
		, m_nPageLength( 0 )
		, m_szParamStart( nullptr )
		, m_nParamLength( 0 )
	{
	}

	CHttpRequestState::~CHttpRequestState()
	{
	}

	XS::EHttpReadState CHttpRequestState::CheckHttpBuffer( 
		const char* szBuffer, uint32 nBufferSize )
	{
		if( nBufferSize < 6 )
			return eHRS_NeedMore;
		if( !memcmp( szBuffer, "POST /", 6 ) )
			m_bGetMethod = false;
		else if( !memcmp( szBuffer, "GET /", 5 ) )
			m_bGetMethod = true;
		else
			return eHRS_Error;

		uint32 nCurPos = m_bGetMethod ? 5 : 6;
		uint32 nNameStart = nCurPos;

		///< get page path  
		while( szBuffer[nCurPos] != ' ' && nCurPos < nBufferSize )
		{
			if( szBuffer[nCurPos] == '?' )
				m_szParamStart = szBuffer + nCurPos + 1;
			nCurPos++;
		}
		if( nCurPos == nBufferSize )
			return eHRS_NeedMore;
		m_szPageStart = szBuffer + nNameStart;
		if( m_szParamStart == nullptr )
		{
			m_nPageLength = nCurPos - nNameStart;
			m_szParamStart = "";
		}
		else
		{
			m_nPageLength = (uint32)( m_szParamStart - m_szPageStart - 1 );
			m_nParamLength = (uint32)( szBuffer + nCurPos - m_szParamStart );
		}

		m_nDataLength = 0;
		m_szDataStart = nullptr;

		///< get data length  
		while( true )
		{ 
			while( szBuffer[nCurPos] != '\n' )
			{
				if( ++nCurPos < nBufferSize )
					continue;
				return eHRS_NeedMore;
			}

			if( szBuffer[++nCurPos] == '\r' )
			{
				///< need more data
				if( nBufferSize < nCurPos + 2 + m_nDataLength )
					return eHRS_NeedMore;
				m_szDataStart = szBuffer + nCurPos + 2;
				return eHRS_Ok;
			}
			else if( nCurPos + 15 < nBufferSize &&
				!memcmp( "Content-Length:", szBuffer + nCurPos, 15 ) )
			{
				///< get data length   
				nCurPos += 15;
				const char* szSizeStart = nullptr;
				while( nCurPos < nBufferSize && szBuffer[nCurPos] != '\r' )
					if( IsNumber( szBuffer[nCurPos++] ) && szSizeStart == nullptr )
						szSizeStart = szBuffer + nCurPos - 1;
				///< need more data
				if( nCurPos == nBufferSize )
					return eHRS_NeedMore;
				if( szBuffer[nCurPos] != '\r' )
					return eHRS_Error;
				m_nDataLength = atoi( szSizeStart );
			}
			else if( nCurPos + 11 < nBufferSize &&
				!memcmp( "Connection:", szBuffer + nCurPos, 11 ) )
			{
				///< get type 
				nCurPos += 11;
				while( nCurPos < nBufferSize && szBuffer[nCurPos] != '\r' )
				{
					if( szBuffer[nCurPos] == 'C' || szBuffer[nCurPos] == 'c' )
						m_nKeepAlive = 0;
					nCurPos++;
				}
				///< need more data
				if( nCurPos == nBufferSize )
					return eHRS_NeedMore;
				if( szBuffer[nCurPos] != '\r' )
					return eHRS_Error;
			}
			else if( nCurPos + 11 < nBufferSize &&
				!memcmp( "Keep-Alive:", szBuffer + nCurPos, 11 ) )
			{
				///< get data length  
				nCurPos += 11;
				const char* szTimeoutStart = nullptr;
				while( nCurPos < nBufferSize && szBuffer[nCurPos] != '\r' )
					if( IsNumber( szBuffer[nCurPos++] ) && szTimeoutStart == nullptr )
						szTimeoutStart = szBuffer + nCurPos - 1;
				///< need more data
				if( nCurPos == nBufferSize )
					return eHRS_NeedMore;
				if( szBuffer[nCurPos] != '\r' )
					return eHRS_Error;
				m_nKeepAlive = atoi( szTimeoutStart );
			}
		}

		return eHRS_Error;
	}

	void CHttpRequestState::Reset()
	{
		m_bGetMethod = true;
		m_szDataStart = nullptr;
		m_nDataLength = 0;
		m_szPageStart = nullptr;
		m_nPageLength = 0;
		m_szParamStart = nullptr;
		m_nPageLength = 0;
	}

	SUrlInfo GetHostAndPortFromUrl( const char* szUrl )
	{
		SUrlInfo Info = { 0, 0, 0 };
		uint32 nHeadLen = 0;
		if( memcmp( szUrl, "http://", 7 ) == 0 )
			nHeadLen = 7;
		else if( memcmp( szUrl, "https://", 8 ) == 0 )
			nHeadLen = 8;

		if( nHeadLen == 0 )
			return Info;

		Info.bHttps = nHeadLen == 8;
		Info.nPort = Info.bHttps ? 443 : 80;
		Info.szHost = szUrl + nHeadLen;
		while( Info.szHost[Info.nHostLen] != '/' && 
			Info.szHost[Info.nHostLen] != ':' )
			Info.nHostLen++;
		if( Info.szHost[Info.nHostLen] == ':' )
			Info.nPort = (uint16)ToInt32( Info.szHost + Info.nHostLen + 1 );
		return Info;
	}
	
	uint32 MakeHttpRequest( char* szBuffer, uint32 nBufferSize, 
		bool bPost, const char* szUrl, const void* pData, uint32 nDataSize )
	{
		uint32 nUrlLen = (uint32)strlen( szUrl );
		if( nBufferSize <= nUrlLen + nDataSize + HTTP_REQUEST_HEAD_SIZE )
			return 0;
		SUrlInfo Info = GetHostAndPortFromUrl( szUrl );
		const char* szMeth = bPost ? "POST " : "GET ";
		uint32 nLen =
			(uint32)( char_stream( szBuffer, nBufferSize ) 
			<< szMeth << ( Info.szHost + Info.nHostLen ) << " HTTP/1.1\r\n"
			<< "Host: " << const_string( Info.szHost, Info.nHostLen, true ) << "\r\n"
			<< "Accept: */*\r\n" 
			<< "Content-Length: " << nDataSize << "\r\n"
			<< "Content-Type: application/x-www-form-urlencoded\r\n\r\n"
			).GetCurPos();
		if( nDataSize )
			memcpy( szBuffer + nLen, pData, nDataSize );
		return nLen + nDataSize;
	}

	uint32 MakeWebSocketShakeHand( char* szBuffer, uint32 nBufferSize, 
		uint8(&aryBinKey)[16], const char* szUrl )
	{
		uint32 nUrlLen = (uint32)strlen( szUrl );
		if( nBufferSize <= nUrlLen + HTTP_REQUEST_HEAD_SIZE )
			return 0;
		SUrlInfo Info = GetHostAndPortFromUrl( szUrl );

		char szUUID[64];
		Base64Encode(szUUID, ELEM_COUNT(szUUID), aryBinKey, sizeof(aryBinKey));

		char_stream ssShakeHand( szBuffer, nBufferSize );
		ssShakeHand <<
			"GET " << ( Info.szHost + Info.nHostLen ) << " HTTP/1.1\r\n"
			"Host: " << const_string( Info.szHost, Info.nHostLen, true ) << " \r\n"
			"Upgrade: websocket\r\n"
			"Connection: Upgrade\r\n"
			"Sec-WebSocket-Key: " << szUUID << "\r\n"
			"Origin: null\r\n"
			"Sec-WebSocket-Version: 13\r\n\r\n";
		return (uint32)ssShakeHand.GetCurPos();
	}

	uint32 WebSocketShakeHandCheck( const char* pBuffer, size_t nSize, 
		bool bServer, const char*& szWebSocketKey, uint32& nWebSocketKeyLen )
	{
		uint32 nReadCount = 0;
		uint32 nPreLineStart = 0;
		uint32 nKeyCount = 0;
		const char* szKeyStart = nullptr;
		bool bFinished = false;
		while( nReadCount < nSize )
		{
			if( pBuffer[nReadCount] == '\r' )
			{
				if( ++nReadCount >= nSize )
					return 0;

				///< must be "\r\n"
				if (pBuffer[nReadCount] != '\n')
				{
					szWebSocketKey = "shakehand error( miss 0x0a )";
					return INVALID_32BITID;
				}

				///< check key of web socket
				if( bServer )
				{
					static const char* szKey = "Sec-WebSocket-Key";
					static const uint32 nKey = (uint32)strlen( szKey );
					if( !memcmp( szKey, pBuffer + nPreLineStart, nKey ) )
					{
						szKeyStart = pBuffer + nPreLineStart + nKey;
						nKeyCount = nReadCount - 1 - nPreLineStart - nKey;
					}
				}
				else
				{
					static const char* szKey = "Sec-WebSocket-Accept";
					static const uint32 nKey = (uint32)strlen( szKey );
					if( !memcmp( szKey, pBuffer + nPreLineStart, nKey ) )
					{
						szKeyStart = pBuffer + nPreLineStart + nKey;
						nKeyCount = nReadCount - 1 - nPreLineStart - nKey;
					}				
				}

				///< empty line
				if( nPreLineStart + 1 == nReadCount )
				{
					bFinished = true;
					nReadCount++;
					break;
				}
				nPreLineStart = nReadCount + 1;
			}
			++nReadCount;
		}	

		if( !bFinished )
			return 0;

		if( !szKeyStart )
		{
			szWebSocketKey = "shakehand error( no key )";
			return INVALID_32BITID;
		}

		uint32 nKeyIndex = 0;
		while( nKeyIndex < nKeyCount && 
			( szKeyStart[nKeyIndex] == ':' || IsBlank( szKeyStart[nKeyIndex] ) ) )
			nKeyIndex++;

		if( nKeyIndex >= nKeyCount )
		{
			szWebSocketKey = "shakehand error( key length = 0 )";
			return INVALID_32BITID;
		}

		if( nKeyCount - nKeyIndex > 64 )
		{
			szWebSocketKey = "shakehand error( key length >= 64 )";
			return INVALID_32BITID;
		}

		szWebSocketKey = szKeyStart + nKeyIndex;
		nWebSocketKeyLen = nKeyCount - nKeyIndex;
		return nReadCount;
	}

	uint32 MakeWebSocketServerShakeHandResponese( char* szBuffer, 
		uint32 nBufferSize, const char* szWebSocketKey, uint32 nWebSocketKeyLen )
	{
		// fill http response head
		char szClientKey[64 + 40];
		assert( nWebSocketKeyLen + 36 < ELEM_COUNT(szClientKey) );
		memcpy(szClientKey, szWebSocketKey, nWebSocketKeyLen);
		memcpy(szClientKey + nWebSocketKeyLen, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);
		szClientKey[nWebSocketKeyLen + 36] = 0;

		tbyte shaHash[20];
		sha1((const tbyte*)szClientKey, nWebSocketKeyLen + 36, shaHash);
		char szBase64[256];
		Base64Encode(szBase64, ELEM_COUNT(szBase64), shaHash, sizeof(shaHash));

		char_stream ssBuffer(szBuffer, nBufferSize);
		ssBuffer <<	"HTTP/1.1 101 Switching Protocols\r\n"
			"Upgrade: websocket\r\n"
			"Connection: upgrade\r\n"
			"Sec-WebSocket-Accept: " << szBase64 << "\r\n\r\n";
		return (uint32)ssBuffer.GetCurPos();
	}

	uint64 GetWebSocketProtocolLen( 
		const SWebSocketProtocal* pProtocol, uint64 nSize )
	{
		union{ uint64 u64; uint8 u8[sizeof(uint64)]; } Size;
		Size.u64 = pProtocol->m_nLen;
		const char* pStart = (const char*)(pProtocol + 1);
		const char* pAppend = pStart;
		if( Size.u64 == 126 )
		{
			if( nSize < sizeof(SWebSocketProtocal) + 2 )
				return INVALID_64BITID;
			Size.u8[1] = *pAppend++;
			Size.u8[0] = *pAppend++;
		}
		else if( Size.u64 == 127 )
		{
			if( nSize < sizeof(SWebSocketProtocal) + 8 )
				return INVALID_64BITID;
			Size.u8[7] = *pAppend++;
			Size.u8[6] = *pAppend++;
			Size.u8[5] = *pAppend++;
			Size.u8[4] = *pAppend++;
			Size.u8[3] = *pAppend++;
			Size.u8[2] = *pAppend++;
			Size.u8[1] = *pAppend++;
			Size.u8[0] = *pAppend++;
		}

		return (pAppend - pStart) + Size.u64 + (pProtocol->m_bMask ? 4 : 0);
	}

	uint64 DecodeWebSocketProtocol( 
		const SWebSocketProtocal* pProtocol, char*& pExtraBuffer, uint64& nSize )
	{
		uint64 nLen = pProtocol->m_nLen;
		const char* pStart = pExtraBuffer;
		char* pAppend = (char*)pStart;
		uint32 nAppendSize = 0;
		if( nLen >= 126 )
		{
			nAppendSize = nLen == 126 ? sizeof(uint16) : sizeof(uint64);
			if( nSize < nAppendSize )
				return INVALID_64BITID;
			char* pDest = (char*)&nLen;
			for( uint32 i = 0; i < nAppendSize; i++ )
				pDest[nAppendSize - 1 - i] = pAppend[i];
			pAppend += nAppendSize;
		}

		if( nSize < nAppendSize + nLen )
			return INVALID_64BITID;

		if( pProtocol->m_bMask )
		{
			const char* szMask = pAppend;
			pAppend += 4;
			for( uint64 i = 0; i < nLen; i++ )
				pAppend[i] ^= szMask[i%4];
		}

		pExtraBuffer = pAppend;
		nSize = nLen;
		return ( pAppend - pStart ) + nLen; 
	}

	uint32 EncodeWebSocketProtocol(
		SWebSocketProtocal& Protocol, char* pExtraBuffer, uint64 nSize )
	{
		union { uint64 u64; uint8 u8[sizeof(uint64)]; } Size;
		Size.u64 = nSize;
		Protocol.m_bFinished = 1;
		if (Size.u64 < 126)
			Protocol.m_nLen = Size.u8[0];
		else if (Size.u64 < 65536)
			Protocol.m_nLen = 126;
		else
			Protocol.m_nLen = 127;

		if (!Protocol.m_bMask)
			return 0;

		static int32 nRand = (int32)time( nullptr );
		char szMask[] =
		{
			(char)( ( ( nRand = nRand * 214013L + 2531011L ) >> 16 ) & 0xff ),
			(char)( ( ( nRand = nRand * 214013L + 2531011L ) >> 16 ) & 0xff ),
			(char)( ( ( nRand = nRand * 214013L + 2531011L ) >> 16 ) & 0xff ),
			(char)( ( ( nRand = nRand * 214013L + 2531011L ) >> 16 ) & 0xff )
		};

		for( uint32 i = 0; i < nSize; i++ )
			pExtraBuffer[i] ^= szMask[i % 4];
		return *(uint32*)szMask;
	}

}