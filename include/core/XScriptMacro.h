/**@file  		XScriptMacro.h
* @brief		Help macro of XScript
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
*/

#ifndef __XS_SCRIPT_MACRO_H__
#define __XS_SCRIPT_MACRO_H__

#include "core/XScriptDef.h"
#include "core/XScriptWrap.h"

#pragma warning(disable: 4624)
#pragma warning(disable: 4510)
#pragma warning(disable: 4610)

#define DEFINE_CLASS_BEGIN_IMPLEMENT( _get_vtable_class, _type, _class, ... ) \
	namespace _class##_namespace { \
	static XS::SGlobalExe _class##_register( \
		XS::CScriptBase::RegisterClass( #_class, \
		XS::TInheritInfo<_class, ##__VA_ARGS__>::size, \
		XS::TInheritInfo<_class, ##__VA_ARGS__>::Types().data(), \
		XS::TInheritInfo<_class, ##__VA_ARGS__>::Values().data() ) ); \
	static XS::CScriptRegisterList listRegister; \
	typedef _class org_class; \
	typedef _class org_##_type; \
	typedef _get_vtable_class CGetClassVTable;\
	typedef struct _first : public CGetClassVTable{}


#define DEFINE_CLASS_END_IMPLEMENT( _get_vtable_class, _type, ... ) _last;\
	struct _class : public _last {}; \
	static XS::SGlobalExe _class_fun_register( listRegister.GetFirst()->Register() ); \
	typedef TConstruct<_get_vtable_class, _class, _type, ##__VA_ARGS__> ConstructType; \
	static XS::SGlobalExe _class_construct_register( \
	XS::CScriptBase::RegisterConstruct( ConstructType::Inst(), typeid( org_##_type ).name() ) ); }


#define REGIST_CLASSFUNCTION_IMPLEMENT( _function_type, _function, _function_name ) \
	_function_name##_Base_Class; \
	namespace _function_name##_namespace \
	{ \
		template<typename _RetType, typename... Param> \
		struct TFunctionRegister \
		{ \
			typedef decltype ( (_function_type)nullptr ) _fun_type;\
			static _RetType Call( org_class* pThis, Param ... p ) \
			{ \
				_fun_type funMember = (_fun_type)&org_class::_function; \
				return (pThis->*funMember)(p...); \
			}\
			static void Register()\
			{ \
				XS::CreateClassFunWrap( &TFunctionRegister::Call, #_function_name );\
			} \
		};  \
		\
		template<typename ClassType, typename RetType, typename... Param> \
		TFunctionRegister<RetType, Param...> Decl( RetType ( ClassType::*pFun )( Param... ) );\
		template<typename ClassType, typename RetType, typename... Param> \
		TFunctionRegister<RetType, Param...> Decl( RetType ( ClassType::*pFun )( Param... ) const );\
		typedef decltype( Decl( (_function_type)nullptr ) ) RegisterImpl;\
		\
		static XS::CScriptRegisterNode RegisterNode( listRegister, &RegisterImpl::Register ); \
	}\
	typedef _function_name##_Base_Class 


#define REGIST_CALLBACKFUNCTION_IMPLEMENT( _is_pure, _function_type, _function, _function_name ) \
	_function##_Base_Class; \
	namespace _function_name##_namespace \
	{ \
		typedef decltype( (_function_type)nullptr ) FunctionType;\
		typedef _function##_Base_Class BaseClass;\
		\
		template<bool bConst, bool bPure, typename _BaseClass, typename... Param> \
		struct TFunctionOverride : public _BaseClass {}; \
		template<typename _BaseClass, typename _RetType, typename... Param> \
		struct TFunctionOverride<false, true, _BaseClass, _RetType, Param...> \
		: public _BaseClass { _RetType _function( Param ... p ) { throw; } }; \
		template<typename _BaseClass, typename _RetType, typename... Param> \
		struct TFunctionOverride<true, true, _BaseClass, _RetType, Param...> \
		: public _BaseClass { _RetType _function( Param ... p ) const { throw; } }; \
		\
		template<typename ClassType, typename RetType, typename... Param> \
		TFunctionOverride<false, _is_pure, BaseClass, RetType, Param...> \
			Decl( RetType ( ClassType::*pFun )( Param... ) );\
		template<typename ClassType, typename RetType, typename... Param> \
		TFunctionOverride<true, _is_pure, BaseClass, RetType, Param...> \
			Decl( RetType ( ClassType::*pFun )( Param... ) const );\
		typedef decltype (Decl( (_function_type)nullptr )) ImplementClass;\
		\
		typedef XS::TCallBackBinder<ImplementClass> CallbackBinder;\
		struct RegisterImpl : public org_class { \
		static FunctionType GetFun() { return (FunctionType)&RegisterImpl::_function; } \
		static void Register(){ CallbackBinder::Bind( false, #_function_name, GetFun() ); } }; \
		static XS::CScriptRegisterNode RegisterNode( listRegister, &RegisterImpl::Register ); \
		static XS::SGlobalExe Execute( CallbackBinder::InsallGetVirtualTable() );\
	}\
	typedef _function_name##_namespace::ImplementClass 


#define REGIST_CLASSMEMBER_GETSET_IMPLEMENT( _member, _new_name, _enableGetter, _enableSetter ) \
	_new_name##_Base_Class; \
	namespace _new_name##_namespace \
	{ \
		static void Register()\
		{ \
			org_class* c = (org_class*)0x4000000; \
			IFunctionWrap* funGetSet[2];\
			funGetSet[0] = _enableGetter ? XS::CreateMemberGetWrap( &c->_member ) : NULL;\
			funGetSet[1] = _enableSetter ? XS::CreateMemberSetWrap( &c->_member ) : NULL;\
			ptrdiff_t offset = ((ptrdiff_t)&c->_member) - (ptrdiff_t)c;\
			XS::CScriptBase::RegisterClassMember( funGetSet, offset,\
				XS::MakeMemberArg( c, &c->_member ), #_new_name );\
		} \
		static XS::CScriptRegisterNode RegisterNode( listRegister, &Register ); \
	};  \
	typedef _new_name##_Base_Class 


#define REGIST_STATICFUNCTION_IMPLEMENT( _function_type, _function, _function_name ) \
	_function_name##_Base_Class; \
	namespace _function_name##_namespace \
	{ \
		static void Register()\
		{ \
			typedef decltype ( (_function_type)nullptr ) _fun_type;\
			XS::CreateGlobalFunWrap( (_fun_type)(&org_class::_function), \
			typeid( org_class ).name(), #_function_name );\
		} \
		static XS::CScriptRegisterNode RegisterNode( listRegister, &Register ); \
	};  \
	typedef _function_name##_Base_Class 


#define REGIST_DESTRUCTOR_IMPLEMENT() \
	destructor_Base_Class; \
	namespace destructor_namespace \
	{ \
		static void Register() { TDestructorWrap<org_class>::Bind(); } \
		static XS::CScriptRegisterNode RegisterNode( listRegister, &Register ); \
	};  \
	typedef destructor_Base_Class


#define REGIST_GLOBALFUNCTION_IMPLEMENT( _fun_type, _function, _fun_name_lua ) \
    XS::SGlobalExe _fun_name_lua##_register( ( XS::CreateGlobalFunWrap( \
		(_fun_type)(&_function), NULL, #_fun_name_lua ), true ) ); 


#define REGIST_ENUMTYPE_IMPLEMENT( EnumType ) \
    static XS::SGlobalExe EnumType##_register( \
    XS::CScriptBase::RegisterEnum( typeid( EnumType ).name(), #EnumType, (int32)sizeof(EnumType) ) );


#endif
