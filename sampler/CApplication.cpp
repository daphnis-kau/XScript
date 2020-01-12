#include "CApplication.h"

static void Test( bool bResult, const char* szMessage )
{
	printf( "%s ........ %s\n", szMessage, ( bResult ? "OK" : "Failed" ) );
}

CApplication::CApplication() 
{
}

CApplication& CApplication::GetInst()
{
	static CApplication s_Instance;
	return s_Instance;
}

SApplicationConfig& CApplication::TestCallObject( IApplicationHandler* Handler,
	SApplicationConfig v0, SApplicationConfig& v1, SApplicationConfig* v2 )
{
	m_pHandler = Handler;
	Test( !strcmp( v0.szName, "sampler" ) && v0.nID == 12345, "Test object value from Script" );
	Test( !strcmp( v1.szName, "sampler" ) && v1.nID == 12345, "Test object reference from Script" );
	Test( !strcmp( v2->szName, "sampler" ) && v2->nID == 12345, "Test object pointer from Script" );
	Test( v2 == &v1 && &v0 != v2, "Test object value copy from Script" );
	return TestVirtual( v0, v1, v2 );
}

const char* CApplication::TestCallPOD(
	ETestEnum e, int8 v0, int16 v1, int32 v2, int64 v3, long v4,
	uint8 v5, uint16 v6, uint32 v7, uint64 v8, unsigned long v9,
	float v10, double v11, const char* v12, const wchar_t* v13 )
{
	Test( e == eTE_0, "Test enum value from Script" );

	Test( v0 == -123, "Test int8 value from Script" );
	Test( v1 == -12345, "Test int16 value from Script" );
	Test( v2 == -12345678, "Test int32 value from Script" );
	Test( v3 == -1234567891011LL, "Test int64 value from Script" );
	Test( v4 == -123456789L, "Test long value from Script" );

	Test( v5 == 123, "Test uint8 value from Script" );
	Test( v6 == 12345, "Test uint16 value from Script" );
	Test( v7 == 12345678, "Test uint32 value from Script" );
	Test( v8 == 1234567891011LL, "Test uint64 value from Script" );
	Test( v9 == 123456789L, "Test unsigned long value from Script" );

	Test( v10 == 1234567.0f, "Test float value from Script" );
	Test( v11 == 123456789101112.0, "Test double value from Script" );

	Test( !strcmp( v12, "abcdefg" ), "Test const char* value from Script" );
	Test( !wcscmp( v13, L"abcdefg" ), "Test const wchar_t* value from Script" );

	return m_pHandler->OnTestPureVirtual(e, 
		v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13);
}

const char* CApplication::TestNoParamFunction()
{
	return m_pHandler->OnTestNoParamPureVirtual();
}

SApplicationConfig& CApplication::TestVirtual( SApplicationConfig v0,
	SApplicationConfig& v1, SApplicationConfig* v2 )
{
	Test( false, "Test virtual function call" );
	return v1;
}
