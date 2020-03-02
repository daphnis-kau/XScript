/**@file  		CScriptJS3.h
* @brief		V8 base wrapper
* @author		Daphnis Kaw
* @date			2020-01-17
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
		SV8Context*					m_pV8Context;
		SObjInfo*					m_pFreeObjectInfo;
		TRBTree<SObjInfo>			m_mapObjInfo;
		TRBTree<SJSClassInfo>		m_mapClassInfo;
		TRBTree<SCallInfo>			m_mapCallBase;
		
		void						BuildRegisterInfo();
		
		SCallInfo*					GetCallInfo( const CCallInfo* pCallBase );
		SObjInfo*					AllocObjectInfo();
		void						FreeObjectInfo(SObjInfo* pObjectInfo);

		virtual bool				CallVM( const CCallbackInfo* pCallBase, void* pRetBuf, void** pArgArray );
		virtual void				DestrucVM( const CCallbackInfo* pCallBase, SVirtualObj* pObject );

		friend class CJSObject;
		friend struct SV8Context;
    public:
		CScriptJS( uint16 nDebugPort );
		~CScriptJS(void);

		SV8Context&					GetV8Context() { return *m_pV8Context; }
		SObjInfo*					FindExistObjInfo( void* pObj );
							
		virtual bool        		RunBuffer( const void* pBuffer, size_t nSize, const char* szFileName );
		virtual bool        		RunFunction( const STypeInfoArray& aryTypeInfo, 
										void* pResultBuf, const char* szFunction, void** aryArg );
		
		virtual int32				Compiler( int32 nArgc, char** szArgv );
		virtual void				UnlinkCppObjFromScript( void* pObj );

		virtual void        		GC();
		virtual void        		GCAll();
	};
}

#endif
