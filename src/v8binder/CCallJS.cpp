#include "CCallJS.h"
#include "CTypeJS.h"
#include "CScriptJS.h"
#include "common/Help.h"

#include <sstream>
#include <iostream>

#ifdef _WIN32
#include <excpt.h>
#elif( defined _ANDROID )
#include <alloca.h>
#endif

namespace Gamma
{
	CJSTypeBase* s_aryJSType[eDT_count] =
	{
		0,
		&CJSChar::GetInst(),
		&CJSInt8::GetInst(),
		&CJSInt16::GetInst(),
		&CJSInt32::GetInst(),
		&CJSInt64::GetInst(),
		&CJSLong::GetInst(),
		&CJSUint8::GetInst(),
		&CJSUint16::GetInst(),
		&CJSUint32::GetInst(),
		&CJSUint64::GetInst(),
		&CJSUlong::GetInst(),
		&CJSWChar::GetInst(),
		&CJSBool::GetInst(),
		&CJSFloat::GetInst(),
		&CJSDouble::GetInst(),
		&CJSString::GetInst(),
		&CJSWString::GetInst(),
		&CJSPointer::GetInst()
	};

	//=====================================================================
	// JS脚本调用C++的接口
	//=====================================================================
	void CByScriptJS::CallByJS(CScriptJS& Script, const CByScriptBase* pCallBase,
		const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		try
		{
			auto& listParam = pCallBase->GetParamList();
			size_t nParamCount = listParam.size();
			const DataType* aryParam = nParamCount ? &listParam[0] : nullptr;
			size_t* aryParamSize = (size_t*)alloca( sizeof( size_t )*nParamCount );
			size_t nParamSize = CalBufferSize(aryParam, nParamCount, aryParamSize );
			DataType nResultType = pCallBase->GetResultType();
			size_t nReturnSize = nResultType ? GetAligenSizeOfType( nResultType ) : sizeof( int64 );
			size_t nArgSize = pCallBase->GetParamCount()*sizeof( void* );
			char* pDataBuf = (char*)alloca( nParamSize + nReturnSize + nArgSize );
			char* pResultBuf = pDataBuf + nParamSize;
			void** pArgArray = (void**)( pResultBuf + nReturnSize );

			int32 nFunctionIndex = pCallBase->GetFunctionIndex();
			int32 nArgCount = args.Length();
			SV8Context& Context = Script.GetV8Context();
			v8::Isolate* isolate = Context.m_pIsolate;
			LocalValue undefined = Undefined(isolate);
			int32 nArgIndex = nFunctionIndex >= eCT_ClassFunction ? -1 : 0;
			for( uint32 nParamIndex = 0; nParamIndex < nParamCount; nParamIndex++, nArgIndex++ )
			{
				DataType nType = aryParam[nParamIndex];
				CJSTypeBase* pParamType = GetTypeBase( nType );
				LocalValue arg = undefined;
				if( nArgIndex < 0 )
					pParamType->FromVMValue( nType, Script, pDataBuf, args.This() );
				else if( nArgIndex < nArgCount )
					pParamType->FromVMValue( nType, Script, pDataBuf, args[nArgIndex] );
				else
					pParamType->FromVMValue( nType, Script, pDataBuf, undefined );
				pArgArray[nParamIndex] = pDataBuf;
				pDataBuf += aryParamSize[nParamIndex];
			}

			pCallBase->Call( pResultBuf, pArgArray, Script );
			if (!nResultType )
				return;
			CJSTypeBase* pReturnType = GetTypeBase( nResultType );
			args.GetReturnValue().Set( pReturnType->ToVMValue( nResultType, Script, pResultBuf ) );
		}
		catch (...)
		{
		}
	}

	void CByScriptJS::GetByJS(CScriptJS& Script, const CByScriptBase* pByScript,
		LocalValue This, v8::ReturnValue<v8::Value> ret)
	{
		try
		{
			void* pObject = NULL;
			DataType nThisType = pByScript->GetParamList()[0];
			CJSObject::GetInst().FromVMValue( nThisType, Script, (char*)&pObject, This);
			if (pObject == NULL)
				return;

			DataType nResultType = pByScript->GetResultType();
			size_t nReturnSize = nResultType ? GetAligenSizeOfType( nResultType ) : sizeof( int64 );
			char* pResultBuf = (char*)alloca( nReturnSize);
			void* aryArg[] = { &pObject, nullptr };
			pByScript->Call( pResultBuf, aryArg, Script );
			if (!nResultType )
				return;
			CJSTypeBase* pReturnType = GetTypeBase( nResultType );
			ret.Set( pReturnType->ToVMValue( nResultType, Script, pResultBuf ) );
		}
		catch (...)
		{
		}
	}

