//=====================================================================
// CThread.h 
// 定义多线程的相关操作操作
// 柯达昭
// 2007-08-31
//=====================================================================
#pragma once

#include "common/Help.h"
#include <string.h>

namespace Gamma
{    

#define GAMMA_THREAD_PRIORITY_IDLE			-15
#define GAMMA_THREAD_PRIORITY_LOWEST		-2
#define GAMMA_THREAD_PRIORITY_BELOW_NORMAL	-1
#define GAMMA_THREAD_PRIORITY_NORMAL		0
#define GAMMA_THREAD_PRIORITY_ABOVE_NORMAL	1
#define GAMMA_THREAD_PRIORITY_HIGHEST		2
#define GAMMA_THREAD_PRIORITY_CRITICAL		15

    typedef void*				HTHREAD;
    typedef void*				HLOCK;
    typedef void*				HSEMAPHORE;

#ifdef _WIN32
    typedef uint32               (__stdcall *THREADPROC)( void * );
#else
    typedef uint32               (*THREADPROC)(void*);
#endif

	//======================================================================
	// Process
	//======================================================================
	GAMMA_COMMON_API uint32		GammaGetCurrentProcessID();
	GAMMA_COMMON_API void		GammaGetCurrentProcessPath( char* szBuffer, size_t nCount );
	GAMMA_COMMON_API bool		GammaCheckProcessExist( uint32 nProcessID );
	GAMMA_COMMON_API uint64		GammaGetProcessMemCost();
	GAMMA_COMMON_API float		GammaGetProcessCpuCost();

    //======================================================================
    // Sleep
    //======================================================================
    GAMMA_COMMON_API void		GammaSleep( uint32 uMilliSecond );

    //======================================================================
    // Gamma Thread
    //======================================================================
    GAMMA_COMMON_API bool		GammaCreateThread( HTHREAD* phThread, uint32 nStackSize, THREADPROC pThreadFun, void* pParam );
    GAMMA_COMMON_API void		GammaDetachThread( HTHREAD hThread );
    GAMMA_COMMON_API void		GammaExitThread( uint32 uExitCode );
    GAMMA_COMMON_API bool		GammaTerminateThread( HTHREAD hThread, uint32 uExitCode );
	GAMMA_COMMON_API uint64		GammmaGetCurrentThreadID();
    GAMMA_COMMON_API bool		GammaIsCurrentThread( uint64 nThreadID );
    GAMMA_COMMON_API bool		GammaJoinThread( HTHREAD hThread );//相当于等待hThread退出
    GAMMA_COMMON_API bool		GammaSetThreadPriority( HTHREAD hThread, int32 nPriority );

    //======================================================================
    // Gamma Lock
    //======================================================================
    GAMMA_COMMON_API HLOCK       GammaCreateLock();
    GAMMA_COMMON_API bool        GammaDestroyLock( HLOCK hLock );
    GAMMA_COMMON_API void        GammaLock( HLOCK hLock );
    GAMMA_COMMON_API void        GammaUnlock( HLOCK hLock );


	//======================================================================
	// Semaphore 
	//======================================================================
	GAMMA_COMMON_API HSEMAPHORE GammaCreateSemaphore( int nInitCount, int nMaxCount );
	GAMMA_COMMON_API HSEMAPHORE GammaCreateSemaphore();
	GAMMA_COMMON_API void		GammaPutSemaphore( HSEMAPHORE hSemaphore );
	GAMMA_COMMON_API bool		GammaGetSemaphore( HSEMAPHORE hSemaphore );
	GAMMA_COMMON_API int32		GammaGetSemaphore( HSEMAPHORE hSemaphore, uint32 nMilliSecs );
	GAMMA_COMMON_API int32		GammaDestroySemaphore( HSEMAPHORE hSemaphore );

    class CLock
    {
        HLOCK    m_hLock;
    public:
        CLock(): m_hLock( GammaCreateLock() )   {};
        ~CLock()                                { GammaDestroyLock( m_hLock );   };
        HLOCK   GetHandle()                     { return m_hLock;                };
        void    Lock()                          { GammaLock( m_hLock );          };
        void    Unlock()                        { GammaUnlock( m_hLock );        };
    };

    class CGuard
    {
        HLOCK    m_hLock;
    public:
        CGuard( HLOCK hLock ):m_hLock( hLock )  { GammaLock( m_hLock );          };
        ~CGuard()                               { GammaUnlock( m_hLock );        };
    };
}

