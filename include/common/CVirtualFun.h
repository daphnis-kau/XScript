//=========================================================================================
// CVirtualFun.h 
// 柯达昭
// 2008-02-27
//=========================================================================================
#ifndef __XS_VIRTUALFUN_H__
#define __XS_VIRTUALFUN_H__

#include "common/Help.h"

namespace XS
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
	struct SFunctionTable
	{
		SFunctionTable();
		int32 GetFunctionCount();

		void* m_pFun[MAX_VTABLE_SIZE];
	};

	struct SVirtualObj 
	{ 
		SFunctionTable* m_pTable;
	};

	// 获取虚表索引导出函数
	typedef void( *VirtualFunCallback )( void*, void* );
	uint32 FindVirtualFunction( uint32 nSize,
		VirtualFunCallback funCallback, void* pContext );

	// 获取普通函数虚表索引
	template< typename ClassType, typename RetType, typename... Param >
	static uint32 GetVirtualFunIndex( RetType ( ClassType::*pFun )( Param... ) )
	{
		typedef void ( ClassType::*FunctionType )();
		FunctionType funCall = (FunctionType)pFun;
		struct SFun { static void Call( ClassType* pObj, void* pContext )
		{ FunctionType funCall = *(FunctionType*)pContext; ( pObj->*funCall )(); } };
		VirtualFunCallback funCallback = (VirtualFunCallback)&SFun::Call;
		return FindVirtualFunction( sizeof( ClassType ), funCallback, &funCall );
	}

	// 获取析构函数虚表索引
	template<typename ClassType>
	uint32 GetDestructorFunIndex()
	{
		struct SFun : public ClassType 
		{ virtual ~SFun(){} static void Call( SFun* pObj, void* ) { pObj->~SFun(); } };
		if( sizeof( SFun ) != sizeof( ClassType ) )
			throw "the class have not virtual table";
		VirtualFunCallback funCallback = (VirtualFunCallback)&SFun::Call;
		return FindVirtualFunction( sizeof( SFun ), funCallback, nullptr );
	}

#endif
}

#endif