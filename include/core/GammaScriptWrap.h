//=============================================================
// GammaScriptWrap.h 
// 生成函数调用以及回调的包装
// 柯达昭
// 2012-08-09
//=====================================================================
#pragma once
#include "core/GammaScriptDef.h"

namespace Gamma
{
	template<typename ClassType, typename RetType, typename... Param>
	inline STypeInfoArray MakeClassFunArg( RetType ( ClassType::*pFun )( Param... ) )
	{
		static STypeInfo aryInfo[] =
		{ GetTypeInfo<ClassType*>(), GetTypeInfo<Param>()..., GetTypeInfo<RetType>() };
		STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
		return TypeInfo;
	}

	template<typename ClassType, typename RetType, typename... Param>
	inline STypeInfoArray MakeClassFunArg( RetType ( ClassType::*pFun )( Param... ) const )
	{
		static STypeInfo aryInfo[] =
		{ GetTypeInfo<ClassType*>(), GetTypeInfo<Param>()..., GetTypeInfo<RetType>() };
		STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
		return TypeInfo;
	}

	template<typename RetType, typename... Param>
	inline STypeInfoArray MakeFunArg( RetType ( *pFun )( Param... ) )
	{
		static STypeInfo aryInfo[] =
		{ GetTypeInfo<Param>()..., GetTypeInfo<RetType>() };
		STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
		return TypeInfo;
	}

	//=======================================================================
	// 函数调用包装
	//=======================================================================
	template< typename T >
	struct ArgFetcher
	{
		static inline T&	CallWrapArg( void* pArgArray )	{ return *(T*)pArgArray; }
		static inline void* CallBackArg( T*& pArgArray )	{ return pArgArray; }
	};

	template<typename T>
	struct ArgFetcher<T&>
	{
		static inline T&	CallWrapArg( void* pArgArray )	{ return **(T**)pArgArray; }
		static inline void* CallBackArg( T*& pArgArray )	{ return &pArgArray; }
	};

#define SIMPLE_TYPE_ARG_FETCHER( T ) \
	template<> \
	struct ArgFetcher<const T&> \
	{ \
		static inline const T&	CallWrapArg( void* pArgArray )	{ return *(const T*)pArgArray; } \
		static inline void* CallBackArg( const T*& pArgArray )	{ return (void*)pArgArray; } \
	}; \
	
	SIMPLE_TYPE_ARG_FETCHER( int8 )
	SIMPLE_TYPE_ARG_FETCHER( int16 )
	SIMPLE_TYPE_ARG_FETCHER( int32 )
	SIMPLE_TYPE_ARG_FETCHER( int64 )
	SIMPLE_TYPE_ARG_FETCHER( uint8 )
	SIMPLE_TYPE_ARG_FETCHER( uint16 )
	SIMPLE_TYPE_ARG_FETCHER( uint32 )
	SIMPLE_TYPE_ARG_FETCHER( uint64 )
	SIMPLE_TYPE_ARG_FETCHER( float )
	SIMPLE_TYPE_ARG_FETCHER( double )
#undef SIMPLE_TYPE_ARG_FETCHER

	enum ECallType
	{
		eCT_CDecl,
		eCT_StdCall,
		eCT_ThisCall,
	};	

//#ifdef _WIN32
//#	define DEFINE_STD_TYPE( n ) typedef RetType (__stdcall *StdCallType)( Param... );
//#	define CHECK_STD_CALL( n ) \
//		if( eType == eCT_StdCall ) \
//			return ( *MakeFun<StdCallType>( funRaw.funPoint ? funRaw : m_funOrg ) )( VALUE( n ) );
//#else
#	define DEFINE_STD_TYPE( n )
#	define CHECK_STD_CALL( n )
//#endif

