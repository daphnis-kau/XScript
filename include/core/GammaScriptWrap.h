//=============================================================
// GammaScriptWrap.h 
// 生成函数调用以及回调的包装
// 柯达昭
// 2012-08-09
//=====================================================================
#pragma once
#include "common/GammaCppTemplateHelp.h"
#include "core/GammaScriptDef.h"

namespace Gamma
{
#define MAKE_CALLARG_FETCH_0
#define MAKE_CALLARG_FETCH_1 							Param0  p0  = (Param0)ArgFetcher<Param0>::CallWrapArg( pArgArray[0] ); 
#define MAKE_CALLARG_FETCH_2  	MAKE_CALLARG_FETCH_1  	Param1  p1  = (Param1)ArgFetcher<Param1>::CallWrapArg( pArgArray[1] ); 
#define MAKE_CALLARG_FETCH_3  	MAKE_CALLARG_FETCH_2  	Param2  p2  = (Param2)ArgFetcher<Param2>::CallWrapArg( pArgArray[2] ); 
#define MAKE_CALLARG_FETCH_4  	MAKE_CALLARG_FETCH_3  	Param3  p3  = (Param3)ArgFetcher<Param3>::CallWrapArg( pArgArray[3] ); 
#define MAKE_CALLARG_FETCH_5  	MAKE_CALLARG_FETCH_4  	Param4  p4  = (Param4)ArgFetcher<Param4>::CallWrapArg( pArgArray[4] ); 
#define MAKE_CALLARG_FETCH_6  	MAKE_CALLARG_FETCH_5  	Param5  p5  = (Param5)ArgFetcher<Param5>::CallWrapArg( pArgArray[5] ); 
#define MAKE_CALLARG_FETCH_7  	MAKE_CALLARG_FETCH_6  	Param6  p6  = (Param6)ArgFetcher<Param6>::CallWrapArg( pArgArray[6] ); 
#define MAKE_CALLARG_FETCH_8  	MAKE_CALLARG_FETCH_7  	Param7  p7  = (Param7)ArgFetcher<Param7>::CallWrapArg( pArgArray[7] ); 
#define MAKE_CALLARG_FETCH_9  	MAKE_CALLARG_FETCH_8  	Param8  p8  = (Param8)ArgFetcher<Param8>::CallWrapArg( pArgArray[8] ); 
#define MAKE_CALLARG_FETCH_10 	MAKE_CALLARG_FETCH_9  	Param9  p9  = (Param9)ArgFetcher<Param9>::CallWrapArg( pArgArray[9] ); 
#define MAKE_CALLARG_FETCH_11 	MAKE_CALLARG_FETCH_10	Param10 p10 = (Param10)ArgFetcher<Param10>::CallWrapArg( pArgArray[10] ); 
#define MAKE_CALLARG_FETCH_12 	MAKE_CALLARG_FETCH_11	Param11 p11 = (Param11)ArgFetcher<Param11>::CallWrapArg( pArgArray[11] ); 
#define MAKE_CALLARG_FETCH_13 	MAKE_CALLARG_FETCH_12	Param12 p12 = (Param12)ArgFetcher<Param12>::CallWrapArg( pArgArray[12] ); 
#define MAKE_CALLARG_FETCH_14 	MAKE_CALLARG_FETCH_13	Param13 p13 = (Param13)ArgFetcher<Param13>::CallWrapArg( pArgArray[13] ); 
#define MAKE_CALLARG_FETCH_15 	MAKE_CALLARG_FETCH_14	Param14 p14 = (Param14)ArgFetcher<Param14>::CallWrapArg( pArgArray[14] ); 
#define MAKE_CALLARG_FETCH_16 	MAKE_CALLARG_FETCH_15	Param15 p15 = (Param15)ArgFetcher<Param15>::CallWrapArg( pArgArray[15] ); 
#define MAKE_CALLARG_FETCH_17 	MAKE_CALLARG_FETCH_16	Param16 p16 = (Param16)ArgFetcher<Param16>::CallWrapArg( pArgArray[16] ); 
#define MAKE_CALLARG_FETCH_18 	MAKE_CALLARG_FETCH_17	Param17 p17 = (Param17)ArgFetcher<Param17>::CallWrapArg( pArgArray[17] ); 
#define MAKE_CALLARG_FETCH_19 	MAKE_CALLARG_FETCH_18	Param18 p18 = (Param18)ArgFetcher<Param18>::CallWrapArg( pArgArray[18] ); 
#define MAKE_CALLARG_FETCH_20 	MAKE_CALLARG_FETCH_19	Param19 p19 = (Param19)ArgFetcher<Param19>::CallWrapArg( pArgArray[19] ); 
#define CALLARG_FETCH( n )		MAKE_CALLARG_FETCH_##n

#define MAKE_CALLBACKARG_FETCH_FETCH_0
#define MAKE_CALLBACKARG_FETCH_FETCH_1 									ArgFetcher<Param0>::CallBackArg( pArgArray[0] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_2  MAKE_CALLBACKARG_FETCH_FETCH_1  ArgFetcher<Param1>::CallBackArg( pArgArray[1] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_3  MAKE_CALLBACKARG_FETCH_FETCH_2  ArgFetcher<Param2>::CallBackArg( pArgArray[2] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_4  MAKE_CALLBACKARG_FETCH_FETCH_3  ArgFetcher<Param3>::CallBackArg( pArgArray[3] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_5  MAKE_CALLBACKARG_FETCH_FETCH_4  ArgFetcher<Param4>::CallBackArg( pArgArray[4] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_6  MAKE_CALLBACKARG_FETCH_FETCH_5  ArgFetcher<Param5>::CallBackArg( pArgArray[5] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_7  MAKE_CALLBACKARG_FETCH_FETCH_6  ArgFetcher<Param6>::CallBackArg( pArgArray[6] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_8  MAKE_CALLBACKARG_FETCH_FETCH_7  ArgFetcher<Param7>::CallBackArg( pArgArray[7] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_9  MAKE_CALLBACKARG_FETCH_FETCH_8  ArgFetcher<Param8>::CallBackArg( pArgArray[8] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_10 MAKE_CALLBACKARG_FETCH_FETCH_9  ArgFetcher<Param9>::CallBackArg( pArgArray[9] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_11 MAKE_CALLBACKARG_FETCH_FETCH_10	ArgFetcher<Param10>::CallBackArg( pArgArray[10] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_12 MAKE_CALLBACKARG_FETCH_FETCH_11	ArgFetcher<Param11>::CallBackArg( pArgArray[11] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_13 MAKE_CALLBACKARG_FETCH_FETCH_12	ArgFetcher<Param12>::CallBackArg( pArgArray[12] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_14 MAKE_CALLBACKARG_FETCH_FETCH_13	ArgFetcher<Param13>::CallBackArg( pArgArray[13] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_15 MAKE_CALLBACKARG_FETCH_FETCH_14	ArgFetcher<Param14>::CallBackArg( pArgArray[14] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_16 MAKE_CALLBACKARG_FETCH_FETCH_15	ArgFetcher<Param15>::CallBackArg( pArgArray[15] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_17 MAKE_CALLBACKARG_FETCH_FETCH_16	ArgFetcher<Param16>::CallBackArg( pArgArray[16] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_18 MAKE_CALLBACKARG_FETCH_FETCH_17	ArgFetcher<Param17>::CallBackArg( pArgArray[17] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_19 MAKE_CALLBACKARG_FETCH_FETCH_18	ArgFetcher<Param18>::CallBackArg( pArgArray[18] ),
#define MAKE_CALLBACKARG_FETCH_FETCH_20 MAKE_CALLBACKARG_FETCH_FETCH_19	ArgFetcher<Param19>::CallBackArg( pArgArray[19] ),
#define CALLBACKARG_FETCH_FETCH( n )	MAKE_CALLBACKARG_FETCH_FETCH_##n

