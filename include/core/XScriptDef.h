/**@file  		XScriptDef.h
* @brief		Base interface of XScript
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
*/

#ifndef __XS_SCRIPT_DEF_H__
#define __XS_SCRIPT_DEF_H__
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
		virtual void			Assign( void*, void* ) = 0;
		virtual void			CopyConstruct( void*, void* ) = 0;
		virtual void			Construct( void*, void** ) = 0;
		virtual void			Destruct( void* ) = 0;
		virtual STypeInfoArray	GetFunArg() = 0;
	};

	class IFunctionWrap
	{
	public:
		virtual void			Call( void* pRetBuf, void** pArgArray, uintptr_t funContext ) = 0;
	};
}


#endif
