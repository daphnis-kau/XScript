#pragma once
#include "core/XScript.h"
#include "../src/luabinder/CScriptLua.h"
#include "../src/v8binder/CScriptJS.h"

using namespace XS;

enum ETestEnum
{
	eTE_0 = 1234,
};

struct SAddress
{
	uint32 nIP;
	uint16 nPort;
};

struct SApplicationConfig
{
	static int nCount;

	SApplicationConfig()
	{
		strContext.assign( "sdfsd" );
	}

	SApplicationConfig(const SApplicationConfig& Config)
	{
		memcpy(szName, Config.szName, sizeof(szName));
		nID = Config.nID;
		Address = Config.Address;
		strContext = Config.strContext;
	}

	~SApplicationConfig()
	{
		if( strContext == "sdfsd" )
			return;
		assert( false );
	}

	void SetName( const char* Name )
	{
		strcpy( szName, Name );
	}

	const char*	GetName()
	{
		return szName;
	}

	char		szName[32];
	uint32		nID;
	SAddress	Address;
	std::string	strContext;
};

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
	std::string				m_strName;
	IApplicationHandler*	m_pHandler;

public:

	static CApplication& GetInst();

	IApplicationHandler* TestCallObjectPointer( IApplicationHandler* Handler );
	SApplicationConfig& TestCallObjectReference( SApplicationConfig& Config );
	SApplicationConfig TestCallObjectValue( SApplicationConfig Config );

	virtual IApplicationHandler* TestVirtualObjectPointer( IApplicationHandler* Handler );
	virtual SApplicationConfig& TestVirtualObjectReference( SApplicationConfig& Config );
	virtual SApplicationConfig TestVirtualObjectValue( SApplicationConfig Config );

	const char* TestCallPOD( 
		ETestEnum e, int8 v0, int16 v1, int32 v2, int64 v3, long v4,
		uint8 v5, uint16 v6, uint32 v7, uint64 v8, unsigned long v9,
		float v10, double v11, const char* v12, const wchar_t* v13 );

	const char* TestNoParamFunction();
};