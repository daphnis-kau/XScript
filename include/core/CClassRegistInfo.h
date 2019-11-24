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
	typedef TRBTree<CClassRegistInfo> CTypeIDNameMap;
	typedef CTypeBase* (*MakeTypeFunction)( CClassRegistInfo* pClassInfo, bool bValue );

    class CClassRegistInfo : public CTypeIDNameMap::CRBTreeNode
    {
		gammacstring					m_szClassName;			// 类的名字
		gammacstring					m_szTypeIDName;			// 编译器生成的类型信息

		gammacstring					m_strObjectIndex;		// 对象指针域
        vector<CCallScriptBase*>		m_vecNewFunction;		// 需要注册的函数对应的索引以及对应的引导函数
		vector<SBaseInfo>				m_vecBaseRegist;        // 包含的基类信息
		vector<SBaseInfo>				m_vecChildRegist;       // 包含的子类信息
        IObjectConstruct*				m_pObjectConstruct;
        uint32							m_nSizeOfClass;
        bool							m_bIsCallBack;
		uint8							m_nInheritDepth;
		uint32							m_nInstanceCount;
		CCallBaseMap					m_mapRegistFunction;
		MakeTypeFunction				m_funMakeType;

    public:
        CClassRegistInfo( const char* szClassName, const char* szTypeIDName,
			uint32 nSize, MakeTypeFunction funMakeType );
		~CClassRegistInfo( void );

		operator const gammacstring&( ) const { return m_szTypeIDName; }
		bool operator < ( const gammacstring& strKey ) { return (const gammacstring&)*this < strKey; }

		static CClassRegistInfo*		GetRegistInfo( const char* szTypeInfoName );
		static CCallBase*				GetGlobalCallBase( const STypeInfoArray& aryTypeInfo );

		void							Create( void * pObject );
		void							Assign( void* pDest, void* pSrc );
		void                        	Release( void * pObject );
		CTypeBase*						MakeType( bool bValue );
		void							InitVirtualTable( SFunctionTable* pNewTable );
		void							SetObjectConstruct( IObjectConstruct* pObjectConstruct );
		void                        	RegistClassCallBack( uint32 nIndex, CCallScriptBase* pCallScriptBase );
		void                        	RegistFunction( CCallBase* pCallBase );
        void                            ReplaceVirtualTable( CScriptBase* pScript, void* pObj, bool bNewByVM, uint32 nInheritDepth );
        void                            RecoverVirtualTable( CScriptBase* pScript, void* pObj );
        bool                            IsCallBack();
        void                            AddBaseRegist( CClassRegistInfo* pRegist, ptrdiff_t nOffset );
		int32                       	GetBaseOffset( CClassRegistInfo* pRegist );
		CCallBase*						GetCallBase( const gammacstring& strFunName );
        bool                            FindBase( CClassRegistInfo* pRegistBase );
		bool							IsBaseObject( ptrdiff_t nDiff );
		int32							GetMaxRegisterFunctionIndex();
		const vector<SBaseInfo>&    	BaseRegist() const { return m_vecBaseRegist; }
		const gammacstring&            	GetTypeIDName() const { return m_szTypeIDName; }
		const gammacstring&            	GetClassName() const { return m_szClassName; }
		const gammacstring&            	GetObjectIndex() const{ return m_strObjectIndex; }
        uint32                          GetClassSize() const{ return m_nSizeOfClass; }
		uint8							GetInheritDepth() const{ return m_nInheritDepth; }
		const CCallBaseMap&				GetRegistFunction() const { return m_mapRegistFunction; }
		CCallBaseMap&					GetRegistFunction() { return m_mapRegistFunction; }
		const vector<CCallScriptBase*>& GetNewFunctionList() const { return m_vecNewFunction; }
		uint32							GetInstanceCount() const { return m_nInstanceCount; }
		void							IncInstanceCount() { m_nInstanceCount++; }
		void							DecInstanceCount() { m_nInstanceCount--; }
    }; 
}                                            
                                            
#endif                                        
