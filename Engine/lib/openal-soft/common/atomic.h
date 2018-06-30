#ifndef AL_ATOMIC_H
#define AL_ATOMIC_H

#include "static_assert.h"
#include "bool.h"

#ifdef __GNUC__
/* This helps cast away the const-ness of a pointer without accidentally
 * changing the pointer type. This is necessary due to Clang's inability to use
 * atomic_load on a const _Atomic variable.
 */
#define CONST_CAST(T, V) __extension__({                                      \
    const T _tmp = (V);                                                       \
    (T)_tmp;                                                                  \
})
#else
#define CONST_CAST(T, V) ((T)(V))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Atomics using C11 */
#ifdef HAVE_C11_ATOMIC

#include <stdatomic.h>

#define almemory_order memory_order
#define almemory_order_relaxed memory_order_relaxed
#define almemory_order_consume memory_order_consume
#define almemory_order_acquire memory_order_acquire
#define almemory_order_release memory_order_release
#define almemory_order_acq_rel memory_order_acq_rel
#define almemory_order_seq_cst memory_order_seq_cst

#define ATOMIC(T)  T _Atomic
#define ATOMIC_FLAG atomic_flag

#define ATOMIC_INIT atomic_init
#define ATOMIC_INIT_STATIC ATOMIC_VAR_INIT
/*#define ATOMIC_FLAG_INIT ATOMIC_FLAG_INIT*/

#define ATOMIC_LOAD atomic_load_explicit
#define ATOMIC_STORE atomic_store_explicit

#define ATOMIC_ADD atomic_fetch_add_explicit
#define ATOMIC_SUB atomic_fetch_sub_explicit

#define ATOMIC_EXCHANGE atomic_exchange_explicit
#define ATOMIC_COMPARE_EXCHANGE_STRONG atomic_compare_exchange_strong_explicit
#define ATOMIC_COMPARE_EXCHANGE_WEAK atomic_compare_exchange_weak_explicit

#define ATOMIC_FLAG_TEST_AND_SET atomic_flag_test_and_set_explicit
#define ATOMIC_FLAG_CLEAR atomic_flag_clear_explicit

#define ATOMIC_THREAD_FENCE atomic_thread_fence

/* Atomics using GCC intrinsics */
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) && !defined(__QNXNTO__)

enum almemory_order {
    almemory_order_relaxed,
    almemory_order_consume,
    almemory_order_acquire,
    almemory_order_release,
    almemory_order_acq_rel,
    almemory_order_seq_cst
};

#define ATOMIC(T)  struct { T volatile value; }
#define ATOMIC_FLAG  ATOMIC(int)

#define ATOMIC_INIT(_val, _newval)  do { (_val)->value = (_newval); } while(0)
#define ATOMIC_INIT_STATIC(_newval) {(_newval)}
#define ATOMIC_FLAG_INIT            ATOMIC_INIT_STATIC(0)

#define ATOMIC_LOAD(_val, _MO)  __extension__({ \
    __typeof((_val)->value) _r = (_val)->value; \
    __asm__ __volatile__("" ::: "memory");      \
    _r;                                         \
})
#define ATOMIC_STORE(_val, _newval, _MO)  do { \
    __asm__ __volatile__("" ::: "memory");     \
    (_val)->value = (_newval);                 \
} while(0)

#define ATOMIC_ADD(_val, _incr, _MO) __sync_fetch_and_add(&(_val)->value, (_incr))
#define ATOMIC_SUB(_val, _decr, _MO) __sync_fetch_and_sub(&(_val)->value, (_decr))

#define ATOMIC_EXCHANGE(_val, _newval, _MO)  __extension__({                  \
    __asm__ __volatile__("" ::: "memory");                                    \
    __sync_lock_test_and_set(&(_val)->value, (_newval));                      \
})
#define ATOMIC_COMPARE_EXCHANGE_STRONG(_val, _oldval, _newval, _MO1, _MO2) __extension__({ \
    __typeof(*(_oldval)) _o = *(_oldval);                                     \
    *(_oldval) = __sync_val_compare_and_swap(&(_val)->value, _o, (_newval));  \
    *(_oldval) == _o;                                                         \
})

#define ATOMIC_FLAG_TEST_AND_SET(_val, _MO)  __extension__({                  \
    __asm__ __volatile__("" ::: "memory");                                    \
    __sync_lock_test_and_set(&(_val)->value, 1);                              \
})
#define ATOMIC_FLAG_CLEAR(_val, _MO)  __extension__({                         \
    __sync_lock_release(&(_val)->value);                                      \
    __asm__ __volatile__("" ::: "memory");                                    \
})