	//=======================================================================
	// 函数调用包装
	//=======================================================================
	template< typename T >
	struct ArgFetcher
	{
		static inline T&	CallWrapArg( void*  pArgArray )	{ return *(T*)pArgArray; }
		static inline void* CallBackArg( void*& pArgArray )	{ return pArgArray; }
	};

	template<typename T>
	struct ArgFetcher<T&>
	{
		static inline T&	CallWrapArg( void* pArgArray )	{ return **(T**)pArgArray; }
		static inline void* CallBackArg( void*& pArgArray )	{ return &pArgArray; }
	};

#define SIMPLE_TYPE_ARG_FETCHER( T ) \
	template<> \
	struct ArgFetcher<const T&> \
	{ \
		static inline const T&	CallWrapArg( void* pArgArray )	{ return *(const T*)pArgArray; } \
		static inline void* CallBackArg( void*& pArgArray )	{ return pArgArray; } \
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

	template< typename RetType >
	class TFunctionWrapBase : public IFunctionWrap
	{
		virtual RetType CallWrap( void* pObj, void** pArgArray, SFunction funRaw ) = 0;

	public:
		void Call( void* pObj, void* pRetBuf, void** pArgArray, SFunction funRaw )
		{
			new( pRetBuf ) RetType( CallWrap( pObj, pArgArray, funRaw ) );
		}
	};

