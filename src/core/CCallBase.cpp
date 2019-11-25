#include "core/CCallBase.h"
#include "core/CScriptBase.h"
#include "common/Help.h"
#include <iostream>

#ifdef _ANDROID
#include <alloca.h>
#endif

namespace Gamma
{
	CCallBase::CCallBase(const STypeInfoArray& aryTypeInfo, int32 nFunIndex,
		const char* szTypeInfoName, gammacstring strFunName )
		: m_nParamSize( 0 )
		, m_nThis( eDT_void )
        , m_nResult( eDT_void )
		, m_nFunIndex( nFunIndex )
		, m_sFunName( strFunName.c_str(), strFunName.size() )
	{
		CClassRegistInfo* pInfo = CClassRegistInfo::GetRegistInfo( szTypeInfoName );
		if( pInfo == NULL )
			throw( "register function on a unregister class." );
		pInfo->RegistFunction( this );

		for( uint32 i = 0; i < aryTypeInfo.nSize; i++ )
		{
			DataType nType = ToDataType( aryTypeInfo.aryInfo[i] );
			if( i == aryTypeInfo.nSize - 1 )
				m_nResult = nType;
			else if( m_nFunIndex >= eCT_ClassFunction && i == 0 )
				m_nThis = nType;
			else
				m_listParam.push_back( nType );
		}

		//for(int32 i = 0; i < m_listParam.size(); i++)
		//	m_nParamSize += AligenUp(m_listParam[i]->GetLen(), sizeof(void*));
		m_nParamCount = (uint32)m_listParam.size();
    }

    CCallBase::~CCallBase(void)
    {
	}

	//=====================================================================
	// 脚本调用C++的基本接口
	//=====================================================================
	CByScriptBase::CByScriptBase(const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, 
		const char* szTypeInfoName, int32 nFunIndex, const char* szFunName)
		: CCallBase(aryTypeInfo, nFunIndex, szTypeInfoName, szFunName)
		, m_funWrap(funWrap)
	{}
	
	void CByScriptBase::Call(void* pObject, void* pRetBuf, void** pArgArray, CScriptBase& Script)
	{
		m_funWrap->Call(pObject, pRetBuf, pArgArray, SFunction());
	}

	//=====================================================================
	// 脚本访问C++的成员接口
	//=====================================================================
	CByScriptMember::CByScriptMember( const STypeInfoArray& aryTypeInfo, 
		IFunctionWrap* funGetSet[2], const char* szTypeInfoName, const char* szFunName ) 
		: CByScriptBase( aryTypeInfo, funGetSet[0], szTypeInfoName, eCT_MemberFunction, szFunName )
		, m_funSet( funGetSet[1] )
	{
		DataType nType = ToDataType( aryTypeInfo.aryInfo[1] );
		//m_nParamSize = AligenUp( pType->GetLen(), sizeof(void*) );
		m_listParam.push_back( nType );
		m_nParamCount = 1;
	}

	void CByScriptMember::Call( void* pObject, void* pRetBuf, void** pArgArray, CScriptBase& Script)
	{
		if( !pRetBuf && m_funSet )
			m_funSet->Call( pObject, &pRetBuf, pArgArray, SFunction() );
		else if( pRetBuf && m_funWrap )
			m_funWrap->Call( pObject, pRetBuf, pArgArray, SFunction() );
	}

	//=====================================================================
	// C++调用脚本的基本接口
	//=====================================================================
    CCallScriptBase::CCallScriptBase( const STypeInfoArray& aryTypeInfo,
		IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunName )
		: CByScriptBase( aryTypeInfo, funWrap, szTypeInfoName, 0, szFunName )
		, m_pBootFun( NULL )
		, m_bPureVirtual( false )
	{
		auto pInfo = CClassRegistInfo::GetRegistInfo(szTypeInfoName);
		m_nFunIndex = szFunName && szFunName[0] ?
			GetVirtualFunIndex( funWrap->GetOrgFun() ) : (uint32)funWrap->GetOrgFun().funPoint;
		pInfo->RegistClassCallBack( m_nFunIndex, this );
	}

    CCallScriptBase::~CCallScriptBase()
    {
	}

	int32 CCallScriptBase::BindFunction( void* pFun, bool bPureVirtual )
	{
		m_pBootFun = pFun; 
		m_bPureVirtual = bPureVirtual;
		return m_nFunIndex;
	}

	int32 CCallScriptBase::CallOrg( SVirtualObj* pObject, void* pRetBuf, void** pArgArray, CScriptBase& Script)
	{
		if( m_bPureVirtual )
			return -1;
		SFunctionTable* pTable = Script.GetOrgVirtualTable( pObject );
		if( !pTable || !pTable->m_pFun[m_nFunIndex] || pTable->m_pFun[m_nFunIndex] == m_pBootFun )
			return -1;
		SFunction funOrg = { (uintptr_t)pTable->m_pFun[m_nFunIndex], NULL };
		m_funWrap->Call( pObject, pRetBuf, pArgArray, funOrg );
		return 0;
	}

	int32 CCallScriptBase::Destruc( SVirtualObj* pObject, void* pParam, CScriptBase& Script )
	{
		if( !pObject->m_pTable || !pObject->m_pTable->m_pFun[m_nFunIndex] )
			return 0;
		SFunction funOrg = { NULL, NULL };
		m_funWrap->Call( pObject, NULL, NULL, funOrg );
		return 0;
	}

	void CCallScriptBase::Call( void* pObject, void* pRetBuf, void** pArgArray, CScriptBase& Script)
	{
		// fun( char int string )lua调用c++ i3cpu 1000000次 2秒
		SFunctionTable* pTable = Script.GetOrgVirtualTable( pObject );
		SFunction CppFun = { (uintptr_t)pTable->m_pFun[m_nFunIndex], 0 };
		m_funWrap->Call( pObject, pRetBuf, pArgArray, CppFun );
	}

}
