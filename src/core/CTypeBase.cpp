#include "core/CTypeBase.h"
#include "core/CScriptBase.h"

namespace Gamma
{

	const int32 s_arySize[eDT_count] =
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
			if( nType != eDT_class )
				return nType;
			const char* szTypeName = argTypeInfo.m_szTypeName;
			auto pClassInfo = CClassRegistInfo::GetRegistInfo( szTypeName );
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
			if( nPointCount > 1 || nType != eDT_class )
				return eDT_class;
			const char* szTypeName = argTypeInfo.m_szTypeName;
			auto pClassInfo = CClassRegistInfo::GetRegistInfo( szTypeName );
			if( !pClassInfo->IsEnum() )
				return ( (DataType)pClassInfo ) | 1;
			return eDT_class;
		}
	}

	size_t GetSizeOfType( DataType nType )
	{
		if( nType < eDT_count )
			return s_arySize[nType];
		if( nType&1 )
			return sizeof( void* );
		return ( (CClassRegistInfo *)nType )->GetClassSize();
	}
}
