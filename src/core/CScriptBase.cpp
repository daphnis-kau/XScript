#include <stdlib.h>
#include "common/Help.h"
#include "common/CThread.h"
#include "common/Memory.h"
#include "core/CScriptBase.h"
#include "core/CCallBase.h"
#include "core/CDebugBase.h"

namespace Gamma
{
	struct SFunctionTableHead
	{
		CScriptBase*			m_pScript;
		SFunctionTable*			m_pOldFunTable;
		const CClassRegistInfo*	m_pClassInfo;
	};

	enum{ eFunctionTableHeadSize = sizeof(SFunctionTableHead)  };
	enum{ ePointerCount = eFunctionTableHeadSize/sizeof(void*) };
	enum{ eFunctionTableHeadAligSize = ePointerCount*sizeof(void*) };
	#if( eFunctionTableHeadAligSize != eFunctionTableHeadSize )
	#error "CScriptBase::SFunctionTableHead size invalid"
	#endif

	//==================================================================
	// 虚函数分配
	//==================================================================
	#define MAX_VIRTUAL_FUN_COUNT	( 1024*1024 )
	#define RESERVED_SIZE			( sizeof(void*)*MAX_VIRTUAL_FUN_COUNT )

	static void** s_aryFuctionTable = (void**)ReserveMemoryPage( NULL, RESERVED_SIZE );
	static void** s_aryFuctionTableEnd = s_aryFuctionTable;

	SFunctionTableHead* AllocFunArray( size_t nArraySize )
	{
		static CLock s_Lock;
		static uint32 s_nFuctionTableUseCount = 0;
		static uint32 s_nFuctionTableCommitCount = 0;

		s_Lock.Lock();
		nArraySize += ePointerCount;
		uint32 nUseCount = s_nFuctionTableUseCount + (uint32)nArraySize;
		if( nUseCount > s_nFuctionTableCommitCount )
		{
			if( nUseCount > MAX_VIRTUAL_FUN_COUNT )
			{
				s_Lock.Unlock();
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
		s_Lock.Unlock();
		return (SFunctionTableHead*)aryFun;
	}

	//==================================================================
	// 虚拟机列表
	//==================================================================
	static TList<CScriptBase> s_listAllScript;
	static CLock s_ScriptListLock;

    CScriptBase::CScriptBase(void)
        : m_pDebugger( NULL )
	{
		s_ScriptListLock.Lock();
		s_listAllScript.PushBack( *this );
		s_ScriptListLock.Unlock();
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
			pFunTableHead->m_pClassInfo = NULL;
			int32 nFunCount = pNewFunTable->GetFunctionCount();
			assert( it->first == pFunTableHead->m_pOldFunTable );
			memcpy( pNewFunTable->m_pFun, it->first->m_pFun, nFunCount*sizeof(void*) );
		}

		s_ScriptListLock.Lock();
		TList<CScriptBase>::CListNode::Remove();
		s_ScriptListLock.Unlock();
	}

	bool CScriptBase::RegistFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName )
	{
		return new CByScriptBase( aryTypeInfo, funWrap, "", eCT_GlobalFunction, szFunctionName ) != nullptr;
	}

