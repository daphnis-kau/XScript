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
		eDT_const_char_str		= 16,
		eDT_const_wchar_t_str	= 17,
		eDT_enum				= 18,
		eDT_class				= 19,
		eDT_count				= 20,
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
		uint32		m_nSize;
		uint32		m_nType;
		const char*	m_szTypeName;
	};
}
#endif
