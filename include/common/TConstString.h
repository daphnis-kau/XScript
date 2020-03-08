/**@file  		TConstString.h
* @brief		Const string implement
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
* @note			The class can construct a instance from C string by 
*				reference to C string's memory or use new memory.
*/
#ifndef __XS_CONST_STRING_H__
#define __XS_CONST_STRING_H__

#include "common/Help.h"
#include <algorithm>
#include <string>

namespace XS
{
	template<typename _Elem>
	class TConstString
	{
		const _Elem*	m_szString;
		int32			m_nSize;

	public:
		TConstString() : m_szString(NULL), m_nSize(0)
		{
		}

		TConstString( const _Elem* szString, bool bRefrence = false )
			: m_nSize( 0 )
			, m_szString( NULL )
		{
			assign( szString, bRefrence );
		}

		TConstString( const _Elem* szString, uint32 nLen, bool bRefrence = false )
			: m_nSize( nLen )
			, m_szString( NULL )
		{
			assign( szString, nLen, bRefrence );
		}

		TConstString( const TConstString& str )
			: m_nSize( str.m_nSize )
			, m_szString( str.m_szString )
		{
			if( m_nSize >= 0 )
				return;
			++( *( ( (uint32*)( m_szString ) ) - 1 ) );
		}

		~TConstString()
		{
			clear();
		}

		void clear() 
		{
			if( m_nSize >= 0 )
			{
				m_szString = NULL;
				m_nSize = 0;
				return;
			}

			uint32* pRefCount = ( (uint32*)( m_szString ) ) - 1;
			if( --( *pRefCount ) == 0 )
				delete [] pRefCount;
			m_szString = NULL;
			m_nSize = 0;
		}

		void assign( const _Elem* szString = NULL, bool bRefrence = false )
		{
			clear();
			bRefrence = !szString || !szString[0] || bRefrence;
			m_szString = szString && szString[0] ? szString : NULL;
			m_nSize = 0;
			while( m_szString && m_szString[m_nSize] )
				m_nSize++;
			if( bRefrence )
				return;
			uint32 nBufferSize = ( m_nSize + 1 )*sizeof(_Elem) + sizeof(uint32);
			uint32* pRefCount = (uint32*)( new tbyte[ nBufferSize ] );
			memcpy( pRefCount + 1, m_szString, m_nSize*sizeof(_Elem) );
			( (_Elem*)( pRefCount + 1 ) )[m_nSize] = 0;
			m_szString = (_Elem*)( pRefCount + 1 );
			*pRefCount = 1;
			m_nSize = -m_nSize;
		}

		void assign( const _Elem* szString, uint32 nLen, bool bRefrence = false )
		{
			clear();
			bRefrence = !szString || !nLen || bRefrence;
			m_szString = szString && nLen ? szString : NULL;
			m_nSize = m_szString ? nLen : 0;
			if( bRefrence )
				return;
			uint32 nBufferSize = ( m_nSize + 1 )*sizeof(_Elem) + sizeof(uint32);
			uint32* pRefCount = (uint32*)( new tbyte[ nBufferSize ] );
			memcpy( pRefCount + 1, m_szString, m_nSize*sizeof(_Elem) );
			( (_Elem*)( pRefCount + 1 ) )[m_nSize] = 0;
			m_szString = (_Elem*)( pRefCount + 1 );
			*pRefCount = 1;
			m_nSize = -m_nSize;
		}

		const _Elem* c_str() const
		{
			static const _Elem s_Empty[2] = { 0, 0 };
			return m_szString ? m_szString : s_Empty;
		}

		bool empty() const
		{
			return m_szString == NULL || m_szString[0] == 0 || m_nSize == 0;
		}

		uint32 size() const
		{
			return m_nSize < 0 ? -m_nSize : m_nSize;
		}

		const TConstString& operator=( const TConstString& str )
		{
			clear();
			m_nSize = str.m_nSize;
			m_szString = str.m_szString;
			if( m_nSize >= 0 )
				return *this;
			++( *( ( (uint32*)( m_szString ) ) - 1 ) );
			return *this;
		}

		const TConstString& operator=( const _Elem* szString )
		{
			assign( szString );
			return *this;
		}

		bool operator == ( const TConstString& str ) const
		{
			uint32 nLen = size();
			if( nLen != str.size() )
				return false;
			const _Elem* src = c_str();
			const _Elem* dst = str.c_str();
			for( uint32 i = 0; i < nLen; ++i )
				if( src[i] != dst[i] )
					return false;
			return true;
		}

		bool operator==( const _Elem* szString ) const
		{
			const _Elem* src = c_str();
			const _Elem* dst = szString;
			while( ( *src - *dst ) == 0 && *dst )
				++src, ++dst;
			const _Elem* end = src + size();
			return ( ( src < end ? *src : 0 ) - *dst ) == 0;
		}

		bool operator!=( const TConstString& str ) const
		{
			return !( *this == str );
		}

		bool operator!=( const _Elem* szString ) const
		{
			return !( *this == szString );
		}

		bool operator < ( const TConstString& str ) const
		{
			const _Elem* src = c_str();
			const _Elem* dst = str.c_str();
			uint32 srcLen = size();
			uint32 dstLen = str.size();
			uint32 nMinLen = std::min( srcLen, dstLen );
			uint32 i = 0;
			while( i < nMinLen && src[i] == dst[i] )
				++i;
			return ( i < srcLen ? src[i] : 0 ) < ( i < dstLen ? dst[i] : 0 );
		}

		bool operator < ( const _Elem* szString ) const
		{
			const _Elem* src = c_str();
			const _Elem* dst = szString;
			while( ( *src - *dst ) == 0 && *dst )
				++src, ++dst;
			const _Elem* end = src + size();
			return ( ( src < end ? *src : 0 ) - *dst ) < 0;
		}

		bool operator > ( const TConstString& str ) const
		{
			return str < *this;
		}

		bool operator > ( const _Elem* szString ) const
		{
			const _Elem* src = c_str();
			const _Elem* dst = szString;
			while( ( *src - *dst ) == 0 && *dst )
				++src, ++dst;
			const _Elem* end = src + size();
			return ( ( src < end ? *src : 0 ) - *dst ) > 0;
		}

		uint32 find( const _Elem nChar, const uint32 nOffset = 0 ) const
		{
			if ( m_szString == NULL )
				return INVALID_32BITID;
			uint32 nPos = nOffset;
			uint32 nLen = size();
			while ( nPos < nLen && m_szString[nPos] != nChar )
				++nPos;
			return nPos >= nLen ? INVALID_32BITID : nPos;
		}

		uint32 rfind( const _Elem nChar, const uint32 nOffset = 0 ) const
		{
			uint32 nLen = size();
			if ( m_szString == NULL || nOffset >= nLen )
				return INVALID_32BITID;
			int32 nCurPos = (int32)( nLen - nOffset - 1 );
			while ( nCurPos >= 0 && m_szString[nCurPos] != nChar )
				--nCurPos;
			return nCurPos >= 0 ? nCurPos : INVALID_32BITID;
		}
	};

	typedef TConstString<char>		const_string;
	typedef TConstString<wchar_t>	const_wstring;
}

#endif
