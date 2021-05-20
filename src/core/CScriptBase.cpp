#include <sstream>
#include "common/Help.h"
#include "common/Memory.h"
#include "core/CScriptBase.h"
#include "core/CCallInfo.h"
#include "core/CDebugBase.h"

namespace XS
{
	struct SFunctionTableHead
	{
		CScriptBase*			m_pScript;
		SFunctionTable*			m_pOldFunTable;
		const CClassInfo*	m_pClassInfo;
	};

	struct SFileContext
	{
		uint32					m_nCacheSize;
		char*					m_pFile;
	};

	enum{ eFunctionTableHeadSize = sizeof(SFunctionTableHead)  };
	enum{ ePointerCount = eFunctionTableHeadSize/sizeof(void*) };
	enum{ eFunctionTableHeadAligSize = ePointerCount*sizeof(void*) };
	#if( eFunctionTableHeadAligSize != eFunctionTableHeadSize )
	#error "CScriptBase::SFunctionTableHead size invalid"
	#endif
	
	std::string CScriptBase::s_CacheTruckPrefix = "CacheTruck_";

	//==================================================================
	// 虚函数分配
	//==================================================================
	#define MAX_VIRTUAL_FUN_COUNT	( 1024*1024 )
	#define RESERVED_SIZE			( sizeof(void*)*MAX_VIRTUAL_FUN_COUNT )

	static void** s_aryFuctionTable = (void**)ReserveMemoryPage( nullptr, RESERVED_SIZE );
	static void** s_aryFuctionTableEnd = s_aryFuctionTable;

	static SFunctionTableHead* AllocFunArray( size_t nArraySize )
	{
		static std::mutex s_Lock;
		static uint32 s_nFuctionTableUseCount = 0;
		static uint32 s_nFuctionTableCommitCount = 0;

		s_Lock.lock();
		nArraySize += ePointerCount;
		uint32 nUseCount = s_nFuctionTableUseCount + (uint32)nArraySize;
		if( nUseCount > s_nFuctionTableCommitCount )
		{
			if( nUseCount > MAX_VIRTUAL_FUN_COUNT )
			{
				s_Lock.unlock();
				throw( "No enough buffer for funtion table!!!!" );
			}

			uint32 nPageSize = GetVirtualPageSize();
			uint32 nPageFuctionCount = nPageSize/sizeof(void*);
			assert( nPageFuctionCount*sizeof(void*) == nPageSize );

			void* pCommitStart = s_aryFuctionTable + s_nFuctionTableCommitCount;
			uint32 nCommitEnd = AligenUp( nUseCount, nPageFuctionCount );
			uint32 nCommitCount = nCommitEnd - s_nFuctionTableCommitCount;
			uint32 nCommitFlag = VIRTUAL_PAGE_READ|VIRTUAL_PAGE_WRITE;
			CommitMemoryPage( pCommitStart, nCommitCount*sizeof(void*), nCommitFlag );
			s_nFuctionTableCommitCount = nCommitEnd;
			s_aryFuctionTableEnd = s_aryFuctionTable + nCommitEnd;
		}

		void** aryFun = s_aryFuctionTable + s_nFuctionTableUseCount;
		s_nFuctionTableUseCount += (uint32)nArraySize;
		s_Lock.unlock();
		return (SFunctionTableHead*)aryFun;
	}

	static bool IsAllocVirtualTable( void* pVirtualTable )
	{
		return pVirtualTable >= s_aryFuctionTable && pVirtualTable < s_aryFuctionTableEnd;
	}

	//==================================================================
	// 虚拟机列表
	//==================================================================
    CScriptBase::CScriptBase(void)
        : m_pDebugger( nullptr )
	{
    }

    CScriptBase::~CScriptBase(void)
	{
		SAFE_DELETE( m_pDebugger );

		// 虚表不释放，这里的内存泄漏是故意的
		for( CFunctionTableMap::iterator it = m_mapVirtualTableOld2New.begin(); 
			it != m_mapVirtualTableOld2New.end(); ++it )
		{
			SFunctionTable* pNewFunTable = it->second;
			SFunctionTableHead* pFunTableHead = ( (SFunctionTableHead*)pNewFunTable ) - 1;
			pFunTableHead->m_pClassInfo = nullptr;
			int32 nFunCount = pNewFunTable->GetFunctionCount();
			assert( it->first == pFunTableHead->m_pOldFunTable );
			memcpy( pNewFunTable->m_pFun, it->first->m_pFun, nFunCount*sizeof(void*) );
		}
	}

