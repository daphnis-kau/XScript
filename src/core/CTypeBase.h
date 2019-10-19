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
#include <map>

namespace Gamma
{
	class CClassRegistInfo;

    /// 函数参数类型
    enum EFunParamType
    { 
		// 传值
        eFPT_Value       = 0x0001,

		// 返回值
		eFPT_Return		= 0x0002,
    };

	///  CTypeBase定义数据类型的设置和获取方式，该类不能单独实现，为实现类的根类
    class CTypeBase
    {    
    public:
		EDataType	GetType()	{ return m_nType; }
		bool		IsNumber()	{ return m_nType >= eDT_char && m_nType <= eDT_double && m_nType != eDT_bool; }
		bool		IsValue()	{ return ( m_nFlag&eFPT_Value ) == eFPT_Value; }
		bool		IsReturn()	{ return ( m_nFlag&eFPT_Return ) == eFPT_Return; }
		bool		IsObj()		{ return GetClassRegistInfo() != NULL; }
		uint32		GetFlag()	{ return m_nFlag; };
		uint32		GetLen()	{ return m_nSize; };
		void		SetFlag( uint32 nFlag )	{ m_nFlag = nFlag; };

        CTypeBase( EDataType eType, uint32 nSize ) 
			: m_nType( eType ), m_nFlag(0), m_nSize( nSize ){}
        virtual ~CTypeBase(){}
		virtual CClassRegistInfo* GetClassRegistInfo(){ return NULL; }

	protected:
		EDataType		m_nType;
        uint32			m_nFlag;
		uint32			m_nSize;
	};
}

#endif
