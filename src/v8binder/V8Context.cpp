#include "V8Context.h"

#define MAX_STRING_BUFFER_SIZE	65536

namespace Gamma
{
	SV8Context::SV8Context()
		: m_platform(nullptr)
		, m_pIsolate(nullptr)
		, m_nStringID(0)
		, m_pTempStrBuffer64K(new tbyte[MAX_STRING_BUFFER_SIZE])
		, m_nCurUseSize(0)
		, m_nStrBufferStack(0)
	{
	}

	void SV8Context::CallJSStatck(bool bAdd)
	{
		if (bAdd)
		{
			if (m_nStrBufferStack == 0)
				ClearCppString((void*)(uintptr_t)(-1));
			++m_nStrBufferStack;
		}
		else
		{
			assert(m_nStrBufferStack);
			--m_nStrBufferStack;
		}
	}

	void SV8Context::ClearCppString(void* pStack)
	{
		uint32 nIndex = (uint32)m_vecStringInfo.size();
		while (nIndex && m_vecStringInfo[nIndex - 1].m_pStack < pStack)
			delete[] m_vecStringInfo[--nIndex].m_pBuffer;
		if (nIndex < m_vecStringInfo.size())
			m_vecStringInfo.erase(m_vecStringInfo.begin() + nIndex, m_vecStringInfo.end());

		while (m_nCurUseSize >= sizeof(SStringFixed))
		{
			SStringFixed* pFixeString = ((SStringFixed*)(m_pTempStrBuffer64K + m_nCurUseSize)) - 1;
			if (pFixeString->m_pStack >= pStack)
				break;
			m_nCurUseSize -= (uint32)(sizeof(SStringFixed) + pFixeString->m_nLen);
		}
	}

	LocalValue SV8Context::StringFromUtf8(const char* szUtf8)
	{
		if (!szUtf8)
			return v8::Null(m_pIsolate);
		return v8::String::NewFromUtf8(m_pIsolate, szUtf8);
	}

	LocalValue SV8Context::StringFromUcs(const wchar_t* szUcs)
	{
		if (!szUcs)
			return v8::Null(m_pIsolate);
		if (sizeof(wchar_t) == sizeof(uint16_t))
			return v8::String::NewFromTwoByte(m_pIsolate,
			(uint16_t*)szUcs, v8::NewStringType::kNormal).ToLocalChecked();
		m_szTempUcs2 = szUcs;
		size_t nSize = m_szTempUcs2.size();
		uint16_t* szDes = (uint16_t*)&m_szTempUcs2[0];
		for (size_t i = 0; i < nSize; i++)
			szDes[i] = m_szTempUcs2[i];
		szDes[nSize] = 0;
		return v8::String::NewFromTwoByte(m_pIsolate,
			(uint16_t*)szDes, v8::NewStringType::kNormal).ToLocalChecked();
	}

	const char* SV8Context::StringToUtf8(LocalValue obj)
	{
		if (obj == v8::Null(m_pIsolate))
			return NULL;
		v8::Local<v8::Context> context = m_pIsolate->GetCurrentContext();
		v8::MaybeLocal<v8::String> v = obj->ToString(context);
		if (v.IsEmpty())
			return NULL;
		v8::Local<v8::String> StringObject = v.ToLocalChecked();
		size_t nStrLen = StringObject->Utf8Length(m_pIsolate);
		if (nStrLen == 0)
			return "";

		size_t nAllocSize = AligenUp((uint32)(nStrLen + 1), sizeof(void*));
		if (nAllocSize + m_nCurUseSize + sizeof(SStringFixed) < MAX_STRING_BUFFER_SIZE)
		{
			char* szUtf8 = (char*)(m_pTempStrBuffer64K + m_nCurUseSize);
			StringObject->WriteUtf8(m_pIsolate, szUtf8);
			SStringFixed strCpp;
			strCpp.m_pStack = &strCpp;
			strCpp.m_nLen = AligenUp((uint32)(nStrLen + 1), sizeof(void*));
			m_nCurUseSize += strCpp.m_nLen + sizeof(SStringFixed);
			memcpy(m_pTempStrBuffer64K + m_nCurUseSize - sizeof(SStringFixed), &strCpp, sizeof(SStringFixed));
			return szUtf8;
		}
		else
		{
			SStringDynamic strCpp;
			strCpp.m_pStack = &strCpp;
			strCpp.m_pBuffer = new char[nStrLen + 1];
			StringObject->WriteUtf8(m_pIsolate, (char*)strCpp.m_pBuffer);
			m_vecStringInfo.push_back(strCpp);
			return (char*)strCpp.m_pBuffer;
		}
	}

