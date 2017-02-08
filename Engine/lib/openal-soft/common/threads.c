/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2007 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#include "config.h"

#include "threads.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "uintmap.h"


extern inline althrd_t althrd_current(void);
extern inline int althrd_equal(althrd_t thr0, althrd_t thr1);
extern inline void althrd_exit(int res);
extern inline void althrd_yield(void);

extern inline int almtx_lock(almtx_t *mtx);
extern inline int almtx_unlock(almtx_t *mtx);
extern inline int almtx_trylock(almtx_t *mtx);

extern inline void *altss_get(altss_t tss_id);
extern inline int altss_set(altss_t tss_id, void *val);


#ifndef UNUSED
#if defined(__cplusplus)
#define UNUSED(x)
#elif defined(__GNUC__)
#define UNUSED(x) UNUSED_##x __attribute__((unused))
#elif defined(__LCLINT__)
#define UNUSED(x) /*@unused@*/ x
#else
#define UNUSED(x) x
#endif
#endif


#define THREAD_STACK_SIZE (2*1024*1024) /* 2MB */

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>


void althrd_setname(althrd_t thr, const char *name)
{
#if defined(_MSC_VER)
#define MS_VC_EXCEPTION 0x406D1388
#pragma pack(push,8)
    struct {
        DWORD dwType;     // Must be 0x1000.
        LPCSTR szName;    // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags;    // Reserved for future use, must be zero.
    } info;
#pragma pack(pop)
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = thr;
    info.dwFlags = 0;

    __try {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except(EXCEPTION_CONTINUE_EXECUTION) {
    }
#undef MS_VC_EXCEPTION
#else
    (void)thr;
    (void)name;
#endif
}


static UIntMap ThrdIdHandle = UINTMAP_STATIC_INITIALIZE;

static void NTAPI althrd_callback(void* UNUSED(handle), DWORD reason, void* UNUSED(reserved))
{
    if(reason == DLL_PROCESS_DETACH)
        ResetUIntMap(&ThrdIdHandle);
}
#ifdef _MSC_VER
#pragma section(".CRT$XLC",read)
__declspec(allocate(".CRT$XLC")) PIMAGE_TLS_CALLBACK althrd_callback_ = althrd_callback;
#elif defined(__GNUC__)
PIMAGE_TLS_CALLBACK althrd_callback_ __attribute__((section(".CRT$XLC"))) = althrd_callback;
#else
PIMAGE_TLS_CALLBACK althrd_callback_ = althrd_callback;
#endif


typedef struct thread_cntr {
    althrd_start_t func;
    void *arg;
} thread_cntr;

static DWORD WINAPI althrd_starter(void *arg)
{
    thread_cntr cntr;
    memcpy(&cntr, arg, sizeof(cntr));
    free(arg);

    return (DWORD)((*cntr.func)(cntr.arg));
}


int althrd_create(althrd_t *thr, althrd_start_t func, void *arg)
{
    thread_cntr *cntr;
    DWORD thrid;
    HANDLE hdl;

    cntr = malloc(sizeof(*cntr));
    if(!cntr) return althrd_nomem;

    cntr->func = func;
    cntr->arg = arg;

    hdl = CreateThread(NULL, THREAD_STACK_SIZE, althrd_starter, cntr, 0, &thrid);
    if(!hdl)
    {
        free(cntr);
        return althrd_error;
    }
    InsertUIntMapEntry(&ThrdIdHandle, thrid, hdl);

    *thr = thrid;
    return althrd_success;
}

int althrd_detach(althrd_t thr)
{
    HANDLE hdl = RemoveUIntMapKey(&ThrdIdHandle, thr);
    if(!hdl) return althrd_error;

    CloseHandle(hdl);
    return althrd_success;
}

int althrd_join(althrd_t thr, int *res)
{
    DWORD code;

    HANDLE hdl = RemoveUIntMapKey(&ThrdIdHandle, thr);
    if(!hdl) return althrd_error;

    WaitForSingleObject(hdl, INFINITE);
    GetExitCodeThread(hdl, &code);
    CloseHandle(hdl);

    if(res != NULL)
        *res = (int)code;
    return althrd_success;
}

int althrd_sleep(const struct timespec *ts, struct timespec* UNUSED(rem))
{
    DWORD msec;

    if(ts->tv_sec < 0 || ts->tv_sec >= (0x7fffffff / 1000) ||
       ts->tv_nsec < 0 || ts->tv_nsec >= 1000000000)
        return -2;

    msec  = (DWORD)(ts->tv_sec * 1000);
    msec += (DWORD)((ts->tv_nsec+999999) / 1000000);
    Sleep(msec);

    return 0;
}


int almtx_init(almtx_t *mtx, int type)
{
    if(!mtx) return althrd_error;

    type &= ~almtx_recursive;
    if(type != almtx_plain)
        return althrd_error;

    InitializeCriticalSection(mtx);
    return althrd_success;
}

void almtx_destroy(almtx_t *mtx)
{
    DeleteCriticalSection(mtx);
}

int almtx_timedlock(almtx_t* UNUSED(mtx), const struct timespec* UNUSED(ts))
{
    /* Windows CRITICAL_SECTIONs don't seem to have a timedlock method. */
    return althrd_error;
}

#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600
int alcnd_init(alcnd_t *cond)
{
    InitializeConditionVariable(cond);
    return althrd_success;
}

int alcnd_signal(alcnd_t *cond)
{
    WakeConditionVariable(cond);
    return althrd_success;
}

int alcnd_broadcast(alcnd_t *cond)
{
    WakeAllConditionVariable(cond);
    return althrd_success;
}

int alcnd_wait(alcnd_t *cond, almtx_t *mtx)
{
    if(SleepConditionVariableCS(cond, mtx, INFINITE) != 0)
        return althrd_success;
    return althrd_error;
}

int alcnd_timedwait(alcnd_t *cond, almtx_t *mtx, const struct timespec *time_point)
{
    struct timespec curtime;
    DWORD sleeptime;

    if(altimespec_get(&curtime, AL_TIME_UTC) != AL_TIME_UTC)
        return althrd_error;

    if(curtime.tv_sec > time_point->tv_sec || (curtime.tv_sec == time_point->tv_sec &&
                                               curtime.tv_nsec >= time_point->tv_nsec))
    {
        if(SleepConditionVariableCS(cond, mtx, 0) != 0)
            return althrd_success;
    }
    else
    {
        sleeptime  = (time_point->tv_nsec - curtime.tv_nsec + 999999)/1000000;
        sleeptime += (time_point->tv_sec - curtime.tv_sec)*1000;
        if(SleepConditionVariableCS(cond, mtx, sleeptime) != 0)
            return althrd_success;
    }
    return (GetLastError()==ERROR_TIMEOUT) ? althrd_timedout : althrd_error;
}

void alcnd_destroy(alcnd_t* UNUSED(cond))
{
    /* Nothing to delete? */
}

#else

/* WARNING: This is a rather poor implementation of condition variables, with
 * known problems. However, it's simple, efficient, and good enough for now to
 * not require Vista. Based on "Strategies for Implementing POSIX Condition
 * Variables" by Douglas C. Schmidt and Irfan Pyarali:
 * http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
 */
/* A better solution may be using Wine's implementation. It requires internals
 * (NtCreateKeyedEvent, NtReleaseKeyedEvent, and NtWaitForKeyedEvent) from
 * ntdll, and implemention of exchange and compare-exchange for RefCounts.
 */

typedef struct {
    RefCount wait_count;

    HANDLE events[2];
} _int_alcnd_t;
enum {
    SIGNAL = 0,
    BROADCAST = 1
};

int alcnd_init(alcnd_t *cond)
{
    _int_alcnd_t *icond = calloc(1, sizeof(*icond));
    if(!icond) return althrd_nomem;

    InitRef(&icond->wait_count, 0);

    icond->events[SIGNAL] = CreateEventW(NULL, FALSE, FALSE, NULL);
    icond->events[BROADCAST] = CreateEventW(NULL, TRUE, FALSE, NULL);
    if(!icond->events[SIGNAL] || !icond->events[BROADCAST])
    {
        if(icond->events[SIGNAL])
            CloseHandle(icond->events[SIGNAL]);
        if(icond->events[BROADCAST])
            CloseHandle(icond->events[BROADCAST]);
        free(icond);
        return althrd_error;
    }

    cond->Ptr = icond;
    return althrd_success;
}

int alcnd_signal(alcnd_t *cond)
{
    _int_alcnd_t *icond = cond->Ptr;
    if(ReadRef(&icond->wait_count) > 0)
        SetEvent(icond->events[SIGNAL]);
    return althrd_success;
}

int alcnd_broadcast(alcnd_t *cond)
{
    _int_alcnd_t *icond = cond->Ptr;
    if(ReadRef(&icond->wait_count) > 0)
        SetEvent(icond->events[BROADCAST]);
    return althrd_success;
}

int alcnd_wait(alcnd_t *cond, almtx_t *mtx)
{
    _int_alcnd_t *icond = cond->Ptr;
    int res;

    IncrementRef(&icond->wait_count);
    LeaveCriticalSection(mtx);

    res = WaitForMultipleObjects(2, icond->events, FALSE, INFINITE);

    if(DecrementRef(&icond->wait_count) == 0 && res == WAIT_OBJECT_0+BROADCAST)
        ResetEvent(icond->events[BROADCAST]);
    EnterCriticalSection(mtx);

    return althrd_success;
}

int alcnd_timedwait(alcnd_t *cond, almtx_t *mtx, const struct timespec *time_point)
{
    _int_alcnd_t *icond = cond->Ptr;
    struct timespec curtime;
    DWORD sleeptime;
    int res;

    if(altimespec_get(&curtime, AL_TIME_UTC) != AL_TIME_UTC)
        return althrd_error;

    if(curtime.tv_sec > time_point->tv_sec || (curtime.tv_sec == time_point->tv_sec &&
                                               curtime.tv_nsec >= time_point->tv_nsec))
        sleeptime = 0;
    else
    {
        sleeptime  = (time_point->tv_nsec - curtime.tv_nsec + 999999)/1000000;
        sleeptime += (time_point->tv_sec - curtime.tv_sec)*1000;
    }

    IncrementRef(&icond->wait_count);
    LeaveCriticalSection(mtx);

    res = WaitForMultipleObjects(2, icond->events, FALSE, sleeptime);

    if(DecrementRef(&icond->wait_count) == 0 && res == WAIT_OBJECT_0+BROADCAST)
        ResetEvent(icond->events[BROADCAST]);
    EnterCriticalSection(mtx);

    return (res == WAIT_TIMEOUT) ? althrd_timedout : althrd_success;
}

void alcnd_destroy(alcnd_t *cond)
{
    _int_alcnd_t *icond = cond->Ptr;
    CloseHandle(icond->events[SIGNAL]);
    CloseHandle(icond->events[BROADCAST]);
    free(icond);
}
#endif /* defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600 */


/* An associative map of uint:void* pairs. The key is the TLS index (given by
 * TlsAlloc), and the value is the altss_dtor_t callback. When a thread exits,
 * we iterate over the TLS indices for their thread-local value and call the
 * destructor function with it if they're both not NULL. To avoid using
 * DllMain, a PIMAGE_TLS_CALLBACK function pointer is placed in a ".CRT$XLx"
 * section (where x is a character A to Z) which will be called by the CRT.
 */
static UIntMap TlsDestructors = UINTMAP_STATIC_INITIALIZE;

static void NTAPI altss_callback(void* UNUSED(handle), DWORD reason, void* UNUSED(reserved))
{
    ALsizei i;

    if(reason == DLL_PROCESS_DETACH)
    {
        ResetUIntMap(&TlsDestructors);
        return;
    }
    if(reason != DLL_THREAD_DETACH)
        return;

    LockUIntMapRead(&TlsDestructors);
    for(i = 0;i < TlsDestructors.size;i++)
    {
        void *ptr = altss_get(TlsDestructors.keys[i]);
        altss_dtor_t callback = (altss_dtor_t)TlsDestructors.values[i];
        if(ptr && callback)
            callback(ptr);
    }
    UnlockUIntMapRead(&TlsDestructors);
}
#ifdef _MSC_VER
#pragma section(".CRT$XLB",read)
__declspec(allocate(".CRT$XLB")) PIMAGE_TLS_CALLBACK altss_callback_ = altss_callback;
#elif defined(__GNUC__)
PIMAGE_TLS_CALLBACK altss_callback_ __attribute__((section(".CRT$XLB"))) = altss_callback;
#else
#warning "No TLS callback support, thread-local contexts may leak references on poorly written applications."
PIMAGE_TLS_CALLBACK altss_callback_ = altss_callback;
#endif

int altss_create(altss_t *tss_id, altss_dtor_t callback)
{
    DWORD key = TlsAlloc();
    if(key == TLS_OUT_OF_INDEXES)
        return althrd_error;

    *tss_id = key;
    if(callback != NULL)
        InsertUIntMapEntry(&TlsDestructors, key, callback);
    return althrd_success;
}

void altss_delete(altss_t tss_id)
{
    RemoveUIntMapKey(&TlsDestructors, tss_id);
    TlsFree(tss_id);
}


int altimespec_get(struct timespec *ts, int base)
{
    static_assert(sizeof(FILETIME) == sizeof(ULARGE_INTEGER),
                  "Size of FILETIME does not match ULARGE_INTEGER");
    if(base == AL_TIME_UTC)
    {
        union {
            FILETIME ftime;
            ULARGE_INTEGER ulint;
        } systime;
        GetSystemTimeAsFileTime(&systime.ftime);
        /* FILETIME is in 100-nanosecond units, or 1/10th of a microsecond. */
        ts->tv_sec = systime.ulint.QuadPart/10000000;
        ts->tv_nsec = (systime.ulint.QuadPart%10000000) * 100;
        return base;
    }

    return 0;
}


void alcall_once(alonce_flag *once, void (*callback)(void))
{
    LONG ret;
    while((ret=InterlockedExchange(once, 1)) == 1)
        althrd_yield();
    if(ret == 0)
        (*callback)();
    InterlockedExchange(once, 2);
}

#else

#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#ifdef HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif


extern inline int althrd_sleep(const struct timespec *ts, struct timespec *rem);
extern inline void alcall_once(alonce_flag *once, void (*callback)(void));


void althrd_setname(althrd_t thr, const char *name)
{
#if defined(HAVE_PTHREAD_SETNAME_NP)
#if defined(PTHREAD_SETNAME_NP_ONE_PARAM)
    if(althrd_equal(thr, althrd_current()))
        pthread_setname_np(name);
#else
    pthread_setname_np(thr, name);
#endif
#elif defined(HAVE_PTHREAD_SET_NAME_NP)
    pthread_set_name_np(thr, name);
#else
    (void)thr;
    (void)name;
#endif
}


typedef struct thread_cntr {
    althrd_start_t func;
    void *arg;
} thread_cntr;

static void *althrd_starter(void *arg)
{
    thread_cntr cntr;
    memcpy(&cntr, arg, sizeof(cntr));
    free(arg);

    return (void*)(intptr_t)((*cntr.func)(cntr.arg));
}


int althrd_create(althrd_t *thr, althrd_start_t func, void *arg)
{
    thread_cntr *cntr;
    pthread_attr_t attr;
    size_t stackmult = 1;
    int err;

    cntr = malloc(sizeof(*cntr));
    if(!cntr) return althrd_nomem;

    if(pthread_attr_init(&attr) != 0)
    {
        free(cntr);
        return althrd_error;
    }
retry_stacksize:
    if(pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE*stackmult) != 0)
    {
        pthread_attr_destroy(&attr);
        free(cntr);
        return althrd_error;
    }

    cntr->func = func;
    cntr->arg = arg;
    if((err=pthread_create(thr, &attr, althrd_starter, cntr)) == 0)
    {
        pthread_attr_destroy(&attr);
        return althrd_success;
    }

    if(err == EINVAL)
    {
        /* If an invalid stack size, try increasing it (limit x4, 8MB). */
        if(stackmult < 4)
        {
            stackmult *= 2;
            goto retry_stacksize;
        }
        /* If still nothing, try defaults and hope they're good enough. */
        if(pthread_create(thr, NULL, althrd_starter, cntr) == 0)
        {
            pthread_attr_destroy(&attr);
            return althrd_success;
        }
    }
    pthread_attr_destroy(&attr);
    free(cntr);
    return althrd_error;
}

