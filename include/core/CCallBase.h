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
		eCT_TempFunction		= -5,
		eCT_GlobalFunction		= -4,
		eCT_ClassStaticFunction	= -3,
		eCT_ClassFunction		= -2,
		eCT_MemberFunction		= -1,
		eCT_ClassCallBack		= 0,
	};

	class CCallBase;
	class CScriptBase;
	typedef TRBTree<CCallBase> CCallBaseMap;

	//=====================================================================
	// 函数调用的基本接口
	//=====================================================================
    class CCallBase : public CCallBaseMap::CRBTreeNode
	{
		const CCallBase& operator= (const CCallBase&);
	protected:
		gammacstring			m_sFunName;
		DataType				m_nThis;
		DataType				m_nResult;
		vector<DataType>		m_listParam;
		uint32					m_nParamCount;
		int32					m_nFunIndex;

    public:
        CCallBase( const STypeInfoArray& aryTypeInfo, int32 nFunIndex, 
			const char* szTypeInfoName, gammacstring strFunName );
		operator const gammacstring&( ) const { return m_sFunName; }
		bool operator < ( const gammacstring& strKey ) { return (const gammacstring&)*this < strKey; }

		virtual ~CCallBase(void);

		const vector<DataType>&	GetParamList()		const { return m_listParam; }
		DataType				GetResultType()		const { return m_nResult; }
		DataType				GetThisType()		const { return m_nThis; }
		uint32					GetParamCount()		const { return m_nParamCount; }
		int32					GetFunctionIndex()	const { return m_nFunIndex; }
		const gammacstring&		GetFunctionName()	const { return m_sFunName; }
		bool					IsCallback()		const { return m_nFunIndex >= eCT_ClassCallBack; }
    };

    //=====================================================================
    // 脚本调用C++的基本接口
    //=====================================================================
    class CByScriptBase 
		: public CCallBase
    {
		const CByScriptBase& operator= ( const CByScriptBase& );
    public:
		CByScriptBase(const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap,
			const char* szTypeInfoName, int32 nFunIndex, const char* szFunName);

		virtual void	Call(void* pObject, void* pRetBuf, void** pArgArray, CScriptBase& Script);
		IFunctionWrap*	GetFunWrap() const { return m_funWrap; }
	protected:
		IFunctionWrap*	m_funWrap;
	};

	//=====================================================================
	// Lua脚本访问C++的成员接口
	//=====================================================================
	class CByScriptMember : public CByScriptBase
	{
		IFunctionWrap*	m_funSet;
	public:
		CByScriptMember( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funGetSet[2], 
			const char* szTypeInfoName, const char* szFunName );
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
        CCallScriptBase( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, 
			const char* szTypeInfoName, const char* szFunName );
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
