#include "common/Memory.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

using namespace XS;

namespace XS
{
	//--------------------------------------------------------------------------
	// 操作系统相关
	//--------------------------------------------------------------------------
	uint32 GetVirtualPageSize()
	{
#ifdef _WIN32
		struct __ : public SYSTEM_INFO { __() { GetSystemInfo( this ); }; };
		static size_t pagesize = __().dwPageSize;
#else
		static size_t pagesize = (size_t)sysconf(_SC_PAGESIZE);
#endif
		return (uint32)pagesize;
	}

	void* ReserveMemoryPage( void* pAddress, size_t nSize )
	{
#ifdef _WIN32
		return VirtualAlloc( pAddress, nSize, MEM_RESERVE, PAGE_NOACCESS );
#else
		void* pReserve = (char*)mmap( pAddress, nSize, PROT_NONE, 
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );
		if( pReserve == MAP_FAILED ) 
			return NULL;

		if( pAddress && pAddress != pReserve ) 
		{
			// 如果地址不一样，释放掉，保证和windows行为一样
            FreeMemoryPage( pReserve, nSize );
			return NULL;
		}
		return pReserve;
#endif // _WIN32
	}

	bool FreeMemoryPage( void* pAddress, size_t nSize )
	{
#ifdef _WIN32
		return VirtualFree( pAddress, 0, MEM_RELEASE ) != 0;
#else
		return munmap( pAddress, nSize ) == 0;
#endif
	}

	bool CommitMemoryPage( void* pAddress, size_t nSize, uint32 nProtectFlag )
	{
#ifdef _WIN32
		if( nProtectFlag&VIRTUAL_PAGE_WRITE )
			nProtectFlag |= VIRTUAL_PAGE_READ;

		if( nProtectFlag == VIRTUAL_PAGE_READ )
			nProtectFlag = PAGE_READONLY;
		else if( nProtectFlag == ( VIRTUAL_PAGE_READ|VIRTUAL_PAGE_WRITE ) )
			nProtectFlag = PAGE_READWRITE;
		else if( nProtectFlag == VIRTUAL_PAGE_EXECUTE )
			nProtectFlag = PAGE_EXECUTE;
		else if( nProtectFlag == ( VIRTUAL_PAGE_EXECUTE|VIRTUAL_PAGE_READ ) )
			nProtectFlag = PAGE_EXECUTE_READ;
		else if( nProtectFlag == ( VIRTUAL_PAGE_EXECUTE|VIRTUAL_PAGE_READ|VIRTUAL_PAGE_WRITE ) )
			nProtectFlag = PAGE_EXECUTE_READWRITE;

		bool bSuccess = false;
		MEMORY_BASIC_INFORMATION mbi;	
		do 
		{
			VirtualQuery( pAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION) );
			size_t nCommitSize = nSize > mbi.RegionSize ? mbi.RegionSize : nSize;
			bSuccess = VirtualAlloc( pAddress, nCommitSize, MEM_COMMIT, nProtectFlag ) != NULL;
			pAddress = (char*)pAddress + nCommitSize;
			nSize -= nCommitSize;
		} while( nSize > 0 && bSuccess );

		return bSuccess;
#else
		int32 nProtect = 0;
		if( nProtectFlag&VIRTUAL_PAGE_READ )
			nProtect |= PROT_READ;
		if( nProtectFlag&VIRTUAL_PAGE_WRITE )
			nProtect |= PROT_WRITE;
		if( nProtectFlag&VIRTUAL_PAGE_EXECUTE )
			nProtect |= PROT_EXEC;
		int32 nFlag = MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS;
		void* pResult = mmap( pAddress, nSize, nProtect, nFlag, -1, 0 );
		if( pResult == pAddress )
			return true;
		//int32 nError = errno;
		return false;
#endif // _WIN32
	}

	bool DecommitMemoryPage( void* pAddress, size_t nSize )
	{
#ifdef _WIN32
		bool bSuccess = false;
		MEMORY_BASIC_INFORMATION mbi;	
		do 
		{
			VirtualQuery( pAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
			size_t nCommitSize = nSize > mbi.RegionSize ? mbi.RegionSize : nSize;
			bSuccess = VirtualFree( pAddress, nCommitSize, MEM_DECOMMIT ) == TRUE;
			pAddress = (char*)pAddress + nCommitSize;
			nSize -= nCommitSize;
		} while( nSize > 0 && bSuccess );

		return bSuccess;
#else
		int32 nFlag = MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS;
		return mmap( pAddress, nSize, PROT_NONE, nFlag, -1, 0 ) == pAddress;
#endif // _WIN32
	}
}

