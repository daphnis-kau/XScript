#include "common/CJson.h"
#include "common/CodeCvs.h"
#include <stdlib.h>
#include <memory.h>

namespace XS
{
	static char s_nArrayFlag  = 0;
	static char s_nObjectFlag = 0;

	CJson::CJson( const char* szName, const char* szContent ) 
		: m_pParent( nullptr )
		, m_szName( nullptr )
		, m_szContent( nullptr )
		, m_nLevel( 0 )
		, m_nChildCount( 0 )
		, m_nContentLen( 0 )
		, m_bForceString( false )
	{
		if( szName )
		{
			m_ptrBuffer = CSharePtr( new std::string );
			*m_ptrBuffer = szName;
			if( szContent && szContent[0] )
			{
				m_ptrBuffer->push_back( 0 );
				size_t nLen = m_ptrBuffer->size();
				m_ptrBuffer->append( szContent );
				m_szName = m_ptrBuffer->c_str();
				m_szContent = m_szName + nLen;
				m_nContentLen = (uint32)( m_ptrBuffer->size() - nLen );
			}
			else
			{
				m_szName = m_ptrBuffer->c_str();
				m_szContent = szContent ? "" : nullptr;
			}
		}
	}

	CJson::CJson( const CJson& rhs )
		: m_pParent( nullptr )
		, m_szName( nullptr )
		, m_szContent( nullptr )
		, m_nLevel( 0 )
		, m_nChildCount( 0 )
		, m_nContentLen( 0 )
		, m_bForceString( false )
	{
		*this = rhs;
	}

	CJson::~CJson()
	{
		Clear();
	}

	void CJson::Clear()
	{
		while( TList<CJson>::GetFirst() )
			delete TList<CJson>::GetFirst();

		if( m_pParent )
		{
			--m_pParent->m_nChildCount;
			m_pParent = nullptr;
		}

		m_ptrBuffer = nullptr;
	}

	bool CJson::Load( const void* pBuffer, uint32 nCount )
	{
		Clear();

		m_ptrBuffer = CSharePtr( new std::string );
		if( (int32)nCount < 0 )
			m_ptrBuffer->assign( (const char*)pBuffer );
		else
			m_ptrBuffer->assign( (const char*)pBuffer, nCount );

		size_t nCurPos = 0;
		if( !NextNode( nCurPos, false ) )
			return false;
		return Parse( m_ptrBuffer, nCurPos, false );
	}

	bool CJson::Save( std::ostream& os, uint32 nStack ) const
	{
		char szBuff[1024];

		if( nStack != INVALID_32BITID )
		{
			for( uint32 i = 0; i < nStack; ++i )
				szBuff[i] = '\t';
			szBuff[nStack] = 0;
			os << szBuff;
		}

		const char* szName = GetName();
		if( szName && szName[0] )
		{
			os << '\"';
			os << GetName();
			os << "\":";
		}

		const char* szContent = m_szContent;
		if( szContent == &s_nArrayFlag )
			os << "[";
		else if( szContent == &s_nObjectFlag )
			os << "{";
		else
			OutContent( os );

		uint32 nCount = 0;
		for( const CJson* pChild = GetChild( (uint32)0 ); 
			pChild; pChild = pChild->GetNext(), ++nCount )
		{
			if( nStack != INVALID_32BITID )
			{
				os << ( nCount ? ",\n" : "\n" );
				pChild->Save( os, nStack + 1 );
			}
			else
			{
				os << ( nCount ? "," : "" );
				pChild->Save( os, INVALID_32BITID );
			}
		}

		if( nStack != INVALID_32BITID )
		{
			if( szContent == &s_nArrayFlag )
				os << "\n" << szBuff << "]";
			else if( szContent == &s_nObjectFlag )
				os << "\n" << szBuff << "}";
		}
		else
		{
			if( szContent == &s_nArrayFlag )
				os << "]";
			else if( szContent == &s_nObjectFlag )
				os << "}";
		}

		if( nStack == 0 )
			os << std::endl;
		os.flush();
		return true;
	}

