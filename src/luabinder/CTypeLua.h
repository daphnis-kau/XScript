#ifndef __TYPE_LUA_H__
#define __TYPE_LUA_H__
//=====================================================================
// CTypeLua.h
// 定义Lua的数据类型接口
// 柯达昭
// 2007-10-16
//=====================================================================

#include <stdlib.h>
#include "common/Help.h"
#include "core/CTypeBase.h"
#include "CScriptLua.h"

struct lua_State;
namespace XS
{
	//=====================================================================
	/// lua对C++数据的操作方法封装
	//=====================================================================
	inline int32 AbsStackIdx( lua_State* pL, int32 nStkId )
	{
		int r = nStkId > 0 ? nStkId : ( lua_gettop( pL ) + nStkId + 1 );
		assert( r >=0 );
		return r;
	}

	inline double GetNumFromLua( lua_State* pL, int32 nStkId )
	{
		nStkId = AbsStackIdx( pL, nStkId );
		if( lua_isboolean( pL, nStkId ) )
			return (double)(int32)lua_toboolean( pL, nStkId );
		if( LUA_TSTRING == lua_type( pL, nStkId ) )
		{
			char *endptr;
			const char* s = lua_tostring( pL, nStkId );
			double result = strtod( s, &endptr );
			if( endptr == s )
				return 0;  /* conversion failed */
			
			/* maybe an hexadecimal constant? */
			if( *endptr == 'x' || *endptr == 'X' )  
			{
				uint64 nValue = 0;
				int n = ValueFromHexNumber( *++endptr );
				while( n >= 0 )
				{
					nValue = ( nValue << 4 )|n;
					n = ValueFromHexNumber( *++endptr );
				}
				result = (double)nValue;
			}

			return result;
		}
		return lua_tonumber( pL, nStkId );
	}

	inline void PushNumToLua( lua_State* pL, double fValue )
	{ 
		lua_pushnumber( pL, fValue );
	}

	inline bool GetBoolFromLua( lua_State* pL, int32 nStkId )
	{
		return lua_toboolean( pL, AbsStackIdx( pL, nStkId ) ) != 0;
	}

	inline void PushBoolToLua( lua_State* pL, bool bValue )
	{
		lua_pushboolean( pL, bValue );
	}

	inline void* GetLightDataFromLua( lua_State* pL, int32 nStkId )
	{
		return lua_touserdata( pL, AbsStackIdx( pL, nStkId ) );
	}

	inline void PushLightDataToLua( lua_State* pL, void* pValue )
	{
		lua_pushlightuserdata( pL, pValue );
	}

	inline const char* GetStrFromLua( lua_State* pL, int32 nStkId )
	{
		return lua_tostring( pL, AbsStackIdx( pL, nStkId ) );
	}

	inline void PushStrToLua( lua_State* pL, const char* szStr )
	{
		lua_pushstring( pL, szStr );
	}

	inline const wchar_t* GetWStrFromLua( lua_State* pL, int32 nStkId )
	{
		return CScriptLua::ConvertUtf8ToUcs2( pL, AbsStackIdx( pL, nStkId ) );
	}

	inline void PushWStrToLua( lua_State* pL, const wchar_t* szStr )
	{
		CScriptLua::NewUnicodeString( pL, szStr );
	}

    //=====================================================================
    /// lua对C++数据的操作方法，调用Lua库实现对数据在lua中的操作
    //=====================================================================
    class CLuaTypeBase
	{
	public:
		CLuaTypeBase(){}
		//不 pop 出堆栈
        virtual void GetFromVM( DataType eType, 
			lua_State* pL, char* pDataBuf, int32 nStkId ) = 0;        
        virtual void PushToVM( DataType eType, 
			lua_State* pL, char* pDataBuf )= 0;
    };

    //=====================================================================
    /// C++内置类型在lua中的操作
    //=====================================================================
    template<typename T>
    class TLuaValue : public CLuaTypeBase
    {
    public:
        void GetFromVM( DataType eType, 
			lua_State* pL, char* pDataBuf, int32 nStkId )
		{ 
			double fValue = GetNumFromLua( pL, nStkId );
			*(T*)( pDataBuf ) = fValue < 0 ? (T)(int64)fValue : (T)(uint64)fValue;
		};