	bool CScriptBase::RegistClassStaticFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName )
	{
		return new CByScriptBase( aryTypeInfo, funWrap, szTypeInfoName, eCT_ClassStaticFunction, szFunctionName ) != nullptr;
	}

	bool CScriptBase::RegistClassFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName )
	{
		return new CByScriptBase( aryTypeInfo, funWrap, szTypeInfoName, eCT_ClassFunction, szFunctionName ) != nullptr;
	}

	ICallBackWrap& CScriptBase::RegistClassCallback( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName )
	{
		return *new CCallScriptBase( aryTypeInfo, funWrap, szTypeInfoName, szFunctionName );
	}

	bool CScriptBase::RegistClassMember( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funGetSet[2], const char* szTypeInfoName, const char* szMemberName )
	{
		assert( funGetSet && ( funGetSet[0] || funGetSet[1] ) );
		const CClassRegistInfo* pInfo = CClassRegistInfo::GetRegistInfo( szTypeInfoName );
		const CCallBaseMap& mapRegisterFun = pInfo->GetRegistFunction();
		gammacstring keyName( szMemberName, true );
		assert( mapRegisterFun.Find( keyName ) == nullptr );
		return new CByScriptMember( aryTypeInfo, funGetSet, szTypeInfoName, szMemberName ) != nullptr;
	}

	bool CScriptBase::RegistClass( const char* szClass, uint32 nCount, const char** aryType, const ptrdiff_t* aryValue )
	{
		auto pClassInfo = CClassRegistInfo::RegisterClass( szClass, aryType[0], (uint32)aryValue[0], false );
		for( uint32 i = 1; i < nCount; i++ )
		{
			assert( CClassRegistInfo::GetRegistInfo( aryType[i] ) != NULL );
			pClassInfo->AddBaseRegist( aryType[0], aryType[i], aryValue[i] );
		}
		return pClassInfo != nullptr;
	}

	bool CScriptBase::RegistConstruct( IObjectConstruct* pObjectConstruct, const char* szTypeIDName )
	{
		assert( CClassRegistInfo::GetRegistInfo( szTypeIDName ) );
		CClassRegistInfo::SetObjectConstruct( szTypeIDName, pObjectConstruct );
		return true;
	}

	ICallBackWrap& CScriptBase::RegistDestructor( const char* szTypeInfoName, IFunctionWrap* funWrap )
	{
		STypeInfo aryInfo[2];
		aryInfo[0].m_nType = ( eDT_custom_type << 24 )|eDTE_Pointer;
		aryInfo[0].m_szTypeName = szTypeInfoName;
		aryInfo[1].m_nType = eDT_void;
		aryInfo[1].m_szTypeName = typeid( void ).name();
		STypeInfoArray aryTypeInfo = { aryInfo, 2 };
		return *( new CCallScriptBase( aryTypeInfo, funWrap, szTypeInfoName, "" ) );
	}

	bool CScriptBase::RegistEnum( const char* szTypeIDName, const char* szEnumName, int32 nTypeSize )
	{
		return CClassRegistInfo::RegisterClass( szEnumName, szTypeIDName, nTypeSize, true ) != nullptr;
	}

	void CScriptBase::CheckUnlinkCppObj()
	{
		void* pObject[1024];
		auto funRead = [&]( CCircelBuffer::CAtomRead& Reader )->bool
		{ return Reader.Read( pObject, sizeof( pObject ) );	};

		uint32 nReadSize = 0;
		while( ( nReadSize = m_UnlinkObjectBuffer.PopBuffer( funRead ) ) != 0 )
		{
			for( uint32 i = 0; i < nReadSize/sizeof( void* ); i++ )
				UnlinkCppObjFromScript( pObject[i] );
		}

		if( !m_pDebugger || !m_pDebugger->RemoteCmdValid() )
			return;
		m_pDebugger->CheckRemoteCmd();
	}

	bool CScriptBase::IsVirtualTableValid( SVirtualObj* pVObj )
	{
		if( !IsAllocVirtualTable( pVObj ) )
			return true;
		SFunctionTableHead* pFunTableHead = ( (SFunctionTableHead*)pVObj->m_pTable ) - 1;
		return pFunTableHead->m_pScript == this;
	}

	bool CScriptBase::IsAllocVirtualTable( void* pVirtualTable )
	{
		return pVirtualTable >= s_aryFuctionTable && pVirtualTable < s_aryFuctionTableEnd;
	}

    SFunctionTable* CScriptBase::GetOrgVirtualTable( void* pObj )
    {
		// 寻找pObj对象的原始虚表，如果pObj的虚表已经被修改过
		// 那么，应该会在m_mapVirtualTableNew2Old中找到相应的虚表
		// 否则说明pObj的虚表没有被修改过，直接返回他自己的虚表
        SVirtualObj* pVObj = (SVirtualObj*)pObj;
		if( !CScriptBase::IsAllocVirtualTable( pVObj->m_pTable ) )
			return pVObj->m_pTable;
		SFunctionTableHead* pFunTableHead = ( (SFunctionTableHead*)pVObj->m_pTable ) - 1;
		return pFunTableHead->m_pOldFunTable;
    }

    SFunctionTable* CScriptBase::CheckNewVirtualTable( SFunctionTable* pOldFunTable, 
		const CClassRegistInfo* pClassInfo, bool bNewByVM, uint32 nInheritDepth )
	{
		assert( !CScriptBase::IsAllocVirtualTable( pOldFunTable ) );

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
			if( VMObjectVTableInfo.first == NULL )
			{
				SFunctionTableHead* pFunTableHead = AllocFunArray( nFunCount + 1 );
				VMObjectVTableInfo.first = (SFunctionTable*)( pFunTableHead + 1 );
			}

			SFunctionTable* pNewFunTable = VMObjectVTableInfo.first;
			SFunctionTableHead* pFunTableHead = ( (SFunctionTableHead*)pNewFunTable ) - 1;
			memcpy( pNewFunTable->m_pFun, pOldFunTable->m_pFun, nFunCount*sizeof(void*) );
			pNewFunTable->m_pFun[nFunCount] = NULL;
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
			if( it != m_mapVirtualTableOld2New.end() && (void**)it->first < (void**)pOldFunTable + nFunCount )
				nFunCount = (int32)(ptrdiff_t)( (void**)it->first - (void**)pOldFunTable );

			SFunctionTableHead* pFunTableHead = AllocFunArray( nFunCount + 1 );
			SFunctionTable* pNewFunTable = (SFunctionTable*)( pFunTableHead + 1 );
			m_mapVirtualTableOld2New.insert( make_pair( pOldFunTable, pNewFunTable ) );
			memcpy( pNewFunTable->m_pFun, pOldFunTable->m_pFun, nFunCount*sizeof(void*) );
			pNewFunTable->m_pFun[nFunCount] = NULL;
			pFunTableHead->m_pScript = this;
			pFunTableHead->m_pOldFunTable = pOldFunTable;
			pFunTableHead->m_pClassInfo = pClassInfo;
			pClassInfo->InitVirtualTable( pNewFunTable );
			return pNewFunTable;
		}
		else if( static_cast<const CClassRegistInfo*>( it->second->m_pFun[-1] )
			->GetInheritDepth() < pClassInfo->GetInheritDepth() )
		{
			pClassInfo->InitVirtualTable( it->second );
		}

        return it->second;
	}

    void CScriptBase::AddSearchPath( const char* szPath )
    {
        m_listSearchPath.push_back( szPath );
    }

	int CScriptBase::Input( char* szBuffer, int nCount )
	{
		std::cin.read( szBuffer, nCount );
		return (int32)strlen( szBuffer );
	}

	void CScriptBase::UnlinkCppObj( void* pObj )
	{
		s_ScriptListLock.Lock();
		CScriptBase* pScript = s_listAllScript.GetFirst();
		while( pScript )
		{
			pScript->m_UnlinkObjectBuffer.Push( pObj );
			pScript = pScript->TList<CScriptBase>::CListNode::GetNext();
		}
		s_ScriptListLock.Unlock();
	}

	int32 CScriptBase::CallBack( int32 nIndex, void* pObject, void* pRetBuf, void** pArgArray )
	{
		SVirtualObj* pVirtualObj = (SVirtualObj*)pObject;
		assert( CScriptBase::IsAllocVirtualTable( pVirtualObj->m_pTable ) );
		SFunctionTableHead* pFunTableHead = ( (SFunctionTableHead*)pVirtualObj->m_pTable ) - 1;
		assert( pFunTableHead->m_pClassInfo && pFunTableHead->m_pOldFunTable );
		const vector<CCallScriptBase*>& listFun = pFunTableHead->m_pClassInfo->GetNewFunctionList();
		CCallScriptBase* pCallScript = listFun[nIndex];
		CScriptBase* pScriptBase = pFunTableHead->m_pScript;
		pScriptBase->CheckUnlinkCppObj();
		auto& strFunctionName = pCallScript->GetFunctionName();
		try
		{
			if( strFunctionName.empty() )
			{
				pScriptBase->DestrucVM( pCallScript, pVirtualObj );
				return pCallScript->Destruc( pVirtualObj, pArgArray[0], *pFunTableHead->m_pScript );
			}
			else
			{
				if( pScriptBase->CallVM( pCallScript, pVirtualObj, pRetBuf, pArgArray ) )
					return 1;
				return pCallScript->CallOrg( pVirtualObj, pRetBuf, pArgArray, *pFunTableHead->m_pScript );
			}
		}
		catch( ... )
		{
			const char* szClass = pFunTableHead->m_pClassInfo->GetClassName().c_str();
			std::cout << "Unknown Error while call VM with " 
				<< strFunctionName.c_str() << "in " << szClass << endl;
		}
		return 0;
	}
}
