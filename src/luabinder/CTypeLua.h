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
#include "CTypeBase.h"

struct lua_State;
namespace Gamma
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
		return (const wchar_t*)lua_tostring( pL, AbsStackIdx( pL, nStkId ) );
	}

	inline void PushWStrToLua( lua_State* pL, const wchar_t* szStr )
	{
		if( szStr == NULL )
			lua_pushnil( pL );
		else
			lua_pushlstring( pL, (const char*)szStr, ( wcslen( szStr ) + 1 )*sizeof(wchar_t) );
	}

    //=====================================================================
    /// lua对C++数据的操作方法，调用Lua库实现对数据在lua中的操作
    //=====================================================================
    class CLuaTypeBase : public CTypeBase
	{
	public:
		CLuaTypeBase( EDataType eType, uint32 nSize ) : CTypeBase( eType, nSize ){}
        virtual void    GetFromVM( lua_State* pL, char* pDataBuf, int32 nStkId, bool bExtend32Bit ) = 0;        //不 pop 出堆栈
        virtual void    PushToVM( lua_State* pL, char* pDataBuf )= 0;
    };

    //=====================================================================
    /// C++内置类型在lua中的操作
    //=====================================================================
    template<typename T>
    class TLuaValue : public CLuaTypeBase
    {
    public:
		TLuaValue( uint32 nSize = sizeof(T) )    
			: CLuaTypeBase( eDT_class, nSize )
		{
			m_nFlag = eFPT_Value;
		}

        void GetFromVM( lua_State* pL, char* pDataBuf, int32 nStkId, bool bExtend32Bit )
		{ 
			if( bExtend32Bit && sizeof(T) < sizeof(uint32) )
			{
				*(int32*)( pDataBuf ) = (T)(int64)GetNumFromLua( pL, nStkId );
			}
			else
			{
				double fValue = GetNumFromLua( pL, nStkId );
				*(T*)( pDataBuf ) = fValue < 0 ? (T)(int64)fValue : (T)(uint64)fValue;
			}
		};

        void PushToVM( lua_State* pL, char* pDataBuf )              
		{ 
			PushNumToLua( pL, (double)*(T*)( pDataBuf ) ); 
		}
    };

    template<> inline TLuaValue<int64>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_int64, nSize ) { m_nFlag = eFPT_Value; }

	template<> inline TLuaValue<long>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_long, nSize )	{ m_nFlag = eFPT_Value; }

	template<> inline TLuaValue<int32>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_int32, nSize )	{ m_nFlag = eFPT_Value; }

	template<> inline TLuaValue<int16>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_int16, nSize )	{ m_nFlag = eFPT_Value; }

	template<> inline TLuaValue<int8>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_int8, nSize )	{ m_nFlag = eFPT_Value; }

    template<> inline TLuaValue<uint64>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_uint64, nSize ) { m_nFlag = eFPT_Value; }

	template<> inline TLuaValue<ulong>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_ulong, nSize )	{ m_nFlag = eFPT_Value; }

	template<> inline TLuaValue<uint32>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_uint32, nSize )	{ m_nFlag = eFPT_Value; }

	template<> inline TLuaValue<uint16>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_uint16, nSize )	{ m_nFlag = eFPT_Value; }

	template<> inline TLuaValue<uint8>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_uint8, nSize )	{ m_nFlag = eFPT_Value; }

    template<> inline TLuaValue<float>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_float, nSize ) { m_nFlag = eFPT_Value; }

	template<> inline TLuaValue<double>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_double, nSize )	{ m_nFlag = eFPT_Value; }

	template<> inline TLuaValue<bool>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_bool, nSize )	{ m_nFlag = eFPT_Value; }

	template<> inline TLuaValue<void*>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_void, nSize )	{ m_nFlag = 0; }

	template<> inline TLuaValue<const char*>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_const_char_str, nSize )	{ m_nFlag = eFPT_Value; }

	template<> inline TLuaValue<const wchar_t*>::TLuaValue( uint32 nSize ) 
	: CLuaTypeBase( eDT_const_wchar_t_str, nSize )	{ m_nFlag = eFPT_Value; }

    //特化部分函数
    template<> inline void TLuaValue<uint32>::PushToVM( lua_State* pL, char* pDataBuf )
    { PushNumToLua( pL, (double)( *(uint32*)pDataBuf ) ); }

    template<> inline void TLuaValue<float>::GetFromVM( lua_State* pL, char* pDataBuf, int32 nStkId, bool bExtend32Bit )
	{ *(float*)( pDataBuf ) = GetNumFromLua( pL, nStkId ); }

    template<> inline void TLuaValue<float>::PushToVM( lua_State* pL, char* pDataBuf )
	{ PushNumToLua( pL, *(float*)( pDataBuf ) ); }

    template<> inline void TLuaValue<double>::GetFromVM( lua_State* pL, char* pDataBuf, int32 nStkId, bool bExtend32Bit )
	{ *(double*)( pDataBuf ) = GetNumFromLua( pL, nStkId ); }

    template<> inline void TLuaValue<double>::PushToVM( lua_State* pL, char* pDataBuf )
	{ PushNumToLua( pL, *(double*)( pDataBuf ) ); }

    template<> inline void TLuaValue<bool>::GetFromVM( lua_State* pL, char* pDataBuf, int32 nStkId, bool bExtend32Bit )
	{ int32 b = GetBoolFromLua( pL, nStkId ); memcpy( pDataBuf, &b, bExtend32Bit ? sizeof(uint32) : sizeof(bool) ); }

    template<> inline void TLuaValue<bool>::PushToVM( lua_State* pL, char* pDataBuf )
	{ PushBoolToLua( pL, *(bool*)( pDataBuf ) ); }

	template<> inline void TLuaValue<void*>::GetFromVM( lua_State* pL, char* pDataBuf, int32 nStkId, bool bExtend32Bit )
	{ *(void**)( pDataBuf ) = GetLightDataFromLua( pL, nStkId ); }

	template<> inline void TLuaValue<void*>::PushToVM( lua_State* pL, char* pDataBuf )
	{ PushLightDataToLua( pL, *(void**)( pDataBuf ) ); }

    template<> inline void TLuaValue<const char*>::GetFromVM( lua_State* pL, char* pDataBuf, int32 nStkId, bool bExtend32Bit )
    { *(const char**)( pDataBuf ) = GetStrFromLua( pL, nStkId ); }

    template<> inline void TLuaValue<const char*>::PushToVM( lua_State* pL, char* pDataBuf )
    { PushStrToLua( pL, *(const char**)( pDataBuf ) ); }

    template<> inline void TLuaValue<const wchar_t*>::GetFromVM( lua_State* pL, char* pDataBuf, int32 nStkId, bool bExtend32Bit )
    { *(const wchar_t**)( pDataBuf ) = GetWStrFromLua( pL, nStkId ); }

    template<> inline void TLuaValue<const wchar_t*>::PushToVM( lua_State* pL, char* pDataBuf )
    { PushWStrToLua( pL, *(const wchar_t**)( pDataBuf ) ); }


	typedef TLuaValue<int64>			CLuaInt64;
	typedef TLuaValue<long>				CLuaLong;
	typedef TLuaValue<int32>			CLuaInt32;
    typedef TLuaValue<int16>			CLuaInt16;
	typedef TLuaValue<int8>				CLuaInt8;
	typedef TLuaValue<uint64>			CLuaUint64;
	typedef TLuaValue<ulong>			CLuaUlong;
	typedef TLuaValue<uint32>			CLuaUint32;
    typedef TLuaValue<uint16>			CLuaUint16;
    typedef TLuaValue<uint8>			CLuaUint8;
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
    protected:
		CClassRegistInfo*    m_pClassInfo;
    public:
        CLuaObject( CClassRegistInfo* pClassInfo, uint32 nSize = sizeof(void*) );

        void				GetFromVM( lua_State* pL, char* pDataBuf, int32 nStkId, bool bExtend32Bit );
        void				PushToVM( lua_State* pL, char* pDataBuf );
		CClassRegistInfo*	GetClassRegistInfo(){ return m_pClassInfo; }
    };

    //=====================================================================
    /// lua数据类型之C++对象类型，必须以CClassRegistInfo* 构造
    //=====================================================================
    class CLuaValueObject : public CLuaObject
    {
    public:
		CLuaValueObject( CClassRegistInfo* pClassInfo );
		void				GetFromVM( lua_State* pL, char* pDataBuf, int32 nStkId, bool bExtend32Bit );
		void				PushToVM( lua_State* pL, char* pDataBuf );
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
		~CLuaBuffer(){}

		void                PushToVM( lua_State* pL, char* pDataBuf );
		void                GetFromVM( lua_State* pL, char* pDataBuf, int32 nStkId, bool bExtend32Bit );

		static void         RegistClass( CScriptLua* pScript );
	};

}

#endif
