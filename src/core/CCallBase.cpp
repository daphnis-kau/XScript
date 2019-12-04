#include "core/CCallBase.h"
#include "core/CScriptBase.h"
#include "common/Help.h"
#include <iostream>

#ifdef _ANDROID
#include <alloca.h>
#endif

namespace Gamma
{
	//=====================================================================
	// 脚本调用C++的基本接口
	//=====================================================================
	CByScriptBase::CByScriptBase( IFunctionWrap* funWrap, const STypeInfoArray& aryTypeInfo,
		uintptr_t funContext, const char* szTypeInfoName, int32 nFunIndex, const char* szFunName )
		: m_funWrap( funWrap )
		, m_funContext(funContext)
		, m_nThis( eDT_void )
		, m_nResult( eDT_void )
		, m_nFunIndex( nFunIndex )
		, m_sFunName( szFunName )
	{
		if( CClassRegistInfo::GetRegistInfo(szTypeInfoName) == NULL )
			throw( "register function on a unregister class." );
		CClassRegistInfo::RegisterFunction( szTypeInfoName, this );

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

		m_nParamCount = (uint32)m_listParam.size();
	}
	
	void CByScriptBase::Call(void* pRetBuf, void** pArgArray, CScriptBase& Script)
	{
		m_funWrap->Call(pRetBuf, pArgArray, m_funContext);
	}

	//=====================================================================
	// 脚本访问C++的成员接口
	//=====================================================================
	CByScriptMember::CByScriptMember( IFunctionWrap* funGetSet[2], 
		const STypeInfoArray& aryTypeInfo, uintptr_t funOrg,
		const char* szTypeInfoName, const char* szMemberName )
		: CByScriptBase( funGetSet[0], aryTypeInfo, funOrg, 
			szTypeInfoName, eCT_MemberFunction, szMemberName )
		, m_funSet( funGetSet[1] )
	{
		DataType nType = ToDataType( aryTypeInfo.aryInfo[1] );
		m_listParam.push_back( nType );
		m_nParamCount = 1;
	}

	void CByScriptMember::Call( void* pRetBuf, void** pArgArray, CScriptBase& Script)
	{
		if (!pRetBuf && m_funSet)
			m_funSet->Call(&pRetBuf, pArgArray, m_funContext);
		else if (pRetBuf && m_funWrap)
			m_funWrap->Call(pRetBuf, pArgArray, m_funContext);
	}

	//=====================================================================
	// C++调用脚本的基本接口
	//=====================================================================
	CCallScriptBase::CCallScriptBase( IFunctionWrap* funWrap, const STypeInfoArray& aryTypeInfo,
		uintptr_t funOrg, int32 nFunIndex, const char* szTypeInfoName, const char* szFunName )
		: CByScriptBase( funWrap, aryTypeInfo, funOrg, szTypeInfoName, 0, szFunName )
	{
		m_nFunIndex = nFunIndex;
		CClassRegistInfo::RegisterCallBack( szTypeInfoName, m_nFunIndex, this );
	}

    CCallScriptBase::~CCallScriptBase()
    {
	}

	int32 CCallScriptBase::Destruc( SVirtualObj* pObject, void* pParam, CScriptBase& Script )
	{
		if( !pObject->m_pTable || !pObject->m_pTable->m_pFun[m_nFunIndex] )
			return 0;
		m_funWrap->Call( NULL, (void**)&pObject, m_funContext);
		return 0;
	}
}
