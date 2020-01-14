#include "V8Context.h"
#include "CScriptJS.h"
#include "CCallJS.h"
#include "CDebugJS.h"

#define MAX_STRING_BUFFER_SIZE	65536

namespace Gamma
{
	SV8Context::SV8Context( CScriptJS* pScript )
		: m_pScript(pScript)
		, m_platform(nullptr)
		, m_pIsolate(nullptr)
		, m_nStringID(0)
		, m_pTempStrBuffer64K(new tbyte[MAX_STRING_BUFFER_SIZE])
		, m_nCurUseSize(0)
		, m_nStrBufferStack(0)
	{
	}

	void SV8Context::CallJSStatck(bool bAdd)
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

	//==================================================================================================================================//
	//                                                        字符串函数																//
	//==================================================================================================================================//
	void SV8Context::ClearCppString(void* pStack)
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

	LocalValue SV8Context::StringFromUtf8(const char* szUtf8)
	{
		if (!szUtf8)
			return v8::Null(m_pIsolate);
		return v8::String::NewFromUtf8(m_pIsolate, szUtf8);
	}

	LocalValue SV8Context::StringFromUcs(const wchar_t* szUcs)
	{
		if (!szUcs)
			return v8::Null(m_pIsolate);
		if (sizeof(wchar_t) == sizeof(uint16_t))
			return v8::String::NewFromTwoByte(m_pIsolate,
			(uint16_t*)szUcs, v8::NewStringType::kNormal).ToLocalChecked();
		m_szTempUcs2 = szUcs;
		size_t nSize = m_szTempUcs2.size();
		uint16_t* szDes = (uint16_t*)&m_szTempUcs2[0];
		for (size_t i = 0; i < nSize; i++)
			szDes[i] = m_szTempUcs2[i];
		szDes[nSize] = 0;
		return v8::String::NewFromTwoByte(m_pIsolate,
			(uint16_t*)szDes, v8::NewStringType::kNormal).ToLocalChecked();
	}

