#ifndef __GAMMA_SCRIPT_H__
#define __GAMMA_SCRIPT_H__
//=====================================================================
// CScript.h
// 定义脚本和C++接口
// 柯达昭
// 2007-10-16
//=====================================================================

#include <string>
#include "core/GammaScriptDef.h"
#include "common/GammaCpp.h"
namespace Gamma
{
    class GAMMA_SCRIPT_API CScript
    {
    public:
        CScript( EScriptVM eScriptVM, uint16 nDebugPort = 0 );
        ~CScript();

		/// 得到原始虚拟机
		void*				GetVM( EScriptVM eScriptVM );
        /// 得到Script数据
        HSCRIPT             GetScript() const;
        /// 调用脚本文件
		bool                RunFile( const char* szFileName, bool bReload );
		/// 调用脚本文件
		bool                RunBuffer( const void* pBuffer, size_t nSize );
		/// 执行一段代码
		bool                RunString( const char* szString );
		/// 执行一个函数
		bool                RunFunction( const STypeInfoArray& aryTypeInfo, void* pResultBuf, const char* szFunction, void** aryArg );

        /// 设置搜索路径
        void                AddSearchPath( const char* szPath );
        /// 注册单个被脚本调用的函数
        void                RegistFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName );
		/// 注册静态成员函数
		void                RegistClassStaticFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName );
        /// 注册类成员函数
        void                RegistClassFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName );
        /// 注册类回调函数
        ICallBackWrap&		RegistClassCallback( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName );
        /// 注册类成员数据
		void                RegistClassMember( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funGetSet[2], const char* szTypeInfoName, const char* szMemberName );
		/// 注册类
		void                RegistClass( uint32 nSize, const char* szTypeIDName, const char* szClass, ... );
		/// 注册类构造析构
		void                RegistConstruct( IObjectConstruct* pObjectConstruct, const char* szTypeIDName );
		/// 注册析构函数
		ICallBackWrap&		RegistDestructor( const char* szTypeInfoName, IFunctionWrap* funWrap );
        /// 注册枚举类型            
        void                RegistEnum( const char* szTypeIDName, const char* szTableName, int32 nTypeSize );
        /// 注册常量或者枚举类型的项            
		void                RegistConstant( const char* szTableName, const char* szFeild, int32  nValue );
		/// 注册常量或者枚举类型的项            
		void				RegistConstant( const char* szTableName, const char* szFeild, double dValue );
		/// 注册常量或者枚举类型的项            
		void				RegistConstant( const char* szTableName, const char* szFeild, const char* szValue );
		/// 基类是否注册了析构函数
		bool				IsDestructorRegisted( const char* szTypeInfoName );
        /// 增加引用计数
        void                RefScriptObj( void* pObj );
        /// 减少引用计数
        void                UnrefScriptObj( void* pObj );
		/// 垃圾回收
		void                GC();
        /// 垃圾回收
		void                GCAll();

    private:
        HSCRIPT             m_hScript;
		EScriptVM			m_eScriptType;
    };

	/// 取消对象和script的连接关系
	GAMMA_SCRIPT_API void	UnlinkCppObjFromScript( void* pObj );
	/// 对象对虚拟机的回调
	GAMMA_SCRIPT_API int32	CallBack( int32 nIndex, void* pObject, void* pRetBuf, void** pArgArray );
}

#endif