	template< typename RetType >
	class TFunctionWrapBase<RetType&> : public IFunctionWrap
	{
		virtual RetType& CallWrap( void* pObj, void** pArgArray, SFunction funRaw ) = 0;

	public:
		void Call( void* pObj, void* pRetBuf, void** pArgArray, SFunction funRaw )
		{
			*(RetType**)pRetBuf = &( CallWrap( pObj, pArgArray, funRaw ) );
		}
	};

	template<>
	class TFunctionWrapBase<void> : public IFunctionWrap
	{
		virtual void CallWrap( void* pObj, void** pArgArray, SFunction funRaw ) = 0;

	public:
		void Call( void* pObj, void* pRetBuf, void** pArgArray, SFunction funRaw )
		{
			CallWrap( pObj, pArgArray, funRaw );
		}
	};

	enum ECallType
	{
		eCT_CDecl,
		eCT_StdCall,
		eCT_ThisCall,
	};	

#ifdef _WIN32
#	define DEFINE_STD_TYPE( n ) typedef RetType (__stdcall *StdCallType)( PARAM( n ) );
#	define CHECK_STD_CALL( n ) \
		if( eType == eCT_StdCall ) \
			return ( *MakeFun<StdCallType>( funRaw.funPoint ? funRaw : m_funOrg ) )( VALUE( n ) );
#else
#	define DEFINE_STD_TYPE( n )
#	define CHECK_STD_CALL( n )
#endif

#define DEFINE_TFUNCTIONWRAP( n ) \
	template< ECallType eType, class ClassType, class RetType CLASS_PARAM( n ) >\
	class TFunctionWrap##n : public TFunctionWrapBase<RetType>\
	{\
		DEFINE_STD_TYPE( n );\
		typedef RetType (*CDeclType)( PARAM( n ) );\
		typedef RetType ( ClassType::*ThisCallType)( PARAM( n ) );\
		typedef RetType ( ClassType::*ThisCallConstType)( PARAM( n ) ) const;\
		union{ CDeclType m_funDeclType; ThisCallType m_funThisCallType; SFunction m_funOrg; };\
	public:\
		TFunctionWrap##n( CDeclType funOrg ) : m_funDeclType( funOrg ){}\
		TFunctionWrap##n( ThisCallType funOrg ) : m_funThisCallType( funOrg ){}\
		TFunctionWrap##n( ThisCallConstType funOrg ) : m_funThisCallType( (ThisCallType)funOrg ){}\
		SFunction GetOrgFun() { return m_funOrg; }\
\
		RetType CallWrap( void* pObj, void** pArgArray, SFunction funRaw )\
		{ \
			CALLARG_FETCH( n );\
			CHECK_STD_CALL( n );\
			union UFunction{ CDeclType funDeclType; ThisCallType funThisCallType; SFunction funOrg; };\
			UFunction funCall; \
			funCall.funOrg = funRaw.funPoint ? funRaw : GetOrgFun();\
			if( eType == eCT_CDecl )\
				return ( *funCall.funDeclType )( VALUE( n ) );\
			return ( ( ( ClassType* )pObj )->*( funCall.funThisCallType ) )( VALUE( n ) );\
		}\
	};