	template< ECallType eType, class ClassType, class RetType, typename... Param >
	class TFunctionWrap : public IFunctionWrap
	{
		#ifdef _WIN32
		typedef RetType (__stdcall *StdCallType)( Param... );
		#endif
		typedef RetType (*CDeclType)( Param... );
		typedef RetType ( ClassType::*ThisCallType)( Param... );
		typedef RetType ( ClassType::*ThisCallConstType)( Param... ) const;
		union { CDeclType m_funDeclType; ThisCallType m_funThisCallType; SFunction m_funOrg; };
		union UFunction { CDeclType funDeclType; ThisCallType funThisCallType; SFunction funOrg; };

		template< typename Type >
		class TCallFunction
		{
		public:
			TCallFunction( void* pObj, UFunction funCall, void* pRetBuf, Param...p )
			{
				if( eType == eCT_CDecl )
					new( pRetBuf ) Type( ( *funCall.funDeclType )( p... ) );
				new( pRetBuf ) Type( ( ( (ClassType*)pObj )->*( funCall.funThisCallType ) )( p... ) );
			}
		};

		template< typename Type >
		class TCallFunction<Type&>
		{
		public:
			TCallFunction( void* pObj, UFunction funCall, void* pRetBuf, Param...p )
			{
				if( eType == eCT_CDecl )
					*(Type**)pRetBuf = &( ( *funCall.funDeclType )( p... ) );
				*(Type**)pRetBuf = &( ( ( (ClassType*)pObj )->*( funCall.funThisCallType ) )( p... ) );
			}
		};

		template<>
		class TCallFunction<void>
		{
		public:
			TCallFunction( void* pObj, UFunction funCall, void* pRetBuf, Param...p )
			{
				if( eType == eCT_CDecl )
					( *funCall.funDeclType )( p... );
				else
					( ( (ClassType*)pObj )->*( funCall.funThisCallType ) )( p... );
			}
		};

		template<typename... RemainParam> class TFetchParam {};

		template<>
		class TFetchParam<>
		{
		public:
			template<typename... FetchParam>
			static void Fetch( size_t nIndex, void* pObj, UFunction funCall, void* pRetBuf, void** pArgArray, FetchParam...p )
			{
				TCallFunction<RetType> Temp( pObj, funCall, pRetBuf, p... );
			}
		};

		template<typename FirstParam, typename... RemainParam>
		class TFetchParam<FirstParam, RemainParam...>
		{
		public:
			template<typename... FetchParam>
			static void Fetch( size_t nIndex, void* pObj, UFunction funCall, void* pRetBuf, void** pArgArray, FetchParam...p )
			{
				TFetchParam<RemainParam...>::Fetch( nIndex + 1, pObj, funCall, pRetBuf, pArgArray,
					p..., ArgFetcher<FirstParam>::CallWrapArg( pArgArray[nIndex] ) );
			}
		};

	public:
		TFunctionWrap( CDeclType funOrg ) : m_funDeclType( funOrg ){}
		TFunctionWrap( ThisCallType funOrg ) : m_funThisCallType( funOrg ){}
		TFunctionWrap( ThisCallConstType funOrg ) : m_funThisCallType( (ThisCallType)funOrg ){}
		SFunction GetOrgFun() { return m_funOrg; }

		void Call( void* pObj, void* pRetBuf, void** pArgArray, SFunction funRaw )
		{
			UFunction funCall;
			funCall.funOrg = funRaw.funPoint ? funRaw : GetOrgFun();
			TFetchParam<Param...>::Fetch( 0, pObj, funCall, pRetBuf, pArgArray );
		}
	};
							  
#if( defined _WIN32 && !( defined _M_X64 ) )
	template< typename ClassType, typename RetType, typename... Param >
	inline IFunctionWrap* CreateFunWrap( RetType ( __stdcall *pFun )( Param... ) )
	{
		return new TFunctionWrap<eCT_StdCall, IFunctionWrap, RetType, Param...>( pFun );
	}
#endif