	void CByScriptJS::SetByJS( CScriptJS& Script, const CByScriptBase* pByScript,
		LocalValue This, LocalValue arg)
	{
		try
		{
			void* pObject = NULL;
			DataType nThisType = pByScript->GetParamList()[0];
			CJSObject::GetInst().FromVMValue( nThisType, Script, (char*)&pObject, This );
			if (pObject == NULL)
				return;

			DataType nType = pByScript->GetParamList()[1];
			size_t nParamSize = GetAligenSizeOfType( nType );
			char* pDataBuf = (char*)alloca( nParamSize );
			SV8Context& Context = Script.GetV8Context();
			v8::Isolate* isolate = Context.m_pIsolate;
			LocalValue undefined = Undefined(isolate);
			CJSTypeBase* pParamType = GetTypeBase( nType );
			pParamType->FromVMValue(nType, Script, pDataBuf, arg);
			void* aryArg[] = { &pObject, pDataBuf, nullptr };
			pByScript->Call( NULL, aryArg, Script );
		}
		catch (...)
		{
		}
	}
	
	//=====================================================================
	// C++调用JS脚本的接口
	//=====================================================================
	bool CCallBackJS::CallVM( CScriptJS& Script, v8::Persistent<v8::String>& strName,
		const CCallScriptBase* pCallBase, void* pRetBuf, void** pArgArray )
	{
		SV8Context& Context = Script.GetV8Context();
		Context.CallJSStatck(true);

		void* pObject = *(void**)*pArgArray++;
		v8::Isolate* isolate = Context.m_pIsolate;
		v8::HandleScope handle_scope(isolate);
		v8::TryCatch try_catch(isolate);

		// Enter the context for compiling and running the hello world script.
		v8::Local<v8::Context> context = Context.m_Context.Get(isolate);
		v8::Context::Scope context_scope(context);

		DataType nThisType = pCallBase->GetParamList()[0];
		LocalValue pThis = CJSObject::GetInst().ToVMValue( nThisType, Script, (char*)&pObject);
		v8::Local<v8::Object> object = pThis->ToObject(isolate);

		int32 nParamCount = (int32)pCallBase->GetParamCount() - 1;
		const DataType* aryParam = nParamCount ? &( pCallBase->GetParamList()[1] ) : nullptr;
		size_t nTotalSize = sizeof( LocalValue )*nParamCount;
		LocalValue* args = ( LocalValue* )alloca( nTotalSize );
		for( int32 nArgIndex = 0; nArgIndex < nParamCount; nArgIndex++ )
		{
			new ( args + nArgIndex ) LocalValue;
			DataType nType = aryParam[nArgIndex];
			CJSTypeBase* pParamType = GetTypeBase( nType );
			args[nArgIndex] = pParamType->ToVMValue( nType, Script, (char*)pArgArray[nArgIndex] );
		}
 
 		v8::MaybeLocal<v8::Value> fun = object->Get(context, strName.Get(isolate));
		if (fun.IsEmpty())
		{
			for( int32 nArgIndex = 0; nArgIndex < nParamCount; nArgIndex++ )
				args[nArgIndex].~LocalValue();				
			Context.CallJSStatck(false);
			return false;
		}

		LocalValue funField = fun.ToLocalChecked();
		if (!funField->IsFunction())
		{
			for( int32 nArgIndex = 0; nArgIndex < nParamCount; nArgIndex++ )
				args[nArgIndex].~LocalValue();
			Context.CallJSStatck(false);
			return false;
		}

		v8::Local<v8::Function> funCallback = v8::Local<v8::Function>::Cast(funField);
		LocalValue result = funCallback->Call( object, nParamCount, args );
		for( int32 nArgIndex = 0; nArgIndex < nParamCount; nArgIndex++ )
			args[nArgIndex].~LocalValue();

		Context.CallJSStatck(false);
		if (result.IsEmpty())
		{
			Context.ReportException(&try_catch, context);
			return false;
		}

		DataType nResultType = pCallBase->GetResultType();
 		if ( nResultType )
			GetTypeBase( nResultType )->FromVMValue( nResultType, Script, (char*)pRetBuf, result);
		return true;
	}

	void CCallBackJS::DestrucVM( CScriptJS& Script, v8::Persistent<v8::String>& strName,
		const CCallScriptBase* pCallBase, SVirtualObj* pObject )
	{
		SV8Context& Context = Script.GetV8Context();
		Context.CallJSStatck(true);

		v8::Isolate* isolate = Context.m_pIsolate;
		v8::HandleScope handle_scope(isolate);
		v8::TryCatch try_catch(isolate);

		// Enter the context for compiling and running the hello world script.
		v8::Local<v8::Context> context = Context.m_Context.Get(isolate);
		v8::Context::Scope context_scope(context);

		DataType nThisType = pCallBase->GetParamList()[0];
		LocalValue pThis = CJSObject::GetInst().ToVMValue( nThisType, Script, (char*)&pObject );
		v8::Local<v8::Object> object = pThis->ToObject( isolate );
		v8::MaybeLocal<v8::Value> fun = object->Get(context, strName.Get(isolate));
		if (fun.IsEmpty())
			return Context.CallJSStatck(true);
		LocalValue funField = fun.ToLocalChecked();
		if (!funField->IsFunction())
			return Context.CallJSStatck(true);
		v8::Local<v8::Function> funCallback = v8::Local<v8::Function>::Cast(funField);
		LocalValue result = funCallback->Call(object, 0, &result);
		Context.CallJSStatck(false);
		if (!result.IsEmpty())
			return;
		Context.ReportException(&try_catch, context);
	}
};