#include "common/CodeCvs.h"
#include "common/TStrStream.h"
#include "core/CClassRegistInfo.h"
#include "CScriptJS.h"
#include "CTypeJS.h"
#include "CDebugJS.h"
#include "CCallJS.h"

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
	#define MAX_STRING_BUFFER_SIZE	65536
	using namespace v8;

	//====================================================================================
    // CScriptJS
	//====================================================================================
    CScriptJS::CScriptJS( uint16 nDebugPort )
		: m_nStringID(0)
		, m_pTempStrBuffer64K(new tbyte[MAX_STRING_BUFFER_SIZE])
		, m_nCurUseSize(0)
		, m_nStrBufferStack(0)
		, m_pIsolate( NULL )
		, m_pFreeObjectInfo( NULL )
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
		m_platform = s_Init.m_platform;

		// Create a new Isolate and make it the current one.
		m_pIsolate = v8::Isolate::New(s_Init.m_create_params);
		m_pIsolate->Enter();
		
		// Create a stack-allocated handle scope.
		v8::HandleScope handle_scope(m_pIsolate);

		// Create a new context.
		v8::Local<v8::Context> context = v8::Context::New(m_pIsolate);
		m_Context.Reset(m_pIsolate, context);

		v8::Context::Scope context_scope(context);
		v8::Local<v8::Object> globalObj = context->Global();
		globalObj->Set(v8::String::NewFromUtf8(m_pIsolate, "window"), globalObj);

		LocalValue console = globalObj->Get(String::NewFromUtf8(m_pIsolate, "console")); 
		v8::Local<v8::Object> consoleObj = console->ToObject(m_pIsolate);
		v8::Local<v8::Function> funLog = v8::Function::New(m_pIsolate, &CScriptJS::Log);
		consoleObj->Set(context, v8::String::NewFromUtf8(m_pIsolate, "log"), funLog);

		v8::Local<v8::External> ScriptContext = v8::External::New(m_pIsolate, this);
		v8::Local<v8::Function> funDebug = Function::New(m_pIsolate, &CScriptJS::Break, ScriptContext);
		consoleObj->Set( context, v8::String::NewFromUtf8( m_pIsolate, "gsd" ), funDebug );
		m_pDebugger = new CDebugJS( this, nDebugPort );

		m_CppField.Reset( m_pIsolate, v8::String::NewFromUtf8( m_pIsolate, "__cpp_obj_info__" ) );
		m_Prototype.Reset( m_pIsolate, String::NewFromUtf8( m_pIsolate, "prototype" ) );
		m_Deconstruction.Reset( m_pIsolate, String::NewFromUtf8( m_pIsolate, "Deconstruction" ) );
		m___proto__.Reset( m_pIsolate, v8::String::NewFromUtf8( m_pIsolate, "__proto__" ) );

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

		LocalValue nsGamma = globalObj->Get(String::NewFromUtf8(m_pIsolate, "Gamma"));
		Local<Object> nsGammaObject = nsGamma->ToObject(m_pIsolate);
		LocalValue GammaClass = nsGammaObject->Get(String::NewFromUtf8(m_pIsolate, "class"));
		m_GammaClass.Reset(m_pIsolate, Local<Function>::Cast(GammaClass));
		m_GammaNameSpace.Reset(m_pIsolate, nsGammaObject);
    }

    CScriptJS::~CScriptJS(void)
	{
		SAFE_DELETE( m_pDebugger );
		ClearCppString((void*)(uintptr_t)(-1));
		m_Context.Reset();
		m_pIsolate->Exit();
		m_pIsolate->Dispose();
		m_pIsolate = NULL;

		while( m_mapClassInfo.GetFirst() )
			delete m_mapClassInfo.GetFirst();
		while( m_mapCallBase.GetFirst() )
			delete m_mapCallBase.GetFirst();
		while( m_mapObjInfo.GetFirst() )
			delete m_mapObjInfo.GetFirst();
	}

	PersistentFunTmplt& CScriptJS::GetPersistentFunTemplate( const CClassRegistInfo* pInfo )
	{
		SClassInfo* pClassInfo = m_mapClassInfo.Find( (const void*)pInfo );
		static PersistentFunTmplt s_Instance;
		return pClassInfo ? pClassInfo->m_FunctionTemplate : s_Instance;
	}

	void CScriptJS::ClearCppString( void* pStack )
	{
		uint32 nIndex = (uint32)m_vecStringInfo.size();
		while (nIndex && m_vecStringInfo[nIndex - 1].m_pStack < pStack)
			delete[] m_vecStringInfo[--nIndex].m_pBuffer;
		if (nIndex < m_vecStringInfo.size())
			m_vecStringInfo.erase(m_vecStringInfo.begin() + nIndex, m_vecStringInfo.end());

		while (m_nCurUseSize >= sizeof(SStringFixed))
		{
			SStringFixed* pFixeString = ((SStringFixed*)(m_pTempStrBuffer64K + m_nCurUseSize)) - 1;
			if (pFixeString->m_pStack >= pStack)
				break;
			m_nCurUseSize -= (uint32)(sizeof(SStringFixed) + pFixeString->m_nLen);
		}
	}

	void CScriptJS::CallJSStatck(bool bAdd)
	{
		if (bAdd)
		{
			if (m_nStrBufferStack == 0)
				ClearCppString((void*)(uintptr_t)(-1));
			++m_nStrBufferStack;
		}
		else
		{
			assert(m_nStrBufferStack);
			--m_nStrBufferStack;
		}
	}

	LocalValue CScriptJS::StringFromUtf8(const char* szUtf8)
	{
		if (!szUtf8)
			return v8::Null(m_pIsolate);
		return String::NewFromUtf8(m_pIsolate, szUtf8);
	}

	LocalValue CScriptJS::StringFromUcs(const wchar_t* szUcs)
	{
		if (!szUcs)
			return v8::Null(m_pIsolate);
		if (sizeof(wchar_t) == sizeof(uint16_t))
			return String::NewFromTwoByte(m_pIsolate, 
			(uint16_t*)szUcs, NewStringType::kNormal).ToLocalChecked();
		m_szTempUcs2 = szUcs;
		size_t nSize = m_szTempUcs2.size();
		uint16_t* szDes = (uint16_t*)&m_szTempUcs2[0];
		for (size_t i = 0; i < nSize; i++)
			szDes[i] = m_szTempUcs2[i];
		szDes[nSize] = 0;
		return String::NewFromTwoByte(m_pIsolate,
			(uint16_t*)szDes, NewStringType::kNormal).ToLocalChecked();
	}

	const char* CScriptJS::StringToUtf8(LocalValue obj)
	{
		if (obj == v8::Null(m_pIsolate))
			return NULL;
		v8::Isolate* isolate = GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::String> v = obj->ToString(context);
		if (v.IsEmpty())
			return NULL;
		v8::Local<v8::String> StringObject = v.ToLocalChecked();
		size_t nStrLen = StringObject->Utf8Length(m_pIsolate);
		if (nStrLen == 0)
			return "";

		size_t nAllocSize = AligenUp((uint32)(nStrLen + 1), sizeof(void*));
		if (nAllocSize + m_nCurUseSize + sizeof(SStringFixed) < MAX_STRING_BUFFER_SIZE)
		{
			char* szUtf8 = (char*)(m_pTempStrBuffer64K + m_nCurUseSize);
			StringObject->WriteUtf8(m_pIsolate, szUtf8);
			SStringFixed strCpp;
			strCpp.m_pStack = &strCpp;
			strCpp.m_nLen = AligenUp((uint32)(nStrLen + 1), sizeof(void*));
			m_nCurUseSize += strCpp.m_nLen + sizeof(SStringFixed);
			memcpy(m_pTempStrBuffer64K + m_nCurUseSize - sizeof(SStringFixed), &strCpp, sizeof(SStringFixed));
			return szUtf8;
		}
		else
		{
			SStringDynamic strCpp;
			strCpp.m_pStack = &strCpp;
			strCpp.m_pBuffer = new char[nStrLen + 1];
			StringObject->WriteUtf8(m_pIsolate, (char*)strCpp.m_pBuffer);
			m_vecStringInfo.push_back(strCpp);
			return (char*)strCpp.m_pBuffer;
		}
	}

	const wchar_t* CScriptJS::StringToUcs(LocalValue obj)
	{
		if (obj == v8::Null(m_pIsolate))
			return NULL;
		v8::Isolate* isolate = GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::String> v = obj->ToString(context);
		if (v.IsEmpty())
			return NULL;
		v8::Local<v8::String> StringObject = v.ToLocalChecked();
		size_t nStrLen = StringObject->Utf8Length(m_pIsolate);
		if (nStrLen == 0)
			return L"";

		uint32 nAllocSize = AligenUp(uint32(nStrLen + 1) * sizeof(wchar_t), sizeof(void*));
		wchar_t* szUcs2 = NULL;
		if (nAllocSize + m_nCurUseSize < MAX_STRING_BUFFER_SIZE)
		{
			szUcs2 = (wchar_t*)(m_pTempStrBuffer64K + m_nCurUseSize);
			StringObject->Write(m_pIsolate, (uint16_t*)szUcs2);
			szUcs2[nStrLen] = 0;
			SStringFixed strCpp;
			strCpp.m_pStack = &strCpp;
			strCpp.m_nLen = nAllocSize;
			m_nCurUseSize += strCpp.m_nLen + sizeof(SStringFixed);
			memcpy(m_pTempStrBuffer64K + m_nCurUseSize - sizeof(SStringFixed), &strCpp, sizeof(SStringFixed));
		}
		else
		{			
			SStringDynamic strCpp;
			strCpp.m_pStack = &strCpp;
			strCpp.m_pBuffer = szUcs2 = new wchar_t[nStrLen + 1];
			StringObject->Write(m_pIsolate, (uint16_t*)szUcs2);
			m_vecStringInfo.push_back(strCpp);
		}

		if (sizeof(wchar_t) == sizeof(uint16_t))
			return szUcs2;
		uint16_t* szSrc = (uint16_t*)szUcs2;
		while (nStrLen--)
			szUcs2[nStrLen] = szSrc[nStrLen];
		return szUcs2;
	}

	void CScriptJS::ReportException(v8::TryCatch* try_catch, Local<Context> context)
	{
		v8::Local<v8::Message> message = try_catch->Message();
		v8::String::Utf8Value exception(m_pIsolate, try_catch->Exception());
		if (message.IsEmpty()) 
		{
			// V8 didn't provide any extra information about this error; just
			// print the exception.
			Output( "Error:", -1 );
			Output( *exception, -1 );
			Output( "\n", -1 );
			return;
		}

		char szNumber[32];
		// Print (filename):(line number): (message).
		v8::String::Utf8Value filename(m_pIsolate, message->GetScriptResourceName());
		sprintf( szNumber, "%d", message->GetLineNumber( context ).ToChecked() );
		Output( *filename, -1 );
		Output( ":", -1 );
		Output( szNumber, -1 );
		Output( "\n\t", -1 );
		Output( *exception, -1 );
		Output( "\n", -1 );

		// Print line of source code.
		v8::Local<String> souceline = message->GetSourceLine( context ).ToLocalChecked();
		Output( *String::Utf8Value( m_pIsolate, souceline ), -1 );
		Output( "\n", -1 );

		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn();
		for( int i = 0; i < start; i++ )
			Output( " ", -1 );
		int end = message->GetEndColumn();
		for( int i = start; i < end; i++ )
			Output( " ^", -1 );
		Output( "\n", -1 );
		v8::MaybeLocal<Value> backTrace = try_catch->StackTrace(context);
		if (backTrace.IsEmpty())
			return;
		v8::Local<Value> strBackTrace = backTrace.ToLocalChecked();
		v8::String::Utf8Value stack_trace(m_pIsolate, strBackTrace->ToString(m_pIsolate));
		if (!stack_trace.length())
			return;
		Output( *stack_trace, -1 );
	}

	CScriptJS::SCallInfo* CScriptJS::GetCallInfo( CByScriptBase* pCallBase )
	{
		SCallInfo* pCallInfo = m_mapCallBase.Find( (const void*)pCallBase );
		if( pCallInfo )
			return pCallInfo;
		pCallInfo = new CScriptJS::SCallInfo;
		pCallInfo->m_pCallBase = pCallBase;
		pCallInfo->m_pScript = this;
		pCallInfo->m_strName.Reset( m_pIsolate, String::NewFromUtf8
			( m_pIsolate, pCallBase->GetFunctionName().c_str() ) );
		return pCallInfo;
	}

	CScriptJS::SObjInfo* CScriptJS::AllocObjectInfo()
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

	void CScriptJS::FreeObjectInfo(CScriptJS::SObjInfo* pObjectInfo)
	{
		pObjectInfo->m_Object.Reset();
		pObjectInfo->m_pObject = m_pFreeObjectInfo;
		m_pFreeObjectInfo = pObjectInfo;
	}

	//===================================================================================
	//
	//===================================================================================
	void CScriptJS::Log(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Local<External> wrap = Local<External>::Cast( args.Data() );
		CScriptJS* pScript = (CScriptJS*)wrap->Value();
		Isolate* isolate = args.GetIsolate();
		HandleScope scope( isolate );
		for (int32 i = 0; i < args.Length(); i++)
		{
			Local<Value> arg = args[i];
			String::Utf8Value value( isolate, arg );
			const char* szValue = *value;
			pScript->Output( szValue, -1 );
			pScript->Output( " ", -1 );
		}
		pScript->Output( "\n", -1 );
	}

	void CScriptJS::Break(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Local<External> wrap = Local<External>::Cast(args.Data());
		CScriptJS* pScript = (CScriptJS*)wrap->Value();
		((CDebugJS*)pScript->m_pDebugger)->Break();
	}

	void CScriptJS::Callback(const FunctionCallbackInfo<Value>& args)
	{
		Local<External> wrap = Local<External>::Cast(args.Data());
		SCallInfo* pCallInfo = (SCallInfo*)wrap->Value();
		if (!pCallInfo )
			return;
		Isolate* isolate = args.GetIsolate();
		HandleScope scope(isolate);
		CByScriptJS::CallByJS(*pCallInfo->m_pScript, pCallInfo->m_pCallBase, args);
	}

	void CScriptJS::NewObject(const FunctionCallbackInfo<Value>& args)
	{
		SClassInfo* pInfo = (SClassInfo*)External::Cast(*args.Data())->Value();
		if (pInfo == NULL)
			return;

		CScriptJS& Script = *static_cast<CScriptJS*>(pInfo->m_pScript);
		Local<Object> ScriptObj = args.This();
		v8::External* pCppBind = NULL;
		if (ScriptObj->InternalFieldCount())
			pCppBind = v8::External::Cast(*ScriptObj->GetInternalField(0));
		else
		{
			v8::Local<v8::Value> key = Script.m_CppField.Get(Script.GetIsolate());
			v8::Local<v8::Context> context = Script.GetIsolate()->GetCurrentContext();
			v8::MaybeLocal<v8::Value> field = ScriptObj->Get(context, key);
			pCppBind = v8::External::Cast(*(field.ToLocalChecked()));
		}
		if (pCppBind->Value())
		{
			args.GetReturnValue().Set(ScriptObj);
			return;
		}

		Script.BindObj( NULL, args.This(), pInfo->m_pClassInfo );
	}

	void CScriptJS::Destruction(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		Local<External> wrap = Local<External>::Cast(args.Data());
		SClassInfo* pClassInfo = (SClassInfo*)wrap->Value();
		CScriptJS& Script = *(CScriptJS*)pClassInfo->m_pScript;
		v8::Object* pScriptObject = v8::Object::Cast(*args.This());
		v8::External* pCppBind = NULL;
		if (pScriptObject->InternalFieldCount())
			pCppBind = v8::External::Cast(*pScriptObject->GetInternalField(0));
		else
		{
			v8::Local<v8::Value> key = Script.m_CppField.Get(Script.GetIsolate());
			v8::Local<v8::Context> context = Script.GetIsolate()->GetCurrentContext();
			v8::MaybeLocal<v8::Value> field = pScriptObject->Get(context, key);
			pCppBind = v8::External::Cast(*(field.ToLocalChecked()));
		}

		SObjInfo* pObjectInfo = (SObjInfo*)pCppBind->Value();
		if (!pObjectInfo)
			return;
		Script.UnbindObj(pObjectInfo, false);
	}

	void CScriptJS::GCCallback(const v8::WeakCallbackInfo<SObjInfo>& data)
	{
		SObjInfo* pObjectInfo = data.GetParameter();
		SClassInfo* pInfo = pObjectInfo->m_pClassInfo;
		pInfo->m_pScript->UnbindObj(pObjectInfo, true);
	}

	void CScriptJS::GetterCallback(Local<Name> property, const PropertyCallbackInfo<Value>& info)
	{
		Local<External> wrap = Local<External>::Cast(info.Data());
		SCallInfo* pCallInfo = (SCallInfo*)wrap->Value();
		if (!pCallInfo )
			return;
		Isolate* isolate = info.GetIsolate();
		HandleScope scope(isolate);
		CByScriptJS::GetByJS( *pCallInfo->m_pScript, 
			pCallInfo->m_pCallBase, info.This(), info.GetReturnValue());
	}

	void CScriptJS::SetterCallback(Local<Name> property, LocalValue value, const PropertyCallbackInfo<void>& info)
	{
		Local<External> wrap = Local<External>::Cast(info.Data());
		SCallInfo* pCallInfo = (SCallInfo*)wrap->Value();
		if (!pCallInfo )
			return;
		Isolate* isolate = info.GetIsolate();
		HandleScope scope(isolate);
		CByScriptJS::SetByJS( *pCallInfo->m_pScript,
			pCallInfo->m_pCallBase, info.This(), value );
	}

	void CScriptJS::BindObj( void* pObject, Local<Object> ScriptObj, const CClassRegistInfo* pInfo, void* pSrc )
	{
		SObjInfo& ObjectInfo = *AllocObjectInfo();
		ObjectInfo.m_bRecycle = false;
		if (!pObject)
		{
			pObject = new tbyte[pInfo->GetClassSize()];
			pInfo->Create( pObject );
			CheckUnlinkCppObj();
			if( pSrc )
			{
				pInfo->Assign( pObject, pSrc );
				CheckUnlinkCppObj();
			}
			ObjectInfo.m_bRecycle = true;
		}

		ObjectInfo.m_Object.Reset(m_pIsolate, ScriptObj);
		ObjectInfo.m_pClassInfo = m_mapClassInfo.Find( (const void*)pInfo );
		ObjectInfo.m_pObject = pObject;
		m_mapObjInfo.Insert(ObjectInfo);

		// 注册回调函数
		if (pInfo->IsCallBack())
			pInfo->ReplaceVirtualTable(this, pObject, ObjectInfo.m_bRecycle, 0);

		Local<External> cppInfo = External::New(m_pIsolate, &ObjectInfo);
		if (ScriptObj->InternalFieldCount())
			ScriptObj->SetInternalField(0, cppInfo);
		else
			ScriptObj->Set(m_pIsolate->GetCurrentContext(), m_CppField.Get(m_pIsolate), cppInfo);
		ObjectInfo.m_Object.SetWeak(&ObjectInfo, &CScriptJS::GCCallback, WeakCallbackType::kParameter);
	}

	void CScriptJS::UnbindObj(SObjInfo* pObjectInfo, bool bFromGC)
	{
		void* pObject = pObjectInfo->m_pObject;
		bool bRecycle = pObjectInfo->m_bRecycle;
		const CClassRegistInfo* pInfo = pObjectInfo->m_pClassInfo->m_pClassInfo;
		CScriptJS* pScript = pObjectInfo->m_pClassInfo->m_pScript;
		Isolate* isolate = pScript->GetIsolate();
		v8::HandleScope handle_scope(isolate);
		v8::TryCatch try_catch(isolate);

		if( !bFromGC )
		{
			Local<v8::Object> ScriptObj = pObjectInfo->m_Object.Get(isolate);
			assert( !ScriptObj.IsEmpty()&&ScriptObj->IsObject() );
			if (ScriptObj->InternalFieldCount())
				ScriptObj->SetInternalField( 0, v8::Null(isolate) );
			else
				ScriptObj->Set( m_pIsolate->GetCurrentContext(), m_CppField.Get( m_pIsolate ), v8::Null(isolate) );
		}

		pObjectInfo->Remove();
		pObjectInfo->m_pObject = NULL;
		pObjectInfo->m_Object.Reset();

		pScript->FreeObjectInfo(pObjectInfo);
		if (!pObject)
			return;
		pInfo->RecoverVirtualTable(pScript, pObject);
		if (!bRecycle)
			return;
		pInfo->Release( pObject );
		CheckUnlinkCppObj();
		delete[](tbyte*)pObject;
	}
	
	bool CScriptJS::CallVM( CCallScriptBase* pCallBase, 
		SVirtualObj* pObject, void* pRetBuf, void** pArgArray )
	{
		SCallInfo* pInfo = GetCallInfo( pCallBase );
		return CCallBackJS::CallVM( *pInfo->m_pScript, 
			pInfo->m_strName, pCallBase, pObject, pRetBuf, pArgArray );
	}

	void CScriptJS::DestrucVM( CCallScriptBase* pCallBase, SVirtualObj* pObject )
	{
		SCallInfo* pInfo = GetCallInfo( pCallBase );
		return CCallBackJS::DestrucVM( *pInfo->m_pScript,
			pInfo->m_pScript->m_Deconstruction, pCallBase, pObject );
	}

	Gamma::CScriptJS::SObjInfo* CScriptJS::FindExistObjInfo( void* pObj )
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
    //                                                        对内部提供的功能性函数                                                    //
	//==================================================================================================================================//
   

	//==================================================================================================================================//
	//                                                        字符串函数																//
	//==================================================================================================================================//
	

    //==================================================================================================================================//
    //                                                        对C++提供的功能性函数                                                     //
    //==================================================================================================================================//
    bool CScriptJS::RunFile( const char* szFileName, bool /*bReload*/ )
	{
		CheckUnlinkCppObj();
		FILE* iFile = NULL;
		std::string sFileName;
		if( szFileName[0] == '/' || ::strchr( szFileName, ':' ) )
		{
			if( NULL == ( iFile = fopen( szFileName, "rb" ) ) )
				return false;
			sFileName = szFileName;
		}
		else
		{
			std::list<std::string>::iterator it = m_listSearchPath.begin();
			while( it != m_listSearchPath.end() && 
				NULL == ( iFile = fopen( ( *it + szFileName ).c_str(), "rb" ) ) )
				++it;
			while( it == m_listSearchPath.end() )
				return false;
			sFileName = *it + szFileName;
		}

		ShortPath( &sFileName[0] );
		if (sFileName[0] == '/')
			sFileName = "file://" + sFileName;
		else
			sFileName = "file:///" + sFileName;

		std::string sBuf;
		fseek( iFile, 0, SEEK_END );
		sBuf.resize( ftell( iFile ) );
		fseek( iFile, 0, SEEK_SET );
		fread( &sBuf[0], sBuf.size(), 1, iFile );
		fclose( iFile );

		HandleScope handle_scope(m_pIsolate);
		v8::TryCatch try_catch(m_pIsolate);

		// Enter the context for compiling and running the hello world script.
		Local<Context> context = m_Context.Get(m_pIsolate);
		Context::Scope context_scope(context);

		// Create a string containing the JavaScript source code.
		Local<String> source =
			String::NewFromUtf8(m_pIsolate, sBuf.c_str(), NewStringType::kNormal)
			.ToLocalChecked();

		// Compile the source code.
		MaybeLocal<String> fileName = String::NewFromUtf8(
			m_pIsolate, sFileName.c_str(), NewStringType::kNormal);
		v8::ScriptOrigin origin(fileName.ToLocalChecked());
		MaybeLocal<Script> temp_script = Script::Compile(context, source, &origin);
		Local<Script> script = temp_script.ToLocalChecked();

		// Run the script to get the result.
		MaybeLocal<Value> result = script->Run(context);
		if (result.IsEmpty())
		{	
			ReportException(&try_catch, context);
			return false;
		}
		if( !GetDebugger() || !GetDebugger()->RemoteDebugEnable() )
			return true;
		GetDebugger()->AddFileContent( sFileName.c_str(), sBuf.c_str() );
		return true;
	}
	
	bool CScriptJS::RunBuffer( const void* pBuffer, size_t nSize )
	{
		CheckUnlinkCppObj();
		std::string strSource((const char*)pBuffer, nSize);
		return RunString(strSource.c_str());
	}

    bool CScriptJS::RunString( const char* szString )
	{
		CheckUnlinkCppObj();

		HandleScope handle_scope(m_pIsolate);
		v8::TryCatch try_catch(m_pIsolate);

		// Enter the context for compiling and running the hello world script.
		Local<Context> context = m_Context.Get(m_pIsolate);
		Context::Scope context_scope(context);

		// Create a string containing the JavaScript source code.
		Local<String> source =
			String::NewFromUtf8(m_pIsolate, szString, NewStringType::kNormal)
			.ToLocalChecked();

		char szLaber[256];
		gammasstream(szLaber) << "$Label$String" << ++m_nStringID;

		// Compile the source code.
		MaybeLocal<String> fileName = String::NewFromUtf8(
			m_pIsolate, szLaber, NewStringType::kNormal);
		v8::ScriptOrigin origin(fileName.ToLocalChecked());
		MaybeLocal<Script> temp_script = Script::Compile(context, source, &origin);
		Local<Script> script = temp_script.ToLocalChecked();

		// Run the script to get the result.
		MaybeLocal<Value> result = script->Run(context);
		if (result.IsEmpty())
		{
			ReportException(&try_catch, context);
			return false;
		}
		if (!GetDebugger() || !GetDebugger()->RemoteDebugEnable())
			return true;
		GetDebugger()->AddFileContent(szLaber, szString);
		return true;
	}

	bool CScriptJS::RunFunction( const STypeInfoArray& aryTypeInfo, void* pResultBuf, const char* szFunction, void** aryArg )
	{
		CheckUnlinkCppObj();
		HandleScope handle_scope(m_pIsolate);
		// Enter the context for compiling and running the hello world script.
		Local<Context> context = m_Context.Get(m_pIsolate);
		Context::Scope context_scope(context);

		LocalValue args[256];
		uint32 nParamCount = aryTypeInfo.nSize - 1;
		for( uint32 nArgIndex = 0; nArgIndex < nParamCount; nArgIndex++ )
		{
			DataType nType = ToDataType( aryTypeInfo.aryInfo[nArgIndex] );
			CJSTypeBase* pParamType = GetTypeBase( nType );
			args[nArgIndex] = pParamType->ToVMValue( nType, *this, (char*)aryArg[nArgIndex] );
		}

		Local<Object> classObject = context->Global();
		const char* szFunName = strrchr( szFunction, '.' );
		if (szFunName)
		{
			std::string szClass;
			szClass = std::string(szFunction, szFunName - szFunction);
			Local<Value>object = classObject->Get(String::NewFromUtf8(m_pIsolate, szClass.c_str()));
			if (object.IsEmpty())
				return false;
			classObject = object->ToObject(m_pIsolate);
			szFunction = szFunName + 1;
		}

		Local<Value> value = classObject->Get(String::NewFromUtf8(m_pIsolate, szFunction));
		Local<Function> func = Local<Function>::Cast(value);
		MaybeLocal<Value> result = !nParamCount ? func->Call(classObject, 0, args) :
			func->Call(args[0], nParamCount - 1, args + 1);
		if(result.IsEmpty())
			return false;
		DataType nResultType = ToDataType( aryTypeInfo.aryInfo[nParamCount] );
		if( nResultType && pResultBuf )
			GetTypeBase( nResultType )->FromVMValue( nResultType, *this, (char*)pResultBuf, result.ToLocalChecked() );
		return true;
	}

	void CScriptJS::BuildRegisterInfo()
	{
		const CTypeIDNameMap& mapRegisterInfo = CClassRegistInfo::GetAllRegisterInfo();
		for( auto pInfo = mapRegisterInfo.GetFirst(); pInfo; pInfo = pInfo->GetNext() )
		{
			if( pInfo->IsEnum() )
				continue;

			if( pInfo->GetTypeIDName().empty() )
			{
				HandleScope handle_scope( m_pIsolate );
				Local<Context> context = m_Context.Get( m_pIsolate );
				Context::Scope context_scope( context );
				Local<Object> globalObj = context->Global();
				Local<String> strPackage = String::NewFromUtf8( m_pIsolate, "window" );
				LocalValue Package = globalObj->Get( strPackage );
				assert( !Package.IsEmpty() );
				const CCallBaseMap& mapFunction = pInfo->GetRegistFunction();
				for( auto pCall = mapFunction.GetFirst(); pCall; pCall = pCall->GetNext() )
				{
					Local<Function> funGlobal = Function::New( GetIsolate(),
						&CScriptJS::Callback, External::New( m_pIsolate, GetCallInfo(pCall) ) );
					const char* szFunName = pCall->GetFunctionName().c_str();
					Package->ToObject( m_pIsolate )->Set( 
						String::NewFromUtf8( m_pIsolate, szFunName ), funGlobal );
				}
				continue;
			}

			HandleScope handle_scope( m_pIsolate );
			Local<Context> context = m_Context.Get( m_pIsolate );
			Context::Scope context_scope( context );
			Local<Object> globalObj = context->Global();

			const char* szClass = pInfo->GetClassName().c_str();
			Local<String> strClassName = String::NewFromUtf8( m_pIsolate, szClass );
			Local<FunctionTemplate> NewTemplate = FunctionTemplate::New(
				m_pIsolate, &CScriptJS::NewObject, External::New( m_pIsolate, pInfo ) );
			NewTemplate->SetClassName( strClassName );
			NewTemplate->InstanceTemplate()->SetInternalFieldCount( 1 );
			Local<Function> NewClass = NewTemplate->GetFunction( context ).ToLocalChecked();
			PersistentFunTmplt& persistentTemplate = GetPersistentFunTemplate( pInfo );
			persistentTemplate.Reset( m_pIsolate, NewTemplate );

			LocalValue Base = Undefined( m_pIsolate );
			if( pInfo->BaseRegist().size() )
			{
				const CClassRegistInfo* pBaseInfo = pInfo->BaseRegist()[0].m_pBaseInfo;
				PersistentFunTmplt& persistentTemplate = GetPersistentFunTemplate( pBaseInfo );
				Local<FunctionTemplate> BaseTemplate = persistentTemplate.Get( m_pIsolate );
				Base = BaseTemplate->GetFunction( context ).ToLocalChecked();
			}

			std::string strNewPath = szClass;
			Local<Function> GammaClass = m_GammaClass.Get( m_pIsolate );
			Local<String> strPathName = String::NewFromUtf8( m_pIsolate, strNewPath.c_str() );
			LocalValue args[] = { NewClass, strPathName, Base };
			GammaClass->Call( globalObj, 3, args );

			MaybeLocal<Value> Prototype = NewClass->Get( context, m_Prototype.Get( m_pIsolate ) );
			Local<Object> PrototypeObj = Prototype.ToLocalChecked()->ToObject( m_pIsolate );
			Local<Value> InfoValue = External::New( m_pIsolate, pInfo );
			PrototypeObj->Set( context, m_Deconstruction.Get( m_pIsolate ),
				Function::New( m_pIsolate, &CScriptJS::Destruction, InfoValue ) );
			for( uint32 i = 1; i < pInfo->BaseRegist().size(); i++ )
				MakeMeberFunction( pInfo->BaseRegist()[i].m_pBaseInfo, NewClass, PrototypeObj, true );
			MakeMeberFunction( pInfo, NewClass, PrototypeObj, false );
		}
	}

	void CScriptJS::MakeMeberFunction( const CClassRegistInfo* pInfo, 
		Local<Function> NewClass, v8::Local<v8::Object> Prototype, bool bBase )
	{
		Local<Context> context = m_pIsolate->GetCurrentContext();
		const CCallBaseMap& mapFunction = pInfo->GetRegistFunction();
		std::string strName = pInfo->GetClassName().c_str();
		for( auto pCall = mapFunction.GetFirst(); pCall; pCall = pCall->GetNext() )
		{
			const char* szFunName = pCall->GetFunctionName().c_str();
			if( pCall->GetFunctionIndex() == eCT_MemberFunction )
			{
				Prototype->SetAccessor( context, String::NewFromUtf8( m_pIsolate, szFunName ),
					&CScriptJS::GetterCallback, &CScriptJS::SetterCallback, 
					External::New( m_pIsolate, GetCallInfo( pCall ) ) );
			}
			else if( pCall->GetFunctionIndex() == eCT_ClassStaticFunction )
			{
				NewClass->Set( context, String::NewFromUtf8( m_pIsolate, szFunName ),
					Function::New( m_pIsolate, &CScriptJS::Callback, 
						External::New( m_pIsolate, GetCallInfo( pCall ) ) ) );
			}
			else
			{
				Prototype->Set( context, String::NewFromUtf8( m_pIsolate, szFunName ),
					Function::New( m_pIsolate, &CScriptJS::Callback, 
						External::New( m_pIsolate, GetCallInfo( pCall ) ) ) );
			}
		}
		if( !bBase )
			return;
		for( uint32 i = 0; i < pInfo->BaseRegist().size(); i++ )
			MakeMeberFunction( pInfo->BaseRegist()[i].m_pBaseInfo, NewClass, Prototype, true );
	}

	int32 CScriptJS::Compiler( int32 nArgc, char** szArgv )
    {
        return -1;
    }

    void CScriptJS::RefScriptObj( void* pObj )
	{
		//[todo]                     // nStk = 0
    }

    void CScriptJS::UnrefScriptObj( void* pObj )
	{
		//[todo]
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
		m_pIsolate->LowMemoryNotification();
	}

	void CScriptJS::GCAll()
	{
		m_pIsolate->IdleNotificationDeadline(0.1);
	}
};