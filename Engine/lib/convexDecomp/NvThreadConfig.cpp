/*

NvThreadConfig.cpp : A simple wrapper class to define threading and mutex locks.

*/
/*!
**
** Copyright (c) 2009 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** Portions of this source has been released with the PhysXViewer application, as well as
** Rocket, CreateDynamics, ODF, and as a number of sample code snippets.
**
** If you find this code useful or you are feeling particularily generous I would
** ask that you please go to http://www.amillionpixels.us and make a donation
** to Troy DeMolay.
**
** DeMolay is a youth group for young men between the ages of 12 and 21.
** It teaches strong moral principles, as well as leadership skills and
** public speaking.  The donations page uses the 'pay for pixels' paradigm
** where, in this case, a pixel is only a single penny.  Donations can be
** made for as small as $4 or as high as a $100 block.  Each person who donates
** will get a link to their own site as well as acknowledgement on the
** donations blog located here http://www.amillionpixels.blogspot.com/
**
** If you wish to contact me you can use the following methods:
**
** Skype ID: jratcliff63367
** Yahoo: jratcliff63367
** AOL: jratcliff1961
** email: jratcliffscarab@gmail.com
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include <cassert>
#include "NvThreadConfig.h"

#if defined(WIN32)

#define _WIN32_WINNT 0x400
#include <windows.h>

#pragma comment(lib,"winmm.lib")

//	#ifndef _WIN32_WINNT

//	#endif
//	#include <windows.h>
//#include <winbase.h>
#endif

#if defined(_XBOX)
	#include <xtl.h>
#endif

#if defined(__linux__) || defined( __APPLE__ )
	//#include <sys/time.h>
	#include <time.h>
	#include <unistd.h>
	#include <errno.h>
	#define __stdcall
#endif

#if defined( __APPLE__ )
   #include <sys/time.h>
#endif

#if defined(__APPLE__) || defined(__linux__)
	#include <pthread.h>
#endif

#if defined( __APPLE__ )
   #define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
#endif


#ifdef	NDEBUG
#define VERIFY( x ) (x)
#else
#define VERIFY( x ) assert((x))
#endif

namespace CONVEX_DECOMPOSITION
{

NxU32 tc_timeGetTime(void)
{
   #if defined(__linux__)
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
   #elif defined( __APPLE__ )
      struct timeval tp;
      gettimeofday(&tp, (struct timezone *)0);
      return tp.tv_sec * 1000 + tp.tv_usec / 1000;
   #elif defined( _XBOX )
      return GetTickCount();
   #else
      return timeGetTime();
   #endif
}

void   tc_sleep(NxU32 ms)
{
   #if defined(__linux__) || defined( __APPLE__ )
      usleep(ms * 1000);
   #else
      Sleep(ms);
   #endif
}

void tc_spinloop()
{
   #ifdef __linux__
      asm ( "pause" );
   #elif defined( _XBOX )
      // Pause would do nothing on the Xbox. Threads are not scheduled.
   #else
      __asm { pause };
   #endif
}

void tc_interlockedExchange(void *dest, const int64_t exchange)
{
   #if defined( __linux__ ) || defined( __APPLE__ )
	  // not working
	  assert(false);
	  //__sync_lock_test_and_set((int64_t*)dest, exchange);
   #elif defined( _XBOX )
   InterlockedExchange((volatile LONG *)dest, exchange);
   #else
      __asm
      {
         mov      ebx, dword ptr [exchange]
         mov      ecx, dword ptr [exchange + 4]
         mov      edi, dest
         mov      eax, dword ptr [edi]
         mov      edx, dword ptr [edi + 4]
         jmp      start
      retry:
         pause
      start:
         lock cmpxchg8b [edi]
         jnz      retry
      };
   #endif
}

NxI32 tc_interlockedCompareExchange(void *dest, NxI32 exchange, NxI32 compare)
{
   #if defined( __linux__ ) || defined( __APPLE__ )
	  // not working
	  assert(false);
	  return 0;
	  //return __sync_val_compare_and_swap((uintptr_t*)dest, exchange, compare);
	  //return __sync_bool_compare_and_swap((uintptr_t*)dest, exchange, compare);
   #elif defined( _XBOX )
     return InterlockedCompareExchange((volatile LONG *)dest, exchange, compare);
   #else
      char _ret;
      //
      __asm
      {
         mov      edx, [dest]
         mov      eax, [compare]
         mov      ecx, [exchange]

         lock cmpxchg [edx], ecx

         setz    al
         mov     byte ptr [_ret], al
      }
      //
      return _ret;
   #endif
}

NxI32 tc_interlockedCompareExchange(void *dest, const NxI32 exchange1, const NxI32 exchange2, const NxI32 compare1, const NxI32 compare2)
{
   #if defined( __linux__ ) || defined( __APPLE__ )
	  // not working
      assert(false);
	  return 0;
	  //uint64_t exchange = ((uint64_t)exchange1 << 32) | (uint64_t)exchange2;
	  //uint64_t compare = ((uint64_t)compare1 << 32) | (uint64_t)compare2;
	  //return __sync_bool_compare_and_swap((int64_t*)dest, exchange, compare);
   #elif defined( _XBOX )
     assert(false);
     return 0;
   #else
      char _ret;
      //
      __asm
      {
         mov     ebx, [exchange1]
         mov     ecx, [exchange2]
         mov     edi, [dest]
         mov     eax, [compare1]
         mov     edx, [compare2]
         lock cmpxchg8b [edi]
         setz    al
         mov     byte ptr [_ret], al
      }
      //
      return _ret;
   #endif
}

class MyThreadMutex : public ThreadMutex
{
public:
  MyThreadMutex(void)
  {
    #if defined(WIN32) || defined(_XBOX)
  	InitializeCriticalSection(&m_Mutex);
    #elif defined(__APPLE__) || defined(__linux__)
  	pthread_mutexattr_t mutexAttr;  // Mutex Attribute
  	VERIFY( pthread_mutexattr_init(&mutexAttr) == 0 );
  	VERIFY( pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE_NP) == 0 );
  	VERIFY( pthread_mutex_init(&m_Mutex, &mutexAttr) == 0 );
  	VERIFY( pthread_mutexattr_destroy(&mutexAttr) == 0 );
    #endif
  }

  ~MyThreadMutex(void)
  {
    #if defined(WIN32) || defined(_XBOX)
  	DeleteCriticalSection(&m_Mutex);
    #elif defined(__APPLE__) || defined(__linux__)
  	VERIFY( pthread_mutex_destroy(&m_Mutex) == 0 );
    #endif
  }

  void lock(void)
  {
    #if defined(WIN32) || defined(_XBOX)
  	EnterCriticalSection(&m_Mutex);
    #elif defined(__APPLE__) || defined(__linux__)
  	VERIFY( pthread_mutex_lock(&m_Mutex) == 0 );
    #endif
  }

  bool tryLock(void)
  {
    #if defined(WIN32) || defined(_XBOX)
  	bool bRet = false;
  	//assert(("TryEnterCriticalSection seems to not work on XP???", 0));
  	bRet = TryEnterCriticalSection(&m_Mutex) ? true : false;
  	return bRet;
    #elif defined(__APPLE__) || defined(__linux__)
  	NxI32 result = pthread_mutex_trylock(&m_Mutex);
  	return (result == 0);
    #endif
  }

  void unlock(void)
  {
    #if defined(WIN32) || defined(_XBOX)
  	LeaveCriticalSection(&m_Mutex);
    #elif defined(__APPLE__) || defined(__linux__)
  	VERIFY( pthread_mutex_unlock(&m_Mutex) == 0 );
    #endif
  }

private:
  #if defined(WIN32) || defined(_XBOX)
	CRITICAL_SECTION m_Mutex;
	#elif defined(__APPLE__) || defined(__linux__)
	pthread_mutex_t  m_Mutex;
	#endif
};

ThreadMutex * tc_createThreadMutex(void)
{
  MyThreadMutex *m = new MyThreadMutex;
  return static_cast< ThreadMutex *>(m);
}

void          tc_releaseThreadMutex(ThreadMutex *tm)
{
  MyThreadMutex *m = static_cast< MyThreadMutex *>(tm);
  delete m;
}

#if defined(WIN32) || defined(_XBOX)
static unsigned long __stdcall _ThreadWorkerFunc(LPVOID arg);
#elif defined(__APPLE__) || defined(__linux__)
static void* _ThreadWorkerFunc(void* arg);
#endif

class MyThread : public Thread
{
public:
  MyThread(ThreadInterface *iface)
  {
    mInterface = iface;
	#if defined(WIN32) || defined(_XBOX)
   	  mThread     = CreateThread(0, 0, _ThreadWorkerFunc, this, 0, 0);
    #elif defined(__APPLE__) || defined(__linux__)
	  VERIFY( pthread_create(&mThread, NULL, _ThreadWorkerFunc, this) == 0 );
	#endif
  }

  ~MyThread(void)
  {
	#if defined(WIN32) || defined(_XBOX)
      if ( mThread )
      {
        CloseHandle(mThread);
        mThread = 0;
      }
	#endif
  }

  void onJobExecute(void)
  {
    mInterface->threadMain();
  }

private:
  ThreadInterface *mInterface;
  #if defined(WIN32) || defined(_XBOX)
    HANDLE           mThread;
  #elif defined(__APPLE__) || defined(__linux__)
    pthread_t mThread;
  #endif
};


Thread      * tc_createThread(ThreadInterface *tinterface)
{
  MyThread *m = new MyThread(tinterface);
  return static_cast< Thread *>(m);
}

void          tc_releaseThread(Thread *t)
{
  MyThread *m = static_cast<MyThread *>(t);
  delete m;
}

#if defined(WIN32) || defined(_XBOX)
static unsigned long __stdcall _ThreadWorkerFunc(LPVOID arg)
#elif defined(__APPLE__) || defined(__linux__)
static void* _ThreadWorkerFunc(void* arg)
#endif
{
  MyThread *worker = (MyThread *) arg;
	worker->onJobExecute();
  return 0;
}


class MyThreadEvent : public ThreadEvent
{
public:
  MyThreadEvent(void)
  {
	#if defined(WIN32) || defined(_XBOX)
      mEvent = ::CreateEventA(NULL,TRUE,TRUE,"ThreadEvent");
	#elif defined(__APPLE__) || defined(__linux__)
	  pthread_mutexattr_t mutexAttr;  // Mutex Attribute
	  VERIFY( pthread_mutexattr_init(&mutexAttr) == 0 );
	  VERIFY( pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE_NP) == 0 );
	  VERIFY( pthread_mutex_init(&mEventMutex, &mutexAttr) == 0 );
	  VERIFY( pthread_mutexattr_destroy(&mutexAttr) == 0 );
	  VERIFY( pthread_cond_init(&mEvent, NULL) == 0 );
	#endif
  }

  ~MyThreadEvent(void)
  {
	#if defined(WIN32) || defined(_XBOX)
    if ( mEvent )
    {
      ::CloseHandle(mEvent);
    }
	#elif defined(__APPLE__) || defined(__linux__)
	  VERIFY( pthread_cond_destroy(&mEvent) == 0 );
	  VERIFY( pthread_mutex_destroy(&mEventMutex) == 0 );
	#endif
  }

  virtual void setEvent(void)  // signal the event
  {
	#if defined(WIN32) || defined(_XBOX)
    if ( mEvent )
    {
      ::SetEvent(mEvent);
    }
	#elif defined(__APPLE__) || defined(__linux__)
	  VERIFY( pthread_mutex_lock(&mEventMutex) == 0 );
	  VERIFY( pthread_cond_signal(&mEvent) == 0 );
	  VERIFY( pthread_mutex_unlock(&mEventMutex) == 0 );
	#endif
  }

  void resetEvent(void)
  {
	#if defined(WIN32) || defined(_XBOX)
    if ( mEvent )
    {
      ::ResetEvent(mEvent);
    }
	#endif
  }

  virtual void waitForSingleObject(NxU32 ms)
  {
	#if defined(WIN32) || defined(_XBOX)
    if ( mEvent )
    {
      ::WaitForSingleObject(mEvent,ms);
    }
	#elif defined(__APPLE__) || defined(__linux__)
      VERIFY( pthread_mutex_lock(&mEventMutex) == 0 );
	  if (ms == 0xffffffff)
	  {
		  VERIFY( pthread_cond_wait(&mEvent, &mEventMutex) == 0 );
	  }
	  else
	  {
	     struct timespec ts;
        #ifdef __APPLE__
        struct timeval tp;
        gettimeofday(&tp, (struct timezone *)0);
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec = tp.tv_sec;
        #else
	     clock_gettime(CLOCK_REALTIME, &ts);
        #endif
	     ts.tv_nsec += ms * 1000000;
	     ts.tv_sec += ts.tv_nsec / 1000000000;
	     ts.tv_nsec %= 1000000000;
		  NxI32 result = pthread_cond_timedwait(&mEvent, &mEventMutex, &ts);
		  assert(result == 0 || result == ETIMEDOUT);
	  }
	  VERIFY( pthread_mutex_unlock(&mEventMutex) == 0 );
	#endif
  }

private:
  #if defined(WIN32) || defined(_XBOX)
    HANDLE mEvent;
  #elif defined(__APPLE__) || defined(__linux__)
    pthread_mutex_t mEventMutex;
    pthread_cond_t mEvent;
  #endif
};

ThreadEvent * tc_createThreadEvent(void)
{
  MyThreadEvent *m = new MyThreadEvent;
  return static_cast<ThreadEvent *>(m);
}

void  tc_releaseThreadEvent(ThreadEvent *t)
{
  MyThreadEvent *m = static_cast< MyThreadEvent *>(t);
  delete m;
}

}; // end of namespace
