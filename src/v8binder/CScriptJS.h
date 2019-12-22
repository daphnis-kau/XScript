#ifndef __SCRIPT_JS_H__
#define __SCRIPT_JS_H__
//=====================================================================
// CScriptAS3.h 
// 为lua定义的C++到虚拟机的接口
// 柯达昭
// 2007-10-16
//=====================================================================
#include "common/TRBTree.h"
#include "core/CScriptBase.h"
#include "v8/v8.h"

namespace Gamma
{
	typedef v8::Persistent<v8::Context>						PersistentContext;
	typedef v8::Persistent<v8::ObjectTemplate>				PersistentObjTmplt;
	typedef v8::Persistent<v8::FunctionTemplate>			PersistentFunTmplt;
	typedef v8::Persistent<v8::Object>						PersistentObject;
	typedef v8::Persistent<v8::String>						PersistentString;
	typedef v8::Persistent<v8::Function>					PersistentFunction;
	typedef v8::Local<v8::Value>							LocalValue;
	typedef v8::ReturnValue<v8::Value>						ReturnValue;
	typedef std::map<void*, PersistentString>				StringCacheMap;

    class CScriptJS : public CScriptBase
	{
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

		struct SClassInfo : public TRBTree<SClassInfo>::CRBTreeNode
		{
			const CClassRegistInfo*	m_pClassInfo;
			CScriptJS*				m_pScript;
			PersistentFunTmplt		m_FunctionTemplate;
			operator const void*() { return m_pClassInfo; }
			bool operator< ( const void* r ) { return m_pClassInfo < r; }
		};

		struct SObjInfo : public TRBTree<SObjInfo>::CRBTreeNode
		{
			void*					m_pObject;
			PersistentObject		m_Object;
			SClassInfo*				m_pClassInfo;
			bool					m_bRecycle;
			bool					m_bFirstAddress;
			operator void*() { return m_pObject; }
			bool operator< ( void* r ) { return m_pObject < r; }
		};

		struct SCallInfo : public TRBTree<SCallInfo>::CRBTreeNode
		{
			const CByScriptBase*	m_pCallBase;
			CScriptJS*				m_pScript;
			PersistentString		m_strName;
			operator const void*() { return m_pCallBase; }
			bool operator< ( const void* r ) { return m_pCallBase < r; }
		};

		uint32						m_nStringID;
		v8::Platform*				m_platform;
		v8::Isolate*				m_pIsolate;
		PersistentContext			m_Context;
		PersistentObject			m_GammaNameSpace;
		PersistentFunction			m_GammaClass;
		PersistentString			m_CppField;
		PersistentString			m_Prototype;
		PersistentString			m_Deconstruction;
		PersistentString			m___proto__;

		std::vector<SStringDynamic>	m_vecStringInfo;
		tbyte*						m_pTempStrBuffer64K;
		uint32						m_nCurUseSize;
		uint32						m_nStrBufferStack;
		std::wstring				m_szTempUcs2;

		SObjInfo*					m_pFreeObjectInfo;
		TRBTree<SObjInfo>			m_mapObjInfo;
		TRBTree<SClassInfo>			m_mapClassInfo;
		TRBTree<SCallInfo>			m_mapCallBase;

		static void					Log(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void					Break(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void					Callback(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void					NewObject(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void					Destruction(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void					GCCallback(const v8::WeakCallbackInfo<SObjInfo>& data);
		static void					GetterCallback(v8::Local<v8::Name> property, 
										const v8::PropertyCallbackInfo<v8::Value>& info);
		static void					SetterCallback(v8::Local<v8::Name> property, v8::Local<v8::Value> value, 
										const v8::PropertyCallbackInfo<void>& info);
		
		void						BuildRegisterInfo();
		void						MakeMeberFunction(const CClassRegistInfo* pInfo, 
										v8::Local<v8::Function> NewClass, 
										v8::Local<v8::Object> Prototype, bool bBase);
		
		SCallInfo*					GetCallInfo( CByScriptBase* pCallBase );
		SObjInfo*					AllocObjectInfo();
		void						FreeObjectInfo(SObjInfo* pObjectInfo);
		void						BindObj(void* pObject, v8::Local<v8::Object> ScriptObj, 
										const CClassRegistInfo* pInfo, void* pSrc = NULL );
		void						UnbindObj( SObjInfo* pObjectInfo, bool bFromGC );

		virtual bool				CallVM( CCallScriptBase* pCallBase, SVirtualObj* pObject, 
										void* pRetBuf, void** pArgArray );
		virtual void				DestrucVM( CCallScriptBase* pCallBase, SVirtualObj* pObject );

		friend class CJSObject;
    public:
		CScriptJS( uint16 nDebugPort );
		~CScriptJS(void);

		v8::Platform*				GetPlatform() { return m_platform; }
		v8::Isolate*				GetIsolate() { return m_pIsolate; }
		PersistentContext&			GetContext() { return m_Context; }
		PersistentString&			GetProto() { return m___proto__; }
		PersistentString&			GetPrototype() { return m_Prototype; }
		PersistentObject&			GetGammaNameSpace() { return m_GammaNameSpace; }
		PersistentFunTmplt&			GetPersistentFunTemplate( const CClassRegistInfo* pInfo );

		void						ClearCppString(void* pStack);
		void						CallJSStatck(bool bAdd);
		v8::Local<v8::Value>		StringFromUtf8(const char* szUtf8);
		v8::Local<v8::Value>		StringFromUcs(const wchar_t* szUcs);
		const char*					StringToUtf8(v8::Local<v8::Value> obj);
		const wchar_t*				StringToUcs(v8::Local<v8::Value> obj);
		void						ReportException( v8::TryCatch* try_catch, v8::Local<v8::Context> context );
		SObjInfo*					FindExistObjInfo( void* pObj );
							
		virtual bool				RunFile( const char* szFileName, bool bReload );
		virtual bool        		RunBuffer( const void* pBuffer, size_t nSize );
		virtual bool        		RunString( const char* szString );
		virtual bool        		RunFunction( const STypeInfoArray& aryTypeInfo, 
										void* pResultBuf, const char* szFunction, void** aryArg );
		
		virtual int32				Compiler( int32 nArgc, char** szArgv );
        virtual void				RefScriptObj( void* pObj );
        virtual void				UnrefScriptObj( void* pObj );
		virtual void				UnlinkCppObjFromScript( void* pObj );

		virtual void        		GC();
		virtual void        		GCAll();
	};
}

#endif