	void CJson::OutContent( std::ostream& os ) const
	{
		const char* szContent = m_szContent;
		uint32 nLen = m_nContentLen;
		if( nLen == 0 )
		{
			os << ( ( szContent || m_bForceString ) ? "\"\"" : "null" );
			return;
		}

		if( nLen == 4 && !memcmp( szContent, "true", nLen ) )
		{
			if( m_bForceString )
				os << "\"true\"";
			else
				os << "true";
			return;
		}

		if( nLen == 5 && !memcmp( szContent, "false", nLen ) )
		{
			if( m_bForceString )
				os << "\"false\"";
			else
				os << "false";
			return;
		}

		if( nLen < 30 )
		{
			char szBuffer[32];
			uint32 i = 0;
			for( ; i < nLen && IsNumber( szContent[i] ); i++ )
				szBuffer[i] = szContent[i];
			szBuffer[i] = 0;
			if( i == nLen )
			{
				if( m_bForceString )
					os << "\"" << szBuffer << "\"";
				else
					os << szBuffer;
				return;
			}
		}
		
		os << '\"';
		char c[32];
		uint32 u = 0;
		const char* szNext = nullptr;
		for( uint32 i = 0; i < nLen; i++ )
		{
			c[0] = szContent[i];
			const char* szCur = szContent + i;
			const char* szOut = c;
			if( c[0]  == '\\' )
				szOut = "\\\\";
			else if( c[0]  == '\"' )
				szOut = "\\\"";
			else if( c[0]  == '\'' )
				szOut = "\\\'";
			else if( c[0]  == '\?' )
				szOut = "\\\?";
			else if( c[0]  == '\a' )
				szOut = "\\a";
			else if( c[0]  == '\b' )
				szOut = "\\b";
			else if( c[0]  == '\f' )
				szOut = "\\f";
			else if( c[0]  == '\n' )
				szOut = "\\n";
			else if( c[0]  == '\r' )
				szOut = "\\r";
			else if( c[0]  == '\t' )
				szOut = "\\t";
			else if( c[0]  == '\v' )
				szOut = "\\v";
			else if( c[0]  == 0 )
				szOut = "\\0";
			else if( ( ( szNext = GetUnicode( u, szCur ) ) != nullptr ) && 
				szNext != szCur + 1 )
			{
				i += (uint32)( szNext - szContent - 1 );
#ifdef _WIN32
					sprintf_s( c, ELEM_COUNT( c ) - 1, "\\u%x", u );
#else
					sprintf( c, "\\u%x", u );
#endif
			}
			else
				c[1] = 0;
			os << szOut;
		}
		os << '\"';
	}

	bool CJson::NextNode( size_t& nCurPos, bool bWithName )
	{
		size_t nSize = m_ptrBuffer->size();
		char* szBuffer = &( (*m_ptrBuffer)[0] );

		// Find start position of the next object 
		while( nCurPos < nSize )
		{
			if( bWithName )
			{
				while( szBuffer[nCurPos] != '\"' &&
					!IsWordChar( szBuffer[nCurPos] ) &&
					szBuffer[nCurPos] != '}' &&
					szBuffer[nCurPos] != ']' )
				{
					szBuffer[nCurPos] = 0;
					if( ++nCurPos >= nSize )
						return false;
				}
			}
			else
			{
				while( szBuffer[nCurPos] != '\"' &&
					szBuffer[nCurPos] != '{' &&
					szBuffer[nCurPos] != '}' &&
					szBuffer[nCurPos] != '[' &&
					szBuffer[nCurPos] != ']' &&
					!IsNumber( szBuffer[nCurPos] ) )
				{
					szBuffer[nCurPos] = 0;
					if( ++nCurPos >= nSize )
						return false;
				}
			}

			// Object or array ended
			if( ( szBuffer[nCurPos] == '}' && m_szContent == &s_nObjectFlag ) ||
				( szBuffer[nCurPos] == ']' && m_szContent == &s_nArrayFlag ) )
				return false;

			// Next object is found
			return true;
		}

		return false;
	}
	
	char CJson::GetString( size_t& nCurPos, bool bBlankEnd, uint32* pLen )
	{
		char* szBuffer = &( (*m_ptrBuffer)[0] );
		size_t nStrPos = nCurPos;
		size_t nStrStart = nStrPos;
		if( pLen ) *pLen = 0;

		while( szBuffer[nCurPos] )
		{
			char c = szBuffer[nCurPos++];
			szBuffer[nStrPos] = 0;
			if( pLen )
				*pLen = (uint32)( nStrPos - nStrStart );

			if( !bBlankEnd && c == '\"' )
				return c;

			if( bBlankEnd && ( IsBlank( c ) || c == ':' ) )
				return c;

			if( c == '\\' )
			{
				c = szBuffer[nCurPos++];
				if( !c )
					return 0;
				if( c == 'a' )
					c = '\a';
				else if( c == 'b' )
					c = '\b';
				else if( c == 'f' )
					c = '\f';
				else if( c == 'n' )
					c = '\n';
				else if( c == 'r' )
					c = '\r';
				else if( c == 't' )
					c = '\t';
				else if( c == 'v' )
					c = '\v';
				else if( c == 'u' )
				{
					char szNum[] = { szBuffer[nCurPos], szBuffer[nCurPos + 1], 
						szBuffer[nCurPos + 2], szBuffer[nCurPos + 3], 0 };
					wchar_t nChar = (uint16)strtol( szNum, nullptr, 16 );
					nStrPos += UcsToUtf8( szBuffer + nStrPos, 4, &nChar, 1 );
					nCurPos += 4;
					c = szBuffer[--nStrPos];
				}
				else if( c == 'x' )
				{
					char szNum[] = { szBuffer[nCurPos], szBuffer[nCurPos + 1], 0 };
					c = (char)strtol( szNum, nullptr, 16 );
					nCurPos += 2;
				}
				else if( IsNumber( c ) && 
					IsNumber( szBuffer[nCurPos] ) && 
					IsNumber( szBuffer[nCurPos + 1] ) )
				{
					char szNum[] = { c, szBuffer[nCurPos], szBuffer[nCurPos + 1], 0 };
					c = (char)strtol( szNum, nullptr, 8 );
					nCurPos += 2;
				}
				else if( c == '0' )
				{
					c = 0;
				}
				// \\ \' \" \? no more convertion needed
			}
			szBuffer[nStrPos++] = c;
		}

		return 0;
	}

