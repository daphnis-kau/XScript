#include "common/CVirtualFun.h"
#include "common/CThread.h"
#include <stdlib.h>
#include <setjmp.h>

#ifdef _WIN32
#include <windows.h>
#elif(defined _IOS)
#include <mach/vm_map.h>
#include <mach/mach.h>
#include <sys/mman.h>
#endif

#pragma warning( disable: 4611 )

namespace Gamma
{
	class CCheckMemoryExecutable
	{
	public:
#ifdef _WIN32
		bool IsExecutable( void* pAddress )
		{
			if( !pAddress )
				return false;
			MEMORY_BASIC_INFORMATION Info;
			VirtualQuery( pAddress, &Info, sizeof(Info) );
			return Info.Protect == PAGE_EXECUTE ||
				Info.Protect == PAGE_EXECUTE_READ ||
				Info.Protect == PAGE_EXECUTE_READWRITE ||
				Info.Protect == PAGE_EXECUTE_WRITECOPY;
		}
#elif( defined _IOS )
		bool IsExecutable( void* pAddress )
		{
			if( !pAddress )
				return false;
			return true;
			/*
			mach_port_t task;
			vm_size_t region_size = 0;
			vm_address_t region = (vm_address_t)pAddress;
			
#if defined(_MAC64) || defined(__LP64__)
			vm_region_basic_info_data_64_t info;
			mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT_64;
			vm_region_flavor_t flavor = VM_REGION_BASIC_INFO_64;
			if( vm_region_64(mach_task_self(), &region, &region_size, flavor, (vm_region_info_t)&info, (mach_msg_type_number_t*)&info_count, (mach_port_t*)&task) != KERN_SUCCESS )
				return true;
#else
			vm_region_basic_info_data_t info;
			mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT;
			vm_region_flavor_t flavor = VM_REGION_BASIC_INFO;
			if( vm_region(mach_task_self(), &region, &region_size, flavor, (vm_region_info_t)&info, (mach_msg_type_number_t*)&info_count, (mach_port_t*)&task) != KERN_SUCCESS )
				return true;
#endif
			if( info.protection&( PROT_EXEC ) )
				return true;
			return false;*/
		}
#else
		/*
		std::map< void*, void* > m_mapExecutable;
		CCheckMemoryExecutable()
		{
			FILE* fp;
			uint32 nPID = (uint32)getpid();
			char szBuffer[2048];
			gammasstream( szBuffer ) << "/proc/" << nPID << "/maps";
			if( NULL != ( fp = fopen( szBuffer, "r" ) ) )
			{
				while ( fgets( szBuffer, 2048, fp ) )
				{
					uint32 nIndex = 0;
					const char* szStart = szBuffer;
					while( nIndex < 2048 && szBuffer[nIndex] != '-' )
						nIndex++;
					const char* szEnd = szBuffer + ( ++nIndex );
					while( nIndex < 2048 && !IsBlank( szBuffer[nIndex] ) )
						nIndex++;
					if( szBuffer[nIndex + 3] != 'x' )
						continue;
					void* pStart = (void*)strtoll( szStart, NULL, 16 );		
					m_mapExecutable[pStart] = (void*)strtoll( szEnd, NULL, 16 );	
				}
				fclose( fp );
			}
		}*/

		bool IsExecutable( void* pAddress )
		{
			if( !pAddress )
				return false;
			/*
			if( m_mapExecutable.empty() )
				return true;
			std::map<void*, void*>::iterator it = m_mapExecutable.lower_bound( pAddress );
			return it != m_mapExecutable.end() && pAddress < it->second;*/
			return true;
		}
#endif
	};

	//=====================================================================
	// 函数表初始化
	//=====================================================================
	void NullFunCall(){ throw( "Can not call a invalid function!"); }

	SFunctionTable::SFunctionTable()
	{
		for( int32 i = 0; i < MAX_VTABLE_SIZE; i++ )
			m_pFun[i] = (void*)&NullFunCall;
	}

	int32 SFunctionTable::GetFunctionCount()
	{        
		CCheckMemoryExecutable Checker;
		for( int32 n = 0; n < MAX_VTABLE_SIZE; n++ )
			if( !Checker.IsExecutable( m_pFun[n] ) )
				return n;
		return MAX_VTABLE_SIZE;
	}

	//=====================================================================
	// 获取虚表索引
	//=====================================================================
	struct SGlobalContext
	{
		jmp_buf m_JumpFlag;
		CLock	m_Lock;
	};

	static SGlobalContext& GetGlobalContext()
	{
		static SGlobalContext s_Instance;
		return s_Instance;
	}

	// 将得到索引的函数赋值到虚函数表
	template<uint32 nStart, uint32 nCount>
	class TSetFuntion
	{
		enum
		{ 
			nLStart = nStart, 
			nLCount = nCount/2, 
			nRStart = nLStart + nLCount, 
			nRCount = nCount - nLCount,
		};

	public:
		TSetFuntion( void** pChechFun )
		{ 
			TSetFuntion<nLStart, nLCount> Left( pChechFun );
			TSetFuntion<nRStart, nRCount> Right( pChechFun );
		}
	};

	template<uint32 nStart>
	class TSetFuntion<nStart, 1>
	{	
		static void GetIndex() { longjmp(GetGlobalContext().m_JumpFlag, nStart + 1); }
	public:
		TSetFuntion( void** pChechFun )
		{ pChechFun[nStart] = (void*)&TSetFuntion<nStart, 1>::GetIndex; }
	};

	uint32 FindVirtualFunction( uint32 nSize,
		VirtualFunCallback funCallback, void* pContext )
	{
		static SFunctionTable FunctionTable;
		static TSetFuntion<0, MAX_VTABLE_SIZE> s_FunIndex( FunctionTable.m_pFun );
		static SGlobalContext& Context = GetGlobalContext();
		uint32 nAllocSize, nIndex, i, j;
		Context.m_Lock.Lock();
		nAllocSize = AligenUp( (uint32)nSize, sizeof( void* ) );
		void** pObj = (void**)alloca( nAllocSize );
		for( i = 0, j = 0; i < nSize; i += sizeof( void* ) )
			pObj[j++] = &FunctionTable;
		nIndex = setjmp( Context.m_JumpFlag );
		if( nIndex == 0 )
			funCallback( pObj, pContext );
		Context.m_Lock.Unlock();
		return nIndex - 1;
	}
}