	DEFINE_TFUNCTIONWRAP( 0 );
	DEFINE_TFUNCTIONWRAP( 1 );
	DEFINE_TFUNCTIONWRAP( 2 );
	DEFINE_TFUNCTIONWRAP( 3 );
	DEFINE_TFUNCTIONWRAP( 4 );
	DEFINE_TFUNCTIONWRAP( 5 );
	DEFINE_TFUNCTIONWRAP( 6 );
	DEFINE_TFUNCTIONWRAP( 7 );
	DEFINE_TFUNCTIONWRAP( 8 );
	DEFINE_TFUNCTIONWRAP( 9 );
	DEFINE_TFUNCTIONWRAP( 10 );
	DEFINE_TFUNCTIONWRAP( 11 );
	DEFINE_TFUNCTIONWRAP( 12 );
	DEFINE_TFUNCTIONWRAP( 13 );
	DEFINE_TFUNCTIONWRAP( 14 );
	DEFINE_TFUNCTIONWRAP( 15 );
	DEFINE_TFUNCTIONWRAP( 16 );
	DEFINE_TFUNCTIONWRAP( 17 );
	DEFINE_TFUNCTIONWRAP( 18 );
	DEFINE_TFUNCTIONWRAP( 19 );
	DEFINE_TFUNCTIONWRAP( 20 );
						  
#if( defined _WIN32 && !( defined _M_X64 ) )
#define DEFINE_STAT_TFUNCTIONWRAP_CREATEER( n )\
	template< class RetType CLASS_PARAM( n ) >\
	inline IFunctionWrap* CreateFunWrap( RetType ( __stdcall *pFun )( PARAM( n ) ) )\
	{\
		return new TFunctionWrap00<eCT_StdCall, IFunctionWrap, RetType  COMMA_PARAM( n )>( pFun );\
	}
#else
#define DEFINE_STAT_TFUNCTIONWRAP_CREATEER( n )
#endif

#define DEFINE_TFUNCTIONWRAP_CREATEER( n )\
	template< class ClassType, class RetType CLASS_PARAM( n ) >\
	inline IFunctionWrap* CreateFunWrap( RetType ( ClassType::*pFun )( PARAM( n ) ) )\
	{\
		return new TFunctionWrap##n<eCT_ThisCall, ClassType, RetType  COMMA_PARAM( n ) >( pFun );\
	}\
	template< class ClassType, class RetType CLASS_PARAM( n ) >\
	inline IFunctionWrap* CreateFunWrap( RetType ( ClassType::*pFun )( PARAM( n ) ) const )\
	{\
		return new TFunctionWrap##n<eCT_ThisCall, ClassType, RetType  COMMA_PARAM( n ) >( pFun );\
	}\
	template< class RetType CLASS_PARAM( n ) >\
	inline IFunctionWrap* CreateFunWrap( RetType ( *pFun )( PARAM( n ) ) )\
	{\
		return new TFunctionWrap##n<eCT_CDecl, IFunctionWrap, RetType  COMMA_PARAM( n ) >( pFun );\
	}\
	DEFINE_STAT_TFUNCTIONWRAP_CREATEER( n )

