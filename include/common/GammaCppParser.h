//=============================================================
// GammaCppParser.h
// 生成参数类型字符串
// 柯达昭
// 2012-08-09
//=====================================================================
#pragma once

#include "common/GammaCpp.h"
#include <typeinfo>

namespace Gamma
{
	template<typename T>
	struct STypeID { enum{ eTypeID = eDT_class }; };

	///< 特化c++内置基本类型
	template<> struct STypeID<void>			{ enum{ eTypeID = eDT_void		}; };
	template<> struct STypeID<char>			{ enum{ eTypeID = eDT_char	    }; };
	template<> struct STypeID<int8>			{ enum{ eTypeID = eDT_int8	    }; };
	template<> struct STypeID<int16>		{ enum{ eTypeID = eDT_int16		}; };
	template<> struct STypeID<int32>		{ enum{ eTypeID = eDT_int32		}; };
	template<> struct STypeID<long>			{ enum{ eTypeID = eDT_long		}; };
	template<> struct STypeID<int64>		{ enum{ eTypeID = eDT_int64		}; };
	template<> struct STypeID<uint8>		{ enum{ eTypeID = eDT_uint8		}; };
	template<> struct STypeID<uint16>		{ enum{ eTypeID = eDT_uint16	}; };
	template<> struct STypeID<uint32>		{ enum{ eTypeID = eDT_uint32	}; };
	template<> struct STypeID<uint64>		{ enum{ eTypeID = eDT_uint64	}; };
	template<> struct STypeID<ulong>		{ enum{ eTypeID = eDT_ulong		}; };
	template<> struct STypeID<wchar_t>		{ enum{ eTypeID = eDT_wchar		}; };
	template<> struct STypeID<bool>			{ enum{ eTypeID = eDT_bool		}; };
	template<> struct STypeID<float>		{ enum{ eTypeID = eDT_float		}; };
	template<> struct STypeID<double>		{ enum{ eTypeID = eDT_double	}; };

	/** 
	使用模板的方式分析出模板参数的类型
	默认为class类型
	其他c++内置类型通过模板特化解释
	*/
	template<typename T>
	struct TTypeInfo
	{
		typedef T Type;

		enum
		{
			m_eType = STypeID<T>::eTypeID,
			m_eTypeEx0 = eDTE_Value,
			m_eTypeEx1 = 0,
			m_eTypeEx2 = 0,
			m_eTypeEx3 = 0,
			m_eTypeEx4 = 0,
			m_eTypeEx5 = 0,
			m_eTotalType = ( m_eType << 24 )|( m_eTypeEx0 << 0 )|( m_eTypeEx1 << 4 )|( m_eTypeEx2 << 8 )|( m_eTypeEx3 << 12 )|( m_eTypeEx4 << 16 )|( m_eTypeEx5 << 20 ),
		};
	};

	///< 特化const类型
	template<typename T>
	struct TTypeInfo<const T>
	{
		typedef typename TTypeInfo<T>::Type Type;

		enum
		{
			m_eType = TTypeInfo<T>::m_eType,
			m_eTypeEx0 = TTypeInfo<T>::m_eTypeEx0,
			m_eTypeEx1 = TTypeInfo<T>::m_eTypeEx1 ? TTypeInfo<T>::m_eTypeEx1 : eDTE_Const,
			m_eTypeEx2 = TTypeInfo<T>::m_eTypeEx2 ? TTypeInfo<T>::m_eTypeEx2 : ( TTypeInfo<T>::m_eTypeEx1 ? eDTE_Const : 0 ),
			m_eTypeEx3 = TTypeInfo<T>::m_eTypeEx3 ? TTypeInfo<T>::m_eTypeEx3 : ( TTypeInfo<T>::m_eTypeEx2 ? eDTE_Const : 0 ),
			m_eTypeEx4 = TTypeInfo<T>::m_eTypeEx4 ? TTypeInfo<T>::m_eTypeEx4 : ( TTypeInfo<T>::m_eTypeEx3 ? eDTE_Const : 0 ),
			m_eTypeEx5 = TTypeInfo<T>::m_eTypeEx5 ? TTypeInfo<T>::m_eTypeEx5 : ( TTypeInfo<T>::m_eTypeEx4 ? eDTE_Const : 0 ),
			m_eTotalType = ( m_eType << 24 )|( m_eTypeEx0 << 0 )|( m_eTypeEx1 << 4 )|( m_eTypeEx2 << 8 )|( m_eTypeEx3 << 12 )|( m_eTypeEx4 << 16 )|( m_eTypeEx5 << 20 ),
		};
	};