        void PushToVM( DataType eType, 
			lua_State* pL, char* pDataBuf )
		{ 
			PushNumToLua( pL, (double)*(T*)( pDataBuf ) ); 
		}

		static TLuaValue<T>& GetInst() 
		{ 
			static TLuaValue<T> s_Instance; 
			return s_Instance; 
		}
    };

    //特化部分函数
    template<> inline void TLuaValue<float>::GetFromVM
	( DataType eType, lua_State* pL, char* pDataBuf, int32 nStkId )
	{ *(float*)( pDataBuf ) = GetNumFromLua( pL, nStkId ); }

    template<> inline void TLuaValue<float>::PushToVM
	( DataType eType, lua_State* pL, char* pDataBuf )
	{ PushNumToLua( pL, *(float*)( pDataBuf ) ); }

    template<> inline void TLuaValue<double>::GetFromVM
	( DataType eType, lua_State* pL, char* pDataBuf, int32 nStkId )
	{ *(double*)( pDataBuf ) = GetNumFromLua( pL, nStkId ); }

    template<> inline void TLuaValue<double>::PushToVM
	( DataType eType, lua_State* pL, char* pDataBuf )
	{ PushNumToLua( pL, *(double*)( pDataBuf ) ); }

    template<> inline void TLuaValue<bool>::GetFromVM
	( DataType eType, lua_State* pL, char* pDataBuf, int32 nStkId )
	{ *(bool*)( pDataBuf ) = GetBoolFromLua( pL, nStkId ); }

    template<> inline void TLuaValue<bool>::PushToVM
	( DataType eType, lua_State* pL, char* pDataBuf )
	{ PushBoolToLua( pL, *(bool*)( pDataBuf ) ); }

	template<> inline void TLuaValue<void*>::GetFromVM
	( DataType eType, lua_State* pL, char* pDataBuf, int32 nStkId )
	{ *(void**)( pDataBuf ) = GetLightDataFromLua( pL, nStkId ); }

	template<> inline void TLuaValue<void*>::PushToVM
	( DataType eType, lua_State* pL, char* pDataBuf )
	{ PushLightDataToLua( pL, *(void**)( pDataBuf ) ); }

    template<> inline void TLuaValue<const char*>::GetFromVM
	( DataType eType, lua_State* pL, char* pDataBuf, int32 nStkId )
    { *(const char**)( pDataBuf ) = GetStrFromLua( pL, nStkId ); }

    template<> inline void TLuaValue<const char*>::PushToVM
	( DataType eType, lua_State* pL, char* pDataBuf )
    { PushStrToLua( pL, *(const char**)( pDataBuf ) ); }

    template<> inline void TLuaValue<const wchar_t*>::GetFromVM
	( DataType eType, lua_State* pL, char* pDataBuf, int32 nStkId )
    { *(const wchar_t**)( pDataBuf ) = GetWStrFromLua( pL, nStkId ); }

    template<> inline void TLuaValue<const wchar_t*>::PushToVM
	( DataType eType, lua_State* pL, char* pDataBuf )
    { PushWStrToLua( pL, *(const wchar_t**)( pDataBuf ) ); }


	typedef TLuaValue<int64>			CLuaInt64;
	typedef TLuaValue<long>				CLuaLong;
	typedef TLuaValue<int32>			CLuaInt32;
    typedef TLuaValue<int16>			CLuaInt16;
	typedef TLuaValue<int8>				CLuaInt8;
	typedef TLuaValue<char>				CLuaChar;
	typedef TLuaValue<uint64>			CLuaUint64;
	typedef TLuaValue<ulong>			CLuaUlong;
	typedef TLuaValue<uint32>			CLuaUint32;
    typedef TLuaValue<uint16>			CLuaUint16;
	typedef TLuaValue<uint8>			CLuaUint8;
	typedef TLuaValue<wchar_t>			CLuaWChar;
    typedef TLuaValue<float>			CLuaFloat;
    typedef TLuaValue<double>			CLuaDouble;
    typedef TLuaValue<bool>				CLuaBool;
	typedef TLuaValue<void*>			CLuaPointer;
    typedef TLuaValue<const char*>		CLuaString;
    typedef TLuaValue<const wchar_t*>	CLuaWString;