	DEFINE_TFUNCTIONWRAP_CREATEER( 0  );
	DEFINE_TFUNCTIONWRAP_CREATEER( 1  );
	DEFINE_TFUNCTIONWRAP_CREATEER( 2  );
	DEFINE_TFUNCTIONWRAP_CREATEER( 3  );
	DEFINE_TFUNCTIONWRAP_CREATEER( 4  );
	DEFINE_TFUNCTIONWRAP_CREATEER( 5  );
	DEFINE_TFUNCTIONWRAP_CREATEER( 6  );
	DEFINE_TFUNCTIONWRAP_CREATEER( 7  );
	DEFINE_TFUNCTIONWRAP_CREATEER( 8  );
	DEFINE_TFUNCTIONWRAP_CREATEER( 9  );
	DEFINE_TFUNCTIONWRAP_CREATEER( 10 );
	DEFINE_TFUNCTIONWRAP_CREATEER( 11 );
	DEFINE_TFUNCTIONWRAP_CREATEER( 12 );
	DEFINE_TFUNCTIONWRAP_CREATEER( 13 );
	DEFINE_TFUNCTIONWRAP_CREATEER( 14 );
	DEFINE_TFUNCTIONWRAP_CREATEER( 15 );
	DEFINE_TFUNCTIONWRAP_CREATEER( 16 );
	DEFINE_TFUNCTIONWRAP_CREATEER( 17 );
	DEFINE_TFUNCTIONWRAP_CREATEER( 18 );
	DEFINE_TFUNCTIONWRAP_CREATEER( 19 );
	DEFINE_TFUNCTIONWRAP_CREATEER( 20 );

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

#define DEFINE_TCALLBACKWRAP( n ) \
	template<int k, class RetType, class ClassType CLASS_PARAM( n ) > \
	class TCallBackWrap##n \
	{\
	public:\
		static int32& GetCallBackIndex() \
		{\
			static int32 s_nCallBackIndex = -1;\
			return s_nCallBackIndex;\
		}\
\
		static bool SetCallBack( ICallBackWrap& CallBackWrap, bool bPureVirtual ) \
		{\
			int32 nIndex = CallBackWrap.BindFunction( \
				GetFunAdress( &TCallBackWrap##n::Wrap ), bPureVirtual );\
			if( nIndex == GetCallBackIndex() )\
				return true; \
			assert( GetCallBackIndex() < 0 );\
			GetCallBackIndex() = nIndex;\
			return true;\
		}\
\
		RetType Wrap( PARAM_VALUE( n ) )	\
		{ \
			void* pArgArray[] = { VALUE_ADDR( n ) NULL };\
			void* pCallArray[] ={ CALLBACKARG_FETCH_FETCH( n ) NULL, pArgArray };\
			return TCallBack<RetType>::OnCall( GetCallBackIndex(), this, pCallArray );\
		}\
	};

	DEFINE_TCALLBACKWRAP( 0  );
	DEFINE_TCALLBACKWRAP( 1  );
	DEFINE_TCALLBACKWRAP( 2  );
	DEFINE_TCALLBACKWRAP( 3  );
	DEFINE_TCALLBACKWRAP( 4  );
	DEFINE_TCALLBACKWRAP( 5  );
	DEFINE_TCALLBACKWRAP( 6  );
	DEFINE_TCALLBACKWRAP( 7  );
	DEFINE_TCALLBACKWRAP( 8  );
	DEFINE_TCALLBACKWRAP( 9  );
	DEFINE_TCALLBACKWRAP( 10 );
	DEFINE_TCALLBACKWRAP( 11 );
	DEFINE_TCALLBACKWRAP( 12 );
	DEFINE_TCALLBACKWRAP( 13 );
	DEFINE_TCALLBACKWRAP( 14 );
	DEFINE_TCALLBACKWRAP( 15 );
	DEFINE_TCALLBACKWRAP( 16 );
	DEFINE_TCALLBACKWRAP( 17 );
	DEFINE_TCALLBACKWRAP( 18 );
	DEFINE_TCALLBACKWRAP( 19 );
	DEFINE_TCALLBACKWRAP( 20 );

#define DEFINE_TCALLBACKWRAP_CREATER( n ) \
	template< class ClassType, class RetType CLASS_PARAM( n ) >\
	static inline void BindWrap( ICallBackWrap& CallBackWrap, \
		bool bPureVirtual, RetType ( ClassType::*pFun )( PARAM( n ) ) )\
	{\
		TCallBackWrap##n<nInstance, RetType, ClassType COMMA_PARAM( n )>\
		::SetCallBack( CallBackWrap, bPureVirtual );\
	}\
	template< class ClassType, class RetType CLASS_PARAM( n ) >\
	static inline void BindWrap( ICallBackWrap& CallBackWrap, \
		bool bPureVirtual, RetType ( ClassType::*pFun )( PARAM( n ) ) const )\
	{\
		TCallBackWrap##n<nInstance, RetType, ClassType COMMA_PARAM( n )>\
		::SetCallBack( CallBackWrap, bPureVirtual );\
	}

	template<int32 nInstance>
	class CCallBackBinder
	{
	public:
		DEFINE_TCALLBACKWRAP_CREATER( 0  );
		DEFINE_TCALLBACKWRAP_CREATER( 1  );
		DEFINE_TCALLBACKWRAP_CREATER( 2  );
		DEFINE_TCALLBACKWRAP_CREATER( 3  );
		DEFINE_TCALLBACKWRAP_CREATER( 4  );
		DEFINE_TCALLBACKWRAP_CREATER( 5  );
		DEFINE_TCALLBACKWRAP_CREATER( 6  );
		DEFINE_TCALLBACKWRAP_CREATER( 7  );
		DEFINE_TCALLBACKWRAP_CREATER( 8  );
		DEFINE_TCALLBACKWRAP_CREATER( 9  );
		DEFINE_TCALLBACKWRAP_CREATER( 10 );
		DEFINE_TCALLBACKWRAP_CREATER( 11 );
		DEFINE_TCALLBACKWRAP_CREATER( 12 );
		DEFINE_TCALLBACKWRAP_CREATER( 13 );
		DEFINE_TCALLBACKWRAP_CREATER( 14 );
		DEFINE_TCALLBACKWRAP_CREATER( 15 );
		DEFINE_TCALLBACKWRAP_CREATER( 16 );
		DEFINE_TCALLBACKWRAP_CREATER( 17 );
		DEFINE_TCALLBACKWRAP_CREATER( 18 );
		DEFINE_TCALLBACKWRAP_CREATER( 19 );
		DEFINE_TCALLBACKWRAP_CREATER( 20 );
	};

	#ifdef _WIN32
		#define BIND_CALLBACK( wrap, pureVirtual, fun ) \
			CCallBackBinder<__COUNTER__>::BindWrap( wrap, pureVirtual, fun )
	#else
		#define BIND_CALLBACK( wrap, pureVirtual, fun ) \
			CCallBackBinder<__LINE__>::BindWrap( wrap, pureVirtual, fun ) 
	#endif

	//=======================================================================
	// 析构函数调用包装
	//=======================================================================
	template< class ClassType >
	inline IFunctionWrap* CreateDestructorWrap( uint32 nIndex )
	{
		class _FunWrap : public TFunctionWrapBase<void>
		{
			uint32 m_nIndex;
		public:
			_FunWrap( uint32 nIndex ) : m_nIndex( nIndex ){}
			SFunction GetOrgFun()	{ return GetFunction( m_nIndex ); }

			void CallWrap( void* pObj, void** pArgArray, SFunction funRaw )
			{ 
				class Derive : public ClassType	{ public: ~Derive(){}; };
				( ( Derive* )pObj )->~Derive();
			}
		};

		return new _FunWrap( nIndex );
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
	inline IFunctionWrap* CreateMemberGetWrap( ClassType* pClass, MemberType* pMember )
	{
		class _FunWrap : public TFunctionWrapBase<MemberType>
		{
			uint32 m_nOffset;
		public:
			_FunWrap( uint32 nOffset ) : m_nOffset( nOffset ){}
			SFunction GetOrgFun() { SFunction fun = { 0, 0 }; return fun; }

			MemberType CallWrap( void* pObj, void** pArgArray, SFunction )
			{ 
				return *(MemberType*)( (char*)pObj + m_nOffset );
			}
		};

		class _FunWrapObject : public TFunctionWrapBase<MemberType*>
		{
			uint32 m_nOffset;
		public:
			_FunWrapObject( uint32 nOffset ) : m_nOffset( nOffset ){}
			SFunction GetOrgFun() { SFunction fun = { 0, 0 }; return fun; }

			MemberType* CallWrap( void* pObj, void** pArgArray, SFunction )
			{ 
				return (MemberType*)( (char*)pObj + m_nOffset );
			}
		};

		STypeInfo TypeInfo;
		GetTypeInfo<MemberType>( TypeInfo );
		if( ( TypeInfo.m_nType >> 24 ) != eDT_class ||
			( ( TypeInfo.m_nType >> 20 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType >> 16 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType >> 12 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType >>  8 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType >>  4 )&0xf ) >= eDTE_Pointer ||
			( ( TypeInfo.m_nType       )&0xf ) >= eDTE_Pointer )
			return new _FunWrap( (uint32)(ptrdiff_t)( (char*)pMember - (char*)pClass ) );
		return new _FunWrapObject( (uint32)(ptrdiff_t)( (char*)pMember - (char*)pClass ) );
	}

	//=======================================================================
	// 成员写入包装
	//=======================================================================
	template< class ClassType, class MemberType >
	inline IFunctionWrap* CreateMemberSetWrap( ClassType* pClass, MemberType* pMember )
	{
		class _FunWrap : public TFunctionWrapBase<void>
		{
			uint32 m_nOffset;
		public:
			_FunWrap( uint32 nOffset ) : m_nOffset( nOffset ){}
			SFunction GetOrgFun() { SFunction fun = { 0, 0 }; return fun; }

			void CallWrap( void* pObj, void** pArgArray, SFunction )
			{ 
				*(MemberType*)( (char*)pObj + m_nOffset ) = ArgFetcher<MemberType>::CallWrapArg( pArgArray[0] );
			}
		};

		return new _FunWrap( (uint32)(ptrdiff_t)( (char*)pMember - (char*)pClass ) );
	}

	template< class ClassType, class MemberType >
	inline STypeInfoArray MakeMemberArg( ClassType*, MemberType* )
	{
		STypeInfoArray aryInfo;
		aryInfo.nSize = 2;
		GetTypeInfo<ClassType*>( aryInfo.aryInfo[0] );
		GetTypeInfo<MemberType>( aryInfo.aryInfo[1] );
		if( ( aryInfo.aryInfo[1].m_nType >> 24 ) == eDT_class &&
			( ( aryInfo.aryInfo[1].m_nType >> 20 )&0xf ) < eDTE_Pointer &&
			( ( aryInfo.aryInfo[1].m_nType >> 16 )&0xf ) < eDTE_Pointer &&
			( ( aryInfo.aryInfo[1].m_nType >> 12 )&0xf ) < eDTE_Pointer &&
			( ( aryInfo.aryInfo[1].m_nType >>  8 )&0xf ) < eDTE_Pointer &&
			( ( aryInfo.aryInfo[1].m_nType >>  4 )&0xf ) < eDTE_Pointer &&
			( ( aryInfo.aryInfo[1].m_nType       )&0xf ) < eDTE_Pointer )
			GetTypeInfo<MemberType&>( aryInfo.aryInfo[1] );
		return aryInfo;
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

	#define DEFINE_FUNCTION_EXPLAINER( n ) \
	template<class ClassType, class RetType CLASS_PARAM( n )>\
	TFunTypeExplain<RetType COMMA_PARAM( n )> GetFunTypeExplain( RetType ( ClassType::*pFun )( PARAM( n ) ) );

	DEFINE_FUNCTION_EXPLAINER( 0  );
	DEFINE_FUNCTION_EXPLAINER( 1  );
	DEFINE_FUNCTION_EXPLAINER( 2  );
	DEFINE_FUNCTION_EXPLAINER( 3  );
	DEFINE_FUNCTION_EXPLAINER( 4  );
	DEFINE_FUNCTION_EXPLAINER( 5  );
	DEFINE_FUNCTION_EXPLAINER( 6  );
	DEFINE_FUNCTION_EXPLAINER( 7  );
	DEFINE_FUNCTION_EXPLAINER( 8  );
	DEFINE_FUNCTION_EXPLAINER( 9  );
	DEFINE_FUNCTION_EXPLAINER( 10 );
	DEFINE_FUNCTION_EXPLAINER( 11 );
	DEFINE_FUNCTION_EXPLAINER( 12 );
	DEFINE_FUNCTION_EXPLAINER( 13 );
	DEFINE_FUNCTION_EXPLAINER( 14 );
	DEFINE_FUNCTION_EXPLAINER( 15 );
	DEFINE_FUNCTION_EXPLAINER( 16 );
	DEFINE_FUNCTION_EXPLAINER( 17 );
	DEFINE_FUNCTION_EXPLAINER( 18 );
	DEFINE_FUNCTION_EXPLAINER( 19 );
	DEFINE_FUNCTION_EXPLAINER( 20 );
	
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
