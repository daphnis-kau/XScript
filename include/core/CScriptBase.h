/**@file  		CScriptBase.h
* @brief		Script VM base wrapper
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
*/

#ifndef __SCRIPT_BASE_H__
#define __SCRIPT_BASE_H__
#include "common/TList.h"
#include "CClassInfo.h"
#include <stdarg.h>
#include <vector>
#include <list>
#include <map>
#include <set>

namespace XS
{
	class CClassInfo;
	class CDebugBase;
	class CCallInfo;
	class CCallbackInfo;
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

		virtual bool			Set( void* pObject, int32 nIndex, void* pArgBuf, const STypeInfo& TypeInfo ) = 0;
		virtual bool			Get( void* pObject, int32 nIndex, void* pResultBuf, const STypeInfo& TypeInfo ) = 0;
		virtual bool			Set( void* pObject, const char* szName, void* pArgBuf, const STypeInfo& TypeInfo ) = 0;
		virtual bool			Get( void* pObject, const char* szName, void* pResultBuf, const STypeInfo& TypeInfo ) = 0;
		virtual bool        	Call( const STypeInfoArray& aryTypeInfo, void* pResultBuf, const char* szFunction, void** aryArg ) = 0;
		virtual bool			Call( const STypeInfoArray& aryTypeInfo, void* pResultBuf, void* pFunction, void** aryArg ) = 0;
		virtual bool        	RunBuffer( const void* pBuffer, size_t nSize, const char* szFileName, bool bForceBuild = false ) = 0;
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
		static bool				RegisterEnumType( const char* szTypeIDName, const char* szEnumType, int32 nTypeSize );
		static bool				RegisterEnumValue( const char* szTypeIDName, const char* szEnumValue, int32 nValue );

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

		virtual int32			IncRef( void* pObj ) = 0;
		virtual int32			DecRef( void* pObj ) = 0;
		virtual void			UnlinkCppObjFromScript( void* pObj ) = 0;
		virtual bool			IsValid( void* pObject ) = 0;
		virtual void        	GC() = 0;
		virtual void        	GCAll() = 0;

		bool        			RunFile( const char* szFileName, bool bForce = false);
		bool        			RunString( const char* szString );

		template<typename Type>
		bool					Evaluate( const char* szExpression, Type& nValue );
		template<typename IndexType, typename ValueType>
		bool					SetField( void* pObject, IndexType Index, ValueType nValue );
		template<typename IndexType, typename ValueType>
		bool					GetField( void* pObject, IndexType Index, ValueType& nValue );

		template<typename FunType, typename RetType, typename... Param>
		bool					RunFunction( RetType* pRetBuf, FunType pFun, Param ... p );
		template<typename FunType, typename... Param>
		bool					RunFunction( nullptr_t, FunType pFun, Param ... p );

		std::string				ReadEntirFile( const char* szFileName );
	};

	template<typename Type>
	bool CScriptBase::Evaluate( const char* szExpression, Type& nValue )
	{
		CheckDebugCmd();
		static STypeInfo TypeInfo = GetTypeInfo<Type>();
		return Get( nullptr, szExpression, &nValue, TypeInfo );
	}

	template<typename IndexType, typename ValueType>
	bool CScriptBase::SetField( void* pObject, IndexType Index, ValueType nValue )
	{
		CheckDebugCmd();
		static STypeInfo TypeInfo = GetTypeInfo<ValueType>();
		return Set( pObject, Index, &nValue, TypeInfo );
	}

	template<typename IndexType, typename ValueType>
	bool CScriptBase::GetField( void* pObject, IndexType Index, ValueType& nValue )
	{
		CheckDebugCmd();
		static STypeInfo TypeInfo = GetTypeInfo<ValueType>();
		return Get( pObject, Index, &nValue, TypeInfo );
	}

	template<typename FunType, typename RetType, typename... Param>
	bool CScriptBase::RunFunction( RetType* pRetBuf, FunType pFun, Param ... p )
	{
		CheckDebugCmd();
		void* aryParam[sizeof...( p ) + 1] = { &p ... };
		static STypeInfo aryInfo[] = { GetTypeInfo<Param>()..., GetTypeInfo<RetType>() };
		static STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
		return Call( TypeInfo, pRetBuf, pFun, aryParam );
	}

	template<typename FunType, typename... Param>
	bool CScriptBase::RunFunction( nullptr_t, FunType pFun, Param ... p )
	{
		CheckDebugCmd();
		void* aryParam[sizeof...( p ) + 1] = { &p ... };
		static STypeInfo aryInfo[] = { GetTypeInfo<Param>()..., GetTypeInfo<void>() };
		static STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
		return Call( TypeInfo, nullptr, pFun, aryParam );
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
