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

	///  CTypeBase定义数据类型的设置和获取方式，该类不能单独实现，为实现类的根类
    class CTypeBase
    {    
    public:
		EDataType	GetType()	{ return m_nType; }
		uint32		GetLen()	{ return m_nSize; };

        CTypeBase( EDataType eType, uint32 nSize ) 
			: m_nType( eType ), m_nSize( nSize ){}
        virtual ~CTypeBase(){}

	protected:
		EDataType		m_nType;
		uint32			m_nSize;
	};
}

#endif