#define ATOMIC_THREAD_FENCE(order) do {        \
    enum { must_be_constant = (order) };       \
    const int _o = must_be_constant;           \
    if(_o > almemory_order_relaxed)            \
        __asm__ __volatile__("" ::: "memory"); \
} while(0)

/* Atomics using x86/x86-64 GCC inline assembly */
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))

#define WRAP_ADD(S, ret, dest, incr) __asm__ __volatile__(                    \
    "lock; xadd"S" %0,(%1)"                                                   \
    : "=r" (ret)                                                              \
    : "r" (dest), "0" (incr)                                                  \
    : "memory"                                                                \
)
#define WRAP_SUB(S, ret, dest, decr) __asm__ __volatile__(                    \
    "lock; xadd"S" %0,(%1)"                                                   \
    : "=r" (ret)                                                              \
    : "r" (dest), "0" (-(decr))                                               \
    : "memory"                                                                \
)

#define WRAP_XCHG(S, ret, dest, newval) __asm__ __volatile__(                 \
    "lock; xchg"S" %0,(%1)"                                                   \
    : "=r" (ret)                                                              \
    : "r" (dest), "0" (newval)                                                \
    : "memory"                                                                \
)
#define WRAP_CMPXCHG(S, ret, dest, oldval, newval) __asm__ __volatile__(      \
    "lock; cmpxchg"S" %2,(%1)"                                                \
    : "=a" (ret)                                                              \
    : "r" (dest), "r" (newval), "0" (oldval)                                  \
    : "memory"                                                                \
)


enum almemory_order {
    almemory_order_relaxed,
    almemory_order_consume,
    almemory_order_acquire,
    almemory_order_release,
    almemory_order_acq_rel,
    almemory_order_seq_cst
};

#define ATOMIC(T)  struct { T volatile value; }

#define ATOMIC_INIT(_val, _newval)  do { (_val)->value = (_newval); } while(0)
#define ATOMIC_INIT_STATIC(_newval) {(_newval)}

#define ATOMIC_LOAD(_val, _MO)  __extension__({ \
    __typeof((_val)->value) _r = (_val)->value; \
    __asm__ __volatile__("" ::: "memory");      \
    _r;                                         \
})
#define ATOMIC_STORE(_val, _newval, _MO)  do { \
    __asm__ __volatile__("" ::: "memory");     \
    (_val)->value = (_newval);                 \
} while(0)

#define ATOMIC_ADD(_val, _incr, _MO) __extension__({                          \
    static_assert(sizeof((_val)->value)==4 || sizeof((_val)->value)==8, "Unsupported size!"); \
    __typeof((_val)->value) _r;                                               \
    if(sizeof((_val)->value) == 4) WRAP_ADD("l", _r, &(_val)->value, _incr);  \
    else if(sizeof((_val)->value) == 8) WRAP_ADD("q", _r, &(_val)->value, _incr); \
    _r;                                                                       \
})
#define ATOMIC_SUB(_val, _decr, _MO) __extension__({                          \
    static_assert(sizeof((_val)->value)==4 || sizeof((_val)->value)==8, "Unsupported size!"); \
    __typeof((_val)->value) _r;                                               \
    if(sizeof((_val)->value) == 4) WRAP_SUB("l", _r, &(_val)->value, _decr);  \
    else if(sizeof((_val)->value) == 8) WRAP_SUB("q", _r, &(_val)->value, _decr); \
    _r;                                                                       \
})

#define ATOMIC_EXCHANGE(_val, _newval, _MO)  __extension__({                  \
    __typeof((_val)->value) _r;                                               \
    if(sizeof((_val)->value) == 4) WRAP_XCHG("l", _r, &(_val)->value, (_newval)); \
    else if(sizeof((_val)->value) == 8) WRAP_XCHG("q", _r, &(_val)->value, (_newval)); \
    _r;                                                                       \
})
#define ATOMIC_COMPARE_EXCHANGE_STRONG(_val, _oldval, _newval, _MO1, _MO2) __extension__({ \
    __typeof(*(_oldval)) _old = *(_oldval);                                   \
    if(sizeof((_val)->value) == 4) WRAP_CMPXCHG("l", *(_oldval), &(_val)->value, _old, (_newval)); \
    else if(sizeof((_val)->value) == 8) WRAP_CMPXCHG("q", *(_oldval), &(_val)->value, _old, (_newval)); \
    *(_oldval) == _old;                                                       \
})

