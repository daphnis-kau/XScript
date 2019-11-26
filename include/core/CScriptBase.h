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
	typedef pair<SFunctionTable*, uint32> CVMObjVTableInfo;
	typedef map<CClassRegistInfo*, CVMObjVTableInfo> CNewFunctionTableMap;
	typedef map<SFunctionTable*, SFunctionTable*> CFunctionTableMap;

    class CScriptBase : public TList<CScriptBase>::CListNode
	{
		friend class CCallScriptBase;
	protected:
		CDebugBase*				m_pDebugger;
		CCircelBuffer			m_UnlinkObjectBuffer;
		CFunctionTableMap		m_mapVirtualTableOld2New;
		CNewFunctionTableMap	m_mapNewVirtualTable;
        list<string>			m_listSearchPath;

		virtual bool			CallVM( CCallScriptBase* pCallBase, SVirtualObj* pObject, void* pRetBuf, void** pArgArray ) = 0;
		virtual void			DestrucVM( CCallScriptBase* pCallBase, SVirtualObj* pObject ) = 0;
    public:
        CScriptBase(void);
		virtual ~CScriptBase( void );

		static void        		RegistFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName );
		static void        		RegistClassStaticFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName );
		static void				RegistClassFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName );
		static ICallBackWrap&	RegistClassCallback( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName );
		static void				RegistClassMember( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funGetSet[2], const char* szTypeInfoName, const char* szMemberName );
		static void				RegistClass( uint32 nSize, const char* szTypeIDName, const char* szClass, ... );
		static void				RegistConstruct( IObjectConstruct* pObjectConstruct, const char* szTypeIDName );
		static ICallBackWrap&	RegistDestructor( const char* szTypeInfoName, IFunctionWrap* funWrap );
		static void				RegistEnum( const char* szTypeIDName, const char* szTableName, int32 nTypeSize );

        static bool				IsAllocVirtualTable( void* pVirtualTable );
		static void				UnlinkCppObj( void* pObj );
		static int32			CallBack( int32 nIndex, void* pObject, void* pRetBuf, void** pArgArray );

		CDebugBase*				GetDebugger() const { return m_pDebugger; }
		void					CheckUnlinkCppObj();
		bool					IsVirtualTableValid( SVirtualObj* pVObj );
        SFunctionTable*			GetOrgVirtualTable( void* pObj );
		SFunctionTable*     	CheckNewVirtualTable( SFunctionTable* pOldFunTable, CClassRegistInfo* pClassInfo, bool bNewByVM, uint32 nInheritDepth );
        void                	AddSearchPath( const char* szPath );
		int						Input( char* szBuffer, int nCount );

		virtual bool        	RunFile( const char* szFileName, bool bReload ) = 0;
		virtual bool        	RunBuffer( const void* pBuffer, size_t nSize ) = 0;
		virtual bool        	RunString( const char* szString ) = 0;
		virtual bool        	RunFunction( const STypeInfoArray& aryTypeInfo, void* pResultBuf, const char* szFunction, void** aryArg ) = 0;
        virtual void			RefScriptObj( void* pObj ) = 0;
        virtual void			UnrefScriptObj( void* pObj ) = 0;
		virtual void			UnlinkCppObjFromScript( void* pObj ) = 0;
		virtual void        	GC() = 0;
		virtual void        	GCAll() = 0;

		template<typename RetType, typename... Param>
		bool					RunFunction( RetType* pRetBuf, const char* szFun, Param ... p );
		template<typename... Param>
		bool					RunFunction( nullptr_t, const char* szFun, Param ... p );
	};

	template<typename RetType, typename... Param>
	bool CScriptBase::RunFunction( RetType* pRetBuf, const char* szFun, Param ... p )
	{
		void* aryParam[sizeof...( p ) + 1] = { &p ... };
		static STypeInfo aryInfo[] = { GetTypeInfo<Param>()..., GetTypeInfo<RetType>() };
		static STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
		return RunFunction( TypeInfo, pRetBuf, szFun, aryParam );
	}

	template<typename... Param>
	bool CScriptBase::RunFunction( nullptr_t, const char* szFun, Param ... p )
	{
		void* aryParam[sizeof...( p ) + 1] = { &p ... };
		static STypeInfo aryInfo[] = { GetTypeInfo<Param>()..., GetTypeInfo<void>() };
		static STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
		return RunFunction( TypeInfo, nullptr, szFun, aryParam );
	}

}

#endif