	template< typename ClassType, typename RetType, typename... Param >
	inline IFunctionWrap* CreateFunWrap( RetType ( ClassType::*pFun )( Param... ) )
	{
		return new TFunctionWrap<eCT_ThisCall, ClassType, RetType, Param...>( pFun );
	}

	template< typename ClassType, typename RetType, typename... Param >
	inline IFunctionWrap* CreateFunWrap( RetType ( ClassType::*pFun )( Param... ) const )
	{
		return new TFunctionWrap<eCT_ThisCall, ClassType, RetType, Param...>( pFun );
	}

	template< typename RetType, typename... Param >
	inline IFunctionWrap* CreateFunWrap( RetType ( *pFun )( Param... ) )
	{
		return new TFunctionWrap<eCT_CDecl, IFunctionWrap, RetType, Param...>( pFun );
	}

	//=======================================================================
	// 类非常量成员函数回调包装
	//=======================================================================
	GAMMA_SCRIPT_API int32 CallBack( int32 nIndex, void* pObject, void* pRetBuf, void** pArgArray );
	GAMMA_SCRIPT_API void  UnlinkCppObj( void* pObj );

	template< typename T >
	struct TCallBack
	{
		static T OnCall( uint32 nCallBackIndex, void* pObject, void** pArgArray )
		{
			T ReturnValue;
			CallBack( nCallBackIndex, pObject, &ReturnValue, pArgArray );
			return ReturnValue;
		}
	};

	template<typename T>
	struct TCallBack<T&>
	{
		static T& OnCall( uint32 nCallBackIndex, void* pObject, void** pArgArray )
		{
			T* pReturnValue;
			int32 ret = CallBack( nCallBackIndex, pObject, &pReturnValue, pArgArray );
			// 纯虚函数回调，但回调没有实现
			if( ret < 0 )
				throw "i can do nothing here!!!";
			return *pReturnValue;
		}
	};

	template<>
	struct TCallBack<void>
	{
		static void OnCall( uint32 nCallBackIndex, void* pObject, void** pArgArray )
		{
			CallBack( nCallBackIndex, pObject, NULL, pArgArray );
		}
	};

	template<>
	struct TCallBack<bool>
	{
		static bool OnCall( uint32 nCallBackIndex, void* pObject, void** pArgArray )
		{
			uint32 ReturnValue = 0;
			CallBack( nCallBackIndex, pObject, &ReturnValue, pArgArray );
			return ReturnValue != 0;
		}
	};

	template<int32 nInstance>
	class CCallBackBinder
	{
		template<class RetType, class ClassType, typename... Param >
		class TCallBackWrap
		{
		public:
			static int32& GetCallBackIndex()
			{
				static int32 s_nCallBackIndex = -1;
				return s_nCallBackIndex;
			}

			static bool SetCallBack( ICallBackWrap& CallBackWrap, bool bPureVirtual )
			{
				int32 nIndex = CallBackWrap.BindFunction(
					GetFunAdress( &TCallBackWrap::Wrap ), bPureVirtual );
				if( nIndex == GetCallBackIndex() )
					return true;
				assert( GetCallBackIndex() < 0 );
				GetCallBackIndex() = nIndex;
				return true;
			}

			template<typename... ParamPtr >
			RetType WrapAddress( ParamPtr ... p )
			{
				void* pCallArray[] = { ArgFetcher<Param...>::CallBackArg( p... ), NULL };
				return TCallBack<RetType>::OnCall( GetCallBackIndex(), this, pCallArray );
			}

			template<>
			RetType WrapAddress<>()
			{
				void* pCallArray[] = { NULL };
				return TCallBack<RetType>::OnCall( GetCallBackIndex(), this, pCallArray );
			}

