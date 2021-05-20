/**@file  		CScriptJS3.h
* @brief		V8 base wrapper
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
*/

#ifndef __SCRIPT_JS_H__
#define __SCRIPT_JS_H__
#include "common/TRBTree.h"
#include "core/CScriptBase.h"

namespace XS
{
	struct SV8Context;
	struct SJSClassInfo;
	struct SObjInfo;
	struct SCallInfo;

    class CScriptJS : public CScriptBase
	{	
		SV8Context*				m_pV8Context;
		SObjInfo*				m_pFreeObjectInfo;
		TRBTree<SObjInfo>		m_mapObjInfo;
		TRBTree<SJSClassInfo>	m_mapClassInfo;
		TRBTree<SCallInfo>		m_mapCallBase;
		
		void					BuildRegisterInfo();
		
		SCallInfo*				GetCallInfo( const CCallInfo* pCallBase );
		SObjInfo*				AllocObjectInfo();
		void					FreeObjectInfo(SObjInfo* pObjectInfo);

		virtual bool			CallVM( const CCallbackInfo* pCallBase, void* pRetBuf, void** pArgArray );
		virtual void			DestrucVM( const CCallbackInfo* pCallBase, SVirtualObj* pObject );

		virtual bool			Set( void* pObject, int32 nIndex, void* pArgBuf, const STypeInfo& TypeInfo );
		virtual bool			Get( void* pObject, int32 nIndex, void* pResultBuf, const STypeInfo& TypeInfo );
		virtual bool			Set( void* pObject, const char* szName, void* pArgBuf, const STypeInfo& TypeInfo );
		virtual bool			Get( void* pObject, const char* szName, void* pResultBuf, const STypeInfo& TypeInfo );
		virtual bool        	Call( const STypeInfoArray& aryTypeInfo, void* pResultBuf, const char* szFunction, void** aryArg );
		virtual bool			Call( const STypeInfoArray& aryTypeInfo, void* pResultBuf, void* pFunction, void** aryArg );
		virtual bool        	RunBuffer( const void* pBuffer, size_t nSize, const char* szFileName, bool bForceBuild = false );
    public:
		CScriptJS( const char* strDebugHost, uint16 nDebugPort );
		~CScriptJS( void );
		friend class CJSObject;
		friend struct SV8Context;

		SV8Context&				GetV8Context() { return *m_pV8Context; }
		SObjInfo*				FindExistObjInfo( void* pObj );
		
		virtual int32			Compiler( int32 nArgc, char** szArgv );
		virtual int32			IncRef( void* pObj );
		virtual int32			DecRef( void* pObj );
		virtual void			UnlinkCppObjFromScript( void* pObj );
		virtual bool			IsValid( void* pObject );
		virtual void        	GC();
		virtual void        	GCAll();
	};
}

#endif
