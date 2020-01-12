#ifndef __SCRIPT_JS_H__
#define __SCRIPT_JS_H__
//=====================================================================
// CScriptJS3.h 
// 为lua定义的C++到虚拟机的接口
// 柯达昭
// 2007-10-16
//=====================================================================
#include "common/TRBTree.h"
#include "core/CScriptBase.h"

namespace Gamma
{
	struct SV8Context;
	struct SClassInfo;
	struct SObjInfo;
	struct SCallInfo;

    class CScriptJS : public CScriptBase
	{	
		SV8Context*					m_pV8Context;
		SObjInfo*					m_pFreeObjectInfo;
		TRBTree<SObjInfo>			m_mapObjInfo;
		TRBTree<SClassInfo>			m_mapClassInfo;
		TRBTree<SCallInfo>			m_mapCallBase;
		
		void						BuildRegisterInfo();
		
		SCallInfo*					GetCallInfo( const CByScriptBase* pCallBase );
		SObjInfo*					AllocObjectInfo();
		void						FreeObjectInfo(SObjInfo* pObjectInfo);

		virtual bool				CallVM( const CCallScriptBase* pCallBase, void* pRetBuf, void** pArgArray );
		virtual void				DestrucVM( const CCallScriptBase* pCallBase, SVirtualObj* pObject );

		friend class CJSObject;
		friend struct SV8Context;
    public:
		CScriptJS( uint16 nDebugPort );
		~CScriptJS(void);

		SV8Context&					GetV8Context() { return *m_pV8Context; }
		SObjInfo*					FindExistObjInfo( void* pObj );
							
		virtual bool				RunFile( const char* szFileName, bool bReload );
		virtual bool        		RunBuffer( const void* pBuffer, size_t nSize, const char* szFileName );
		virtual bool        		RunString( const char* szString );
		virtual bool        		RunFunction( const STypeInfoArray& aryTypeInfo, 
										void* pResultBuf, const char* szFunction, void** aryArg );
		
		virtual int32				Compiler( int32 nArgc, char** szArgv );
		virtual void				UnlinkCppObjFromScript( void* pObj );

		virtual void        		GC();
		virtual void        		GCAll();
	};
}

#endif
