/**@file  		CScriptLua.h
* @brief		LUA VM base wrapper
* @author		Daphnis Kau
* @date			2019-06-24
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
		struct SMemoryBlock	{ SMemoryBlock* m_pNext; };
		typedef std::vector<SMemoryBlock*> CMemoryBlockList;

		std::vector<lua_State*>	m_vecLuaState;
		std::wstring			m_szTempUcs2;
		std::string				m_szTempUtf8;


		std::vector<void*>		m_pAllAllocBlock;
		CMemoryBlockList		m_aryBlockByClass;
		bool					m_bPreventExeInRunBuffer;

        //==============================================================================
        // aux function
		//==============================================================================
		static void				PushCppObjAndWeakTable( lua_State* pL );
		static int32			GetIndexClosure( lua_State* pL );
		static int32			GetNewIndexClosure( lua_State* pL );
		static int32			GetInstanceField( lua_State* pL );
		static int32			SetInstanceField( lua_State* pL );
		static int32			ClassCast( lua_State* pL );
		static int32			CallByLua( lua_State* pL );
		static int32			ErrorHandler( lua_State* pState );
		static int32			DebugBreak( lua_State* pState );
		static int32			BackTrace( lua_State* pState );
        static int32			ObjectGC( lua_State* pL );
        static int32			ObjectConstruct( lua_State* pL );
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

		static void             RegistToLua( lua_State* pL, const CClassInfo* pInfo, void* pObj, 
									int32 nObj, int32 nGlobalWeakTable, int32 nCppObjTable );

		virtual bool			CallVM( const CCallbackInfo* pCallBase, void* pRetBuf, void** pArgArray );
		virtual void			DestrucVM( const CCallbackInfo* pCallBase, SVirtualObj* pObject );

		virtual bool			Set( void* pObject, int32 nIndex, void* pArgBuf, const STypeInfo& TypeInfo );
		virtual bool			Get( void* pObject, int32 nIndex, void* pResultBuf, const STypeInfo& TypeInfo );
		virtual bool			Set( void* pObject, const char* szName, void* pArgBuf, const STypeInfo& TypeInfo );
		virtual bool			Get( void* pObject, const char* szName, void* pResultBuf, const STypeInfo& TypeInfo );
		virtual bool        	Call( const STypeInfoArray& aryTypeInfo, void* pResultBuf, const char* szFunction, void** aryArg );
		virtual bool        	Call( const STypeInfoArray& aryTypeInfo, void* pResultBuf, void* pFunction, void** aryArg );
		virtual bool        	RunBuffer( const void* pBuffer, size_t nSize, const char* szFileName, bool bForce = false );

		virtual void*			GetVM();

		friend class CDebugLua;
		friend class CLuaBuffer;

    public:
        CScriptLua(const char* strDebugHost, uint16 nDebugPort = 0, bool bWaitForDebugger = false );
		~CScriptLua(void);

		//==============================================================================
		// built keys
		//==============================================================================
		static void*			ms_pGlobObjectWeakTable;
		static void*			ms_pGlobObjectTable;
		static void*			ms_pRegistScriptLua;
		static void*			ms_pFirstClassInfo;
		static void*			ms_pErrorHandler;

        //==============================================================================
        // common function
        //==============================================================================
        static void*			NewLuaObj( lua_State* pL, const CClassInfo* pInfo, int32 nCppObjs );
		static void				RegisterObject( lua_State* pL, const CClassInfo* pInfo, 
									void* pObj, bool bGC, int32 nCppObjStr, int32 nWeakTable );
		static void				NewUnicodeString( lua_State* pL, const wchar_t* szStr );
		static const wchar_t*	ConvertUtf8ToUcs2( lua_State* pL, int32 nStkId );

		lua_State*              GetLuaState();
		void					PushLuaState( lua_State* pL );
		void					PopLuaState();
		void					SetDebugLine();

        static  CScriptLua*     GetScript( lua_State* pL );
		virtual int32			IncRef( void* pObj );
		virtual int32			DecRef( void* pObj );
		virtual void            UnlinkCppObjFromScript( void* pObj );
		virtual void        	GC();
		virtual void        	GCAll();
		virtual bool			IsValid( void* pObject );
	};
}

#endif