	///< 特化指针类型
	template<typename T>
	struct TTypeInfo<T*>
	{
		typedef typename TTypeInfo<T>::Type Type;

		enum
		{
			m_eType = TTypeInfo<T>::m_eType,
			m_eTypeEx0 = TTypeInfo<T>::m_eTypeEx0,
			m_eTypeEx1 = TTypeInfo<T>::m_eTypeEx1 ? TTypeInfo<T>::m_eTypeEx1 : eDTE_Pointer,
			m_eTypeEx2 = TTypeInfo<T>::m_eTypeEx2 ? TTypeInfo<T>::m_eTypeEx2 : ( TTypeInfo<T>::m_eTypeEx1 ? eDTE_Pointer : 0 ),
			m_eTypeEx3 = TTypeInfo<T>::m_eTypeEx3 ? TTypeInfo<T>::m_eTypeEx3 : ( TTypeInfo<T>::m_eTypeEx2 ? eDTE_Pointer : 0 ),
			m_eTypeEx4 = TTypeInfo<T>::m_eTypeEx4 ? TTypeInfo<T>::m_eTypeEx4 : ( TTypeInfo<T>::m_eTypeEx3 ? eDTE_Pointer : 0 ),
			m_eTypeEx5 = TTypeInfo<T>::m_eTypeEx5 ? TTypeInfo<T>::m_eTypeEx5 : ( TTypeInfo<T>::m_eTypeEx4 ? eDTE_Pointer : 0 ),
			m_eTotalType = ( m_eType << 24 )|( m_eTypeEx0 << 0 )|( m_eTypeEx1 << 4 )|( m_eTypeEx2 << 8 )|( m_eTypeEx3 << 12 )|( m_eTypeEx4 << 16 )|( m_eTypeEx5 << 20 ),
		};
	};

	///< 特化常量指针的类型
	template<typename T>
	struct TTypeInfo<T *const>
	{
		typedef typename TTypeInfo<T>::Type Type;

		enum
		{
			m_eType = TTypeInfo<T>::m_eType,
			m_eTypeEx0 = TTypeInfo<T>::m_eTypeEx0,
			m_eTypeEx1 = TTypeInfo<T>::m_eTypeEx1 ? TTypeInfo<T>::m_eTypeEx1 : eDTE_ConstPointer,
			m_eTypeEx2 = TTypeInfo<T>::m_eTypeEx2 ? TTypeInfo<T>::m_eTypeEx2 : ( TTypeInfo<T>::m_eTypeEx1 ? eDTE_ConstPointer : 0 ),
			m_eTypeEx3 = TTypeInfo<T>::m_eTypeEx3 ? TTypeInfo<T>::m_eTypeEx3 : ( TTypeInfo<T>::m_eTypeEx2 ? eDTE_ConstPointer : 0 ),
			m_eTypeEx4 = TTypeInfo<T>::m_eTypeEx4 ? TTypeInfo<T>::m_eTypeEx4 : ( TTypeInfo<T>::m_eTypeEx3 ? eDTE_ConstPointer : 0 ),
			m_eTypeEx5 = TTypeInfo<T>::m_eTypeEx5 ? TTypeInfo<T>::m_eTypeEx5 : ( TTypeInfo<T>::m_eTypeEx4 ? eDTE_ConstPointer : 0 ),
			m_eTotalType = ( m_eType << 24 )|( m_eTypeEx0 << 0 )|( m_eTypeEx1 << 4 )|( m_eTypeEx2 << 8 )|( m_eTypeEx3 << 12 )|( m_eTypeEx4 << 16 )|( m_eTypeEx5 << 20 ),
		};
	};

	///< 特化引用的类型
	template<typename T>
	struct TTypeInfo<T&>
	{
		typedef typename TTypeInfo<T>::Type Type;

