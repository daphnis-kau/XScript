#include "core/CCallInfo.h"
#include "core/CScriptBase.h"
#include "common/Help.h"
#include <iostream>

#ifdef _ANDROID
#include <alloca.h>
#endif

namespace XS
{
	//=====================================================================
	// 脚本调用C++的基本接口
	//=====================================================================
	CCallInfo::CCallInfo( IFunctionWrap* funWrap, const STypeInfoArray& aryTypeInfo,
		uintptr_t funOrg, const char* szTypeInfoName, int32 nFunIndex, const char* szFunName )
		: m_funWrap( funWrap )
		, m_funOrg(funOrg)
		, m_nResult( eDT_void )
		, m_nFunIndex( nFunIndex )
		, m_sFunName( szFunName )
	{
		if( CClassRegistInfo::GetRegistInfo(szTypeInfoName) == NULL )
			throw( "register function on a unregister class." );
		CClassRegistInfo::RegisterFunction( szTypeInfoName, this );

		for( uint32 i = 0; i < aryTypeInfo.nSize - 1; i++ )
			m_listParam.push_back( ToDataType( aryTypeInfo.aryInfo[i] ) );
		m_nResult = ToDataType( aryTypeInfo.aryInfo[aryTypeInfo.nSize - 1] );
		m_nParamCount = (uint32)m_listParam.size();
	}
	
	void CCallInfo::Call(void* pRetBuf, void** pArgArray, CScriptBase& Script) const
	{
		m_funWrap->Call( pRetBuf, pArgArray, m_funOrg );
		Script.CheckDebugCmd();
	}

	//=====================================================================
	// 脚本访问C++的成员接口
	//=====================================================================
	CMemberInfo::CMemberInfo( IFunctionWrap* funGetSet[2], 
		const STypeInfoArray& aryTypeInfo, uintptr_t nOffset,
		const char* szTypeInfoName, const char* szMemberName )
		: CCallInfo( funGetSet[0], aryTypeInfo, nOffset,
			szTypeInfoName, eCT_MemberFunction, szMemberName )
		, m_funSet( funGetSet[1] )
	{
		DataType nType = ToDataType( aryTypeInfo.aryInfo[1] );
		m_listParam.push_back( nType );
		m_nParamCount = 1;
	}

	void CMemberInfo::Call( void* pRetBuf, void** pArgArray, CScriptBase& Script) const
	{
		if (!pRetBuf && m_funSet)
			m_funSet->Call(&pRetBuf, pArgArray, GetOffset());
		else if (pRetBuf && m_funWrap)
			m_funWrap->Call(pRetBuf, pArgArray, GetOffset());
	}

	//=====================================================================
	// C++调用脚本的基本接口
	//=====================================================================
	CCallbackInfo::CCallbackInfo( 
		IFunctionWrap* funWrap, const STypeInfoArray& aryTypeInfo, uintptr_t funBoot,
		int32 nFunIndex, bool bPureVirtual, const char* szTypeInfoName, const char* szFunName )
		: CCallInfo( funWrap, aryTypeInfo, funBoot, szTypeInfoName, 0, szFunName )
		, m_bPureVirtual(bPureVirtual)
	{
		m_nFunIndex = nFunIndex;
		CClassRegistInfo::RegisterCallBack( szTypeInfoName, m_nFunIndex, this );
	}

    CCallbackInfo::~CCallbackInfo()
    {
	}

	void CCallbackInfo::Call( void* pRetBuf, void** pArgArray, CScriptBase& Script ) const
	{
		if( m_bPureVirtual )
			return;
		SFunctionTable* pTable = Script.GetOrgVirtualTable( *(void**)pArgArray[0] );
		if( !pTable || !pTable->m_pFun[m_nFunIndex] || 
			pTable->m_pFun[m_nFunIndex] == GetBootFun() )
			return;
		m_funWrap->Call( pRetBuf, pArgArray, (uintptr_t)pTable->m_pFun[m_nFunIndex] );
	}

	int32 CCallbackInfo::Destruc( SVirtualObj* pObject, void* pParam, CScriptBase& Script ) const
	{
		if( !pObject->m_pTable || !pObject->m_pTable->m_pFun[m_nFunIndex] )
			return 0;
		m_funWrap->Call( NULL, (void**)&pObject, 0 );
		return 0;
	}
}
