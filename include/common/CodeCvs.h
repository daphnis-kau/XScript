
//===============================================
// GammaCodeCvs.h 
// 定义定义字符串编码转换
// 柯达昭
// 2007-09-15
//===============================================

#ifndef __GAMMA_CODE_COVS_H__
#define __GAMMA_CODE_COVS_H__

#include "common/CommonType.h"
#include <string>
#include <string.h>

namespace Gamma
{
	//==================================================================
	// 检查是否Utf8字符串
	//==================================================================
	GAMMA_COMMON_API bool IsUtf8( const char* pUtf8, uint32 nLen = -1 );
	GAMMA_COMMON_API int32 GetCharacterCount( const char* pUtf8, uint32 nLen = -1 );

	//========================================================================================
	// 将utf8转为unicode, pBuf为空返回装载Unicode字符的个数, nSize为wchar_t或uint16的个数
	//========================================================================================
	GAMMA_COMMON_API uint32 Utf8ToUcs( wchar_t* pUnicode, uint32 nSize, const char* pUtf8, uint32 nLen = -1 );
	GAMMA_COMMON_API uint32 Utf8ToUcs2( uint16* pUnicode, uint32 nSize, const char* pUtf8, uint32 nLen = -1 );
	GAMMA_COMMON_API const char* GetUnicode( uint32& cUnicode, const char* pUtf8 );

	inline const std::wstring Utf8ToUcs( const char* pUtf8, uint32 nLen = -1 )
	{
		std::wstring sTemp;
		if( !pUtf8 )
			return sTemp;
		if( nLen == -1 )
			nLen = (uint32)strlen( pUtf8 );
		sTemp.resize( nLen );
		Utf8ToUcs( &sTemp[0], (uint32)sTemp.size(), pUtf8, nLen );
		return sTemp.c_str();
	}

	inline std::wstring Utf8ToUcs( const std::string& s )
	{
		return Utf8ToUcs( s.c_str(), (uint32)s.size() );
	}

	//==================================================================
	// 将Unicode转为utf8, pBuf为空返回装载char的字符个数
	//==================================================================
	GAMMA_COMMON_API uint32 UcsToUtf8( char* pUtf8, uint32 nSize, const wchar_t* pUnicode, uint32 nLen = -1 );
	GAMMA_COMMON_API uint32 Ucs2ToUtf8( char* pUtf8, uint32 nSize, const uint16* pUnicode, uint32 nLen = -1 );

	inline const std::string UcsToUtf8( const wchar_t* pUnicode, uint32 nLen = -1 )
	{
		std::string sTemp;
		if( !pUnicode )
			return sTemp;
		if( nLen == -1 )
			nLen = (uint32)wcslen( pUnicode );
		sTemp.resize( nLen*3 );
		UcsToUtf8( &sTemp[0], (uint32)sTemp.size(), pUnicode, nLen );
		return sTemp.c_str();
	}

	inline std::string UcsToUtf8( const std::wstring& s )
	{
		return UcsToUtf8( s.c_str(), (uint32)s.size() );
	}

	//==================================================================
	// 将Uint8转为Base16, pBuf为空返回装载char的字符个数
	//==================================================================
	GAMMA_COMMON_API uint32 Uint82Base16( const uint8* pUint8, char* pBase16, uint32 sizeBuf );

	//======================================================================
	// 转换wchar_t字符串为Uint8格式
	//======================================================================
	inline std::string Conv2Utf8String( const wchar_t* szStr )
	{
		return UcsToUtf8( szStr );
	}

	//==================================================================
	// URLEncode
	//==================================================================
	GAMMA_COMMON_API uint32 URLEncode( const uint8* pUint8, char* pEncode, uint32 sizeBuf );

	inline const std::string URLEncode( const uint8* pUint8 )
	{
		std::string sTemp;
		if( !pUint8 )
			return sTemp;
		sTemp.resize( strlen( (const char*)pUint8 ) * 3 + 1 );
		URLEncode( pUint8, &sTemp[0], (uint32)sTemp.size() );
		return sTemp.c_str();
	}

	//==================================================================
	// URLDecode
	//==================================================================
	GAMMA_COMMON_API uint32 URLDecode( const char* pEncode, uint8* pUint8, uint32 sizeBuf );

	inline const std::string URLDecode( const char* pEncode )
	{
		std::string sTemp;
		if( !pEncode )
			return sTemp;
		sTemp.resize( strlen( pEncode ) );
		URLDecode( pEncode, (uint8*)&sTemp[0], (uint32)sTemp.size() );
		return sTemp.c_str();
	}

	//==================================================================
	// Base64Encode
	//==================================================================
	GAMMA_COMMON_API int32 Base64Encode( char* pBase64Buffer, 
		uint32 nOutLen, const void* pSrcBuffer, uint32 nSrcLen );

	//==================================================================
	// Base64Decode
	//==================================================================
	GAMMA_COMMON_API int32 Base64Decode( void* pDesBuffer, 
		uint32 nOutLen, const char* pBase64Buffer, uint32 nSrcLen );

	//==================================================================
	// Base64UrlEncode +变为- /变为_ =变为.
	//==================================================================
	GAMMA_COMMON_API int32 Base64UrlEncode( char* pBase64Buffer, 
		uint32 nOutLen, const void* pSrcBuffer, uint32 nSrcLen );

	//==================================================================
	// Base64UrlDecode
	//==================================================================
	GAMMA_COMMON_API int32 Base64UrlDecode( void* pDesBuffer, 
		uint32 nOutLen, const char* pBase64Buffer, uint32 nSrcLen );

	//========================================================================
	// 计算utf8字符串中文字个数
	//========================================================================
	inline uint32 WordsCount( const char* szStr )
	{
		uint32 nSize = 0;
		for( uint8* pBuf = (uint8*)szStr; *pBuf; nSize++ )
		{        
			if( ( ( pBuf[0] )&0x80 ) == 0 )
				pBuf++;
			else if( ( ( pBuf[0] )&0xE0 ) == 0xC0 )
				pBuf += 2;
			else
				pBuf += 3;
		}
		return nSize;
	}

	inline uint32 WordsCount( const wchar_t* szStr )
	{
		uint32 n = 0;
		for( ; *szStr; n++ )
			szStr++;
		return n;
	}

}

#endif
