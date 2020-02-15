#include "core/CCallInfo.h"
#include "core/CScriptBase.h"
#include "core/CClassInfo.h"

namespace XS
{
	//=====================================================================
	// 类型管理器
	//=====================================================================
	class CGlobalClassRegist
	{
		CGlobalClassRegist();
		~CGlobalClassRegist();
	public:
		static CGlobalClassRegist& GetInst();
		CTypeIDNameMap	m_mapTypeID2ClassInfo;
	};

	CGlobalClassRegist::CGlobalClassRegist()
	{
		m_mapTypeID2ClassInfo.Insert( *new CClassInfo( "" ) );
	}

	CGlobalClassRegist::~CGlobalClassRegist()
	{
		while( m_mapTypeID2ClassInfo.GetFirst() )
		{
			auto* pClassInfo = m_mapTypeID2ClassInfo.GetFirst();
			delete static_cast<CClassInfo*>( pClassInfo );
		}
	}

	inline CGlobalClassRegist& CGlobalClassRegist::GetInst()
	{
		static CGlobalClassRegist s_Instance;
		return s_Instance;
	}

	const CClassInfo* CClassInfo::RegisterClass(
		const char* szClassName, const char* szTypeIDName, uint32 nSize, bool bEnum )
	{
		const_string strKey( szTypeIDName, true );
		CGlobalClassRegist& Inst = CGlobalClassRegist::GetInst();
		CClassInfo* pInfo = Inst.m_mapTypeID2ClassInfo.Find( strKey );
		if( !pInfo )
		{
			pInfo = new CClassInfo( szTypeIDName );
			Inst.m_mapTypeID2ClassInfo.Insert( *pInfo );
		}

		if( szClassName && szClassName[0] )
			pInfo->m_szClassName = szClassName;
		pInfo->m_bIsEnum = bEnum;
		assert( pInfo->m_nSizeOfClass == 0 || pInfo->m_nSizeOfClass == nSize );
		pInfo->m_nSizeOfClass = nSize;
		pInfo->m_nAligenSizeOfClass = AligenUp( nSize, sizeof( void* ) );
		return pInfo;
	}

	const CClassInfo* CClassInfo::GetClassInfo( const char* szTypeInfoName )
	{
		const_string strKey( szTypeInfoName, true );
		CGlobalClassRegist& Inst = CGlobalClassRegist::GetInst();
		return Inst.m_mapTypeID2ClassInfo.Find( strKey );
	}

	const CClassInfo* CClassInfo::SetObjectConstruct( 
		const char* szTypeInfoName, IObjectConstruct* pObjectConstruct )
	{
		const_string strKey( szTypeInfoName, true );
		CGlobalClassRegist& Inst = CGlobalClassRegist::GetInst();
		assert( Inst.m_mapTypeID2ClassInfo.Find( strKey ) );
		CClassInfo* pInfo = Inst.m_mapTypeID2ClassInfo.Find( strKey );
		pInfo->m_pObjectConstruct = pObjectConstruct;
		return pInfo;
	}

	const CClassInfo* CClassInfo::AddBaseInfo( 
		const char* szTypeInfoName, const char* szBaseTypeInfoName, ptrdiff_t nOffset )
	{
		CGlobalClassRegist& Inst = CGlobalClassRegist::GetInst();
		const_string strDeriveKey( szTypeInfoName, true );
		CClassInfo* pInfo = Inst.m_mapTypeID2ClassInfo.Find( strDeriveKey );
		const_string strBaseKey( szBaseTypeInfoName, true );
		CClassInfo* pBaseInfo = Inst.m_mapTypeID2ClassInfo.Find( strBaseKey );
		assert( pInfo && pBaseInfo && nOffset >= 0 );
		SBaseInfo BaseInfo = { pBaseInfo, (int32)nOffset };
		if( pBaseInfo->m_nInheritDepth + 1 > pInfo->m_nInheritDepth )
			pInfo->m_nInheritDepth = pBaseInfo->m_nInheritDepth + 1;
		pInfo->m_vecBaseRegist.push_back( BaseInfo );

		BaseInfo.m_pBaseInfo = pInfo;
		BaseInfo.m_nBaseOff = -BaseInfo.m_nBaseOff;
		pBaseInfo->m_vecChildRegist.push_back( BaseInfo );

		if( nOffset )
			return pInfo;

		// 自然继承，虚表要延续
		auto& vecNewFunction = pBaseInfo->m_vecOverridableFun;
		for( int32 i = 0; i < (int32)vecNewFunction.size(); i++ )
		{
			if( !vecNewFunction[i] )
				continue;
			assert( vecNewFunction[i]->GetFunctionIndex() == i );
			RegisterCallBack( szTypeInfoName, i, vecNewFunction[i] );
		}
		return pInfo;
	}

