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


/* An associative map of uint:void* pairs. The key is the unique Thread ID and
 * the value is the thread HANDLE. The thread ID is passed around as the
 * althrd_t since there is only one ID per thread, whereas a thread may be
 * referenced by multiple different HANDLEs. This map allows retrieving the
 * original handle which is needed to join the thread and get its return value.
 */
static UIntMap ThrdIdHandle = UINTMAP_STATIC_INITIALIZE;

/* An associative map of uint:void* pairs. The key is the TLS index (given by
 * TlsAlloc), and the value is the altss_dtor_t callback. When a thread exits,
 * we iterate over the TLS indices for their thread-local value and call the
 * destructor function with it if they're both not NULL.
 */
static UIntMap TlsDestructors = UINTMAP_STATIC_INITIALIZE;


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

void alcnd_destroy(alcnd_t *cond)
{
    _int_alcnd_t *icond = cond->Ptr;
    CloseHandle(icond->events[SIGNAL]);
    CloseHandle(icond->events[BROADCAST]);
    free(icond);
}
#endif /* defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600 */


int alsem_init(alsem_t *sem, unsigned int initial)
{
    *sem = CreateSemaphore(NULL, initial, INT_MAX, NULL);
    if(*sem != NULL) return althrd_success;
    return althrd_error;
}

void alsem_destroy(alsem_t *sem)
{
    CloseHandle(*sem);
}

int alsem_post(alsem_t *sem)
{
    DWORD ret = ReleaseSemaphore(*sem, 1, NULL);
    if(ret) return althrd_success;
    return althrd_error;
}

int alsem_wait(alsem_t *sem)
{
    DWORD ret = WaitForSingleObject(*sem, INFINITE);
    if(ret == WAIT_OBJECT_0) return althrd_success;
    return althrd_error;
}

int alsem_trywait(alsem_t *sem)
{
    DWORD ret = WaitForSingleObject(*sem, 0);
    if(ret == WAIT_OBJECT_0) return althrd_success;
    if(ret == WAIT_TIMEOUT) return althrd_busy;
    return althrd_error;
}


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


void althrd_deinit(void)
{
    ResetUIntMap(&ThrdIdHandle);
    ResetUIntMap(&TlsDestructors);
}

void althrd_thread_detach(void)
{
    ALsizei i;

    LockUIntMapRead(&TlsDestructors);
    for(i = 0;i < TlsDestructors.size;i++)
    {
        void *ptr = altss_get(TlsDestructors.keys[i]);
        altss_dtor_t callback = (altss_dtor_t)TlsDestructors.values[i];
        if(ptr && callback) callback(ptr);
    }
    UnlockUIntMapRead(&TlsDestructors);
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

extern inline void althrd_deinit(void);
extern inline void althrd_thread_detach(void);

void althrd_setname(althrd_t thr, const char *name)
{
#if defined(HAVE_PTHREAD_SETNAME_NP)
#if defined(PTHREAD_SETNAME_NP_ONE_PARAM)
    if(althrd_equal(thr, althrd_current()))
        pthread_setname_np(name);
#elif defined(PTHREAD_SETNAME_NP_THREE_PARAMS)
    pthread_setname_np(thr, "%s", (void*)name);
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
    if((type&~almtx_recursive) != 0)
        return althrd_error;

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

void alcnd_destroy(alcnd_t *cond)
{
    pthread_cond_destroy(cond);
}


int alsem_init(alsem_t *sem, unsigned int initial)
{
    if(sem_init(sem, 0, initial) == 0)
        return althrd_success;
    return althrd_error;
}

void alsem_destroy(alsem_t *sem)
{
    sem_destroy(sem);
}

int alsem_post(alsem_t *sem)
{
    if(sem_post(sem) == 0)
        return althrd_success;
    return althrd_error;
}

int alsem_wait(alsem_t *sem)
{
    if(sem_wait(sem) == 0) return althrd_success;
    if(errno == EINTR) return -2;
    return althrd_error;
}

int alsem_trywait(alsem_t *sem)
{
    if(sem_trywait(sem) == 0) return althrd_success;
    if(errno == EWOULDBLOCK) return althrd_busy;
    if(errno == EINTR) return -2;
    return althrd_error;
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
