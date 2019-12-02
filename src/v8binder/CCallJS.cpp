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
	//=====================================================================
	// JS脚本调用C++的接口
	//=====================================================================
	void CByScriptJS::CallByJS(CScriptJS& Script, CByScriptBase* pCallBase, 
		const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		try
		{
			const vector<DataType>& listParam = pCallBase->GetParamList();
			size_t nParamCount = listParam.size();
			size_t* aryParamSize = (size_t*)alloca( sizeof( size_t )*nParamCount );
			size_t nParamSize = CalBufferSize( listParam, aryParamSize );
			DataType nResultType = pCallBase->GetResultType();
			size_t nReturnSize = nResultType ? GetAligenSizeOfType( nResultType ) : sizeof( int64 );
			size_t nArgSize = pCallBase->GetParamCount()*sizeof( void* );
			char* pDataBuf = (char*)alloca( nParamSize + nReturnSize + nArgSize );
			char* pResultBuf = pDataBuf + nParamSize;
			void** pArgArray = (void**)( pResultBuf + nReturnSize );
			void* pObject = NULL;

			DataType nThisType = pCallBase->GetThisType();
			if( nThisType )
			{
				CJSObject::GetInst().FromVMValue( nThisType, Script, (char*)&pObject, args.This() );
				assert( pObject );
			}

			uint32 nArgCount = args.Length();
			LocalValue undefined = Undefined( Script.GetIsolate() );
			for( uint32 nArgIndex = 0; nArgIndex < listParam.size(); nArgIndex++ )
			{
				DataType nType = listParam[nArgIndex];
				CJSTypeBase* pParamType = GetTypeBase( nType );
				LocalValue arg = nArgIndex >= nArgCount ? undefined : args[nArgIndex];
				pParamType->FromVMValue( nType, Script, pDataBuf, arg );
				pArgArray[nArgIndex] = pDataBuf;
				pDataBuf += aryParamSize[nArgIndex];
			}

			pCallBase->Call(pObject, pResultBuf, pArgArray, Script);
			Script.CheckUnlinkCppObj();
			if (!nResultType )
				return;
			CJSTypeBase* pReturnType = GetTypeBase( nResultType );
			args.GetReturnValue().Set( pReturnType->ToVMValue( nResultType, Script, pResultBuf ) );
		}
		catch (...)
		{
		}
	}

	void CByScriptJS::GetByJS(CScriptJS& Script, CByScriptBase* pByScript,
		LocalValue This, v8::ReturnValue<v8::Value> ret)
	{
		try
		{
			void* pObject = NULL;
			DataType nThisType = pByScript->GetThisType();
			CJSObject::GetInst().FromVMValue( nThisType, Script, (char*)&pObject, This);
			if (pObject == NULL)
				return;

			DataType nResultType = pByScript->GetResultType();
			size_t nReturnSize = nResultType ? GetAligenSizeOfType( nResultType ) : sizeof( int64 );
			char* pResultBuf = (char*)alloca( nReturnSize);
			pByScript->Call(pObject, pResultBuf, NULL, Script);
			Script.CheckUnlinkCppObj();
			if (!nResultType )
				return;
			CJSTypeBase* pReturnType = GetTypeBase( nResultType );
			ret.Set( pReturnType->ToVMValue( nResultType, Script, pResultBuf ) );
		}
		catch (...)
		{
		}
	}

	void CByScriptJS::SetByJS( CScriptJS& Script, CByScriptBase* pByScript,
		LocalValue This, LocalValue arg)
	{
		try
		{
			void* pObject = NULL;
			DataType nThisType = pByScript->GetThisType();
			CJSObject::GetInst().FromVMValue( nThisType, Script, (char*)&pObject, This );
			if (pObject == NULL)
				return;

			DataType nType = pByScript->GetParamList()[0];
			size_t nParamSize = GetAligenSizeOfType( nType );
			char* pDataBuf = (char*)alloca( nParamSize );
			LocalValue undefined = Undefined( Script.GetIsolate() );
			CJSTypeBase* pParamType = GetTypeBase( nType );
			pParamType->FromVMValue(nType, Script, pDataBuf, arg);
			void* aryArg[] = { pDataBuf };
			pByScript->Call(pObject, NULL, aryArg, Script);
			Script.CheckUnlinkCppObj();
		}
		catch (...)
		{
		}
	}
	
	//=====================================================================
	// C++调用JS脚本的接口
	//=====================================================================
	bool CCallBackJS::CallVM( CScriptJS& Script, v8::Persistent<v8::String>& strName,
		CCallScriptBase* pCallBase, SVirtualObj* pObject, void* pRetBuf, void** pArgArray )
	{
		Script.CallJSStatck(true);

		v8::Isolate* isolate = Script.GetIsolate();
		v8::HandleScope handle_scope(isolate);
		v8::TryCatch try_catch(isolate);

		// Enter the context for compiling and running the hello world script.
		v8::Local<v8::Context> context = Script.GetContext().Get(isolate);
		v8::Context::Scope context_scope(context);

		DataType nThisType = pCallBase->GetThisType();
		LocalValue pThis = CJSObject::GetInst().ToVMValue( nThisType, Script, (char*)&pObject);
		v8::Local<v8::Object> object = pThis->ToObject(isolate);

		const vector<DataType>& listParam = pCallBase->GetParamList();
		int32 nParamCount = (int32)listParam.size();
		size_t nTotalSize = sizeof( LocalValue )*nParamCount;
		LocalValue* args = ( LocalValue* )alloca( sizeof( nTotalSize ) );
		for( int32 nArgIndex = 0; nArgIndex < nParamCount; nArgIndex++ )
		{
			new ( args + nArgIndex ) LocalValue;
			DataType nType = listParam[nArgIndex];
			CJSTypeBase* pParamType = GetTypeBase( nType );
			args[nArgIndex] = pParamType->ToVMValue( nType, Script, (char*)pArgArray[nArgIndex] );
		}
 
 		v8::MaybeLocal<v8::Value> fun = object->Get(context, strName.Get(isolate));
		if (fun.IsEmpty())
		{
			for( int32 nArgIndex = 0; nArgIndex < nParamCount; nArgIndex++ )
				args[nArgIndex].~LocalValue();				
			Script.CallJSStatck(false);
			return false;
		}

		LocalValue funField = fun.ToLocalChecked();
		if (!funField->IsFunction())
		{
			for( int32 nArgIndex = 0; nArgIndex < nParamCount; nArgIndex++ )
				args[nArgIndex].~LocalValue();
			Script.CallJSStatck(false);
			return false;
		}

		v8::Local<v8::Function> funCallback = v8::Local<v8::Function>::Cast(funField);
		LocalValue result = funCallback->Call( object, nParamCount, args );
		for( int32 nArgIndex = 0; nArgIndex < nParamCount; nArgIndex++ )
			args[nArgIndex].~LocalValue();

		Script.CallJSStatck(false);
		if (result.IsEmpty())
		{
			Script.ReportException(&try_catch, context);
			return false;
		}

		DataType nResultType = pCallBase->GetResultType();
 		if ( nResultType )
			GetTypeBase( nResultType )->FromVMValue( nResultType, Script, (char*)pRetBuf, result);
		return true;
	}

	void CCallBackJS::DestrucVM( CScriptJS& Script, v8::Persistent<v8::String>& strName,
		CCallScriptBase* pCallBase, SVirtualObj* pObject )
	{
		Script.CallJSStatck(true);
		v8::Isolate* isolate = Script.GetIsolate();
		v8::HandleScope handle_scope(isolate);
		v8::TryCatch try_catch(isolate);

		// Enter the context for compiling and running the hello world script.
		v8::Local<v8::Context> context = Script.GetContext().Get(isolate);
		v8::Context::Scope context_scope(context);

		DataType nThisType = pCallBase->GetThisType();
		LocalValue pThis = CJSObject::GetInst().ToVMValue( nThisType, Script, (char*)&pObject );
		v8::Local<v8::Object> object = pThis->ToObject( isolate );
		v8::MaybeLocal<v8::Value> fun = object->Get(context, strName.Get(isolate));
		if (fun.IsEmpty())
			return Script.CallJSStatck(true);
		LocalValue funField = fun.ToLocalChecked();
		if (!funField->IsFunction())
			return Script.CallJSStatck(true);
		v8::Local<v8::Function> funCallback = v8::Local<v8::Function>::Cast(funField);
		LocalValue result = funCallback->Call(object, 0, &result);
		Script.CallJSStatck(false);
		if (!result.IsEmpty())
			return;
		Script.ReportException(&try_catch, context);
	}
};