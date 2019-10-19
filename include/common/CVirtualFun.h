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

	struct SDestructorFinder
	{ 
		ptrdiff_t nIndex;
	};

	GAMMA_COMMON_API uint32 GetVirtualFunIndex( SFunction fun );
	GAMMA_COMMON_API void* CreateDestructorFinder( SDestructorFinder* pBuf, size_t nSize );

	template<typename _Ty>
	uint32 GetDestructorFunIndex()
	{
		tbyte aryBuffer[TMax2< sizeof(_Ty), sizeof(SDestructorFinder) >::eValue];
		SDestructorFinder* pFinder = (SDestructorFinder*)aryBuffer;
		_Ty* pObject = (_Ty*)CreateDestructorFinder( pFinder, sizeof(_Ty) );
		pObject->~_Ty();
		void** pValue = (void**)aryBuffer;
		for( size_t i = 0, j = 0; i < sizeof(_Ty); i += sizeof(void*), j++ )
		{
			if( pValue[j] < (void*)MAX_VTABLE_SIZE )
				return (uint32)(ptrdiff_t)pValue[j];
		}

		return INVALID_32BITID;
	}
#endif	//end as3_alchemy_swig Gambey 2012-8-2
}
