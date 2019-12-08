#ifndef __TYPE_JS_H__
#define __TYPE_JS_H__
//=====================================================================
// CTypeJS.h
// 定义JS的数据类型接口
// 柯达昭
// 2007-10-16
//=====================================================================

#include "common/Memory.h"
#include "common/CodeCvs.h"
#include "core/CTypeBase.h"
#include "CScriptJS.h"

namespace Gamma
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
			v8::Isolate* isolate = Script.GetIsolate();
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::MaybeLocal<v8::Int32> v = obj->ToInt32(context);
			*(T*)(pDataBuf) = (T)( v.IsEmpty() ? 0 : v.ToLocalChecked()->Value() );
		}

		LocalValue ToVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf)
		{
			return v8::Int32::New(Script.GetIsolate(), (int32)*(T*)(pDataBuf));
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
		v8::Isolate* isolate = Script.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Number> v = obj->ToNumber(context);
		*(double*)(pDataBuf) = v.IsEmpty() ? 0 : v.ToLocalChecked()->Value();
	}

	template<> inline LocalValue TJSValue<double>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return v8::Number::New(Script.GetIsolate(), *(double*)(pDataBuf));
	}

	template<> inline void TJSValue<float>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Number> v = obj->ToNumber(context);
		*(float*)(pDataBuf) = (float)( v.IsEmpty() ? 0 : v.ToLocalChecked()->Value() );
	}

	template<> inline LocalValue TJSValue<float>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return v8::Number::New(Script.GetIsolate(), *(float*)(pDataBuf));
	}

	template<> inline void TJSValue<uint64>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Number> v = obj->ToNumber(context);
		*(uint64*)(pDataBuf) = (uint64)( v.IsEmpty() ? 0 : v.ToLocalChecked()->Value() );
	}

	template<> inline LocalValue TJSValue<uint64>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return v8::Number::New(Script.GetIsolate(), (double)*(uint64*)(pDataBuf));
	}

	template<> inline void TJSValue<ulong>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Number> v = obj->ToNumber(context);
		*(ulong*)(pDataBuf) = (ulong)( v.IsEmpty() ? 0 : v.ToLocalChecked()->Value() );
	}

	template<> inline LocalValue TJSValue<ulong>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return v8::Number::New(Script.GetIsolate(), (double)*(ulong*)(pDataBuf));
	}

	template<> inline void TJSValue<int64>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Number> v = obj->ToNumber(context);
		*(int64*)(pDataBuf) = (int64)( v.IsEmpty() ? 0 : v.ToLocalChecked()->Value() );
	}

	template<> inline LocalValue TJSValue<int64>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return v8::Number::New(Script.GetIsolate(), (double)*(int64*)(pDataBuf));
	}

	template<> inline void TJSValue<long>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Number> v = obj->ToNumber(context);
		*(long*)(pDataBuf) = (long)( v.IsEmpty() ? 0 : v.ToLocalChecked()->Value());
	}

	template<> inline LocalValue TJSValue<long>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return v8::Number::New(Script.GetIsolate(), *(long*)(pDataBuf));
	}

	template<> inline void TJSValue<uint32>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Uint32> v = obj->ToUint32(context);
		*(uint32*)(pDataBuf) = v.IsEmpty() ? 0 : v.ToLocalChecked()->Value();
	}

	template<> inline LocalValue TJSValue<uint32>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return v8::Uint32::New(Script.GetIsolate(), *(uint32*)(pDataBuf));
	}

	template<> inline void TJSValue<bool>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		v8::Isolate* isolate = Script.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::MaybeLocal<v8::Boolean> v = obj->ToBoolean(context);
		*(bool*)(pDataBuf) = v.IsEmpty() ? 0 : v.ToLocalChecked()->Value();
	}

	template<> inline LocalValue TJSValue<bool>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return v8::Boolean::New(Script.GetIsolate(), *(bool*)(pDataBuf));
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
		return v8::ArrayBuffer::New(Script.GetIsolate(), 
			*(void**)(pDataBuf), MAX_UNKNOW_ARRAYBUFFER_SIZE);
	}

	template<> inline void TJSValue<const char*>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		*(const char**)(pDataBuf) = Script.StringToUtf8( obj );
	}

	template<> inline LocalValue TJSValue<const char*>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return Script.StringFromUtf8( *(const char**)(pDataBuf) );
	}

	template<> inline void TJSValue<const wchar_t*>::FromVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf, LocalValue obj)
	{
		*(const wchar_t**)(pDataBuf) = Script.StringToUcs( obj );
	}

	template<> inline LocalValue TJSValue<const wchar_t*>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf)
	{
		return Script.StringFromUcs(*(const wchar_t**)(pDataBuf));
	}

	typedef TJSValue<int64>				CJSInt64;
	typedef TJSValue<int32>				CJSInt32;
	typedef TJSValue<int16>				CJSInt16;
	typedef TJSValue<int8>				CJSInt8;
	typedef TJSValue<long>				CJSLong;
	typedef TJSValue<ulong>				CJSUlong;
	typedef TJSValue<uint64>			CJSUint64;
	typedef TJSValue<uint32>			CJSUint32;
	typedef TJSValue<uint16>			CJSUint16;
	typedef TJSValue<uint8>				CJSUint8;
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
	
	//=====================================================================
	/// C++内建对象以JS形式的表示及其操作
	//=====================================================================
	template<class Type> class TArrayType	{ public: typedef v8::ArrayBuffer Array; };
	template<> class TArrayType<int8>		{ public: typedef v8::Int8Array Array; };
	template<> class TArrayType<int16>		{ public: typedef v8::Int16Array Array; };
	template<> class TArrayType<int32>		{ public: typedef v8::Int32Array Array; };
	template<> class TArrayType<uint8>		{ public: typedef v8::Uint8Array Array; };
	template<> class TArrayType<uint16>		{ public: typedef v8::Uint16Array Array; };
	template<> class TArrayType<uint32>		{ public: typedef v8::Uint32Array Array; };
	template<> class TArrayType<float>		{ public: typedef v8::Float32Array Array; };
	template<> class TArrayType<double>		{ public: typedef v8::Float64Array Array; };

	template<class Type, class ImpClass>
	class TBuildinObject : public CJSPointer
	{
		typedef typename TArrayType<Type>::Array ArrayType;
		typedef typename v8::Local<v8::ArrayBuffer> ArrayBuffer;
		const char*				m_szBuildinName;
		PersistentObject		m_ClassPrototype;
	public:
		TBuildinObject( uint32 nSize, const char* szName ) 
			: CJSPointer( sizeof( Type )*nSize ), m_szBuildinName( szName ) 
		{
			m_nType = eDT_class;
			m_nFlag = 0;
		}
		virtual LocalValue ToVMValue(DataType eType, 
			CScriptJS& Script, char* pDataBuf );
	};

	template<class Type, class ImpClass>
	inline LocalValue 
	Gamma::TBuildinObject<Type, ImpClass>::ToVMValue(
		DataType eType, CScriptJS& Script, char* pDataBuf )
	{
		ArrayBuffer ArrBuf = ImpClass::NewBuff( Script, pDataBuf, CJSPointer::m_nSize );
		v8::Local<ArrayType> ArrayObj = ArrayType::New( ArrBuf, 0, CJSPointer::m_nSize/sizeof( Type ) );
		v8::Isolate* isolate = Script.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		if( m_szBuildinName )
		{
			v8::Local<v8::Object> nsGamma = Script.GetGammaNameSpace().Get( isolate );
			v8::Local<v8::String> name = v8::String::NewFromUtf8( isolate, m_szBuildinName );
			v8::Local<v8::Value> JSClassV = nsGamma->Get( context, name ).ToLocalChecked();
			v8::Local<v8::Object> JSClass = v8::Local<v8::Object>::Cast( JSClassV );
			v8::Local<v8::String> PrototypeName = Script.GetPrototype().Get( isolate );
			v8::MaybeLocal<v8::Value> ProtoValue = JSClass->Get( context, PrototypeName );
			m_ClassPrototype.Reset( isolate, ProtoValue.ToLocalChecked()->ToObject( isolate ) );
			m_ClassPrototype.SetWeak();
			m_szBuildinName = NULL;
		}
		v8::Local<v8::String> __proto__ = Script.GetProto().Get( isolate );
		ArrayObj->Set( context, __proto__, m_ClassPrototype.Get( isolate ) );
		return ArrayObj;
	}

	//=====================================================================
	/// C++内建对象以JS形式的表示及其操作---指针类型
	//=====================================================================
	template<class Type>
	class TBuildinPoint : public TBuildinObject<Type, TBuildinPoint<Type>>
	{
	public:
		TBuildinPoint( uint32 nSize, const char* szName )
			: TBuildinObject<Type, TBuildinPoint<Type>>( nSize, szName ) {}
		static v8::Local<v8::ArrayBuffer> NewBuff( 
			CScriptJS& Script, char* pDataBuf, uint32 nSize );
	};

	template<class Type>
	inline v8::Local<v8::ArrayBuffer> 
	Gamma::TBuildinPoint<Type>::NewBuff( 
		CScriptJS& Script, char* pDataBuf, uint32 nSize )
	{
		return v8::ArrayBuffer::New( Script.GetIsolate(), *(void**)( pDataBuf ), nSize );
	}

	//=====================================================================
	/// C++内建对象以JS形式的表示及其操作---值类型
	//=====================================================================
	template<class Type>
	class TBuildinValue : public TBuildinObject<Type, TBuildinValue<Type>>
	{
	public:
		TBuildinValue( uint32 nSize, const char* szName ) 
			: TBuildinObject<Type, TBuildinValue<Type>>( nSize, szName ) {}
		static v8::Local<v8::ArrayBuffer> NewBuff( 
			CScriptJS& Script, char* pDataBuf, uint32 nSize );
		void FromVMValue(DataType eType,  
			CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj );
	};

	template<class Type>
	void Gamma::TBuildinValue<Type>::FromVMValue(
		DataType eType,  CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj )
	{
		if( obj->IsArrayBufferView() )
		{
			v8::ArrayBufferView* view = v8::ArrayBufferView::Cast( *obj );
			char* data = (char*)view->Buffer()->GetContents().Data();
			memcpy( pDataBuf, data + view->ByteOffset(), CJSPointer::m_nSize );
			return;
		}

		if (obj->IsArrayBuffer())
		{
			char* data = v8::ArrayBuffer::Cast(*obj)->GetContents().Data();
			memcpy(pDataBuf, data, CJSPointer::m_nSize);
			return;
		}
		
		memset( pDataBuf, 0, CJSPointer::m_nSize );
	}

	template<class Type>
	inline v8::Local<v8::ArrayBuffer>
		Gamma::TBuildinValue<Type>::NewBuff( 
			CScriptJS& Script, char* pDataBuf, uint32 nSize )
	{
		v8::Local<v8::ArrayBuffer> ArrayBuf =
			v8::ArrayBuffer::New( Script.GetIsolate(), nSize );
		memcpy( ArrayBuf->GetContents().Data(), *(void**)( pDataBuf ), nSize );
		return ArrayBuf;
	}

	template<class Type>
	inline CJSTypeBase* CreateBuildinType( bool bValue, uint32 nSize, const char* szName )
	{
		if( bValue )
			return new Gamma::TBuildinValue<Type>( nSize, szName );
		return new Gamma::TBuildinPoint<Type>( nSize, szName );
	}
}

#endif
