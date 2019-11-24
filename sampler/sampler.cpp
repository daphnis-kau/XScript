#include "sampler.h"
#include "core/GammaScriptX.h"
#include "../src/luabinder/CScriptLua.h"
#include <string>

using namespace std;
using namespace Gamma;

const char* szLua =
"function aaa( a, b ) print( a, b ) end\n"
"run_cpp_lua( 1234, 3456, 98765, \"122\" );\n";

CScriptBase* g_ScriptLua;

class A
{
	uint32 n;
public:
	A() : n( 0 ) {}
};

class B : public A
{
	uint32 m;
public:
	B() : m( 1 ) {}
	virtual ~B() {}
};

class C : public B
{
	uint32 o;
public:
	C() : o( 2 ) {}
};

struct CVector2f 
{
	float x, y;
};

class TestBase
{
public:
	string aaa;
	CVector2f kkk;
	float x;
	TestBase()
	{
		aaa = "aaaa";
		x = 1234567.0f;
	}

	virtual ~TestBase() {}
	CVector2f GetNumber( CVector2f& v, int32 n )
	{
		v.x = 456;
		v.y = 789;
		printf( "%d\n", GetThis( v ) );
		return v;
	};

	virtual uint32 GetThis( CVector2f v )
	{
		return 0;
	};

	virtual TestBase* NewThis() = 0;

	static const char* GetCppName( CVector2f v )
	{
		return "TestBase";
	}

	uint64 TestRet64( const char* p1, uint16 p2 )
	{
		printf( "%f,%s,%d\n", x, p1, p2 );
		return 9876543210;
	}
};

typedef void( TestBase::*FunType )( void );

template<int a>
class CCC
{
};

void run_cpp_lua( int a, int b, int c, int d )
{
	g_ScriptLua->RunFunction( NULL, "aaa", a, "qqqqqq" );
}



void TestLua()
{
	g_ScriptLua = new CScriptLua;
	REGIST_B_CLASS_WITHNAME( *g_ScriptLua, TestBase, Test_Base );
	REGIST_B_CLASS( *g_ScriptLua, CVector2f );

	REGIST_CLASS_FUNCTION_BEGIN(CVector2f)
	REGIST_CLASSMEMBER( x )
	REGIST_CLASSMEMBER( y )
	REGIST_CLASS_FUNCTION_END( *g_ScriptLua );

	REGIST_CLASS_FUNCTION_BEGIN( TestBase )
	REGIST_DESTRUCTOR()
	REGIST_CLASSMEMBER( kkk )
	REGIST_CLASSMEMBER( x )
	REGIST_CLASSFUNCTION( GetNumber )
	REGIST_CALLBACKFUNCTION( GetThis )
	REGIST_PUREVIRTUALFUNCTION( NewThis )
	REGIST_STATICFUNCTION( GetCppName )
	REGIST_CLASSFUNCTION( TestRet64 )
	REGIST_CLASS_FUNCTION_END( *g_ScriptLua );

	REGIST_GLOBALFUNCTION( *g_ScriptLua, run_cpp_lua );

	int32 a[2];
	g_ScriptLua->RunString( szLua );
	//uint32 n = GetTickCount();
	g_ScriptLua->RunFunction( NULL, "aaa", a, "sadfasdf" );
	g_ScriptLua->RunString(
		"a = Test_Base:new(); \n"

		"function a:GetThis( v ) \n"
		"	print( \"call GetThis\", v:x(), v:y() );\n"
		"   return 3;\n"
		"end\n"

		"a:kkk():x(10)\n"
		"print( a:x() )\n"
		"a:kkk( CVector2f:new())\n"
		"a:kkk():x(1000)\n"
		"a:x(100)\n"

		"r = a:GetNumber( CVector2f:new(), 10 )\n"
		"print( r );\n"
		"print( r:x(), r:y() );\n"
		"print( Test_Base.GetCppName( a:kkk() ) );"
		"print( a:TestRet64( \"asdfff\", \"9580\" ) );"
	);

	g_ScriptLua->RunString( " " );
}

int main( int argc, const char* argv[] )
{
	TestLua();
	getchar();
	return 0;
}

