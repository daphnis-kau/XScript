#ifndef __CALL_JS_H__
#define __CALL_JS_H__
//=====================================================================
// CCallBase.h 
// 定义基本的脚本和C++互相调用接口
// 柯达昭
// 2007-10-16
//=====================================================================

#include "core/CCallBase.h"
#include "CTypeJS.h"

using namespace std;

namespace Gamma
{
	class CScriptJS;

	extern CJSTypeBase* s_aryJSType[eDT_count];
	inline CJSTypeBase* GetTypeBase(DataType eType)
	{
		if (eType < eDT_count)
			return s_aryJSType[eType];
		if (eType & 1)
			return &CJSObject::GetInst();
		return &CJSValueObject::GetInst();
	}

	//=====================================================================
	// JS脚本调用C++的接口
	//=====================================================================
	class CByScriptJS
	{
	public:
		static void CallByJS(CScriptJS* pScript, CByScriptBase* pByScript,
			const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetByJS(CScriptJS* pScript, CByScriptBase* pByScript,
			v8::Local<v8::Value> This, v8::ReturnValue<v8::Value> ret );
		static void SetByJS(CScriptJS* pScript, CByScriptBase* pByScript,
			v8::Local<v8::Value> This, v8::Local<v8::Value> arg );
	};

	//=====================================================================
	// C++调用JS脚本的接口
	//=====================================================================
	class CCallBackJS : public CCallScriptBase
	{
	protected:
		static bool CallVM(CScriptJS* pScript,
			CCallScriptBase* pCallBase, SVirtualObj* pObject, void* pRetBuf, void** pArgArray);
		static void DestrucVM(CScriptJS* pScript,
			CCallScriptBase* pCallBase, SVirtualObj* pObject);;
	};
};

#endif
