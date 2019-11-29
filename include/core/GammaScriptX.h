#ifndef __GAMMA_SCRIPTX_H__
#define __GAMMA_SCRIPTX_H__
#pragma warning(disable: 4624)
#pragma warning(disable: 4510)
#pragma warning(disable: 4610)

#include "core/GammaScriptHelp.h"
#include "core/GammaScriptDef.h"
#include "core/GammaScriptWrap.h"

//=====================================================================
// GammaScriptX.h 
// 定义脚本和C++的注册宏
// 柯达昭
// 2007-10-21
//=====================================================================
#define DEFINE_CLASS_BEGIN( _class, ... ) \
	namespace _class##_namespace { \
	static Gamma::SGlobalExe _class##_register( \
	Gamma::CScriptBase::RegistClass( #_class, \
	Gamma::TInheritInfo<_class, ##__VA_ARGS__>::size, \
	Gamma::TInheritInfo<_class, ##__VA_ARGS__>::Types().data(), \
	Gamma::TInheritInfo<_class, ##__VA_ARGS__>::Values().data() ) ); \
	typedef _class org_class; Gamma::CScriptRegisterList listRegister; \
	typedef Gamma::SFunctionTable* (*GetVirtualTableFun)( void* ); \
	static GetVirtualTableFun funGetVirtualTable = NULL; \
	static Gamma::SFunctionTable* pVirtual = NULL; \
	DEFINE_GETVTABLE_IMP( funGetVirtualTable, pVirtual ); \
	struct _first : public _class, public SGetVTable{};\
	typedef _first 


#define DEFINE_UNDUPLICATION_CLASS_BEGIN( _class, ... ) \
	namespace _class##_namespace { \
	static Gamma::SGlobalExe _class##_register( \
	Gamma::CScriptBase::RegistClass( #_class, \
	Gamma::TInheritInfo<_class, ##__VA_ARGS__>::size, \
	Gamma::TInheritInfo<_class, ##__VA_ARGS__>::Types().data(), \
	Gamma::TInheritInfo<_class, ##__VA_ARGS__>::Values().data() ) ); \
	typedef _class org_class; Gamma::CScriptRegisterList listRegister; \
	typedef Gamma::SFunctionTable* (*GetVirtualTableFun)( void* ); \
	static GetVirtualTableFun funGetVirtualTable = NULL; \
	static Gamma::SFunctionTable* pVirtual = NULL; \
	DEFINE_GETVTABLE_IMP( funGetVirtualTable, pVirtual ); \
	struct _first : public _class, public SGetVTable{};\
	typedef _first 


#define DEFINE_ABSTRACT_CLASS_BEGIN( _class, ... ) \
	namespace _class##_namespace { \
	static Gamma::SGlobalExe _class##_register( \
	Gamma::CScriptBase::RegistClass( #_class, \
	Gamma::TInheritInfo<_class, ##__VA_ARGS__>::size, \
	Gamma::TInheritInfo<_class, ##__VA_ARGS__>::Types().data(), \
	Gamma::TInheritInfo<_class, ##__VA_ARGS__>::Values().data() ) ); \
	typedef _class org_class; Gamma::CScriptRegisterList listRegister; \
	typedef Gamma::SFunctionTable* (*GetVirtualTableFun)( void* ); \
	static GetVirtualTableFun funGetVirtualTable = NULL; \
	static Gamma::SFunctionTable* pVirtual = NULL; \
	DEFINE_GETVTABLE_IMP( funGetVirtualTable, pVirtual ); \
	typedef struct _first {}


#define DEFINE_CLASS_END() _last;\
	struct _class : public _last {}; \
	static Gamma::SGlobalExe _class_fun_register( listRegister.GetFirst()->Register() ); \
	NORMAL_OBJECT_CONSTRUCT( _class, funGetVirtualTable, pVirtual ); \
	static _class##Construct s_Instance; \
	static Gamma::SGlobalExe _class_construct_register( \
	Gamma::CScriptBase::RegistConstruct( &s_Instance, typeid( org_class ).name() ) ); }


#define DEFINE_UNDUPLICATION_CLASS_END() _last;\
	struct _class : public _last {}; \
	static Gamma::SGlobalExe _class_fun_register( listRegister.GetFirst()->Register() ); \
	UNDUPLICATION_OBJECT_CONSTRUCT( _class, funGetVirtualTable, pVirtual ); \
	static _class##Construct s_Instance; \
	static Gamma::SGlobalExe _class_construct_register( \
	Gamma::CScriptBase::RegistConstruct( &s_Instance, typeid( org_class ).name() ) ); }


#define DEFINE_ABSTRACT_CLASS_END() \
	_class; static Gamma::SGlobalExe _class_fun_register( listRegister.GetFirst()->Register() ); \
	static Gamma::SGlobalExe _class_construct_register( \
	Gamma::CScriptBase::RegistConstruct( NULL, typeid( org_class ).name() ) ); }


