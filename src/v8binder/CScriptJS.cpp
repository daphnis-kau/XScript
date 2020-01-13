#include "common/CodeCvs.h"
#include "common/TStrStream.h"
#include "core/CClassRegistInfo.h"
#include "CScriptJS.h"
#include "CTypeJS.h"
#include "CDebugJS.h"
#include "CCallJS.h"
#include "V8Context.h"
#include <functional>

#ifdef _WIN32
#pragma warning(disable: 4345)
#pragma comment( linker, "/defaultlib:v8.dll.lib" )
#pragma comment( linker, "/defaultlib:v8_libbase.dll.lib" )
#pragma comment( linker, "/defaultlib:v8_libplatform.dll.lib" )
#elif( defined _ANDROID )
#include <typeinfo>
#include <alloca.h>
#endif

#ifdef _WIN32
	#ifdef _M_X64
		#ifdef _DEBUG
			#include "v8/snapshot_blob/win64.debug.h"
		#else
			#include "v8/snapshot_blob/win64.release.h"
		#endif
	#else
		#ifdef _DEBUG
			#include "v8/snapshot_blob/win32.debug.h"
		#else
			#include "v8/snapshot_blob/win32.release.h"
		#endif
	#endif
#else
	#include "v8/snapshot_blob/linux64.release.h"
#endif

#include "v8/v8-natives_blob.h"
#include "v8/libplatform/libplatform.h"

namespace Gamma
{