	const char* SV8Context::StringToUtf8(LocalValue obj)
	{
		if (obj == v8::Null(m_pIsolate))
			return NULL;
		v8::Local<v8::Context> context = m_pIsolate->GetCurrentContext();
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

	const wchar_t* SV8Context::StringToUcs(LocalValue obj)
	{
		if (obj == v8::Null(m_pIsolate))
			return NULL;
		v8::Local<v8::Context> context = m_pIsolate->GetCurrentContext();
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


	//==================================================================================================================================//
	//                                                        对内部提供的功能性函数                                                    //
	//==================================================================================================================================//
	void SV8Context::ReportException(v8::TryCatch* try_catch, v8::Local<v8::Context> context)
	{
		v8::Local<v8::Message> message = try_catch->Message();
		v8::String::Utf8Value exception(m_pIsolate, try_catch->Exception());
		if (message.IsEmpty())
		{
			// V8 didn't provide any extra information about this error; just
			// print the exception.
			m_pScript->Output("Error:", -1);
			m_pScript->Output(*exception, -1);
			m_pScript->Output("\n", -1);
			return;
		}

		char szNumber[32];
		// Print (filename):(line number): (message).
		v8::String::Utf8Value filename(m_pIsolate, message->GetScriptResourceName());
		sprintf(szNumber, "%d", message->GetLineNumber(context).ToChecked());
		m_pScript->Output(*filename, -1);
		m_pScript->Output(":", -1);
		m_pScript->Output(szNumber, -1);
		m_pScript->Output("\n\t", -1);
		m_pScript->Output(*exception, -1);
		m_pScript->Output("\n", -1);

		// Print line of source code.
		auto souceline = message->GetSourceLine(context).ToLocalChecked();
		m_pScript->Output(*v8::String::Utf8Value(m_pIsolate, souceline), -1);
		m_pScript->Output("\n", -1);

		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn();
		for (int i = 0; i < start; i++)
			m_pScript->Output(" ", -1);
		int end = message->GetEndColumn();
		for (int i = start; i < end; i++)
			m_pScript->Output(" ^", -1);
		m_pScript->Output("\n", -1);
		auto backTrace = try_catch->StackTrace(context);
		if (backTrace.IsEmpty())
			return;
		v8::Local<v8::Value> strBackTrace = backTrace.ToLocalChecked();
		v8::String::Utf8Value stack_trace(m_pIsolate, strBackTrace->ToString(m_pIsolate));
		if (!stack_trace.length())
			return;
		m_pScript->Output(*stack_trace, -1);
	}

	void SV8Context::Log( const v8::FunctionCallbackInfo<v8::Value>& args )
	{
		v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast( args.Data() );
		CScriptJS* pScript = (CScriptJS*)wrap->Value();
		v8::Isolate* isolate = args.GetIsolate();
		v8::HandleScope scope( isolate );
		for( int32 i = 0; i < args.Length(); i++ )
		{
			v8::Local<v8::Value> arg = args[i];
			v8::String::Utf8Value value( isolate, arg );
			const char* szValue = *value;
			pScript->Output( szValue, -1 );
			pScript->Output( " ", -1 );
		}
		pScript->Output( "\n", -1 );
	}

	void SV8Context::Break( const v8::FunctionCallbackInfo<v8::Value>& args )
	{
		v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(args.Data());
		CScriptJS* pScript = (CScriptJS*)wrap->Value();
		( (CDebugJS*)( pScript->GetDebugger() ) )->Stop();
	}

	void SV8Context::Callback( const v8::FunctionCallbackInfo<v8::Value>& args )
	{
		v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast( args.Data() );
		SCallInfo* pCallInfo = (SCallInfo*)wrap->Value();
		if( !pCallInfo )
			return;
		v8::Isolate* isolate = args.GetIsolate();
		v8::HandleScope scope( isolate );
		CByScriptJS::CallByJS( *pCallInfo->m_pScript, pCallInfo->m_pCallBase, args );
	}

	void SV8Context::NewObject( const v8::FunctionCallbackInfo<v8::Value>& args )
	{
		SClassInfo* pInfo = (SClassInfo*)v8::External::Cast( *args.Data() )->Value();
		if( pInfo == NULL )
			return;

		CScriptJS& Script = *static_cast<CScriptJS*>( pInfo->m_pScript );
		SV8Context& Context = Script.GetV8Context();
		v8::Isolate* isolate = Context.m_pIsolate;
		v8::Local<v8::Object> ScriptObj = args.This();
		v8::External* pCppBind = NULL;
		if( ScriptObj->InternalFieldCount() )
			pCppBind = v8::External::Cast( *ScriptObj->GetInternalField( 0 ) );
		else
		{
			v8::Local<v8::Value> key = Context.m_CppField.Get( isolate );
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::MaybeLocal<v8::Value> field = ScriptObj->Get( context, key );
			pCppBind = v8::External::Cast( *( field.ToLocalChecked() ) );
		}
		if( pCppBind->Value() )
		{
			args.GetReturnValue().Set( ScriptObj );
			return;
		}

		Context.BindObj( NULL, args.This(), pInfo->m_pClassInfo );
	}

	void SV8Context::Destruction( const v8::FunctionCallbackInfo<v8::Value>& args )
	{
		v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast( args.Data() );
		SClassInfo* pClassInfo = (SClassInfo*)wrap->Value();
		CScriptJS& Script = *(CScriptJS*)pClassInfo->m_pScript;
		SV8Context& Context = Script.GetV8Context();
		v8::Isolate* isolate = Context.m_pIsolate;
		v8::Object* pScriptObject = v8::Object::Cast( *args.This() );
		v8::External* pCppBind = NULL;
		if( pScriptObject->InternalFieldCount() )
			pCppBind = v8::External::Cast( *pScriptObject->GetInternalField( 0 ) );
		else
		{
			v8::Local<v8::Value> key = Context.m_CppField.Get( isolate );
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::MaybeLocal<v8::Value> field = pScriptObject->Get( context, key );
			pCppBind = v8::External::Cast( *( field.ToLocalChecked() ) );
		}

		SObjInfo* pObjectInfo = (SObjInfo*)pCppBind->Value();
		if( !pObjectInfo )
			return;
		Context.UnbindObj( pObjectInfo, false );
	}

	void SV8Context::GCCallback( const v8::WeakCallbackInfo<SObjInfo>& data )
	{
		SObjInfo* pObjectInfo = data.GetParameter();
		SClassInfo* pInfo = pObjectInfo->m_pClassInfo;
		pInfo->m_pScript->GetV8Context().UnbindObj( pObjectInfo, true );
	}

	void SV8Context::GetterCallback( v8::Local<v8::Name> property, 
		const v8::PropertyCallbackInfo<v8::Value>& info )
	{
		v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast( info.Data() );
		SCallInfo* pCallInfo = (SCallInfo*)wrap->Value();
		if( !pCallInfo )
			return;
		v8::Isolate* isolate = info.GetIsolate();
		v8::HandleScope scope( isolate );
		CByScriptJS::GetByJS( *pCallInfo->m_pScript,
			pCallInfo->m_pCallBase, info.This(), info.GetReturnValue() );
	}

	void SV8Context::SetterCallback( v8::Local<v8::Name> property, 
		LocalValue value, const v8::PropertyCallbackInfo<void>& info )
	{
		v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast( info.Data() );
		SCallInfo* pCallInfo = (SCallInfo*)wrap->Value();
		if( !pCallInfo )
			return;
		v8::Isolate* isolate = info.GetIsolate();
		v8::HandleScope scope( isolate );
		CByScriptJS::SetByJS( *pCallInfo->m_pScript,
			pCallInfo->m_pCallBase, info.This(), value );
	}

	void SV8Context::BindObj( void* pObject, v8::Local<v8::Object> ScriptObj, 
		const CClassRegistInfo* pInfo, void* pSrc )
	{
		SObjInfo& ObjectInfo = *m_pScript->AllocObjectInfo();
		ObjectInfo.m_bRecycle = false;
		if( !pObject )
		{
			pObject = new tbyte[pInfo->GetClassSize()];
			pInfo->Create( m_pScript, pObject );
			if( pSrc )
				pInfo->Assign( m_pScript, pObject, pSrc );
			ObjectInfo.m_bRecycle = true;
		}

		ObjectInfo.m_Object.Reset( m_pIsolate, ScriptObj );
		ObjectInfo.m_pClassInfo = m_pScript->m_mapClassInfo.Find( (const void*)pInfo );
		ObjectInfo.m_pObject = pObject;
		m_pScript->m_mapObjInfo.Insert( ObjectInfo );

		// 注册回调函数
		if( pInfo->IsCallBack() )
			pInfo->ReplaceVirtualTable( m_pScript, pObject, ObjectInfo.m_bRecycle, 0 );

		v8::Local<v8::External> cppInfo = v8::External::New( m_pIsolate, &ObjectInfo );
		if( ScriptObj->InternalFieldCount() )
			ScriptObj->SetInternalField( 0, cppInfo );
		else
			ScriptObj->Set( m_pIsolate->GetCurrentContext(), m_CppField.Get( m_pIsolate ), cppInfo );
		ObjectInfo.m_Object.SetWeak( &ObjectInfo, 
			&SV8Context::GCCallback, v8::WeakCallbackType::kParameter );
	}

	void SV8Context::UnbindObj( SObjInfo* pObjectInfo, bool bFromGC )
	{
		void* pObject = pObjectInfo->m_pObject;
		bool bRecycle = pObjectInfo->m_bRecycle;
		const CClassRegistInfo* pInfo = pObjectInfo->m_pClassInfo->m_pClassInfo;
		v8::HandleScope handle_scope( m_pIsolate );
		v8::TryCatch try_catch( m_pIsolate );

		if( !bFromGC )
		{
			v8::Local<v8::Object> ScriptObj = pObjectInfo->m_Object.Get( m_pIsolate );
			assert( !ScriptObj.IsEmpty()&&ScriptObj->IsObject() );
			if( ScriptObj->InternalFieldCount() )
				ScriptObj->SetInternalField( 0, v8::Null( m_pIsolate ) );
			else
				ScriptObj->Set( m_pIsolate->GetCurrentContext(), 
					m_CppField.Get( m_pIsolate ), v8::Null( m_pIsolate ) );
		}

		pObjectInfo->Remove();
		pObjectInfo->m_pObject = NULL;
		pObjectInfo->m_Object.Reset();

		m_pScript->FreeObjectInfo( pObjectInfo );
		if( !pObject )
			return;
		pInfo->RecoverVirtualTable( m_pScript, pObject );
		if( !bRecycle )
			return;
		pInfo->Release( m_pScript, pObject );
		delete[]( tbyte* )pObject;
	}
}