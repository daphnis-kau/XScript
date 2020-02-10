/**@file  		XScriptWrap.h
* @brief		Template wrapper of XScript
* @author		Daphnis Kau
* @date			2020-01-17
* @version		V1.0
*/

#pragma once
#include <array>
#include "core/CScriptBase.h"

namespace XS
{
	///< Capture compile error
	template<bool bCompileSucceed> struct TCompileSucceed {};
	template<> struct TCompileSucceed<true> { struct Succeeded {}; };

	///< Check size of the giving class are equal
	template<typename _T1, typename _T2> struct TClassSizeEqual
	{ 
		typedef TCompileSucceed<sizeof(_T1) == sizeof(_T2)> CCompileSucceed;
		typedef typename CCompileSucceed::Succeeded Succeeded;
	};

	///< Get original virtual table
	typedef SFunctionTable* ( *GetVirtualTableFun )( void* );
	template<typename _T, bool bDuplicatable> struct TCopy 
	{ TCopy( void* pDest, void* pSrc ) { *(_T*)pDest = *(_T*)pSrc; } };
	template<typename _T> struct TCopy<_T, false> 
	{ TCopy( void* pDest, void* pSrc ) { throw( "Can not duplicate object" ); } };

	template<typename ClassType>
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

		TGetVTable()
		{
			if( !GetFunInst() || GetVTbInst() )
				return;
			GetVTbInst() = GetFunInst()( this );
		}
	};

	template<>
	struct TGetVTable<void> 
	{
		static GetVirtualTableFun& GetFunInst()
		{
			static GetVirtualTableFun s_fun;
			return s_fun;
		}
	};

	///< Construct the object
	template<typename GetVTableType, typename ClassType, bool bDuplicatable>
	struct TConstruct : public IObjectConstruct
	{
		virtual void Assign( void* pDest, void* pSrc )
		{
			TCopy<ClassType, bDuplicatable>( pDest, pSrc );
		}

		virtual void Construct( void* pObj )
		{
			ClassType* pNew = new( pObj )ClassType;
			if( !GetVTableType::GetFunInst() )
				return;
			( ( XS::SVirtualObj* )pNew )->m_pTable = GetVTableType::GetVTbInst();
		}

		virtual void Destruct( void* pObj )
		{
			static_cast<ClassType*>( pObj )->~ClassType();
		}

		static IObjectConstruct* Inst()
		{
			static TConstruct<GetVTableType, ClassType, bDuplicatable> s_Instance;
			return &s_Instance;
		}
	};

	template<typename ClassType, bool bDuplicatable>
	struct TConstruct<TGetVTable<void>, ClassType, bDuplicatable>
	{ static IObjectConstruct* Inst() { return nullptr; } };

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

	template<typename _Derive, typename ... _Base>
	class TInheritInfo
	{
	public:
		enum { size = sizeof...( _Base ) + 1 };
		template<typename...Param> struct TOffset {};
		template<> struct TOffset<> { static void Get( ptrdiff_t* ary ) {} };

		template<typename First, typename...Param>
		struct TOffset<First, Param...>
		{
			static void Get( ptrdiff_t* ary )
			{
				*ary = ( (ptrdiff_t)(First*)(_Derive*)0x40000000 ) - 0x40000000;
				TOffset<Param...>::Get( ++ary );
			}
		};

		static std::array<ptrdiff_t, size + 1> Values()
		{
			std::array<ptrdiff_t, size + 1> result = { sizeof( _Derive ) };
			TOffset<_Base...>::Get( &result[1] );
			return result;
		}

		static std::array<const char*, size + 1> Types()
		{
			return { typeid( _Derive ).name(), typeid( _Base... ).name()... };
		}
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

	///< Function calling wrapper
	template< typename T > 
	struct ArgFetcher
	{ 
		static inline T& CallWrapArg( void* pData )		{ return *(T*)pData; }	
		static inline void* CallBackArg( T*& pData )	{ return pData; }
	};

	template<typename T> 
	struct ArgFetcher<T&>
	{ 
		static inline T& CallWrapArg( void* pData )		{ return **(T**)pData; } 
		static inline void* CallBackArg( T*& pData )	{ return &pData; }
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

	template<typename RetType, typename... Param >
	class TFunctionWrap : public IFunctionWrap
	{
		typedef RetType( *FunctionType )( Param... );

		template< typename Type > struct TFetchResult
		{
			TFetchResult( FunctionType funCall, void* pRetBuf, Param...p )
			{ new( pRetBuf ) Type( funCall( p... ) ); }
		};

		template< typename Type > struct TFetchResult<Type&>
		{
			TFetchResult( FunctionType funCall, void* pRetBuf, Param...p )
			{ *(Type**)pRetBuf = &( funCall( p... ) ); }
		};

		template<> struct TFetchResult<void>
		{
			TFetchResult( FunctionType funCall, void* pRetBuf, Param...p )
			{ funCall( p... ); }
		};

		template<typename... RemainParam> struct TFetchParam {};
		template<> struct TFetchParam<>
		{
			template<typename... FetchParam>
			static void CallFun( size_t nIndex, FunctionType funCall, 
				void* pRetBuf, void** pArgArray, FetchParam&...p )
			{ TFetchResult<RetType> Temp( funCall, pRetBuf, p... ); }
		};

		template<typename FirstParam, typename... RemainParam>
		struct TFetchParam<FirstParam, RemainParam...>
		{
			template<typename... FetchParam>
			static void CallFun( size_t nIndex, FunctionType funCall,
				void* pRetBuf, void** pArgArray, FetchParam&...p )
			{ 
				FirstParam f = ArgFetcher<FirstParam>::CallWrapArg( pArgArray[nIndex] );
				TFetchParam<RemainParam...>::CallFun( nIndex + 1, funCall, pRetBuf, pArgArray, p..., f );
			}
		};

	public:
		void Call( void* pRetBuf, void** pArgArray, uintptr_t funRaw )
		{
			TFetchParam<Param...>::CallFun( 0, *(FunctionType*)&funRaw, pRetBuf, pArgArray );
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
		CScriptBase::RegistGlobalFunction(pWrap, (uintptr_t)pFun, InfoArray, szType, szName);
	}
	
	template< typename RetType, typename ClassType, typename... Param >
	inline void CreateClassFunWrap(RetType(pFun)(ClassType*, Param...), const char* szName)
	{
		IFunctionWrap* pWrap = TFunctionWrap<RetType, ClassType*, Param...>::GetInst();
		STypeInfoArray InfoArray = MakeFunArg<RetType, ClassType*, Param...>();
		CScriptBase::RegistClassFunction( pWrap, (uintptr_t)pFun, InfoArray, szName );
	}

	///< Callback function access wrapper
	template<typename ClassFunType>
	class TCallBackBinder
	{
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
				CScriptBase::CallBack( nCallBackIndex, NULL, pArgArray );
			}
		};

		template<typename RetType, typename ClassType, typename... Param >
		class TCallBackWrap
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
				void* pCallArray[] = { &pThis, ArgFetcher<Param>::CallBackArg( p )..., NULL };
				return TCallBack<RetType>::OnCall( GetCallBackIndex(), pCallArray );
			}

			RetType BootFunction( Param ... p )
			{
				return WrapAddress( &p ... );
			}

			typedef decltype(&TCallBackWrap::BootFunction) FunctionType;
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
			typedef TFunctionWrap<RetType, ClassType*, Param...> FunctionWrap;

			/**
			* @note If compile error occur, it is mean the size of FunctionType 
			* and uintptr_t are not equal. It is dependence to compiler, but 
			* most compile will work well.
			*/
			typedef TClassSizeEqual<CallBackWrap::FunctionType, uintptr_t> SizeCheck;
			typedef typename SizeCheck::Succeeded Succeeded;

			IFunctionWrap* pWrap = FunctionWrap::GetInst();
			STypeInfoArray InfoArray = MakeFunArg<RetType, ClassType*, Param...>();
			CallBackWrap::FunctionType funBoot = &CallBackWrap::BootFunction;
			CallBackWrap::GetCallBackIndex() = GetVirtualFunIndex(pFun);
			CScriptBase::RegistClassCallback( pWrap, *(uintptr_t*)&funBoot,
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
			CScriptBase::CallBack( GetCallBackIndex(), NULL, pArg );
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
			typedef typename SizeCheck::Succeeded Succeeded;

			static TDestructorWrap s_instance;
			GetCallBackIndex() = XS::GetDestructorFunIndex<ClassType>();
			FunctionType funBoot = &TDestructorWrap::Wrap;
			STypeInfo aryInfo[2] = { GetTypeInfo<ClassType*>(), GetTypeInfo<void>() };
			STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
			XS::CScriptBase::RegistDestructor(&s_instance, 
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
			*(MemberType*)( (*(char**)pArgArray[0]) + funContext ) =
			ArgFetcher<MemberType>::CallWrapArg( pArgArray[1] );
		}
		static IFunctionWrap* GetInst() { static TMemberSetWrap s_Inst; return &s_Inst; }
	};

	template<typename MemberType>
	inline IFunctionWrap* CreateMemberSetWrap( MemberType* )
	{
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
