#ifndef __XS_SCRIPT_DEF_H__
#define __XS_SCRIPT_DEF_H__
//=====================================================================
// XScriptDef.h 
// 定义脚本和C++接口的基本数据类型
// 柯达昭
// 2007-10-16
//=====================================================================
#include "common/CppTypeParser.h"
#include <string>

namespace XS
{
#define MAX_BASE_CLASS_NUM 5
#define MAX_FUNCTION_PARAM_NUM 10

	class CScriptBase;

	struct STypeInfoArray
	{
		STypeInfo*	aryInfo;
		uint32		nSize;
	};

	class IObjectConstruct
	{
	public:
		virtual void		Assign( void*, void* ) = 0;
		virtual void		Construct( void* ) = 0;
		virtual void		Destruct( void* ) = 0;
	};

	class IFunctionWrap
	{
	public:
		virtual void		Call( void* pRetBuf, void** pArgArray, uintptr_t funContext ) = 0;
	};
}


#endif
