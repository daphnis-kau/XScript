﻿/**@file  		CLuaDebug.h
* @brief		LUA debugger interface
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
*/

#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__
#include "common/CommonType.h"

#if ( defined( _WIN32 ))
#define DEBUG_API __declspec(dllexport)
#else
#define DEBUG_API 
#endif

#ifdef __cplusplus
extern "C" {
#endif
	typedef struct lua_State lua_State;
	typedef struct lua_Debug lua_Debug;

	/*create debugger*/
	DEBUG_API void* new_debugger(lua_State* pState, const char* strDebugHost, uint16 nDebugPort, bool bWaitDebug);

	/*destroy debugger*/
	DEBUG_API void del_debugger(void* _pDebugger );

	/*call every frame to check debugger's command*/
	DEBUG_API int check_debug_cmd(void* _pDebugger );

#ifdef __cplusplus
};
#endif
#endif