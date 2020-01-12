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

int main( int argc, const char* argv[] )
{
	CScriptBase* pScript = new CScriptLua( 5067 );
	pScript->AddSearchPath( "F:/GitHub/XScript/sampler/lua/" );
	pScript->RunFile( "./test.lua" );
	while( true )
	{
		pScript->RunFunction( NULL, "StartApplication", "sampler", 12345 );
		GammaSleep( 10 );
	}
	delete pScript;
	getchar();
	return 0;
}

