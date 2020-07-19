/**@file  		XScriptWrap.h
* @brief		Template wrapper of XScript
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
*/

#pragma once
#include <array>
#include "core/CScriptBase.h"

namespace XS
{
	///< Function calling wrapper
	template< typename T >
	struct ArgFetcher
	{
		static inline T& CallWrapArg( void* pData )
		{ 
			return *(T*)pData; 
		}

		static inline void* CallBackArg( T*& pData )
		{ 
			return pData; 
		}
	};

	template<typename T>
	struct ArgFetcher<T&>
	{
		static inline T& CallWrapArg( void* pData ) 
		{ 
			return **(T**)pData; 
		}

		static inline void* CallBackArg( T*& pData ) 
		{
			return &pData; 
		}
	};

	template<> struct ArgFetcher<const char		&> : public ArgFetcher<char		> {};
	template<> struct ArgFetcher<const int8		&> : public ArgFetcher<int8		> {};
	template<> struct ArgFetcher<const int16	&> : public ArgFetcher<int16	> {};
	template<> struct ArgFetcher<const int32	&> : public ArgFetcher<int32	> {};
	template<> struct ArgFetcher<const int64	&> : public ArgFetcher<int64	> {};
	template<> struct ArgFetcher<const long		&> : public ArgFetcher<long		> {};
	template<> struct ArgFetcher<const wchar_t	&> : public ArgFetcher<wchar_t	> {};
	template<> struct ArgFetcher<const uint8	&> : public ArgFetcher<uint8	> {};
	template<> struct ArgFetcher<const uint16	&> : public ArgFetcher<uint16	> {};
	template<> struct ArgFetcher<const uint32	&> : public ArgFetcher<uint32	> {};
	template<> struct ArgFetcher<const uint64	&> : public ArgFetcher<uint64	> {};
	template<> struct ArgFetcher<const ulong	&> : public ArgFetcher<ulong	> {};
	template<> struct ArgFetcher<const float	&> : public ArgFetcher<float	> {};
	template<> struct ArgFetcher<const double	&> : public ArgFetcher<double	> {};

	///< Capture compile error
	template<bool bCompileSucceed> struct TCompileSucceed {};
	template<> struct TCompileSucceed<true> { struct Succeeded {}; };

	///< Check size of the giving class are equal
	template<typename _T1, typename _T2> struct TClassSizeEqual
	{ 
		typedef TCompileSucceed<sizeof(_T1) == sizeof(_T2)> CCompileSucceed;
		typedef typename CCompileSucceed::Succeeded Succeeded;
	};

	///< Fetch function type
	template<typename RetType, typename... Param>
	inline STypeInfoArray MakeFunArg()
	{
		static STypeInfo aryInfo[] =
		{ GetTypeInfo<Param>()..., GetTypeInfo<RetType>() };
		STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
		return TypeInfo;
	}

	///< Construct Type
	enum EConstructType
	{
		eConstructType_Normal,
		eConstructType_Unduplicatable,
		eConstructType_Abstract,
	};

	///< Get original virtual table
	typedef SFunctionTable* ( *GetVirtualTableFun )( void* );
	template<typename _T, bool bDuplicatable> struct TCopy 
	{ TCopy( void* pDest, void* pSrc ) { *(_T*)pDest = *(_T*)pSrc; } };
	template<typename _T> struct TCopy<_T, false> 
	{ TCopy( void* pDest, void* pSrc ) { throw( "Can not duplicate object" ); } };
	template<typename _T, bool bDuplicatable> struct TCopyConstruct
	{ TCopyConstruct( void* pDest, void* pSrc ) { new( pDest )_T( *(_T*)pSrc ); } };
	template<typename _T> struct TCopyConstruct<_T, false>
	{ TCopyConstruct( void* pDest, void* pSrc ) { throw( "Can not duplicate object" ); } };

	template<typename ClassType, EConstructType eType, typename... Param>
	struct TGetVTable : public ClassType
	{
		static GetVirtualTableFun& GetFunInst()
		{
			static GetVirtualTableFun s_fun;
			return s_fun;
		}

		static XS::SFunctionTable*& GetVTbInst()
		{
			static XS::SFunctionTable* s_table;
			return s_table;
		}

		TGetVTable( Param...p )
			: ClassType( p... )
		{
			if( !GetFunInst() || GetVTbInst() )
				return;
			GetVTbInst() = GetFunInst()( this );
		}
	};

