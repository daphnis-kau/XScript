#include <string>
#include <locale>
#include <algorithm>

extern "C"
{
	#include "lua.h"
	#include "lauxlib.h"
}

#include "common/Help.h"
#include "common/CodeCvs.h"
#include "common/TStrStream.h"
#include "core/CClassInfo.h"
#include "CTypeLua.h"
#include "CScriptLua.h"

namespace XS
{
	//=====================================================================
	// 帮助函数及结构
	//=====================================================================
	static const char* s_szLuaBufferClass = "CBufferStream";
	static const char* s_szLuaBufferInfo = "CBufferStream_hObject";

	struct SBufferInfo
	{
		tbyte*			pBuffer;
		uint32			nPosition;
		uint32			nDataSize;
		uint32			nCapacity;
	};

	//=====================================================================
	/// lua对C++数据的操作方法封装
	//=====================================================================
	inline int32 ToAbsStackIndex( lua_State* pL, int32 nStkId )
	{
		if( nStkId > 0 )
			return nStkId;
		return lua_gettop( pL ) + nStkId + 1;
	}

	//=====================================================================
    // lua数据类型之C++对象类型
	//=====================================================================
	static void ConstructLua( lua_State* pL )
	{
		lua_getfield( pL, -1, "construction" );
		if( !lua_isnil( pL, -1 ) )
		{
			lua_pushvalue( pL, -2 );
			lua_call( pL, 1, 0 );
		}
		else
		{
			lua_pop( pL, 1 );
		}
	}

    CLuaObject::CLuaObject()
    { 
    }

	CLuaObject& CLuaObject::GetInst()
	{
		static CLuaObject s_Instance;
		return s_Instance;
	}

	void CLuaObject::GetFromVM(DataType eType, lua_State* pL, char* pDataBuf,
		int32 nStkId, int32 nCppObjStr, int32 nWeakTable)
    {
        nStkId = ToAbsStackIndex( pL, nStkId );
		int32 nType = lua_type( pL, nStkId );
        if( nType == LUA_TNIL || nType == LUA_TNONE )
            *(void**)( pDataBuf ) = nullptr;    
        else
        {
			if( !lua_istable( pL, nStkId )  )
			{
				luaL_error( pL, "GetFromVM error id:%d", nStkId );
				return;
			}

			lua_pushvalue( pL, nCppObjStr );
			lua_rawget( pL, nStkId );
			if( !lua_istable( pL, -1 ) )
			{
				lua_pushlightuserdata( pL, (void*)s_szLuaBufferInfo );
				lua_rawget( pL, nStkId );
				if( lua_islightuserdata( pL, -1 ) || lua_type( pL, -1 ) == LUA_TUSERDATA )
				{
					SBufferInfo* pInfo = (SBufferInfo*)lua_touserdata( pL, -1 );
					*(void**)( pDataBuf ) = pInfo ? pInfo->pBuffer : nullptr;
					lua_pop( pL, 2 );
				}
				else
				{
					*(void**)( pDataBuf ) = nullptr;
					lua_pop( pL, 2 );
				}
				return;
			}
			
			auto pClassInfo = (const CClassInfo*)( ( eType >> 1 ) << 1 );
			lua_pushlightuserdata( pL, (void*)pClassInfo );
			lua_rawget( pL, -2 );
            *(void**)( pDataBuf ) = lua_touserdata( pL, -1 );
            lua_pop( pL, 2 );
        }
    }

    void CLuaObject::PushToVM( DataType eType, lua_State* pL, char* pDataBuf, 
		int32 nCppObjStr, int32 nWeakTable )
    {
        void* pObj = *(void**)( pDataBuf );
        if( pObj == nullptr )
        {
            lua_pushnil( pL );
            return;
        }

        lua_pushlightuserdata( pL, pObj );		
        lua_rawget( pL, nWeakTable );			//[Obj]

		auto pClassInfo = (const CClassInfo*)( ( eType >> 1 ) << 1 );
        if( !lua_isnil( pL, -1 ) )
		{
			lua_pushvalue( pL, nCppObjStr );
			lua_rawget( pL, -2 );				//[Obj,tblCppObjs]
			if( !lua_isnil( pL, -1 ) )
			{
				lua_pushlightuserdata( pL, (void*)pClassInfo );
				lua_rawget( pL, -2 );			//[Obj,tblCppObjs,cppObj]
				bool bNil = lua_isnil( pL, -1 );
				lua_pop( pL, 2 );
				if( !bNil )
					return;
			}
			lua_pop( pL, 1 );					//[Obj]
			CScriptLua::GetScript( pL )->UnlinkCppObjFromScript( pObj );
        }

        lua_pop( pL, 1 );						//[]

		// Table 在Lua栈顶
		lua_newtable( pL );
		lua_getglobal( pL, pClassInfo->GetClassName().c_str() );
		if( lua_isnil( pL, -1 ) )
		{
			//szClass必须被注册
			luaL_error( pL, "PushToVM Class Not Registed:%s", pClassInfo->GetClassName().c_str() );
			return;
		}

		lua_setmetatable( pL, -2 );

		// (cppobjs) 绑定ObjectIndex
		lua_pushvalue( pL, nCppObjStr );
		lua_newtable( pL );
		lua_pushlightuserdata( pL, (void*)pClassInfo );
		lua_pushlightuserdata( pL, *(void**)( pDataBuf ) );
		lua_rawset( pL, -3 );
		lua_rawset( pL, -3 );

		CScriptLua::RegisterObject( pL, pClassInfo,
			*(void**)( pDataBuf ), false, nCppObjStr, nWeakTable );
		ConstructLua( pL );
    }

