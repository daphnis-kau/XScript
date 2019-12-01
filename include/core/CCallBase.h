#ifndef __CALL_BASE_H__
#define __CALL_BASE_H__
//=====================================================================
// CCallBase.h 
// 定义基本的脚本和C++互相调用接口
// 柯达昭
// 2007-10-16
//=====================================================================
#include "common/TRBTree.h"
#include "common/CVirtualFun.h"
#include "common/TConstString.h"
#include "CTypeBase.h"
#include <list>

using namespace std;

namespace Gamma
{
	enum ECallingType
	{
		eCT_GlobalFunction		= -4,
		eCT_ClassStaticFunction	= -3,
		eCT_ClassFunction		= -2,
		eCT_MemberFunction		= -1,
		eCT_ClassCallBack		= 0,
	};

	class CByScriptBase;
	class CScriptBase;
	typedef TRBTree<CByScriptBase> CCallBaseMap;

    //=====================================================================
    // 脚本调用C++的基本接口
    //=====================================================================
    class CByScriptBase 
		: public CCallBaseMap::CRBTreeNode
    {
		const CByScriptBase& operator= ( const CByScriptBase& );
    public:
		CByScriptBase( IFunctionWrap* funWrap, const STypeInfoArray& aryTypeInfo, 
			SFunction funOrg, const char* szTypeInfoName, int32 nFunIndex, const char* szFunName );
		operator const gammacstring&( ) const { return m_sFunName; }
		bool operator < ( const gammacstring& strKey ) { return (const gammacstring&)*this < strKey; }

		virtual void			Call(void* pObject, void* pRetBuf, void** pArgArray, CScriptBase& Script);
		IFunctionWrap*			GetFunWrap()		const { return m_funWrap; }
		const vector<DataType>&	GetParamList()		const { return m_listParam; }
		DataType				GetResultType()		const { return m_nResult; }
		DataType				GetThisType()		const { return m_nThis; }
		uint32					GetParamCount()		const { return m_nParamCount; }
		int32					GetFunctionIndex()	const { return m_nFunIndex; }
		const gammacstring&		GetFunctionName()	const { return m_sFunName; }

	protected:
		IFunctionWrap*			m_funWrap;
		SFunction				m_funOrg;
		gammacstring			m_sFunName;
		DataType				m_nThis;
		DataType				m_nResult;
		vector<DataType>		m_listParam;
		uint32					m_nParamCount;
		int32					m_nFunIndex;
	};

	//=====================================================================
	// Lua脚本访问C++的成员接口
	//=====================================================================
	class CByScriptMember : public CByScriptBase
	{
		IFunctionWrap*	m_funSet;
	public:
		CByScriptMember( IFunctionWrap* funGetSet[2], const STypeInfoArray& aryTypeInfo, 
			SFunction funOrg, const char* szTypeInfoName, const char* szMemberName );
		virtual void	Call( void* pObject, void* pRetBuf, void** pArgArray, CScriptBase& Script);
		IFunctionWrap*	GetFunSet() const { return m_funSet; }
	};

    //=====================================================================
    // C++调用脚本的基本接口
    //=====================================================================
    class CCallScriptBase 
		: public CByScriptBase
		, public ICallBackWrap
	{
		const CCallScriptBase& operator= ( const CCallScriptBase& );
    public:
		CCallScriptBase( IFunctionWrap* funWrap, const STypeInfoArray& aryTypeInfo, 
			SFunction funOrg, const char* szTypeInfoName, const char* szFunName );
        ~CCallScriptBase();

		void*			GetBootFun()				{ return m_pBootFun; }
		uint32			GetFunIndex()				{ return m_nFunIndex; }

		virtual int32	BindFunction( void* pFun, bool bPureVirtual );
		virtual void	Call( void* pObject, void* pRetBuf, void** pArgArray, CScriptBase& Script );

		int32			Destruc( SVirtualObj* pObject, void* pParam, CScriptBase& Script );
		int32			CallOrg( SVirtualObj* pObject, void* pRetBuf, void** pArgArray, CScriptBase& Script );

	protected:
		void*			m_pBootFun;
		bool			m_bPureVirtual;
	};
}

#endif
