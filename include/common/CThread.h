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
	uint32		GammaGetCurrentProcessID();
	void		GammaGetCurrentProcessPath( char* szBuffer, size_t nCount );
	bool		GammaCheckProcessExist( uint32 nProcessID );
	uint64		GammaGetProcessMemCost();
	float		GammaGetProcessCpuCost();

    //======================================================================
    // Sleep
    //======================================================================
    void		GammaSleep( uint32 uMilliSecond );

    //======================================================================
    // Gamma Thread
    //======================================================================
    bool		GammaCreateThread( HTHREAD* phThread, uint32 nStackSize, THREADPROC pThreadFun, void* pParam );
    void		GammaDetachThread( HTHREAD hThread );
    void		GammaExitThread( uint32 uExitCode );
    bool		GammaTerminateThread( HTHREAD hThread, uint32 uExitCode );
	uint64		GammmaGetCurrentThreadID();
    bool		GammaIsCurrentThread( uint64 nThreadID );
    bool		GammaJoinThread( HTHREAD hThread );
    bool		GammaSetThreadPriority( HTHREAD hThread, int32 nPriority );

    //======================================================================
    // Gamma Lock
    //======================================================================
    HLOCK       GammaCreateLock();
    bool        GammaDestroyLock( HLOCK hLock );
    void        GammaLock( HLOCK hLock );
    void        GammaUnlock( HLOCK hLock );


	//======================================================================
	// Semaphore 
	//======================================================================
	HSEMAPHORE GammaCreateSemaphore( int nInitCount, int nMaxCount );
	HSEMAPHORE GammaCreateSemaphore();
	void		GammaPutSemaphore( HSEMAPHORE hSemaphore );
	bool		GammaGetSemaphore( HSEMAPHORE hSemaphore );
	int32		GammaGetSemaphore( HSEMAPHORE hSemaphore, uint32 nMilliSecs );
	int32		GammaDestroySemaphore( HSEMAPHORE hSemaphore );

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

