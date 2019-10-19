#include <typeinfo>
#include "core/CScript.h"
#include "core/CScriptBase.h"
#include "core/CCallBase.h"

namespace Gamma
{

    CScript::CScript( EScriptVM eScriptVM, uint16 nDebugPort )
        : m_hScript( NULL )
		, m_eScriptType(eScriptVM)
	{		/*
		if (m_eScriptType == eSVM_AS3)
			m_hScript = new CScriptAS3( *this, m_eScriptType, nDebugPort );
		else if (m_eScriptType == eSVM_JS)
			m_hScript = new CScriptJS( *this, nDebugPort );
		else if (m_eScriptType == eSVM_Lua)
			m_hScript = new CScriptLua( *this, nDebugPort );*/

		assert( m_hScript != NULL );
    }

    CScript::~CScript()
    {
        delete m_hScript;
    }

	void* CScript::GetVM(EScriptVM eScriptVM)
	{/*
		if (eScriptVM == m_eScriptType)
		{
			if( eScriptVM == eSVM_AS3 )
			{
#ifndef DISABLE_TAMARIN
				return static_cast<CScriptAS3 *>(m_hScript)->GetTamarin();
#endif // DISABLE_TAMARIN
			}
			else if( eScriptVM == eSVM_Lua )
			{
				return static_cast<CScriptLua *>(m_hScript)->GetLuaState();
			}
		}*/
		return NULL;
	}

    //===============================================================
    // 功能：得到Script数据
    // 参数：无
    // 返回：脚本接口句柄
    //===============================================================
    HSCRIPT CScript::GetScript() const
    {
        return m_hScript;
    }

    //===============================================================
    // 功能：调用脚本文件
    // 参数：脚本文件
    // 返回：错误信息
    //===============================================================
    bool CScript::RunFile( const char* szFileName, bool bReload )
    {
        return m_hScript->RunFile( szFileName, bReload );
	}

	//===============================================================
	// 功能：调用脚本文件
	// 参数：脚本文件
	// 返回：错误信息
	//===============================================================
	bool CScript::RunBuffer( const void* pBuffer, size_t nSize )
	{
		return m_hScript->RunBuffer( pBuffer, nSize );
	}

    //===============================================================
    // 功能：执行一段代码
    // 参数：代码
    // 返回：错误信息
    //===============================================================
    bool CScript::RunString( const char* szString )
    {
        return m_hScript->RunString( szString );
	}

	//===============================================================
	// 功能：执行一个函数
	// 参数：代码
	// 返回：错误信息
	//===============================================================
	bool CScript::RunFunction( const STypeInfoArray& aryTypeInfo, void* pResultBuf, const char* szFunction, void** aryArg )
	{
		return m_hScript->RunFunction( aryTypeInfo, pResultBuf, szFunction, aryArg );
	}

    //设置搜索路径
    void CScript::AddSearchPath( const char* szPath )
    {
        return m_hScript->AddSearchPath( szPath );
    }

    //注册单个被脚本调用的函数
    void CScript::RegistFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName )
    {
        return m_hScript->RegistFunction( aryTypeInfo, funWrap, szTypeInfoName, szFunctionName );
	}

	/// 注册静态成员函数
	void CScript::RegistClassStaticFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName )
	{
		return m_hScript->RegistClassStaticFunction( aryTypeInfo, funWrap, szTypeInfoName, szFunctionName );
	}

    // 注册类成员函数
    void CScript::RegistClassFunction( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName )
    {
        return m_hScript->RegistClassFunction( aryTypeInfo, funWrap, szTypeInfoName, szFunctionName );
    }

    // 注册类回调函数
    ICallBackWrap&  CScript::RegistClassCallback( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funWrap, const char* szTypeInfoName, const char* szFunctionName )
    {
        return m_hScript->RegistClassCallback( aryTypeInfo, funWrap, szTypeInfoName, szFunctionName );
    }

    // 注册类成员数据
    void CScript::RegistClassMember( const STypeInfoArray& aryTypeInfo, IFunctionWrap* funGetSet[2], const char* szTypeInfoName, const char* szMemberName )
    {
        return m_hScript->RegistClassMember( aryTypeInfo, funGetSet, szTypeInfoName, szMemberName );
	}

	// 注册类
	void CScript::RegistClass( uint32 nSize, const char* szTypeIDName, const char* szClass, ... )
	{
		va_list listBase;
		va_start( listBase, szClass );
		m_hScript->RegistClass( nSize, szTypeIDName, szClass, listBase );
		va_end( listBase );
	}

	// 注册类
	void CScript::RegistConstruct( IObjectConstruct* pObjectConstruct, const char* szTypeIDName )
	{
		m_hScript->RegistConstruct( pObjectConstruct, szTypeIDName );
	}

	//注册析构函数
	ICallBackWrap& CScript::RegistDestructor( const char* szTypeInfoName, IFunctionWrap* funWrap )
	{
		return m_hScript->RegistDestructor( szTypeInfoName, funWrap );
	}

    //注册枚举类型            
    void CScript::RegistEnum( const char* szTypeIDName, const char* szTableName, int32 nTypeSize )
    {
        return m_hScript->RegistEnum( szTypeIDName, szTableName, nTypeSize );
    }

    //注册常量            
    void CScript::RegistConstant( const char* szTableName, const char* szFeild, int32 nValue )
    {
        return m_hScript->RegistConstant( szTableName, szFeild, nValue );
	}

	void CScript::RegistConstant( const char* szTableName, const char* szFeild, double dValue )
	{
		return m_hScript->RegistConstant( szTableName, szFeild, dValue );
	}

	void CScript::RegistConstant( const char* szTableName, const char* szFeild, const char* szValue )
	{
		return m_hScript->RegistConstant( szTableName, szFeild, szValue );
	}

    //增加引用计数
    void CScript::RefScriptObj( void* pObj )
    {
        return m_hScript->RefScriptObj( pObj );
    }

    //减少引用计数
    void CScript::UnrefScriptObj( void* pObj )
    {
        return m_hScript->UnrefScriptObj( pObj );
    }

	void CScript::GCAll()
	{
		return m_hScript->GCAll();
	}

	void CScript::GC()
	{
		return m_hScript->GC();
	}

	void UnlinkCppObjFromScript( void* pObj )
	{		
		CScriptBase::UnlinkCppObj( pObj );
	}

	int32 CallBack( int32 nIndex, void* pObject, void* pRetBuf, void** pArgArray )
	{
		return CScriptBase::CallBack( nIndex, pObject, pRetBuf, pArgArray );
	}

}
