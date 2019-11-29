#ifndef __GAMMA_SCRIPTHELP_H__
#define __GAMMA_SCRIPTHELP_H__
#pragma warning(disable: 4624)
#pragma warning(disable: 4510)
#pragma warning(disable: 4610)

#include "common/Help.h"
#include "common/TList.h"
#include <array>

//=====================================================================
// GammaScriptHelp.h 
// 定义脚本注册的辅助模板类和辅助宏
// 柯达昭
// 2007-10-21
//=====================================================================

//==========================================================================
// 注册类的辅助类和宏
//==========================================================================
#define NORMAL_OBJECT_CONSTRUCT( _class, funGetVirtualTable, pVTable ) \
class _class##Construct : public IObjectConstruct \
{\
	virtual void Assign( void* pDest, void* pSrc )	\
	{ *(_class*)pDest = *(_class*)pSrc; }\
	virtual void Construct( void* pObj ) \
	{ \
		_class* pNew = new( pObj )_class;\
		if( !funGetVirtualTable ) return; \
		( (SVirtualObj*)pNew )->m_pTable = pVTable; \
	}\
	virtual void Destruct( void* pObj )		\
	{ static_cast<_class*>( pObj )->~_class(); }\
};

#define UNDUPLICATION_OBJECT_CONSTRUCT( _class, funGetVirtualTable, pVTable ) \
class _class##Construct : public IObjectConstruct \
{\
	virtual void Assign( void* pDest, void* pSrc )	\
	{ throw( "Can not call construct on unduplication object" ); }\
	virtual void Construct( void* pObj ) \
	{ \
		_class* pNew = new( pObj )_class;\
		if( !funGetVirtualTable ) return; \
		( (SVirtualObj*)pNew )->m_pTable = pVTable; \
	}\
	virtual void Destruct( void* pObj )		\
	{ static_cast<_class*>( pObj )->~_class(); }\
};

#define DEFINE_GETVTABLE_IMP( funGetVirtualTable, pVirtual )	\
struct SGetVTable { SGetVTable() \
{ \
	if( !funGetVirtualTable || pVirtual ) return; \
	pVirtual = funGetVirtualTable( this );\
} };

namespace Gamma
{
	class CScriptRegisterNode : public Gamma::TList<CScriptRegisterNode>::CListNode
	{
		void( *m_funRegister )( );
		typedef typename Gamma::TList<CScriptRegisterNode>::CListNode ParentType;
	public:
		CScriptRegisterNode( Gamma::TList<CScriptRegisterNode>& list, void( *fun )( ) )
			: m_funRegister( fun )
		{
			list.PushBack( *this );
		}

		bool Register()
		{
			m_funRegister();
			auto n = GetNext();
			Remove();
			return n ? n->Register() : true;
		}
	};

	typedef Gamma::TList<CScriptRegisterNode> CScriptRegisterList;
	struct SGlobalExe { SGlobalExe( bool ) {} };

	template<typename _Derive, typename ... _Base>
	class TInheritInfo
	{
	public:
		enum { size = sizeof...( _Base ) + 1 };
		template<typename...Param> struct TOffset {};
		template<> struct TOffset<> { static void Get( ptrdiff_t* ary ) {} };

		template<typename First, typename...Param>
		struct TOffset<First, Param...> {
			static void Get( ptrdiff_t* ary )
			{
				*ary = ( (ptrdiff_t)(First*)(_Derive*)0x40000000 ) - 0x40000000;
				TOffset<Param...>::Get( ++ary );
			}
		};

		static std::array<ptrdiff_t, size + 1> Values()
		{
			std::array<ptrdiff_t, size + 1> result = { sizeof( _Derive ) };
			TOffset<_Base...>::Get( &result[1] );
			return result;
		}

		static std::array<const char*, size + 1> Types()
		{
			return { typeid( _Derive ).name(), typeid( _Base... ).name()... };
		}
	};
}

#endif