#define ATOMIC_EXCHANGE_PTR(_val, _newval, _MO)  __extension__({              \
    void *_r;                                                                 \
    if(sizeof(void*) == 4) WRAP_XCHG("l", _r, &(_val)->value, (_newval));     \
    else if(sizeof(void*) == 8) WRAP_XCHG("q", _r, &(_val)->value, (_newval));\
    _r;                                                                       \
})
#define ATOMIC_COMPARE_EXCHANGE_PTR_STRONG(_val, _oldval, _newval, _MO1, _MO2) __extension__({ \
    void *_old = *(_oldval);                                                  \
    if(sizeof(void*) == 4) WRAP_CMPXCHG("l", *(_oldval), &(_val)->value, _old, (_newval)); \
    else if(sizeof(void*) == 8) WRAP_CMPXCHG("q", *(_oldval), &(_val)->value, _old, (_newval)); \
    *(_oldval) == _old;                                                       \
})

#define ATOMIC_THREAD_FENCE(order) do {        \
    enum { must_be_constant = (order) };       \
    const int _o = must_be_constant;           \
    if(_o > almemory_order_relaxed)            \
        __asm__ __volatile__("" ::: "memory"); \
} while(0)

/* Atomics using Windows methods */
#elif defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* NOTE: This mess is *extremely* touchy. It lacks quite a bit of safety
 * checking due to the lack of multi-statement expressions, typeof(), and C99
 * compound literals. It is incapable of properly exchanging floats, which get
 * casted to LONG/int, and could cast away potential warnings.
 *
 * Unfortunately, it's the only semi-safe way that doesn't rely on C99 (because
 * MSVC).
 */

inline LONG AtomicAdd32(volatile LONG *dest, LONG incr)
{
    return InterlockedExchangeAdd(dest, incr);
}
inline LONGLONG AtomicAdd64(volatile LONGLONG *dest, LONGLONG incr)
{
    return InterlockedExchangeAdd64(dest, incr);
}
inline LONG AtomicSub32(volatile LONG *dest, LONG decr)
{
    return InterlockedExchangeAdd(dest, -decr);
}
inline LONGLONG AtomicSub64(volatile LONGLONG *dest, LONGLONG decr)
{
    return InterlockedExchangeAdd64(dest, -decr);
}

inline LONG AtomicSwap32(volatile LONG *dest, LONG newval)
{
    return InterlockedExchange(dest, newval);
}
inline LONGLONG AtomicSwap64(volatile LONGLONG *dest, LONGLONG newval)
{
    return InterlockedExchange64(dest, newval);
}
inline void *AtomicSwapPtr(void *volatile *dest, void *newval)
{
    return InterlockedExchangePointer(dest, newval);
}

inline bool CompareAndSwap32(volatile LONG *dest, LONG newval, LONG *oldval)
{
    LONG old = *oldval;
    *oldval = InterlockedCompareExchange(dest, newval, *oldval);
    return old == *oldval;
}
inline bool CompareAndSwap64(volatile LONGLONG *dest, LONGLONG newval, LONGLONG *oldval)
{
    LONGLONG old = *oldval;
    *oldval = InterlockedCompareExchange64(dest, newval, *oldval);
    return old == *oldval;
}
inline bool CompareAndSwapPtr(void *volatile *dest, void *newval, void **oldval)
{
    void *old = *oldval;
    *oldval = InterlockedCompareExchangePointer(dest, newval, *oldval);
    return old == *oldval;
}

#define WRAP_ADDSUB(T, _func, _ptr, _amnt)  _func((T volatile*)(_ptr), (_amnt))
#define WRAP_XCHG(T, _func, _ptr, _newval)  _func((T volatile*)(_ptr), (_newval))
#define WRAP_CMPXCHG(T, _func, _ptr, _newval, _oldval)  _func((T volatile*)(_ptr), (_newval), (T*)(_oldval))


enum almemory_order {
    almemory_order_relaxed,
    almemory_order_consume,
    almemory_order_acquire,
    almemory_order_release,
    almemory_order_acq_rel,
    almemory_order_seq_cst
};

#define ATOMIC(T)  struct { T volatile value; }

#define ATOMIC_INIT(_val, _newval)  do { (_val)->value = (_newval); } while(0)
#define ATOMIC_INIT_STATIC(_newval) {(_newval)}