    //=====================================================================
    // lua数据类型之C++对象类型
    //=====================================================================
    CLuaValueObject::CLuaValueObject()
	{
	}

	CLuaValueObject& CLuaValueObject::GetInst()
	{

		static CLuaValueObject s_Instance;
		return s_Instance;
	}

	void CLuaValueObject::GetFromVM(DataType eType, lua_State* pL, char* pDataBuf,
		int32 nStkId, int32 nCppObjStr, int32 nWeakTable)
	{
		CLuaObject::GetFromVM( eType, pL, pDataBuf, nStkId, nCppObjStr, nWeakTable );
	}

	void CLuaValueObject::PushToVM( DataType eType, lua_State* pL, char* pDataBuf, 
		int32 nCppObjStr, int32 nWeakTable )
	{
		// Table 在Lua栈顶
		lua_newtable( pL ); // Obj
		lua_pushvalue( pL, nCppObjStr );
		lua_newtable( pL );
		lua_rawset( pL, -3 );
		int32 nStkId = ToAbsStackIndex( pL, -1 );

		auto pClassInfo = (const CClassInfo*)( ( eType >> 1 ) << 1 );
		lua_getglobal( pL, pClassInfo->GetClassName().c_str() );
		lua_setmetatable( pL, nStkId );

		void* pNewObj = CScriptLua::NewLuaObj( pL, pClassInfo, nCppObjStr );
		CScriptLua* pScriptLua = CScriptLua::GetScript( pL );
		pScriptLua->PushLuaState( pL );
		pClassInfo->CopyConstruct( pScriptLua, pNewObj, pDataBuf );
		pScriptLua->PopLuaState();

		//stack top = 2, 对象指针在栈顶,保存对象的表在下面
		CScriptLua::RegisterObject( pL, pClassInfo, pNewObj, true, nCppObjStr, nWeakTable );
		ConstructLua( pL );
	}

	//=====================================================================
    // lua数据类型之C++指针类型
	//=====================================================================
	class CLuaBuffer
	{
	public:
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

		static int32		SetBit( lua_State* pL );
		static int32		WriteBoolean( lua_State* pL );
		static int32		WriteInt8( lua_State* pL );
		static int32		WriteUint8( lua_State* pL );
		static int32		WriteDouble( lua_State* pL );
		static int32		WriteFloat( lua_State* pL );
		static int32		WriteInt32( lua_State* pL );
		static int32		WriteUint32( lua_State* pL );
		static int32		WriteInt64( lua_State* pL );
		static int32		WriteUint64( lua_State* pL );
		static int32		WriteInt16( lua_State* pL );
		static int32		WriteUint16( lua_State* pL );
		static int32		WriteUTF( lua_State* pL );
		static int32		WriteUTFBytes( lua_State* pL );
		static int32		WriteBytes( lua_State* pL );

		static int32		SetPosition( lua_State* pL );
		static int32		GetPosition( lua_State* pL );

		static int32		SetDataSize( lua_State* pL );
		static int32		GetDataSize( lua_State* pL );

		static int32		Reset( lua_State* pL );
		static int32		Clear( lua_State* pL );

		friend void*		GetPointerFromLua( lua_State* pL, int32 nStkId );
		friend void			PushPointerToLua( lua_State* pL, void* pBuffer );
	};

	inline bool CLuaBuffer::IsLightData( SBufferInfo* pInfo )
	{
		return (void*)( pInfo + 1 ) != pInfo->pBuffer;
	}

	inline SBufferInfo* CLuaBuffer::CheckBufferSpace( 
		SBufferInfo* pInfo, uint32 nTotalSize, lua_State* pL, int32 nStkID )
	{
		if( pInfo && IsLightData( pInfo ) && nTotalSize > pInfo->nDataSize )
		{
			luaL_error( pL, "can not write data to native buffer over nDataSize" );
			return nullptr;
		}

		if( pInfo == nullptr || pInfo->pBuffer == nullptr || nTotalSize > pInfo->nCapacity )
		{
			lua_pushlightuserdata( pL, (void*)s_szLuaBufferInfo );
			uint32 nLen = std::max<uint32>( nTotalSize, 16 );
			uint32 nCapacity = nLen + nLen/2;
			uint32 nAlocSize = nCapacity + sizeof(SBufferInfo);
			SBufferInfo* pNewBufferInfo = (SBufferInfo*)lua_newuserdata( pL, nAlocSize );
			pNewBufferInfo->pBuffer = (tbyte*)( pNewBufferInfo + 1 );
			pNewBufferInfo->nPosition = pInfo ? pInfo->nPosition : 0;
			pNewBufferInfo->nDataSize = pInfo ? pInfo->nDataSize : 0;
			pNewBufferInfo->nCapacity = nCapacity;

			if( pInfo && pInfo->pBuffer )
				memcpy( pNewBufferInfo->pBuffer, pInfo->pBuffer, pInfo->nDataSize );
			pInfo = pNewBufferInfo;
			lua_rawset( pL, nStkID );
			uint32 nNewStart = pInfo->nDataSize;
			uint32 nNewSize = nCapacity - pInfo->nDataSize;
			memset( pNewBufferInfo->pBuffer + nNewStart, 0, nNewSize );
		}	
		return pInfo;
	}

