/**@file  		CCallInfo.h
* @brief		memory interface
* @author		Daphnis Kau
* @date			2020-01-17
* @version		V1.0
*/

#ifndef __CALL_BASE_H__
#define __CALL_BASE_H__
//=====================================================================
// CCallInfo.h 
// 定义基本的脚本和C++互相调用接口
// 柯达昭
// 2007-10-16
//=====================================================================
#include "common/TRBTree.h"
#include "common/CVirtualFun.h"
#include "common/TConstString.h"
#include "CTypeBase.h"
#include <list>

namespace XS
{
	enum ECallingType
	{
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
    // 脚本调用C++的基本接口
    //=====================================================================
    class CCallInfo 
		: public CCallBaseMap::CRBTreeNode
    {
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
		DataType				GetResultType()		const { return m_nResult; }
		uint32					GetParamCount()		const { return m_nParamCount; }
		int32					GetFunctionIndex()	const { return m_nFunIndex; }
		const const_string&		GetFunctionName()	const { return m_sFunName; }

	protected:
		IFunctionWrap*			m_funWrap;
		uintptr_t				m_funOrg;
		const_string			m_sFunName;
		DataType				m_nResult;
		DataTypeArray			m_listParam;
		uint32					m_nParamCount;
		int32					m_nFunIndex;
	};

	//=====================================================================
	// Lua脚本访问C++的成员接口
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
    // C++调用脚本的基本接口
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