			RetType Wrap( Param ... p )
			{
				return WrapAddress( &p ... );
			}
		};
	public:
		template< class ClassType, class RetType, typename... Param >
		static inline void BindWrap( ICallBackWrap& CallBackWrap,
			bool bPureVirtual, RetType ( ClassType::*pFun )( Param... ) )
		{
			TCallBackWrap<RetType, ClassType, Param...>::SetCallBack( CallBackWrap, bPureVirtual ); 
		}

		template< class ClassType, class RetType, typename... Param >
		static inline void BindWrap( ICallBackWrap& CallBackWrap,
			bool bPureVirtual, RetType ( ClassType::*pFun )( Param... ) const )
		{
			TCallBackWrap<RetType, ClassType, Param...>::SetCallBack( CallBackWrap, bPureVirtual );
		}
	};

	#define BIND_CALLBACK( wrap, pureVirtual, fun ) \
		CCallBackBinder<__LINE__>::BindWrap( wrap, pureVirtual, fun ) 

	//=======================================================================
	// 析构函数调用包装
	//=======================================================================
	template< class ClassType >
	class TDestructorWrap : public IFunctionWrap
	{
		uint32 m_nIndex;
	public:
		TDestructorWrap( uint32 nIndex ) : m_nIndex( nIndex ) {}
		SFunction GetOrgFun() { return GetFunction( m_nIndex ); }

		void Call( void* pObj, void* pRetBuf, void** pArgArray, SFunction funRaw )
		{
			class Derive : public ClassType { public: ~Derive() {}; };
			( (Derive*)pObj )->~Derive();
		}
	};

	template< class ClassType >
	inline IFunctionWrap* CreateDestructorWrap( uint32 nIndex )
	{
		return new TDestructorWrap<ClassType>( nIndex );
	}

	//=======================================================================
	// 析构函数调用包装绑定
	//=======================================================================
	template<class ClassType>
	static inline void BindDestructorWrap( ICallBackWrap& CallBackWrap )
	{
		static int32 s_nIndex = -1;
		class _FunWrap
		{
		public:
			void Wrap( uint32 p0 )	
			{ 
				void* pArg = &p0;
				CallBack( s_nIndex, this, NULL, &pArg );
			}
		};

		s_nIndex = CallBackWrap.BindFunction( GetFunAdress( &_FunWrap::Wrap ), false );
	}

	//=======================================================================
	// 成员读取包装
	//=======================================================================
	template< class ClassType, class MemberType >
	class TMemberGetWrap : public IFunctionWrap
	{
		ptrdiff_t m_nOffset;
	public:
		TMemberGetWrap( ptrdiff_t nOffset ) : m_nOffset( nOffset ) {}
		SFunction GetOrgFun() { SFunction fun = { 0, 0 }; return fun; }

		void Call( void* pObj, void* pRetBuf, void** pArgArray, SFunction funRaw )
		{
			new( pRetBuf ) MemberType( *(MemberType*)( (char*)pObj + m_nOffset ) );
		}
	};

	template< class ClassType, class MemberType >
	class TMemberGetWrapObject : public IFunctionWrap
	{
		ptrdiff_t m_nOffset;
	public:
		TMemberGetWrapObject( ptrdiff_t nOffset ) : m_nOffset( nOffset ) {}
		SFunction GetOrgFun() { SFunction fun = { 0, 0 }; return fun; }

		void Call( void* pObj, void* pRetBuf, void** pArgArray, SFunction funRaw )
		{
			*(MemberType**)pRetBuf = (MemberType*)( (char*)pObj + m_nOffset );
		}
	};

	template< class ClassType, class MemberType >
	inline IFunctionWrap* CreateMemberGetWrap( ClassType* pClass, MemberType* pMember )
	{
		STypeInfo TypeInfo;
		GetTypeInfo<MemberType>( TypeInfo );
		if( ( TypeInfo.m_nType >> 24 ) != eDT_class ||
			( ( TypeInfo.m_nType >> 20 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType >> 16 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType >> 12 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType >>  8 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType >>  4 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType       )&0xf ) >= eDTE_Pointer )
			return new TMemberGetWrap<ClassType, MemberType>( (char*)pMember - (char*)pClass );
		return new TMemberGetWrapObject<ClassType, MemberType>( (char*)pMember - (char*)pClass );
	}

