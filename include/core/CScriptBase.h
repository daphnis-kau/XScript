#ifndef __SCRIPT_BASE_H__
#define __SCRIPT_BASE_H__
//=====================================================================
// CScriptBase.h 
// 定义脚本和C++接口的基类，不同的脚本虚拟机可以从这里派生不同的接口代码
// 柯达昭
// 2007-10-16
//=====================================================================

#include "common/TList.h"
#include "CClassInfo.h"
#include <stdarg.h>
#include <vector>
#include <list>
#include <map>
#include <set>

namespace XS
{
	class CDebugBase;
	class CCallInfo;
	typedef std::pair<SFunctionTable*, uint32> CVMObjVTableInfo;
	typedef std::map<const CClassInfo*, CVMObjVTableInfo> CNewFunctionTableMap;
	typedef std::map<SFunctionTable*, SFunctionTable*> CFunctionTableMap;

    class CScriptBase
	{
		friend class CCallbackInfo;
	protected:
		static std::string		s_CacheTruckPrefix;

		CDebugBase*				m_pDebugger;
		CFunctionTableMap		m_mapVirtualTableOld2New;
		CNewFunctionTableMap	m_mapNewVirtualTable;
		std::list<std::string>	m_listSearchPath;
		std::set<const_string>	m_setRuningString;

		virtual bool			CallVM( const CCallbackInfo* pCallBase, void* pRetBuf, void** pArgArray ) = 0;
		virtual void			DestrucVM( const CCallbackInfo* pCallBase, SVirtualObj* pObject ) = 0;

		virtual bool        	RunFunction( const STypeInfoArray& aryTypeInfo, void* pResultBuf, const char* szFunction, void** aryArg ) = 0;
		virtual bool        	RunBuffer( const void* pBuffer, size_t nSize, const char* szFileName ) = 0;
    public:
        CScriptBase(void);
		virtual ~CScriptBase( void );

		static bool        		RegisterGlobalFunction( IFunctionWrap* funWrap, uintptr_t funOrg,
									const STypeInfoArray& aryTypeInfo, const char* szTypeInfoName, const char* szFunctionName );
		static bool				RegisterClassFunction( IFunctionWrap* funWrap, uintptr_t funOrg,
									const STypeInfoArray& aryTypeInfo, const char* szFunctionName );
		static bool				RegisterClassCallback( IFunctionWrap* funWrap, uintptr_t funBoot, uint32 nFunIndex, 
									bool bPureVirtual, const STypeInfoArray& aryTypeInfo, const char* szFunctionName );
		static bool				RegisterClassMember( IFunctionWrap* funGetSet[2], uintptr_t nOffset,
									const STypeInfoArray& aryTypeInfo, const char* szMemberName );
		static bool				RegisterDestructor( IFunctionWrap* funWrap, uintptr_t funBoot, 
									uint32 nFunIndex, const STypeInfoArray& aryTypeInfo );
		static bool				RegisterConstruct( IObjectConstruct* pObjectConstruct, const char* szTypeIDName );
		static bool				RegisterClass( const char* szClass, uint32 nCount, const char** aryType, const ptrdiff_t* aryValue);
		static bool				RegisterEnum( const char* szTypeIDName, const char* szEnumName, int32 nTypeSize );

		static void				CallBack( int32 nIndex, void* pRetBuf, void** pArgArray );

		CDebugBase*				GetDebugger() const { return m_pDebugger; }
		void					CheckDebugCmd();
		bool					IsVirtualTableValid( SVirtualObj* pVObj );
        SFunctionTable*			GetOrgVirtualTable( void* pObj );
		SFunctionTable*     	CheckNewVirtualTable( SFunctionTable* pOldFunTable, const CClassInfo* pClassInfo, bool bNewByVM, uint32 nInheritDepth );
        void                	AddSearchPath( const char* szPath );

		virtual int32			Input( char* szBuffer, int nCount );
		virtual int32			Output( const char* szBuffer, int nCount );

		virtual void*			OpenFile( const char* szFileName );
		virtual int32			ReadFile( void* pContext, char* szBuffer, int32 nCount );
		virtual void			CloseFile( void* pContext );

		virtual void			UnlinkCppObjFromScript( void* pObj ) = 0;
		virtual void        	GC() = 0;
		virtual void        	GCAll() = 0;

		bool        			RunFile( const char* szFileName );
		bool        			RunString( const char* szString );

		template<typename RetType, typename... Param>
		bool					RunFunction( RetType* pRetBuf, const char* szFun, Param ... p );
		template<typename... Param>
		bool					RunFunction( nullptr_t, const char* szFun, Param ... p );

		std::string				ReadEntirFile( const char* szFileName );
	};

	template<typename RetType, typename... Param>
	bool CScriptBase::RunFunction( RetType* pRetBuf, const char* szFun, Param ... p )
	{
		CheckDebugCmd();
		void* aryParam[sizeof...( p ) + 1] = { &p ... };
		static STypeInfo aryInfo[] = { GetTypeInfo<Param>()..., GetTypeInfo<RetType>() };
		static STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
		return RunFunction( TypeInfo, pRetBuf, szFun, aryParam );
	}

	template<typename... Param>
	bool CScriptBase::RunFunction( nullptr_t, const char* szFun, Param ... p )
	{
		CheckDebugCmd();
		void* aryParam[sizeof...( p ) + 1] = { &p ... };
		static STypeInfo aryInfo[] = { GetTypeInfo<Param>()..., GetTypeInfo<void>() };
		static STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
		return RunFunction( TypeInfo, nullptr, szFun, aryParam );
	}
	
	inline std::string CScriptBase::ReadEntirFile( const char* szFileName )
	{
		std::string strBuffer;
		void* pContext = OpenFile( szFileName );
		if( !pContext )
			return strBuffer;
		char szBuffer[1024];
		int32 nReadSize = 0;
		while( ( nReadSize = ReadFile( pContext, szBuffer, 1024 ) ) > 0 )
			strBuffer.append( szBuffer, nReadSize );
		CloseFile( pContext );
		return strBuffer;
	}
}

#endif
