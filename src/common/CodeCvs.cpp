#include "common/Help.h"
#include "common/CodeCvs.h"
#include <locale.h>
#include <stdlib.h>
#pragma warning(disable: 4996)

//    UNICODE UTF-8 
//    00000000 - 0000007F 0xxxxxxx 
//    00000080 - 000007FF 110xxxxx 10xxxxxx 
//    00000800 - 0000FFFF 1110xxxx 10xxxxxx 10xxxxxx 
//    00010000 - 001FFFFF 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx 
//    00200000 - 03FFFFFF 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 
//    04000000 - 7FFFFFFF 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 

namespace Gamma
{
	template<class wchar_type>
	inline const uint8* GetUcs2( wchar_type* pUcs, const uint8* pUtf8 )
	{
		if( ( pUtf8[0]&0x80 ) == 0 )
		{
			if( pUcs )
				*pUcs = pUtf8[0];
			return pUtf8 + 1;
		}

		uint8 nZeroBit = 6;
		// 0100 0000 --------> 0000 0010，查找第几位是0
		for( uint8 nMask = 0x40; nZeroBit && ( nMask&pUtf8[0] ); nMask = nMask >> 1 )
			nZeroBit--;

		// 第六位或者1～7位都为1的字符串不是utf8字符串
		if( nZeroBit == 0 || nZeroBit == 6 )
			return NULL;

		// 位数不够也不是utf8字符串
		uint8 nLearderByte = *( pUtf8++ );
		size_t nFollowByte = 6 - nZeroBit;
		for( size_t i = 0; i < nFollowByte; i++ )
			if( ( pUtf8[i]&0xC0 ) != 0x80 )
				return NULL;

		if( pUcs )
		{
			*pUcs = nLearderByte&( 0xff >> ( 8 - nZeroBit ) );
			for( size_t i = 0; i < nFollowByte; i++ )
				*pUcs = ( *pUcs << 6 )|( pUtf8[i]&0x3F );
		}

		return pUtf8 + nFollowByte;
	}

	const char* GetUnicode( uint32& cUnicode, const char* pUtf8 )
	{
		return (const char*)GetUcs2( &cUnicode, (const uint8*)pUtf8 );
	}

	bool IsUtf8( const char* pUtf8, uint32 nLen )
	{
		if( !pUtf8 )
			return false;

		const uint32 nMaxLen = (uint32)-1;
		const uint8* pBuf = (const uint8*)pUtf8;
		while( *pBuf && nLen )
		{       
			const uint8* pNextBuf = GetUcs2( (wchar_t*)NULL, pBuf );
			if( !pNextBuf )
				return false;
			nLen = ( nLen == nMaxLen ) ? nMaxLen : (uint32)( nLen - ( pNextBuf - pBuf ) );
			pBuf = pNextBuf;
		}

		return true;		
	}
	
	int32 GetCharacterCount( const char* pUtf8, uint32 nLen )
	{
		if( !pUtf8 )
			return 0;
		uint32 nPos = 0;
		for( const uint8* pBuf = (const uint8*)pUtf8; *pBuf && nPos < nLen; nPos++ )
			if( ( pBuf = GetUcs2( (wchar_t*)NULL, pBuf ) ) == NULL )
				return -1;
		return (int32)nPos;
	}

	uint32 Utf8ToUcs( wchar_t* pUcs, uint32 nSize, const char* pUtf8, uint32 nLen )
	{    
		if( !pUtf8 )
			return 0;

		const uint32 nMaxLen = (uint32)-1;
		uint32 nPos = 0;
		for( const uint8* pBuf = (const uint8*)pUtf8; *pBuf && nLen; nPos++ )
		{
			if( pUcs && nPos >= nSize )
				break;

			const uint8* pNextBuf = GetUcs2( pUcs ? pUcs + nPos : NULL, pBuf );
			if( !pNextBuf )
				break;
			nLen = ( nLen == nMaxLen ) ? nMaxLen : (uint32)( nLen - ( pNextBuf - pBuf ) );
			pBuf = pNextBuf;
		}

		if( nPos < nSize && pUcs )
			pUcs[nPos] = 0;

		return nPos;
	}