	template<typename ClassType>
	struct TGetVTable<ClassType, eConstructType_Abstract>
	{
		static GetVirtualTableFun& GetFunInst()
		{
			static GetVirtualTableFun s_fun;
			return s_fun;
		}
	};

	template<typename ClassType, typename... RemainParam> 
	struct TConstructFromParam {};
	template<typename ClassType> 
	struct TConstructFromParam<ClassType>
	{
		template<typename... FetchParam>
		static ClassType* Construct( size_t nIndex, void* pObj, void** aryArg, FetchParam&...p )
		{
			return new( pObj )ClassType( p... );
		}
	};

	template<typename ClassType, typename FirstParam, typename... RemainParam>
	struct TConstructFromParam<ClassType, FirstParam, RemainParam...>
	{
		template<typename... FetchParam>
		static ClassType* Construct( size_t nIndex, void* pObj, void** aryArg, FetchParam&...p )
		{
			FirstParam f = ArgFetcher<FirstParam>::CallWrapArg( aryArg[nIndex] );
			typedef TConstructFromParam<ClassType, RemainParam...> NextType;
			return NextType::Construct( nIndex + 1, pObj, aryArg, p..., f );
		}
	};

	///< Construct the object
	template<typename... Param> class TConstructParams;
	template<typename OrgClass, typename ClassType,
		typename ConstructParamsType, EConstructType eType>
	class TConstruct : public IObjectConstruct
	{
		template<typename... Param>
		ClassType* PlacementNew( TConstructParams<Param...>*, void* pObj, void** aryArg )
		{
			return TConstructFromParam<ClassType, Param...>::Construct( 0, pObj, aryArg );
		}

		template<typename... Param>
		void SetVTable( TConstructParams<Param...>*, ClassType* pObj )
		{
			typedef TGetVTable<OrgClass, eType, Param...> GetVTableType;
			if( !GetVTableType::GetFunInst() )
				return;
			( ( XS::SVirtualObj* )pObj )->m_pTable = GetVTableType::GetVTbInst();
		}

		template<typename... Param>
		STypeInfoArray GetTypes( TConstructParams<Param...>* )
		{
			return MakeFunArg<void, Param...>();
		}

	public:
		virtual void Assign( void* pDest, void* pSrc )
		{
			TCopy<ClassType, eType == eConstructType_Normal>( pDest, pSrc );
		}

		virtual void CopyConstruct( void* pDest, void* pSrc )
		{
			TCopyConstruct<ClassType, eType == eConstructType_Normal>( pDest, pSrc );
		}

		virtual void Construct( void* pObj, void** aryArg )
		{
			ConstructParamsType* pType = nullptr;
			SetVTable( pType, PlacementNew( pType, pObj, aryArg ) );
		}

		virtual void Destruct( void* pObj )
		{
			static_cast<ClassType*>( pObj )->~ClassType();
		}

		virtual STypeInfoArray GetFunArg()
		{
			return GetTypes((ConstructParamsType*)nullptr);
		}

		static IObjectConstruct* Inst()
		{
			static TConstruct<OrgClass, ClassType, ConstructParamsType, eType> s_Instance;
			return &s_Instance;
		}
	};

	template<typename OrgClass, typename ClassType>
	class TConstruct<OrgClass, ClassType, TConstructParams<>, eConstructType_Abstract>
	{ public: static IObjectConstruct* Inst() { return nullptr; } };

	///< Put all function register in a list and invoke them recursively
	class CScriptRegisterNode : public TList<CScriptRegisterNode>::CListNode
	{
		void( *m_funRegister )( );
		typedef typename TList<CScriptRegisterNode>::CListNode ParentType;
	public:
		CScriptRegisterNode( TList<CScriptRegisterNode>& list, void( *fun )() )
			: m_funRegister( fun )
		{
			list.PushBack( *this );
		}

		bool Register()
		{
			m_funRegister();
			auto n = GetNext();
			Remove();
			return n ? n->Register() : true;
		}
	};

	///< Fetch inherit information
	typedef TList<CScriptRegisterNode> CScriptRegisterList;
	struct SGlobalExe { SGlobalExe( bool b = false ) {} };

	template<typename Derive, typename...Base> struct TBaseClassOffset {};
	template<typename Derive> struct TBaseClassOffset<Derive> 
	{
		static void GetOffset( ptrdiff_t* ary ) {}
		static void GetTypes( const char** ary ) {}
	};