	inline SBufferInfo* CLuaBuffer::GetBufferInfo( lua_State* pL, int32 nStkID )
	{
		nStkID = ToAbsStackIndex(pL, nStkID);
		lua_pushlightuserdata( pL, (void*)s_szLuaBufferInfo );
		lua_rawget( pL, nStkID );
		SBufferInfo* pInfo = (SBufferInfo*)lua_touserdata( pL, -1 );
		lua_pop( pL, 1 );
		return pInfo;
	}

	template<class Type>
	Type CLuaBuffer::ReadData( lua_State* pL )
	{
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		if( !pInfo || pInfo->nPosition + sizeof(Type) > pInfo->nDataSize )
		{
			luaL_error( pL, "invalid buffer" );
			return 0;
		}
		Type data;
		memcpy( &data, pInfo->pBuffer + pInfo->nPosition, sizeof(Type) );
		pInfo->nPosition += sizeof(Type);
		return data;
	}

	int32 CLuaBuffer::GetBit( lua_State* pL )
	{
		uint32 nArg = lua_gettop( pL );
		if( nArg < 2 )
		{
			luaL_error( pL, "GetBit Invalid Param" );
			return 0;
		}

		uint32 nReadPos = (uint32)GetNumFromLua( pL, 2 );
		uint32 nNum = 1;
		if( nArg > 3 )
			nNum = (uint32)GetNumFromLua( pL, 3 );
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		if( nNum <= 0 || nNum > 64 )
			return 0;
		if( nReadPos + nNum > pInfo->nDataSize * 8 )
			return 0;
		int32 nOffset = nReadPos%8;
		int32 nStart = nReadPos/8;
		int32 nEnd = ( nReadPos + nNum - 1 )/8 + 1;
		char aryBuffer[9];
		memcpy( aryBuffer, pInfo->pBuffer, nEnd - nStart );
		uint64& nValue = *(uint64*)aryBuffer;
		nValue = nValue >> nOffset;
		aryBuffer[7] |= aryBuffer[8] << ( 8 - nOffset );
		lua_pushnumber( pL, (double)(int64)nValue );
		return 1;
	}

	int32 CLuaBuffer::ReadBoolean( lua_State* pL )
	{
		lua_pushboolean( pL, ReadData<uint8>( pL ) );		
		return 1;
	}

	int32 CLuaBuffer::ReadInt8( lua_State* pL )
	{
		lua_pushnumber( pL, ReadData<int8>( pL ) );
		return 1;
	}

	int32 CLuaBuffer::ReadDouble( lua_State* pL )
	{
		lua_pushnumber( pL, ReadData<double>( pL ) );
		return 1;
	}

	int32 CLuaBuffer::ReadFloat( lua_State* pL )
	{
		lua_pushnumber( pL, ReadData<float>( pL ) );
		return 1;
	}

	int32 CLuaBuffer::ReadInt64( lua_State* pL )
	{
		lua_pushnumber( pL, (double)ReadData<int64>( pL ) );
		return 1;
	}

	int32 CLuaBuffer::ReadInt32( lua_State* pL )
	{
		lua_pushnumber( pL, ReadData<int32>( pL ) );
		return 1;
	}

	int32 CLuaBuffer::ReadInt16( lua_State* pL )
	{
		lua_pushnumber( pL, ReadData<int16>( pL ) );
		return 1;
	}

	int32 CLuaBuffer::ReadUint8( lua_State* pL )
	{
		lua_pushnumber( pL, ReadData<uint8>( pL ) );
		return 1;
	}

	int32 CLuaBuffer::ReadUint64( lua_State* pL )
	{
		lua_pushnumber( pL, (double)ReadData<uint64>( pL ) );
		return 1;
	}

	int32 CLuaBuffer::ReadUint32( lua_State* pL )
	{
		lua_pushnumber( pL, ReadData<uint32>( pL ) );
		return 1;
	}

	int32 CLuaBuffer::ReadUint16( lua_State* pL )
	{
		lua_pushnumber( pL, ReadData<uint16>( pL ) );
		return 1;
	}

	int32 CLuaBuffer::ReadUTF( lua_State* pL )
	{
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		if( !pInfo || pInfo->nPosition + sizeof(uint16) > pInfo->nDataSize )
		{
			luaL_error( pL, "invalid buffer" );
			return 0;
		}
		uint16 nLen;
		memcpy( &nLen, pInfo->pBuffer + pInfo->nPosition, sizeof(uint16) );
		const char* szUtf8 = (const char*)( pInfo->pBuffer + pInfo->nPosition + sizeof(uint16) );
		
		if( pInfo->nPosition + sizeof(uint16) + nLen > pInfo->nDataSize )
		{
			luaL_error( pL, "invalid buffer" );
			return 0;
		}
		lua_pushlstring( pL, szUtf8, nLen );
		pInfo->nPosition += sizeof(uint16) + nLen;
		return 1;
	}