#define ATOMIC_LOAD(_val, _MO)  ((_val)->value)
#define ATOMIC_STORE(_val, _newval, _MO)  do {  \
    (_val)->value = (_newval);                  \
} while(0)

int _al_invalid_atomic_size(); /* not defined */
void *_al_invalid_atomic_ptr_size(); /* not defined */

#define ATOMIC_ADD(_val, _incr, _MO)                                          \
    ((sizeof((_val)->value)==4) ? WRAP_ADDSUB(LONG, AtomicAdd32, &(_val)->value, (_incr)) : \
     (sizeof((_val)->value)==8) ? WRAP_ADDSUB(LONGLONG, AtomicAdd64, &(_val)->value, (_incr)) : \
     _al_invalid_atomic_size())
#define ATOMIC_SUB(_val, _decr, _MO)                                          \
    ((sizeof((_val)->value)==4) ? WRAP_ADDSUB(LONG, AtomicSub32, &(_val)->value, (_decr)) : \
     (sizeof((_val)->value)==8) ? WRAP_ADDSUB(LONGLONG, AtomicSub64, &(_val)->value, (_decr)) : \
     _al_invalid_atomic_size())

#define ATOMIC_EXCHANGE(_val, _newval, _MO)                                   \
    ((sizeof((_val)->value)==4) ? WRAP_XCHG(LONG, AtomicSwap32, &(_val)->value, (_newval)) : \
     (sizeof((_val)->value)==8) ? WRAP_XCHG(LONGLONG, AtomicSwap64, &(_val)->value, (_newval)) :   \
     (LONG)_al_invalid_atomic_size())
#define ATOMIC_COMPARE_EXCHANGE_STRONG(_val, _oldval, _newval, _MO1, _MO2)    \
    ((sizeof((_val)->value)==4) ? WRAP_CMPXCHG(LONG, CompareAndSwap32, &(_val)->value, (_newval), (_oldval)) : \
     (sizeof((_val)->value)==8) ? WRAP_CMPXCHG(LONGLONG, CompareAndSwap64, &(_val)->value, (_newval), (_oldval)) : \
     (bool)_al_invalid_atomic_size())

#define ATOMIC_EXCHANGE_PTR(_val, _newval, _MO) \
    ((sizeof((_val)->value)==sizeof(void*)) ? AtomicSwapPtr((void*volatile*)&(_val)->value, (_newval)) : \
     _al_invalid_atomic_ptr_size())
#define ATOMIC_COMPARE_EXCHANGE_PTR_STRONG(_val, _oldval, _newval, _MO1, _MO2)\
    ((sizeof((_val)->value)==sizeof(void*)) ? CompareAndSwapPtr((void*volatile*)&(_val)->value, (_newval), (void**)(_oldval)) : \
     (bool)_al_invalid_atomic_size())

#define ATOMIC_THREAD_FENCE(order) do {        \
    enum { must_be_constant = (order) };       \
    const int _o = must_be_constant;           \
    if(_o > almemory_order_relaxed)            \
        _ReadWriteBarrier();                   \
} while(0)

#else

#error "No atomic functions available on this platform!"

#define ATOMIC(T)  T

#define ATOMIC_INIT(_val, _newval)  ((void)0)
#define ATOMIC_INIT_STATIC(_newval) (0)

#define ATOMIC_LOAD(...)   (0)
#define ATOMIC_STORE(...)  ((void)0)

#define ATOMIC_ADD(...) (0)
#define ATOMIC_SUB(...) (0)

#define ATOMIC_EXCHANGE(...) (0)
#define ATOMIC_COMPARE_EXCHANGE_STRONG(...) (0)

#define ATOMIC_THREAD_FENCE(...) ((void)0)
#endif

/* If no PTR xchg variants are provided, the normal ones can handle it. */
#ifndef ATOMIC_EXCHANGE_PTR
#define ATOMIC_EXCHANGE_PTR ATOMIC_EXCHANGE
#define ATOMIC_COMPARE_EXCHANGE_PTR_STRONG ATOMIC_COMPARE_EXCHANGE_STRONG
#define ATOMIC_COMPARE_EXCHANGE_PTR_WEAK ATOMIC_COMPARE_EXCHANGE_WEAK
#endif

/* If no weak cmpxchg is provided (not all systems will have one), substitute a
 * strong cmpxchg. */