	//=======================================================================
	// 成员写入包装
	//=======================================================================
	template< class ClassType, class MemberType >
	class TMemberSetWrap : public IFunctionWrap
	{
		uint32 m_nOffset;
	public:
		TMemberSetWrap( uint32 nOffset ) : m_nOffset( nOffset ) {}
		SFunction GetOrgFun() { SFunction fun = { 0, 0 }; return fun; }

		void Call( void* pObj, void* pRetBuf, void** pArgArray, SFunction funRaw )
		{
			*(MemberType*)( (char*)pObj + m_nOffset ) = ArgFetcher<MemberType>::CallWrapArg( pArgArray[0] );
		}
	};

	template< class ClassType, class MemberType >
	inline IFunctionWrap* CreateMemberSetWrap( ClassType* pClass, MemberType* pMember )
	{
		return new TMemberSetWrap<ClassType, MemberType>( (char*)pMember - (char*)pClass );
	}

	template< class ClassType, class MemberType >
	inline STypeInfoArray MakeMemberArg( ClassType*, MemberType* )
	{
		static STypeInfo aryInfo[2];
		GetTypeInfo<ClassType*>( aryInfo[0] );
		GetTypeInfo<MemberType>( aryInfo[1] );
		if( ( aryInfo[1].m_nType >> 24 ) == eDT_class &&
			( ( aryInfo[1].m_nType >> 20 )&0xf ) < eDTE_Pointer &&
			( ( aryInfo[1].m_nType >> 16 )&0xf ) < eDTE_Pointer &&
			( ( aryInfo[1].m_nType >> 12 )&0xf ) < eDTE_Pointer &&
			( ( aryInfo[1].m_nType >>  8 )&0xf ) < eDTE_Pointer &&
			( ( aryInfo[1].m_nType >>  4 )&0xf ) < eDTE_Pointer &&
			( ( aryInfo[1].m_nType       )&0xf ) < eDTE_Pointer )
			GetTypeInfo<MemberType&>( aryInfo[1] );
		STypeInfoArray TypeInfo = { aryInfo, sizeof( aryInfo )/sizeof( STypeInfo ) };
		return TypeInfo;
	}	

	//=======================================================================
	// 纯虚类支持包装
	//=======================================================================
	template<class _RetType, 
	class _P0 = int, class _P1 = int, class _P2 = int, class _P3 = int, class _P4 = int, 
	class _P5 = int, class _P6 = int, class _P7 = int, class _P8 = int, class _P9 = int, 
	class _P10 = int, class _P11 = int, class _P12 = int, class _P13 = int, class _P14 = int, 
	class _P15 = int, class _P16 = int, class _P17 = int, class _P18 = int, class _P19 = int >
	class TFunTypeExplain
	{
	public:
		typedef _RetType	RetType;
		typedef _P0 		P0 ;
		typedef _P1 		P1 ;
		typedef _P2 		P2 ;
		typedef _P3 		P3 ;
		typedef _P4 		P4 ;
		typedef _P5 		P5 ;
		typedef _P6 		P6 ;
		typedef _P7 		P7 ;
		typedef _P8 		P8 ;
		typedef _P9 		P9 ;
		typedef _P10		P10;
		typedef _P11		P11;
		typedef _P12		P12;
		typedef _P13		P13;
		typedef _P14		P14;
		typedef _P15		P15;
		typedef _P16		P16;
		typedef _P17		P17;
		typedef _P18		P18;
		typedef _P19		P19;
	};

	template<class RetType>	inline RetType	GetDefaultValue( RetType* p ) { return *p; }
	template<>				inline void		GetDefaultValue( void* ) {}