	bool CScriptBase::RegisterGlobalFunction( IFunctionWrap* funWrap, uintptr_t funOrg,
		const STypeInfoArray& aryTypeInfo, const char* szTypeInfoName, const char* szFunctionName )
	{
		ECallingType eCallingType = szTypeInfoName && szTypeInfoName[0] ?
			eCT_ClassStaticFunction : eCT_GlobalFunction;
		return new CCallInfo( funWrap, aryTypeInfo, funOrg,
			szTypeInfoName, eCallingType, szFunctionName ) != nullptr;
	}

	bool CScriptBase::RegisterClassFunction( IFunctionWrap* funWrap, uintptr_t funOrg,
		const STypeInfoArray& aryTypeInfo, const char* szFunctionName )
	{
		return new CCallInfo( funWrap, aryTypeInfo, funOrg,
			aryTypeInfo.aryInfo[0].m_szTypeName, eCT_ClassFunction, szFunctionName ) != nullptr;
	}

	bool CScriptBase::RegisterClassCallback( 
		IFunctionWrap* funWrap, uintptr_t funBoot, uint32 nFunIndex, bool bPureVirtual,
		const STypeInfoArray& aryTypeInfo, const char* szFunctionName )
	{
		return new CCallbackInfo( funWrap, aryTypeInfo, funBoot, nFunIndex, 
			bPureVirtual, aryTypeInfo.aryInfo[0].m_szTypeName, szFunctionName ) != nullptr;
	}

	bool CScriptBase::RegisterClassMember( IFunctionWrap* funGetSet[2], uintptr_t nOffset,
		const STypeInfoArray& aryTypeInfo, const char* szMemberName )
	{
		assert( funGetSet && ( funGetSet[0] || funGetSet[1] ) );
		const_string keyName( szMemberName, true );
		const char* szTypeInfoName = aryTypeInfo.aryInfo[0].m_szTypeName;
		assert( CClassInfo::GetClassInfo( szTypeInfoName )->
			GetRegistFunction().Find( keyName ) == nullptr );
		return new CMemberInfo( funGetSet, aryTypeInfo, nOffset,
			szTypeInfoName, szMemberName ) != nullptr;
	}

	bool CScriptBase::RegisterClass( const char* szClass, uint32 nCount, 
		const char** aryType, const ptrdiff_t* aryValue )
	{
		auto pClassInfo = CClassInfo::RegisterClass( 
			szClass, aryType[0], (uint32)aryValue[0], false );
		for( uint32 i = 1; i < nCount; i++ )
		{
			assert( CClassInfo::GetClassInfo( aryType[i] ) != nullptr );
			pClassInfo->AddBaseInfo( aryType[0], aryType[i], aryValue[i] );
		}
		return pClassInfo != nullptr;
	}

	bool CScriptBase::RegisterConstruct( IObjectConstruct* pObjectConstruct, const char* szTypeIDName )
	{
		assert( CClassInfo::GetClassInfo( szTypeIDName ) );
		CClassInfo::SetObjectConstruct( szTypeIDName, pObjectConstruct );
		return true;
	}

	bool CScriptBase::RegisterDestructor( IFunctionWrap* funWrap,
		uintptr_t funBoot, uint32 nFunIndex, const STypeInfoArray& aryTypeInfo )
	{
		return new CCallbackInfo( funWrap, aryTypeInfo,funBoot, nFunIndex, 
			false, aryTypeInfo.aryInfo[0].m_szTypeName, "" ) != nullptr;
	}

	bool CScriptBase::RegisterEnumType( const char* szTypeIDName, const char* szEnumType, int32 nTypeSize )
	{
		return CClassInfo::RegisterClass( szEnumType, szTypeIDName, nTypeSize, true ) != nullptr;
	}

