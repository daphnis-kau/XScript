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
	typedef v8::Persistent<v8::Context>					PersistentContext;
	typedef v8::Persistent<v8::ObjectTemplate>			PersistentObjTemplate;
	typedef v8::Persistent<v8::FunctionTemplate>		PersistentFunTemplate;
	typedef v8::Persistent<v8::Object>					PersistentObject;
	typedef v8::Persistent<v8::String>					PersistentString;
	typedef v8::Persistent<v8::Function>				PersistentFunction;
	typedef v8::Local<v8::Value>						LocalValue;
	typedef v8::ReturnValue<v8::Value>					ReturnValue;
	typedef TClassRegisterInfo<PersistentFunTemplate>	CJSClassRegisterInfo;

    class CScriptJS : public CScriptBase
	{
		struct SStringDynamic
		{
			void*				m_pBuffer;
			void*				m_pStack;
		};

		struct SStringFixed
		{
			uint32				m_nLen;
			void*				m_pStack;
		};

		struct SObjInfo : public TRBTree<SObjInfo>::CRBTreeNode
		{
			void*				m_pObject;
			PersistentObject	m_Object;
			CClassRegistInfo*	m_pClassInfo;
			bool				m_bRecycle;
			bool				m_bFirstAddress;
			operator void*() { return m_pObject; }
			bool operator< (void* r) { return m_pObject < r; }
		};

		uint32					m_nStringID;
		v8::Platform*			m_platform;
		v8::Isolate*			m_pIsolate;
		PersistentContext		m_Context;
		PersistentObject		m_GammaNameSpace;
		PersistentFunction		m_GammaClass;
		PersistentString		m_CppField;
		PersistentString		m_Prototype;
		PersistentString		m___proto__;

		vector<SStringDynamic>	m_vecStringInfo;
		tbyte*					m_pTempStrBuffer64K;
		uint32					m_nCurUseSize;
		uint32					m_nStrBufferStack;
		wstring					m_szTempUcs2;

		SObjInfo*				m_pFreeObjectInfo;
		TRBTree<SObjInfo>	m_mapObjInfo;

		static void				Log(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void				Break(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void				Callback(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void				NewObject(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void				Destruction(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void				GCCallback(const v8::WeakCallbackInfo<SObjInfo>& data);
		static void				GetterCallback(v8::Local<v8::Name> property, const v8::PropertyCallbackInfo<v8::Value>& info);
		static void				SetterCallback(v8::Local<v8::Name> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

		static CTypeBase*		MakeType(CClassRegistInfo* pInfo, bool bValue);
		CTypeBase*				MakeObject(const STypeInfo& argInfo, bool bValue);
		void					RegistClass( MakeTypeFunction funMakeType, uint32 nSize, 
									const char* szTypeIDName, const char* szClass, va_list listBase );
		void					MakeMeberFunction(CClassRegistInfo* pInfo, v8::Local<v8::Function> NewClass, v8::Local<v8::Object> Prototype, bool bBase);
		
		SObjInfo*				AllocObjectInfo();
		void					FreeObjectInfo(SObjInfo* pObjectInfo);
		void					BindObj(void* pObject, v8::Local<v8::Object> ScriptObj, CClassRegistInfo* pInfo, void* pSrc = NULL);
		void					UnbindObj( SObjInfo* pObjectInfo, bool bFromGC );

		friend class CJSObject;
    public:
		CScriptJS( uint16 nDebugPort );
		~CScriptJS(void);

		v8::Platform*			GetPlatform() { return m_platform; }
		v8::Isolate*			GetIsolate() { return m_pIsolate; }
		PersistentContext&		GetContext() { return m_Context; }
		PersistentString&		GetProto() { return m___proto__; }
		PersistentString&		GetPrototype() { return m_Prototype; }
		PersistentObject&		GetGammaNameSpace() { return m_GammaNameSpace; }

		void					ClearCppString(void* pStack);
		void					CallJSStatck(bool bAdd);
		v8::Local<v8::Value>	StringFromUtf8(const char* szUtf8);
		v8::Local<v8::Value>	StringFromUcs(const wchar_t* szUcs);
		const char*				StringToUtf8(v8::Local<v8::Value> obj);
		const wchar_t*			StringToUcs(v8::Local<v8::Value> obj);
		void					ReportException( v8::TryCatch* try_catch, v8::Local<v8::Context> context );
		SObjInfo*				FindExistObjInfo( void* pObj );

		CTypeBase*              MakeParamType(const STypeInfo& argTypeInfo);							
		virtual bool            RunFile( const char* szFileName, bool bReload );
		virtual bool        	RunBuffer( const void* pBuffer, size_t nSize );
		virtual bool        	RunString( const char* szString );
		virtual bool        	RunFunction( const STypeInfoArray& aryTypeInfo, void* pResultBuf, const char* szFunction, void** aryArg );
		virtual void            RegistFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName );
		virtual void            RegistClassStaticFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName );
        virtual void            RegistClassFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName );
		virtual ICallBackWrap& 	RegistClassCallback( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName );
		virtual ICallBackWrap&	RegistDestructor( const char* szTypeInfoName, IFunctionWrap* funWrap );
        virtual void            RegistClassMember( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funGetSet[2], const char* szTypeInfoName, const char* szMemberName );
        virtual void            RegistClass( uint32 nSize, const char* szTypeIDName, const char* szClass, ... );
		virtual void			RegistConstruct( IObjectConstruct* pObjectConstruct, const char* szTypeIDName );


		virtual void            RegistEnum( const char* szTypeIDName, const char* szTableName, int32 nTypeSize );
        virtual void            RegistConstant( const char* szTableName, const char* szFeild, int32 nValue );
		virtual	void			RegistConstant( const char* szTableName, const char* szFeild, double dValue );
		virtual	void			RegistConstant( const char* szTableName, const char* szFeild, const char* szValue );
        virtual int32           Compiler( int32 nArgc, char** szArgv );
        virtual void            RefScriptObj( void* pObj );
        virtual void            UnrefScriptObj( void* pObj );
		virtual void            UnlinkCppObjFromScript( void* pObj );

		virtual void        	GC();
		virtual void        	GCAll();

		virtual string			GetInstancesDump();
		virtual void			DumpInstanceTree(const char* szFileName);
		virtual void			FrameMove();
		virtual std::string		GetProfile();

		virtual size_t			GetClassCount();
		virtual bool			GetInstanceInfo(size_t nClassIndex, size_t& nInstanceCount, size_t& nMemory, char szClassName[256]);
		virtual size_t			GetVMTotalSize();;
	};
}

#endif
