#ifndef __CALL_LUA_H__
#define __CALL_LUA_H__
//=====================================================================
// CCallBase.h 
// 定义基本的脚本和C++互相调用接口
// 柯达昭
// 2007-10-16
//=====================================================================

#include "core/CCallBase.h"

using namespace std;

namespace Gamma
{
	class CScriptLua;

    //=====================================================================
    // Lua脚本调用C++的接口
    //=====================================================================
    class CByScriptLua
	{
		static void GetParam( lua_State* pL, int32 nStartIndex, 
			const vector<DataType>& listParam, char* pDataBuf, void** pArgArray );
    public:
        static int32 CallByLua( lua_State* pL );
	};

    //=====================================================================
    // C++调用Lua脚本的接口
    //=====================================================================
    class CCallBackLua : public CCallScriptBase
	{
		static void	PushParam2VM( CScriptLua* pScript,
			const vector<DataType>& listParam, lua_State* pL, void** pArgArray );
	public:
		static bool	CallVM( CScriptLua* pScript, 
			CCallScriptBase* pCallBase, SVirtualObj* pObject, void* pRetBuf, void** pArgArray );
		static void	DestrucVM( CScriptLua* pScript,
			CCallScriptBase* pCallBase, SVirtualObj* pObject );
	};
};

#endif
