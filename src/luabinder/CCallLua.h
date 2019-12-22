#ifndef __CALL_LUA_H__
#define __CALL_LUA_H__
//=====================================================================
// CCallBase.h 
// 定义基本的脚本和C++互相调用接口
// 柯达昭
// 2007-10-16
//=====================================================================

#include "core/CCallBase.h"
#include "CTypeLua.h"

namespace Gamma
{
	class CScriptLua;

	extern CLuaTypeBase* s_aryLuaType[eDT_count];
	inline CLuaTypeBase* GetTypeBase(DataType eType)
	{
		if( eType <= eDT_enum )
			return s_aryLuaType[eType];
		if (eType & 1)
			return &CLuaObject::GetInst();
		return &CLuaValueObject::GetInst();
	}

    //=====================================================================
    // Lua脚本调用C++的接口
    //=====================================================================
    class CByScriptLua
	{
	public:
        static int32 CallByLua( lua_State* pL );
	};

    //=====================================================================
    // C++调用Lua脚本的接口
    //=====================================================================
    class CCallBackLua : public CCallScriptBase
	{
	public:
		static bool	CallVM( CScriptLua* pScript, 
			const CCallScriptBase* pCallBase, void* pRetBuf, void** pArgArray );
		static void	DestrucVM( CScriptLua* pScript,
			const CCallScriptBase* pCallBase, SVirtualObj* pObject );
	};
};

#endif
