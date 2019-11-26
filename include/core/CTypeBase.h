#ifndef __TYPE_BASE_H__
#define __TYPE_BASE_H__
//=====================================================================
/** CTypeBase.h
  定义基本的数据类型
  柯达昭
  2007-10-16
*/
//=====================================================================

#include "core/GammaScriptDef.h"
#include <vector>

namespace Gamma
{
	// 特别注意，为了处理方便，将eDT_custom_type类型定义为buffer
	typedef ptrdiff_t DataType;

	DataType ToDataType( const STypeInfo& argTypeInfo );
	size_t	 GetSizeOfType( DataType nType );
	size_t	 GetAligenSizeOfType( DataType nType );
	size_t	 CalBufferSize( const std::vector<DataType>& aryParam, size_t arySize[] );
}

#endif
