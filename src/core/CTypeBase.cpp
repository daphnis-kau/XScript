#include "core/CTypeBase.h"
#include "core/CScriptBase.h"

namespace XS
{
	static const size_t s_aryOrgSize[] =
	{
		0,
		sizeof( char ),
		sizeof( int8 ),
		sizeof( int16 ),
		sizeof( int32 ),
		sizeof( int64 ),
		sizeof( long ),
		sizeof( uint8 ),
		sizeof( uint16 ),
		sizeof( uint32 ),
		sizeof( uint64 ),
		sizeof( ulong ),
		sizeof( wchar_t ),
		sizeof( bool ),
		sizeof( float ),
		sizeof( double ),
		sizeof( const char* ),
		sizeof( const wchar_t* ),
		sizeof( void* )
	};

	static const size_t s_aryAligenSize[] =
	{
		0,
		AligenUp(sizeof(char), sizeof(void*)),
		AligenUp(sizeof(int8), sizeof(void*)),
		AligenUp(sizeof(int16), sizeof(void*)),
		AligenUp(sizeof(int32), sizeof(void*)),
		AligenUp(sizeof(int64), sizeof(void*)),
		AligenUp(sizeof(long), sizeof(void*)),
		AligenUp(sizeof(uint8), sizeof(void*)),
		AligenUp(sizeof(uint16), sizeof(void*)),
		AligenUp(sizeof(uint32), sizeof(void*)),
		AligenUp(sizeof(uint64), sizeof(void*)),
		AligenUp(sizeof(ulong), sizeof(void*)),
		AligenUp(sizeof(wchar_t), sizeof(void*)),
		AligenUp(sizeof(bool), sizeof(void*)),
		AligenUp(sizeof(float), sizeof(void*)),
		AligenUp(sizeof(double), sizeof(void*)),
		AligenUp(sizeof(const char*), sizeof(void*)),
		AligenUp( sizeof( const wchar_t* ), sizeof( void* ) ),
		AligenUp(sizeof(void*), sizeof(void*))
	};

	DataType ToDataType( const STypeInfo& argTypeInfo )
	{
		uint32 n = 5;
		STypeInfo argInfo = argTypeInfo;
		while( n && !( ( argInfo.m_nType >> ( n * 4 ) ) & 0xf ) )
			n--;

		uint32 nPointCount = 0;
		for( uint32 i = 0; i <= n; i++ )
			nPointCount += ( ( argInfo.m_nType >> ( i * 4 ) ) & 0xf ) >= eDTE_Pointer;
		uint32 nType = argInfo.m_nType >> 24;

		if( nPointCount == 0 )
		{
			if( nType < eDT_enum )
				return nType;
			const char* szTypeName = argTypeInfo.m_szTypeName;
			auto pClassInfo = CClassInfo::RegisterClass( 
				"", szTypeName, argTypeInfo.m_nSize, nType == eDT_enum );
			if( !pClassInfo->IsEnum() )
				return (DataType)pClassInfo;
			if( pClassInfo->GetClassSize() == 4 )
				return eDT_int32;
			if( pClassInfo->GetClassSize() == 2 )
				return eDT_int16;
			return eDT_int8;
		}
		else
		{
			// 特别注意，为了处理方便，将eDT_enum类型定义为buffer
			if( nPointCount > 1 || nType != eDT_class )
				return eDT_enum;
			const char* szTypeName = argTypeInfo.m_szTypeName;
			auto pClassInfo = CClassInfo::RegisterClass(
				"", szTypeName, argTypeInfo.m_nSize, nType == eDT_enum );
			if( !pClassInfo->IsEnum() )
				return ( (DataType)pClassInfo ) | 1;
			return eDT_enum;
		}
	}

	bool IsValueClass( DataType nType )
	{
		return nType > eDT_enum && !( nType & 1 );
	}

	size_t GetSizeOfType( DataType nType )
	{
		if( nType <= eDT_enum )
			return s_aryOrgSize[nType];
		if( nType&1 )
			return sizeof( void* );
		return ( (const CClassInfo*)nType )->GetClassSize();
	}

	size_t GetAligenSizeOfType(DataType nType)
	{
		if( nType <= eDT_enum )
			return s_aryAligenSize[nType];
		if (nType & 1)
			return sizeof(void*);
		return ((const CClassInfo*)nType)->GetClassAligenSize();
	}

	size_t CalBufferSize(const DataType* aryParam, size_t nParamCount, size_t arySize[])
	{
		size_t nTotalSize = 0;
		for (size_t i = 0; i < nParamCount; i++)
			nTotalSize += (arySize[i] = GetAligenSizeOfType(aryParam[i]));
		return nTotalSize;
	}
	
	CGlobalTypes::CGlobalTypes(
		CTypeBase* pCharType,
		CTypeBase* pInt8Type,
		CTypeBase* pInt16Type,
		CTypeBase* pInt32Type,
		CTypeBase* pInt64Type,
		CTypeBase* pLongType,
		CTypeBase* pUint8Type,
		CTypeBase* pUint16Type,
		CTypeBase* pUint32Type,
		CTypeBase* pUint64Type,
		CTypeBase* pUlongType,
		CTypeBase* pWCharType,
		CTypeBase* pBoolType,
		CTypeBase* pFloatType,
		CTypeBase* pDoubleType,
		CTypeBase* pStringType,
		CTypeBase* pWStringType,
		CTypeBase* pPointerType,
		CTypeBase* pClassPointType,
		CTypeBase* pClassValueType )
		: m_aryTypes
		{
			pCharType,
			pInt8Type,
			pInt16Type,
			pInt32Type,
			pInt64Type,
			pLongType,
			pUint8Type,
			pUint16Type,
			pUint32Type,
			pUint64Type,
			pUlongType,
			pWCharType,
			pBoolType,
			pFloatType,
			pDoubleType,
			pStringType,
			pWStringType,
			pPointerType,
			pClassPointType,
			pClassValueType
		}
	{
	}

	XS::CTypeBase* CGlobalTypes::GetTypeBase( DataType eType )
	{
		if( eType == eDT_void )
			return nullptr;
		if( eType < eDT_count )
			return m_aryTypes[eType - 1];
		if( eType & 1 )
			return m_aryTypes[eDT_count - 2];
		return m_aryTypes[eDT_count - 1];
	}
}