		enum
		{
			m_eType = TTypeInfo<T>::m_eType,
			m_eTypeEx0 = TTypeInfo<T>::m_eTypeEx0,
			m_eTypeEx1 = TTypeInfo<T>::m_eTypeEx1 ? TTypeInfo<T>::m_eTypeEx1 : eDTE_Reference,
			m_eTypeEx2 = TTypeInfo<T>::m_eTypeEx2 ? TTypeInfo<T>::m_eTypeEx2 : ( TTypeInfo<T>::m_eTypeEx1 ? eDTE_Reference : 0 ),
			m_eTypeEx3 = TTypeInfo<T>::m_eTypeEx3 ? TTypeInfo<T>::m_eTypeEx3 : ( TTypeInfo<T>::m_eTypeEx2 ? eDTE_Reference : 0 ),
			m_eTypeEx4 = TTypeInfo<T>::m_eTypeEx4 ? TTypeInfo<T>::m_eTypeEx4 : ( TTypeInfo<T>::m_eTypeEx3 ? eDTE_Reference : 0 ),
			m_eTypeEx5 = TTypeInfo<T>::m_eTypeEx5 ? TTypeInfo<T>::m_eTypeEx5 : ( TTypeInfo<T>::m_eTypeEx4 ? eDTE_Reference : 0 ),
			m_eTotalType = ( m_eType << 24 )|( m_eTypeEx0 << 0 )|( m_eTypeEx1 << 4 )|( m_eTypeEx2 << 8 )|( m_eTypeEx3 << 12 )|( m_eTypeEx4 << 16 )|( m_eTypeEx5 << 20 ),
		};
	};

#define SIMPLE_TYPE_INFO( T ) \
	template<> \
	struct TTypeInfo<const T&> \
	{ \
		typedef TTypeInfo<T>::Type Type; \
		enum \
		{ \
			m_eType = STypeID<T>::eTypeID, \
			m_eTypeEx0 = eDTE_Value, \
			m_eTypeEx1 = 0, \
			m_eTypeEx2 = 0, \
			m_eTypeEx3 = 0, \
			m_eTypeEx4 = 0, \
			m_eTypeEx5 = 0, \
			m_eTotalType = ( m_eType << 24 )|( m_eTypeEx0 << 0 )|( m_eTypeEx1 << 4 )|( m_eTypeEx2 << 8 )|( m_eTypeEx3 << 12 )|( m_eTypeEx4 << 16 )|( m_eTypeEx5 << 20 ), \
		}; \
	};
	
	SIMPLE_TYPE_INFO( int8 )
	SIMPLE_TYPE_INFO( int16 )
	SIMPLE_TYPE_INFO( int32 )
	SIMPLE_TYPE_INFO( int64 )
	SIMPLE_TYPE_INFO( uint8 )
	SIMPLE_TYPE_INFO( uint16 )
	SIMPLE_TYPE_INFO( uint32 )
	SIMPLE_TYPE_INFO( uint64 )
	SIMPLE_TYPE_INFO( float )
	SIMPLE_TYPE_INFO( double )
	#undef SIMPLE_TYPE_INFO

	///< 特化字符串的类型
	template<>
	struct TTypeInfo<const wchar_t*>
	{
		typedef const wchar_t* Type;

		enum
		{
			m_eType = eDT_const_wchar_t_str,
			m_eTypeEx0 = eDTE_Value,
			m_eTypeEx1 = 0,
			m_eTypeEx2 = 0,
			m_eTypeEx3 = 0,
			m_eTypeEx4 = 0,
			m_eTypeEx5 = 0,
			m_eTotalType = ( m_eType << 24 )|( m_eTypeEx0 << 0 ),
		};
	};

	///< 特化字符串的类型
	template<>
	struct TTypeInfo<const char*>
	{
		typedef const char* Type;

		enum
		{
			m_eType = eDT_const_char_str,
			m_eTypeEx0 = eDTE_Value,
			m_eTypeEx1 = 0,
			m_eTypeEx2 = 0,
			m_eTypeEx3 = 0,
			m_eTypeEx4 = 0,
			m_eTypeEx5 = 0,
			m_eTotalType = ( m_eType << 24 )|( m_eTypeEx0 << 0 ),
		};
	};

	///< 特化void类型
	template<>
	struct TTypeInfo<void>
	{
		typedef const void Type;

		enum
		{
			m_eType = eDT_void,
			m_eTypeEx0 = eDTE_Value,
			m_eTypeEx1 = 0,
			m_eTypeEx2 = 0,
			m_eTypeEx3 = 0,
			m_eTypeEx4 = 0,
			m_eTypeEx5 = 0,
			m_eTotalType = ( m_eType << 24 )|( m_eTypeEx0 << 0 ),
		};
	};

	template<typename T>
	void GetTypeInfo( STypeInfo& Info )
	{
		Info.m_nType = TTypeInfo<T>::m_eTotalType;
		Info.m_szTypeName = typeid(typename TTypeInfo<T>::Type).name();
	}

	template<typename T>
	STypeInfo GetTypeInfo()
	{
		STypeInfo Info;
		Info.m_nType = TTypeInfo<T>::m_eTotalType;
		Info.m_szTypeName = typeid(typename TTypeInfo<T>::Type).name();
		return Info;
	}
}