	bool CJson::GetNumber( size_t& nCurPos )
	{
		char* szBuffer = &( (*m_ptrBuffer)[0] );
		char c = szBuffer[nCurPos];
		while( c )
		{
			if( !IsNumber( c ) && !IsHexNumber( c ) && 
				c != 'x' && c != '-' && c != '+' && c != '.' )
				return true;
			c = szBuffer[++nCurPos];
		}

		return false;
	}

	bool CJson::Parse( CSharePtr& DomXmlBuffer, size_t& nCurPos, bool bWithName )
	{
		m_ptrBuffer = DomXmlBuffer;
		size_t nSize = m_ptrBuffer->size();
		char* szBuffer = &( (*m_ptrBuffer)[0] );

		// This is a string, maybe name or content
		if( bWithName )
		{
			bool bBlankEnd = szBuffer[nCurPos] != '\"';
			if( !bBlankEnd )
				++nCurPos;
			size_t nStringStart = nCurPos;
			char cEndChar = GetString( nCurPos, bBlankEnd, nullptr );
			if( !cEndChar )
				return false;

			while( cEndChar != ':' )
			{
				if( nCurPos >= nSize )
					return false;
				cEndChar = szBuffer[nCurPos++];
			}

			while( IsBlank( szBuffer[nCurPos] ) )
				nCurPos++;
			m_szName = szBuffer + nStringStart;
		}

		if( !memcmp( szBuffer + nCurPos, "null", 4 ) )
		{
			m_szContent = nullptr;
			m_nContentLen = 0;
			nCurPos += 4;
			return true;
		}

		char c = szBuffer[nCurPos++];
		if( c == '\"' )
		{
			m_bForceString = true;
			m_szContent = szBuffer + nCurPos;
			return GetString( nCurPos, false, &m_nContentLen ) != 0;
		}

		if( c == 't' && !memcmp( szBuffer + nCurPos, "rue", 3 ) )
		{
			m_szContent = szBuffer + nCurPos - 1;
			m_nContentLen = 4;
			nCurPos += 3;
			return true;
		}

		if( c == 'f' && !memcmp( szBuffer + nCurPos, "alse", 4 ) )
		{
			m_szContent = szBuffer + nCurPos - 1;
			m_nContentLen = 5;
			nCurPos += 4;
			return true;
		}
		
		// This is a number
		if( IsNumber( c ) || c == '-' || c == '+' )
		{
			m_szContent = szBuffer + ( --nCurPos );
			if( !GetNumber( nCurPos ) )
				return false;
			m_nContentLen = (uint32)( szBuffer + nCurPos - m_szContent );
			return true;
		}

		bool bArray = c == '[';
		m_szContent = c == '[' ? &s_nArrayFlag : &s_nObjectFlag;
		m_nContentLen = 0;

		while( NextNode( nCurPos, c != '[' ) )
		{
			CJson* pChild = new CJson;
			TList<CJson>::PushBack( *pChild );
			m_nChildCount++;
			pChild->m_pParent = this;
			pChild->m_nLevel = m_nLevel + 1;
			if( !pChild->Parse( DomXmlBuffer, nCurPos, !bArray ) )
				return false;
		}

		while( nCurPos < nSize )
		{
			char c = szBuffer[nCurPos];
			szBuffer[nCurPos++] = 0;
			if( ( m_szContent == &s_nArrayFlag && c == ']' ) ||
				( m_szContent == &s_nObjectFlag && c == '}') )
				return true;
		}

		return false;
	}

	uint32 CJson::GetChildCount() const
	{
		return m_nChildCount;
	}

	CJson* CJson::GetChild( uint32 nChildIndex )
	{
		CJson* pChild = TList<CJson>::GetFirst();
		while( pChild && nChildIndex-- )
			pChild = pChild->GetNext();
		return pChild;
	}