	template<typename Derive, typename First, typename...Base>
	struct TBaseClassOffset<Derive, First, Base...>
	{
		static void GetOffset( ptrdiff_t* ary )
		{
			*ary = ( (ptrdiff_t)(First*)(Derive*)0x40000000 ) - 0x40000000;
			TBaseClassOffset<Derive, Base...>::Get( ++ary );
		}

		static void GetTypes( const char** ary )
		{
			*ary = typeid( First ).name();
			TBaseClassOffset<Derive, Base...>::Get( ++ary );
		}
	};

	template<typename ... _Base>
	class TInheritInfo
	{
	public:
		enum { size = sizeof...( _Base ) + 1 };

		template<typename _Derive>
		static std::array<ptrdiff_t, size + 1> Values()
		{
			std::array<ptrdiff_t, size + 1> result = { sizeof( _Derive ) };
			TBaseClassOffset<_Derive, _Base...>::GetOffset( &result[1] );
			return result;
		}

		template<typename _Derive>
		static std::array<const char*, size + 1> Types()
		{
			std::array<const char*, size + 1> result = { typeid( _Derive ).name() };
			TBaseClassOffset<_Derive, _Base...>::GetTypes( &result[1] );
			return result;
		}
	};

	template< typename Type > struct TFetchResult
	{
		template<typename FunctionType, typename... Param>
		TFetchResult( FunctionType funCall, void* pRetBuf, Param...p )
		{ new( pRetBuf ) Type( funCall( p... ) ); }
	};

	template< typename Type > struct TFetchResult<Type&>
	{
		template<typename FunctionType, typename... Param>
		TFetchResult( FunctionType funCall, void* pRetBuf, Param...p )
		{ *(Type**)pRetBuf = &( funCall( p... ) ); }
	};

	template<> struct TFetchResult<void>
	{
		template<typename FunctionType, typename... Param>
		TFetchResult( FunctionType funCall, void* pRetBuf, Param...p )
		{ funCall( p... ); }
	};

	template<typename RetType, typename... RemainParam> 
	struct TFunctionCaller {};
	template<typename RetType> struct TFunctionCaller<RetType>
	{
		template<typename FunctionType, typename... FetchParam>
		static void CallFun( size_t nIndex, FunctionType funCall, 
			void* pRetBuf, void** pArgArray, FetchParam&...p )
		{ TFetchResult<RetType> Temp( funCall, pRetBuf, p... ); }
	};

	template<typename RetType, typename FirstParam, typename... RemainParam>
	struct TFunctionCaller<RetType, FirstParam, RemainParam...>
	{
		template<typename FunctionType, typename... FetchParam>
		static void CallFun( size_t nIndex, FunctionType funCall,
			void* pRetBuf, void** pArgArray, FetchParam&...p )
		{ 
			FirstParam f = ArgFetcher<FirstParam>::CallWrapArg( pArgArray[nIndex] );
			typedef TFunctionCaller<RetType, RemainParam...> NextFunCaller;
			NextFunCaller::CallFun( nIndex + 1, funCall, pRetBuf, pArgArray, p..., f );
		}
	};

	template<typename RetType, typename... Param >
	class TFunctionWrap : public IFunctionWrap
	{
		typedef RetType( *FunctionType )( Param... );
		typedef TFunctionCaller<RetType, Param...> FunctionCaller;

	public:
		void Call( void* pRetBuf, void** pArgArray, uintptr_t funRaw )
		{
			FunctionCaller::CallFun( 0, *(FunctionType*)&funRaw, pRetBuf, pArgArray );
		}

		static TFunctionWrap* GetInst()
		{ 
			static TFunctionWrap s_Inst; 
			return &s_Inst;
		}
	};

	template< typename RetType, typename... Param >
	inline void CreateGlobalFunWrap(RetType ( *pFun )( Param... ), 
		const char* szType, const char* szName)
	{
		IFunctionWrap* pWrap = TFunctionWrap<RetType, Param...>::GetInst();
		STypeInfoArray InfoArray = MakeFunArg<RetType, Param...>();
		CScriptBase::RegisterGlobalFunction(pWrap, (uintptr_t)pFun, InfoArray, szType, szName);
	}
	