	int32 CLuaBuffer::ReadUTFBytes( lua_State* pL )
	{
		uint32 nLen = (uint32)GetNumFromLua( pL, -1 );
		lua_pop( pL, 1 );

		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		if( !pInfo || pInfo->nPosition + nLen > pInfo->nDataSize )
		{
			luaL_error( pL, "invalid buffer" );
			return 0;
		}
		const char* szUtf8 = (const char*)pInfo->pBuffer + pInfo->nPosition;
		lua_pushlstring( pL, szUtf8, strnlen( szUtf8, nLen ) );
		pInfo->nPosition += nLen;
		return 1;
	}

	int32 CLuaBuffer::ReadUCS( lua_State* pL )
	{
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		if( !pInfo || pInfo->nPosition + sizeof(uint16) > pInfo->nDataSize )
		{
			luaL_error( pL, "invalid buffer" );
			return 0;
		}


		uint16 nLen;
		memcpy( &nLen, pInfo->pBuffer + pInfo->nPosition, sizeof(uint16) );
		const uint16* szUtf16 = (const uint16*)( pInfo->pBuffer + pInfo->nPosition + sizeof(uint16) );
		if( pInfo->nPosition + sizeof(uint16) + nLen*sizeof(uint16) > pInfo->nDataSize)
		{
			luaL_error( pL, "invalid buffer" );
			return 0;
		}
		pInfo->nPosition += sizeof(uint16) + nLen*sizeof(uint16);
		CScriptLua* pScript = CScriptLua::GetScript( pL );
		pScript->m_szTempUtf8.resize( nLen*6 + 1 );
		nLen = Ucs2ToUtf8( &pScript->m_szTempUtf8[0], nLen*6+1, szUtf16 );
		lua_pushlstring( pL, pScript->m_szTempUtf8.c_str(), nLen );
		return 1;
	}

	int32 CLuaBuffer::ReadUCSCounts( lua_State* pL )
	{
		uint32 nLen = (uint32)GetNumFromLua( pL, 2 );
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		if( !pInfo || pInfo->nPosition + nLen*sizeof(uint16) > pInfo->nDataSize )
		{
			luaL_error( pL, "invalid buffer" );
			return 0;
		}

		lua_settop( pL, 0 );

		const uint16* szUtf16 = (const uint16*)( pInfo->pBuffer + pInfo->nPosition );
		pInfo->nPosition += nLen*sizeof(uint16);

		CScriptLua* pScript = CScriptLua::GetScript( pL );
		pScript->m_szTempUtf8.resize( nLen*6 + 1 );
		nLen = Ucs2ToUtf8( &pScript->m_szTempUtf8[0], nLen*6+1, szUtf16 );
		lua_pushlstring( pL, pScript->m_szTempUtf8.c_str(), nLen );
		return 1;
	}

	int32 CLuaBuffer::ReadBytes( lua_State* pL )
	{
		uint32 nArg = lua_gettop( pL );
		SBufferInfo* pInfoSrc = GetBufferInfo( pL, 1 );

		if( !pInfoSrc || !pInfoSrc->pBuffer )
		{
			luaL_error( pL, "invalid buffer" );
			return 0;
		}

		if( lua_type( pL, 2 ) != LUA_TTABLE )
		{
			uint32 nReadCount = nArg >= 2 ? (uint32)GetNumFromLua( pL, 2 ) : INVALID_32BITID;

			if( nReadCount == INVALID_32BITID )
				nReadCount = pInfoSrc->nDataSize - pInfoSrc->nPosition;
			lua_settop( pL, 0 );

			if( pInfoSrc->nPosition + nReadCount > pInfoSrc->nDataSize )
			{
				luaL_error( pL, "invalid buffer" );
				return 0;
			}

			lua_pushlstring( pL, (char*)pInfoSrc->pBuffer + pInfoSrc->nPosition, nReadCount );
			pInfoSrc->nPosition += nReadCount;
			return 1;
		}
		else
		{
			SBufferInfo* pInfoDes = GetBufferInfo( pL, 2 );
			uint32 nOffset = nArg >= 3 ? (uint32)GetNumFromLua( pL, 3 ) : 0;
			uint32 nReadCount = nArg >= 4 ? (uint32)GetNumFromLua( pL, 4 ) : INVALID_32BITID;
			tbyte* pDesBuf = nullptr;
			uint32 nMaxSize = 0;

			if( nReadCount == INVALID_32BITID )
				nReadCount = pInfoSrc->nDataSize - pInfoSrc->nPosition;

			if( pInfoSrc->nPosition + nReadCount > pInfoSrc->nDataSize )
			{
				luaL_error( pL, "invalid buffer" );
				return 0;
			}

			if( nReadCount > 200*1024*1024 )
			{
				luaL_error( pL, "invalid size" );
				return 0;
			}

			if( lua_isnil( pL, 2 ) )
			{
				pInfoSrc = CheckBufferSpace( pInfoSrc, nOffset + nReadCount, pL, 2 );
				pDesBuf = pInfoSrc->pBuffer;
				nMaxSize = pInfoSrc->nDataSize;
				pInfoSrc->nDataSize = std::max<uint32>( nOffset + nReadCount, pInfoSrc->nDataSize );
			}
			else if( pInfoDes && pInfoDes->pBuffer )
			{
				pInfoDes = CheckBufferSpace( pInfoDes, nOffset + nReadCount, pL, 2 );
				pDesBuf = pInfoDes->pBuffer;
				nMaxSize = pInfoDes->nDataSize;
				pInfoDes->nDataSize = std::max<uint32>( nOffset + nReadCount, pInfoDes->nDataSize );
			}
			else
			{
				lua_pushstring( pL, "(cppobjs)" );
				lua_rawget( pL, 2 );
				if( lua_istable( pL, -1 ) )
				{
					lua_pushlightuserdata( pL, CScriptLua::ms_pFirstClassInfo );
					lua_gettable( pL, 2 );
					if( lua_isuserdata( pL, -1 ) )
					{
						auto pClassInfo = (const CClassInfo*)lua_touserdata( pL, -1 );
						nMaxSize = pClassInfo->GetClassSize();
						if( nOffset + nReadCount > nMaxSize ) 
						{
							luaL_error( pL, "invalid size" );
							return 0;
						}
						lua_rawget( pL, -2 );
						pDesBuf = (tbyte*)lua_touserdata( pL, -1 );
						lua_pop( pL, 2 );
					}
				}
			}

			lua_settop( pL, 0 );
			memmove( pDesBuf + nOffset, pInfoSrc->pBuffer + pInfoSrc->nPosition, nReadCount );
			pInfoSrc->nPosition += nReadCount;
			return 0;
		}
	}

