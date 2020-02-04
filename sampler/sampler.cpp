#ifndef _WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif

#include <chrono>
#include <thread>

#include "sampler.h"
#include "CApplication.h"

DEFINE_CLASS_BEGIN( SApplicationConfig )
	REGIST_CLASSMEMBER( szName )
	REGIST_CLASSMEMBER( nID )
DEFINE_CLASS_END();

DEFINE_CLASS_BEGIN( IApplicationHandler )
	REGIST_PUREVIRTUALFUNCTION( OnTestPureVirtual )
	REGIST_PUREVIRTUALFUNCTION( OnTestNoParamPureVirtual )
DEFINE_CLASS_END();

DEFINE_ABSTRACT_CLASS_BEGIN( CApplication )
	REGIST_DESTRUCTOR()
	REGIST_CLASSFUNCTION( TestCallObject )
	REGIST_CLASSFUNCTION( TestCallPOD )
	REGIST_CLASSFUNCTION( TestNoParamFunction )
	REGIST_CALLBACKFUNCTION( TestVirtual )
	REGIST_STATICFUNCTION( GetInst )
DEFINE_ABSTRACT_CLASS_END();

template<typename ScriptType>
CScriptBase* CreateScript(const char* szFilePath)
{
	char szWorkDir[2048];
	getcwd(szWorkDir, ELEM_COUNT(szWorkDir));
	CScriptBase* pScript = new ScriptType(5067);
	pScript->AddSearchPath(szWorkDir);
	pScript->RunFile(szFilePath);
	return pScript;
}

int main( int argc, const char* argv[] )
{
	CScriptBase* pScript = CreateScript<CScriptLua>("lua/test.lua");
	//CScriptBase* pScript = CreateScript<CScriptJS>("js/test.js");

	while( true )
	{
		pScript->RunFunction( NULL, "StartApplication", "sampler", 12345 );
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	delete pScript;
	getchar();
	return 0;
}

