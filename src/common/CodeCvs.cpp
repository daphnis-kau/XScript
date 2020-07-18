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

namespace XS
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
}