	//====================================================================================
    // CScriptJS
	//====================================================================================
    CScriptJS::CScriptJS( uint16 nDebugPort, bool bV8Protocal)
		: m_pFreeObjectInfo( NULL )
		, m_pV8Context( new SV8Context( this ) )
	{
		struct SV8Init
		{
			v8::StartupData					m_natives;
			v8::StartupData					m_snapshot;
			v8::Platform*					m_platform;
			v8::Isolate::CreateParams		m_create_params;
			SV8Init()
			{
				m_natives.data = (const char*)native_blob;
				m_natives.raw_size = sizeof(native_blob);
				m_snapshot.data = (const char*)snapshot_blob;
				m_snapshot.raw_size = sizeof(snapshot_blob);
				v8::V8::SetNativesDataBlob(&m_natives);
				v8::V8::SetSnapshotDataBlob(&m_snapshot);
				m_platform = v8::platform::CreateDefaultPlatform();
				v8::V8::InitializePlatform( m_platform );
				v8::V8::Initialize();
				m_create_params.array_buffer_allocator =
					v8::ArrayBuffer::Allocator::NewDefaultAllocator();
			}

			~SV8Init()
			{
				v8::V8::Dispose();
				v8::V8::ShutdownPlatform();
				delete m_create_params.array_buffer_allocator;
				delete m_platform;
			}
		};
		static SV8Init s_Init;
		m_pV8Context->m_platform = s_Init.m_platform;

		// Create a new Isolate and make it the current one.
		v8::Isolate* pIsolate = v8::Isolate::New(s_Init.m_create_params);
		m_pV8Context->m_pIsolate = pIsolate;
		m_pV8Context->m_pIsolate->Enter();
		
		// Create a stack-allocated handle scope.
		v8::HandleScope handle_scope(m_pV8Context->m_pIsolate);

		// Create a new context.
		v8::Local<v8::Context> context = v8::Context::New(m_pV8Context->m_pIsolate);
		m_pV8Context->m_Context.Reset(pIsolate, context);

		v8::Context::Scope context_scope(context);
		v8::Local<v8::Object> globalObj = context->Global();
		globalObj->Set(v8::String::NewFromUtf8(pIsolate, "window"), globalObj);

		v8::Local<v8::External> ScriptContext = v8::External::New( pIsolate, this );

		LocalValue console = globalObj->Get(v8::String::NewFromUtf8(pIsolate, "console")); 

		v8::Local<v8::Object> consoleObj = console->ToObject(pIsolate);
		v8::Local<v8::Function> funLog = 
			v8::Function::New(pIsolate, &SV8Context::Log, ScriptContext );
		consoleObj->Set(context, v8::String::NewFromUtf8(pIsolate, "log"), funLog);

		v8::Local<v8::Function> funDebug = 
			v8::Function::New(pIsolate, &SV8Context::Break, ScriptContext);
		globalObj->Set( context, v8::String::NewFromUtf8(pIsolate, "gdb" ), funDebug );

		m_pDebugger = new CDebugJS( this, nDebugPort, bV8Protocal );

		m_pV8Context->m_CppField.Reset(pIsolate, 
			v8::String::NewFromUtf8(pIsolate, "__cpp_obj_info__" ) );
		m_pV8Context->m_Prototype.Reset(pIsolate, 
			v8::String::NewFromUtf8(pIsolate, "prototype" ) );
		m_pV8Context->m_Deconstruction.Reset(pIsolate, 
			v8::String::NewFromUtf8(pIsolate, "Deconstruction" ) );
		m_pV8Context->m___proto__.Reset(pIsolate, 
			v8::String::NewFromUtf8(pIsolate, "__proto__" ) );

		RunString(	
			"var Gamma = {};\n"
			"(function()\n"
			"{\n"
			"	var s_aryGammaPackage = [];\n"
			"	Gamma.class = function(Derive, szGlobalName, Base)\n"
			"	{\n"
			"		if (Base)\n"
			"		{\n"
			"			var Super = function() {};\n"
			"			Super.prototype = Base.prototype;\n"
			"			Derive.prototype = new Super();\n"

			"			for (var Property in Base)\n"
			"			{\n"
			"				if (!Base.hasOwnProperty(Property))\n"
			"					continue;\n"
			"				var Descriptor = Object.getOwnPropertyDescriptor(Base, Property);\n"
			"				if (!Descriptor.get && !Descriptor.set)\n"
			"					Derive[Property] = Base[Property];\n"
			"				else\n"
			"					Object.defineProperty(Derive, Property, Descriptor);\n"
			"			}\n"
			"			Derive.prototype.__super = Base;\n"
			"		}\n"
			"		Derive.prototype.__class__ = Derive;\n"

			"		var s_aryAlloc = [];\n"
			"		var s_nAllocCount = 0;\n"
			"		Derive.Alloc = function()\n"
			"		{\n"
			"			if (s_nAllocCount)\n"
			"				return s_aryAlloc[--s_nAllocCount];\n"
			"			return new Derive;\n"
			"		}\n"

			"		Derive.Free = function(Obj)\n"
			"		{\n"
			"			s_aryAlloc[s_nAllocCount++] = Obj;\n"
			"		}\n"

			"		if (!szGlobalName)\n"
			"			return Derive;\n"
			"		var CurPackage = window, aryPath = szGlobalName.split('.');\n"
			"		for (var i = 0; i < aryPath.length - 1; i++)\n"
			"		{\n"
			"			if (!CurPackage[aryPath[i]])\n"
			"				CurPackage[aryPath[i]] = {}\n"
			"			CurPackage = CurPackage[aryPath[i]];\n"
			"		}\n"
			"		CurPackage[aryPath[i]] = Derive;\n"
			"		Derive.prototype.__class__name__ = aryPath[i];\n"
			"		Derive.prototype.__class__path__ = szGlobalName;\n"
			"		return Derive;\n"
			"	};\n"

			"	Gamma.getset = function(Class, szName, funGet, funSet)\n"
			"	{\n"
			"		if (funGet && funSet)\n"
			"			Object.defineProperty(Class, szName, { get:funGet, set : funSet, enumerable : false, configurable : true });\n"
			"		else if (funGet)\n"
			"			Object.defineProperty(Class, szName, { get:funGet, enumerable : false, configurable : true });\n"
			"		else if (funSet)\n"
			"			Object.defineProperty(Class, szName, { set:funSet, enumerable : false, configurable : true });\n"
			"	};\n"

			"	Gamma.getClass = function(szName)\n"
			"	{\n"
			"		var CurPackage = window, aryPath = szName.split('.');\n"
			"		for (var i = 0; i < aryPath.length - 1; i++)\n"
			"		{\n"
			"			if (!CurPackage[aryPath[i]])\n"
			"				return null;\n"
			"			CurPackage = CurPackage[aryPath[i]];\n"
			"		}\n"
			"		return CurPackage[aryPath[i]];\n"
			"	}\n"

			"	Gamma.ClassCast = function( obj, __class, ... arguments )\n"
			"	{\n"
			"		obj.__proto__ = __class.prototype;\n"
			"		__class.apply( obj, arguments );\n"
			"		return obj;\n"
			"	}\n"

			"	return this;\n"
			"})()"
		);

		LocalValue nsGamma = globalObj->Get(v8::String::NewFromUtf8(pIsolate, "Gamma"));
		v8::Local<v8::Object> nsGammaObject = nsGamma->ToObject(pIsolate);
		LocalValue GammaClass = nsGammaObject->Get(v8::String::NewFromUtf8(pIsolate, "class"));
		m_pV8Context->m_GammaClass.Reset(pIsolate, v8::Local<v8::Function>::Cast(GammaClass));
		m_pV8Context->m_GammaNameSpace.Reset(pIsolate, nsGammaObject);

		BuildRegisterInfo();
    }

