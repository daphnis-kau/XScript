#pragma once
#include "Help.h"
#include "Memory.h"
#include <functional>

namespace Gamma
{
	template<class Alloc, uint32 nPageSize>
	class TCircelBuffer 
	{
		struct SQuerryBuffer;
		typedef SQuerryBuffer* SQuerryNodePtr;

		struct SQuerryBufferHead
		{
			volatile uint32			m_nReadPos;
			volatile uint32			m_nWritePos;
			volatile SQuerryNodePtr	m_pNextBuffer;
		};

		enum { nNodeBufferSize = nPageSize - sizeof(SQuerryBufferHead) };

		struct SQuerryBuffer : public SQuerryBufferHead
		{
			tbyte					m_aryBuffer[nNodeBufferSize];
		};

		volatile SQuerryNodePtr		m_pReadBuffer;
		volatile SQuerryNodePtr		m_pWriteBuffer;
		uint64						m_nPushCount;  
		uint64						m_nPopCount; 
	public:
		TCircelBuffer();
		~TCircelBuffer();

		class CAtomWrite
		{
			friend class TCircelBuffer;
			SQuerryNodePtr pWriteBuffer;
			SQuerryNodePtr pReadBuffer;
			uint32 nWritePos;
		public:
			void Write( const void* pBuffer, uint32 nSize );
		};

		class CAtomRead
		{
			friend class TCircelBuffer;
			SQuerryNodePtr pReadBuffer;
			uint32 nReadPos;
			uint32 nTotalRead;
		public:
			uint32 Read( void* pBuffer, uint32 nSize );
		};

		void						PushBuffer( std::function<void( CAtomWrite& )> funWrite );
		uint32						PopBuffer( std::function<bool( CAtomRead& )> funRead );

		template<typename Type>
		void						Push( const Type& value );
		template<typename Type>	
		uint32						Pop( Type& value );
	
		bool						CanPop() const;
		uint32						GetWaitingBufferCount() const;
	};

	typedef TCircelBuffer< TFixedPageAlloc<8192>, 8192 > CCircelBuffer;
}

#include "TCircelBuffer.inl"