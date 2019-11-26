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

	extern CLuaTypeBase* s_aryType[eDT_count];
	inline CLuaTypeBase* GetTypeBase(DataType eType)
	{
		if (eType < eDT_count)
			return s_aryType[eType];
		if (eType & 1)
			return &CLuaObject::GetInst();
		return &CLuaValueObject::GetInst();
	}

    //=====================================================================
    // Lua脚本调用C++的接口
    //=====================================================================
    class CByScriptLua
	{
		static void GetParam( lua_State* pL, int32 nStartIndex, size_t arySize[],
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
