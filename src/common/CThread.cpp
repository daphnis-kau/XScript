#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#else
#ifdef _IOS
#include <dispatch/dispatch.h>
#include <mach/mach_time.h>
#else
#include <semaphore.h>
#endif
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#endif

#include "common/CThread.h"
#include "common/Help.h"
#include "common/CodeCvs.h"
#include <sstream>
#include <stdlib.h>

#include "common/CThread.h"

using namespace std;

namespace Gamma
{
#ifdef _WIN32
	typedef CRITICAL_SECTION SLock;
#else
	typedef pthread_mutex_t SLock;
#endif

	//======================================================================
	// Process
	//======================================================================
	uint32 GammaGetCurrentProcessID()
	{
		return (uint32)getpid();
	}

	GAMMA_COMMON_API void GammaGetCurrentProcessPath( char* szBuffer, size_t nCount )
	{
#ifdef _WIN32
		wchar_t szTemp[2048] = { 0 };
		::GetModuleFileName( NULL, szTemp, 2048 );
		UcsToUtf8( szBuffer, (uint32)nCount, szTemp );
#else
		memset( szBuffer, 0, nCount );
		readlink( "/proc/self/exe", szBuffer, nCount );
#endif
	}
	
	bool GammaCheckProcessExist( uint32 nProcessID )
	{
#ifdef _WIN32
		HANDLE hProcess = ::OpenProcess( PROCESS_QUERY_INFORMATION, false, nProcessID );
		::CloseHandle( hProcess );
		return NULL != hProcess;
#else
		int32 nRes = kill( nProcessID, 0 );
		return !nRes || errno != ESRCH;
#endif
	}

	uint64 GammaGetProcessMemCost()
	{
#ifdef _WIN32
		MEMORYSTATUS stat;
		GlobalMemoryStatus( &stat );
		return stat.dwTotalVirtual - stat.dwAvailVirtual;
#else
		return 0;
#endif
	}

	float GammaGetProcessCpuCost()
	{
		return 0;
	}

    //======================================================================
    // Sleep
    //======================================================================
    void GammaSleep( uint32 nMiliSecond )
    {
#ifdef _WIN32
        ::Sleep( nMiliSecond );
#else
		struct timespec ts = {0};
		ts.tv_sec	= nMiliSecond/1000;
		ts.tv_nsec	= nMiliSecond%1000*1000000;
		while( FAILED( nanosleep( &ts, &ts ) ) )
			continue;
#endif
    }

    //======================================================================
    // Gamma Thread
    //======================================================================
    bool GammaCreateThread( HTHREAD* phThread, uint32 nStackSize, THREADPROC pThreadFun, void* pParam )
    {
        if ( pThreadFun == NULL || phThread == NULL )
            return false;

#ifdef _WIN32
        unsigned ThreadID = 0;
        *phThread = (HANDLE)_beginthreadex( 0, nStackSize, pThreadFun, pParam, 0, &ThreadID );
        return *phThread != NULL;
#else
        return SUCCEEDED( pthread_create( (pthread_t*)phThread, NULL, reinterpret_cast<void*(*)(void*)>( pThreadFun ), pParam ) );
#endif
    }

    void GammaDetachThread( HTHREAD hThread )
    {
#ifdef _WIN32
        ::CloseHandle( hThread );
#else
        pthread_detach( (pthread_t)hThread );
#endif
    }

    void GammaExitThread( uint32 uExitCode )
    {
#ifdef _WIN32
        ::_endthreadex(uExitCode);
#else
        pthread_exit( &uExitCode );
#endif
    }

    bool GammaTerminateThread( HTHREAD hThread, uint32 uExitCode )
    {
#ifdef _WIN32
		return ::TerminateThread( hThread, uExitCode ) != 0;
#else
		return pthread_kill( (pthread_t)hThread, SIGUSR1 );
#endif
    }

	uint64 GammmaGetCurrentThreadID()
	{
#ifdef _WIN32
		return ::GetCurrentThreadId();
#else
		return (uint64)(ptrdiff_t)pthread_self();
#endif
	}

	bool GammaIsCurrentThread( uint64 nThreadID )
	{
#ifdef _WIN32
		return ::GetCurrentThreadId() == nThreadID;
#else
		return pthread_equal( (pthread_t)(ptrdiff_t)nThreadID, pthread_self() );
#endif
	}

    bool GammaJoinThread( HTHREAD hThread )
    {
#ifdef _WIN32
        switch ( ::WaitForSingleObject( hThread, INFINITE ) )
        {
        case WAIT_FAILED:
            return false;
        case WAIT_ABANDONED:
        case WAIT_OBJECT_0:
            ::CloseHandle( hThread );
            return true;
        default:
            throw( "WaitForSingleObject failed.");
		}
#else
		return !pthread_join( (pthread_t)hThread, NULL );//相当于等待hThread退出
#endif
    }

