#ifndef __GAMMA_SCRIPTX_H__
#define __GAMMA_SCRIPTX_H__
#pragma warning(disable: 4624)
#pragma warning(disable: 4510)
#pragma warning(disable: 4610)

#include "core/GammaScriptDef.h"
#include "core/GammaScriptWrap.h"

//=====================================================================
// GammaScriptX.h 
// 定义脚本和C++的注册宏
// 柯达昭
// 2007-10-21
//=====================================================================
#define DEFINE_CLASS_BEGIN_IMPLEMENT( _get_vtable_class, _type, _class, ... ) \
	namespace _class##_namespace { \
	static Gamma::SGlobalExe _class##_register( \
	Gamma::CScriptBase::RegistClass( #_class, \
	Gamma::TInheritInfo<_class, ##__VA_ARGS__>::size, \
	Gamma::TInheritInfo<_class, ##__VA_ARGS__>::Types().data(), \
	Gamma::TInheritInfo<_class, ##__VA_ARGS__>::Values().data() ) ); \
	static Gamma::CScriptRegisterList listRegister; \
	typedef _class org_class; \
	typedef _class org_##_type; \
	typedef _get_vtable_class CGetClassVTable;\
	typedef struct _first : public CGetClassVTable{}


#define DEFINE_CLASS_END_IMPLEMENT( _get_vtable_class, _type ) _last;\
	struct _class : public _last {}; \
	static Gamma::SGlobalExe _class_fun_register( listRegister.GetFirst()->Register() ); \
	typedef TConstruct<_get_vtable_class, _class, _type> ConstructType; \
	static Gamma::SGlobalExe _class_construct_register( \
	Gamma::CScriptBase::RegistConstruct( ConstructType::Inst(), typeid( org_##_type ).name() ) ); }


#define REGIST_CLASSFUNCTION_IMPLEMENT( _function_type, _function, _function_name ) \
	_function_name##_Base_Class; struct _function_name##_Impl_Class \
	{ \
		static void Register()\
		{ \
			typedef decltype ( (_function_type)nullptr ) _fun_type;\
			Gamma::CreateFunWrap( (_fun_type)(&org_class::_function), #_function_name );\
		} \
	};  \
	static Gamma::CScriptRegisterNode _function_name##RegisterNode( listRegister, &_function_name##_Impl_Class::Register ); \
	typedef _function_name##_Base_Class 


#define REGIST_CALLBACKFUNCTION_IMPLEMENT( _is_pure, _function_type, _function, _function_name ) \
	_function##_Base_Class; \
	auto _function_name##_Pointer = (_function_type)&org_class::_function;\
	typedef decltype( _function_name##_Pointer ) _function_name##_Type;\
	template<bool bConst, bool bPure, typename _BaseClass, typename _RetType, typename... Param> \
	struct _function_name##_Impl_ClassTemplate;\
	template<typename _BaseClass, typename _RetType, typename... Param> \
	struct _function_name##_Impl_ClassTemplate<false, false, _BaseClass, _RetType, Param...> \
		: public _BaseClass \
	{	\
		_RetType _function( Param ... p ) { return org_class::_function(p...); }\
		typedef _function_name##_Impl_ClassTemplate\
		<false, false, _BaseClass, _RetType, Param...> MyType; \
		struct __class : public org_class, public Gamma::TCallBackBinder<MyType> { \
		static _function_name##_Type GetFun() { return (_function_name##_Type)&__class::_function; }\
		static void Register(){ Bind( false, #_function_name, GetFun() ); } }; \
	}; \
	template<typename _BaseClass, typename _RetType, typename... Param> \
	struct _function_name##_Impl_ClassTemplate<true, false, _BaseClass, _RetType, Param...> \
		: public _BaseClass \
	{	\
		_RetType _function( Param ... p ) const { return org_class::_function(p...); }\
		typedef _function_name##_Impl_ClassTemplate\
		<true, false, _BaseClass, _RetType, Param...> MyType; \
		struct __class : public org_class, public Gamma::TCallBackBinder<MyType> { \
		static _function_name##_Type GetFun() { return (_function_name##_Type)&__class::_function; }\
		static void Register(){ Bind( false, #_function_name, GetFun() ); } }; \
	}; \
	template<typename _BaseClass, typename _RetType, typename... Param> \
	struct _function_name##_Impl_ClassTemplate<false, true, _BaseClass, _RetType, Param...> \
		: public _BaseClass \
	{	\
		_RetType _function( Param ... p ) { throw; }\
		typedef _function_name##_Impl_ClassTemplate\
		<false, true, _BaseClass, _RetType, Param...> MyType; \
		struct __class : public org_class, public Gamma::TCallBackBinder<MyType> { \
		static _function_name##_Type GetFun() { return (_function_name##_Type)&__class::_function; }\
		static void Register(){ Bind( true, #_function_name, GetFun() ); } }; \
	}; \
	template<typename _BaseClass, typename _RetType, typename... Param> \
	struct _function_name##_Impl_ClassTemplate<true, true, _BaseClass, _RetType, Param...> \
		: public _BaseClass \
	{	\
		_RetType _function( Param ... p ) const { throw; }\
		typedef _function_name##_Impl_ClassTemplate\
		<true, true, _BaseClass, _RetType, Param...> MyType; \
		struct __class : public org_class, public Gamma::TCallBackBinder<MyType> { \
		static _function_name##_Type GetFun() { return (_function_name##_Type)&__class::_function; }\
		static void Register(){ Bind( true, #_function_name, GetFun() ); } }; \
	}; \
	template<typename ClassType, typename RetType, typename... Param> \
	_function_name##_Impl_ClassTemplate<false, _is_pure, _function##_Base_Class, RetType, Param...> \
	_function_name##_Impl_ClassTemplateDecl( RetType ( ClassType::*pFun )( Param... ) );\
	template<typename ClassType, typename RetType, typename... Param> \
	_function_name##_Impl_ClassTemplate<true, _is_pure, _function##_Base_Class, RetType, Param...> \
	_function_name##_Impl_ClassTemplateDecl( RetType ( ClassType::*pFun )( Param... ) const );\
	typedef decltype( _function_name##_Impl_ClassTemplateDecl( _function_name##_Pointer ) ) \
		_function_name##_Impl_Class;\
	static Gamma::CScriptRegisterNode _function_name##_register_node( \
		listRegister, &_function_name##_Impl_Class::__class::Register ); \
	static Gamma::SGlobalExe _function_name##_get_table( \
		_function_name##_Impl_Class::__class::InsallGetVirtualTable() );\
	typedef _function_name##_Impl_Class 


#define REGIST_CLASSMEMBER_GETSET_IMPLEMENT( _member, _new_name, get, set ) \
	_new_name##_Base_Class; struct _new_name##_Impl_Class \
	{ \
		static void Register()\
		{ \
			org_class* c = (org_class*)0x4000000; IFunctionWrap* funGetSet[2];\
			funGetSet[0] = get ? Gamma::CreateMemberGetWrap( c, &c->_member ) : NULL;\
			funGetSet[1] = set ? Gamma::CreateMemberSetWrap( c, &c->_member ) : NULL;\
			ptrdiff_t offset = ((ptrdiff_t)&c->_member) - (ptrdiff_t)c;\
			Gamma::CScriptBase::RegistClassMember( funGetSet, offset,\
				Gamma::MakeMemberArg( c, &c->_member ), typeid( org_class ).name(), #_new_name );\
		} \
	};  \
	static Gamma::CScriptRegisterNode _new_name##_get_RegisterNode( listRegister, &_new_name##_Impl_Class::Register ); \
	typedef _new_name##_Base_Class 


#define REGIST_STATICFUNCTION_IMPLEMENT( _function_type, _function, _function_name ) \
	_function_name##_Base_Class; struct _function_name##_Impl_Class \
	{ \
		static void Register()\
		{ \
			typedef decltype ( (_function_type)nullptr ) _fun_type;\
			Gamma::CreateFunWrap( (_fun_type)(&org_class::_function), \
			typeid( org_class ).name(), #_function_name );\
		} \
	};  \
	static Gamma::CScriptRegisterNode _function_name##RegisterNode( listRegister, &_function_name##_Impl_Class::Register ); \
	typedef _function_name##_Base_Class 


#define REGIST_DESTRUCTOR_IMPLEMENT() \
	destructor_Base_Class; struct destructor_Impl_Class \
	{ static void Register() { TDestructorWrap<org_class>::Bind(); } };  \
	static Gamma::CScriptRegisterNode destructor##_register_node( listRegister, &destructor_Impl_Class::Register ); \
	typedef destructor_Base_Class


#define REGIST_GLOBALFUNCTION_IMPLEMENT( _fun_type, _function, _fun_name_lua ) \
    Gamma::SGlobalExe _fun_name_lua##_register( ( Gamma::CreateFunWrap( \
		(_fun_type)(&_function), NULL, #_fun_name_lua ), true ) ); 


#define REGIST_ENUMTYPE_IMPLEMENT( EnumType ) \
    static Gamma::SGlobalExe EnumType##_register( \
    Gamma::CScriptBase::RegistEnum( typeid( EnumType ).name(), #EnumType, (int32)sizeof(EnumType) ) );


#endif