#ifndef ATOMIC_COMPARE_EXCHANGE_WEAK
#define ATOMIC_COMPARE_EXCHANGE_WEAK ATOMIC_COMPARE_EXCHANGE_STRONG
#endif
#ifndef ATOMIC_COMPARE_EXCHANGE_PTR_WEAK
#define ATOMIC_COMPARE_EXCHANGE_PTR_WEAK ATOMIC_COMPARE_EXCHANGE_PTR_STRONG
#endif

/* If no ATOMIC_FLAG is defined, simulate one with an atomic int using exchange
 * and store ops.
 */
#ifndef ATOMIC_FLAG
#define ATOMIC_FLAG      ATOMIC(int)
#define ATOMIC_FLAG_INIT ATOMIC_INIT_STATIC(0)
#define ATOMIC_FLAG_TEST_AND_SET(_val, _MO) ATOMIC_EXCHANGE(_val, 1, _MO)
#define ATOMIC_FLAG_CLEAR(_val, _MO)        ATOMIC_STORE(_val, 0, _MO)
#endif


#define ATOMIC_LOAD_SEQ(_val) ATOMIC_LOAD(_val, almemory_order_seq_cst)
#define ATOMIC_STORE_SEQ(_val, _newval) ATOMIC_STORE(_val, _newval, almemory_order_seq_cst)

#define ATOMIC_ADD_SEQ(_val, _incr) ATOMIC_ADD(_val, _incr, almemory_order_seq_cst)
#define ATOMIC_SUB_SEQ(_val, _decr) ATOMIC_SUB(_val, _decr, almemory_order_seq_cst)

#define ATOMIC_EXCHANGE_SEQ(_val, _newval) ATOMIC_EXCHANGE(_val, _newval, almemory_order_seq_cst)
#define ATOMIC_COMPARE_EXCHANGE_STRONG_SEQ(_val, _oldval, _newval) \
    ATOMIC_COMPARE_EXCHANGE_STRONG(_val, _oldval, _newval, almemory_order_seq_cst, almemory_order_seq_cst)
#define ATOMIC_COMPARE_EXCHANGE_WEAK_SEQ(_val, _oldval, _newval) \
    ATOMIC_COMPARE_EXCHANGE_WEAK(_val, _oldval, _newval, almemory_order_seq_cst, almemory_order_seq_cst)

#define ATOMIC_EXCHANGE_PTR_SEQ(_val, _newval) ATOMIC_EXCHANGE_PTR(_val, _newval, almemory_order_seq_cst)
#define ATOMIC_COMPARE_EXCHANGE_PTR_STRONG_SEQ(_val, _oldval, _newval) \
    ATOMIC_COMPARE_EXCHANGE_PTR_STRONG(_val, _oldval, _newval, almemory_order_seq_cst, almemory_order_seq_cst)
#define ATOMIC_COMPARE_EXCHANGE_PTR_WEAK_SEQ(_val, _oldval, _newval) \
    ATOMIC_COMPARE_EXCHANGE_PTR_WEAK(_val, _oldval, _newval, almemory_order_seq_cst, almemory_order_seq_cst)


typedef unsigned int uint;
typedef ATOMIC(uint) RefCount;

inline void InitRef(RefCount *ptr, uint value)
{ ATOMIC_INIT(ptr, value); }
inline uint ReadRef(RefCount *ptr)
{ return ATOMIC_LOAD(ptr, almemory_order_acquire); }
inline uint IncrementRef(RefCount *ptr)
{ return ATOMIC_ADD(ptr, 1, almemory_order_acq_rel)+1; }
inline uint DecrementRef(RefCount *ptr)
{ return ATOMIC_SUB(ptr, 1, almemory_order_acq_rel)-1; }


/* WARNING: A livelock is theoretically possible if another thread keeps
 * changing the head without giving this a chance to actually swap in the new
 * one (practically impossible with this little code, but...).
 */
#define ATOMIC_REPLACE_HEAD(T, _head, _entry) do {                            \
    T _first = ATOMIC_LOAD(_head, almemory_order_acquire);                    \
    do {                                                                      \
        ATOMIC_STORE(&(_entry)->next, _first, almemory_order_relaxed);        \
    } while(ATOMIC_COMPARE_EXCHANGE_PTR_WEAK(_head, &_first, _entry,          \
            almemory_order_acq_rel, almemory_order_acquire) == 0);            \
} while(0)

#ifdef __cplusplus
}
#endif

#endif /* AL_ATOMIC_H */
