/**@file  		TStrStream.h
* @brief		String stream
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
* @note			The string stream can be created with std::string, C array.
*/
#ifndef __XS_STR_STREAM_H__
#define __XS_STR_STREAM_H__
#include "common/CommonType.h"
#include "common/TConstString.h"
#include <string.h>
#include <ostream>
#include <sstream>
#include <iomanip>

namespace XS
{
	template<class _Elem, class _Traits = std::char_traits<_Elem> >
	class TStrStream
		: private std::basic_streambuf<_Elem>
		, private std::basic_ostream<_Elem, _Traits >
	{
		typedef std::basic_streambuf<_Elem> Parent_t;
		typedef typename Parent_t::int_type int_type;
		typedef typename Parent_t::pos_type pos_type_t;
		typedef typename Parent_t::off_type off_type_t;

		typedef		std::basic_string<_Elem>		_Str;
		size_t		m_nElemCount;
		size_t		m_nCurPos;
		_Elem		m_charTag;		

		union
		{
			_Elem*	m_pBuf;
			_Str*	m_pString;
		};

		const TStrStream& operator=( const TStrStream& );
		TStrStream( const TStrStream& );

		int_type overflow( int_type n )
		{
			Flush( (_Elem*)&n, 1 );
			return n;
		}

		size_t Flush( const _Elem* szBuf, size_t nLen )
		{
			if( !nLen )
				return 0;

			if( m_nCurPos == (size_t)-1 )
			{
				m_pString->append( szBuf, nLen );
				return nLen;
			}
			else if( m_nCurPos < m_nElemCount - 1 )
			{
				size_t nCopyLen = m_nElemCount - 1 - m_nCurPos;
				if( nLen < nCopyLen )
					nCopyLen = nLen;
				memcpy( m_pBuf + m_nCurPos, szBuf, nCopyLen*sizeof(_Elem) );
				m_nCurPos += nCopyLen;
				m_pBuf[m_nCurPos] = 0;
				return nCopyLen;
			}
			else
			{
				return 0;
			}
		}

	public:
		template<class Type>
		struct SValueFormat
		{
			SValueFormat& operator = ( SValueFormat& );
		public:
			SValueFormat( const Type& Val, uint32 flag, size_t nMinCount, _Elem cSpace )
				: m_Val( Val ), m_nFlag( flag ), m_nMinCount( nMinCount ), m_cSpace( cSpace ){}
			const Type&	m_Val;
			uint32		m_nFlag;
			size_t		m_nMinCount;
			_Elem		m_cSpace;
		};

		template<class Type>
		static SValueFormat<Type> GenFormat( const Type& Val, uint32 flag = 0, size_t nMinCount = 0, _Elem cSpace = ' ' )
		{
			return SValueFormat<Type>( Val, flag, nMinCount, cSpace );
		}

		TStrStream( _Str& szString, const _Elem cTag = 0 )
			: std::basic_ostream<_Elem, _Traits >( static_cast<std::basic_streambuf<_Elem>*>( this ) )
			, m_pString( &szString) 
			, m_nElemCount( 0 )
			, m_nCurPos( (size_t)-1 )
			, m_charTag( cTag )
		{
		}

		TStrStream( _Elem* szString, size_t nElemCount, const _Elem cTag = 0 )
			: std::basic_ostream<_Elem, _Traits >( static_cast< std::basic_streambuf<_Elem>* >( this ) )
			, m_pBuf( szString )
			, m_nElemCount( nElemCount )
			, m_nCurPos( 0 )
			, m_charTag( cTag )
		{
			m_pBuf[m_nCurPos] = 0;
		}

		template<size_t nElemCount>
		TStrStream( _Elem(&szString)[nElemCount] )
			: std::basic_ostream<_Elem, _Traits >( static_cast< std::basic_streambuf<_Elem>* >( this ) )
			, m_pBuf( szString )
			, m_nElemCount( nElemCount )
			, m_nCurPos( 0 )
			, m_charTag( 0 )
		{
			m_pBuf[m_nCurPos] = 0;
		}

		template<class Type>
		TStrStream<_Elem>& operator << ( const Type& _Val )
		{
			std::basic_ostream<_Elem, _Traits >& _this = *this;
			_this << _Val;
			if( m_charTag )
				Flush( &m_charTag, sizeof(m_charTag) );
			return *this;
		}

		TStrStream<_Elem>& operator << ( const int8& _Val )
		{
			std::basic_ostream<_Elem, _Traits >& _this = *this;
			_this << (int32)_Val;
			return *this;
		}

		TStrStream<_Elem>& operator << ( const uint8& _Val )
		{
			std::basic_ostream<_Elem, _Traits >& _this = *this;
			_this << (int32)_Val;
			return *this;
		}

		TStrStream<_Elem>& operator << ( std::basic_ostream<_Elem>& (fun)( std::basic_ostream<_Elem>& ) )
		{
			std::basic_ostream<_Elem, _Traits >& _this = *this;
			_this << fun;
			return *this;
		}

		template<class Type>
		TStrStream<_Elem>& operator << ( const SValueFormat<Type>& fmtValue )
		{
			std::basic_ostream<_Elem, _Traits >& _this = *this;
			size_t nPrePos = m_nCurPos == (size_t)-1 ? m_pString->size() : m_nCurPos;

			if( fmtValue.m_nFlag == 0 )
				fmtValue.m_nFlag = std::ios_base::dec|std::ios_base::right;
			std::ios_base::fmtflags fmt = (std::ios_base::fmtflags)( std::ios_base::hex|std::ios_base::oct|std::ios_base::dec );
			std::ios_base::fmtflags pre = (std::ios_base::fmtflags)_this.setf( (std::ios_base::fmtflags)fmtValue.m_nFlag, fmt );
			_this << fmtValue.m_Val;
			_this.setf( pre, fmt );

			size_t nCurPos = m_nCurPos == (size_t)-1 ? m_pString->size() : m_nCurPos;
			size_t nMinCount = fmtValue.m_nMinCount;
			if( nCurPos - nPrePos < nMinCount && ( ( m_nCurPos == (size_t)-1 ) || nPrePos + nMinCount < m_nElemCount - 1 ) )
			{
				_Elem* pPos;
				if( m_nCurPos == (size_t)-1 )
				{
					m_pString->resize( nPrePos + nMinCount );
					pPos = &( (*m_pString)[nPrePos] );
				}
				else
				{
					m_pBuf[nPrePos + nMinCount] = 0;
					pPos = m_pBuf + nPrePos;
				}

				size_t nStart = nCurPos - nPrePos;
				size_t nCount = nMinCount - nStart;
				if( ( fmtValue.m_nFlag&std::ios_base::left ) == 0 )
				{
					memmove( pPos + nCount, pPos, ( nCurPos - nPrePos )*sizeof(_Elem) );
					nStart = 0;
				}

				for( size_t i = 0; i < nCount; i++ )
					pPos[ nStart + i] = fmtValue.m_cSpace;
				m_nCurPos += nCount;
			}

			if( m_charTag )
				Flush( &m_charTag, sizeof(m_charTag) );

			return *this;
		}

		TStrStream<_Elem>& operator << ( const TConstString<_Elem>& _Val )
		{
			const _Elem* szString = _Val.c_str();
			uint32 nSize = _Val.size();
			Flush( szString, nSize );
			if( m_charTag )
				Flush( &m_charTag, sizeof(m_charTag) );
			return *this;
		}

		size_t length()
		{
			return m_nCurPos == (size_t)-1 ? m_pString->length() : m_nCurPos;
		}

		_Elem CharTag() const 
		{
			return m_charTag;
		}

		void CharTag( _Elem val ) 
		{ 
			m_charTag = val;
		}

		const _Elem* GetBuff() const
		{
			return m_nCurPos == (size_t)-1 ? m_pString->c_str() : m_pBuf;
		}
		
		void clear()
		{
			if ( m_nCurPos == (size_t)-1 )
				m_pString->clear();
			else
				m_nCurPos = 0;
		}

		size_t GetCurPos()
		{
			return m_nCurPos;
		}
	};

	typedef TStrStream<char>	char_stream;
	typedef TStrStream<wchar_t>	wchar_stream;
	#define char_stream_f		char_stream::GenFormat
	#define wchar_stream_f		wchar_stream::GenFormat

	/**
	 * example:
	 * char_stream( szElem ) << "0x" << char_stream_f( 26, iostream::hex, 8, '0' ) << endl;
	 * szElem: 0x0000001a
	 * char_stream( szElem ) << "0x" << char_stream_f( 26, iostream::dec, 8, '0' ) << endl;
	 * szElem: 0x00000026
	 * char_stream( szElem ) << "0x" << char_stream_f( 26, iostream::dec|iostream::left, 8, '0' ) << endl;
	 * szElem: 0x26000000
	 */
}

#endif