	uint32 Utf8ToUcs2( uint16* pUcs, uint32 nSize, const char* pUtf8, uint32 nLen /*= -1 */ )
	{
		if( !pUtf8 )
			return 0;

		const uint32 nMaxLen = (uint32)-1;
		uint32 nPos = 0;
		for( const uint8* pBuf = (const uint8*)pUtf8; *pBuf && nLen; nPos++ )
		{
			if( pUcs && nPos >= nSize )
				break;

			const uint8* pNextBuf = GetUcs2( pUcs ? pUcs + nPos : NULL, pBuf );
			if( !pNextBuf )
				break;
			nLen = ( nLen == nMaxLen ) ? nMaxLen : (uint32)( nLen - ( pNextBuf - pBuf ) );
			pBuf = pNextBuf;
		}

		if( nPos < nSize && pUcs )
			pUcs[nPos] = 0;

		return nPos;
	}

	template<typename wchar_type>
	uint32 TUcsToUtf8( char* pUtf8, uint32 sizeBuf, const wchar_type* pUnicode, uint32 nLen )
	{
		if( !pUnicode )
			return 0;

		uint32 nDesPos = 0;
		uint32 nSrcPos = 0;
		while( nSrcPos < nLen && pUnicode[nSrcPos] )
		{
			uint32 cUnicode = pUnicode[nSrcPos++];
			if( cUnicode < 0x00000080 )
			{
				if( pUtf8 )
				{
					if( nDesPos >= sizeBuf )
						break;
					pUtf8[nDesPos] = (char)cUnicode;
				}
				nDesPos++;
			}
			else if( cUnicode < 0x000007FF )
			{
				if( pUtf8 )
				{
					if( nDesPos + 1 >= sizeBuf )
						break;
					pUtf8[nDesPos]		= (char)( ( cUnicode >> 6 )|0xC0 );
					pUtf8[nDesPos+1]	= (char)( ( cUnicode&0x3f )|0x80 );
				}
				nDesPos += 2;
			}
			else if( cUnicode < 0x0000FFFF )
			{
				if( pUtf8 )
				{
					if( nDesPos + 2 >= sizeBuf )
						break;
					pUtf8[nDesPos]		= (char)( ( cUnicode >> 12 )|0xE0 );
					pUtf8[nDesPos+1]	= (char)( ( ( cUnicode >> 6 )&0x3f )|0x80 );
					pUtf8[nDesPos+2]	= (char)( ( cUnicode&0x3f )|0x80 );
				}
				nDesPos += 3;
			}
			else if( cUnicode < 0x001FFFFF )
			{
				if( pUtf8 )
				{
					if( nDesPos + 3 >= sizeBuf )
						break;
					pUtf8[nDesPos]		= (char)( ( cUnicode >> 18 )|0xF0 );
					pUtf8[nDesPos+1]	= (char)( ( ( cUnicode >> 12 )&0x3f )|0x80 );
					pUtf8[nDesPos+2]	= (char)( ( ( cUnicode >> 6  )&0x3f )|0x80 );
					pUtf8[nDesPos+3]	= (char)( ( cUnicode&0x3f )|0x80 );
				}
				nDesPos += 4;
			}
			else if( cUnicode < 0x03FFFFFF )
			{
				if( pUtf8 )
				{
					if( nDesPos + 4 >= sizeBuf )
						break;
					pUtf8[nDesPos]		= (char)( ( cUnicode >> 24 )|0xF8 );
					pUtf8[nDesPos+1]	= (char)( ( ( cUnicode >> 18 )&0x3f )|0x80 );
					pUtf8[nDesPos+2]	= (char)( ( ( cUnicode >> 12 )&0x3f )|0x80 );
					pUtf8[nDesPos+3]	= (char)( ( ( cUnicode >> 6  )&0x3f )|0x80 );
					pUtf8[nDesPos+4]	= (char)( ( cUnicode&0x3f )|0x80 );
				}
				nDesPos += 5;
			}
			else
			{
				if( pUtf8 )
				{
					if( nDesPos + 5 >= sizeBuf )
						break;
					pUtf8[nDesPos]		= (char)( ( cUnicode >> 30 )|0xFC );
					pUtf8[nDesPos+1]	= (char)( ( ( cUnicode >> 24 )&0x3f )|0x80 );
					pUtf8[nDesPos+1]	= (char)( ( ( cUnicode >> 18 )&0x3f )|0x80 );
					pUtf8[nDesPos+2]	= (char)( ( ( cUnicode >> 12 )&0x3f )|0x80 );
					pUtf8[nDesPos+3]	= (char)( ( ( cUnicode >> 6  )&0x3f )|0x80 );
					pUtf8[nDesPos+4]	= (char)( ( cUnicode&0x3f )|0x80 );
				}
				nDesPos += 6;
			}
		}

		if( nDesPos < sizeBuf && pUtf8 )
			pUtf8[nDesPos] = 0;

		return nDesPos;
	}
	
