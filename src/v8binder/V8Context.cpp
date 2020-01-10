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
}