    CScriptJS::~CScriptJS(void)
	{
		SAFE_DELETE( m_pDebugger );
		m_pV8Context->ClearCppString((void*)(uintptr_t)(-1));
		m_pV8Context->m_Context.Reset();
		m_pV8Context->m_pIsolate->Exit();
		m_pV8Context->m_pIsolate->Dispose();
		m_pV8Context->m_pIsolate = NULL;

		while( m_mapClassInfo.GetFirst() )
			delete m_mapClassInfo.GetFirst();
		while( m_mapCallBase.GetFirst() )
			delete m_mapCallBase.GetFirst();
		while( m_mapObjInfo.GetFirst() )
			delete m_mapObjInfo.GetFirst();
		delete m_pV8Context;
	}

	SCallInfo* CScriptJS::GetCallInfo( const CByScriptBase* pCallBase )
	{
		v8::Isolate* isolate = GetV8Context().m_pIsolate;
		SCallInfo* pCallInfo = m_mapCallBase.Find( (const void*)pCallBase );
		if( pCallInfo )
			return pCallInfo;
		pCallInfo = new SCallInfo;
		pCallInfo->m_pCallBase = pCallBase;
		pCallInfo->m_pScript = this;
		pCallInfo->m_strName.Reset( isolate, v8::String::NewFromUtf8
			( isolate, pCallBase->GetFunctionName().c_str() ) );
		return pCallInfo;
	}

	SObjInfo* CScriptJS::AllocObjectInfo()
	{
		if( !m_pFreeObjectInfo )
		{
			SObjInfo* aryObjectInfo = new SObjInfo[512];
			m_pFreeObjectInfo = aryObjectInfo;
			m_pFreeObjectInfo->m_bFirstAddress = true;
			for( size_t i = 0; i < 511; i++, aryObjectInfo++ )
			{
				aryObjectInfo->m_pObject = aryObjectInfo + 1;
				aryObjectInfo->m_bFirstAddress = false;
			}
			aryObjectInfo->m_pObject = NULL;
			aryObjectInfo->m_bFirstAddress = false;
		}
		SObjInfo* pObjectInfo = m_pFreeObjectInfo;
		m_pFreeObjectInfo = (SObjInfo*)m_pFreeObjectInfo->m_pObject;
		return pObjectInfo;
	}

	void CScriptJS::FreeObjectInfo( SObjInfo* pObjectInfo )
	{
		pObjectInfo->m_Object.Reset();
		pObjectInfo->m_pObject = m_pFreeObjectInfo;
		m_pFreeObjectInfo = pObjectInfo;
	}

	//===================================================================================
	//
	//===================================================================================
	
	bool CScriptJS::CallVM( const CCallScriptBase* pCallBase,
		void* pRetBuf, void** pArgArray )
	{
		SCallInfo* pInfo = GetCallInfo( pCallBase );
		return CCallBackJS::CallVM( *pInfo->m_pScript, 
			pInfo->m_strName, pCallBase, pRetBuf, pArgArray );
	}