int althrd_detach(althrd_t thr)
{
    if(pthread_detach(thr) != 0)
        return althrd_error;
    return althrd_success;
}

int althrd_join(althrd_t thr, int *res)
{
    void *code;

    if(pthread_join(thr, &code) != 0)
        return althrd_error;
    if(res != NULL)
        *res = (int)(intptr_t)code;
    return althrd_success;
}


int almtx_init(almtx_t *mtx, int type)
{
    int ret;

    if(!mtx) return althrd_error;
#ifdef HAVE_PTHREAD_MUTEX_TIMEDLOCK
    if((type&~(almtx_recursive|almtx_timed)) != 0)
        return althrd_error;
#else
    if((type&~almtx_recursive) != 0)
        return althrd_error;
#endif

    type &= ~almtx_timed;
    if(type == almtx_plain)
        ret = pthread_mutex_init(mtx, NULL);
    else
    {
        pthread_mutexattr_t attr;

        ret = pthread_mutexattr_init(&attr);
        if(ret) return althrd_error;

        if(type == almtx_recursive)
        {
            ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#ifdef HAVE_PTHREAD_MUTEXATTR_SETKIND_NP
            if(ret != 0)
                ret = pthread_mutexattr_setkind_np(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif
        }
        else
            ret = 1;
        if(ret == 0)
            ret = pthread_mutex_init(mtx, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    return ret ? althrd_error : althrd_success;
}

void almtx_destroy(almtx_t *mtx)
{
    pthread_mutex_destroy(mtx);
}

int almtx_timedlock(almtx_t *mtx, const struct timespec *ts)
{
#ifdef HAVE_PTHREAD_MUTEX_TIMEDLOCK
    int ret = pthread_mutex_timedlock(mtx, ts);
    switch(ret)
    {
        case 0: return althrd_success;
        case ETIMEDOUT: return althrd_timedout;
        case EBUSY: return althrd_busy;
    }
#endif
    return althrd_error;
}

int alcnd_init(alcnd_t *cond)
{
    if(pthread_cond_init(cond, NULL) == 0)
        return althrd_success;
    return althrd_error;
}

int alcnd_signal(alcnd_t *cond)
{
    if(pthread_cond_signal(cond) == 0)
        return althrd_success;
    return althrd_error;
}

int alcnd_broadcast(alcnd_t *cond)
{
    if(pthread_cond_broadcast(cond) == 0)
        return althrd_success;
    return althrd_error;
}

int alcnd_wait(alcnd_t *cond, almtx_t *mtx)
{
    if(pthread_cond_wait(cond, mtx) == 0)
        return althrd_success;
    return althrd_error;
}

int alcnd_timedwait(alcnd_t *cond, almtx_t *mtx, const struct timespec *time_point)
{
    if(pthread_cond_timedwait(cond, mtx, time_point) == 0)
        return althrd_success;
    return althrd_error;
}

void alcnd_destroy(alcnd_t *cond)
{
    pthread_cond_destroy(cond);
}


int altss_create(altss_t *tss_id, altss_dtor_t callback)
{
    if(pthread_key_create(tss_id, callback) != 0)
        return althrd_error;
    return althrd_success;
}

void altss_delete(altss_t tss_id)
{
    pthread_key_delete(tss_id);
}


int altimespec_get(struct timespec *ts, int base)
{
    if(base == AL_TIME_UTC)
    {
        int ret;
#if _POSIX_TIMERS > 0
        ret = clock_gettime(CLOCK_REALTIME, ts);
        if(ret == 0) return base;
#else /* _POSIX_TIMERS > 0 */
        struct timeval tv;
        ret = gettimeofday(&tv, NULL);
        if(ret == 0)
        {
            ts->tv_sec = tv.tv_sec;
            ts->tv_nsec = tv.tv_usec * 1000;
            return base;
        }
#endif
    }

    return 0;
}

#endif


void al_nssleep(unsigned long nsec)
{
    struct timespec ts, rem;
    ts.tv_sec = nsec / 1000000000ul;
    ts.tv_nsec = nsec % 1000000000ul;

    while(althrd_sleep(&ts, &rem) == -1)
        ts = rem;
}
