#ifndef __SCRIPT_LUA_H__
#define __SCRIPT_LUA_H__
//=====================================================================
// CScriptLua.h 
// 为lua定义的C++到虚拟机的接口
// 柯达昭
// 2007-10-16
//=====================================================================

#include "core/CScriptBase.h"

struct lua_State;
struct lua_Debug;

namespace Gamma
{
    //========================================定义的全局table==========================================

    class CDebugLua;
	class CScriptLua : public CScriptBase
	{
		enum { eMemoryStep = 8, eMaxManageMemoryCount = 8 };
		struct SMemoryBlock	{ SMemoryBlock* m_pNext; };

		std::vector<lua_State*>	m_vecLuaState;
		std::wstring			m_szTempUcs2;
		std::string				m_szTempUtf8;

		SMemoryBlock*			m_pAllAllocBlock;
		SMemoryBlock*			m_aryBlock[eMaxManageMemoryCount];
		bool					m_bPreventExeInRunBuffer;

        //==============================================================================
        // 对Lua提供的功能性函数
        //==============================================================================
		static int32			ClassCast( lua_State* pL );
		static int32			ErrorHandler( lua_State* pState );
		static int32			DebugBreak( lua_State* pState );
		static int32			BackTrace( lua_State* pState );
		static int32			NewUcs2String( lua_State* pL );
		static int32			NewUtf8String( lua_State* pL );
        static int32			ToUint32( lua_State* pL );
        static int32			ToInt32( lua_State* pL );
        static int32			ToUint16( lua_State* pL );
        static int32			ToInt16( lua_State* pL );
        static int32			ToUint8( lua_State* pL );
        static int32			ToInt8( lua_State* pL );
        static int32			ToChar( lua_State* pL );
        static int32			BitAnd( lua_State* pL );
		static int32			BitOr( lua_State* pL );
		static int32			BitNot( lua_State* pL );
		static int32			BitXor( lua_State* pL );
		static int32			LeftShift( lua_State* pL );
		static int32			RightShift( lua_State* pL );
        static int32			Delete( lua_State* pL );
        static int32			Construct( lua_State* pL );
		static int32			LoadFile( lua_State* pL );
		static int32			DoFile( lua_State* pL );
		static int32			Panic( lua_State* pL );	
		static void*			Realloc( void* pContex, void* pPreBuff, size_t nOldSize, size_t nNewSize );	
		static int32			Print( lua_State* pL );
		static int32			ToString( lua_State* pL );

		static void				DebugHookProc( lua_State *pState, lua_Debug* pDebug );
		static bool				GetGlobObject( lua_State* pL, const char* szKey );
		static bool				SetGlobObject( lua_State* pL, const char* szKey );

		void					BuildRegisterInfo();
        void					AddLoader();
		void					IO_Replace();

        static void             RegistToLua( lua_State* pL, const CClassRegistInfo* pInfo, void* pObj, int32 nObjTable, int32 nObj );
        static void             RemoveFromLua( lua_State* pL, const CClassRegistInfo* pInfo, void* pObj, int32 nObjTable, int32 nObj );

		virtual bool			CallVM( const CCallScriptBase* pCallBase, void* pRetBuf, void** pArgArray );
		virtual void			DestrucVM( const CCallScriptBase* pCallBase, SVirtualObj* pObject );

		friend class CDebugLua;
		friend class CLuaBuffer;

    public:
        CScriptLua( uint16 nDebugPort = 0 );
		~CScriptLua(void);

        // 预定义字符串
		static void*			ms_pGlobObjectTableKey;
		static void*			ms_pRegistScriptLuaKey;
		static void*			ms_pErrorHandlerKey;
		static void*			ms_pClassInfoKey;

        //==============================================================================
        // 通用函数
        //==============================================================================
        static void				NewLuaObj( lua_State* pL, const CClassRegistInfo* pInfo, void* pSrc );
		static void				RegisterObject( lua_State* pL, const CClassRegistInfo* pInfo, void* pObj, bool bGC );
							    
		lua_State*              GetLuaState();
		void					PushLuaState( lua_State* pL );
		void					PopLuaState();
		void					SetDebugLine();

        static  CScriptLua*     GetScript( lua_State* pL );
		virtual bool        	RunBuffer( const void* pBuffer, size_t nSize, const char* szFileName );
		virtual bool        	RunFunction( const STypeInfoArray& aryTypeInfo, void* pResultBuf, const char* szFunction, void** aryArg );
		virtual void            UnlinkCppObjFromScript( void* pObj );
		virtual void        	GC();
		virtual void        	GCAll();
	};
}

#endif
