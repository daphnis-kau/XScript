//===============================================
// CommonType.h 
// 定义常用类型
// 柯达昭
// 2007-08-30
//===============================================

#ifndef __GAMMA_COMMON_TYPE__
#define __GAMMA_COMMON_TYPE__

#if ( defined( _WIN32 ) && defined( GAMMA_DLL ) )
	#if defined( GAMMA_COMMON_EXPORTS )
		#define GAMMA_COMMON_API __declspec(dllexport)
	#else
		#define GAMMA_COMMON_API __declspec(dllimport)
	#endif
#else
	#define GAMMA_COMMON_API 
#endif


#pragma warning( disable: 4100 )
#pragma warning( disable: 4121 )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4200 )
#pragma warning( disable: 4251 )
#pragma warning( disable: 4297 )
#pragma warning( disable: 4310 )
#pragma warning( disable: 4355 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4819 )
#pragma warning( disable: 4996 )
#pragma warning( error: 4553 )
#pragma warning( error: 4715 )

#ifdef _WIN32
	#define		chdir				_chdir
	#define		itoa				_itoa
#else
	#include <sys/types.h>
	#include <stdint.h>
	#include <stddef.h>
	#define		_vsnprintf			vsnprintf

#ifndef MAX_PATH
	#define		MAX_PATH			260
#endif
#endif

	typedef		signed char			int8;
	typedef		signed short		int16;
	typedef		signed int			int32;
	typedef		signed long long	int64;

	typedef		unsigned char		uint8;
	typedef		unsigned short		uint16;
	typedef		unsigned int		uint32;
	typedef		unsigned long long	uint64;

	typedef		unsigned long		ulong;
	typedef		unsigned char		tbyte;

#endif
