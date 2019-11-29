//=============================================================
// GammaScriptWrap.h 
// 生成函数调用以及回调的包装
// 柯达昭
// 2012-08-09
//=====================================================================
#pragma once
#include "core/CScriptBase.h"

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

	SIMPLE_TYPE_ARG_FETCHER( char )
	SIMPLE_TYPE_ARG_FETCHER( int8 )
	SIMPLE_TYPE_ARG_FETCHER( int16 )
	SIMPLE_TYPE_ARG_FETCHER( int32 )
	SIMPLE_TYPE_ARG_FETCHER( int64 )
	SIMPLE_TYPE_ARG_FETCHER( long )
	SIMPLE_TYPE_ARG_FETCHER( wchar_t )
	SIMPLE_TYPE_ARG_FETCHER( uint8 )
	SIMPLE_TYPE_ARG_FETCHER( uint16 )
	SIMPLE_TYPE_ARG_FETCHER( uint32 )
	SIMPLE_TYPE_ARG_FETCHER( uint64 )
	SIMPLE_TYPE_ARG_FETCHER( ulong )
	SIMPLE_TYPE_ARG_FETCHER( float )
	SIMPLE_TYPE_ARG_FETCHER( double )
#undef SIMPLE_TYPE_ARG_FETCHER

	enum ECallType
	{
		eCT_CDecl,
		eCT_ThisCall,
	};	

	template<typename ClassType, typename RetType, typename... Param>
	struct TCallFunctionTrait
	{
	public:
		typedef RetType ( *CDeclType )( Param... );
		typedef RetType ( ClassType::*ThisCallType )( Param... );
		typedef RetType ( ClassType::*ThisCallConstType )( Param... ) const;
		union UFunction { CDeclType funDecl; ThisCallType funThisCall; SFunction funOrg; };
	};

	template<ECallType eType, typename ClassType, typename RetType, typename... Param>
	struct TCallFunction 
	{
	public:
		typedef TCallFunctionTrait<ClassType, RetType, Param...> CallFunctionTrait;
		typedef typename CallFunctionTrait::UFunction UFunction;
		static RetType CallFunction( void* pObj, UFunction funCall, void* pRetBuf, Param...p )
		{ return ( ( (ClassType*)pObj )->*( funCall.funThisCall ) )( p... ); }
	};

	template<typename ClassType, typename RetType, typename... Param>
	struct TCallFunction<eCT_CDecl, ClassType, RetType, Param...>
	{
	public:
		typedef TCallFunctionTrait<ClassType, RetType, Param...> CallFunctionTrait;
		typedef typename CallFunctionTrait::UFunction UFunction;
		static RetType CallFunction( void* pObj, UFunction funCall, void* pRetBuf, Param...p )
		{ return ( funCall.funDecl )( p... ); }
	};

	template< ECallType eType, typename ClassType, typename RetType, typename... Param >
	class TFunctionWrap : public IFunctionWrap
	{
		typedef TCallFunction<eType, ClassType, RetType, Param...> CallFunctionType;
		typedef TCallFunctionTrait<ClassType, RetType, Param...> CallFunctionTrait;
		typedef typename CallFunctionTrait::UFunction UFunction;
		typedef typename CallFunctionTrait::CDeclType CDeclType;
		typedef typename CallFunctionTrait::ThisCallType ThisCallType;
		typedef typename CallFunctionTrait::ThisCallConstType ThisCallConstType;
		union { CDeclType m_funDecl; ThisCallType m_funThisCall; SFunction m_funOrg; };

		template< typename Type > struct TCallFunction
		{
			TCallFunction( void* pObj, UFunction funCall, void* pRetBuf, Param...p )
			{ new( pRetBuf ) Type( CallFunctionType::CallFunction(pObj, funCall, pRetBuf, p...) ); }
		};

		template< typename Type > struct TCallFunction<Type&>
		{
			TCallFunction( void* pObj, UFunction funCall, void* pRetBuf, Param...p )
			{ *(Type**)pRetBuf = &( CallFunctionType::CallFunction( pObj, funCall, pRetBuf, p... ) ); }
		};

		template<> struct TCallFunction<void>
		{
			TCallFunction( void* pObj, UFunction funCall, void* pRetBuf, Param...p )
			{ CallFunctionType::CallFunction( pObj, funCall, pRetBuf, p... ); }
		};

		template<typename... RemainParam> struct TFetchParam {};
		template<> struct TFetchParam<>
		{
			template<typename... FetchParam>
			static void Fetch( size_t nIndex, void* pObj, UFunction funCall, void* pRetBuf, void** pArgArray, FetchParam...p )
			{ TCallFunction<RetType> Temp( pObj, funCall, pRetBuf, p... ); }
		};

		template<typename FirstParam, typename... RemainParam>
		struct TFetchParam<FirstParam, RemainParam...>
		{
			template<typename... FetchParam>
			static void Fetch( size_t nIndex, void* pObj, UFunction funCall, void* pRetBuf, void** pArgArray, FetchParam...p )
			{ TFetchParam<RemainParam...>::Fetch( nIndex + 1, pObj, funCall, pRetBuf, pArgArray, p..., ArgFetcher<FirstParam>::CallWrapArg( pArgArray[nIndex] ) ); }
		};

	public:
		TFunctionWrap( CDeclType funOrg ) : m_funDecl( funOrg ) {}
		TFunctionWrap( ThisCallType funOrg ) : m_funThisCall( funOrg ){}
		TFunctionWrap( ThisCallConstType funOrg ) : m_funThisCall( (ThisCallType)funOrg ){}
		SFunction GetOrgFun() { return m_funOrg; }

		void Call( void* pObj, void* pRetBuf, void** pArgArray, SFunction funRaw )
		{
			UFunction funCall;
			funCall.funOrg = funRaw.funPoint ? funRaw : m_funOrg;
			TFetchParam<Param...>::Fetch( 0, pObj, funCall, pRetBuf, pArgArray );
		}
	};

	template< typename ClassType, typename RetType, typename... Param >
	inline IFunctionWrap* CreateFunWrap( RetType ( ClassType::*pFun )( Param... ) )
	{ return new TFunctionWrap<eCT_ThisCall, ClassType, RetType, Param...>( pFun ); }

	template< typename ClassType, typename RetType, typename... Param >
	inline IFunctionWrap* CreateFunWrap( RetType ( ClassType::*pFun )( Param... ) const )
	{ return new TFunctionWrap<eCT_ThisCall, ClassType, RetType, Param...>( pFun ); }

	template< typename RetType, typename... Param >
	inline IFunctionWrap* CreateFunWrap( RetType ( *pFun )( Param... ) )
	{ return new TFunctionWrap<eCT_CDecl, IFunctionWrap, RetType, Param...>( pFun ); }

	//=======================================================================
	// 类非常量成员函数回调包装
	//=======================================================================
	template< typename T >
	struct TCallBack
	{
		static T OnCall( uint32 nCallBackIndex, void* pObject, void** pArgArray )
		{
			T ReturnValue;
			CScriptBase::CallBack( nCallBackIndex, pObject, &ReturnValue, pArgArray );
			return ReturnValue;
		}
	};

	template<typename T>
	struct TCallBack<T&>
	{
		static T& OnCall( uint32 nCallBackIndex, void* pObject, void** pArgArray )
		{
			T* pReturnValue;
			int32 ret = CScriptBase::CallBack( nCallBackIndex, pObject, &pReturnValue, pArgArray );
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
			CScriptBase::CallBack( nCallBackIndex, pObject, NULL, pArgArray );
		}
	};

	template<int32 nInstance>
	class CCallBackBinder
	{
		template<typename RetType, typename ClassType, typename... Param >
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
		template< typename ClassType, typename RetType, typename... Param >
		static inline void BindWrap( ICallBackWrap& CallBackWrap,
			bool bPureVirtual, RetType ( ClassType::*pFun )( Param... ) )
		{
			TCallBackWrap<RetType, ClassType, Param...>::SetCallBack( CallBackWrap, bPureVirtual ); 
		}

		template< typename ClassType, typename RetType, typename... Param >
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
	template< typename ClassType >
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

	template< typename ClassType >
	inline IFunctionWrap* CreateDestructorWrap( uint32 nIndex )
	{
		return new TDestructorWrap<ClassType>( nIndex );
	}

	//=======================================================================
	// 析构函数调用包装绑定
	//=======================================================================
	template<typename ClassType>
	static inline void BindDestructorWrap( ICallBackWrap& CallBackWrap )
	{
		static int32 s_nIndex = -1;
		class _FunWrap
		{
		public:
			void Wrap( uint32 p0 )	
			{ 
				void* pArg = &p0;
				CScriptBase::CallBack( s_nIndex, this, NULL, &pArg );
			}
		};

		s_nIndex = CallBackWrap.BindFunction( GetFunAdress( &_FunWrap::Wrap ), false );
	}

	//=======================================================================
	// 成员读取包装
	//=======================================================================
	template< typename ClassType, typename MemberType >
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

	template< typename ClassType, typename MemberType >
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

	template< typename ClassType, typename MemberType >
	inline IFunctionWrap* CreateMemberGetWrap( ClassType* pClass, MemberType* pMember )
	{
		STypeInfo TypeInfo;
		GetTypeInfo<MemberType>( TypeInfo );
		if( ( TypeInfo.m_nType >> 24 ) != eDT_custom_type ||
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
	template< typename ClassType, typename MemberType >
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

	template< typename ClassType, typename MemberType >
	inline IFunctionWrap* CreateMemberSetWrap( ClassType* pClass, MemberType* pMember )
	{
		return new TMemberSetWrap<ClassType, MemberType>( (char*)pMember - (char*)pClass );
	}

	template< typename ClassType, typename MemberType >
	inline STypeInfoArray MakeMemberArg( ClassType*, MemberType* )
	{
		static STypeInfo aryInfo[2];
		GetTypeInfo<ClassType*>( aryInfo[0] );
		GetTypeInfo<MemberType>( aryInfo[1] );
		if( ( aryInfo[1].m_nType >> 24 ) == eDT_custom_type &&
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
}