	bool CScriptBase::RegisterEnumValue( const char* szTypeIDName, const char* szEnumValue, int32 nValue )
	{
		const CClassInfo* pClassInfo = CClassInfo::GetClassInfo( szTypeIDName );
		const_string keyName( szEnumValue, true );
		assert( pClassInfo && pClassInfo->GetRegistFunction().Find( keyName ) == nullptr );
		STypeInfo TypeInfo = GetTypeInfo<int32>(); 
		STypeInfoArray aryInfo = { &TypeInfo, 1 };
		return new CCallInfo( nullptr, aryInfo, nValue, szTypeIDName, eCT_Value, szEnumValue ) != nullptr;
	}

	void CScriptBase::CheckDebugCmd()
	{
		if( !m_pDebugger || !m_pDebugger->RemoteCmdValid() )
			return;
		m_pDebugger->CheckEnterRemoteDebug();
	}

	bool CScriptBase::IsVirtualTableValid( SVirtualObj* pVObj )
	{
		if( !IsAllocVirtualTable( pVObj->m_pTable ) )
			return true;
		SFunctionTableHead* pFunTableHead = ( (SFunctionTableHead*)pVObj->m_pTable ) - 1;
		return pFunTableHead->m_pScript == this;
	}

    SFunctionTable* CScriptBase::GetOrgVirtualTable( void* pObj )
    {
		// 寻找pObj对象的原始虚表，如果pObj的虚表已经被修改过
		// 那么，应该会在m_mapVirtualTableNew2Old中找到相应的虚表
		// 否则说明pObj的虚表没有被修改过，直接返回他自己的虚表
        SVirtualObj* pVObj = (SVirtualObj*)pObj;
		if( !IsAllocVirtualTable( pVObj->m_pTable ) )
			return pVObj->m_pTable;
		SFunctionTableHead* pFunTableHead = ( (SFunctionTableHead*)pVObj->m_pTable ) - 1;
		return pFunTableHead->m_pOldFunTable;
    }

    SFunctionTable* CScriptBase::CheckNewVirtualTable( SFunctionTable* pOldFunTable, 
		const CClassInfo* pClassInfo, bool bNewByVM, uint32 nInheritDepth )
	{
		assert( !IsAllocVirtualTable( pOldFunTable ) );

		// 如果是由脚本建立的类，不需要进行类型提升，   
		// 也就是不需要通过原始的虚表匹配继承树上更深一层的虚表  
		// 这时可以直接使用CClassRegistInfo上对应的虚表  
		// 另外，由脚本建立的类被也不能使用原始的虚表来匹配新的虚表，  
		// 原因是因为这个类可能是纯虚类，在某些编译器下，可能会因为优化
		// 使得不同的纯虚类使用了相同的虚表，导致虚表相互覆盖
		if( bNewByVM )
		{
			CVMObjVTableInfo& VMObjectVTableInfo = m_mapNewVirtualTable[pClassInfo];
			if( VMObjectVTableInfo.first && VMObjectVTableInfo.second <= nInheritDepth )
				return VMObjectVTableInfo.first;

			VMObjectVTableInfo.second = nInheritDepth;
			int32 nFunCount = pOldFunTable->GetFunctionCount();	
			if( VMObjectVTableInfo.first == nullptr )
			{
				SFunctionTableHead* pFunTableHead = AllocFunArray( nFunCount + 1 );
				VMObjectVTableInfo.first = (SFunctionTable*)( pFunTableHead + 1 );
			}

			SFunctionTable* pNewFunTable = VMObjectVTableInfo.first;
			SFunctionTableHead* pFunTableHead = ( (SFunctionTableHead*)pNewFunTable ) - 1;
			memcpy( pNewFunTable->m_pFun, pOldFunTable->m_pFun, nFunCount*sizeof(void*) );
			pNewFunTable->m_pFun[nFunCount] = nullptr;
			pFunTableHead->m_pScript = this;
			pFunTableHead->m_pOldFunTable = pOldFunTable;
			pFunTableHead->m_pClassInfo = pClassInfo;
			pClassInfo->InitVirtualTable( pNewFunTable );
			return pNewFunTable;
		}

		CFunctionTableMap::iterator it = m_mapVirtualTableOld2New.lower_bound( pOldFunTable );

		if( it == m_mapVirtualTableOld2New.end() || it->first != pOldFunTable )
		{
			int32 nFunCount = pOldFunTable->GetFunctionCount();			
			if( it != m_mapVirtualTableOld2New.end() && 
				(void**)it->first < (void**)pOldFunTable + nFunCount )
				nFunCount = (int32)(ptrdiff_t)( (void**)it->first - (void**)pOldFunTable );

			SFunctionTableHead* pFunTableHead = AllocFunArray( nFunCount + 1 );
			SFunctionTable* pNewFunTable = (SFunctionTable*)( pFunTableHead + 1 );
			m_mapVirtualTableOld2New.insert( std::make_pair( pOldFunTable, pNewFunTable ) );
			memcpy( pNewFunTable->m_pFun, pOldFunTable->m_pFun, nFunCount*sizeof(void*) );
			pNewFunTable->m_pFun[nFunCount] = nullptr;
			pFunTableHead->m_pScript = this;
			pFunTableHead->m_pOldFunTable = pOldFunTable;
			pFunTableHead->m_pClassInfo = pClassInfo;
			pClassInfo->InitVirtualTable( pNewFunTable );
			return pNewFunTable;
		}
		else if( static_cast<const CClassInfo*>( it->second->m_pFun[-1] )
			->GetInheritDepth() < pClassInfo->GetInheritDepth() )
		{
			pClassInfo->InitVirtualTable( it->second );
		}

        return it->second;
	}

