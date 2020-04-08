#include "CTypeJS.h"
#include "CScriptJS.h"
#include "core/CClassInfo.h"
#include "common/Help.h"
#include <string>

namespace XS
{
	//=====================================================================
	/// CJSObject
	//=====================================================================
	CJSObject::CJSObject()
	{
	}

	CJSObject& CJSObject::GetInst()
	{
		static CJSObject s_Instance;
		return s_Instance;
	}

	inline const CClassInfo* CJSObject::_FromVMValue(DataType eType,
		CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj )
	{
		SV8Context& Context = Script.GetV8Context();
		v8::Isolate* isolate = Context.m_pIsolate;
		auto pClassInfo = (const CClassInfo*)((eType >> 1) << 1);
		if( obj == v8::Null(isolate) ||	!obj->IsObject() )
		{
			*(void**)( pDataBuf ) = NULL;
			return NULL;
		}

		v8::Object* pScriptObject = v8::Object::Cast( *obj );
		v8::External* pCppBind = NULL;
		if( pScriptObject->InternalFieldCount() )
			pCppBind = v8::External::Cast( *pScriptObject->GetInternalField( 0 ) );
		else
		{
			v8::Local<v8::Value> key = Context.m_CppField.Get(isolate);
			v8::Local<v8::Context> context = isolate->GetCurrentContext();
			v8::MaybeLocal<v8::Value> field = pScriptObject->Get( context, key );
			if( field.IsEmpty() )
				return NULL;
			v8::Local<v8::Value> extenalField = field.ToLocalChecked();
			if( !extenalField->IsExternal() )
				return NULL;
			pCppBind = v8::External::Cast( *extenalField );
		}

		const SObjInfo* pInfo = ( const SObjInfo* )pCppBind->Value();
		if( !pInfo || !pInfo->m_pObject )
		{
			*(void**)( pDataBuf ) = NULL;
			return NULL;
		}

		const CClassInfo* pObjInfo = pInfo->m_pClassInfo->m_pClassInfo;
		if( pObjInfo == pClassInfo)
		{
			*(void**)( pDataBuf ) = pInfo->m_pObject;
			return pObjInfo;
		}

		int32 nOffset = pObjInfo->GetBaseOffset(pClassInfo);
		if( nOffset >= 0 )
			*(void**)( pDataBuf ) = ( (char*)pInfo->m_pObject ) + nOffset;
		else if( ( nOffset = pClassInfo->GetBaseOffset( pObjInfo ) ) >= 0 )
			*(void**)( pDataBuf ) = ( (char*)pInfo->m_pObject ) - nOffset;
		else
			*(void**)( pDataBuf ) = pInfo->m_pObject;
		return pObjInfo;
	}

	inline XS::LocalValue CJSObject::_ToVMValue(DataType eType, 
		CScriptJS& Script, void* pObj, bool bCopy )
	{
		auto pClassInfo = (const CClassInfo*)((eType >> 1) << 1);
		SV8Context& Context = Script.GetV8Context();
		v8::Isolate* isolate = Context.m_pIsolate;
		if( pObj == NULL )
			return v8::Null( isolate );

		const SObjInfo* pObjInfo = NULL;
		if( !bCopy && ( pObjInfo = Script.FindExistObjInfo( pObj ) ) != NULL &&
			pObjInfo->m_pClassInfo->m_pClassInfo->FindBase(pClassInfo) )
			return pObjInfo->m_Object.Get( isolate );

		static PersistentFunTmplt s_Instance;
		SJSClassInfo* classInfo = Script.m_mapClassInfo.Find((const void*)pClassInfo);
		PersistentFunTmplt& persistentTemplate = classInfo ? classInfo->m_FunctionTemplate : s_Instance;
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::FunctionTemplate> funTemplate = persistentTemplate.Get(isolate);
		v8::Local<v8::Function> JSClass = funTemplate->GetFunction(context).ToLocalChecked();
		v8::Local<v8::ObjectTemplate> objTemplate = funTemplate->InstanceTemplate();
		v8::Local<v8::Object> NewObj = objTemplate->NewInstance(context).ToLocalChecked();
		v8::Local<v8::String> __proto__ = Context.m___proto__.Get(isolate);
		v8::Local<v8::String> Prototype = Context.m_Prototype.Get(isolate);
		NewObj->Set(context, __proto__, JSClass->Get(context, Prototype).ToLocalChecked() );

		if( !bCopy )
		{
			Context.BindObj( pObj, NewObj, pClassInfo, false );
			return NewObj;
		}

		void* pNewObject = new tbyte[pClassInfo->GetClassSize()];
		pClassInfo->CopyConstruct( &Script, pNewObject, pObj );
		Context.BindObj( pNewObject, NewObj, pClassInfo, true );
		return NewObj;
	}

	void CJSObject::FromVMValue(DataType eType, 
		CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj)
	{
		_FromVMValue(eType, Script, pDataBuf, obj);
	}

	XS::LocalValue CJSObject::ToVMValue(DataType eType, 
		CScriptJS& Script, char* pDataBuf)
	{
		void* pObj = *(void**)(pDataBuf);
		if (!pObj)
			return v8::Null(Script.GetV8Context().m_pIsolate);
		return _ToVMValue(eType, Script, pObj, false);
	}

	//=====================================================================
	/// CJSValueObject
	//=====================================================================
	CJSValueObject::CJSValueObject()
	{
	}

	CJSValueObject& CJSValueObject::GetInst()
	{
		static CJSValueObject s_Instance;
		return s_Instance;
	}

	void CJSValueObject::FromVMValue(DataType eType,
		CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj)
	{
		CJSObject::FromVMValue(eType, Script, pDataBuf, obj);
	}

	XS::LocalValue CJSValueObject::ToVMValue(DataType eType, 
		CScriptJS& Script, char* pDataBuf)
	{
		void* pObj = pDataBuf;
		if (!pObj)
			return v8::Null(Script.GetV8Context().m_pIsolate);
		return _ToVMValue(eType, Script, pObj, true);
	}

	//=====================================================================
	/// 所有JS数据类型
	//=====================================================================
	static CGlobalTypes s_listTypes( 
		GlobalTypeTemplateArgs( TJSValue, CJSObject, CJSValueObject ) );

	XS::CJSTypeBase* GetJSTypeBase( DataType eType )
	{
		return s_listTypes.GetTypeImp<CJSTypeBase>( eType );
	}
}