	template<typename ClassType, typename RetType, typename... Param>
	TFunTypeExplain<RetType, Param...> GetFunTypeExplain( RetType ( ClassType::*pFun )( Param... ) );
	
	#define MAKE_EXPLAIN_PARAM_0
	#define MAKE_EXPLAIN_PARAM_1 							D::E::P0
	#define MAKE_EXPLAIN_PARAM_2  	MAKE_EXPLAIN_PARAM_1, 	D::E::P1
	#define MAKE_EXPLAIN_PARAM_3  	MAKE_EXPLAIN_PARAM_2, 	D::E::P2
	#define MAKE_EXPLAIN_PARAM_4  	MAKE_EXPLAIN_PARAM_3, 	D::E::P3
	#define MAKE_EXPLAIN_PARAM_5  	MAKE_EXPLAIN_PARAM_4, 	D::E::P4
	#define MAKE_EXPLAIN_PARAM_6  	MAKE_EXPLAIN_PARAM_5, 	D::E::P5
	#define MAKE_EXPLAIN_PARAM_7  	MAKE_EXPLAIN_PARAM_6, 	D::E::P6
	#define MAKE_EXPLAIN_PARAM_8  	MAKE_EXPLAIN_PARAM_7, 	D::E::P7
	#define MAKE_EXPLAIN_PARAM_9  	MAKE_EXPLAIN_PARAM_8, 	D::E::P8
	#define MAKE_EXPLAIN_PARAM_10 	MAKE_EXPLAIN_PARAM_9, 	D::E::P9
	#define MAKE_EXPLAIN_PARAM_11 	MAKE_EXPLAIN_PARAM_10,	D::E::P10
	#define MAKE_EXPLAIN_PARAM_12 	MAKE_EXPLAIN_PARAM_11,	D::E::P11
	#define MAKE_EXPLAIN_PARAM_13 	MAKE_EXPLAIN_PARAM_12,	D::E::P12
	#define MAKE_EXPLAIN_PARAM_14 	MAKE_EXPLAIN_PARAM_13,	D::E::P13
	#define MAKE_EXPLAIN_PARAM_15 	MAKE_EXPLAIN_PARAM_14,	D::E::P14
	#define MAKE_EXPLAIN_PARAM_16 	MAKE_EXPLAIN_PARAM_15,	D::E::P15
	#define MAKE_EXPLAIN_PARAM_17 	MAKE_EXPLAIN_PARAM_16,	D::E::P16
	#define MAKE_EXPLAIN_PARAM_18 	MAKE_EXPLAIN_PARAM_17,	D::E::P17
	#define MAKE_EXPLAIN_PARAM_19 	MAKE_EXPLAIN_PARAM_18,	D::E::P18
	#define MAKE_EXPLAIN_PARAM_20 	MAKE_EXPLAIN_PARAM_19,	D::E::P19
	#define EXPLAIN_PARAM( n )		MAKE_EXPLAIN_PARAM_##n
	
#ifdef _WIN32
	#define EXPLAIN_PARAM_ORGCLASS org_class
#else
	#define EXPLAIN_PARAM_ORGCLASS D
#endif
	#define DEFINE_PUREVIRTUAL_IMPLEMENT( _fun, _base_class )\
	struct D : public org_class { typedef decltype( GetFunTypeExplain( &EXPLAIN_PARAM_ORGCLASS::_fun ) ) E; };\
	D::E::RetType _fun( EXPLAIN_PARAM( 0  ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 1  ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 2  ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 3  ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 4  ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 5  ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 6  ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 7  ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 8  ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 9  ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 10 ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 11 ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 12 ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 13 ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 14 ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 15 ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 16 ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 17 ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 18 ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 19 ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }\
	D::E::RetType _fun( EXPLAIN_PARAM( 20 ) ) { return GetDefaultValue( (D::E::RetType*)0 ); }
}