	template< typename RetType, typename ClassType, typename... Param >
	inline void CreateClassFunWrap( RetType (pFun)(ClassType*, Param...), const char* szName)
	{
		IFunctionWrap* pWrap = TFunctionWrap<RetType, ClassType*, Param...>::GetInst();
		STypeInfoArray InfoArray = MakeFunArg<RetType, ClassType*, Param...>();
		CScriptBase::RegisterClassFunction( pWrap, (uintptr_t)pFun, InfoArray, szName );
	}

	template< typename T >
	struct TCallBack
	{
		static T OnCall( uint32 nCallBackIndex, void** pArgArray )
		{
			T ReturnValue;
			CScriptBase::CallBack( nCallBackIndex, &ReturnValue, pArgArray );
			return ReturnValue;
		}
	};

	template<typename T>
	struct TCallBack<T&>
	{
		static T& OnCall( uint32 nCallBackIndex, void** pArgArray )
		{
			T* pReturnValue;
			CScriptBase::CallBack( nCallBackIndex, &pReturnValue, pArgArray );
			return *pReturnValue;
		}
	};

	template<>
	struct TCallBack<void>
	{
		static void OnCall( uint32 nCallBackIndex, void** pArgArray )
		{
			CScriptBase::CallBack( nCallBackIndex, nullptr, pArgArray );
		}
	};

	template< typename Type > 
	struct TOrgFunResult
	{
		template<typename CallBackWrap, typename FunctionType, typename... Param>
		static void Call( FunctionType funCall, void* pRetBuf, CallBackWrap* pObj, Param...p )
		{
			Type result = ( pObj->*funCall )( p... );
			new( pRetBuf ) Type( result );
		}
	};

	template< typename Type > 
	struct TOrgFunResult<Type&>
	{
		template<typename CallBackWrap, typename FunctionType, typename... Param>
		static void Call( FunctionType funCall, void* pRetBuf, CallBackWrap* pObj, Param...p )
		{
			*(Type**)pRetBuf = &( ( pObj->*funCall )( p... ) );
		}
	};

	template<> 
	struct TOrgFunResult<void>
	{
		template<typename CallBackWrap, typename FunctionType, typename... Param>
		static void Call( FunctionType funCall, void* pRetBuf, CallBackWrap* pObj, Param...p )
		{
			( pObj->*funCall )( p... );
		}
	};

	template<typename RetType, typename... RemainParam>
	struct TOrgFunParam {};

	template<typename RetType>
	struct TOrgFunParam<RetType>
	{
		template<typename CallBackWrap, typename FunctionType, typename... FetchParam>
		static void CallFun( size_t nIndex, FunctionType funCall,
			void* pRetBuf, CallBackWrap* pObj, void** pArgArray, FetchParam&...p )
		{
			TOrgFunResult<RetType>::Call( funCall, pRetBuf, pObj, p... );
		}
	};

	template<typename RetType, typename FirstParam, typename... RemainParam>
	struct TOrgFunParam<RetType, FirstParam, RemainParam...>
	{
		template<typename CallBackWrap, typename FunctionType, typename... FetchParam>
		static void CallFun( size_t nIndex, FunctionType funCall,
			void* pRetBuf, CallBackWrap* pObj, void** pArgArray, FetchParam&...p )
		{
			FirstParam f = ArgFetcher<FirstParam>::CallWrapArg( pArgArray[nIndex] );
			TOrgFunParam<RetType, RemainParam...>::CallFun( nIndex + 1, funCall, pRetBuf, pObj, pArgArray, p..., f );
		}
	};

	///< Callback function access wrapper
	template<typename ClassFunType>
	class TCallBackBinder
	{
		template<typename RetType, typename ClassType, typename... Param >
		class TCallBackWrap : public IFunctionWrap
		{
		public:
			static int32& GetCallBackIndex()
			{
				static int32 s_nCallBackIndex = -1;
				return s_nCallBackIndex;
			}

			template<typename... ParamPtr >
			RetType WrapAddress( ParamPtr ... p )
			{
				TCallBackWrap* pThis = this;
				void* pCallArray[] = { &pThis, ArgFetcher<Param>::CallBackArg( p )..., nullptr };
				return TCallBack<RetType>::OnCall( GetCallBackIndex(), pCallArray );
			}

			RetType BootFunction( Param ... p )
			{
				return WrapAddress( &p ... );
			}

			typedef decltype( &TCallBackWrap::BootFunction ) FunctionType;

