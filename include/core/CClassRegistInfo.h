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

namespace Gamma
{
	class CTypeBase;
	class CScriptBase;
	class CByScriptBase;
	class CCallScriptBase;
	class CClassRegistInfo; 
	class CGlobalClassRegist;
	typedef TRBTree<CByScriptBase> CCallBaseMap;
	typedef TRBTree<CClassRegistInfo> CTypeIDNameMap;

    class CClassRegistInfo : public CTypeIDNameMap::CRBTreeNode
	{
		struct SBaseInfo
		{
			const CClassRegistInfo*		m_pBaseInfo;        // 包含的基类
			int32						m_nBaseOff;         // 包含的基类相对于子类的偏移
		};

		gammacstring					m_szClassName;		// 类的名字
		gammacstring					m_szTypeIDName;		// 编译器生成的类型信息

		std::vector<CCallScriptBase*>	m_vecOverridableFun;// 可重写的函数
		std::vector<SBaseInfo>			m_vecBaseRegist;    // 包含的基类信息
		std::vector<SBaseInfo>			m_vecChildRegist;   // 包含的子类信息
        IObjectConstruct*				m_pObjectConstruct;
		uint32							m_nSizeOfClass;
		uint32							m_nAligenSizeOfClass;
		bool							m_bIsEnum;
		uint8							m_nInheritDepth;
		CCallBaseMap					m_mapRegistFunction;
		
		friend class CGlobalClassRegist;
		CClassRegistInfo(const char* szClassName);
		~CClassRegistInfo( void );
    public:

		operator const gammacstring&( ) const { return m_szTypeIDName; }
		bool operator < ( const gammacstring& strKey ) { return (const gammacstring&)*this < strKey; }

		static const CClassRegistInfo*	RegisterClass( const char* szClassName, const char* szTypeIDName, uint32 nSize, bool bEnum );
		static const CClassRegistInfo*	GetRegistInfo( const char* szTypeInfoName );
		static const CClassRegistInfo*	SetObjectConstruct( const char* szTypeInfoName, IObjectConstruct* pObjectConstruct );
		static const CClassRegistInfo*	AddBaseRegist( const char* szTypeInfoName, const char* szBaseTypeInfoName, ptrdiff_t nOffset );
		static const CByScriptBase*		RegisterFunction( const char* szTypeInfoName, CByScriptBase* pCallBase );
		static const CByScriptBase*		RegisterCallBack( const char* szTypeInfoName, uint32 nIndex, CCallScriptBase* pCallScriptBase );
		static const CTypeIDNameMap&	GetAllRegisterInfo();

		void							Create( void * pObject ) const;
		void							Assign( void* pDest, void* pSrc ) const;
		void                        	Release( void * pObject ) const;
		void							InitVirtualTable( SFunctionTable* pNewTable ) const;
		int32							GetMaxRegisterFunctionIndex() const;
        void                            ReplaceVirtualTable( CScriptBase* pScript, void* pObj, bool bNewByVM, uint32 nInheritDepth ) const;
		void                            RecoverVirtualTable(CScriptBase* pScript, void* pObj) const;
		bool                            IsCallBack() const;
		int32                       	GetBaseOffset( const CClassRegistInfo* pRegist ) const;
		const CByScriptBase*			GetCallBase( const gammacstring& strFunName ) const;
        bool                            FindBase( const CClassRegistInfo* pRegistBase ) const;
		bool							IsBaseObject(ptrdiff_t nDiff) const;
		bool							IsEnum() const { return m_bIsEnum; }
		const std::vector<SBaseInfo>&  	BaseRegist() const { return m_vecBaseRegist; }
		const gammacstring&            	GetTypeIDName() const { return m_szTypeIDName; }
		const gammacstring&            	GetClassName() const { return m_szClassName; }
		const gammacstring&            	GetObjectIndex() const { return m_szTypeIDName; }
		uint32                          GetClassSize() const { return m_nSizeOfClass; }
		uint32                          GetClassAligenSize() const { return m_nAligenSizeOfClass; }
		uint8							GetInheritDepth() const { return m_nInheritDepth; }
		const CCallBaseMap&				GetRegistFunction() const { return m_mapRegistFunction; }
		const CCallScriptBase*			GetOverridableFunction( int32 nIndex ) const { return m_vecOverridableFun[nIndex]; }
    }; 
}                                            
                                            
#endif                                        
