#pragma once

namespace XS { class CScriptBase; }
typedef XS::CScriptBase* HScript;

#ifdef _WIN32
	#if defined( DLL_EXPORTS )
		#define EXPORT_API __declspec(dllexport)
	#else
		#define EXPORT_API __declspec(dllimport)
	#endif
#else
	#define EXPORT_API 
#endif

#ifdef __cplusplus
extern "C" {
#endif
	EXPORT_API HScript CreateLuaScript(unsigned short nDebugPort);
	EXPORT_API bool RunFile(HScript hScript, const wchar_t* szFileName);
	EXPORT_API bool RunString(HScript hScript, const wchar_t* szString);
	EXPORT_API void TestCom(void* pObject);
#ifdef __cplusplus
}
#endif