    //=====================================================================
    /// C++对象以lua形式的表示及其操作，必须以CClassRegistInfo* 构造
    //=====================================================================
    class CLuaObject : public CLuaPointer
    {
    public:
		CLuaObject();

		static CLuaObject&	GetInst();
        void GetFromVM( DataType eType, lua_State* pL, char* pDataBuf, int32 nStkId );
        void PushToVM( DataType eType,lua_State* pL, char* pDataBuf );
    };

    //=====================================================================
    /// lua数据类型之C++对象类型，必须以CClassRegistInfo* 构造
    //=====================================================================
    class CLuaValueObject : public CLuaObject
    {
    public:
		CLuaValueObject();

		static CLuaValueObject&	GetInst();
		void GetFromVM( DataType eType, lua_State* pL, char* pDataBuf, int32 nStkId );
		void PushToVM( DataType eType, lua_State* pL, char* pDataBuf );
    };

	//=====================================================================
	/// lua数据类型之C++指针类型
	//=====================================================================
	class CScriptLua;
	static const char* s_szLuaBufferClass	= "CBufferStream";
	static const char* s_szLuaBufferInfo	= "CBufferStream_hObject";

	struct SBufferInfo
	{
		tbyte*			pBuffer;
		uint32			nPosition;
		uint32			nDataSize;
		uint32			nCapacity;
	};

	class CLuaBuffer : public CLuaPointer
	{     
		static bool			IsLightData( SBufferInfo* pInfo );
		static SBufferInfo*	GetBufferInfo( lua_State* pL, int32 nStkID );
		static SBufferInfo* CheckBufferSpace( SBufferInfo* pInfo, 
								uint32 nSize, lua_State* pL, int32 nStkID );

		template<class Type>
		static Type			ReadData( lua_State* pL );

		template<class Type>
		static void			WriteData( lua_State* pL, Type v );

		static int32		GetBit( lua_State* pL );
		static int32		ReadBoolean( lua_State* pL );
		static int32		ReadInt8( lua_State* pL );
		static int32		ReadDouble( lua_State* pL );
		static int32		ReadFloat( lua_State* pL );
		static int32		ReadInt64( lua_State* pL );
		static int32		ReadInt32( lua_State* pL );
		static int32		ReadInt16( lua_State* pL );
		static int32		ReadUint8( lua_State* pL );
		static int32		ReadUint64( lua_State* pL );
		static int32		ReadUint32( lua_State* pL );
		static int32		ReadUint16( lua_State* pL );
		static int32		ReadUTF( lua_State* pL );
		static int32		ReadUTFBytes( lua_State* pL );
		static int32		ReadUCS( lua_State* pL );
		static int32		ReadUCSCounts( lua_State* pL );
		static int32		ReadBytes( lua_State* pL );

		static int32		SetBit( lua_State* pL  );
		static int32		WriteBoolean( lua_State* pL );
		static int32		WriteInt8( lua_State* pL );
		static int32		WriteDouble( lua_State* pL );
		static int32		WriteFloat( lua_State* pL );
		static int32		WriteInt32( lua_State* pL );
		static int32		WriteInt64( lua_State* pL );
		static int32		WriteUint64( lua_State* pL );
		static int32		WriteInt16( lua_State* pL );
		static int32		WriteUTF( lua_State* pL );
		static int32		WriteUTFBytes( lua_State* pL );
		static int32		WriteBytes( lua_State* pL );

		static int32		Uncompress( lua_State* pL );
		static int32		Compress( lua_State* pL );

		static int32		SetPosition( lua_State* pL );
		static int32		GetPosition( lua_State* pL );

		static int32		SetDataSize( lua_State* pL );
		static int32		GetDataSize( lua_State* pL );

		static int32		Reset( lua_State* pL );
		static int32		Clear( lua_State* pL );
	public:
		CLuaBuffer();
		~CLuaBuffer() {}

		static CLuaBuffer& GetInst();
		static void RegistClass(CScriptLua* pScript);
		void PushToVM( DataType eType, lua_State* pL, char* pDataBuf );
		void GetFromVM( DataType eType, lua_State* pL, char* pDataBuf, int32 nStkId );
	};

}

#endif
