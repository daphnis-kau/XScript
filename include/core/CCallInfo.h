/**@file  		CCallInfo.h
* @brief		Call each other between C++&Script
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
*/

#ifndef __CALL_INFO_H__
#define __CALL_INFO_H__
#include "common/TRBTree.h"
#include "common/CVirtualFun.h"
#include "common/TConstString.h"
#include "CTypeBase.h"
#include <list>

namespace XS
{
	enum ECallingType
	{
		eCT_Value				= -5,
		eCT_GlobalFunction		= -4,
		eCT_ClassStaticFunction	= -3,
		eCT_ClassFunction		= -2,
		eCT_MemberFunction		= -1,
		eCT_ClassCallBack		= 0,
	};

	class CCallInfo;
	class CScriptBase;
	typedef TRBTree<CCallInfo> CCallBaseMap;

    //=====================================================================
    // Information of c++ function
    //=====================================================================
    class CCallInfo 
		: public CCallBaseMap::CRBTreeNode
    {
		typedef std::vector<uint32> DataTypeSize;
		typedef std::vector<DataType> DataTypeArray;
		const CCallInfo& operator= ( const CCallInfo& );
    public:
		CCallInfo( IFunctionWrap* funWrap, const STypeInfoArray& aryTypeInfo, 
			uintptr_t funOrg, const char* szTypeInfoName, int32 nFunIndex, const char* szFunName );
		operator const const_string&( ) const { return m_sFunName; }
		bool operator < ( const const_string& strKey ) { return (const const_string&)*this < strKey; }

		virtual void			Call(void* pRetBuf, void** pArgArray, CScriptBase& Script) const;
		IFunctionWrap*			GetFunWrap()		const { return m_funWrap; }
		const DataTypeArray&	GetParamList()		const { return m_listParam; }
		const DataTypeSize&		GetParamSize()		const { return m_listParamSize; }
		uint32					GetParamTotalSize()	const { return m_nTotalParamSize; }
		DataType				GetResultType()		const { return m_nResult; }
		uint32					GetResultSize()		const { return m_nReturnSize; }
		int32					GetFunctionIndex()	const { return m_nFunIndex; }
		const const_string&		GetFunctionName()	const { return m_sFunName; }
		uintptr_t				GetFunctionOrg()	const { return m_funOrg; }

	protected:
		IFunctionWrap*			m_funWrap;
		uintptr_t				m_funOrg;
		const_string			m_sFunName;
		DataType				m_nResult;
		DataTypeArray			m_listParam;
		DataTypeSize			m_listParamSize;
		uint32					m_nTotalParamSize;
		uint32					m_nReturnSize;
		int32					m_nFunIndex;
	};

	//=====================================================================
	// Information of class's data member
	//=====================================================================
	class CMemberInfo : public CCallInfo
	{
		IFunctionWrap*	m_funSet;
	public:
		CMemberInfo( IFunctionWrap* funGetSet[2], const STypeInfoArray& aryTypeInfo, 
			uintptr_t nOffset, const char* szTypeInfoName, const char* szMemberName );
		virtual void	Call(void* pRetBuf, void** pArgArray, CScriptBase& Script) const;
		uintptr_t		GetOffset() const { return m_funOrg; }
		IFunctionWrap*	GetFunSet() const { return m_funSet; }
	};

    //=====================================================================
    // Information of c++ function which can be override
    //=====================================================================
    class CCallbackInfo : public CCallInfo
	{
		bool			m_bPureVirtual;
		const CCallbackInfo& operator= ( const CCallbackInfo& );
    public:
		CCallbackInfo( IFunctionWrap* funWrap, const STypeInfoArray& aryTypeInfo, uintptr_t funBoot, 
			int32 nFunIndex, bool bPureVirtual, const char* szTypeInfoName, const char* szFunName );
        ~CCallbackInfo();

		virtual void	Call( void* pRetBuf, void** pArgArray, CScriptBase& Script ) const;
		void*			GetBootFun() const { return (void*)m_funOrg; }
		int32			Destruc( SVirtualObj* pObject, void* pParam, CScriptBase& Script ) const;
	};
}

#endif
