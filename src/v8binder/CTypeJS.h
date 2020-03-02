/**@file  		CTypeJS.h
* @brief		Data interface between V8&c++
* @author		Daphnis Kaw
* @date			2020-01-17
* @version		V1.0
*/

#ifndef __TYPE_JS_H__
#define __TYPE_JS_H__
#include "common/Memory.h"
#include "core/CTypeBase.h"
#include "V8Context.h"
#include "CScriptJS.h"

namespace XS
{
	#define MAX_UNKNOW_ARRAYBUFFER_SIZE	(100*1024*1024)

	class CJSTypeBase;
	//=====================================================================
	/// aux function
	//=====================================================================
	CJSTypeBase* GetJSTypeBase( DataType eType );
	
	//=====================================================================
    /// Base class of data type
    //=====================================================================
    class CJSTypeBase : public CTypeBase
    {
	public:
		CJSTypeBase() {}
		virtual void FromVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj) = 0;
		virtual LocalValue ToVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf) = 0;
    };
	
	//=====================================================================
	/// Common class of data type
	//=====================================================================
	template<typename T>
	class TJSValue : public CJSTypeBase
	{
	public:		
		void FromVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf, LocalValue obj)
		{
			v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::MaybeLocal<v8::Int32> v = obj->ToInt32(context);
			*(T*)(pDataBuf) = (T)( v.IsEmpty() ? 0 : v.ToLocalChecked()->Value() );
		}

		LocalValue ToVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf)
		{
			return v8::Int32::New(Script.GetV8Context().m_pIsolate, (int32)*(T*)(pDataBuf));
		}

		static TJSValue<T>& GetInst()
		{
			static TJSValue<T> s_Instance;
			return s_Instance;
		}
	};

	// POD type class specialization
	template<> inline void TJSValue<double>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Number> v = obj->ToNumber(context);
		*(double*)(pDataBuf) = v.IsEmpty() ? 0 : v.ToLocalChecked()->Value();
	}

	template<> inline LocalValue TJSValue<double>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return v8::Number::New(Script.GetV8Context().m_pIsolate, *(double*)(pDataBuf));
	}

	template<> inline void TJSValue<float>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Number> v = obj->ToNumber(context);
		*(float*)(pDataBuf) = (float)( v.IsEmpty() ? 0 : v.ToLocalChecked()->Value() );
	}

	template<> inline LocalValue TJSValue<float>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return v8::Number::New(Script.GetV8Context().m_pIsolate, *(float*)(pDataBuf));
	}

	template<> inline void TJSValue<uint64>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Number> v = obj->ToNumber(context);
		*(uint64*)(pDataBuf) = (uint64)( v.IsEmpty() ? 0 : v.ToLocalChecked()->Value() );
	}

	template<> inline LocalValue TJSValue<uint64>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		return v8::Number::New(isolate, (double)*(uint64*)(pDataBuf));
	}

	template<> inline void TJSValue<ulong>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Number> v = obj->ToNumber(context);
		*(ulong*)(pDataBuf) = (ulong)( v.IsEmpty() ? 0 : v.ToLocalChecked()->Value() );
	}

	template<> inline LocalValue TJSValue<ulong>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		return v8::Number::New(isolate, (double)*(ulong*)(pDataBuf));
	}

	template<> inline void TJSValue<int64>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Number> v = obj->ToNumber(context);
		*(int64*)(pDataBuf) = (int64)( v.IsEmpty() ? 0 : v.ToLocalChecked()->Value() );
	}

	template<> inline LocalValue TJSValue<int64>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		return v8::Number::New(isolate, (double)*(int64*)(pDataBuf));
	}

	template<> inline void TJSValue<long>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Number> v = obj->ToNumber(context);
		*(long*)(pDataBuf) = (long)( v.IsEmpty() ? 0 : v.ToLocalChecked()->Value());
	}

	template<> inline LocalValue TJSValue<long>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		return v8::Number::New(isolate, *(long*)(pDataBuf));
	}

	template<> inline void TJSValue<uint32>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Uint32> v = obj->ToUint32(context);
		*(uint32*)(pDataBuf) = v.IsEmpty() ? 0 : v.ToLocalChecked()->Value();
	}

	template<> inline LocalValue TJSValue<uint32>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		return v8::Uint32::New(isolate, *(uint32*)(pDataBuf));
	}

	template<> inline void TJSValue<bool>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Boolean> v = obj->ToBoolean(context);
		*(bool*)(pDataBuf) = v.IsEmpty() ? 0 : v.ToLocalChecked()->Value();
	}

	template<> inline LocalValue TJSValue<bool>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		v8::Isolate* isolate = Script.GetV8Context().m_pIsolate;
		return v8::Boolean::New(isolate, *(bool*)(pDataBuf));
	}

	template<> inline void TJSValue<void*>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{		
		if (obj->IsArrayBufferView())
		{
			v8::ArrayBufferView* view = v8::ArrayBufferView::Cast(*obj);
			char* data = (char*)view->Buffer()->GetContents().Data();
			*(void**)(pDataBuf) = data + view->ByteOffset();
		}
		else if (obj->IsArrayBuffer())
			*(void**)(pDataBuf) = v8::ArrayBuffer::Cast(*obj)->GetContents().Data();
		else
			*(void**)(pDataBuf) = NULL;
	}

	template<> inline LocalValue TJSValue<void*>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return v8::ArrayBuffer::New(Script.GetV8Context().m_pIsolate,
			*(void**)(pDataBuf), MAX_UNKNOW_ARRAYBUFFER_SIZE);
	}

	template<> inline void TJSValue<const char*>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		*(const char**)(pDataBuf) = Script.GetV8Context().StringToUtf8( obj );
	}

	template<> inline LocalValue TJSValue<const char*>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return Script.GetV8Context().StringFromUtf8( *(const char**)(pDataBuf) );
	}

	template<> inline void TJSValue<const wchar_t*>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		*(const wchar_t**)(pDataBuf) = Script.GetV8Context().StringToUcs( obj );
	}

	template<> inline LocalValue TJSValue<const wchar_t*>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return Script.GetV8Context().StringFromUcs(*(const wchar_t**)(pDataBuf));
	}

	//=====================================================================
	/// Interface of class pointer
	//=====================================================================
	class CJSObject : public TJSValue<void*>
	{
	protected:
		const CClassInfo* _FromVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj);
		LocalValue _ToVMValue(DataType eType, 
			CScriptJS& Script, void* pObj, bool bCopy);
	public:
		CJSObject();

		static CJSObject& GetInst();
		virtual void FromVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj);
		virtual LocalValue ToVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf);
	};

	//=====================================================================
	/// Interface of class value
	//=====================================================================
	class CJSValueObject : public CJSObject
	{
	public:
		CJSValueObject();
		static CJSValueObject& GetInst();
		virtual void FromVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj);
		virtual LocalValue ToVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf);
	};
}

#endif
