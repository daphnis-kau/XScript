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
	CJSObject::CJSObject(CClassRegistInfo* pClassInfo, uint32 nSize /*= sizeof(void*)*/)
		: CJSPointer(nSize)
		, m_pClassInfo(pClassInfo)
	{
		m_nType = eDT_class;
		m_nFlag = 0;
	}

	inline CClassRegistInfo* CJSObject::_FromVMValue(
		CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj )
	{
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

		CClassRegistInfo* pObjInfo = pInfo->m_pClassInfo;
		if( pObjInfo == m_pClassInfo )
		{
			*(void**)( pDataBuf ) = pInfo->m_pObject;
			return pObjInfo;
		}

		int32 nOffset = pObjInfo->GetBaseOffset( m_pClassInfo );
		if( nOffset >= 0 )
			*(void**)( pDataBuf ) = ( (char*)pInfo->m_pObject ) + nOffset;
		else if( ( nOffset = m_pClassInfo->GetBaseOffset( pObjInfo ) ) >= 0 )
			*(void**)( pDataBuf ) = ( (char*)pInfo->m_pObject ) - nOffset;
		else
			*(void**)( pDataBuf ) = pInfo->m_pObject;
		return pObjInfo;
	}

	inline Gamma::LocalValue CJSObject::_ToVMValue( CScriptJS& Script, void* pObj, bool bCopy )
	{
		v8::Isolate* isolate = Script.GetIsolate();
		if( pObj == NULL )
			return v8::Null( isolate );

		const CScriptJS::SObjInfo* pObjInfo = NULL;
		if( !bCopy && ( pObjInfo = Script.FindExistObjInfo( pObj ) ) != NULL )
			return pObjInfo->m_Object.Get( Script.GetIsolate() );

		CJSClassRegisterInfo* pInfo = (CJSClassRegisterInfo*)m_pClassInfo;
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		v8::Local<v8::FunctionTemplate> funTemplate = pInfo->GetContext().Get(isolate);
		v8::Local<v8::Function> JSClass = funTemplate->GetFunction(context).ToLocalChecked();
		v8::Local<v8::ObjectTemplate> objTemplate = funTemplate->InstanceTemplate();
		v8::Local<v8::Object> NewObj = objTemplate->NewInstance(context).ToLocalChecked();
		v8::Local<v8::String> __proto__ = Script.m___proto__.Get(isolate);
		v8::Local<v8::String> Prototype = Script.m_Prototype.Get(isolate);
		NewObj->Set(context, __proto__, JSClass->Get(context, Prototype).ToLocalChecked() );

		if (bCopy)
		{
			Script.BindObj(NULL, NewObj, m_pClassInfo, pObj);
			m_pClassInfo->Release(pObj);
		}
		else
			Script.BindObj(pObj, NewObj, m_pClassInfo);
		return NewObj;
	}

	void CJSObject::FromVMValue(CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj)
	{
		_FromVMValue(Script, pDataBuf, obj);
	}

	Gamma::LocalValue CJSObject::ToVMValue(CScriptJS& Script, char* pDataBuf)
	{
		void* pObj = *(void**)(pDataBuf);
		if (!pObj)
			return v8::Null(Script.GetIsolate());
		return _ToVMValue(Script, pObj, false);
	}

	//=====================================================================
	/// CJSValueObject
	//=====================================================================
	CJSValueObject::CJSValueObject(CClassRegistInfo* pClassInfo, uint32 nSize /*= 0*/)
		: CJSObject(pClassInfo, nSize ? nSize : (uint32)pClassInfo->GetClassSize())
	{
		m_nFlag = eFPT_Value;
	}

	void CJSValueObject::FromVMValue(CScriptJS& Script, char* pDataBuf, v8::Local<v8::Value> obj)
	{
		void* pObject = NULL;
		CClassRegistInfo* pClassInfo = _FromVMValue(Script, (char*)&pObject, obj);
		CJSObject::FromVMValue(Script, (char*)&pObject, obj);
		assert(pClassInfo);
		int32 nOffset = pClassInfo->GetBaseOffset(m_pClassInfo);
		assert(nOffset >= 0);

		pObject = ((char*)pObject) + nOffset;
		m_pClassInfo->Assign(pDataBuf, pObject);
	}

	Gamma::LocalValue CJSValueObject::ToVMValue(CScriptJS& Script, char* pDataBuf)
	{
		void* pObj = *(void**)(pDataBuf);
		if (!pObj)
			return v8::Null(Script.GetIsolate());
		return _ToVMValue(Script, pObj, true);
	}
}

