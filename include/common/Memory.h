/**@file  		Memory.h
* @brief		memory interface
* @author		Daphnis Kau
* @date			2020-01-17
* @version		V1.0
*/

#ifndef __XS_MEMORY_H__
#define __XS_MEMORY_H__

#include "common/CommonType.h"

namespace XS
{
	#define VIRTUAL_PAGE_READ		0x01
	#define VIRTUAL_PAGE_WRITE		0x02
	#define VIRTUAL_PAGE_EXECUTE	0x04

	/**
	* @brief Get virtual memory page size of current system
	* @return Memory page size
	*/
	uint32 GetVirtualPageSize();

	/**
	* @brief Reserve memory address in [pAddress, pAddress + nSize)
	* @param [in] pAddress begin memory address needed to be reserved
	* @param [in] nSize memory size needed to be reserved
	* @note the reserved memory will be align down to the page's begin
	*	and align up to the pages' end
	*/
	void* ReserveMemoryPage( void* pAddress, size_t nSize );

	/**
	* @brief Free the reserved memory address in [pAddress, pAddress + nSize)
	* @param [in] pAddress begin memory address needed to be reserved
	* @param [in] nSize memory size needed to be reserved
	* @note the reserved memory will be align down to the page's begin
	*	and align up to the pages' end
	*/
	bool FreeMemoryPage( void* pAddress, size_t nSize );
	
	/**
	* @brief Alloc physical memory to reserved memory with protect flag \n
	*	in [pAddress, pAddress + nSize)
	* @param [in] pAddress begin memory address needed to be reserved
	* @param [in] nSize memory size needed to be reserved
	* @param [in] nProtectFlag protect flag for committed memory
	* @note the memory will be align down to the page's begin
	*	and align up to the pages' end
	*/
	bool CommitMemoryPage( void* pAddress, size_t nSize, uint32 nProtectFlag );

	/**
	* @brief Free physical memory from reserved memory in [pAddress, pAddress + nSize)
	* @param [in] pAddress begin memory address needed to be reserved
	* @param [in] nSize memory size needed to be reserved
	* @note the memory will be align down to the page's begin
	*	and align up to the pages' end
	*/
	bool DecommitMemoryPage( void* pAddress, size_t nSize );
}

#endif