    void CScriptBase::AddSearchPath( const char* szPath )
	{
		m_listSearchPath.push_back( szPath );
		auto& strPath = *m_listSearchPath.rbegin();
		for( uint32 i = 0; i < strPath.size(); i++ )
		{
			if( strPath[i] != '\\' )
				continue;
			strPath[i] = '/';
		}

		if( *strPath.rbegin() == '/' )
			return;
		strPath.push_back( '/' );
	}

	int CScriptBase::Input( char* szBuffer, int nCount )
	{
		for( int32 i = 0; i < nCount - 1; i++ )
		{
			std::cin.read( &szBuffer[i], 1 );
			if( szBuffer[i] != '\n' )
				continue;
			szBuffer[i] = 0;
			return i;
		}
		return nCount;
	}

	int CScriptBase::Output( const char* szBuffer, int nCount )
	{
		std::cout << szBuffer;
		return nCount;
	}

	void* CScriptBase::OpenFile( const char* szFileName )
	{
		size_t nNameLen = strlen( szFileName );
		size_t nKeyLen = s_CacheTruckPrefix.size();
		if( nNameLen > s_CacheTruckPrefix.size() &&
			!memcmp( szFileName, s_CacheTruckPrefix.c_str(), nKeyLen ) )
		{
			uintptr_t address = 0;
			std::stringstream( szFileName + nKeyLen ) >> address;
			if( !address )
				return nullptr;
			SFileContext* pContext = new SFileContext;
			pContext->m_nCacheSize = (uint32)strlen( (const char*)address );
			pContext->m_pFile = (char*)address;
			return pContext;
		}

		FILE* fp = fopen( szFileName, "rb" );
		if( nullptr == fp )
			return nullptr;
		SFileContext* pContext = new SFileContext;
		pContext->m_nCacheSize = INVALID_32BITID;
		pContext->m_pFile = (char*)fp;
		return pContext;
	}

	int32 CScriptBase::ReadFile( void* pContext, char* szBuffer, int32 nCount )
	{
		if( !pContext )
			return -1;
		SFileContext* pFileContext = (SFileContext*)pContext;
		if( pFileContext->m_nCacheSize == INVALID_32BITID )
			return (int32)fread( szBuffer, 1, nCount, (FILE*)( pFileContext->m_pFile ) );
		if( pFileContext->m_nCacheSize == 0 )
			return 0;
		if( (uint32)nCount > pFileContext->m_nCacheSize )
			nCount = pFileContext->m_nCacheSize;
		memcpy( szBuffer, pFileContext->m_pFile, nCount );
		pFileContext->m_pFile += nCount;
		pFileContext->m_nCacheSize -= nCount;
		return nCount;
	}