	template<class Type>
	void CLuaBuffer::WriteData( lua_State* pL, Type v )
	{
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		pInfo = CheckBufferSpace( pInfo, ( pInfo ? pInfo->nPosition : 0 )  + sizeof(Type), pL, 1 );
		memcpy( pInfo->pBuffer + pInfo->nPosition, &v, sizeof(Type) );
		pInfo->nPosition += sizeof(Type);
		pInfo->nDataSize = std::max<uint32>( pInfo->nPosition, pInfo->nDataSize );
		lua_settop( pL, 0 );
	}

	int32 CLuaBuffer::SetBit( lua_State* pL )
	{
		uint32 nArg = lua_gettop( pL );
		if( nArg < 3 )
		{
			luaL_error( pL, "invalid parameter count" );
			return 0;
		}

		char aryBuffer[9];
		uint64& nValue = *(uint64*)aryBuffer;
		uint32 nWritePos = (uint32)GetNumFromLua( pL, 2 );
		nValue = (uint64)( LUA_TBOOLEAN == lua_type( pL, 3 ) ?
			lua_toboolean( pL, 3 ) : (int64)GetNumFromLua( pL, 3 ) );

		uint32 nNum = 1;
		if( nArg > 4 )
			nNum = (uint32)GetNumFromLua( pL, 4 );
		if( nNum <= 0 || nNum > 64 )
			return 0;
		int32 nOffset = nWritePos%8;
		int32 nStart = nWritePos/8;
		int32 nEnd = ( nWritePos + nNum - 1 )/8 + 1;
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		pInfo = CheckBufferSpace( pInfo, nEnd, pL, 1 );

		aryBuffer[8] = aryBuffer[7] >> ( 8 - nOffset );
		nValue = nValue << nOffset;
		tbyte* pStart = pInfo->pBuffer + nStart;
		pStart[0] = ( pStart[0]&( 0xff >> nOffset ) )|aryBuffer[0];
		memcpy( pStart + 1, aryBuffer + 1, 7 );
		pStart[8] = ( pStart[8]&( 0xff << nOffset ) )|aryBuffer[8];
		return 0;
	}

	int32 CLuaBuffer::WriteBoolean( lua_State* pL )
	{
		WriteData( pL, (uint8)(int64)lua_toboolean( pL, 2 ) );
		return 0;
	}

	int32 CLuaBuffer::WriteInt8( lua_State* pL )
	{
		WriteData( pL, (int8)(int64)GetNumFromLua( pL, 2 ) );
		return 0;
	}

	int32 CLuaBuffer::WriteUint8( lua_State* pL )
	{
		WriteData( pL, (uint8)(int64)GetNumFromLua( pL, 2 ) );
		return 0;
	}

	int32 CLuaBuffer::WriteDouble( lua_State* pL )
	{
		WriteData( pL, GetNumFromLua( pL, 2 ) );
		return 0;
	}

	int32 CLuaBuffer::WriteFloat( lua_State* pL )
	{
		WriteData( pL, (float)GetNumFromLua( pL, 2 ) );
		return 0;
	}

	int32 CLuaBuffer::WriteInt64( lua_State* pL )
	{
		WriteData( pL, (int64)GetNumFromLua( pL, 2 ) );
		return 0;
	}

	int32 CLuaBuffer::WriteInt32( lua_State* pL )
	{
		WriteData( pL, (int32)(int64)GetNumFromLua( pL, 2 ) );
		return 0;
	}

	int32 CLuaBuffer::WriteUint32( lua_State* pL )
	{
		WriteData( pL, (uint32)(int64)GetNumFromLua( pL, 2 ) );
		return 0;
	}

	int32 CLuaBuffer::WriteUint64( lua_State* pL )
	{
		WriteData( pL, (uint64)GetNumFromLua( pL, 2 ) );
		return 0;
	}
	
	int32 CLuaBuffer::WriteInt16( lua_State* pL )
	{
		WriteData( pL, (int16)(int64)GetNumFromLua( pL, 2 ) );
		return 0;
	}

	int32 CLuaBuffer::WriteUint16( lua_State* pL )
	{
		WriteData( pL, (uint16)(int64)GetNumFromLua( pL, 2 ) );
		return 0;
	}

