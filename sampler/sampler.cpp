#include "sampler.h"
#include "core/GammaScript.h"
#include "../src/luabinder/CScriptLua.h"
#include <string>

using namespace std;
using namespace Gamma;

const char* szLua =
"function aaa( a, b ) print( a, b ) end\n"
"run_cpp_lua( 1234, 3456, 98765, \"122\" );\n";

CScriptBase* g_ScriptLua;

enum ETestEnum
{
	eTE_0,
	eTE_1,
};

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

class CUnDuplicatableObject
{
	CUnDuplicatableObject& operator=( const CUnDuplicatableObject & );
public:
	double TestRetDouble() { return 123456.0; }
};

class CAbstractObject
{
	CAbstractObject();
public:
	double TestRetFloat() { return 123456.0f; }
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
		if( NewThis() == this )
			printf( "NewThis OK\n" );

		return v;
	};

	virtual uint32 GetThis( CVector2f v ) const
	{
		return 0;
	};

	virtual TestBase* NewThis() const = 0;

	virtual TestBase* TestThis() = 0;

	static const char* GetCppName( CVector2f v )
	{
		return "TestBase";
	}

	uint64 TestRet( const char* p1, ETestEnum p2 ) const
	{
		printf( "%f,%s,%d\n", x, p1, p2 );
		return 9876543210;
	}

	uint64 TestRet2()
	{
		return 9876543210;
	}
};

typedef void( TestBase::*FunType )( void );

template<int a>
class CCC
{
};

void run_cpp_lua( int a, const int& b, int c, int d )
{
	g_ScriptLua->RunFunction( NULL, "aaa", a, "qqqqqq" );
}

DEFINE_CLASS_BEGIN( CVector2f )
	REGIST_CLASSMEMBER( x )
	REGIST_CLASSMEMBER( y )
DEFINE_CLASS_END();

DEFINE_UNDUPLICATABLE_CLASS_BEGIN( CUnDuplicatableObject )
	REGIST_CLASSFUNCTION( TestRetDouble )
DEFINE_UNDUPLICATABLE_CLASS_END();

DEFINE_ABSTRACT_CLASS_BEGIN( CAbstractObject )
	REGIST_CLASSFUNCTION( TestRetFloat )
DEFINE_ABSTRACT_CLASS_END();

typedef TestBase Test_Base;
DEFINE_CLASS_BEGIN( Test_Base )
	REGIST_DESTRUCTOR()
	REGIST_CLASSMEMBER( kkk )
	REGIST_CLASSMEMBER( x )
	REGIST_CLASSFUNCTION( GetNumber )
	REGIST_CLASSFUNCTION_WITHNAME( TestRet, TestRet64 )
	REGIST_CLASSFUNCTION_OVERLOAD( uint64( TestBase::* )(), TestRet2, TestRet2 )
	REGIST_CALLBACKFUNCTION( GetThis )
	REGIST_PUREVIRTUALFUNCTION_WITHNAME( NewThis, NewThisLua )
	REGIST_PUREVIRTUALFUNCTION_OVERLOAD( TestBase*(TestBase::*)(), TestThis, TestThisLua )
	REGIST_STATICFUNCTION( GetCppName )
DEFINE_CLASS_END();

REGIST_GLOBALFUNCTION( run_cpp_lua );

void TestLua()
{
	g_ScriptLua = new CScriptLua;
	typedef decltype ( ( TestBase*( TestBase::* )( ) )nullptr ) aaa;
	typedef decltype ( ( decltype ( &TestBase::NewThis ) )nullptr ) bbbb;

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

		"function a:NewThisLua() \n"
		"   return self;\n"
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
	delete g_ScriptLua;
}

int main( int argc, const char* argv[] )
{
	TestLua();
	getchar();
	return 0;
}

