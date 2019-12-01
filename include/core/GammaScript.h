﻿#ifndef __GAMMA_SCRIPT_H__
#define __GAMMA_SCRIPT_H__
#pragma warning(disable: 4624)
#pragma warning(disable: 4510)
#pragma warning(disable: 4610)

#include "core/GammaScriptX.h"

//普通类的注册
#define DEFINE_CLASS_BEGIN( _class, ... ) \
	DEFINE_CLASS_BEGIN_IMPLEMENT( TGetVTable<_class>, true, _class, ##__VA_ARGS__)
#define DEFINE_CLASS_END() \
	DEFINE_CLASS_END_IMPLEMENT( TGetVTable<_class>, true )

//不可复制类的注册
#define DEFINE_UNDUPLICATABLE_CLASS_BEGIN( _class, ... ) \
	DEFINE_CLASS_BEGIN_IMPLEMENT( TGetVTable<_class>, false, _class, ##__VA_ARGS__)
#define DEFINE_UNDUPLICATABLE_CLASS_END() \
	DEFINE_CLASS_END_IMPLEMENT( TGetVTable<_class>, false )

//不可实例化类的注册
#define DEFINE_ABSTRACT_CLASS_BEGIN( _class, ... ) \
	DEFINE_CLASS_BEGIN_IMPLEMENT( TGetVTable<void>, nullptr, _class, ##__VA_ARGS__)
#define DEFINE_ABSTRACT_CLASS_END() \
	DEFINE_CLASS_END_IMPLEMENT( TGetVTable<void>, nullptr )

#define REGIST_DESTRUCTOR()	REGIST_DESTRUCTOR_IMPLEMENT()

#define REGIST_CLASSFUNCTION( _function ) \
	REGIST_CLASSFUNCTION_IMPLEMENT( decltype( &org_class::##_function ), _function, _function )
#define REGIST_CLASSFUNCTION_WITHNAME( _function, _fun_name ) \
	REGIST_CLASSFUNCTION_IMPLEMENT( decltype( &org_class::##_function ), _function, _fun_name )
#define REGIST_CLASSFUNCTION_OVERLOAD( _function_type, _function, _fun_name ) \
	REGIST_CLASSFUNCTION_IMPLEMENT( _function_type, _function, _fun_name )

#define REGIST_STATICFUNCTION( _function ) \
	REGIST_STATICFUNCTION_IMPLEMENT( decltype( &org_class::##_function ), _function, _function )
#define REGIST_STATICFUNCTION_WITHNAME( _function, _fun_name ) \
	REGIST_STATICFUNCTION_IMPLEMENT( decltype( &org_class::##_function ), _function, _fun_name )
#define REGIST_STATICFUNCTION_OVERLOAD( _function_type, _function, _fun_name ) \
	REGIST_STATICFUNCTION_IMPLEMENT( _function_type, _function, _fun_name )

#define REGIST_CLASSMEMBER_GETSET( _member, get, set ) \
	REGIST_CLASSMEMBER_GETSET_IMPLEMENT( _member, _member, get, set )
#define REGIST_CLASSMEMBER_GETSET_WITHNAME( _member, _new_name, get, set ) \
	REGIST_CLASSMEMBER_GETSET_IMPLEMENT( _member, _new_name, get, set )
#define REGIST_CLASSMEMBER_SET( _member ) \
	REGIST_CLASSMEMBER_GETSET_IMPLEMENT( _member, _member, false, true )
#define REGIST_CLASSMEMBER_SET_WITHNAME( _member, _new_name ) \
	REGIST_CLASSMEMBER_GETSET_IMPLEMENT( _member, _new_name, false, true )
#define REGIST_CLASSMEMBER_GET( _member ) \
	REGIST_CLASSMEMBER_GETSET_IMPLEMENT( _member, _member, true, false )
#define REGIST_CLASSMEMBER_GET_WITHNAME( _member, _new_name ) \
	REGIST_CLASSMEMBER_GETSET_IMPLEMENT( _member, _new_name, true, false )
#define REGIST_CLASSMEMBER( _member ) \
	REGIST_CLASSMEMBER_GETSET_IMPLEMENT( _member, _member, true, true )
#define REGIST_CLASSMEMBER_WITHNAME( _member, _new_name ) \
	REGIST_CLASSMEMBER_GETSET_IMPLEMENT( _member, _new_name, true, true )

#define REGIST_GLOBALFUNCTION( _function ) \
	REGIST_GLOBALFUNCTION_IMPLEMENT( decltype( &_function ), _function, _function )
#define REGIST_GLOBALFUNCTION_WITHNAME( _function, _fun_name ) \
	REGIST_GLOBALFUNCTION_IMPLEMENT( decltype( &_function ), _function, _fun_name )
#define REGIST_GLOBALFUNCTION_OVERLOAD( _function_type, _function, _fun_name ) \
	REGIST_GLOBALFUNCTION_IMPLEMENT( _function_type, _function, _fun_name )

#define REGIST_CALLBACKFUNCTION( _function ) \
	REGIST_CALLBACKFUNCTION_IMPLEMENT( decltype( &org_class::##_function ), _function, _function )
#define REGIST_CALLBACKFUNCTION_WITHNAME( _function, _fun_name ) \
	REGIST_CALLBACKFUNCTION_IMPLEMENT( decltype( &org_class::##_function ), _function, _fun_name )
#define REGIST_CALLBACKFUNCTION_OVERLOAD( _function, _fun_type, _fun_name ) \
	REGIST_CALLBACKFUNCTION_IMPLEMENT( _function, _fun_type, _fun_name )

#define REGIST_ENUMTYPE( EnumType ) REGIST_ENUMTYPE_IMPLEMENT( EnumType )

#endif
