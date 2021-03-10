#include "CApplication.h"

int SApplicationConfig::nCount = 0;

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

IApplicationHandler* CApplication::TestCallObjectPointer( IApplicationHandler* Handler )
{
	m_pHandler = Handler;
	Test( !strcmp( m_pHandler->OnTestNoParamPureVirtual(), "OK" ), "Test object pointer from Script" );
	return TestVirtualObjectPointer( Handler );
}

SApplicationConfig& CApplication::TestCallObjectReference( SApplicationConfig& Config )
{
	Test( !strcmp( Config.szName, "sampler" ) && Config.nID == 12345, "Test object reference from Script" );
	return TestVirtualObjectReference( Config );
}

SApplicationConfig CApplication::TestCallObjectValue( SApplicationConfig Config )
{
	Test( !strcmp( Config.szName, "sampler" ) && Config.nID == 12345, "Test object value from Script" );
	return TestVirtualObjectValue( Config );
}

IApplicationHandler* CApplication::TestVirtualObjectPointer( IApplicationHandler* Handler )
{
	Test( true, "Test object pointer virtual function super call" );
	return Handler;
}

SApplicationConfig& CApplication::TestVirtualObjectReference( SApplicationConfig& Config )
{
	Test( true, "Test object reference virtual function super call" );
	return Config;
}

SApplicationConfig CApplication::TestVirtualObjectValue( SApplicationConfig Config )
{
	Test( true, "Test object value virtual function super call" );
	return Config;
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

void* CApplication::TestBuffer(void* buffer)
{
	return m_pHandler->TestBuffer(buffer);
}
