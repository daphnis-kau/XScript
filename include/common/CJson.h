/**@file  		CJson.h
* @brief		Read/write Json
* @author		Daphnis Kaw
* @date			2020-01-17
* @version		V1.0
*/
#ifndef _XS_JSON_H_
#define _XS_JSON_H_

#include "common/CommonType.h"
#include "common/TList.h"
#include "common/TTinyList.h"
#include <string>
#include <memory>

namespace XS
{
	class CJson
		: private TList<CJson>
		, private TList<CJson>::CListNode
	{
		typedef std::shared_ptr<std::string> CSharePtr;
		CJson*				m_pParent;
		CSharePtr			m_ptrBuffer;
		const char*			m_szName;
		const char*			m_szContent;
		uint32				m_nContentLen;
		uint32				m_nLevel;
		uint32				m_nChildCount;
		bool				m_bForceString;

		friend class TList<CJson>;

		bool NextNode( size_t& nCurPos, bool bWithName );
		char GetString( size_t& nCurPos, bool bBlankEnd, uint32* pLen );
		bool GetNumber( size_t& nCurPos );
		void OutContent( std::ostream& os ) const;
		void SetLevel( uint32 nLevel );

		bool ParseArray( CSharePtr& DomXmlBuffer, size_t& nCurPos );
		bool ParseObject( CSharePtr& DomXmlBuffer, size_t& nCurPos );
		bool Parse( CSharePtr& DomXmlBuffer, size_t& nCurPos, bool bWithName );
	public:
		CJson( const char* szName = NULL, const char* szContent = NULL );
		CJson( const CJson& rhs );
		~CJson();

		const CJson& operator= ( const CJson& rhs );

		bool Load( const void* szBuffer, uint32 nSize );
		bool Save( std::ostream& os, uint32 nStack = 0 ) const;
		void Clear();

		uint32 GetChildCount() const;
		CJson* AddChild( CJson* pChild, CJson* pBefore = NULL );
		CJson* AddChild( const char* szName, CJson* pBefore = NULL );
		template <typename ValueType>
		CJson* AddChild( const char* szName, ValueType Value, CJson* pBefore = NULL );

		CJson* GetChild( uint32 nChildIndex );
		CJson* GetChild( const char* szChildName );
		CJson* operator[]( uint32 nChildIndex );
		CJson* operator[]( const char* szChildName );
		CJson* GetNext();
		CJson* GetPre();

		const CJson* GetChild( uint32 nChildIndex ) const;
		const CJson* GetChild( const char* szChildName ) const;
		const CJson* operator[]( uint32 nChildIndex ) const;
		const CJson* operator[]( const char* szChildName ) const;
		const CJson* GetNext() const;
		const CJson* GetPre() const;

		const char* GetName() const;
		const char* GetValue() const;
		const char* GetContent() const;
		uint32 GetContentLen() const;
		bool IsForceString() const;
		void ForceString( bool bForce );

		template <typename ValueType>
		ValueType As() const;

		template <typename ValueType>
		ValueType At( const char* szChildName, ValueType eDefaultValue = ValueType() ) const;

		template <typename ValueType>
		ValueType At( uint32 nChildIndex, ValueType eDefaultValue = ValueType() ) const;
	};

	template <typename ValueType>
	inline ValueType CJson::At( const char* szChildName, ValueType eDefaultValue ) const
	{
		const CJson* pChild = GetChild( szChildName );
		return pChild ? pChild->As<ValueType>() : eDefaultValue;
	}

	template <typename ValueType>
	inline ValueType CJson::At( uint32 nChildIndex, ValueType eDefaultValue ) const
	{
		const CJson* pChild = GetChild( nChildIndex );
		return pChild ? pChild->As<ValueType>() : eDefaultValue;
	}

	template<>
	inline char CJson::As<char>() const
	{
		return m_szContent[0];
	}

	template<>
	inline uint8 CJson::As<uint8>() const
	{
		return (uint8)ToInt32( m_szContent );
	}

	template<>
	inline int8 CJson::As<int8>() const
	{
		return (int8)ToInt32( m_szContent );
	}

	template<>
	inline uint16 CJson::As<uint16>() const
	{
		return (uint16)ToInt32( m_szContent );
	}

	template<>
	inline int16 CJson::As<int16>() const
	{
		return (int16)ToInt32( m_szContent );
	}

	template<>
	inline uint32 CJson::As<uint32>() const
	{
		return (uint32)ToInt32( m_szContent );
	}

	template<>
	inline int32 CJson::As<int32>() const
	{
		return ToInt32( m_szContent );
	}

	template<>
	inline float CJson::As<float>() const
	{
		return (float)ToFloat( m_szContent );
	}

	template<>
	inline uint64 CJson::As<uint64>() const
	{
		return (uint64)ToInt64( m_szContent );
	}

	template<>
	inline int64 CJson::As<int64>() const
	{
		return (int64)ToInt64( m_szContent );
	}

	template<>
	inline bool CJson::As<bool>() const
	{
		if(
			(m_szContent[0]=='t' || m_szContent[0]=='T')  &&
			(m_szContent[1]=='r' || m_szContent[1]=='R')  &&
			(m_szContent[2]=='u' || m_szContent[2]=='U')  &&
			(m_szContent[3]=='e' || m_szContent[3]=='E') )
			return true;
		return As<int32>() != 0;
	}

	template<>
	inline std::string CJson::As<std::string>() const
	{
		return GetValue();
	}

	template<>
	inline const char* CJson::As<const char*>() const
	{
		return GetValue();
	}

	template<typename ValueType>
	inline CJson* CJson::AddChild( const char* szName, ValueType Value, CJson* pBefore /*= NULL */ )
	{
		std::stringstream ss; ss << Value;
		return AddChild( szName, ss.str().c_str(), pBefore );
	}

	template<>
	inline CJson*  CJson::AddChild( const char* szName, const char* szValue, CJson* pBefore )
	{
		return AddChild( new CJson( szName, szValue ), pBefore );
	}

	template<>
	inline CJson*  CJson::AddChild( const char* szName, const std::string& strValue, CJson* pBefore )
	{
		return AddChild( szName, strValue.c_str(), pBefore );
	}

	template<>
	inline CJson*  CJson::AddChild( const char* szName, bool bValue, CJson* pBefore )
	{
		return AddChild( szName, bValue ? "true" : "false", pBefore );
	}
}


#endif // _JSON_H_