	void CScriptJS::DestrucVM( const CCallScriptBase* pCallBase, SVirtualObj* pObject )
	{
		SCallInfo* pInfo = GetCallInfo( pCallBase );
		return CCallBackJS::DestrucVM( *pInfo->m_pScript,
			pInfo->m_pScript->GetV8Context().m_Deconstruction, pCallBase, pObject );
	}

	SObjInfo* CScriptJS::FindExistObjInfo( void* pObj )
	{
		if( pObj == NULL )
			return NULL;

		SObjInfo* pRight = m_mapObjInfo.UpperBound( pObj );
		if( pRight == m_mapObjInfo.GetFirst() )
			return NULL;

		SObjInfo* pLeft = pRight ? pRight->GetPre() : m_mapObjInfo.GetLast();
		if( !pLeft )
			return NULL;
		ptrdiff_t nDiff = ( (const char*)pObj ) - ( (const char*)pLeft->m_pObject );
		if( nDiff >= (ptrdiff_t)( pLeft->m_pClassInfo->m_pClassInfo->GetClassSize() ) )
			return NULL;
		return pLeft;
	}

    //==================================================================================================================================//
    //                                                        对C++提供的功能性函数                                                     //
    //==================================================================================================================================//
   	bool CScriptJS::RunBuffer( const void* pBuffer, size_t nSize, const char* szFileName )
	{
		SV8Context& Context = GetV8Context();
		v8::Isolate* isolate = Context.m_pIsolate;
		v8::HandleScope handle_scope( isolate );
		v8::TryCatch try_catch( isolate );

		// Enter the context for compiling and running the hello world script.
		v8::Local<v8::Context> context = Context.m_Context.Get( isolate );
		v8::Context::Scope context_scope(context);

		// Create a string containing the JavaScript source code.
		v8::Local<v8::String> source = v8::String::NewFromUtf8( 
			isolate, (const char*)pBuffer, v8::NewStringType::kNormal, nSize )
			.ToLocalChecked();

		// Compile the source code.
		v8::MaybeLocal<v8::String> fileName = v8::String::NewFromUtf8(
			isolate, szFileName, v8::NewStringType::kNormal );
		v8::ScriptOrigin origin(fileName.ToLocalChecked());
		auto temp_script = v8::Script::Compile(context, source, &origin);
		if( temp_script.IsEmpty() )
			return false;
		v8::Local<v8::Script> script = temp_script.ToLocalChecked();

		// Run the script to get the result.
		v8::MaybeLocal<v8::Value> result = script->Run( context );
		if( !result.IsEmpty() )
			return true;
		Context.ReportException( &try_catch, context );
		return false;
	}

	bool CScriptJS::RunFunction( const STypeInfoArray& aryTypeInfo, 
		void* pResultBuf, const char* szFunction, void** aryArg )
	{
		SV8Context& Context = GetV8Context();
		v8::Isolate* isolate = Context.m_pIsolate;
		v8::HandleScope handle_scope( isolate );
		// Enter the context for compiling and running the hello world script.
		v8::Local<v8::Context> context = Context.m_Context.Get( isolate );
		v8::Context::Scope context_scope(context);

		LocalValue args[256];
		uint32 nParamCount = aryTypeInfo.nSize - 1;
		for( uint32 nArgIndex = 0; nArgIndex < nParamCount; nArgIndex++ )
		{
			DataType nType = ToDataType( aryTypeInfo.aryInfo[nArgIndex] );
			CJSTypeBase* pParamType = GetTypeBase( nType );
			args[nArgIndex] = pParamType->ToVMValue( nType, *this, (char*)aryArg[nArgIndex] );
		}

		v8::Local<v8::Object> classObject = context->Global();
		const char* szFunName = strrchr( szFunction, '.' );
		if (szFunName)
		{
			std::string szClass;
			szClass = std::string(szFunction, szFunName - szFunction);
			auto object = classObject->Get( v8::String::NewFromUtf8( isolate, szClass.c_str()));
			if (object.IsEmpty())
				return false;
			classObject = object->ToObject( isolate );
			szFunction = szFunName + 1;
		}

		auto value = classObject->Get( v8::String::NewFromUtf8( isolate, szFunction ) );
		v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast( value );
		v8::MaybeLocal<v8::Value> result = func->Call( classObject, nParamCount, args );
		if( result.IsEmpty() )
			return false;
		DataType nResultType = ToDataType( aryTypeInfo.aryInfo[nParamCount] );
		if( nResultType && pResultBuf )
			GetTypeBase( nResultType )->FromVMValue( nResultType, 
				*this, (char*)pResultBuf, result.ToLocalChecked() );
		return true;
	}

