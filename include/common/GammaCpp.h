//=============================================================
// GammaCpp.h 
// 定义C++语言相关的接口
// 柯达昭
// 2007-10-19
//=====================================================================
#ifndef ___GAMMA_CPP_H___
#define ___GAMMA_CPP_H___
#include "common/Help.h"
#include "common/CommonType.h"

namespace Gamma
{
	enum EDataType
	{
		//对c++数据类型进行定义
		eDT_void				= 0,
		eDT_char				= 1,
		eDT_int8				= 2,
		eDT_int16				= 3,
		eDT_int32				= 4,
		eDT_int64				= 5,
		eDT_long				= 6,
		eDT_uint8				= 7,
		eDT_uint16				= 8,
		eDT_uint32				= 9,
		eDT_uint64				= 10,
		eDT_ulong				= 11,
		eDT_wchar				= 12,
		eDT_bool				= 13,
		eDT_float				= 14,
		eDT_double				= 15,
		eDT_class				= 16,
		eDT_const_char_str		= 17,
		eDT_const_wchar_t_str	= 18
	};    

	enum EDataTypeEx
	{
		eDTE_Invalid			= 0,
		eDTE_Value				= 1,		
		eDTE_Const				= 2,
		eDTE_Pointer			= 3,
		eDTE_ConstPointer		= 4,
		eDTE_Reference			= 5,
		eDTE_Count          ,
	};

    struct STypeInfo
	{
        uint32		m_nType;
		const char*	m_szTypeName;
	};

	struct STypeInfoArray
	{
		uint32 nSize;
		STypeInfo aryInfo[22];
	};

	struct SFunction { uintptr_t funPoint; uintptr_t offset; };
	template< class _FunTy >    void* GetFunAdress( _FunTy fun )	{ return *((void**)&fun); }
	template< class _FunTy >    SFunction GetFunction( _FunTy fun )	{ return *((SFunction*)&fun); }
	template< class _FunTy >    _FunTy MakeFun( void* fun )			{ return *((_FunTy*)&fun); }
	template< class _FunTy >    _FunTy MakeFun( SFunction fun )		{ return *((_FunTy*)&fun); }

	#define GetClassMemberOffset( Class, Member )					( ( (char*)&( (Class*)0x4000000 )->Member ) - ( (char*)0x4000000 ) )
	template<class B, class D>	ptrdiff_t GetClassOffSet()			{ return ((char*)static_cast<B*>((D*)0x4000000)) - (char*)0x4000000; }
}
#endif