	const wchar_t* SV8Context::StringToUcs(LocalValue obj)
	{
		if (obj == v8::Null(m_pIsolate))
			return NULL;
		v8::Local<v8::Context> context = m_pIsolate->GetCurrentContext();
		v8::MaybeLocal<v8::String> v = obj->ToString(context);
		if (v.IsEmpty())
			return NULL;
		v8::Local<v8::String> StringObject = v.ToLocalChecked();
		size_t nStrLen = StringObject->Utf8Length(m_pIsolate);
		if (nStrLen == 0)
			return L"";

		uint32 nAllocSize = AligenUp(uint32(nStrLen + 1) * sizeof(wchar_t), sizeof(void*));
		wchar_t* szUcs2 = NULL;
		if (nAllocSize + m_nCurUseSize < MAX_STRING_BUFFER_SIZE)
		{
			szUcs2 = (wchar_t*)(m_pTempStrBuffer64K + m_nCurUseSize);
			StringObject->Write(m_pIsolate, (uint16_t*)szUcs2);
			szUcs2[nStrLen] = 0;
			SStringFixed strCpp;
			strCpp.m_pStack = &strCpp;
			strCpp.m_nLen = nAllocSize;
			m_nCurUseSize += strCpp.m_nLen + sizeof(SStringFixed);
			memcpy(m_pTempStrBuffer64K + m_nCurUseSize - sizeof(SStringFixed), &strCpp, sizeof(SStringFixed));
		}
		else
		{
			SStringDynamic strCpp;
			strCpp.m_pStack = &strCpp;
			strCpp.m_pBuffer = szUcs2 = new wchar_t[nStrLen + 1];
			StringObject->Write(m_pIsolate, (uint16_t*)szUcs2);
			m_vecStringInfo.push_back(strCpp);
		}

		if (sizeof(wchar_t) == sizeof(uint16_t))
			return szUcs2;
		uint16_t* szSrc = (uint16_t*)szUcs2;
		while (nStrLen--)
			szUcs2[nStrLen] = szSrc[nStrLen];
		return szUcs2;
	}

	void SV8Context::ReportException(v8::TryCatch* try_catch, Local<Context> context)
	{
		v8::Local<v8::Message> message = try_catch->Message();
		v8::String::Utf8Value exception(m_pIsolate, try_catch->Exception());
		if (message.IsEmpty())
		{
			// V8 didn't provide any extra information about this error; just
			// print the exception.
			Output("Error:", -1);
			Output(*exception, -1);
			Output("\n", -1);
			return;
		}

		char szNumber[32];
		// Print (filename):(line number): (message).
		v8::String::Utf8Value filename(m_pIsolate, message->GetScriptResourceName());
		sprintf(szNumber, "%d", message->GetLineNumber(context).ToChecked());
		Output(*filename, -1);
		Output(":", -1);
		Output(szNumber, -1);
		Output("\n\t", -1);
		Output(*exception, -1);
		Output("\n", -1);

		// Print line of source code.
		v8::Local<String> souceline = message->GetSourceLine(context).ToLocalChecked();
		Output(*String::Utf8Value(m_pIsolate, souceline), -1);
		Output("\n", -1);

		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn();
		for (int i = 0; i < start; i++)
			Output(" ", -1);
		int end = message->GetEndColumn();
		for (int i = start; i < end; i++)
			Output(" ^", -1);
		Output("\n", -1);
		v8::MaybeLocal<Value> backTrace = try_catch->StackTrace(context);
		if (backTrace.IsEmpty())
			return;
		v8::Local<Value> strBackTrace = backTrace.ToLocalChecked();
		v8::String::Utf8Value stack_trace(m_pIsolate, strBackTrace->ToString(m_pIsolate));
		if (!stack_trace.length())
			return;
		Output(*stack_trace, -1);
	}
}