/**@file  		CTypeLua.h
* @brief		Data interface between LUA&c++
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
*/

#ifndef __TYPE_LUA_H__
#define __TYPE_LUA_H__
#include <stdlib.h>
#include "common/Help.h"
#include "core/CTypeBase.h"
#include "CScriptLua.h"

struct lua_State;
namespace XS
{
	class CLuaTypeBase;
	//=====================================================================
	/// aux function
	//=====================================================================
	double			GetNumFromLua( lua_State* pL, int32 nStkId );
	void*			GetPointerFromLua( lua_State* pL, int32 nStkId, 
						int32 nCppObjStr, int32 nWeakTable );
	bool			PushPointerToLua( lua_State* pL, void* pBuffer, 
						int32 nCppObjStr, int32 nWeakTable, bool bCreateStreamBuff = true );
	void			RegisterPointerClass( CScriptLua* pScript );
	CLuaTypeBase*	GetLuaTypeBase( DataType eType );

    //=====================================================================
    /// Base class of data type
    //=====================================================================
    class CLuaTypeBase : public CTypeBase
	{
	public:
		CLuaTypeBase(){}
        virtual void GetFromVM( DataType eType, lua_State* pL, char* pDataBuf, 
			int32 nStkId, int32 nCppObjStr, int32 nWeakTable ) = 0;        
        virtual void PushToVM( DataType eType, lua_State* pL, char* pDataBuf, 
			int32 nCppObjStr, int32 nWeakTable )= 0;
    };

    //=====================================================================
    /// Common class of data type
    //=====================================================================
    template<typename T>
    class TLuaValue : public CLuaTypeBase
    {
    public:
        void GetFromVM( DataType eType, lua_State* pL, char* pDataBuf, 
			int32 nStkId, int32 /*nCppObjStr*/, int32 /*nWeakTable*/ )
		{ 
			double fValue = GetNumFromLua( pL, nStkId );
			*(T*)( pDataBuf ) = fValue < 0 ? (T)(int64)fValue : (T)(uint64)fValue;
		};

        void PushToVM( DataType eType, lua_State* pL, char* pDataBuf,
			int32 /*nCppObjStr*/, int32 /*nWeakTable*/ )
		{ 
			lua_pushnumber( pL, (double)*(T*)( pDataBuf ) );
		}

		static TLuaValue<T>& GetInst() 
		{ 
			static TLuaValue<T> s_Instance; 
			return s_Instance; 
		}
    };

    // POD type class specialization
    template<> inline void TLuaValue<float>::GetFromVM( DataType eType, 
		lua_State* pL, char* pDataBuf, int32 nStkId, int32, int32 )
	{ *(float*)( pDataBuf ) = (float)GetNumFromLua( pL, nStkId ); }

    template<> inline void TLuaValue<float>::PushToVM( DataType eType, 
		lua_State* pL, char* pDataBuf, int32, int32 )
	{ lua_pushnumber( pL, *(float*)( pDataBuf ) ); }

	template<> inline void TLuaValue<double>::GetFromVM( DataType eType,
		lua_State* pL, char* pDataBuf, int32 nStkId, int32, int32 )
	{ *(double*)( pDataBuf ) = GetNumFromLua( pL, nStkId ); }

	template<> inline void TLuaValue<double>::PushToVM( DataType eType,
		lua_State* pL, char* pDataBuf, int32, int32 )
	{ lua_pushnumber( pL, *(double*)( pDataBuf ) ); }

	template<> inline void TLuaValue<bool>::GetFromVM( DataType eType,
		lua_State* pL, char* pDataBuf, int32 nStkId, int32, int32 )
	{ *(bool*)( pDataBuf ) = lua_toboolean( pL, nStkId ); }

	template<> inline void TLuaValue<bool>::PushToVM( DataType eType,
		lua_State* pL, char* pDataBuf, int32, int32 )
	{ lua_pushboolean( pL, *(bool*)( pDataBuf ) ); }

	template<> inline void TLuaValue<void*>::GetFromVM( DataType eType,
		lua_State* pL, char* pDataBuf, int32 nStkId, 
		int32 nCppObjStr, int32 nWeakTable )
	{ *(void**)( pDataBuf ) = GetPointerFromLua( pL, nStkId, nCppObjStr, nWeakTable ); }

	template<> inline void TLuaValue<void*>::PushToVM( DataType eType,
		lua_State* pL, char* pDataBuf, int32 nCppObjStr, int32 nWeakTable )
	{ PushPointerToLua( pL, *(void**)( pDataBuf ), nCppObjStr, nWeakTable ); }

	template<> inline void TLuaValue<const char*>::GetFromVM( DataType eType,
		lua_State* pL, char* pDataBuf, int32 nStkId, int32, int32 )
    { *(const char**)( pDataBuf ) = lua_tostring( pL, nStkId ); }

	template<> inline void TLuaValue<const char*>::PushToVM( DataType eType,
		lua_State* pL, char* pDataBuf, int32, int32 )
    { lua_pushstring( pL, *(const char**)( pDataBuf ) ); }

	template<> inline void TLuaValue<const wchar_t*>::GetFromVM( DataType eType,
		lua_State* pL, char* pDataBuf, int32 nStkId, int32, int32 )
    { *(const wchar_t**)( pDataBuf ) = CScriptLua::ConvertUtf8ToUcs2( pL, nStkId ); }

	template<> inline void TLuaValue<const wchar_t*>::PushToVM( DataType eType,
		lua_State* pL, char* pDataBuf, int32, int32 )
    { CScriptLua::NewUnicodeString( pL, *(const wchar_t**)( pDataBuf ) ); }

    //=====================================================================
    /// Interface of class pointer
    //=====================================================================
    class CLuaObject : public TLuaValue<void*>
    {
    public:
		CLuaObject();

		static CLuaObject&	GetInst();
        void GetFromVM( DataType eType, lua_State* pL, char* pDataBuf,
			int32 nStkId, int32 nCppObjStr, int32 nWeakTable );
        void PushToVM( DataType eType,lua_State* pL, char* pDataBuf,
			int32 nCppObjStr, int32 nWeakTable );
    };

    //=====================================================================
    /// Interface of class value
    //=====================================================================
    class CLuaValueObject : public CLuaObject
    {
    public:
		CLuaValueObject();

		static CLuaValueObject&	GetInst();
		void GetFromVM( DataType eType, lua_State* pL, char* pDataBuf, 
			int32 nStkId, int32 nCppObjStr, int32 nWeakTable );
		void PushToVM( DataType eType, lua_State* pL, char* pDataBuf, 
			int32 nCppObjStr, int32 nWeakTable );
    };
}

#endif
