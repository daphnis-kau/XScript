//===============================================
// Help.h 
// 定义常用帮助函数  
// 柯达昭  
// 2007-09-07
//===============================================

#ifndef __GAMMA_HELP_H__
#define __GAMMA_HELP_H__

#include "common/CommonType.h"
#include <assert.h>
#include <string>
#include <vector>
#include <utility>
#include <sstream>

#undef SAFE_RELEASE
#undef SAFE_DELETE
#undef SAFE_DEL_GROUP
#undef SUCCEEDED
#undef FAILED
#undef MAKE_DWORD

#define SAFE_RELEASE( p )				{ if( p ){ (p)->Release(); (p) = NULL; } }
#define SAFE_DELETE( p )				{ delete (p); (p) = NULL; }
#define SAFE_DEL_GROUP( p )				{ delete[] (p); (p) = NULL; }
#define ELEM_COUNT( _array )			( (uint32)( sizeof( _array )/sizeof( _array[0] ) ) )

#define INVALID_64BITID		0xffffffffffffffffULL
#define INVALID_32BITID		0xffffffff
#define INVALID_16BITID		0xffff
#define INVALID_8BITID		0xff

#define MAX_INT64			((int64)0x7fffffffffffffffLL)
#define MIN_INT64			((int64)0x8000000000000000LL)
#define MAX_INT32			((int32)0x7fffffff)
#define MIN_INT32			((int32)0x80000000)
#define MAX_INT16			((int16)0x7fff)
#define MIN_INT16			((int16)0x8000)
#define MAX_INT8			((int8)0x7f)
#define MIN_INT8			((int8)0x80)

// 由4个字符组成一个DWORD
#define MAKE_DWORD(ch0, ch1, ch2, ch3)                            \
	((uint32)(uint8)(int8)(ch0) | ((uint32)(uint8)(int8)(ch1) << 8) |       \
	((uint32)(uint8)(int8)(ch2) << 16) | ((uint32)(uint8)(int8)(ch3) << 24 ))

#define MAKE_UINT64( ch0, ch1 ) ((uint64)(uint32)(int32)(ch0) | ((uint64)(uint32)(int32)(ch1) << 32) )
#define MAKE_UINT32( ch0, ch1 ) ((uint32)(uint16)(int16)(ch0) | ((uint32)(uint16)(int16)(ch1) << 16) )
#define MAKE_UINT16( ch0, ch1 ) ((uint16)(uint8)(int8)(ch0) | ((uint16)(uint8)(int8)(ch1) << 8) )

#define HIUINT32(l)	((uint32)((uint64)((int64)(l)) >> 32))
#define LOUINT32(l)	((uint32)((uint64)((int64)(l)) & 0xffffffff))
#define HIUINT16(l)	((uint16)((uint32)((int32)(l)) >> 16))
#define LOUINT16(l)	((uint16)((uint32)((int32)(l)) & 0xffff))
#define HIUINT8(w)	((uint8)((uint16)((int16)(w)) >> 8))
#define LOUINT8(w)	((uint8)((uint16)((int16)(w)) & 0xff))	
#define HIINT32(l)	((int32)((uint64)((int64)(l)) >> 32))
#define LOINT32(l)	((int32)((uint64)((int64)(l)) & 0xffffffff))
#define HIINT16(l)	((int16)((uint32)((int32)(l)) >> 16))
#define LOINT16(l)	((int16)((uint32)((int32)(l)) & 0xffff))
#define HIINT8(w)	((int8)((uint16)((int16)(w)) >> 8))
#define LOINT8(w)	((int8)((uint16)((int16)(w)) & 0xff))

namespace Gamma
{	
	//========================================================================
	// 整数向上对其
	//========================================================================
	inline uint32 AligenUp( uint32 n, uint32 nAligen )
	{
		return n ? ( ( n - 1 )/nAligen + 1 )*nAligen : 0;
	}

	//========================================================================
	// 整数向上对其
	//========================================================================
	inline uint32 AligenDown( uint32 n, uint32 nAligen )
	{
		return ( n / nAligen )*nAligen;
	}