		public:
			void Call( void* pRetBuf, void** pArgArray, uintptr_t funRaw )
			{
				typedef TOrgFunParam<RetType, Param...> OrgFunctionCaller;
				TCallBackWrap* pObj = *(TCallBackWrap**)pArgArray[0];
				FunctionType fun = *(FunctionType*)&funRaw;
				OrgFunctionCaller::CallFun( 0, fun, pRetBuf, pObj, pArgArray + 1 );
			}

			static TCallBackWrap* GetInst()
			{
				static TCallBackWrap s_Inst;
				return &s_Inst;
			}
		};
	public:
		static XS::SFunctionTable* GetVirtualTable( void* p )
		{ 
			return ( (SVirtualObj*)(ClassFunType*)p )->m_pTable; 
		}

		static bool InsallGetVirtualTable()
		{
			ClassFunType::GetFunInst() = (GetVirtualTableFun)&GetVirtualTable;
			return false;
		}

		template<typename RetType, typename ClassType, typename... Param >
		static void Bind( bool bPureVirtual, const char* szFunName,
			RetType(ClassType::*pFun)(Param...) )
		{
			typedef TCallBackWrap<RetType, ClassType, Param...> CallBackWrap;
			typedef typename CallBackWrap::FunctionType FunctionType;

			/**
			* @note If compile error occur, it is mean the size of FunctionType 
			* and uintptr_t are not equal. It is dependence to compiler, but 
			* most compile will work well.
			*/
			typedef TClassSizeEqual<FunctionType, uintptr_t> SizeCheck;
			//typedef typename SizeCheck::Succeeded Succeeded;

			IFunctionWrap* pWrap = CallBackWrap::GetInst();
			STypeInfoArray InfoArray = MakeFunArg<RetType, ClassType*, Param...>();
			FunctionType funBoot = &CallBackWrap::BootFunction;
			CallBackWrap::GetCallBackIndex() = GetVirtualFunIndex(pFun);
			CScriptBase::RegisterClassCallback( pWrap, *(uintptr_t*)&funBoot,
				CallBackWrap::GetCallBackIndex(), bPureVirtual, InfoArray, szFunName );
		}

		template< typename ClassType, typename RetType, typename... Param >
		static inline void Bind(bool bPureVirtual, const char* szFunName, 
			RetType (ClassType::*pFun)(Param...) const )
		{
			typedef RetType(ClassType::*FunctionType)(Param...);
			Bind( bPureVirtual, szFunName, (FunctionType)pFun );
		}
	};

	///< Virtual destructor wrapper
	template< typename ClassType >
	class TDestructorWrap : public IFunctionWrap
	{
		static int32& GetCallBackIndex()
		{
			static int32 s_nCallBackIndex = -1;
			return s_nCallBackIndex;
		}

		void Call( void* pRetBuf, void** pArgArray, uintptr_t funContext)
		{
			class Derive : public ClassType { public: ~Derive() {}; };
			( ( *(Derive**)pArgArray[0] ) )->~Derive();
		}

		void Wrap(uint32 p0)
		{
			void* pArg[] = { this, &p0 };
			CScriptBase::CallBack( GetCallBackIndex(), nullptr, pArg );
		}

		typedef decltype(&TDestructorWrap::Wrap) FunctionType;
	public:
		static void Bind()
		{
			/**
			* @note If compile error occur, it is mean the size of FunctionType
			* and uintptr_t are not equal. It is dependence to compiler, but
			* most compile will work well.
			*/
			typedef TClassSizeEqual<FunctionType, uintptr_t> SizeCheck;
			//typedef typename SizeCheck::Succeeded Succeeded;

			static TDestructorWrap s_instance;
			GetCallBackIndex() = XS::GetDestructorFunIndex<ClassType>();
			FunctionType funBoot = &TDestructorWrap::Wrap;
			STypeInfo aryInfo[2] = { GetTypeInfo<ClassType*>(), GetTypeInfo<void>() };
			STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
			XS::CScriptBase::RegisterDestructor(&s_instance, 
				*(uintptr_t*)&funBoot, GetCallBackIndex(), TypeInfo );
		}
	};

	///< Data member access(read) wrapper
	template<typename MemberType>
	class TMemberGetWrap : public IFunctionWrap
	{
	public:
		void Call( void* pRetBuf, void** pArgArray, uintptr_t funContext )
		{ new( pRetBuf ) MemberType( *(MemberType*)( ( *(char**)pArgArray[0] ) + funContext ) ); }
		static IFunctionWrap* GetInst() { static TMemberGetWrap s_Inst; return &s_Inst; }
	};

