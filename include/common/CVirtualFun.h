#pragma once
#include "common/GammaCpp.h"

namespace Gamma
{
#ifndef AS3_ALCHEMY_SWIG

	#define MAX_VTABLE_SIZE        512

	//假定c++对象虚表结构为：（与编译器相关，在vc++和gcc中成立）
	// pObj-> ----------------
	//       | SFunctionTable |  -> --------------
	//        ----------------       |  FunMember1  |
	//       |   DataMember1  |    |  FunMember2  |
	//       |   DataMember2  |    |  FunMember3  |
	//       |   DataMember3  |    |  FunMember4  |
	//       |   DataMember4  |    |  FunMember5  |
	//       |   DataMember5  |    | ...........  |
	//       |   ...........  |     --------------
	//        --------------     
	//                             
	//
	struct GAMMA_COMMON_API SFunctionTable
	{
		SFunctionTable();
		int32 GetFunctionCount();

		void* m_pFun[MAX_VTABLE_SIZE];
	};

	struct SVirtualObj 
	{ 
		SFunctionTable* m_pTable;
	};

	typedef void( *GetVirtualFunIndexCallback )( void*, void* );
	GAMMA_COMMON_API uint32 GetVirtualFunIndex( uint32 nSize,
		GetVirtualFunIndexCallback funCallback, void* pContext );

	template< typename ClassType, typename RetType, typename... Param >
	static uint32 GetVirtualFunIndex( RetType ( ClassType::*pFun )( Param... ) )
	{
		typedef void ( ClassType::*FunctionType )();
		FunctionType funCall = (FunctionType)pFun;
		struct SFun { static void Call( ClassType* pObj, void* pContext )
		{ FunctionType funCall = *(FunctionType*)pContext; ( pObj->*funCall )();	} };
		GetVirtualFunIndexCallback funCallback = (GetVirtualFunIndexCallback)&SFun::Call;
		return GetVirtualFunIndex( sizeof( ClassType ), funCallback, &funCall );
	}

	template<typename ClassType>
	uint32 GetDestructorFunIndex()
	{
		struct SFun { static void Call( ClassType* pObj, void* ) { pObj->~ClassType(); } };
		GetVirtualFunIndexCallback funCallback = (GetVirtualFunIndexCallback)&SFun::Call;
		return GetVirtualFunIndex( sizeof( ClassType ), funCallback, nullptr );
	}

#endif	//end as3_alchemy_swig Gambey 2012-8-2
}