	//========================================================================
	// 整数向上对其
	//========================================================================
	inline uint32 AligenUpTo2Power( uint32 n )
	{
		if( n == 0 )
			return 1;
		uint32 p = 1;
		for( n = n - 1; n; n = n >> 1 )
			p = p << 1;
		return p;
	}

	//========================================================================
	// 整数向上对其
	//========================================================================
	inline uint32 AligenDownTo2Power( uint32 n )
	{
		if( n == 0 )
			return 1;
		uint32 p = 1;
		for( ; n; n = n >> 1 )
			p = p << 1;
		return p >> 1;
	}

	//========================================================================
	// 限制数的上下限 
	// 返回 a>=min && a<= max
	//=========================================================================
	template<class T>
	inline T Limit( const T& a, const T& min, const T& max )
	{
		if( a < min )
			return min;
		else if( a > max )
			return max;

		return a;
	}

	template<class T>
	inline T Min( const T& a, const T& b )
	{
		return a < b ? a : b;
	} 

	template<class T>
	inline T Max( const T& a, const T& b )
	{
		return a > b ? a : b;
	} 

	template<class T>
	inline T GetIntervalIntersect( const T& min1, const T& max1, const T& min2, const T& max2 )
	{
		return Min( max1, max2 ) - Max( min1, min2 );
	}

	template<class T>
	inline T Abs( const T& a )
	{
		return a < 0 ? -a : a;
	} 

	// 常量求最大值  
	template<size_t n1, size_t n2>
	class TMax2
	{
	public:
		enum { eValue = n1 > n2 ? n1 : n2 };
	};

	template<size_t n1, size_t n2, size_t n3>
	class TMax3
	{
	public:
		enum { eValue = TMax2<n1,n2>::eValue > n3 ? TMax2<n1,n2>::eValue : n3 };
	};

	template<size_t n1, size_t n2, size_t n3, size_t n4>
	class TMax4
	{
	public:
		enum { eValue = TMax3<n1,n2,n3>::eValue > n4 ? TMax3<n1,n2,n3>::eValue : n4 };
	};

	template<size_t n1, size_t n2, size_t n3, size_t n4, size_t n5>
	class TMax5
	{
	public:
		enum { eValue = TMax4<n1,n2,n3,n4>::eValue > n5 ? TMax4<n1,n2,n3,n4>::eValue : n5 };
	};

	template<size_t n1, size_t n2, size_t n3, size_t n4, size_t n5, size_t n6>
	class TMax6
	{
	public:
		enum { eValue = TMax5<n1,n2,n3,n4,n5>::eValue > n6 ? TMax5<n1,n2,n3,n4,n5>::eValue : n6 };
	};

	template<size_t n1, size_t n2, size_t n3, size_t n4, size_t n5, size_t n6, size_t n7>
	class TMax7
	{
	public:
		enum { eValue = TMax6<n1,n2,n3,n4,n5,n6>::eValue > n7 ? TMax6<n1,n2,n3,n4,n5,n6>::eValue : n7 };
	};

	template<size_t n1, size_t n2, size_t n3, size_t n4, size_t n5, size_t n6, size_t n7, size_t n8>
	class TMax8
	{
	public:
		enum { eValue = TMax7<n1,n2,n3,n4,n5,n6,n7>::eValue > n8 ? TMax7<n1,n2,n3,n4,n5,n6,n7>::eValue : n8 };
	};

	template<size_t n1, size_t n2, size_t n3, size_t n4, size_t n5, size_t n6, size_t n7, size_t n8, size_t n9>
	class TMax9
	{
	public:
		enum { eValue = TMax8<n1,n2,n3,n4,n5,n6,n7,n8>::eValue > n9 ? TMax8<n1,n2,n3,n4,n5,n6,n7,n8>::eValue : n9 };
	};

