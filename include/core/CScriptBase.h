#ifndef __SCRIPT_BASE_H__
#define __SCRIPT_BASE_H__
//=====================================================================
// CScriptBase.h 
// 定义脚本和C++接口的基类，不同的脚本虚拟机可以从这里派生不同的接口代码
// 柯达昭
// 2007-10-16
//=====================================================================

#include "common/TList.h"
#include "common/TCircelBuffer.h"
#include "CClassRegistInfo.h"
#include <stdarg.h>
#include <vector>
#include <list>
#include <map>
#include <set>

using namespace std;

namespace Gamma
{
	class CTypeBase;
	class CCallBase;
	class CDebugBase;
	typedef map<SFunctionTable*, SFunctionTable*> CFunctionTableMap;

    class CScriptBase : public TList<CScriptBase>::CListNode
	{
	protected:
		CDebugBase*				m_pDebugger;
		CCircelBuffer			m_UnlinkObjectBuffer;
        CTypeIDNameMap			m_mapTypeID2ClassInfo;
        CClassNameMap			m_mapRegistClassInfo;
        CFunctionTableMap		m_mapVirtualTableOld2New;
        map<string,int32>		m_mapSizeOfEnum;
        list<string>			m_listSearchPath;

    public:
        CScriptBase(void);
		virtual ~CScriptBase( void );
		friend void UnlinkCppObj( void* );

		CDebugBase*				GetDebugger() const { return m_pDebugger; }

        virtual CTypeBase*		MakeParamType( const STypeInfo& argTypeInfo ) = 0;
		static bool				IsAllocVirtualTable( void* pVirtualTable );
								
		void					CheckUnlinkCppObj();
        SFunctionTable*			GetOrgVirtualTable( void* pObj );
		SFunctionTable*     	CheckNewVirtualTable( SFunctionTable* pOldFunTable, CClassRegistInfo* pClassInfo, bool bNewByVM, uint32 nInheritDepth );
        CClassRegistInfo*		GetRegistInfo( const char* szClassName );
		CClassRegistInfo*   	GetRegistInfoByTypeInfoName( const char* szTypeInfoName );
		CCallBase*				GetGlobalCallBase( const STypeInfoArray& aryTypeInfo );
		void                	AddSearchPath( const char* szPath );
		int						Input( char* szBuffer, int nCount );

		virtual bool        	RunFile( const char* szFileName, bool bReload ) = 0;
		virtual bool        	RunBuffer( const void* pBuffer, size_t nSize ) = 0;
		virtual bool        	RunString( const char* szString ) = 0;
		virtual bool        	RunFunction( const STypeInfoArray& aryTypeInfo, void* pResultBuf, const char* szFunction, void** aryArg ) = 0;
		virtual void        	RegistFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName ) = 0;
		virtual void        	RegistClassStaticFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName ) = 0;
        virtual void			RegistClassFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName ) = 0;
		virtual ICallBackWrap&	RegistClassCallback( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName ) = 0;
		virtual ICallBackWrap&	RegistDestructor( const char* szTypeInfoName, IFunctionWrap* funWrap ) = 0;
        virtual void			RegistClassMember( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funGetSet[2], const char* szTypeInfoName, const char* szMemberName ) = 0;
		virtual void			RegistClass( uint32 nSize, const char* szTypeIDName, const char* szClass, ... ) = 0;
		virtual void			RegistConstruct( IObjectConstruct* pObjectConstruct, const char* szTypeIDName ) = 0;
		virtual void			RegistEnum( const char* szTypeIDName, const char* szTableName, int32 nTypeSize ) = 0;
        virtual void			RegistConstant( const char* szTableName, const char* szFeild, int32 nValue ) = 0;
		virtual void			RegistConstant( const char* szTableName, const char* szFeild, double dValue ) = 0;
		virtual void			RegistConstant( const char* szTableName, const char* szFeild, const char* szValue ) = 0;
        virtual void			RefScriptObj( void* pObj ) = 0;
        virtual void			UnrefScriptObj( void* pObj ) = 0;
		virtual void			UnlinkCppObjFromScript( void* pObj ) = 0;
		virtual void        	GC() = 0;
		virtual void        	GCAll() = 0;
	};
}

#endif
