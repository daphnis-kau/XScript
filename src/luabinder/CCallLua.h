#ifndef __CALL_LUA_H__
#define __CALL_LUA_H__
//=====================================================================
// CCallBase.h 
// 定义基本的脚本和C++互相调用接口
// 柯达昭
// 2007-10-16
//=====================================================================

#include "CCallBase.h"

using namespace std;

namespace Gamma
{

    //=====================================================================
    // Lua脚本调用C++的接口
    //=====================================================================
    class CByScriptLua
	{
		static void GetParam( lua_State* pL, int32 nStartIndex, 
			const list<CTypeBase*>& listParam, char* pDataBuf, void** pArgArray );
    public:
        static int32 CallByLua( lua_State* pL );
	};

    //=====================================================================
    // C++调用Lua脚本的接口
    //=====================================================================
    class CCallBackLua : public CCallScriptBase
    {
    public:
        CCallBackLua( CScriptBase& Script, const STypeInfoArray& aryTypeInfo, 
			IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunName )
            : CCallScriptBase( Script, aryTypeInfo, funWrap, szTypeInfoName, szFunName )
		{}

    protected:
        void			PushParam2VM( void* pVM, void** pArgArray );
		virtual bool    CallVM( SVirtualObj* pObject, void* pRetBuf, void** pArgArray );
		virtual void	DestrucVM( SVirtualObj* pObject ){};
	};
};

#endif