	template<size_t n1, size_t n2, size_t n3, size_t n4, size_t n5, size_t n6, size_t n7, size_t n8, size_t n9, size_t n10>
	class TMax10
	{
	public:
		enum { eValue = TMax9<n1,n2,n3,n4,n5,n6,n7,n8,n9>::eValue > n10 ? TMax9<n1,n2,n3,n4,n5,n6,n7,n8,n9>::eValue : n10 };
	};

	template<size_t n1, size_t n2, size_t n3, size_t n4, size_t n5, size_t n6, size_t n7, size_t n8, size_t n9, size_t n10, size_t n11>
	class TMax11
	{
	public:
		enum { eValue = TMax10<n1,n2,n3,n4,n5,n6,n7,n8,n9,n10>::eValue > n11 ? TMax10<n1,n2,n3,n4,n5,n6,n7,n8,n9,n10>::eValue : n11 };
	};
	//========================================================================
	// 求常数的2的幂对齐
	//========================================================================
	template<uint32 n>
	class TAligenUpTo2Power
	{
		template<uint32 v,uint32 l>	struct _A{ enum{ eValue = _A< v*2, l/2 >::eValue }; };
		template<uint32 v>			struct _A<v, 0>{ enum{ eValue = v }; };
	public:
		enum { eValue = _A< 1, n - 1 >::eValue };
	};
	template<> class TAligenUpTo2Power<0> { public: enum { eValue = 0 }; };

	//========================================================================
	// 求常数的向上对齐
	//========================================================================
	template<uint32 n, uint32 nAligen>
	class TAligenUp
	{
	public:
		enum { eValue = n ? ( ( n - 1 )/nAligen + 1 )*nAligen : 0 };
	};

	//========================================================================
	// 求常数的向下对齐
	//========================================================================
	template<uint32 n, uint32 nAligen>
	class TAligenDown
	{
	public:
		enum { eValue = ( n / nAligen )*nAligen };
	};

	//========================================================================
	// 求常数的的对数
	//========================================================================
	template<uint32 n>
	class TLog2
	{
	public:
		enum { eValue = TLog2< n/2 >::eValue + 1 };
	};
	template<> class TLog2<1> { public: enum { eValue = 0 }; };

	//========================================================================
	// 判断是否字母
	//========================================================================
	template<class CharType>
	inline bool IsLetter( CharType c )
	{
		return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' );
	}

	//========================================================================
	// 判断是否文字
	//========================================================================
	template<class CharType>
	inline bool IsWordChar( CharType c )
	{
		return IsLetter( c ) || ( (uint32)c ) > 127;
	}

	//========================================================================
	// 判断是否数字
	//========================================================================
	template<class CharType>
	inline bool IsNumber( CharType c )
	{
		return c >= '0' && c <= '9';
	}

	//========================================================================
	// 16进制数字转为数值
	//========================================================================
	template<class CharType>
	inline int ValueFromHexNumber( CharType c )
	{
		if( c >= '0' && c <= '9' )
			return c - '0';
		if( c >= 'A' && c <= 'F' )
			return c - 'A' + 10;
		if( c >= 'a' && c <= 'f' )
			return c - 'a' + 10;
		return -1;
	}

	//========================================================================
	// 数值转为16进制数字
	//========================================================================
	inline int ValueToHexNumber( int n )
	{
		assert( n >= 0 && n <= 0xf );
		if( n < 10 )
			return '0' + n;
		return n - 10 + 'A';
	}

	//========================================================================
	// 判断是否16进制数字
	//========================================================================
	template<class CharType>
	inline bool IsHexNumber( CharType c )
	{
		return ValueFromHexNumber( c ) >= 0;
	}

	//========================================================================
	// 判断是空格
	//========================================================================
	template<class CharType>
	inline bool IsBlank( CharType c )
	{
		return c == ' ' || c == '\t' || c == '\r' || c == '\n';
	}

	//========================================================================
	// 从文件读入字符串
	//========================================================================
	template< class File, class Fun >
	inline void ReadString( File& fileRead, Fun funRead, std::basic_string<char>& sRead )
	{
		uint32 uStringLen;
		( fileRead.*funRead )( (char*)&uStringLen, sizeof( uint32 ) );
		if( !uStringLen )
			return sRead.clear();
		sRead.resize( uStringLen );
		( fileRead.*funRead )( (char*)&sRead[0], uStringLen*sizeof(char) );
	}

