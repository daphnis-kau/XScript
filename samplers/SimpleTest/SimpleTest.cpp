#ifndef _WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif

#include <stdio.h>
#include <chrono>
#include <thread>

#include "SimpleTest.h"
#include "CApplication.h"

DEFINE_CLASS_BEGIN( SAddress )
	REGIST_CONSTRUCTOR( uint32, uint16 )
	REGIST_CLASSMEMBER( nIP )
	REGIST_CLASSMEMBER( nPort )
	REGIST_CALLBACKFUNCTION( OnTestVirtualWithConstruct )
DEFINE_CLASS_END();

DEFINE_CLASS_BEGIN( SApplicationConfig )
	REGIST_CLASSFUNCTION( SetName )
	REGIST_CLASSFUNCTION( GetName )
	REGIST_CLASSMEMBER( nID )
	REGIST_CLASSMEMBER( Address )
DEFINE_CLASS_END();

DEFINE_UNDUPLICATABLE_CLASS_BEGIN( IApplicationHandler )
	REGIST_PUREVIRTUALFUNCTION( OnTestPureVirtual )
	REGIST_PUREVIRTUALFUNCTION( OnTestNoParamPureVirtual )
DEFINE_CLASS_END();

DEFINE_ABSTRACT_CLASS_BEGIN( CApplication )
	REGIST_DESTRUCTOR()
	REGIST_CLASSFUNCTION( TestCallObjectPointer )
	REGIST_CLASSFUNCTION( TestCallObjectReference )
	REGIST_CLASSFUNCTION( TestCallObjectValue )
	REGIST_CALLBACKFUNCTION( TestVirtualObjectPointer )
	REGIST_CALLBACKFUNCTION( TestVirtualObjectReference )
	REGIST_CALLBACKFUNCTION( TestVirtualObjectValue )
	REGIST_CLASSFUNCTION( TestCallPOD )
	REGIST_CLASSFUNCTION( TestNoParamFunction )
	REGIST_STATICFUNCTION( GetInst )
DEFINE_CLASS_END();

REGIST_GLOBALFUNCTION( AligenUp )

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
	while(true)
	{
		FILE* fp = fopen( "lua/test.lua", "rb" );
		if(fp)
		{
			fclose( fp );
			break;
		}

#ifdef _WIN32
		printf( 
			"Can not find the sampler script file. \n"
			"Please enter the path of sampler folder.\n"
			"Such as: C:\\XScript\\sampler\n" );
#else
		printf(
			"Can not find the sampler script file. \n"
			"Please enter the path of sampler folder.\n"
			"Such as: /XScript/sampler\n" );
#endif
		char szDir[256];
		scanf( "%s", szDir );
		chdir( szDir );
	}

	CScriptBase* pScript = CreateScript<CScriptLua>("lua/test.lua");
	//CScriptBase* pScript = CreateScript<CScriptJS>("js/test.js");

	while( true )
	{
		pScript->RunFunction( nullptr, "StartApplication", "sampler", 12345 );
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	delete pScript;
	getchar();
	return 0;
}