	uint32 UcsToUtf8( char* pUtf8, uint32 sizeBuf, const wchar_t* pUnicode, uint32 nLen )
	{
		return TUcsToUtf8( pUtf8, sizeBuf, pUnicode, nLen );
	}

	uint32 Ucs2ToUtf8( char* pUtf8, uint32 sizeBuf, const uint16* pUnicode, uint32 nLen )
	{
		return TUcsToUtf8( pUtf8, sizeBuf, pUnicode, nLen );
	}

	uint32 Uint82Base16( const uint8* pUint8, char* pBase16, uint32 sizeBuf )
	{
		if( !pUint8 || !pBase16 )
			return 0;

		uint32 nPos = 0;
		for( uint8* pBuf = (uint8*)pUint8; *pBuf; nPos++ )
		{
			if(pBase16)
			{
				if( 2 * nPos + 1>= sizeBuf )
					break;
				pBase16[2 * nPos + 1] = pUint8[nPos] & 0x0f;
				pBase16[2 * nPos] = pUint8[nPos]>>4;
				pBase16[2 * nPos + 1] += pBase16[2 * nPos + 1] < 10 ? '0' : 'a' - 10;
				pBase16[2 * nPos] += pBase16[2 * nPos] < 10 ? '0' : 'a' - 10;
			}
		}
		if(pBase16 && 2 * nPos < sizeBuf)
			pBase16[2 * nPos] = '\0';

		return 2 * nPos + 1;
	}

	GAMMA_COMMON_API uint32 URLEncode( const uint8* pUint8, char* pEncode, uint32 sizeBuf )
	{
		uint32 i = 0;
		static unsigned char hexchars[] = "0123456789ABCDEF";

		while( *pUint8 )
		{
			uint8 c = *pUint8++;

			if( c < 0x80 )
			{
				if( i + 1 >= sizeBuf )
					break;
				pEncode[i++] = c;
			}
			else
			{
				if( i + 3 >= sizeBuf )
					break;
				pEncode[i++] = '%';
				pEncode[i++] = hexchars[c >> 4];
				pEncode[i++] = hexchars[c & 15];
			}
		}

		pEncode[i++] = 0;
		return i;
	}

	GAMMA_COMMON_API uint32 URLDecode( const char* pEncode, uint8* pUint8, uint32 sizeBuf )
	{
		uint32 i = 0;

		while( *pEncode && i + 1 < sizeBuf )
		{
			uint8 c = *pEncode++;

			if( c == '+' )
				pUint8[i++] = (uint8)' ';
			else if( c == '%' && IsHexNumber( pEncode[0] ) && IsHexNumber( pEncode[1] ) )
            {
                uint32 c1 = ValueFromHexNumber( *pEncode++ ) << 4;
                uint32 c2 = ValueFromHexNumber( *pEncode++ );
				pUint8[i++] = (uint8)( c1|c2 );
            }
			else
				pUint8[i++] = (uint8)c;
		}
		if (*pEncode)
			pUint8[i++] = *pEncode;
		pUint8[i++] = 0;
		return i;
	}