	//========================================================================
	template< class File, class Fun >
	inline void ReadString( File& fileRead, Fun funRead, std::basic_string<wchar_t>& sRead )
	{
		uint32 uStringLen;
		( fileRead.*funRead )( (char*)&uStringLen, sizeof( uint32 ) );
		if( !uStringLen )
			return sRead.clear();
		sRead.resize( uStringLen );
		for( uint32 i = 0; i < uStringLen; i++ )
			( fileRead.*funRead )( (char*)&sRead[i], sizeof(uint16) );
	}

	//========================================================================
	// 将字符串写入文件
	//========================================================================
	template< class File, class Fun >
	inline void WriteString( File& fileWrite, Fun funWrite, const std::basic_string<char>& sWrite )
	{
		uint32 uStringLen = (uint32)sWrite.size();
		( fileWrite.*funWrite )( (const char*)&uStringLen, sizeof( uint32 ) );
		if( !uStringLen )
			return;
		( fileWrite.*funWrite )( (const char*)sWrite.c_str(), uStringLen*sizeof(char) );
	}

	template< class File, class Fun>
	inline void WriteString( File& fileWrite, Fun funWrite, const std::basic_string<wchar_t>& sWrite )
	{
		uint32 uStringLen = (uint32)sWrite.size();
		( fileWrite.*funWrite )( (const char*)&uStringLen, sizeof( uint32 ) );
		if( !uStringLen )
			return;
		for( uint32 i = 0; i < uStringLen; i++ )
			( fileWrite.*funWrite )( (const char*)&sWrite[i], sizeof(uint16) );
	}

	//========================================================================
	// 将字符串进行异或
	//========================================================================
	inline void BufferXor( tbyte* pDes, uint32 nSize, const uint8(&nKey)[16], 
		uint32 nOffset = 0, const tbyte* pSrc = NULL )
	{
		if( !pSrc )
			pSrc = pDes;
		for( uint32 i = 0; i < nSize; i++, nOffset++ )
			pDes[i] = pSrc[i] ^ nKey[ nOffset & 0xF ];
	}

	//========================================================================
	// 将字符串进行异或
	//========================================================================
	template<typename Elem>
	inline bool IsAllBeTheElem( const Elem* pBuffer, size_t nCount, const Elem& e )
	{
		for( size_t i = 0; i < nCount; i++ )
			if( pBuffer[i] != e )
				return false;
		return true;
	}

	//========================================================================
	// 从路径中提取文件名
	//========================================================================
	template<class CharType>
	CharType* GetFileNameFromPath( CharType* szPath )
	{
		size_t nPos = 0;
		for( size_t i = 0; szPath[i]; i++ )
			if( szPath[i] == '/' || szPath[i] == '\\' )
				nPos = i + 1;
		return szPath + nPos;
	}

	//========================================================================
	// 从路径中提取扩展名
	//========================================================================
	template<class CharType>
	const CharType* GetFileExtend( const CharType* szPath )
	{
		uint32 nPos = INVALID_32BITID;
		for( uint32 i = 0; szPath[i]; i++ )
			if( szPath[i] == '.' )
				nPos = i + 1;
		return nPos == INVALID_32BITID ? NULL : szPath + nPos;
	}