	void CScriptBase::CloseFile( void* pContext )
	{
		if (!pContext)
			return;
		SFileContext* pFileContext = (SFileContext*)pContext;
		if (pFileContext->m_nCacheSize == INVALID_32BITID)
			fclose((FILE*)(pFileContext->m_pFile));
		delete pFileContext;
	}

	bool CScriptBase::RunFile( const char* szFileName, bool bForce/* = false */)
	{
		CheckDebugCmd();
		if( !szFileName )
			return false;

		if( szFileName[0] == '/' || ::strchr( szFileName, ':' ) )
		{
			std::string strFileContent = ReadEntirFile( szFileName );
			if( strFileContent.empty() )
				return false;
			if( !RunBuffer( strFileContent.c_str(), strFileContent.size(), szFileName, bForce) )
				return false;
			if( GetDebugger() && GetDebugger()->RemoteDebugEnable() )
				GetDebugger()->AddFileContent( szFileName, "" );
			return true;
		}

		for( auto it = m_listSearchPath.begin(); it != m_listSearchPath.end(); ++it )
		{
			std::string sFileName = *it + szFileName;
			ShortPath( &sFileName[0] );
			std::string strFileContent = ReadEntirFile( sFileName.c_str() );
			if( strFileContent.empty() )
				continue;
			if( !RunBuffer( strFileContent.c_str(), strFileContent.size(), sFileName.c_str() ) )
				return false;
			if( GetDebugger() )
				GetDebugger()->AddFileContent( sFileName.c_str(), "" );
			return true;
		}
		return false;
	}

	bool CScriptBase::RunString( const char* szString )
	{
		CheckDebugCmd();

		const_string strKey( szString, true );
		auto itPre = m_setRuningString.find( strKey );
		if( itPre == m_setRuningString.end() )
			itPre = m_setRuningString.insert( szString ).first;

		std::stringstream name;
		name << s_CacheTruckPrefix << (uintptr_t)(void*)( itPre->c_str() );
		std::string strName = name.str();
		const char* szName = strName.c_str();
		if( !RunBuffer( itPre->c_str(), itPre->size(), szName, true ) )
			return false;
		if( GetDebugger() )
			GetDebugger()->AddFileContent( szName, szString );
		return true;
	}

	void CScriptBase::CallBack( int32 nIndex, void* pRetBuf, void** pArgArray )
	{
		SVirtualObj* pVirtualObj = *(SVirtualObj**)pArgArray[0];
		assert( IsAllocVirtualTable( pVirtualObj->m_pTable ) );
		SFunctionTableHead* pFunTableHead = ( (SFunctionTableHead*)pVirtualObj->m_pTable ) - 1;
		assert( pFunTableHead->m_pClassInfo && pFunTableHead->m_pOldFunTable );
		const CClassInfo* pClassInfo = pFunTableHead->m_pClassInfo;
		const CCallbackInfo* pCallScript = pClassInfo->GetOverridableFunction( nIndex );
		CScriptBase* pScriptBase = pFunTableHead->m_pScript;
		pScriptBase->CheckDebugCmd();
		auto& strFunctionName = pCallScript->GetFunctionName();
		try
		{
			if( strFunctionName.empty() )
			{
				pScriptBase->DestrucVM( pCallScript, pVirtualObj );
				pCallScript->Destruc( pVirtualObj, pArgArray[1], *pFunTableHead->m_pScript );
			}
			else
			{
				if( pScriptBase->CallVM( pCallScript, pRetBuf, pArgArray ) )
					return;
				return pCallScript->Call( pRetBuf, pArgArray, *pFunTableHead->m_pScript );
			}
		}
		catch( ... )
		{
			const char* szClass = pFunTableHead->m_pClassInfo->GetClassName().c_str();
			pScriptBase->Output( "Unknown Error while call VM with ", -1 );
			pScriptBase->Output( strFunctionName.c_str(), -1 ); 
			pScriptBase->Output( "in ", -1 );
			pScriptBase->Output( szClass, -1 );
			pScriptBase->Output( "\n", -1 );
		}
	}
}