	void CScriptJS::BuildRegisterInfo()
	{
		SV8Context& Context = GetV8Context();
		v8::Isolate* isolate = Context.m_pIsolate;
		std::function<void( const CClassRegistInfo*,
			v8::Local<v8::Function>, v8::Local<v8::Object>, bool )> MakeMeberFunction;

		MakeMeberFunction = [&]( const CClassRegistInfo* pInfo,
			v8::Local<v8::Function> NewClass, v8::Local<v8::Object> Prototype, bool bBase )->void
		{
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			const CCallBaseMap& mapFunction = pInfo->GetRegistFunction();
			std::string strName = pInfo->GetClassName().c_str();
			for( auto pCall = mapFunction.GetFirst(); pCall; pCall = pCall->GetNext() )
			{
				const char* szFunName = pCall->GetFunctionName().c_str();
				if( pCall->GetFunctionIndex() == eCT_MemberFunction )
				{
					Prototype->SetAccessor( context, 
						v8::String::NewFromUtf8( isolate, szFunName ),
						&SV8Context::GetterCallback, &SV8Context::SetterCallback,
						v8::External::New( isolate, GetCallInfo( pCall ) ) );
				}
				else if( pCall->GetFunctionIndex() == eCT_ClassStaticFunction )
				{
					NewClass->Set( context, 
						v8::String::NewFromUtf8( isolate, szFunName ),
						v8::Function::New( isolate, &SV8Context::Callback,
						v8::External::New( isolate, GetCallInfo( pCall ) ) ) );
				}
				else
				{
					Prototype->Set( context, 
						v8::String::NewFromUtf8( isolate, szFunName ),
						v8::Function::New( isolate, &SV8Context::Callback,
						v8::External::New( isolate, GetCallInfo( pCall ) ) ) );
				}
			}
			if( !bBase )
				return;
			for( uint32 i = 0; i < pInfo->BaseRegist().size(); i++ )
				MakeMeberFunction( pInfo->BaseRegist()[i].m_pBaseInfo, NewClass, Prototype, true );
		};

		auto CheckClassInfo = [this]( const CClassRegistInfo* pInfo, 
			v8::Isolate* isolate, v8::Local<v8::Context> context )->SClassInfo*
		{
			SClassInfo* classInfo = m_mapClassInfo.Find( (const void*)pInfo );
			if( classInfo != nullptr )
				return classInfo;

			classInfo = new SClassInfo;
			classInfo->m_pScript = this;
			classInfo->m_pClassInfo = pInfo;
			const char* szClass = pInfo->GetClassName().c_str();
			v8::Local<v8::String> strClassName = v8::String::NewFromUtf8( isolate, szClass );
			v8::Local<v8::FunctionTemplate> NewTemplate = v8::FunctionTemplate::New(
				isolate, &SV8Context::NewObject, v8::External::New( isolate, classInfo ) );
			NewTemplate->SetClassName( strClassName );
			NewTemplate->InstanceTemplate()->SetInternalFieldCount( 1 );
			PersistentFunTmplt& persistentTemplate = classInfo->m_FunctionTemplate;
			persistentTemplate.Reset( isolate, NewTemplate );
			m_mapClassInfo.Insert( *classInfo );
			return classInfo;
		};

		const CTypeIDNameMap& mapRegisterInfo = CClassRegistInfo::GetAllRegisterInfo();
		for( auto pInfo = mapRegisterInfo.GetFirst(); pInfo; pInfo = pInfo->GetNext() )
		{
			if( pInfo->IsEnum() )
				continue;

			if( pInfo->GetTypeIDName().empty() )
			{
				v8::HandleScope handle_scope( isolate );
				v8::Local<v8::Context> context = Context.m_Context.Get( isolate );
				v8::Context::Scope context_scope( context );
				v8::Local<v8::Object> globalObj = context->Global();
				v8::Local<v8::String> strPackage = v8::String::NewFromUtf8( isolate, "window" );
				LocalValue Package = globalObj->Get( strPackage );
				assert( !Package.IsEmpty() );
				const CCallBaseMap& mapFunction = pInfo->GetRegistFunction();
				for( auto pCall = mapFunction.GetFirst(); pCall; pCall = pCall->GetNext() )
				{
					v8::Local<v8::Function> funGlobal = v8::Function::New( isolate,
						&SV8Context::Callback, v8::External::New( isolate, GetCallInfo(pCall) ) );
					const char* szFunName = pCall->GetFunctionName().c_str();
					Package->ToObject( isolate )->Set(
						v8::String::NewFromUtf8( isolate, szFunName ), funGlobal );
				}
				continue;
			}

			v8::HandleScope handle_scope( isolate );
			v8::Local<v8::Context> context = Context.m_Context.Get( isolate );
			v8::Context::Scope context_scope( context );
			v8::Local<v8::Object> globalObj = context->Global();
			SClassInfo* classInfo = CheckClassInfo( pInfo, isolate, context );
			assert( classInfo != nullptr );

			const char* szClass = pInfo->GetClassName().c_str();
			PersistentFunTmplt& persistentTemplate = classInfo->m_FunctionTemplate;
			v8::Local<v8::FunctionTemplate> NewTemplate = persistentTemplate.Get( isolate );
			v8::Local<v8::Function> NewClass = NewTemplate->GetFunction( context ).ToLocalChecked();

			LocalValue Base = Undefined( isolate );
			if( pInfo->BaseRegist().size() )
			{
				const CClassRegistInfo* pBaseInfo = pInfo->BaseRegist()[0].m_pBaseInfo;
				SClassInfo* classInfo = CheckClassInfo( pBaseInfo, isolate, context );
				assert( classInfo != nullptr );
				PersistentFunTmplt& persistentTemplate = classInfo->m_FunctionTemplate;
				v8::Local<v8::FunctionTemplate> BaseTemplate = persistentTemplate.Get( isolate );
				Base = BaseTemplate->GetFunction( context ).ToLocalChecked();
			}

			std::string strNewPath = szClass;
			v8::Local<v8::Function> GammaClass = Context.m_GammaClass.Get( isolate );
			v8::Local<v8::String> strPathName = v8::String::NewFromUtf8( isolate, strNewPath.c_str() );
			LocalValue args[] = { NewClass, strPathName, Base };
			GammaClass->Call( globalObj, 3, args );

			v8::MaybeLocal<v8::Value> Prototype = NewClass->Get( context, Context.m_Prototype.Get( isolate ) );
			v8::Local<v8::Object> PrototypeObj = Prototype.ToLocalChecked()->ToObject( isolate );
			v8::Local<v8::Value> InfoValue = v8::External::New( isolate, pInfo );
			PrototypeObj->Set( context, Context.m_Deconstruction.Get( isolate ),
				v8::Function::New( isolate, &SV8Context::Destruction, InfoValue ) );
			for( uint32 i = 1; i < pInfo->BaseRegist().size(); i++ )
				MakeMeberFunction( pInfo->BaseRegist()[i].m_pBaseInfo, NewClass, PrototypeObj, true );
			MakeMeberFunction( pInfo, NewClass, PrototypeObj, false );
		}
	}

	int32 CScriptJS::Compiler( int32 nArgc, char** szArgv )
    {
        return -1;
    }

    void CScriptJS::UnlinkCppObjFromScript( void* pObj )
	{
		SObjInfo* pObjInfo = FindExistObjInfo( pObj );
		if( !pObjInfo )
			return;
		// 这里仅仅解除绑定
		pObjInfo->Remove();
		pObjInfo->m_pObject = NULL;
	}

	void CScriptJS::GC()
	{
		GetV8Context().m_pIsolate->LowMemoryNotification();
	}

	void CScriptJS::GCAll()
	{
		GetV8Context().m_pIsolate->IdleNotificationDeadline(0.1);
	}
};