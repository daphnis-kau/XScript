
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

namespace XS
{
	//==================================================================
	// 检查是否Utf8字符串
	//==================================================================
	bool IsUtf8( const char* pUtf8, uint32 nLen = -1 );
	int32 GetCharacterCount( const char* pUtf8, uint32 nLen = -1 );

	//========================================================================================
	// 将utf8转为unicode, pBuf为空返回装载Unicode字符的个数, nSize为wchar_t或uint16的个数
	//========================================================================================
	uint32 Utf8ToUcs( wchar_t* pUnicode, uint32 nSize, const char* pUtf8, uint32 nLen = -1 );
	uint32 Utf8ToUcs2( uint16* pUnicode, uint32 nSize, const char* pUtf8, uint32 nLen = -1 );
	const char* GetUnicode( uint32& cUnicode, const char* pUtf8 );

	//==================================================================
	// 将Unicode转为utf8, pBuf为空返回装载char的字符个数
	//==================================================================
	uint32 UcsToUtf8( char* pUtf8, uint32 nSize, const wchar_t* pUnicode, uint32 nLen = -1 );
	uint32 Ucs2ToUtf8( char* pUtf8, uint32 nSize, const uint16* pUnicode, uint32 nLen = -1 );
}

#endif
