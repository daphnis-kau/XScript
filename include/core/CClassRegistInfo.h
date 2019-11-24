#ifndef __CLASS_REGIST_INFO_H__
#define __CLASS_REGIST_INFO_H__
//=====================================================================
/** CClassRegistInfo.h
// 定义注册的类型的继承关系
// 柯达昭
// 2007-10-16
*/
//=====================================================================

#include "common/TRBTree.h"
#include "common/CVirtualFun.h"
#include "common/TConstString.h"
#include "core/GammaScriptDef.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

namespace Gamma
{
	class CCallBase;
	class CTypeBase;
	class CScriptBase;
	class CCallScriptBase;
	class CClassRegistInfo;

    struct SBaseInfo
    {
        CClassRegistInfo*		m_pBaseInfo;        // 包含的基类
        int32					m_nBaseOff;         // 包含的基类相对于子类的偏移
	};

	class CClassName;
	class CTypeIDName;
	typedef TRBTree<CClassName> CClassNameMap;
	typedef TRBTree<CTypeIDName> CTypeIDNameMap;
	typedef pair<SFunctionTable*, uint32> CVMObjVTableInfo;
	typedef CTypeBase* (*MakeTypeFunction)( CClassRegistInfo* pClassInfo, bool bValue );

	class CClassName 
		: public CClassNameMap::CRBTreeNode
	{
	protected:
		CClassName( const char* szClassName ) : m_szClassName( szClassName ){}
		gammacstring					m_szClassName;			// 类的名字
	public:
		operator const gammacstring&() const { return m_szClassName; }
		bool operator < ( const gammacstring& strKey ) { return (const gammacstring&)*this < strKey; }

	};

	class CTypeIDName 
		: public CTypeIDNameMap::CRBTreeNode
	{
	protected:
		CTypeIDName( const char* szTypeName ) : m_szTypeIDName( szTypeName ){}
		gammacstring					m_szTypeIDName;			// 编译器生成的类型信息
	public:
		operator const gammacstring&() const { return m_szTypeIDName; }
		bool operator < ( const gammacstring& strKey ) { return (const gammacstring&)*this < strKey; }

	};

    class CClassRegistInfo 
		: public CClassName
		, public CTypeIDName
    {
        CScriptBase*					m_pScriptBase; 
        CVMObjVTableInfo				m_VMObjVTableInfo;
		gammacstring					m_strObjectIndex;		// 对象指针域
        vector<CCallScriptBase*>		m_vecNewFunction;		// 需要注册的函数对应的索引以及对应的引导函数
		vector<SBaseInfo>				m_vecBaseRegist;        // 包含的基类信息
		vector<SBaseInfo>				m_vecChildRegist;       // 包含的子类信息
        IObjectConstruct*				m_pObjectConstruct;
        uint32							m_nSizeOfClass;
        bool							m_bIsCallBack;
		uint8							m_nInheritDepth;
		uint32							m_nInstanceCount;
		map<string,CCallBase*>			m_mapRegistFunction;	
		MakeTypeFunction				m_funMakeType;

    public:
        CClassRegistInfo( CScriptBase* pScriptBase, 
			const char* szClassName, const char* szTypeIDName,
			uint32 nSize, MakeTypeFunction funMakeType );
        ~CClassRegistInfo(void);

		void							Create( void * pObject );
		void							Assign( void* pDest, void* pSrc );
		void                        	Release( void * pObject );
		CTypeBase*						MakeType( bool bValue );
		void							InitVirtualTable( SFunctionTable* pNewTable );
		void							SetObjectConstruct( IObjectConstruct* pObjectConstruct );
		void                        	RegistClassCallBack( uint32 nIndex, CCallScriptBase* pCallScriptBase );
		void                        	RegistFunction( const string& szFunName, CCallBase* pCallBase );
        void                            ReplaceVirtualTable( void* pObj, bool bNewByVM, uint32 nInheritDepth );
        void                            RecoverVirtualTable( void* pObj );
        bool                            IsCallBack();
        void                            AddBaseRegist( CClassRegistInfo* pRegist, ptrdiff_t nOffset );
		int32                       	GetBaseOffset( CClassRegistInfo* pRegist );
        CClassRegistInfo*               GetChildRegist( void* pVirtualTable );
		CCallBase*						GetCallBase( const string& strFunName );
        bool                            FindBase( CClassRegistInfo* pRegistBase );
		bool							IsBaseObject( ptrdiff_t nDiff );
		int32							GetMaxRegisterFunctionIndex();
		CScriptBase*					GetScript() const { return m_pScriptBase; }
		const vector<SBaseInfo>&    	BaseRegist() const { return m_vecBaseRegist; }
		const gammacstring&            	GetTypeIDName() const { return m_szTypeIDName; }
		const gammacstring&            	GetClassName() const { return m_szClassName; }
		const gammacstring&            	GetObjectIndex() const{ return m_strObjectIndex; }
        uint32                          GetClassSize() const{ return m_nSizeOfClass; }
		uint8							GetInheritDepth() const{ return m_nInheritDepth; }
		CVMObjVTableInfo&				GetVMObjectVTbl() { return m_VMObjVTableInfo; }
		const map<string,CCallBase*>&	GetRegistFunction() const { return m_mapRegistFunction; }
		map<string,CCallBase*>&			GetRegistFunction() { return m_mapRegistFunction; }
		const vector<CCallScriptBase*>& GetNewFunctionList() const { return m_vecNewFunction; }
		uint32							GetInstanceCount() const { return m_nInstanceCount; }
		void							IncInstanceCount() { m_nInstanceCount++; }
		void							DecInstanceCount() { m_nInstanceCount--; }
    }; 
}                                            
                                            
#endif                                        