	CJson* CJson::GetChild( const char* szChildName )
	{
		if( !szChildName || !szChildName[0] )
			return nullptr;
		CJson* pChild = TList<CJson>::GetFirst();
		while( pChild && strcmp( pChild->GetName(), szChildName ) )
			pChild = pChild->GetNext();
		return pChild;
	}

	CJson* CJson::operator[]( uint32 nChildIndex )
	{
		CJson* pChild = TList<CJson>::GetFirst();
		while( pChild && nChildIndex-- )
			pChild = pChild->GetNext();
		return pChild;
	}

	CJson* CJson::operator[]( const char* szChildName )
	{
		if( !szChildName || !szChildName[0] )
			return nullptr;
		CJson* pChild = TList<CJson>::GetFirst();
		while( pChild && strcmp( pChild->GetName(), szChildName ) )
			pChild = pChild->GetNext();
		return pChild;
	}

	const CJson* CJson::GetChild( uint32 nChildIndex ) const
	{
		CJson* pChild = TList<CJson>::GetFirst();
		while( pChild && nChildIndex-- )
			pChild = pChild->GetNext();
		return pChild;
	}

	const CJson* CJson::GetChild( const char* szChildName ) const
	{
		if( !szChildName || !szChildName[0] )
			return nullptr;
		CJson* pChild = TList<CJson>::GetFirst();
		while( pChild && strcmp( pChild->GetName(), szChildName ) )
			pChild = pChild->GetNext();
		return pChild;
	}

	const CJson* CJson::operator[]( uint32 nChildIndex ) const
	{
		CJson* pChild = TList<CJson>::GetFirst();
		while( pChild && nChildIndex-- )
			pChild = pChild->GetNext();
		return pChild;
	}

	const CJson* CJson::operator[]( const char* szChildName ) const
	{
		if( !szChildName || !szChildName[0] )
			return nullptr;
		CJson* pChild = TList<CJson>::GetFirst();
		while( pChild && strcmp( pChild->GetName(), szChildName ) )
			pChild = pChild->GetNext();
		return pChild;
	}

	CJson* CJson::GetNext() 
	{
		return TList<CJson>::CListNode::GetNext();
	}

	CJson* CJson::GetPre() 
	{
		return TList<CJson>::CListNode::GetPre();
	}

	const CJson* CJson::GetNext() const
	{
		return TList<CJson>::CListNode::GetNext();
	}

	const CJson* CJson::GetPre() const
	{
		return TList<CJson>::CListNode::GetPre();
	}

	void CJson::SetLevel( uint32 nLevel )
	{
		m_nLevel = nLevel;
		CJson* pChild = TList<CJson>::GetFirst();
		while( pChild )
		{
			pChild->SetLevel( m_nLevel + 1 );
			pChild = pChild->GetNext();
		}
	}

	CJson* CJson::AddChild( CJson* pChild, CJson* pBefore )
	{
		if( m_szContent != &s_nArrayFlag && m_szContent != &s_nObjectFlag )
			m_szContent = pChild->GetName()[0] ? &s_nObjectFlag : &s_nArrayFlag;
		assert( ( m_szContent == &s_nArrayFlag ) == !( pChild->GetName()[0] ) );
		TList<CJson>::InsertBefore( *pChild, pBefore );
		pChild->SetLevel( m_nLevel + 1 );
		return pChild;
	}

	CJson* CJson::AddChild( const char* szName, CJson* pBefore /*= nullptr */ )
	{
		return AddChild( new CJson( szName ), pBefore );
	}

	bool CJson::IsForceString() const
	{
		return m_bForceString;
	}

	void CJson::ForceString( bool bForce )
	{
		m_bForceString = bForce;
	}

	const char* CJson::GetName() const
	{
		return m_szName ? m_szName : "";
	}

	const char* CJson::GetValue() const
	{
		return m_szContent ? m_szContent : "";
	}

	const char* CJson::GetContent() const
	{
		return m_szContent;
	}

	uint32 CJson::GetContentLen() const
	{
		return m_nContentLen;
	}

	const CJson& CJson::operator=(const CJson& rhs)
	{
		while( TList<CJson>::GetFirst() )
			delete TList<CJson>::GetFirst();

		m_ptrBuffer = rhs.m_ptrBuffer;
		m_szName = rhs.m_szName;
		m_szContent = rhs.m_szContent;
		m_nContentLen = rhs.m_nContentLen;
		m_nChildCount = rhs.m_nChildCount;
		CJson* pChild = rhs.TList<CJson>::GetFirst();
		while( pChild )
		{
			CJson* pNewChild = new CJson;
			pNewChild->m_nLevel = m_nLevel + 1;
			TList<CJson>::PushBack( *pNewChild );
			*pNewChild = *pChild;
			pChild = pChild->GetNext();
		}
		return *this;
	}

}