	int32 CLuaBuffer::WriteUTF( lua_State* pL )
	{
		const char* szString = lua_tostring( pL, 2 );
		szString = szString ? szString : "";
		uint16 nSize = (uint16)strlen( szString );
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		uint32 nNewSize = ( pInfo ? pInfo->nPosition : 0 ) + nSize + sizeof(uint16);
		pInfo = CheckBufferSpace( pInfo, nNewSize, pL, 1 );
		lua_settop( pL, 0 );

		memcpy( pInfo->pBuffer + pInfo->nPosition, &nSize, sizeof(uint16) );
		memcpy( pInfo->pBuffer + pInfo->nPosition + sizeof(uint16), szString, nSize );
		pInfo->nPosition += nSize + sizeof(uint16);
		pInfo->nDataSize = std::max<uint32>( pInfo->nPosition, pInfo->nDataSize );
		return 0;
	}

	int32 CLuaBuffer::WriteUTFBytes( lua_State* pL )
	{
		uint32 nArg = lua_gettop( pL );
		uint32 nLen = (uint32)lua_strlen( pL, 2 );
		const char* szString = lua_tostring( pL, 2 );
		uint32 nSize = nArg >= 3 ? (uint32)(int64)GetNumFromLua( pL, 3 ) : nLen;
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		pInfo = CheckBufferSpace( pInfo, ( pInfo ? pInfo->nPosition : 0 ) + nSize, pL, 1 );
		lua_settop( pL, 0 );

		szString = szString ? szString : "";
		memcpy( pInfo->pBuffer + pInfo->nPosition, szString, std::min( nSize, nLen ) );
		if( nSize > nLen )
			memset( pInfo->pBuffer + pInfo->nPosition + nLen, 0, nSize - nLen );
		pInfo->nPosition += nSize;
		pInfo->nDataSize = std::max<uint32>( pInfo->nPosition, pInfo->nDataSize );
		return 0;
	}

	int32 CLuaBuffer::WriteBytes( lua_State* pL )
	{
		uint32 nArg = lua_gettop( pL );
		SBufferInfo* pInfoDes = GetBufferInfo( pL, 1 );

		if( lua_type( pL, 2 ) != LUA_TTABLE )
		{
			size_t nDataSize = 0;
			const char* szSrc = luaL_checklstring( pL, 2, &nDataSize );
			uint32 nOffset = nArg >= 3 ? (uint32)GetNumFromLua( pL, 3 ) : 0;
			uint32 nWriteCount = nArg >= 4 ? (uint32)GetNumFromLua( pL, 4 ) : INVALID_32BITID;

			if( nWriteCount == INVALID_32BITID )
				nWriteCount = (uint32)nDataSize - nOffset;

			if( nWriteCount > 200*1024*1024 )
			{
				luaL_error( pL, "invalid size" );
				return 0;
			}

			if( nOffset + nWriteCount > nDataSize )
			{
				luaL_error( pL, "invalid buffer" );
				return 0;
			}

			pInfoDes = CheckBufferSpace( pInfoDes, ( pInfoDes ? pInfoDes->nPosition : 0 ) + nWriteCount, pL, 1 );
			lua_settop( pL, 0 );

			memmove( pInfoDes->pBuffer + pInfoDes->nPosition, szSrc + nOffset, nWriteCount );
			pInfoDes->nPosition += nWriteCount;
			pInfoDes->nDataSize = std::max<uint32>( pInfoDes->nPosition, pInfoDes->nDataSize );
			return 0;
		}
		else
		{
			SBufferInfo* pInfoSrc = GetBufferInfo( pL, 2 );
			uint32 nOffset = nArg >= 3 ? (uint32)(int64)GetNumFromLua( pL, 3 ) : 0;
			uint32 nWriteCount = nArg >= 4 ? (uint32)(int64)GetNumFromLua( pL, 4 ) : INVALID_32BITID;
			const tbyte* pSrcBuf = nullptr;
			uint32 nMaxSize = 0;

			if( lua_isnil( pL, 2 ) )
			{
				pSrcBuf = pInfoDes->pBuffer;
				nMaxSize = pInfoDes->nDataSize;
			}
			else if( pInfoSrc && pInfoSrc->pBuffer )
			{
				pSrcBuf = pInfoSrc->pBuffer;
				nMaxSize = pInfoSrc->nDataSize;
			}
			else
			{
				lua_pushstring( pL, "(cppobjs)" );
				lua_rawget( pL, 2 );
				if( lua_istable( pL, -1 ) )
				{
					lua_pushlightuserdata( pL, CScriptLua::ms_pFirstClassInfo );
					lua_gettable( pL, 2 );
					if( lua_isuserdata( pL, -1 ) )
					{
						auto pClassInfo = (const CClassInfo*)lua_touserdata( pL, -1 );
						nMaxSize = pClassInfo->GetClassSize();
						lua_rawget( pL, -2 );
						pSrcBuf = (const tbyte*)lua_touserdata( pL, -1 );
						lua_pop( pL, 2 );
					}
				}
			}

			if( nWriteCount == INVALID_32BITID )
				nWriteCount = nMaxSize - nOffset;

			if( pSrcBuf == nullptr || nOffset + nWriteCount > nMaxSize)
			{
				luaL_error( pL, "invalid buffer" );
				return 0;
			}

			if( nWriteCount > 200 * 1024 * 1024 )
			{
				luaL_error( pL, "invalid size" );
				return 0;
			}

			pInfoDes = CheckBufferSpace( pInfoDes, (pInfoDes ? pInfoDes->nPosition : 0) + nWriteCount, pL, 1 );
			lua_settop( pL, 0 );

			memmove( pInfoDes->pBuffer + pInfoDes->nPosition, pSrcBuf + nOffset, nWriteCount );
			pInfoDes->nPosition += nWriteCount;
			pInfoDes->nDataSize = std::max<uint32>( pInfoDes->nPosition, pInfoDes->nDataSize );
			return 0;
		}
	}