	const XS::CCallInfo* CClassInfo::RegisterFunction(
		const char* szTypeInfoName, CCallInfo* pCallBase )
	{
		const_string strKey( szTypeInfoName, true );
		CGlobalClassRegist& Inst = CGlobalClassRegist::GetInst();
		CClassInfo* pInfo = Inst.m_mapTypeID2ClassInfo.Find( strKey );
		if( !pInfo )
			return nullptr;
		assert( pInfo->m_mapRegistFunction.find( 
			pCallBase->GetFunctionName() ) == pInfo->m_mapRegistFunction.end() );
		pInfo->m_mapRegistFunction.Insert( *pCallBase );
		return pCallBase;
	}

	const CCallInfo* CClassInfo::RegisterCallBack(
		const char* szTypeInfoName, uint32 nIndex, CCallbackInfo* pCallScriptBase )
	{
		const_string strKey( szTypeInfoName, true );
		CGlobalClassRegist& Inst = CGlobalClassRegist::GetInst();
		CClassInfo* pInfo = Inst.m_mapTypeID2ClassInfo.Find( strKey );
		if( !pInfo )
			return nullptr;
		// 不能重复注册
		if( nIndex >= pInfo->m_vecOverridableFun.size() )
			pInfo->m_vecOverridableFun.resize( nIndex + 1 );
		assert( pInfo->m_vecOverridableFun[nIndex] == NULL );
		pInfo->m_vecOverridableFun[nIndex] = pCallScriptBase;

		for( size_t i = 0; i < pInfo->m_vecChildRegist.size(); ++i )
		{
			if( pInfo->m_vecChildRegist[i].m_nBaseOff )
				continue;
			auto& strName = pInfo->m_vecChildRegist[i].m_pBaseInfo->GetTypeIDName();
			RegisterCallBack( strName.c_str(), nIndex, pCallScriptBase );
		}
		return pCallScriptBase;
	}

	const XS::CTypeIDNameMap& CClassInfo::GetAllRegisterInfo()
	{
		return CGlobalClassRegist::GetInst().m_mapTypeID2ClassInfo;
	}

	//=====================================================================
    // 类型的继承关系
    //=====================================================================
	CClassInfo::CClassInfo( const char* szTypeIDName )
		: m_szTypeIDName( szTypeIDName )
        , m_nSizeOfClass( 0 )
		, m_nAligenSizeOfClass( 0 )
        , m_pObjectConstruct( NULL )
        , m_bIsEnum(false)
		, m_nInheritDepth(0)
	{
    }

    CClassInfo::~CClassInfo()
	{
		while( m_mapRegistFunction.GetFirst() )
			delete m_mapRegistFunction.GetFirst();
    }

    void CClassInfo::InitVirtualTable( SFunctionTable* pNewTable ) const
	{
		for( int32 i = 0; i < (int32)m_vecOverridableFun.size(); i++ )
		{
			if( !m_vecOverridableFun[i] )
				continue;
			CCallbackInfo* pCallInfo = m_vecOverridableFun[i];
			assert( pCallInfo->GetFunctionIndex() == i );
			pNewTable->m_pFun[i] = pCallInfo->GetBootFun();
		}
    }

    int32 CClassInfo::GetMaxRegisterFunctionIndex() const
    {        
		return (int32)m_vecOverridableFun.size();
    }

    void CClassInfo::Create( CScriptBase* pScript, void* pObject ) const
	{
		pScript->CheckDebugCmd();
		//声明性质的类不可创建
		assert( m_nSizeOfClass );
		assert( m_pObjectConstruct );
		if( !m_pObjectConstruct )
			return;
		m_pObjectConstruct->Construct( pObject );
	}

	void CClassInfo::Assign( CScriptBase* pScript, void* pDest, void* pSrc ) const
	{
		pScript->CheckDebugCmd();
		assert( m_pObjectConstruct );
		if( !m_pObjectConstruct )
			return;
		m_pObjectConstruct->Assign( pDest, pSrc );
	}

