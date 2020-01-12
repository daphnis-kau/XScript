#pragma once
#include "common/CThread.h"
#include "core/GammaScript.h"
#include "../src/luabinder/CScriptLua.h"
#include "../src/v8binder/CScriptJS.h"

using namespace Gamma;

enum ETestEnum
{
	eTE_0 = 1234,
};

struct SApplicationConfig
{
	const char* szName;
	uint32		nID;
};

// ¥ø–È¿‡
class IApplicationHandler
{
public:
	virtual const char* OnTestPureVirtual(
		ETestEnum e, int8 v0, int16 v1, int32 v2, int64 v3, long v4,
		uint8 v5, uint16 v6, uint32 v7, uint64 v8, unsigned long v9,
		float v10, double v11, const char* v12, const wchar_t* v13 ) const = 0;

	virtual const char* OnTestNoParamPureVirtual() const = 0;
};

class CApplication
{
	CApplication& operator=( const CApplication & );
	CApplication();

protected:
	virtual ~CApplication(){}
	IApplicationHandler* m_pHandler;

public:

	static CApplication& GetInst();
	
	SApplicationConfig& TestCallObject( IApplicationHandler* Handler,
		SApplicationConfig v0, SApplicationConfig& v1, SApplicationConfig* v2 );

	const char* TestCallPOD( 
		ETestEnum e, int8 v0, int16 v1, int32 v2, int64 v3, long v4,
		uint8 v5, uint16 v6, uint32 v7, uint64 v8, unsigned long v9,
		float v10, double v11, const char* v12, const wchar_t* v13 );

	const char* TestNoParamFunction();

	virtual SApplicationConfig& TestVirtual( SApplicationConfig v0, 
		SApplicationConfig& v1, SApplicationConfig* v2 );
};