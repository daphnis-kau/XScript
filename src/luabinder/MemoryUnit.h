//=========================================================================================
// MemoryUnit.h  
// 定义内存池分类
// 柯达昭
// 2020-07-18
//=========================================================================================
#ifndef __XMEMORY_MEMORYUNIT_H__
#define __XMEMORY_MEMORYUNIT_H__
#include "common/Help.h"

#include <stdint.h>
#include <stdlib.h>

namespace XS
{
	template<int32_t PageSize, int32_t UnitStep, int32_t UnitSize>
	class TUnitClassSize
	{
		enum
		{
			eNextMinCount = PageSize/UnitSize + 1,
			eNextCountBySize = PageSize/( UnitSize - UnitStep ),
			eNextUnitCount = TMax<eNextMinCount, eNextCountBySize>::eValue,
			eNextUnitRawSize = PageSize/eNextUnitCount,
			eNextUnitSize = TAligenDown<eNextUnitRawSize, UnitStep>::eValue,
		};

		typedef TUnitClassSize<PageSize, UnitStep, eNextUnitSize> NextUnitSize;

	public:
		enum { eUnitIndex = NextUnitSize::eUnitIndex + 1 };

		static void BuildClassSize( uint32_t aryClassSize[] )
		{
			NextUnitSize::BuildClassSize( aryClassSize );
			aryClassSize[eUnitIndex] = UnitSize;
		}
	};

	template<int32_t PageSize, int32_t UnitStep>
	class TUnitClassSize<PageSize, UnitStep, UnitStep>
	{
	public:
		enum { eUnitIndex = 0 };

		static void BuildClassSize( uint32_t aryClassSize[] )
		{
			aryClassSize[eUnitIndex] = UnitStep;
		}
	};

	// 通过模板计算分配器类型数量
	template<int32_t PageSize, int32_t UnitStep, int32_t MaxUnitSize>
	struct TMemoryUnitInfo
	{
		typedef TUnitClassSize<PageSize, UnitStep, MaxUnitSize> MaxSizeClass;
		enum { eUnitClassCount = MaxSizeClass::eUnitIndex + 1 };
		uint32_t m_aryClassSize[eUnitClassCount];
		uint8_t m_aryClassIndex[MaxUnitSize / UnitStep];

		TMemoryUnitInfo()
		{
			MaxSizeClass::BuildClassSize( m_aryClassSize );
			for( uint32_t i = 0, n = 1; i < eUnitClassCount; i++ )
			{
				while( n * UnitStep <= m_aryClassSize[i] )
				{
					uint32_t nSize = n++ * UnitStep;
					m_aryClassIndex[(nSize - 1) / UnitStep] = (uint8_t)i;
				}
			}
		}
	};
}

#endif