	//========================================================================
	// 简化路径
	//========================================================================
	template<class CharType>
	inline uint32 ShortPath( CharType* szPath )
	{
		uint32 nPos[256];
		uint32 nCount = 0;
		uint32 nCurPos = 0;
		uint32 nPrePos = 0;

		for( uint32 i = 0; szPath[i]; i++ )
		{
			if( szPath[i] == '.' && ( szPath[i+1] == '/' || szPath[i+1] == '\\' ) )
			{
				i += 1;
				continue;
			}
			else if( szPath[i] == '.' && szPath[i+1] == '.' && ( szPath[i+2] == '/' || szPath[i+2] == '\\' ) )
			{
				i += 2;
				if( nCount )
				{
					nCurPos = nPos[--nCount];
					nPrePos = nCurPos;
					continue;
				}
				else
				{
					szPath[nCurPos++] = '.';
					szPath[nCurPos++] = '.';
					szPath[nCurPos++] = '/';
					nPrePos = nCurPos;
					continue;
				}
			}

			if( szPath[i] == '/' || szPath[i] == '\\' )
			{
				szPath[nCurPos++] = '/';
				nPos[nCount++] = nPrePos;
				nPrePos = nCurPos;
			}
			else
			{
				szPath[nCurPos++] = szPath[i];
			}
		}

		szPath[nCurPos] = 0;
		return (int32)nCurPos;
	}

	//========================================================================
	// 根据分隔符将字符串拆成字符串组
	//========================================================================

	template< class _Elem >
	inline std::vector< std::basic_string<_Elem> > SeparateString( const _Elem* szSrc, _Elem nSeparator )
	{
		std::vector< std::basic_string<_Elem> > vecStr;

		size_t nSize = 1;
		for( int32 i = 0; szSrc[i]; i++ )
			if( szSrc[i] == nSeparator )
				nSize++;

		vecStr.resize( nSize );
		int32 nPreItem = 0;
		for( int32 i = 0, n = 0; ; i++ )
		{
			if( szSrc[i] == nSeparator )
			{
				vecStr[n++].assign( szSrc + nPreItem, i - nPreItem );
				nPreItem = i + 1;
			}
			else if( szSrc[i] == 0 )
			{
				vecStr[n++].assign( szSrc + nPreItem, i - nPreItem );
				break;
			}
		}
		return vecStr;
	}

	GAMMA_COMMON_API int32 GammaA2I( const wchar_t* szStr );
	GAMMA_COMMON_API int32 GammaA2I( const char* szStr );
	GAMMA_COMMON_API int64 GammaA2I64( const wchar_t* szStr );
	GAMMA_COMMON_API int64 GammaA2I64( const char* szStr );
	GAMMA_COMMON_API double GammaA2F( const wchar_t* szStr );
	GAMMA_COMMON_API double GammaA2F( const char* szStr );

    template< class _CharType, class _intType >
    inline std::vector<_intType> SeparateStringToIntArray( const _CharType* szSrc, _CharType nSeparator )
    {        
        std::vector< _intType > results;
		_CharType szBuffer[64];		
		uint32 n = 0;
		bool bDot = false;
        while( *szSrc )
        {
			if( *szSrc != nSeparator )
			{
				if( n < ELEM_COUNT( szBuffer ) - 1 )
					szBuffer[n++] = *szSrc;
				bDot = bDot || *szSrc == '.';
			}
			else
			{
				szBuffer[n] = 0;       
				if( bDot )
					results.push_back( (_intType)GammaA2F( szBuffer ) );
				else
					results.push_back( (_intType)GammaA2I64( szBuffer ) );
				bDot = false;
				n = 0;
			} 
			szSrc++;
		}
		szBuffer[n] = 0;       
		if( bDot )
			results.push_back( (_intType)GammaA2F( szBuffer ) );
		else
			results.push_back( (_intType)GammaA2I64( szBuffer ) );
        return results;
	}

	template< class _CharType, class _SepType, class _IntType >
	size_t SeparateStringFast( _CharType* szSrc, _SepType nSeparator,
		std::pair<_CharType*, _IntType>* vecResult, size_t nMaxSize )
	{        
		size_t nCount = 0;
		int32 nPreItem = 0;
		for( int32 i = 0; ; i++ )
		{
			if( nCount >= nMaxSize )
				return nCount;
			if( szSrc[i] == nSeparator )
			{
				vecResult[nCount].first = szSrc + nPreItem;
				vecResult[nCount].second = (_IntType)( i - nPreItem );
				nCount++;
				nPreItem = i + 1;
			}
			else if( szSrc[i] == 0 )
			{
				vecResult[nCount].first = szSrc + nPreItem;
				vecResult[nCount].second = (_IntType)( i - nPreItem );
				nCount++;
				break;
			}
		}
		return nCount;
	}

