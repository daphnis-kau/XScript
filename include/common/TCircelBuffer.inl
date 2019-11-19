
namespace Gamma
{
	template<class Alloc, uint32 nPageSize>
	TCircelBuffer<Alloc, nPageSize>::TCircelBuffer()
		: m_pReadBuffer( NULL )
		, m_pWriteBuffer( NULL )
		, m_nPushCount( 0 )
		, m_nPopCount( 0 )
	{
		assert( nPageSize == sizeof(SQuerryBuffer) );
		m_pWriteBuffer = (SQuerryBuffer*)Alloc::Alloc();
		m_pWriteBuffer->m_pNextBuffer = (SQuerryBuffer*)Alloc::Alloc();
		m_pWriteBuffer->m_pNextBuffer->m_pNextBuffer = m_pWriteBuffer;
		m_pReadBuffer = m_pWriteBuffer;
		m_pWriteBuffer->m_nWritePos = 0;
		m_pWriteBuffer->m_nReadPos = 0;
	}

	template<class Alloc, uint32 nPageSize>
	TCircelBuffer<Alloc, nPageSize>::~TCircelBuffer()
	{
		SQuerryBuffer* pBuffer = m_pWriteBuffer;

		while( true )
		{
			SQuerryBuffer* pNextBuffer = pBuffer->m_pNextBuffer;
			Alloc::Free( pBuffer );
			if( pNextBuffer == m_pWriteBuffer )
				break;
			pBuffer = pNextBuffer;
		}
		m_pReadBuffer = NULL;
		m_pWriteBuffer = NULL;
	}

	template<class Alloc, uint32 nPageSize>
	inline void TCircelBuffer<Alloc, nPageSize>::CAtomWrite::
		Write( const void* pBuffer, uint32 nSize )
	{
		const tbyte* pCurBuffer = (const tbyte*)pBuffer;

		while( nSize )
		{
			uint32 nLeftSize = nNodeBufferSize - nWritePos;
			uint32 nWriteSize = Min( nSize, nLeftSize );
			tbyte* pTargetBuf = pWriteBuffer->m_aryBuffer + nWritePos;
			memcpy( pTargetBuf, pCurBuffer, nWriteSize );
			pCurBuffer += nWriteSize;
			nWritePos += nWriteSize;
			nSize -= nWriteSize;

			if( nWritePos != nNodeBufferSize )
				break;

			if( pWriteBuffer->m_pNextBuffer == pReadBuffer )
			{
				SQuerryNodePtr pNewBuffer = (SQuerryBuffer*)Alloc::Alloc();
				pNewBuffer->m_pNextBuffer = pReadBuffer;
				pWriteBuffer->m_pNextBuffer = pNewBuffer;
			}

			pWriteBuffer = pWriteBuffer->m_pNextBuffer;
			nWritePos = 0;
		}
	}

	template<class Alloc, uint32 nPageSize>
	inline uint32 TCircelBuffer<Alloc, nPageSize>::CAtomRead::
		Read( void* pBuffer, uint32 nSize )
	{
		tbyte* pCurBuffer = (tbyte*)pBuffer;
		while( nSize )
		{
			if( nReadPos >= pReadBuffer->m_nWritePos )
				return (uint32)( pCurBuffer - (tbyte*)pBuffer );
			uint32 nLeftSize = nNodeBufferSize - nReadPos;
			uint32 nReadSize = Min( nSize, nLeftSize );
			const tbyte* pSrcBuf = pReadBuffer->m_aryBuffer + nReadPos;
			memcpy( pCurBuffer, pSrcBuf, nReadSize );
			pCurBuffer += nReadSize;
			nReadPos += nReadSize;
			nSize -= nReadSize;
			nTotalRead += nReadSize;

			if( nReadPos != nNodeBufferSize )
				break;
			pReadBuffer = pReadBuffer->m_pNextBuffer;
			nReadPos = 0;
		}
		return (uint32)( pCurBuffer - (tbyte*)pBuffer );
	}

	template<class Alloc, uint32 nPageSize>
	inline void TCircelBuffer<Alloc, nPageSize>::PushBuffer( 
		std::function<void( CAtomWrite& )> funWrite )
	{
		CAtomWrite Writer;
		SQuerryNodePtr pBufferStart = m_pWriteBuffer;
		Writer.pWriteBuffer = m_pWriteBuffer;
		Writer.pReadBuffer = m_pReadBuffer;
		Writer.nWritePos = pBufferStart->m_nWritePos;
		funWrite( Writer );

		// 保证原子操作，一次query完整写入
		while( pBufferStart != Writer.pWriteBuffer )
		{
			pBufferStart->m_nWritePos = nNodeBufferSize;
			pBufferStart = pBufferStart->m_pNextBuffer;
		}
		Writer.pWriteBuffer->m_nWritePos = Writer.nWritePos;
		m_pWriteBuffer = Writer.pWriteBuffer;
		m_nPushCount++;
	}

	template<class Alloc, uint32 nPageSize>
	inline uint32 TCircelBuffer<Alloc, nPageSize>::PopBuffer(
		std::function<bool( CAtomRead& )> funRead )
	{
		if( m_pWriteBuffer->m_nWritePos == nNodeBufferSize )
			return 0;

		CAtomRead Reader;
		Reader.pReadBuffer = m_pReadBuffer;
		Reader.nReadPos = m_pReadBuffer->m_nReadPos;
		Reader.nTotalRead = 0;
		if( Reader.pReadBuffer == m_pWriteBuffer &&
			Reader.nReadPos == m_pWriteBuffer->m_nWritePos )
			return 0;

		if(!funRead(Reader))
			return 0;

		// 保证原子操作，一次query完整读出
		Reader.pReadBuffer->m_nReadPos = Reader.nReadPos;
		m_pReadBuffer = Reader.pReadBuffer;
		m_nPopCount++;
		return Reader.nTotalRead;
	}

	template<class Alloc, uint32 nPageSize>
	template<typename Type>
	void TCircelBuffer<Alloc, nPageSize>::Push( const Type& value )
	{
		PushBuffer( [&]( CAtomWrite& Writer )->void 
		{ Writer.Write( &value, sizeof( Type ) ); } );
	}

	template<class Alloc, uint32 nPageSize>
	template<typename Type>
	uint32 TCircelBuffer<Alloc, nPageSize>::Pop( Type& value )
	{
		return PopBuffer( [&]( CAtomRead& Reader )->bool
		{ return Reader.Read( &value, sizeof( Type ) ); } );
	}

	template<class Alloc, uint32 nPageSize>
	bool Gamma::TCircelBuffer<Alloc, nPageSize>::CanPop() const
	{
		if( m_pWriteBuffer->m_nWritePos == nNodeBufferSize )
			return false;
		uint32 nReadPos = m_pReadBuffer->m_nReadPos;
		if( m_pReadBuffer == m_pWriteBuffer && 
			nReadPos == m_pWriteBuffer->m_nWritePos )
			return false;
		return true;
	}

	template<class Alloc, uint32 nPageSize>
	uint32 Gamma::TCircelBuffer<Alloc, nPageSize>::GetWaitingBufferCount() const
	{
		return (uint32)( m_nPushCount - m_nPopCount );
	}
}