    bool GammaSetThreadPriority( HTHREAD hThread, int32 nPriority )
    {
#ifdef _WIN32
		return ::SetThreadPriority( hThread, nPriority ) != 0;
#else
        return true;
#endif
    }

    //======================================================================
    // Gamma Lock
    //======================================================================
    HLOCK GammaCreateLock()
    {
		HLOCK hLock = new SLock;
#ifdef _WIN32
		InitializeCriticalSection( (SLock*)hLock );
#else
		pthread_mutex_init( (SLock*)hLock, NULL );
#endif
        return hLock;
    }

    bool GammaDestroyLock( HLOCK hLock )
    {
		if( !hLock )
			return false;
#ifdef _WIN32
		DeleteCriticalSection( (SLock*)hLock );
		delete (SLock*)hLock;
		return true;
#else
		int nRetCode = pthread_mutex_destroy( (SLock*)hLock );
		delete (SLock*)hLock;
        return nRetCode == 0;
#endif
    }

    void GammaLock( HLOCK hLock )
	{
		if( !hLock )
			return;
#ifdef _WIN32
		EnterCriticalSection( (SLock*)hLock );
#else
		pthread_mutex_lock( (SLock*)hLock );
#endif
    }

    void GammaUnlock( HLOCK hLock )
	{
		if( !hLock )
			return;
#ifdef _WIN32
		LeaveCriticalSection( (SLock*)hLock );
#else
		pthread_mutex_unlock( (SLock*)hLock );
#endif
	}    
    
    //==============================================================
    // Semaphore 信号量支持
    //==============================================================
    HSEMAPHORE GammaCreateSemaphore()
    {
        return GammaCreateSemaphore( 0, 0x7fffffff );
    }
    
    HSEMAPHORE GammaCreateSemaphore( int nInitCount, int nMaxCount )
    {
#ifdef _WIN32
        return CreateSemaphore( NULL, nInitCount, nMaxCount, NULL );
#elif defined _IOS
        return (void*)( dispatch_semaphore_create( nInitCount ) );
#else
        sem_t* phSemaphore = new sem_t;
        sem_init( phSemaphore, 0, nInitCount );
        return phSemaphore;
#endif
    }
    
    void GammaPutSemaphore( HSEMAPHORE hSemaphore )
    {
#ifdef _WIN32
        ReleaseSemaphore( hSemaphore, 1, NULL );
#elif defined _IOS
        dispatch_semaphore_signal( (dispatch_semaphore_t)hSemaphore );
#else
        sem_post( (sem_t*)hSemaphore );
#endif
    }
    
    bool GammaGetSemaphore( HSEMAPHORE hSemaphore )
    {
#ifdef _WIN32
        if ( WAIT_FAILED == WaitForSingleObjectEx( hSemaphore, INFINITE, FALSE ) )
            return false;
#elif defined _IOS
        return dispatch_semaphore_wait( (dispatch_semaphore_t)hSemaphore, DISPATCH_TIME_FOREVER ) == 0;
#else
        while( sem_wait( (sem_t*)hSemaphore ) )
        {
            switch( errno )
            {
                case EINTR:
                    continue;
                default:
                    return false;
            }
        }
#endif
        return true;
    }
    
    int32 GammaGetSemaphore( HSEMAPHORE hSemaphore, unsigned nMilliSecs )
    {
#ifdef _WIN32
        switch( WaitForSingleObjectEx( hSemaphore, nMilliSecs, FALSE ) )
        {
            case WAIT_OBJECT_0:
                break;
            case WAIT_TIMEOUT:
                return 1;
		default:
			return -1;
		}
#elif defined _IOS
		int64 nTime = (int64)nMilliSecs * NSEC_PER_MSEC;
		dispatch_time_t waittime = dispatch_time( DISPATCH_TIME_NOW, nTime );
        return dispatch_semaphore_wait( (dispatch_semaphore_t)hSemaphore, waittime ) == 0;
#else
		struct timespec ts;
		struct timeval tv;
		if( gettimeofday( &tv, NULL ) )
			return false;
		ts.tv_sec = tv.tv_sec + nMilliSecs/1000;
		ts.tv_nsec = tv.tv_usec * 1000 + ( nMilliSecs%1000 )*1000000;

		while( sem_timedwait( (sem_t*)hSemaphore, &ts ) )
		{
			switch(errno)
			{
			case ETIMEDOUT:
				return 1;
			case EINTR:
				continue;
			default:
				return -1;
			}
		}
#endif
		return 0;
	}

	int GammaDestroySemaphore( HSEMAPHORE hSemaphore )
	{
#ifdef _WIN32
		CloseHandle( hSemaphore );
#elif defined _IOS
        dispatch_release( (dispatch_semaphore_t)hSemaphore );
#else
        sem_destroy( (sem_t*)hSemaphore );
#endif
		return true;
	}
}

