#ifndef __GAMMA_SCRIPT_RUN_FUN_H__
#define __GAMMA_SCRIPT_RUN_FUN_H__
//=====================================================================
// GammaScriptRunFun.h 
// 定义脚本和C++接口的辅助函数和辅助宏
// 使用辅助宏可以快速注册c++类型以及函数
// 柯达昭
// 2007-10-21
//=====================================================================
#include "core/CScript.h"
#include "common/CVirtualFun.h"
#include "common/GammaCppParser.h"
#include "common/GammaCppFunArg.h"
#include "core/GammaScriptWrap.h"

namespace Gamma
{
	template<typename RetType>
	class TRunFun
	{
	public:
		static void RunFunction( CScript& Script, void* pRetBuf, const char* szFun )
		{
			static STypeInfoArray aryInfo = Gamma::MakeFunArg<RetType COMMA_PARAM( 0 )>( NULL );
			Script.RunFunction( aryInfo, pRetBuf, szFun, NULL );
		}

		template< class Param0 >
		static void RunFunction( CScript& Script, void* pRetBuf, const char* szFun, Param0 p0 )
		{
			static STypeInfoArray aryInfo = Gamma::MakeFunArg<RetType COMMA_PARAM( 1 )>( NULL );
			void* aryParam[] = { &p0 }; 
			Script.RunFunction( aryInfo, pRetBuf, szFun, aryParam );
		}

		template< class Param0, class Param1 >
		static void RunFunction( CScript& Script, void* pRetBuf, const char* szFun, Param0 p0, Param1 p1 )
		{
			static STypeInfoArray aryInfo = Gamma::MakeFunArg<RetType COMMA_PARAM( 2 )>( NULL );
			void* aryParam[] = { &p0, &p1 }; 
			Script.RunFunction( aryInfo, pRetBuf, szFun, aryParam );
		}

		template< class Param0, class Param1, class Param2 >
		static void RunFunction( CScript& Script, void* pRetBuf, const char* szFun, Param0 p0, Param1 p1, Param2 p2 )
		{
			static STypeInfoArray aryInfo = Gamma::MakeFunArg<RetType COMMA_PARAM( 3 )>( NULL );
			void* aryParam[] = { &p0, &p1, &p2 }; 
			Script.RunFunction( aryInfo, pRetBuf, szFun, aryParam );
		}

		template< class Param0, class Param1, class Param2, class Param3 >
		static void RunFunction( CScript& Script, void* pRetBuf, const char* szFun, Param0 p0, Param1 p1, Param2 p2, Param3 p3 )
		{
			static STypeInfoArray aryInfo = Gamma::MakeFunArg<RetType COMMA_PARAM( 4 )>( NULL );
			void* aryParam[] = { &p0, &p1, &p2, &p3 }; 
			Script.RunFunction( aryInfo, pRetBuf, szFun, aryParam );
		}

		template< class Param0, class Param1, class Param2, class Param3, class Param4 >
		static void RunFunction( CScript& Script, void* pRetBuf, const char* szFun, Param0 p0, Param1 p1, Param2 p2, Param3 p3, Param4 p4 )
		{
			static STypeInfoArray aryInfo = Gamma::MakeFunArg<RetType COMMA_PARAM( 5 )>( NULL );
			void* aryParam[] = { &p0, &p1, &p2, &p3, &p4 }; 
			Script.RunFunction( aryInfo, pRetBuf, szFun, aryParam );
		}

		template< class Param0, class Param1, class Param2, class Param3, class Param4, class Param5 >
		static void RunFunction( CScript& Script, void* pRetBuf, const char* szFun, Param0 p0, Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 )
		{
			static STypeInfoArray aryInfo = Gamma::MakeFunArg<RetType COMMA_PARAM( 6 )>( NULL );
			void* aryParam[] = { &p0, &p1, &p2, &p3, &p4, &p5 }; 
			Script.RunFunction( aryInfo, pRetBuf, szFun, aryParam );
		}

		template< class Param0, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6 >
		static void RunFunction( CScript& Script, void* pRetBuf, const char* szFun, Param0 p0, Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 )
		{
			static STypeInfoArray aryInfo = Gamma::MakeFunArg<RetType COMMA_PARAM( 7 )>( NULL );
			void* aryParam[] = { &p0, &p1, &p2, &p3, &p4, &p5, &p6 }; 
			Script.RunFunction( aryInfo, pRetBuf, szFun, aryParam );
		}

		template< class Param0, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7 >
		static void RunFunction( CScript& Script, void* pRetBuf, const char* szFun, Param0 p0, Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 )
		{
			static STypeInfoArray aryInfo = Gamma::MakeFunArg<RetType COMMA_PARAM( 8 )>( NULL );
			void* aryParam[] = { &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7 }; 
			Script.RunFunction( aryInfo, pRetBuf, szFun, aryParam );
		}

		template< class Param0, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8 >
		static void RunFunction( CScript& Script, void* pRetBuf, const char* szFun, Param0 p0, Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 )
		{
			static STypeInfoArray aryInfo = Gamma::MakeFunArg<RetType COMMA_PARAM( 9 )>( NULL );
			void* aryParam[] = { &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8 }; 
			Script.RunFunction( aryInfo, pRetBuf, szFun, aryParam );
		}

		template< class Param0, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9 >
		static void RunFunction( CScript& Script, void* pRetBuf, const char* szFun, Param0 p0, Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8, Param9 p9 )
		{
			static STypeInfoArray aryInfo = Gamma::MakeFunArg<RetType COMMA_PARAM( 10 )>( NULL );
			void* aryParam[] = { &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9 }; 
			Script.RunFunction( aryInfo, pRetBuf, szFun, aryParam );
		}

		template< class Param0, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class Param9, class Param10 >
		static void RunFunction( CScript& Script, void* pRetBuf, const char* szFun, Param0 p0, Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8, Param9 p9,Param10 p10 )
		{
			static STypeInfoArray aryInfo = Gamma::MakeFunArg<RetType COMMA_PARAM( 11 )>( NULL );
			void* aryParam[] = { &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10 }; 
			Script.RunFunction( aryInfo, pRetBuf, szFun, aryParam );
		}
	};
};
#endif
