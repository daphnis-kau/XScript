/**@file  		CScriptLua.h
* @brief		LUA VM base wrapper
* @author		Daphnis Kaw
* @date			2020-01-17
* @version		V1.0
*/

#ifndef __SCRIPT_LUA_H__
#define __SCRIPT_LUA_H__
#include "core/CScriptBase.h"

struct lua_State;
struct lua_Debug;

namespace XS
{
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
        // aux function
        //==============================================================================
		static int32			ClassCast( lua_State* pL );
		static int32			CallByLua( lua_State* pL );
		static int32			ErrorHandler( lua_State* pState );
		static int32			DebugBreak( lua_State* pState );
		static int32			BackTrace( lua_State* pState );
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

        static void             RegistToLua( lua_State* pL, const CClassInfo* pInfo, void* pObj, int32 nObjTable, int32 nObj );
        static void             RemoveFromLua( lua_State* pL, const CClassInfo* pInfo, void* pObj, int32 nObjTable, int32 nObj );

		virtual bool			CallVM( const CCallbackInfo* pCallBase, void* pRetBuf, void** pArgArray );
		virtual void			DestrucVM( const CCallbackInfo* pCallBase, SVirtualObj* pObject );

		friend class CDebugLua;
		friend class CLuaBuffer;

    public:
        CScriptLua( uint16 nDebugPort = 0 );
		~CScriptLua(void);

		//==============================================================================
		// built keys
		//==============================================================================
		static void*			ms_pGlobObjectTableKey;
		static void*			ms_pRegistScriptLuaKey;
		static void*			ms_pErrorHandlerKey;
		static void*			ms_pClassInfoKey;

        //==============================================================================
        // common function
        //==============================================================================
        static void				NewLuaObj( lua_State* pL, const CClassInfo* pInfo, void* pSrc );
		static void				RegisterObject( lua_State* pL, const CClassInfo* pInfo, void* pObj, bool bGC );
		static void				NewUnicodeString( lua_State* pL, const wchar_t* szStr );
		static const wchar_t*	ConvertUtf8ToUcs2( lua_State* pL, int32 nStkId );

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