	//========================================================================
	// 将字符串组根据分隔符合成字符串
	//========================================================================
	template<class VecStr, class _CharType>
	inline std::basic_string<_CharType> CombinationString( const VecStr& vecStr, _CharType nSeparator )
	{
		std::basic_string<_CharType> strTemp;
		for( typename VecStr::const_iterator it = vecStr.begin(); it != vecStr.end(); it++ )
		{
			if( it != vecStr.begin() )
				strTemp += nSeparator;
			strTemp += *it;
		}
		return strTemp;
	}

    /** 去除字符串首尾的引号
    */
    template< class CharType >
    inline void TrimRoundQuotes( std::basic_string<CharType>& s )
    {
        if ( s.empty() )
            return;

        if ( s[0] == (CharType)'\"' )
            s.erase(0, 1 );
        
        if ( !s.empty() )
        {
            size_t nLastIndex = s.length() - 1;
            if ( s[ nLastIndex ] == (CharType)'\"' )
                s.erase( nLastIndex, 1 );
        }
    }

	inline size_t UrlEncode( const char* szUtf8, size_t nSize, char* szOut, size_t nMaxSize )
	{
		if ( !szUtf8 )
			return 0;

		size_t nEncodeLen = 0;
		for ( size_t i = 0; i < nSize && szUtf8[i]; i++ )
		{
			unsigned char c = szUtf8[i];
			if ( IsNumber( c ) || IsLetter( c ) )
			{
				if( nEncodeLen >= nMaxSize )
					return nEncodeLen;
				szOut[nEncodeLen++] = c;
			}
			else
			{
				if( nEncodeLen + 2 >= nMaxSize )
					return nEncodeLen;
				const char* szX = "0123456789ABCDEF";
				szOut[nEncodeLen++] = '%';
				szOut[nEncodeLen++] = szX[ c >> 4 ];
				szOut[nEncodeLen++] = szX[ c & 0x0f ];
			}
		}

		return nEncodeLen;
	}

	//========================================================================
	// 检查标志是否存在
	//========================================================================
	inline bool IsHasFlag( uint32 nStyle, uint32 nFlag )
	{
		return (nStyle&nFlag) != 0; 
	}

	//========================================================================
	// 设定标志
	//========================================================================
	inline uint32 SetFlag( uint32 nStyle, uint32 nFlag, int32 bValue )
	{
		if( bValue )
			return nStyle|nFlag;
		return nStyle&(~nFlag);
	}

	//=========================================================================
	// 以\n为分隔符，从流中获取一个字符串
	//=========================================================================
	template<class stream_t, class _Elem>
	inline void GetString( stream_t& istream, std::basic_string<_Elem>& str )
	{
		_Elem n = (_Elem)istream.get();
		while( n == '\n' && istream )
			n = istream.get();
		while( n != '\n' && istream )
		{
			str.push_back( n );
			n = istream.get();
		}
	}

	//==========================================================================
	// 将数组求和
	//==========================================================================
	template<typename T1, typename T2>
	inline T2 Sum( const T1* pArray, size_t nSize )
	{
		T2 n = 0;
		for( size_t i = 0; i < nSize; i++ )
			n += pArray[i];
		return n;
	}

	template<class _Type>
	_Type tolower( _Type c ){ return c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c; }
	template<class _Type>
	_Type toupper( _Type c ){ return c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c; }

	template<class _Type>
	int32 stricmp( const _Type* src, const _Type* dst )
	{
		int ret = 0 ;

		while( 0 == ( ret = tolower(*src) - tolower(*dst) ) && *dst )
			++src, ++dst;

		if ( ret < 0 )
			ret = -1 ;
		else if ( ret > 0 )
			ret = 1 ;

		return ret;
	}