	int32 Base64Decode( const int8* map64, uint32 nMapSize, char cMin, char cEnd, 
		void* pDesBuffer, uint32 nOutLen, const char* pBase64Buffer, uint32 nSrcLen )
	{
		if( nSrcLen == 0 )
			return -1;
		if( nSrcLen == INVALID_32BITID )
			nSrcLen = (uint32)strlen( pBase64Buffer );
		if( nSrcLen&0x3 )
			return -1;
		uint32 nNeedOutLen = ( nSrcLen >> 2 )*3;
		if( pBase64Buffer[nSrcLen - 1] == cEnd )
			nNeedOutLen--;
		if( pBase64Buffer[nSrcLen - 2] == cEnd )
			nNeedOutLen--;

		if( nOutLen < nNeedOutLen )
			return -1;

		uint8* pDes = (uint8*)pDesBuffer;
		uint32 nDes = 0;

		for( uint32 i = 0; i < nSrcLen; i += 4 )
		{
			const char* pSrc = pBase64Buffer + i;
			uint32 nValue0 = pSrc[0] - cMin;
			if( nValue0 >= nMapSize || map64[nValue0] == -1 )
				return -1;
			uint32 nValue1 = pSrc[1] - cMin;
			if( nValue1 >= nMapSize || map64[nValue1] == -1 )
				return -1;
			
			int32 nValue = ( map64[nValue0] << 6 )|map64[nValue1];
			pDes[nDes++] = (uint8)( nValue >> 4 );

			if( pSrc[2] == cEnd )
				break;
			uint32 nValue2 = pSrc[2] - cMin;
			if( nValue2 < 0 || nValue2 >= nMapSize || map64[nValue2] == -1 )
				return -1;

			nValue = ( ( nValue&0xf ) << 6 )|map64[nValue2];
			pDes[nDes++] = (uint8)( nValue >> 2 );

			if( pSrc[3] == cEnd )
				break;
			uint32 nValue3 = pSrc[3] - cMin;
			if( nValue3 < 0 || nValue3 >= nMapSize || map64[nValue3] == -1 )
				return -1;

			nValue = ( ( nValue&0x3 ) << 6 )|map64[nValue3];
			pDes[nDes++] = (uint8)nValue;
		}
		return nDes;
	}

	int32 Base64Encode( const char* aryTable64, char cEnd, 
		char* pBase64Buffer, uint32 nOutLen, const void* pSrcBuffer, uint32 nSrcLen )
	{
		if( nOutLen < (uint32)AligenUp( nSrcLen, 3 )*4/3 )
			return -1;

		const uint8* pSrc = (const uint8*)pSrcBuffer;
		uint32 nSrc = 0;
		uint32 nDes = 0;
		while( nSrc < nSrcLen )
		{
			// 剩余的位在下一个编码的高位
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

	GAMMA_COMMON_API int32 Base64Decode( 
		void* pDesBuffer, uint32 nOutLen, const char* pBase64Buffer, uint32 nSrcLen )
	{
		static int8 map64[] =
		{
			62, -1, -1, -1, 63,	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 
			-1, -1, -1, -1, -1, -1,	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
			10, 11, 12, 13, 14,	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 
			-1, -1, -1, -1, -1,	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 
			36, 37, 38, 39, 40,	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
		};

		return Base64Decode( map64, ELEM_COUNT(map64), 
			'+', '=', pDesBuffer, nOutLen, pBase64Buffer, nSrcLen );
	}

	GAMMA_COMMON_API int32 Base64Encode( 
		char* pBase64Buffer, uint32 nOutLen, const void* pSrcBuffer, uint32 nSrcLen )
	{
		static char alphabet64[64] =
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
		return Base64Encode( alphabet64, '=', pBase64Buffer, nOutLen, pSrcBuffer, nSrcLen );
	}

	GAMMA_COMMON_API int32 Base64UrlDecode( 
		void* pDesBuffer, uint32 nOutLen, const char* pBase64Buffer, uint32 nSrcLen )
	{
		static int8 map64[] =
		{
			62, -1, -1, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, 
			-1, -1, -1,	-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 
			12, 13, 14,	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, 
			-1, -1, 63,	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 
			38, 39, 40,	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
		};

		return Base64Decode( map64, ELEM_COUNT(map64), 
			'-', '.', pDesBuffer, nOutLen, pBase64Buffer, nSrcLen );
	}

	GAMMA_COMMON_API int32 Base64UrlEncode( 
		char* pBase64Buffer, uint32 nOutLen, const void* pSrcBuffer, uint32 nSrcLen )
	{
		static char alphabet64[64] =
		{
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
			'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
			'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
			'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
			'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
			'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
			'w', 'x', 'y', 'z', '0', '1', '2', '3',
			'4', '5', '6', '7', '8', '9', '-', '_',
		};
		return Base64Encode( alphabet64, '.', pBase64Buffer, nOutLen, pSrcBuffer, nSrcLen );
	}
}