#define REGIST_CLASSFUNCTION( _function ) \
	_function##_Base_Class; struct _function##_Impl_Class \
	{ \
		static void Register()\
		{ \
			Gamma::CScriptBase::RegistClassFunction( Gamma::MakeClassFunArg( &org_class::_function ), \
			Gamma::CreateFunWrap( &org_class::_function ), typeid( org_class ).name(), #_function );\
		} \
	};  \
	static Gamma::CScriptRegisterNode _function##RegisterNode( listRegister, &_function##_Impl_Class::Register ); \
	typedef _function##_Base_Class 


#define REGIST_CLASSFUNCTION_WITHNAME( _function, _function_name ) \
	_function_name##_Base_Class; struct _function_name##_Impl_Class \
	{ \
		static void Register()\
		{ \
			Gamma::CScriptBase::RegistClassFunction( Gamma::MakeClassFunArg( &org_class::_function ), \
			Gamma::CreateFunWrap( &org_class::_function ), typeid( org_class ).name(), #_function_name );\
		} \
	};  \
	static Gamma::CScriptRegisterNode _function_name##RegisterNode( listRegister, &_function_name##_Impl_Class::Register ); \
	typedef _function_name##_Base_Class 


#define REGIST_CLASSFUNCTION_OVERLOAD( _fun_type, _fun_name_cpp, _fun_name_lua ) \
	_fun_name_lua##_Base_Class; struct _fun_name_lua##_Impl_Class \
	{ \
		static void Register()\
		{ \
			Gamma::CScriptBase::RegistClassFunction( Gamma::MakeClassFunArg( static_cast<_fun_type>(&org_class::_fun_name_cpp) ), \
			Gamma::CreateFunWrap( static_cast<_fun_type>(&org_class::_fun_name_cpp) ), typeid( org_class ).name(), #_fun_name_lua );\
		} \
	};  \
	static Gamma::CScriptRegisterNode _fun_name_lua##RegisterNode( listRegister, &_fun_name_lua##_Impl_Class::Register ); \
	typedef _fun_name_lua##_Base_Class 


#define REGIST_STATICFUNCTION( _function ) \
	_function##_Base_Class; struct _function##_Impl_Class \
	{ \
		static void Register()\
		{ \
			Gamma::CScriptBase::RegistClassStaticFunction( Gamma::MakeFunArg( &org_class::_function ), \
			Gamma::CreateFunWrap( &org_class::_function ), typeid( org_class ).name(), #_function );\
		} \
	};  \
	static Gamma::CScriptRegisterNode _function##RegisterNode( listRegister, &_function##_Impl_Class::Register ); \
	typedef _function##_Base_Class 


#define REGIST_STATICFUNCTION_WITHNAME( _function, _function_name ) \
	_function_name##_Base_Class; struct _function_name##_Impl_Class \
	{ \
		static void Register()\
		{ \
			Gamma::CScriptBase::RegistClassStaticFunction( Gamma::MakeFunArg( &org_class::_function ), \
			Gamma::CreateFunWrap( &org_class::_function ), typeid( org_class ).name(), #_function_name );\
		} \
	};  \
	static Gamma::CScriptRegisterNode _function_name##RegisterNode( listRegister, &_function_name##_Impl_Class::Register ); \
	typedef _function_name##_Base_Class 


#define REGIST_STATICFUNCTION_OVERLOAD( _fun_type, _fun_name_cpp, _fun_name_lua ) \
	_fun_name_lua##_Base_Class; struct _fun_name_lua##_Impl_Class \
	{ \
		static void Register()\
		{ \
			Gamma::CScriptBase::RegistClassStaticFunction( Gamma::MakeFunArg( static_cast<_fun_type>(&org_class::_fun_name_cpp) ), \
			Gamma::CreateFunWrap( static_cast<_fun_type>(&org_class::_fun_name_cpp) ), typeid( org_class ).name(), #_fun_name_lua );\
		} \
	};  \
	static Gamma::CScriptRegisterNode _fun_name_lua##RegisterNode( listRegister, &_fun_name_lua##_Impl_Class::Register ); \
	typedef _fun_name_lua##_Base_Class 


#define REGIST_CLASSMEMBER_GETSET( _member, get, set ) \
	_member##_Base_Class; struct _member##_Impl_Class \
	{ \
		static void Register()\
		{ \
			org_class* c = (org_class*)0x4000000; IFunctionWrap* funGetSet[2];\
			funGetSet[0] = get ? Gamma::CreateMemberGetWrap( c, &c->_member ) : NULL;\
			funGetSet[1] = set ? Gamma::CreateMemberSetWrap( c, &c->_member ) : NULL;\
			Gamma::CScriptBase::RegistClassMember( Gamma::MakeMemberArg( c, &c->_member ),\
			funGetSet, typeid( org_class ).name(), #_member );\
		} \
	};  \
	static Gamma::CScriptRegisterNode _member##_get_RegisterNode( listRegister, &_member##_Impl_Class::Register ); \
	typedef _member##_Base_Class 


#define REGIST_CLASSMEMBER_GETSET_WITHNAME( _member, _new_name, get, set ) \
	_new_name##_Base_Class; struct _new_name##_Impl_Class \
	{ \
		static void Register()\
		{ \
			org_class* c = (org_class*)0x4000000; IFunctionWrap* funGetSet[2];\
			funGetSet[0] = get ? Gamma::CreateMemberGetWrap( c, &c->_member ) : NULL;\
			funGetSet[1] = set ? Gamma::CreateMemberSetWrap( c, &c->_member ) : NULL;\
			Gamma::CScriptBase::RegistClassMember( Gamma::MakeMemberArg( c, &c->_member ),\
			funGetSet, typeid( org_class ).name(), #_new_name );\
		} \
	};  \
	static Gamma::CScriptRegisterNode _new_name##_get_RegisterNode( listRegister, &_new_name##_Impl_Class::Register ); \
	typedef _new_name##_Base_Class 


#define REGIST_CLASSMEMBER_SET( _member ) \
	REGIST_CLASSMEMBER_GETSET( _member, false, true )


#define REGIST_CLASSMEMBER_SET_WITHNAME( _member, _new_name ) \
	REGIST_CLASSMEMBER_GETSET_WITHNAME( _member, _new_name, false, true )


#define REGIST_CLASSMEMBER_GET( _member ) \
	REGIST_CLASSMEMBER_GETSET( _member, true, false )


#define REGIST_CLASSMEMBER_GET_WITHNAME( _member, _new_name ) \
	REGIST_CLASSMEMBER_GETSET_WITHNAME( _member, _new_name, true, false )


#define REGIST_CLASSMEMBER( _member ) \
	REGIST_CLASSMEMBER_GETSET( _member, true, true )


#define REGIST_CLASSMEMBER_WITHNAME( _member, _new_name ) \
	REGIST_CLASSMEMBER_GETSET_WITHNAME( _member, _new_name, true, true )


#define REGIST_DESTRUCTOR() \
	destructor_Base_Class; struct destructor_Impl_Class \
	{ \
		static void Register()\
		{ \
			Gamma::BindDestructorWrap<org_class>( Gamma::CScriptBase::RegistDestructor( typeid( org_class ).name(), \
			Gamma::CreateDestructorWrap<org_class>( Gamma::GetDestructorFunIndex<org_class>() ) ) );\
		} \
	};  \
	static Gamma::CScriptRegisterNode destructor##_register_node( listRegister, &destructor_Impl_Class::Register ); \
	typedef destructor_Base_Class


#define REGIST_GLOBALFUNCTION( _function ) \
	Gamma::SGlobalExe _function##_register( Gamma::CScriptBase::RegistFunction( \
	Gamma::MakeFunArg( &_function ), Gamma::CreateFunWrap( &_function ), typeid( _function ).name(), #_function ) ); 


#define REGIST_GLOBALFUNCTION_WITHNAME( _function, _function_name ) \
	Gamma::SGlobalExe _function_name##_register( Gamma::CScriptBase::RegistFunction( \
	Gamma::MakeFunArg( &_function ), Gamma::CreateFunWrap( &_function ), NULL, #_function_name ) ); 


#define REGIST_GLOBALFUNCTION_OVERLOAD(  _fun_type, _fun_name_cpp, _fun_name_lua ) \
    Gamma::SGlobalExe _fun_name_lua##_register( Gamma::CScriptBase::RegistFunction( \
	Gamma::MakeFunArg( static_cast<_fun_type>(&_fun_name_cpp) ), \
	Gamma::CreateFunWrap( static_cast<_fun_type>(&_fun_name_cpp) ), NULL, #_fun_name_lua ) ); 


#define REGIST_CALLBACKFUNCTION( _function ) \
	_function##_Base_Class; \
	auto _function##_Type = &org_class::_function;\
	template<typename _BaseClass, typename _RetType, typename... Param> \
	struct _function##_Impl_ClassTemplate : public _BaseClass \
	{ \
		_RetType _function( Param ... p ) { throw; }\
		struct __class : public org_class { \
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, true, &__class::_function ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( &__class::_function ); }\
		static STypeInfoArray MakeFunArg()	{ return Gamma::MakeClassFunArg( &__class::_function ); } }; \
		static void Register(){ __class::Bind( Gamma::CScriptBase::RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_function ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	template<typename ClassType, typename RetType, typename... Param> \
	_function##_Impl_ClassTemplate<_function##_Base_Class, RetType, Param...> \
	_function##_Impl_ClassTemplateDecl( RetType ( ClassType::*pFun )( Param... ) );\
	typedef decltype( _function##_Impl_ClassTemplateDecl( _function##_Type ) ) _function##_Impl_Class;\
	static Gamma::CScriptRegisterNode _function##_register_node( listRegister, &_function##_Impl_Class::Register ); \
	static Gamma::SGlobalExe _function##_get_table( funGetVirtualTable = (GetVirtualTableFun)&_function##_Impl_Class::GetVirtualTable );\
	typedef _function##_Impl_Class 


#define REGIST_CALLBACKFUNCTION_WITHNAME( _function, _function_name ) \
	_function##_Base_Class; \
	auto _function_name##_Type = &org_class::_function;\
	template<typename _BaseClass, typename _RetType, typename... Param> \
	struct _function_name##_Impl_ClassTemplate : public _function##_Base_Class \
	{	\
		_RetType _function( Param ... p ) { throw; }\
		struct __class : public org_class { \
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, true, &__class::_function ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( &__class::_function ); }\
		static STypeInfoArray MakeFunArg()	{ return Gamma::MakeClassFunArg( &__class::_function ); } }; \
		static void Register(){ __class::Bind( Gamma::CScriptBase::RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_function_name ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function_name##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	template<typename ClassType, typename RetType, typename... Param> \
	_function_name##_Impl_ClassTemplate<_function##_Base_Class, RetType, Param...> \
	_function_name##_Impl_ClassTemplateDecl( RetType ( ClassType::*pFun )( Param... ) );\
	typedef decltype( _function_name##_Impl_ClassTemplateDecl( _function_name##_Type ) ) _function_name##_Impl_Class;\
	static Gamma::CScriptRegisterNode _function_name##_register_node( listRegister, &_function_name##_Impl_Class::Register ); \
	static Gamma::SGlobalExe _function_name##_get_table( funGetVirtualTable = (GetVirtualTableFun)&_function_name##_Impl_Class::GetVirtualTable );\
	typedef _function_name##_Impl_Class 


#define REGIST_CALLBACKFUNCTION_OVERLOAD( _function, _fun_type, _function_name ) \
	_function##_Base_Class; \
	auto _function_name##_Type = (_fun_type)&org_class::_function;\
	template<typename _BaseClass, typename _RetType, typename... Param> \
	struct _function_name##_Impl_ClassTemplate : public _function##_Base_Class \
	{	\
		_RetType _function( Param ... p ) { throw; }\
		struct __class : public org_class { \
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, true, (_fun_type)&__class::_function ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( (_fun_type)&__class::_function ); }\
		static STypeInfoArray MakeFunArg()	{ return Gamma::MakeClassFunArg( (_fun_type)&__class::_function ); } }; \
		static void Register(){ __class::Bind( Gamma::CScriptBase::RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_function_name ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function_name##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	template<typename ClassType, typename RetType, typename... Param> \
	_function_name##_Impl_ClassTemplate<_function##_Base_Class, RetType, Param...> \
	_function_name##_Impl_ClassTemplateDecl( RetType ( ClassType::*pFun )( Param... ) );\
	typedef decltype( _function_name##_Impl_ClassTemplateDecl( _function_name##_Type ) ) _function_name##_Impl_Class;\
	static Gamma::CScriptRegisterNode _function_name##_register_node( listRegister, &_function_name##_Impl_Class::Register ); \
	static Gamma::SGlobalExe _function_name##_get_table( funGetVirtualTable = (GetVirtualTableFun)&_function_name##_Impl_Class::GetVirtualTable );\
	typedef _function_name##_Impl_Class 


#define REGIST_ENUMTYPE( EnumType ) \
    static Gamma::SGlobalExe EnumType##_register( \
    Gamma::CScriptBase::RegistEnum( typeid( EnumType ).name(), #EnumType, (int32)sizeof(EnumType) ) );


#endif