	template<class _Type>
	int32 strnicmp( const _Type* src, const _Type* dst, size_t nMaxLen )
	{
		int ret = 0;
		size_t n = 0;
		while( n < nMaxLen && 0 == ( ret = tolower(*src) - tolower(*dst) ) && *dst )
			++src, ++dst, ++n;

		if ( ret < 0 )
			ret = -1 ;
		else if ( ret > 0 )
			ret = 1 ;

		return ret;
	}

	template<class _Type>
	void strlwr( _Type* src )
	{
		while( *src )
			*src = (_Type)tolower(*src),++src;
	}

	template<typename _Type>
	uint32 strcpy_safe( _Type* pDes, const _Type* pSrc, uint32 nSize, uint32 nMaxSrcLen )
	{		
		assert( nSize );
		if( !pSrc )
		{
			pDes[0] = 0;
			return 0;
		}

		uint32 i = 0;
		--nSize;
		while( i < nSize && i < nMaxSrcLen && *pSrc )
			pDes[i++] = *pSrc++;
		pDes[i] = 0;
		return i;
	}

	template<typename _Type>
	uint32 strcat_safe( _Type* pDes, const _Type* pSrc, uint32 nSize, uint32 nMaxSrcLen )
	{		
		if( !pSrc )
			return 0;
		assert( nSize );
		uint32 i = 0;
		--nSize;
		while( i < nSize && pDes[i] )
			i++;

		uint32 j = 0;
		while( i < nSize && j < nMaxSrcLen && pSrc[j] )
			pDes[i++] = pSrc[j++];
		pDes[i] = 0;
		return i;
	}


	template<typename _Type>
	int32 strcmp_safe( const _Type* pDes, const _Type* pSrc, uint32 nSize, uint32 nMaxSrcLen )
	{ 
		if( !pSrc && !pDes )
			return 0;
		if( !pDes )
			return -1;
		if( !pSrc )
			return 1;
		uint32 nCount = Min( nSize, nMaxSrcLen );
		for( uint32 i = 0; i < nCount; ++i )
		{
			if( pDes[i] != pSrc[i] )
				return (int32)pDes[i] - (int32)pSrc[i];
			if( !pDes[i] )
				return 0;
		}

		int32 nDesEnd = nCount == nSize ? 0 : pDes[nCount];
		int32 nSrcEnd = nCount == nMaxSrcLen ? 0 : pSrc[nCount];
		return nDesEnd - nSrcEnd;
	}

	template<typename _Type, uint32 n> 
	uint32 strcpy2array_safe( _Type(&array_pointer)[n], const _Type* pSrc )
	{ 
		return strcpy_safe( array_pointer, pSrc, n, INVALID_32BITID );
	}

	template<typename _Type, uint32 n> 
	uint32 strcat2array_safe( _Type(&array_pointer)[n], const _Type* pSrc )
	{ 
		return strcat_safe( array_pointer, pSrc, n, INVALID_32BITID );
	}

	template<typename _Type, uint32 n> 
	int32 strcmp2array_safe( const _Type(&array_pointer)[n], const _Type* pSrc )
	{ 
		return strcmp_safe( array_pointer, pSrc, n, INVALID_32BITID );
	}

	template<typename _Type, uint32 n> 
	uint32 strncpy2array_safe( _Type(&array_pointer)[n], const _Type* pSrc, uint32 nMaxSrcLen )
	{ 
		return strcpy_safe( array_pointer, pSrc, n, nMaxSrcLen );
	}

	template<typename _Type, uint32 n> 
	uint32 strncat2array_safe( _Type(&array_pointer)[n], const _Type* pSrc, uint32 nMaxSrcLen )
	{ 
		return strcat_safe( array_pointer, pSrc, n, nMaxSrcLen );
	}

	template<typename _Type, uint32 n> 
	int32 strncmp2array_safe( const _Type(&array_pointer)[n], const _Type* pSrc, uint32 nMaxSrcLen )
	{ 
		return strcmp_safe( array_pointer, pSrc, n, nMaxSrcLen );
	}

}
#endif
