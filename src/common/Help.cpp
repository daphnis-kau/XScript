#include "common/Help.h"
#include <stdlib.h>
#include <stdio.h>

#pragma warning(disable:4740)

namespace XS
{

	int32 ToInt32( const wchar_t* szStr )
	{
		if( !szStr )
			return 0;
		char szBuf[256];
		uint32 i = 0;
		while( i < 128 && szStr[i] )
			szBuf[i] = (char)Min<uint32>( szStr[i], 0x7f );
		szBuf[i] = 0;
		return (int32)strtol( szBuf, NULL, 0 );
	}

	int32 ToInt32( const char* szStr )
	{
		if( !szStr )
			return 0;
		return (int32)strtol( szStr, NULL, 0 );
	}

	int64 ToInt64( const wchar_t* szStr )
	{
		if( !szStr )
			return 0;
		char szBuf[256];
		uint32 i = 0;
		while( i < 128 && szStr[i] )
			szBuf[i] = (char)Min<uint32>( szStr[i], 0x7f );
		szBuf[i] = 0;
		return ToInt64( szBuf );
	}

	int64 ToInt64( const char* szStr )
	{
		if( !szStr )
			return 0;
#ifdef _WIN32 
		int64 nVal = _strtoi64( szStr, NULL, 0 );
#else
		int64 nVal = strtoll( szStr, NULL, 0 );
#endif
		return nVal;
	}

	double ToFloat( const wchar_t* szStr )
	{
		char szBuf[256];
		uint32 i = 0;
		while( i < 128 && szStr[i] )
			szBuf[i] = (char)Min<uint32>(szStr[i], 0x7f);
		szBuf[i] = 0;
		return atof( szBuf );
	}

	double ToFloat( const char* szStr )
	{
		return atof( szStr );
	}
}	
