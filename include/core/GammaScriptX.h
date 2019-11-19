#ifndef __GAMMA_SCRIPTX_H__
#define __GAMMA_SCRIPTX_H__
#pragma warning(disable: 4624)
#pragma warning(disable: 4510)
#pragma warning(disable: 4610)

#include "common/Help.h"
#include "common/TList.h"
#include "core/GammaScriptRunFun.h"
#include "core/GammaScriptDef.h"

//=====================================================================
// GammaScriptX.h 
// 定义脚本和C++接口的辅助函数和辅助宏
// 使用辅助宏可以快速注册c++类型以及函数
// 柯达昭
// 2007-10-21
//=====================================================================

//==========================================================================
// 注册类的辅助类和宏
//==========================================================================
#define NORMAL_OBJECT_CONSTRUCT( _class, funGetVirtualTable, pVTable ) \
class _class##Construct : public IObjectConstruct \
{\
	virtual void Assign( void* pDest, void* pSrc )	\
	{ *(_class*)pDest = *(_class*)pSrc; }\
	virtual void Construct( void* pObj ) \
	{ \
		_class* pNew = new( pObj )_class;\
		if( !funGetVirtualTable ) return; \
		( (SVirtualObj*)pNew )->m_pTable = pVTable; \
	}\
	virtual void Destruct( void* pObj )		\
	{ static_cast<_class*>( pObj )->~_class(); }\
};

#define UNDUPLICATION_OBJECT_CONSTRUCT( _class, funGetVirtualTable, pVTable ) \
class _class##Construct : public IObjectConstruct \
{\
	virtual void Assign( void* pDest, void* pSrc )	\
	{ throw( "Can not call construct on unduplication object" ); }\
	virtual void Construct( void* pObj ) \
	{ \
		_class* pNew = new( pObj )_class;\
		if( !funGetVirtualTable ) return; \
		( (SVirtualObj*)pNew )->m_pTable = pVTable; \
	}\
	virtual void Destruct( void* pObj )		\
	{ static_cast<_class*>( pObj )->~_class(); }\
};

#define DEFINE_GETVTABLE_IMP( funGetVirtualTable, pVirtual )	\
struct SGetVTable { SGetVTable() \
{ \
	if( !funGetVirtualTable || pVirtual ) return; \
	pVirtual = funGetVirtualTable( this );\
} };

template<class ScriptType>
class TScriptRegisterNode : public Gamma::TList< TScriptRegisterNode<ScriptType> >::CListNode
{
	void(*m_funRegister)(ScriptType&);
	typedef typename Gamma::TList< TScriptRegisterNode<ScriptType> >::CListNode ParentType;
public:
	TScriptRegisterNode( void(*fun)(ScriptType&) ) : m_funRegister( fun ){}
	void Register( ScriptType& Script ) { ParentType::Remove(); m_funRegister( Script ); }
};

#define CScriptRegisterNode TScriptRegisterNode<CScriptBase>
#define CScriptRegisterList Gamma::TList< TScriptRegisterNode<CScriptBase> >