    void CClassInfo::Release( CScriptBase* pScript, void* pObject ) const
	{
		pScript->CheckDebugCmd();
		//声明性质的类不可销毁
		assert( m_pObjectConstruct );
		if( !m_pObjectConstruct )
			return;
		m_pObjectConstruct->Destruct( pObject );
	}

	const CCallInfo* CClassInfo::GetCallBase( const const_string& strFunName ) const
	{
		return m_mapRegistFunction.Find( strFunName );
	}

    bool CClassInfo::IsCallBack() const
    {
		return !m_vecOverridableFun.empty();
    }

    int32 CClassInfo::GetBaseOffset( const CClassInfo* pRegist ) const
    {
        if( pRegist == this )
            return 0;

        for( size_t i = 0; i < m_vecBaseRegist.size(); i++ )
        {
            int32 nOffset = m_vecBaseRegist[i].m_pBaseInfo->GetBaseOffset( pRegist );
            if( nOffset >= 0 )
                return m_vecBaseRegist[i].m_nBaseOff + nOffset;
        }

        return -1;
	}

    void CClassInfo::ReplaceVirtualTable( CScriptBase* pScript,
		void* pObj, bool bNewByVM, uint32 nInheritDepth ) const
    {
        SVirtualObj* pVObj        = (SVirtualObj*)pObj;
        SFunctionTable* pOldTable = pVObj->m_pTable;
        SFunctionTable* pNewTable = NULL;

		if( !m_vecOverridableFun.empty() )
		{
			// 确保pOldTable是原始虚表，因为pVObj有可能已经被修改过了
			pOldTable = pScript->GetOrgVirtualTable( pVObj );
			pNewTable = pScript->CheckNewVirtualTable( pOldTable, this, bNewByVM, nInheritDepth );
		}

        for( size_t i = 0; i < m_vecBaseRegist.size(); i++ )
        {
            if( m_vecBaseRegist[i].m_pBaseInfo->IsCallBack() )
			{
				void* pBaseObj = ( (char*)pObj ) + m_vecBaseRegist[i].m_nBaseOff;
                m_vecBaseRegist[i].m_pBaseInfo->ReplaceVirtualTable( 
					pScript, pBaseObj, bNewByVM, nInheritDepth + 1 );
			}
        }

        if( pNewTable )
		{
			// 不允许不同的虚拟机共同使用同一份虚表
			assert( pScript->IsVirtualTableValid(pVObj) );
            pVObj->m_pTable = pNewTable;
		}
    }

    void CClassInfo::RecoverVirtualTable( CScriptBase* pScript, void* pObj ) const
    {
        SFunctionTable* pOrgTable = NULL;
        if( !m_vecOverridableFun.empty() )
            pOrgTable = pScript->GetOrgVirtualTable( pObj );

        for( size_t i = 0; i < m_vecBaseRegist.size(); i++ )
            m_vecBaseRegist[i].m_pBaseInfo->RecoverVirtualTable( 
				pScript, ( (char*)pObj ) + m_vecBaseRegist[i].m_nBaseOff );

        if( pOrgTable )
            ( (SVirtualObj*)pObj )->m_pTable = pOrgTable;
    }

    bool CClassInfo::FindBase( const CClassInfo* pRegistBase ) const
    {
        if( pRegistBase == this )
            return true;
        for( size_t i = 0; i < m_vecBaseRegist.size(); i++ )
            if( m_vecBaseRegist[i].m_pBaseInfo->FindBase( pRegistBase ) )
                return true;
        return false;
    }

	bool CClassInfo::IsBaseObject( ptrdiff_t nDiff ) const
	{
		// 命中基类
		if( nDiff == 0 )
			return true;

		// 超出基类内存范围
		if( nDiff > (int32)GetClassSize() )
			return false;

		for( size_t i = 0; i < m_vecBaseRegist.size(); i++ )
		{
			ptrdiff_t nBaseDiff = m_vecBaseRegist[i].m_nBaseOff;
			if( nDiff < nBaseDiff )
				continue;
			if( m_vecBaseRegist[i].m_pBaseInfo->IsBaseObject( nDiff - nBaseDiff ) )
				return true;
		}

		// 没有适配的基类
		return false;
	}
}
