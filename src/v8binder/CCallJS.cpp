#include "CCallJS.h"
#include "CTypeJS.h"

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
	void CByScriptJS::CallByJS(CByScriptBase* pByScript, const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		CScriptJS& Script = *(CScriptJS*)pByScript->GetScript();
		try
		{
			const list<CTypeBase*>& listParam = pByScript->GetParamList();
			CJSTypeBase* pResultType = static_cast<CJSTypeBase*>(pByScript->GetResultType());
			uint32 nParamSize = AligenUp(pByScript->GetParamSize(), sizeof(void*));
			uint32 nReturnSize = AligenUp(pResultType ? pResultType->GetLen() : sizeof(int64), sizeof(void*));
			uint32 nArgSize = AligenUp(pByScript->GetParamCount() * sizeof(void*), sizeof(void*));
			char* pDataBuf = (char*)alloca(nParamSize + nReturnSize + nArgSize);
			char* pResultBuf = pDataBuf + nParamSize;
			void** pArgArray = (void**)(pResultBuf + nReturnSize);
			void* pObject = NULL;

			memset(pDataBuf, 0, pByScript->GetParamSize());
			uint32 nArgIndex = 0;
			uint32 nArgCount = args.Length();
			LocalValue undefined = Undefined(Script.GetIsolate());
			for(list<CTypeBase*>::const_iterator it = listParam.begin(); 
				it != listParam.end(); ++it, ++nArgIndex)
			{
				CJSTypeBase* pParamType = static_cast<CJSTypeBase*>(*it);
				LocalValue arg = nArgIndex >= nArgCount ? undefined : args[nArgIndex];
				pParamType->FromVMValue(Script, pDataBuf, arg);
				pArgArray[nArgIndex] = pDataBuf;
				pDataBuf += AligenUp(pParamType->GetLen(), sizeof(void*));
			}

			if (pByScript->GetThisType())
			{
				CJSTypeBase* pThis = static_cast<CJSTypeBase*>(pByScript->GetThisType());
				pThis->FromVMValue(Script, (char*)&pObject, args.This());
				if (pObject == NULL)
					return;
			}

			pByScript->Call(pObject, pResultBuf, pArgArray);
			if (!pResultType)
				return;
			args.GetReturnValue().Set( pResultType->ToVMValue( Script, pResultBuf ) );
		}
		catch (...)
		{
		}
	}

	void CByScriptJS::GetByJS(CByScriptBase* pByScript, 
		v8::Local<v8::Value> This, v8::ReturnValue<v8::Value> ret)
	{
		CScriptJS& Script = *(CScriptJS*)pByScript->GetScript();
		try
		{
			void* pObject = NULL;
			CJSTypeBase* pThis = static_cast<CJSTypeBase*>(pByScript->GetThisType());
			pThis->FromVMValue(Script, (char*)&pObject, This);
			if (pObject == NULL)
				return;

			CJSTypeBase* pResultType = static_cast<CJSTypeBase*>(pByScript->GetResultType());
			uint32 nReturnSize = AligenUp(pResultType ? pResultType->GetLen() : sizeof(int64), sizeof(void*));
			char* pResultBuf = (char*)alloca( nReturnSize);
			pByScript->Call(pObject, pResultBuf, NULL);
			if (!pResultType)
				return;
			ret.Set( pResultType->ToVMValue( Script, pResultBuf ) );
		}
		catch (...)
		{
		}
	}

	void CByScriptJS::SetByJS(CByScriptBase* pByScript, 
		v8::Local<v8::Value> This, v8::Local<v8::Value> arg)
	{
		CScriptJS& Script = *(CScriptJS*)pByScript->GetScript();
		try
		{
			void* pObject = NULL;
			CJSTypeBase* pThis = static_cast<CJSTypeBase*>(pByScript->GetThisType());
			pThis->FromVMValue(Script, (char*)&pObject, This);
			if (pObject == NULL)
				return;

			const list<CTypeBase*>& listParam = pByScript->GetParamList();
			uint32 nParamSize = AligenUp(pByScript->GetParamSize(), sizeof(void*));
			char* pDataBuf = (char*)alloca( nParamSize );

			memset(pDataBuf, 0, pByScript->GetParamSize());
			LocalValue undefined = Undefined(Script.GetIsolate());
			CJSTypeBase* pParamType = static_cast<CJSTypeBase*>(*listParam.begin());
			pParamType->FromVMValue(Script, pDataBuf, arg);
			void* aryArg[] = { pDataBuf };
			pByScript->Call( pObject, NULL, aryArg );
		}
		catch (...)
		{
		}
	}
	
	//=====================================================================
	// C++调用JS脚本的接口
	//=====================================================================
	CCallBackJS::CCallBackJS(CScriptJS& Script, const STypeInfoArray& aryTypeInfo,
		IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunName)
		: CCallScriptBase(Script, aryTypeInfo, funWrap, szTypeInfoName, szFunName)
	{
		v8::Isolate* isolate = Script.GetIsolate();
		v8::HandleScope handle_scope(isolate);
		szFunName = szFunName && szFunName[0] ? szFunName : "Deconstruction";
		m_strName.Reset(isolate, v8::String::NewFromUtf8(isolate, szFunName));
	}

	bool CCallBackJS::CallVM(SVirtualObj* pObject, void* pRetBuf, void** pArgArray)
	{
		CScriptJS* pScript = static_cast<CScriptJS*>(m_pScript);
		pScript->CallJSStatck(true);

		v8::Isolate* isolate = pScript->GetIsolate();
		v8::HandleScope handle_scope(isolate);
		v8::TryCatch try_catch(isolate);

		// Enter the context for compiling and running the hello world script.
		v8::Local<v8::Context> context = pScript->GetContext().Get(isolate);
		v8::Context::Scope context_scope(context);

		CJSTypeBase* pThisType = static_cast<CJSTypeBase*>(m_pThis);
		LocalValue pThis = pThisType->ToVMValue(*pScript, (char*)&pObject);
		v8::Local<v8::Object> object = pThis->ToObject(isolate);

 		v8::Local<v8::Value> args[256];
 		int32 nArg = 0;
 		for (list<CTypeBase*>::iterator it = m_listParam.begin(); it != m_listParam.end(); ++it, ++nArg)
 			args[nArg] = static_cast<CJSTypeBase*>(*it)->ToVMValue(*pScript, (char*)pArgArray[nArg]);
 
 		v8::MaybeLocal<v8::Value> fun = object->Get(context, m_strName.Get(isolate));
		if (fun.IsEmpty())
		{
			pScript->CallJSStatck(false);
			return false;
		}

		v8::Local<v8::Value> funField = fun.ToLocalChecked();
		if (!funField->IsFunction())
		{
			pScript->CallJSStatck(false);
			return false;
		}

		v8::Local<v8::Function> funCallback = v8::Local<v8::Function>::Cast(funField);
		LocalValue result = funCallback->Call(object, nArg, args);
		pScript->CallJSStatck(false);
		if (result.IsEmpty())
		{
			pScript->ReportException(&try_catch, context);
			return false;
		}
 
 		if (m_pResult)
 			static_cast<CJSTypeBase*>(m_pResult)->FromVMValue(*pScript, (char*)pRetBuf, result);
		return true;
	}

	void CCallBackJS::DestrucVM(SVirtualObj* pObject)
	{
		CScriptJS* pScript = static_cast<CScriptJS*>(m_pScript);
		pScript->CallJSStatck(true);

		v8::Isolate* isolate = pScript->GetIsolate();
		v8::HandleScope handle_scope(isolate);
		v8::TryCatch try_catch(isolate);

		// Enter the context for compiling and running the hello world script.
		v8::Local<v8::Context> context = pScript->GetContext().Get(isolate);
		v8::Context::Scope context_scope(context);

		CJSTypeBase* pThisType = static_cast<CJSTypeBase*>(m_pThis);
		LocalValue pThis = pThisType->ToVMValue(*pScript, (char*)&pObject);
		v8::Local<v8::Object> object = pThis->ToObject(isolate);

		v8::MaybeLocal<v8::Value> fun = object->Get(context, m_strName.Get(isolate));
		if (fun.IsEmpty())
			return pScript->CallJSStatck(true);
		v8::Local<v8::Value> funField = fun.ToLocalChecked();
		if (!funField->IsFunction())
			return pScript->CallJSStatck(true);
		v8::Local<v8::Function> funCallback = v8::Local<v8::Function>::Cast(funField);
		LocalValue result = funCallback->Call(object, 0, &result);
		pScript->CallJSStatck(false);
		if (!result.IsEmpty())
			return;
		pScript->ReportException(&try_catch, context);
	}
};