#ifndef __CALL_JS_H__
#define __CALL_JS_H__
//=====================================================================
// CCallBase.h 
// 定义基本的脚本和C++互相调用接口
// 柯达昭
// 2007-10-16
//=====================================================================

#include "core/CCallBase.h"
#include "CScriptJS.h"

using namespace std;

namespace Gamma
{
	//=====================================================================
	// JS脚本调用C++的接口
	//=====================================================================
	class CByScriptJS
	{
	public:
		static void CallByJS(CByScriptBase* pByScript, const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetByJS(CByScriptBase* pByScript, v8::Local<v8::Value> This, v8::ReturnValue<v8::Value> ret );
		static void SetByJS(CByScriptBase* pByScript, v8::Local<v8::Value> This, v8::Local<v8::Value> arg );
	};

	//=====================================================================
	// C++调用JS脚本的接口
	//=====================================================================
	class CCallBackJS : public CCallScriptBase
	{
		v8::Persistent<v8::String> m_strName;
	public:
		CCallBackJS(CScriptJS& Script, const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunName);
	protected:
		virtual bool    CallVM(SVirtualObj* pObject, void* pRetBuf, void** pArgArray);
		virtual void	DestrucVM(SVirtualObj* pObject);;
	};
};

#endif
