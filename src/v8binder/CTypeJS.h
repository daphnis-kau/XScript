#ifndef __TYPE_JS_H__
#define __TYPE_JS_H__
//=====================================================================
// CTypeJS.h
// 定义JS的数据类型接口
// 柯达昭
// 2007-10-16
//=====================================================================

#include "common/Memory.h"
#include "core/CTypeBase.h"
#include "V8Context.h"
#include "CScriptJS.h"

namespace XS
{
	#define MAX_UNKNOW_ARRAYBUFFER_SIZE	(100*1024*1024)

	//=====================================================================
    /// lua对C++数据的操作方法，调用JS库实现对数据在lua中的操作
    //=====================================================================
    class CJSTypeBase
    {
	public:
		CJSTypeBase() {}
		virtual void FromVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj) = 0;
		virtual LocalValue ToVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf) = 0;
    };
	
	//=====================================================================
	/// C++内置类型在JS中的操作
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

	//特化部分函数
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

	typedef TJSValue<int64>				CJSInt64;
	typedef TJSValue<int32>				CJSInt32;
	typedef TJSValue<int16>				CJSInt16;
	typedef TJSValue<int8>				CJSInt8;
	typedef TJSValue<char>				CJSChar;
	typedef TJSValue<long>				CJSLong;
	typedef TJSValue<ulong>				CJSUlong;
	typedef TJSValue<uint64>			CJSUint64;
	typedef TJSValue<uint32>			CJSUint32;
	typedef TJSValue<uint16>			CJSUint16;
	typedef TJSValue<uint8>				CJSUint8;
	typedef TJSValue<wchar_t>			CJSWChar;
	typedef TJSValue<float>				CJSFloat;
	typedef TJSValue<double>			CJSDouble;
	typedef TJSValue<bool>				CJSBool;
	typedef TJSValue<void*>				CJSPointer;
	typedef TJSValue<const char*>		CJSString;
	typedef TJSValue<const wchar_t*>	CJSWString;

	//=====================================================================
	/// C++对象以JS形式的表示及其操作，必须以CClassRegistInfo* 构造
	//=====================================================================
	class CJSObject : public CJSPointer
	{
	protected:
		const CClassRegistInfo* _FromVMValue(DataType eType, 
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
	/// lua数据类型之C++对象类型，必须以CClassRegistInfo* 构造
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
