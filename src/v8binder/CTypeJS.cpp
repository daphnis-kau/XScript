#include "CTypeJS.h"
#include "CScriptJS.h"
#include "core/CClassRegistInfo.h"
#include "common/Help.h"
#include <string>

using namespace std;

namespace Gamma
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

	inline const CClassRegistInfo* CJSObject::_FromVMValue(DataType eType,
		CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj )
	{
		auto pClassInfo = (const CClassRegistInfo*)((eType >> 1) << 1);
		if( obj == v8::Null( Script.GetIsolate() ) ||	!obj->IsObject() )
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
			v8::Local<v8::Value> key = Script.m_CppField.Get( Script.GetIsolate() );
			v8::Local<v8::Context> context = Script.GetIsolate()->GetCurrentContext();
			v8::MaybeLocal<v8::Value> field = pScriptObject->Get( context, key );
			if( field.IsEmpty() )
				return NULL;
			v8::Local<v8::Value> extenalField = field.ToLocalChecked();
			if( !extenalField->IsExternal() )
				return NULL;
			pCppBind = v8::External::Cast( *extenalField );
		}

		const CScriptJS::SObjInfo* pInfo = ( const CScriptJS::SObjInfo* )pCppBind->Value();
		if( !pInfo || !pInfo->m_pObject )
		{
			*(void**)( pDataBuf ) = NULL;
			return NULL;
		}

		const CClassRegistInfo* pObjInfo = pInfo->m_pClassInfo->m_pClassInfo;
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

	inline Gamma::LocalValue CJSObject::_ToVMValue(DataType eType, 
		CScriptJS& Script, void* pObj, bool bCopy )
	{
		auto pClassInfo = (const CClassRegistInfo*)((eType >> 1) << 1);
		v8::Isolate* isolate = Script.GetIsolate();
		if( pObj == NULL )
			return v8::Null( isolate );

		const CScriptJS::SObjInfo* pObjInfo = NULL;
		if( !bCopy && ( pObjInfo = Script.FindExistObjInfo( pObj ) ) != NULL )
			return pObjInfo->m_Object.Get( Script.GetIsolate() );

		PersistentFunTmplt& persistentTemplate = Script.GetPersistentFunTemplate(pClassInfo);
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::FunctionTemplate> funTemplate = persistentTemplate.Get(isolate);
		v8::Local<v8::Function> JSClass = funTemplate->GetFunction(context).ToLocalChecked();
		v8::Local<v8::ObjectTemplate> objTemplate = funTemplate->InstanceTemplate();
		v8::Local<v8::Object> NewObj = objTemplate->NewInstance(context).ToLocalChecked();
		v8::Local<v8::String> __proto__ = Script.m___proto__.Get(isolate);
		v8::Local<v8::String> Prototype = Script.m_Prototype.Get(isolate);
		NewObj->Set(context, __proto__, JSClass->Get(context, Prototype).ToLocalChecked() );

		if (bCopy)
		{
			Script.BindObj(NULL, NewObj, pClassInfo, pObj);
			pClassInfo->Release( pObj );
			Script.CheckUnlinkCppObj();
		}
		else
			Script.BindObj(pObj, NewObj, pClassInfo);
		return NewObj;
	}

	void CJSObject::FromVMValue(DataType eType, 
		CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj)
	{
		_FromVMValue(eType, Script, pDataBuf, obj);
	}

	Gamma::LocalValue CJSObject::ToVMValue(DataType eType, 
		CScriptJS& Script, char* pDataBuf)
	{
		void* pObj = *(void**)(pDataBuf);
		if (!pObj)
			return v8::Null(Script.GetIsolate());
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
		void* pObject = NULL;
		const CClassRegistInfo* pClassInfo = 
			_FromVMValue(eType, Script, (char*)&pObject, obj);
		CJSObject::FromVMValue(eType, Script, (char*)&pObject, obj);
		assert(pClassInfo);
		auto pCurClassInfo = (const CClassRegistInfo*)((eType >> 1) << 1);
		int32 nOffset = pClassInfo->GetBaseOffset(pCurClassInfo);
		assert(nOffset >= 0);

		pObject = ((char*)pObject) + nOffset;
		pCurClassInfo->Assign( pDataBuf, pObject );
		Script.CheckUnlinkCppObj();
	}

	Gamma::LocalValue CJSValueObject::ToVMValue(DataType eType, 
		CScriptJS& Script, char* pDataBuf)
	{
		void* pObj = *(void**)(pDataBuf);
		if (!pObj)
			return v8::Null(Script.GetIsolate());
		return _ToVMValue(eType, Script, pObj, true);
	}
}