	int32 CLuaBuffer::SetPosition( lua_State* pL )
	{
		uint32 nPosition = (uint32)(int64)GetNumFromLua( pL, 2 );
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		if( pInfo && IsLightData( pInfo ) )
		{
			if( nPosition >= pInfo->nDataSize )
				nPosition = pInfo->nDataSize;
			pInfo->nPosition = nPosition;
			return 0;
		}
		if( !pInfo )
			pInfo = CheckBufferSpace( pInfo, nPosition, pL, 1 );
		pInfo->nPosition = nPosition;
		lua_settop( pL, 0 );
		return 0;
	}

	int32 CLuaBuffer::GetPosition( lua_State* pL )
	{
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		lua_settop( pL, 0 );
		lua_pushnumber( pL, pInfo ? pInfo->nPosition : 0 );
		return 1;
	}

	int32 CLuaBuffer::SetDataSize( lua_State* pL )
	{
		uint32 nLen = (uint32)(int64)GetNumFromLua( pL, 2 );
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		if( pInfo && pInfo->pBuffer && IsLightData( pInfo ) )
		{
			if( nLen < pInfo->nCapacity )
				pInfo->nDataSize = nLen;
			return 0;
		}

		pInfo = CheckBufferSpace( pInfo, nLen, pL, 1 );
		pInfo->nDataSize = nLen;
		lua_settop( pL, 0 );
		return 0;
	}

	int32 CLuaBuffer::GetDataSize( lua_State* pL )
	{
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		lua_settop( pL, 0 );
		lua_pushnumber( pL, pInfo ? pInfo->nDataSize : 0 );
		return 1;
	}

