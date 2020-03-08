#ifndef __V8CONTEXT_H__
#define __V8CONTEXT_H__
#include "v8/v8.h"
#include "common/TRBTree.h"
#include <map>

//=====================================================================
// V8Context.h 
// context for v8 engine
// ¿Â´ïÕÑ
// 2020-01-10
//=====================================================================

namespace XS
{
	class CScriptJS;
	class CCallInfo;
	class CClassInfo;

	typedef v8::Persistent<v8::Context>						PersistentContext;
	typedef v8::Persistent<v8::ObjectTemplate>				PersistentObjTmplt;
	typedef v8::Persistent<v8::FunctionTemplate>			PersistentFunTmplt;
	typedef v8::Persistent<v8::Object>						PersistentObject;
	typedef v8::Persistent<v8::String>						PersistentString;
	typedef v8::Persistent<v8::Function>					PersistentFunction;
	typedef v8::Local<v8::Value>							LocalValue;
	typedef v8::ReturnValue<v8::Value>						ReturnValue;
	typedef std::map<void*, PersistentString>				StringCacheMap;

	struct SJSClassInfo : public TRBTree<SJSClassInfo>::CRBTreeNode
	{
		const CClassInfo*		m_pClassInfo;
		CScriptJS*				m_pScript;
		PersistentFunTmplt		m_FunctionTemplate;
		operator const void*() { return m_pClassInfo; }
		bool operator< (const void* r) { return m_pClassInfo < r; }
	};

	struct SObjInfo : public TRBTree<SObjInfo>::CRBTreeNode
	{
		void*					m_pObject;
		PersistentObject		m_Object;
		SJSClassInfo*			m_pClassInfo;
		bool					m_bRecycle;
		bool					m_bFirstAddress;
		operator void*() { return m_pObject; }
		bool operator< (void* r) { return m_pObject < r; }
	};

	struct SCallInfo : public TRBTree<SCallInfo>::CRBTreeNode
	{
		const CCallInfo*		m_pCallBase;
		CScriptJS*				m_pScript;
		PersistentString		m_strName;
		operator const void*() { return m_pCallBase; }
		bool operator< (const void* r) { return m_pCallBase < r; }
	};

	struct SStringDynamic
	{
		void*					m_pBuffer;
		void*					m_pStack;
	};

	struct SStringFixed
	{
		uint32					m_nLen;
		void*					m_pStack;
	};

	struct SV8Context
	{
		SV8Context( CScriptJS* pScript );

		CScriptJS*					m_pScript;
		uint32						m_nStringID;
		std::vector<SStringDynamic>	m_vecStringInfo;
		tbyte*						m_pTempStrBuffer64K;
		uint32						m_nCurUseSize;
		uint32						m_nStrBufferStack;
		std::wstring				m_szTempUcs2;

		v8::Platform*				m_platform;
		v8::Isolate*				m_pIsolate;
		PersistentContext			m_Context;
		PersistentObject			m_XSNameSpace;
		PersistentFunction			m_XSClass;
		PersistentString			m_CppField;
		PersistentString			m_Prototype;
		PersistentString			m_Deconstruction;
		PersistentString			m___proto__;

		void						MakeMeberFunction(const CClassInfo* pInfo, 
										v8::Local<v8::Function> NewClass, 
										v8::Local<v8::Object> Prototype, bool bBase);
		void						BindObj(void* pObject, v8::Local<v8::Object> ScriptObj, 
										const CClassInfo* pInfo, void* pSrc = NULL );
		void						UnbindObj( SObjInfo* pObjectInfo, bool bFromGC );

		v8::Local<v8::Value>		StringFromUtf8(const char* szUtf8);
		v8::Local<v8::Value>		StringFromUcs(const wchar_t* szUcs);
		const char*					StringToUtf8(v8::Local<v8::Value> obj);
		const wchar_t*				StringToUcs(v8::Local<v8::Value> obj);
		void						ClearCppString(void* pStack);
		void						CallJSStatck(bool bAdd);

		void						ReportException( v8::TryCatch* try_catch, v8::Local<v8::Context> context );

		static void					Log(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void					Break(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void					NewObject(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void					Destruction(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void					GCCallback(const v8::WeakCallbackInfo<SObjInfo>& data);

		static void					CallFromV8(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void					GetterFromV8(v8::Local<v8::Name> property, 
			 							const v8::PropertyCallbackInfo<v8::Value>& info);
		static void					SetterFromV8(v8::Local<v8::Name> property, LocalValue value,
			 							const v8::PropertyCallbackInfo<void>& info);
	};
}
#endif