//====================================================================
// 普通类注册
//====================================================================
#define DECLARE_CLASS( scriptClass, _class )    \
	(scriptClass).RegistClass( 1, typeid( _class ).name(), #_class, NULL );

#define REGIST_B_CLASS( scriptClass, _class )    \
    (scriptClass).RegistClass( sizeof(_class), typeid( _class ).name(), #_class, NULL );

#define REGIST_D_CLASS( scriptClass, _class, _base_class )    \
	(scriptClass).RegistClass( sizeof(_class), typeid( _class ).name(), #_class, \
    typeid( _base_class ).name(), Gamma::GetClassOffSet<_base_class, _class>(), NULL ); 

#define REGIST_D_CLASS_2( scriptClass, _class, _base_class1, _base_class2 )    \
	(scriptClass).RegistClass( sizeof(_class), typeid( _class ).name(), #_class, \
    typeid( _base_class1 ).name(), Gamma::GetClassOffSet<_base_class1, _class>(), \
    typeid( _base_class2 ).name(), Gamma::GetClassOffSet<_base_class2, _class>(), NULL ); 

#define REGIST_D_CLASS_3( scriptClass, _class, _base_class1, _base_class2, _base_class3 ) \
	(scriptClass).RegistClass( sizeof(_class), typeid( _class ).name(), #_class, \
    typeid( _base_class1 ).name(), Gamma::GetClassOffSet<_base_class1, _class>(), \
    typeid( _base_class2 ).name(), Gamma::GetClassOffSet<_base_class2, _class>(), \
    typeid( _base_class3 ).name(), Gamma::GetClassOffSet<_base_class3, _class>(), NULL ); 

#define REGIST_D_CLASS_4( scriptClass, _class, _base_class1, _base_class2, _base_class3, _base_class4 )    \
	(scriptClass).RegistClass( sizeof(_class), typeid( _class ).name(), #_class, \
    typeid( _base_class1 ).name(), Gamma::GetClassOffSet<_base_class1, _class>(), \
    typeid( _base_class2 ).name(), Gamma::GetClassOffSet<_base_class2, _class>(), \
    typeid( _base_class3 ).name(), Gamma::GetClassOffSet<_base_class3, _class>(), \
    typeid( _base_class4 ).name(), Gamma::GetClassOffSet<_base_class4, _class>(), NULL ); 

#define REGIST_D_CLASS_5( scriptClass, _class, _base_class1, _base_class2, _base_class3, _base_class4, _base_class5 )    \
	(scriptClass).RegistClass( sizeof(_class), typeid( _class ).name(), #_class, \
    typeid( _base_class1 ).name(), Gamma::GetClassOffSet<_base_class1, _class>(), \
    typeid( _base_class2 ).name(), Gamma::GetClassOffSet<_base_class2, _class>(), \
    typeid( _base_class3 ).name(), Gamma::GetClassOffSet<_base_class3, _class>(), \
    typeid( _base_class4 ).name(), Gamma::GetClassOffSet<_base_class4, _class>(), \
    typeid( _base_class5 ).name(), Gamma::GetClassOffSet<_base_class5, _class>(), NULL ); 

#define REGIST_B_CLASS_WITHNAME( scriptClass, _class, _new_name )    \
    (scriptClass).RegistClass( sizeof(_class), typeid( _class ).name(), #_new_name, NULL );

#define REGIST_D_CLASS_WITHNAME( scriptClass, _class, _new_name, _base_class )    \
	(scriptClass).RegistClass( sizeof(_class), typeid( _class ).name(), #_new_name, \
    typeid( _base_class ).name(), Gamma::GetClassOffSet<_base_class, _class>(), NULL ); 

#define REGIST_D_CLASS_2_WITHNAME( scriptClass, _class, _new_name, _base_class1, _base_class2 )    \
	(scriptClass).RegistClass( sizeof(_class), typeid( _class ).name(), #_new_name, \
    typeid( _base_class1 ).name(), Gamma::GetClassOffSet<_base_class1, _class>(), \
    typeid( _base_class2 ).name(), Gamma::GetClassOffSet<_base_class2, _class>(), NULL ); 

#define REGIST_D_CLASS_3_WITHNAME( scriptClass, _class, _new_name, _base_class1, _base_class2, _base_class3 ) \
	(scriptClass).RegistClass( sizeof(_class), typeid( _class ).name(), #_new_name, \
    typeid( _base_class1 ).name(), Gamma::GetClassOffSet<_base_class1, _class>(), \
    typeid( _base_class2 ).name(), Gamma::GetClassOffSet<_base_class2, _class>(), \
    typeid( _base_class3 ).name(), Gamma::GetClassOffSet<_base_class3, _class>(), NULL ); 

#define REGIST_D_CLASS_4_WITHNAME( scriptClass, _class, _new_name, _base_class1, _base_class2, _base_class3, _base_class4 )    \
	(scriptClass).RegistClass( sizeof(_class), typeid( _class ).name(), #_new_name, \
    typeid( _base_class1 ).name(), Gamma::GetClassOffSet<_base_class1, _class>(), \
    typeid( _base_class2 ).name(), Gamma::GetClassOffSet<_base_class2, _class>(), \
    typeid( _base_class3 ).name(), Gamma::GetClassOffSet<_base_class3, _class>(), \
    typeid( _base_class4 ).name(), Gamma::GetClassOffSet<_base_class4, _class>(), NULL ); 

#define REGIST_D_CLASS_5_WITHNAME( scriptClass, _class, _new_name, _base_class1, _base_class2, _base_class3, _base_class4, _base_class5 )    \
	(scriptClass).RegistClass( sizeof(_class), typeid( _class ).name(), #_new_name, \
    typeid( _base_class1 ).name(), Gamma::GetClassOffSet<_base_class1, _class>(), \
    typeid( _base_class2 ).name(), Gamma::GetClassOffSet<_base_class2, _class>(), \
    typeid( _base_class3 ).name(), Gamma::GetClassOffSet<_base_class3, _class>(), \
    typeid( _base_class4 ).name(), Gamma::GetClassOffSet<_base_class4, _class>(), \
    typeid( _base_class5 ).name(), Gamma::GetClassOffSet<_base_class5, _class>(), NULL ); 


#define REGIST_CLASS_FUNCTION_BEGIN( _class ) \
	{ typedef _class org_class; CScriptRegisterList listRegister; \
	typedef Gamma::SFunctionTable* (*GetVirtualTableFun)( void* ); \
	static GetVirtualTableFun funGetVirtualTable = NULL; \
	static Gamma::SFunctionTable* pVirtual = NULL; \
	DEFINE_GETVTABLE_IMP( funGetVirtualTable, pVirtual ); \
	struct _first : public _class, public SGetVTable{};\
	typedef _first 


#define REGIST_CLASS_FUNCTION_BEGIN_UNDUPLICATION( _class ) \
	{ typedef _class org_class; CScriptRegisterList listRegister; \
	typedef Gamma::SFunctionTable* (*GetVirtualTableFun)( void* ); \
	static GetVirtualTableFun funGetVirtualTable = NULL; \
	static Gamma::SFunctionTable* pVirtual = NULL; \
	DEFINE_GETVTABLE_IMP( funGetVirtualTable, pVirtual ); \
	struct _first : public _class, public SGetVTable{};\
	typedef _first 


#define REGIST_CLASS_FUNCTION_BEGIN_ABSTRACT( _class ) \
	{ typedef _class org_class; CScriptRegisterList listRegister; \
	typedef Gamma::SFunctionTable* (*GetVirtualTableFun)( void* ); \
	static GetVirtualTableFun funGetVirtualTable = NULL; \
	static Gamma::SFunctionTable* pVirtual = NULL; \
	DEFINE_GETVTABLE_IMP( funGetVirtualTable, pVirtual ); \
	typedef struct _first {}


#define REGIST_CLASS_FUNCTION_END( scriptClass ) _last;\
	struct _class : public _last {}; \
	while( listRegister.GetFirst() )listRegister.GetFirst()->Register( (scriptClass) ); \
	NORMAL_OBJECT_CONSTRUCT( _class, funGetVirtualTable, pVirtual ); \
	static _class##Construct s_Instance; \
	(scriptClass).RegistConstruct( &s_Instance, typeid( org_class ).name() ); }


#define REGIST_CLASS_FUNCTION_END_UNDUPLICATION( scriptClass ) _last;\
	struct _class : public _last {}; \
	while( listRegister.GetFirst() )listRegister.GetFirst()->Register( (scriptClass) ); \
	UNDUPLICATION_OBJECT_CONSTRUCT( _class, funGetVirtualTable, pVirtual ); \
	static _class##Construct s_Instance; \
	(scriptClass).RegistConstruct( &s_Instance, typeid( org_class ).name() ); }


#define REGIST_CLASS_FUNCTION_END_ABSTRACT( scriptClass ) \
	_class; while( listRegister.GetFirst() )listRegister.GetFirst()->Register( (scriptClass) ); \
	(scriptClass).RegistConstruct( NULL, typeid( org_class ).name() ); }


#define REGIST_CLASSFUNCTION( _function ) \
	_function##_Base_Class; struct _function##_Impl_Class \
	{ \
		static void Register( CScriptBase& Script )\
		{ \
			Script.RegistClassFunction( Gamma::MakeClassFunArg( &org_class::_function ), \
			Gamma::CreateFunWrap( &org_class::_function ), typeid( org_class ).name(), #_function );\
		} \
	};  \
	CScriptRegisterNode _function##RegisterNode( &_function##_Impl_Class::Register ); \
	listRegister.PushBack( _function##RegisterNode ); \
	typedef _function##_Base_Class 


#define REGIST_CLASSFUNCTION_WITHNAME( _function, _function_name ) \
	_function_name##_Base_Class; struct _function_name##_Impl_Class \
	{ \
		static void Register( CScriptBase& Script )\
		{ \
			Script.RegistClassFunction( Gamma::MakeClassFunArg( &org_class::_function ), \
			Gamma::CreateFunWrap( &org_class::_function ), typeid( org_class ).name(), #_function_name );\
		} \
	};  \
	CScriptRegisterNode _function_name##RegisterNode( &_function_name##_Impl_Class::Register ); \
	listRegister.PushBack( _function_name##RegisterNode ); \
	typedef _function_name##_Base_Class 


#define REGIST_CLASSFUNCTION_OVERLOAD( _fun_type, _fun_name_cpp, _fun_name_lua ) \
	_fun_name_lua##_Base_Class; struct _fun_name_lua##_Impl_Class \
	{ \
		static void Register( CScriptBase& Script )\
		{ \
			Script.RegistClassFunction( Gamma::MakeClassFunArg( static_cast<_fun_type>(&org_class::_fun_name_cpp) ), \
			Gamma::CreateFunWrap( static_cast<_fun_type>(&org_class::_fun_name_cpp) ), typeid( org_class ).name(), #_fun_name_lua );\
		} \
	};  \
	CScriptRegisterNode _fun_name_lua##RegisterNode( &_fun_name_lua##_Impl_Class::Register ); \
	listRegister.PushBack( _fun_name_lua##RegisterNode ); \
	typedef _fun_name_lua##_Base_Class 


#define REGIST_STATICFUNCTION( _function ) \
	_function##_Base_Class; struct _function##_Impl_Class \
	{ \
		static void Register( CScriptBase& Script )\
		{ \
			Script.RegistClassStaticFunction( Gamma::MakeFunArg( &org_class::_function ), \
			Gamma::CreateFunWrap( &org_class::_function ), typeid( org_class ).name(), #_function );\
		} \
	};  \
	CScriptRegisterNode _function##RegisterNode( &_function##_Impl_Class::Register ); \
	listRegister.PushBack( _function##RegisterNode ); \
	typedef _function##_Base_Class 


#define REGIST_STATICFUNCTION_WITHNAME( _function, _function_name ) \
	_function_name##_Base_Class; struct _function_name##_Impl_Class \
	{ \
		static void Register( CScriptBase& Script )\
		{ \
			Script.RegistClassStaticFunction( Gamma::MakeFunArg( &org_class::_function ), \
			Gamma::CreateFunWrap( &org_class::_function ), typeid( org_class ).name(), #_function_name );\
		} \
	};  \
	CScriptRegisterNode _function_name##RegisterNode( &_function_name##_Impl_Class::Register ); \
	listRegister.PushBack( _function_name##RegisterNode ); \
	typedef _function_name##_Base_Class 


#define REGIST_STATICFUNCTION_OVERLOAD( _fun_type, _fun_name_cpp, _fun_name_lua ) \
	_fun_name_lua##_Base_Class; struct _fun_name_lua##_Impl_Class \
	{ \
		static void Register( CScriptBase& Script )\
		{ \
			Script.RegistClassStaticFunction( Gamma::MakeFunArg( static_cast<_fun_type>(&org_class::_fun_name_cpp) ), \
			Gamma::CreateFunWrap( static_cast<_fun_type>(&org_class::_fun_name_cpp) ), typeid( org_class ).name(), #_fun_name_lua );\
		} \
	};  \
	CScriptRegisterNode _fun_name_lua##RegisterNode( &_fun_name_lua##_Impl_Class::Register ); \
	listRegister.PushBack( _fun_name_lua##RegisterNode ); \
	typedef _fun_name_lua##_Base_Class 


#define REGIST_CLASSMEMBER_GETSET( _member, get, set ) \
	_member##_Base_Class; struct _member##_Impl_Class \
	{ \
		static void Register( CScriptBase& Script )\
		{ \
			org_class* c = (org_class*)0x4000000; IFunctionWrap* funGetSet[2];\
			funGetSet[0] = get ? Gamma::CreateMemberGetWrap( c, &c->_member ) : NULL;\
			funGetSet[1] = set ? Gamma::CreateMemberSetWrap( c, &c->_member ) : NULL;\
			Script.RegistClassMember( Gamma::MakeMemberArg( c, &c->_member ),\
			funGetSet, typeid( org_class ).name(), #_member );\
		} \
	};  \
	CScriptRegisterNode _member##_get_RegisterNode( &_member##_Impl_Class::Register ); \
	listRegister.PushBack( _member##_get_RegisterNode ); \
	typedef _member##_Base_Class 


#define REGIST_CLASSMEMBER_GETSET_WITHNAME( _member, _new_name, get, set ) \
	_new_name##_Base_Class; struct _new_name##_Impl_Class \
	{ \
		static void Register( CScriptBase& Script )\
		{ \
			org_class* c = (org_class*)0x4000000; IFunctionWrap* funGetSet[2];\
			funGetSet[0] = get ? Gamma::CreateMemberGetWrap( c, &c->_member ) : NULL;\
			funGetSet[1] = set ? Gamma::CreateMemberSetWrap( c, &c->_member ) : NULL;\
			Script.RegistClassMember( Gamma::MakeMemberArg( c, &c->_member ),\
			funGetSet, typeid( org_class ).name(), #_new_name );\
		} \
	};  \
	CScriptRegisterNode _new_name##_get_RegisterNode( &_new_name##_Impl_Class::Register ); \
	listRegister.PushBack( _new_name##_get_RegisterNode ); \
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
		static void Register( CScriptBase& Script )\
		{ \
			Gamma::BindDestructorWrap<org_class>( Script.RegistDestructor( typeid( org_class ).name(), \
			Gamma::CreateDestructorWrap<org_class>( Gamma::GetDestructorFunIndex<org_class>() ) ) );\
		} \
	};  \
	CScriptRegisterNode destructor##_RegisterNode( &destructor_Impl_Class::Register ); \
	listRegister.PushBack( destructor##_RegisterNode ); \
	typedef destructor_Base_Class


#define REGIST_GLOBALFUNCTION( scriptClass, _function ) \
	(scriptClass).RegistFunction( Gamma::MakeFunArg( &_function ), Gamma::CreateFunWrap( &_function ), typeid( _function ).name(), #_function ); 


#define REGIST_GLOBALFUNCTION_WITHNAME( scriptClass, _function, _function_name ) \
	(scriptClass).RegistFunction( Gamma::MakeFunArg( &_function ), Gamma::CreateFunWrap( &_function ), NULL, #_function_name ); 


#define REGIST_GLOBALFUNCTION_OVERLOAD( scriptClass,  _fun_type, _fun_name_cpp, _fun_name_lua ) \
    (scriptClass).RegistFunction( Gamma::MakeFunArg( static_cast<_fun_type>(&_fun_name_cpp) ), \
	Gamma::CreateFunWrap( static_cast<_fun_type>(&_fun_name_cpp) ), NULL, #_fun_name_lua ); 	


#define REGIST_CALLBACKFUNCTION( _function ) \
	_function##_Base_Class; struct _function##_Impl_Class : public _function##_Base_Class \
	{	\
		struct __class : public org_class { \
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, false, &__class::_function ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( &__class::_function ); }\
		static STypeInfoArray MakeFunArg()	{ return Gamma::MakeClassFunArg( &__class::_function ); } }; \
		static void Register( CScriptBase& Script ){ __class::Bind( Script.RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_function ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	CScriptRegisterNode _function##_RegisterNode( &_function##_Impl_Class::Register ); \
	listRegister.PushBack( _function##_RegisterNode ); \
	funGetVirtualTable = (GetVirtualTableFun)&_function##_Impl_Class::GetVirtualTable;\
	typedef _function##_Impl_Class 


#define REGIST_CALLBACKFUNCTION_WITHNAME( _function, _function_name ) \
	_function##_Base_Class; struct _function_name##_Impl_Class : public _function##_Base_Class \
	{	\
		struct __class : public org_class { typedef _function##_Impl_Class T; \
		static void* GetVirtualTable( SGetVTable* p ){ return ((SVirtualObj*)(T*)( p ))->m_pTable; }\
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, false, &__class::_function ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( &__class::_function ); }\
		static STypeInfoArray MakeFunArg()	{ return Gamma::MakeClassFunArg( &__class::_function ); } }; \
		static void Register( CScriptBase& Script ){ __class::Bind( Script.RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_function_name ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	CScriptRegisterNode _function_name##_RegisterNode( &_function_name##_Impl_Class::Register ); \
	listRegister.PushBack( _function##_RegisterNode ); \
	funGetVirtualTable = (GetVirtualTableFun)&_function##_Impl_Class::GetVirtualTable;\
	typedef _function_name##_Impl_Class 


#define REGIST_CALLBACKFUNCTION_OVERLOAD( _function, _fun_type, _fun_name_cpp, _fun_name_lua ) \
	_function##_Base_Class; struct _fun_name_lua##_Impl_Class : public _function##_Base_Class \
	{	\
		struct __class : public org_class { typedef _function##_Impl_Class T; \
		static void* GetVirtualTable( SGetVTable* p ){ return ((SVirtualObj*)(T*)( p ))->m_pTable; }\
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, false, static_cast<_fun_type>(&__class::_fun_name_cpp) ) ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( static_cast<_fun_type>(&__class::_fun_name_cpp) ); }\
		static STypeInfoArray MakeFunArg() { return Gamma::MakeClassFunArg( static_cast<_fun_type>(&__class::_fun_name_cpp) ); } }; \
		static void Register( CScriptBase& Script ){ __class::Bind( Script.RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_fun_name_lua ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	CScriptRegisterNode _fun_name_lua##_RegisterNode( &_fun_name_lua##_Impl_Class::Register ); \
	listRegister.PushBack( _fun_name_lua##_RegisterNode ); \
	funGetVirtualTable = (GetVirtualTableFun)&_fun_name_lua##_Impl_Class::GetVirtualTable;\
	typedef _fun_name_lua##_Impl_Class 	


#define REGIST_PUREVIRTUALFUNCTION( _function ) \
	_function##_Base_Class; struct _function##_Impl_Class : public _function##_Base_Class \
	{	\
		DEFINE_PUREVIRTUAL_IMPLEMENT( _function, _function##_Base_Class );\
		struct __class : public org_class { \
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, true, &__class::_function ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( &__class::_function ); }\
		static STypeInfoArray MakeFunArg()	{ return Gamma::MakeClassFunArg( &__class::_function ); } }; \
		static void Register( CScriptBase& Script ){ __class::Bind( Script.RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_function ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	CScriptRegisterNode _function##_RegisterNode( &_function##_Impl_Class::Register ); \
	listRegister.PushBack( _function##_RegisterNode ); \
	funGetVirtualTable = (GetVirtualTableFun)&_function##_Impl_Class::GetVirtualTable;\
	typedef _function##_Impl_Class 


#define REGIST_PUREVIRTUALFUNCTION_WITHNAME( _function, _function_name ) \
	_function##_Base_Class; struct _function_name##_Impl_Class : public _function##_Base_Class \
	{	\
		DEFINE_PUREVIRTUAL_IMPLEMENT( _function, _function##_Base_Class );\
		struct __class : public org_class { typedef _function##_Impl_Class T; \
		static void* GetVirtualTable( SGetVTable* p ){ return ((SVirtualObj*)(T*)( p ))->m_pTable; }\
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, true, &__class::_function ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( &__class::_function ); }\
		static STypeInfoArray MakeFunArg()	{ return Gamma::MakeClassFunArg( &__class::_function ); } }; \
		static void Register( CScriptBase& Script ){ __class::Bind( Script.RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_function_name ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	CScriptRegisterNode _function_name##_RegisterNode( &_function_name##_Impl_Class::Register ); \
	listRegister.PushBack( _function##_RegisterNode ); \
	funGetVirtualTable = (GetVirtualTableFun)&_function##_Impl_Class::GetVirtualTable;\
	typedef _function_name##_Impl_Class 


#define REGIST_PUREVIRTUALFUNCTION_OVERLOAD( _function, _fun_type, _fun_name_cpp, _fun_name_lua ) \
	_function##_Base_Class; struct _fun_name_lua##_Impl_Class : public _function##_Base_Class \
	{	\
		DEFINE_PUREVIRTUAL_IMPLEMENT( _function, _function##_Base_Class );\
		struct __class : public org_class { typedef _function##_Impl_Class T; \
		static void* GetVirtualTable( SGetVTable* p ){ return ((SVirtualObj*)(T*)( p ))->m_pTable; }\
		static void Bind( ICallBackWrap& c ) { Gamma::BIND_CALLBACK( c, true, static_cast<_fun_type>(&__class::_fun_name_cpp) ) ); }\
		static IFunctionWrap* CreateFunWrap(){ return Gamma::CreateFunWrap( static_cast<_fun_type>(&__class::_fun_name_cpp) ); }\
		static STypeInfoArray MakeFunArg() { return Gamma::MakeClassFunArg( static_cast<_fun_type>(&__class::_fun_name_cpp) ); } }; \
		static void Register( CScriptBase& Script ){ __class::Bind( Script.RegistClassCallback( \
		__class::MakeFunArg(), __class::CreateFunWrap(), typeid( org_class ).name(), #_fun_name_lua ) ); } \
		static Gamma::SFunctionTable* GetVirtualTable( SGetVTable* p )\
		{ return ((SVirtualObj*)(_function##_Impl_Class*)( p ) )->m_pTable; } \
	}; \
	CScriptRegisterNode _fun_name_lua##_RegisterNode( &_fun_name_lua##_Impl_Class::Register ); \
	listRegister.PushBack( _fun_name_lua##_RegisterNode ); \
	funGetVirtualTable = (GetVirtualTableFun)&_fun_name_lua##_Impl_Class::GetVirtualTable;\
	typedef _fun_name_lua##_Impl_Class 


#define REGIST_CONSTANT( scriptClass, ValueName ) \
    (scriptClass).RegistConstant( NULL, #ValueName, (int32)( ValueName ) )

#define REGIST_CONSTANT_DOUBLE(  scriptClass, ValueName  )\
	(scriptClass).RegistConstant( NULL, #ValueName, (double)( ValueName ) )

#define REGIST_CONSTANT_WITHNAME( scriptClass, ValueName, Value ) \
	(scriptClass).RegistConstant( NULL, #ValueName, Value )

#define REGIST_ENUMTYPE( scriptClass, EnumType ) \
    (scriptClass).RegistEnum( typeid( EnumType ).name(), #EnumType, (int32)sizeof(EnumType) );

#define REGIST_ENUMERATION( scriptClass, EnumType, ValueName ) \
	{ (scriptClass).RegistConstant( #EnumType, #ValueName, (int32)( ValueName ) ); }

#define REGIST_ENUMERATION_WITHNAME( scriptClass, EnumType, ValueName, Value ) \
	{ (scriptClass).RegistConstant( #EnumType, #ValueName, (int32)( Value ) ); }

#endif