	int32 CLuaBuffer::Reset( lua_State* pL )
	{
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );
		if( !pInfo || IsLightData( pInfo ) )
			return 0;
		pInfo->nPosition = 0;
		pInfo->nDataSize = 0;
		memset( pInfo->pBuffer, 0, pInfo->nCapacity );
		return 0;
	}
	
	int32 CLuaBuffer::Clear( lua_State* pL )
	{
		uint32 nArg = lua_gettop( pL );
		uint32 nClearCount = nArg >= 3 ? (uint32)(int64)GetNumFromLua( pL, 3 ) : 1;
		uint32 nOffset = nArg >= 2 ? (uint32)(int64)GetNumFromLua( pL, 2 ) : 0;
		SBufferInfo* pInfo = GetBufferInfo( pL, 1 );

		if( nClearCount == INVALID_32BITID || nArg < 2 )
			nClearCount = pInfo->nDataSize - nOffset;

		pInfo = CheckBufferSpace( pInfo, nOffset + nClearCount, pL, 1 );
		lua_settop( pL, 0 );

		memset( pInfo->pBuffer + nOffset, 0, nClearCount );
		pInfo->nDataSize = std::max<uint32>( nOffset + nClearCount, pInfo->nDataSize );
		return 0;
	}

	void RegisterPointerClass( CScriptLua* pScript )
	{
		char szSrc[256];
		char_stream( szSrc ) << s_szLuaBufferClass << " = class();";
		pScript->RunString( szSrc );

		lua_State* pL = pScript->GetLuaState();
		lua_getglobal( pL, s_szLuaBufferClass );

		#define REGISTER( f ) \
		lua_pushcfunction( pL, CLuaBuffer::f ); \
		lua_setfield( pL, -2, #f )

		REGISTER( GetBit );
		REGISTER( ReadBoolean );
		REGISTER( ReadInt8 );
		REGISTER( ReadDouble );
		REGISTER( ReadFloat );
		REGISTER( ReadInt64 );
		REGISTER( ReadInt32 );
		REGISTER( ReadInt16 );
		REGISTER( ReadUint8 );
		REGISTER( ReadUint64 );
		REGISTER( ReadUint32 );
		REGISTER( ReadUint16 );
		REGISTER( ReadUTF );
		REGISTER( ReadUTFBytes );
		REGISTER( ReadUCS );
		REGISTER( ReadUCSCounts );
		REGISTER( ReadBytes );

		REGISTER( SetBit );
		REGISTER( WriteBoolean );
		REGISTER( WriteInt8 );
		REGISTER( WriteDouble );
		REGISTER( WriteUint8 );
		REGISTER( WriteFloat );
		REGISTER( WriteInt64 );
		REGISTER( WriteInt32 );
		REGISTER( WriteUint64 );
		REGISTER( WriteUint32 );
		REGISTER( WriteInt16 );
		REGISTER( WriteUint16 );
		REGISTER( WriteUTF );
		REGISTER( WriteUTFBytes );
		REGISTER( WriteBytes );

		REGISTER( SetPosition );
		REGISTER( GetPosition );
		REGISTER( GetDataSize );
		REGISTER( SetDataSize );
		REGISTER( Reset );
		REGISTER( Clear );

        lua_pop( pL, 1 );
	}

	//=====================================================================
	/// lua对C++数据的操作方法封装
	//=====================================================================
	double GetNumFromLua( lua_State* pL, int32 nStkId )
	{
		nStkId = ToAbsStackIndex( pL, nStkId );
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

	void* GetPointerFromLua( lua_State* pL,
		int32 nStkId, int32 nCppObjStr, int32 nWeakTable )
	{
		nStkId = ToAbsStackIndex( pL, nStkId );
		if (nStkId > lua_gettop(pL))
			return nullptr;
		int32 nType = lua_type( pL, nStkId );
		if( nType == LUA_TNIL || nType == LUA_TNONE || 
			nType == LUA_TBOOLEAN || nType == LUA_TNUMBER )
			return nullptr;

		bool bDone = false;
		void* pPointer = nullptr;
		if( nType == LUA_TSTRING )
		{
			pPointer = (void*)lua_tostring( pL, nStkId );
			bDone = true;
		}
		else if( nType == LUA_TUSERDATA || nType == LUA_TLIGHTUSERDATA )
		{
			pPointer = lua_touserdata( pL, nStkId );
			bDone = true;
		}
		else if( nType == LUA_TTABLE )
		{
			SBufferInfo* pInfo = nullptr;
			if( (pInfo = CLuaBuffer::GetBufferInfo( pL, nStkId )) )
			{
				pPointer = pInfo->pBuffer;
				bDone = true;
			}
			else
			{
				lua_pushvalue( pL, nCppObjStr );
				lua_rawget( pL, nStkId );
				if( lua_istable( pL, -1 ) )
				{
					lua_pushlightuserdata( pL, CScriptLua::ms_pFirstClassInfo );
					lua_gettable( pL, nStkId );
					if( lua_isuserdata( pL, -1 ) )
					{
						lua_rawget( pL, -2 );
						pPointer = lua_touserdata( pL, -1 );
					}
					lua_pop( pL, 1 );
					bDone = true;
				}
				lua_pop( pL, 1 );
			}
		}

		if( bDone && pPointer == nullptr )
			return pPointer;

		if( pPointer == nullptr )
			pPointer = (void*)lua_topointer( pL, nStkId );
		lua_pushlightuserdata( pL, pPointer );
		lua_pushvalue( pL, nStkId );
		lua_rawset( pL, nWeakTable );
		return pPointer;
	}

	bool PushPointerToLua( lua_State* pL, void* pBuffer, 
		int32 nCppObjStr, int32 nWeakTable, bool bCreateStreamBuff )
	{
		if( pBuffer == nullptr )
		{
			lua_pushnil( pL );
			return false;
		}

		lua_pushlightuserdata( pL, pBuffer );
		lua_rawget( pL, nWeakTable );

		int32 type = lua_type( pL, -1 );
		if( type != LUA_TNIL )
		{
			if( type == LUA_TTABLE )
			{
				SBufferInfo* pInfo = CLuaBuffer::GetBufferInfo( pL, -1 );
				if( pInfo && pInfo->pBuffer == pBuffer )
				{
					pInfo->nDataSize = (uint32)INVALID_32BITID;
					pInfo->nCapacity = (uint32)INVALID_32BITID;
					pInfo->nPosition = 0;
					return true;
				}
			}

			if (type == LUA_TTABLE || type == LUA_TUSERDATA ||
				type == LUA_TFUNCTION || type == LUA_TTHREAD)
			{
				return true;
			}

			CScriptLua::GetScript( pL )->UnlinkCppObjFromScript( pBuffer );
		}

		if( !bCreateStreamBuff )
			return false;

		lua_pop( pL, 1 );

		lua_newtable( pL );// Obj
		int32 nStkId = ToAbsStackIndex( pL, -1 );

		lua_getglobal( pL, s_szLuaBufferClass );
		if( lua_isnil( pL, -1 ) )//szClass必须被注册
		{
			luaL_error( pL, "PushToVM Class:%s", s_szLuaBufferClass );
			return false;
		}

		lua_setmetatable( pL, nStkId );

		// 设置数据到table上
		lua_pushlightuserdata( pL, (void*)s_szLuaBufferInfo );
		SBufferInfo* pInfo = (SBufferInfo*)lua_newuserdata( pL, sizeof( SBufferInfo ) );
		pInfo->nDataSize = (uint32)INVALID_32BITID;
		pInfo->nCapacity = (uint32)INVALID_32BITID;
		pInfo->nPosition = 0;
		pInfo->pBuffer = (tbyte*)pBuffer;
		lua_rawset( pL, nStkId );

		// 挂到全局表上
		lua_pushlightuserdata( pL, pBuffer );
		lua_pushvalue( pL, nStkId );
		lua_settable( pL, nWeakTable );
		return true;
	}

	//=====================================================================
	/// 所有Lua数据类型
	//=====================================================================
	static CGlobalTypes s_listTypes( 
		GlobalTypeTemplateArgs( TLuaValue, CLuaObject, CLuaValueObject ) );

	XS::CLuaTypeBase* GetLuaTypeBase( DataType eType )
	{
		return s_listTypes.GetTypeImp<CLuaTypeBase>( eType );
	}
}
