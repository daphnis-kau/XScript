#include "core/CCallBase.h"
#include "core/CScriptBase.h"
#include "core/CClassRegistInfo.h"

namespace Gamma
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
	}

	CGlobalClassRegist::~CGlobalClassRegist()
	{
		while( m_mapTypeID2ClassInfo.GetFirst() )
		{
			auto* pClassInfo = m_mapTypeID2ClassInfo.GetFirst();
			delete static_cast<CClassRegistInfo*>( pClassInfo );
		}
	}

	inline CGlobalClassRegist& CGlobalClassRegist::GetInst()
	{
		static CGlobalClassRegist s_Instance;
		return s_Instance;
	}

	const CClassRegistInfo* CClassRegistInfo::RegisterClass(
		const char* szClassName, const char* szTypeIDName, uint32 nSize, bool bEnum )
	{
		gammacstring strKey( szTypeIDName, true );
		CGlobalClassRegist& Inst = CGlobalClassRegist::GetInst();
		CClassRegistInfo* pInfo = Inst.m_mapTypeID2ClassInfo.Find( strKey );
		if( !pInfo )
			pInfo = new CClassRegistInfo(szTypeIDName);
		assert( pInfo->GetClassSize() == 0 );
		pInfo->m_szClassName = szClassName;
		pInfo->m_nSizeOfClass = nSize;
		pInfo->m_bIsEnum = bEnum;
		pInfo->m_nAligenSizeOfClass = AligenUp( nSize, sizeof( void* ) );
		return pInfo;
	}

	const CClassRegistInfo* CClassRegistInfo::GetRegistInfo( const char* szTypeInfoName )
	{
		gammacstring strKey( szTypeInfoName, true );
		CGlobalClassRegist& Inst = CGlobalClassRegist::GetInst();
		CClassRegistInfo* pInfo = Inst.m_mapTypeID2ClassInfo.Find( strKey );
		if( pInfo )
			return pInfo;
		return new CClassRegistInfo( szTypeInfoName );
	}

	const CClassRegistInfo* CClassRegistInfo::SetObjectConstruct( 
		const char* szTypeInfoName, IObjectConstruct* pObjectConstruct )
	{
		gammacstring strKey( szTypeInfoName, true );
		CGlobalClassRegist& Inst = CGlobalClassRegist::GetInst();
		assert( Inst.m_mapTypeID2ClassInfo.Find( strKey ) );
		CClassRegistInfo* pInfo = Inst.m_mapTypeID2ClassInfo.Find( strKey );
		pInfo->m_pObjectConstruct = pObjectConstruct;
		return pInfo;
	}

	const CClassRegistInfo* CClassRegistInfo::AddBaseRegist( 
		const char* szTypeInfoName, const char* szBaseTypeInfoName, ptrdiff_t nOffset )
	{
		CGlobalClassRegist& Inst = CGlobalClassRegist::GetInst();
		gammacstring strDeriveKey( szTypeInfoName, true );
		CClassRegistInfo* pInfo = Inst.m_mapTypeID2ClassInfo.Find( strDeriveKey );
		gammacstring strBaseKey( szBaseTypeInfoName, true );
		CClassRegistInfo* pBaseInfo = Inst.m_mapTypeID2ClassInfo.Find( strBaseKey );
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
		vector<CCallScriptBase*>& vecNewFunction = pBaseInfo->m_vecNewFunction;
		for( uint32 i = 0; i < vecNewFunction.size(); i++ )
		{
			if( !vecNewFunction[i] )
				continue;
			assert( vecNewFunction[i]->GetFunIndex() == i );
			RegisterCallBack( szTypeInfoName, i, vecNewFunction[i] );
		}
		return pInfo;
	}

	const Gamma::CByScriptBase* CClassRegistInfo::RegisterFunction(
		const char* szTypeInfoName, CByScriptBase* pCallBase )
	{
		gammacstring strKey( szTypeInfoName, true );
		CGlobalClassRegist& Inst = CGlobalClassRegist::GetInst();
		CClassRegistInfo* pInfo = Inst.m_mapTypeID2ClassInfo.Find( strKey );
		if( !pInfo )
			return nullptr;
		assert( pInfo->m_mapRegistFunction.find( 
			pCallBase->GetFunctionName() ) == pInfo->m_mapRegistFunction.end() );
		pInfo->m_mapRegistFunction.Insert( *pCallBase );
		return pCallBase;
	}

	const CByScriptBase* CClassRegistInfo::RegisterCallBack(
		const char* szTypeInfoName, uint32 nIndex, CCallScriptBase* pCallScriptBase )
	{
		gammacstring strKey( szTypeInfoName, true );
		CGlobalClassRegist& Inst = CGlobalClassRegist::GetInst();
		CClassRegistInfo* pInfo = Inst.m_mapTypeID2ClassInfo.Find( strKey );
		if( !pInfo )
			return nullptr;
		// 不能重复注册
		if( nIndex >= pInfo->m_vecNewFunction.size() )
			pInfo->m_vecNewFunction.resize( nIndex + 1 );
		assert( pInfo->m_vecNewFunction[nIndex] == NULL );
		pInfo->m_vecNewFunction[nIndex] = pCallScriptBase;

		for( size_t i = 0; i < pInfo->m_vecChildRegist.size(); ++i )
		{
			if( pInfo->m_vecChildRegist[i].m_nBaseOff )
				continue;
			auto& strName = pInfo->m_vecChildRegist[i].m_pBaseInfo->GetTypeIDName();
			RegisterCallBack( strName.c_str(), nIndex, pCallScriptBase );
		}
		return pCallScriptBase;
	}

	const Gamma::CTypeIDNameMap& CClassRegistInfo::GetAllRegisterInfo()
	{
		return CGlobalClassRegist::GetInst().m_mapTypeID2ClassInfo;
	}

	//=====================================================================
    // 类型的继承关系
    //=====================================================================
	CClassRegistInfo::CClassRegistInfo( const char* szTypeIDName )
		: m_szTypeIDName( szTypeIDName )
        , m_nSizeOfClass( 0 )
		, m_nAligenSizeOfClass( 0 )
        , m_pObjectConstruct( NULL )
        , m_bIsEnum(false)
		, m_nInheritDepth(0)
	{
		CGlobalClassRegist::GetInst().m_mapTypeID2ClassInfo.Insert( *this );
    }

    CClassRegistInfo::~CClassRegistInfo()
	{
		while( m_mapRegistFunction.GetFirst() )
			delete m_mapRegistFunction.GetFirst();
    }

    void CClassRegistInfo::InitVirtualTable( SFunctionTable* pNewTable ) const
	{
		for( uint32 i = 0; i < m_vecNewFunction.size(); i++ )
		{
			if( !m_vecNewFunction[i] )
				continue;
			CCallScriptBase* pCallInfo = m_vecNewFunction[i];
			assert( pCallInfo->GetFunIndex() == i );
			pNewTable->m_pFun[i] = pCallInfo->GetBootFun();
		}
    }

    int32 CClassRegistInfo::GetMaxRegisterFunctionIndex() const
    {        
		return (int32)m_vecNewFunction.size();
    }

    void CClassRegistInfo::Create( void* pObject ) const
    {
		//声明性质的类不可创建
		assert( m_nSizeOfClass );
		assert( m_pObjectConstruct );
		if( !m_pObjectConstruct )
			return;
		m_pObjectConstruct->Construct( pObject );
	}

	void CClassRegistInfo::Assign( void* pDest, void* pSrc ) const
	{
		assert( m_pObjectConstruct );
		if( !m_pObjectConstruct )
			return;
		m_pObjectConstruct->Assign( pDest, pSrc );
	}

    void CClassRegistInfo::Release( void* pObject ) const
	{
		//声明性质的类不可销毁
		assert( m_pObjectConstruct );
		if( !m_pObjectConstruct )
			return;
		m_pObjectConstruct->Destruct( pObject );
	}

	const CByScriptBase* CClassRegistInfo::GetCallBase( const gammacstring& strFunName ) const
	{
		return m_mapRegistFunction.Find( strFunName );
	}

    bool CClassRegistInfo::IsCallBack() const
    {
		return !m_vecNewFunction.empty();
    }

    int32 CClassRegistInfo::GetBaseOffset( const CClassRegistInfo* pRegist ) const
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

    void CClassRegistInfo::ReplaceVirtualTable( CScriptBase* pScript,
		void* pObj, bool bNewByVM, uint32 nInheritDepth ) const
    {
        SVirtualObj* pVObj        = (SVirtualObj*)pObj;
        SFunctionTable* pOldTable = pVObj->m_pTable;
        SFunctionTable* pNewTable = NULL;

		if( !m_vecNewFunction.empty() )
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

    void CClassRegistInfo::RecoverVirtualTable( CScriptBase* pScript, void* pObj ) const
    {
        SFunctionTable* pOrgTable = NULL;
        if( !m_vecNewFunction.empty() )
            pOrgTable = pScript->GetOrgVirtualTable( pObj );

        for( size_t i = 0; i < m_vecBaseRegist.size(); i++ )
            m_vecBaseRegist[i].m_pBaseInfo->RecoverVirtualTable( 
				pScript, ( (char*)pObj ) + m_vecBaseRegist[i].m_nBaseOff );

        if( pOrgTable )
            ( (SVirtualObj*)pObj )->m_pTable = pOrgTable;
    }

    bool CClassRegistInfo::FindBase( const CClassRegistInfo* pRegistBase ) const
    {
        if( pRegistBase == this )
            return true;
        for( size_t i = 0; i < m_vecBaseRegist.size(); i++ )
            if( m_vecBaseRegist[i].m_pBaseInfo->FindBase( pRegistBase ) )
                return true;
        return false;
    }

	bool CClassRegistInfo::IsBaseObject( ptrdiff_t nDiff ) const
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
