#include "XScriptDll.h"
#include "core/XScript.h"
#include "../include/common/CodeCvs.h"
#include "../src/luabinder/CScriptLua.h"
#include "../src/v8binder/CScriptJS.h"

#ifdef _WIN32
	#define STDMETHODCALLTYPE __stdcall
#else
	#define STDMETHODCALLTYPE
#endif

namespace Export
{
	class IUnknown
	{
	public:
		virtual long STDMETHODCALLTYPE QueryInterface( void* id, void** ppvObject) = 0;
		virtual unsigned long STDMETHODCALLTYPE AddRef(void) = 0;
		virtual unsigned long STDMETHODCALLTYPE Release(void) = 0;
	};
}

#ifdef __cplusplus
extern "C" {
#endif


	EXPORT_API HScript CreateLuaScript(unsigned short nDebugPort)
	{
		return new XS::CScriptLua(nDebugPort);
	}

	EXPORT_API bool RunFile(HScript hScript, const wchar_t* szFileName)
	{
		std::string strFileName;
		strFileName.resize(wcslen(szFileName)*6 + 1);
		XS::UcsToUtf8(&strFileName[0], (uint32)strFileName.size(), szFileName);
		return hScript->RunFile(strFileName.c_str());
	}

	EXPORT_API bool RunString(HScript hScript, const wchar_t* szString)
	{
		std::string strContent;
		strContent.resize(wcslen(szString) * 6 + 1);
		XS::UcsToUtf8(&strContent[0], (uint32)strContent.size(), szString);
		return hScript->RunString(strContent.c_str());
	}

	EXPORT_API void TestCom(void* pObject)
	{
		Export::IUnknown* pInterface = (Export::IUnknown*)pObject;
		if (!pInterface)
			return;
		unsigned long ref1 = pInterface->AddRef();
		unsigned long ref2 = pInterface->Release();
		if (ref1 == ref2)
			return;
	}

#ifdef __cplusplus
}
#endif