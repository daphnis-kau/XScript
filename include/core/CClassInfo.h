/**@file  		CClassInfo.h
* @brief		Register information of class 
* @author		Daphnis Kaw
* @date			2020-01-17
* @version		V1.0
*/

#ifndef __CLASS_REGIST_INFO_H__
#define __CLASS_REGIST_INFO_H__

#include "common/TRBTree.h"
#include "common/CVirtualFun.h"
#include "common/TConstString.h"
#include "core/XScriptDef.h"
#include <string>
#include <vector>
#include <map>

namespace XS
{
	class CTypeBase;
	class CScriptBase;
	class CCallInfo;
	class CCallbackInfo;
	class CClassInfo;
	class CGlobalClassRegist;
	typedef TRBTree<CCallInfo> CCallBaseMap;
	typedef TRBTree<CClassInfo> CTypeIDNameMap;

    class CClassInfo : public CTypeIDNameMap::CRBTreeNode
	{
		struct SBaseInfo
		{
			const CClassInfo*			m_pBaseInfo;        // Base class information
			int32						m_nBaseOff;         // Base class offset
		};

		const_string					m_szClassName;		// Name of class
		const_string					m_szTypeIDName;		// typeid of the class

		std::vector<CCallbackInfo*>		m_vecOverridableFun;// Overridable function information
		std::vector<SBaseInfo>			m_vecBaseRegist;    // All base classes' information
		std::vector<SBaseInfo>			m_vecChildRegist;   // All subclass information
        IObjectConstruct*				m_pObjectConstruct;
		uint32							m_nSizeOfClass;
		uint32							m_nAligenSizeOfClass;
		bool							m_bIsEnum;
		uint8							m_nInheritDepth;
		CCallBaseMap					m_mapRegistFunction;
		
		friend class CGlobalClassRegist;
		CClassInfo(const char* szClassName);
		~CClassInfo( void );
    public:

		operator const const_string&( ) const { return m_szTypeIDName; }
		bool operator < ( const const_string& strKey ) { return (const const_string&)*this < strKey; }

		static const CClassInfo*		RegisterClass( const char* szClassName, const char* szTypeIDName, uint32 nSize, bool bEnum );
		static const CClassInfo*		GetClassInfo( const char* szTypeInfoName );
		static const CClassInfo*		SetObjectConstruct( const char* szTypeInfoName, IObjectConstruct* pObjectConstruct );
		static const CClassInfo*		AddBaseInfo( const char* szTypeInfoName, const char* szBaseTypeInfoName, ptrdiff_t nOffset );
		static const CCallInfo*			RegisterFunction( const char* szTypeInfoName, CCallInfo* pCallBase );
		static const CCallInfo*			RegisterCallBack( const char* szTypeInfoName, uint32 nIndex, CCallbackInfo* pCallScriptBase );
		static const CTypeIDNameMap&	GetAllRegisterInfo();

		void							Create( CScriptBase* pScript, void * pObject ) const;
		void							Assign( CScriptBase* pScript, void* pDest, void* pSrc ) const;
		void                        	Release( CScriptBase* pScript, void * pObject ) const;
		void							InitVirtualTable( SFunctionTable* pNewTable ) const;
		int32							GetMaxRegisterFunctionIndex() const;
        void                            ReplaceVirtualTable( CScriptBase* pScript, void* pObj, bool bNewByVM, uint32 nInheritDepth ) const;
		void                            RecoverVirtualTable( CScriptBase* pScript, void* pObj ) const;
		bool                            IsCallBack() const;
		int32                       	GetBaseOffset( const CClassInfo* pRegist ) const;
		const CCallInfo*				GetCallBase( const const_string& strFunName ) const;
        bool                            FindBase( const CClassInfo* pRegistBase ) const;
		bool							IsBaseObject(ptrdiff_t nDiff) const;
		bool							IsEnum() const { return m_bIsEnum; }
		const std::vector<SBaseInfo>&  	BaseRegist() const { return m_vecBaseRegist; }
		const const_string&            	GetTypeIDName() const { return m_szTypeIDName; }
		const const_string&            	GetClassName() const { return m_szClassName; }
		const const_string&            	GetObjectIndex() const { return m_szTypeIDName; }
		uint32                          GetClassSize() const { return m_nSizeOfClass; }
		uint32                          GetClassAligenSize() const { return m_nAligenSizeOfClass; }
		uint8							GetInheritDepth() const { return m_nInheritDepth; }
		const CCallBaseMap&				GetRegistFunction() const { return m_mapRegistFunction; }
		const CCallbackInfo*			GetOverridableFunction( int32 nIndex ) const { return m_vecOverridableFun[nIndex]; }
    }; 
}                                            
                                            
#endif                                        