	template<typename MemberType>
	class TMemberGetWrapObject : public IFunctionWrap
	{
	public:
		void Call( void* pRetBuf, void** pArgArray, uintptr_t funContext )
		{ *(MemberType**)pRetBuf = (MemberType*)( ( *(char**)pArgArray[0] ) + funContext ); }
		static IFunctionWrap* GetInst() { static TMemberGetWrapObject s_Inst; return &s_Inst; }
	};

	template<typename MemberType>
	inline IFunctionWrap* CreateMemberGetWrap(MemberType*)
	{
		STypeInfo TypeInfo = GetTypeInfo<MemberType>();
		if( ( TypeInfo.m_nType >> 24 ) != eDT_class ||
			( ( TypeInfo.m_nType >> 20 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType >> 16 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType >> 12 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType >>  8 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType >>  4 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType       )&0xf ) >= eDTE_Pointer )
			return TMemberGetWrap<MemberType>::GetInst();
		return TMemberGetWrapObject<MemberType>::GetInst();
	}

	///< Data member access(write) wrapper
	template<typename MemberType>
	class TMemberSetWrap : public IFunctionWrap
	{
	public:
		void Call( void* pRetBuf, void** pArgArray, uintptr_t funContext )
		{
			*(MemberType*)( ( *(char**)pArgArray[0] ) + funContext ) =
				ArgFetcher<MemberType>::CallWrapArg( pArgArray[1] );
		}
		static IFunctionWrap* GetInst() { static TMemberSetWrap s_Inst; return &s_Inst; }
	};

	template<typename MemberType>
	class TMemberSetWrapObject : public IFunctionWrap
	{
	public:
		void Call( void* pRetBuf, void** pArgArray, uintptr_t funContext )
		{
			*(MemberType*)( ( *(char**)pArgArray[0] ) + funContext ) =
				ArgFetcher<MemberType>::CallWrapArg( *(void**)pArgArray[1] );
		}
		static IFunctionWrap* GetInst() { static TMemberSetWrapObject s_Inst; return &s_Inst; }
	};

	template<typename MemberType>
	inline IFunctionWrap* CreateMemberSetWrap( MemberType* )
	{
		static STypeInfo MemberInfo = GetTypeInfo<MemberType>();
		if( ( ( MemberInfo.m_nType >> 24 ) == eDT_class &&
			( ( MemberInfo.m_nType >> 20 )&0xf ) < eDTE_Pointer &&
			( ( MemberInfo.m_nType >> 16 )&0xf ) < eDTE_Pointer &&
			( ( MemberInfo.m_nType >> 12 )&0xf ) < eDTE_Pointer &&
			( ( MemberInfo.m_nType >>  8 )&0xf ) < eDTE_Pointer &&
			( ( MemberInfo.m_nType >>  4 )&0xf ) < eDTE_Pointer &&
			( ( MemberInfo.m_nType )&0xf ) < eDTE_Pointer ) )
			return TMemberSetWrapObject<MemberType>::GetInst();
		return TMemberSetWrap<MemberType>::GetInst();
	}

	template< typename ClassType, typename MemberType >
	inline STypeInfoArray MakeMemberArg( ClassType*, MemberType* )
	{
		static STypeInfo MemberInfo = GetTypeInfo<MemberType>();
		static STypeInfo aryInfo[2] = 
		{ 
			GetTypeInfo<ClassType*>(), 
			( ( MemberInfo.m_nType >> 24 ) == eDT_class &&
			( ( MemberInfo.m_nType >> 20 )&0xf ) < eDTE_Pointer &&
			( ( MemberInfo.m_nType >> 16 )&0xf ) < eDTE_Pointer &&
			( ( MemberInfo.m_nType >> 12 )&0xf ) < eDTE_Pointer &&
			( ( MemberInfo.m_nType >>  8 )&0xf ) < eDTE_Pointer &&
			( ( MemberInfo.m_nType >>  4 )&0xf ) < eDTE_Pointer &&
			( ( MemberInfo.m_nType )&0xf ) < eDTE_Pointer ) ?
			GetTypeInfo<MemberType&>() : GetTypeInfo<MemberType>()
		};

		STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
		return TypeInfo;
	}	
}
