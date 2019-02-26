
#include "config.h"

#include "rwlock.h"

#include "bool.h"
#include "atomic.h"
#include "threads.h"


/* A simple spinlock. Yield the thread while the given integer is set by
 * another. Could probably be improved... */
#define LOCK(l) do {                                                          \
    while(ATOMIC_FLAG_TEST_AND_SET(&(l), almemory_order_acq_rel) == true)     \
        althrd_yield();                                                       \
} while(0)
#define UNLOCK(l) ATOMIC_FLAG_CLEAR(&(l), almemory_order_release)


void RWLockInit(RWLock *lock)
{
    InitRef(&lock->read_count, 0);
    InitRef(&lock->write_count, 0);
    ATOMIC_FLAG_CLEAR(&lock->read_lock, almemory_order_relaxed);
    ATOMIC_FLAG_CLEAR(&lock->read_entry_lock, almemory_order_relaxed);
    ATOMIC_FLAG_CLEAR(&lock->write_lock, almemory_order_relaxed);
}

void ReadLock(RWLock *lock)
{
    LOCK(lock->read_entry_lock);
    LOCK(lock->read_lock);
    /* NOTE: ATOMIC_ADD returns the *old* value! */
    if(ATOMIC_ADD(&lock->read_count, 1, almemory_order_acq_rel) == 0)
        LOCK(lock->write_lock);
    UNLOCK(lock->read_lock);
    UNLOCK(lock->read_entry_lock);
}

void ReadUnlock(RWLock *lock)
{
    /* NOTE: ATOMIC_SUB returns the *old* value! */
    if(ATOMIC_SUB(&lock->read_count, 1, almemory_order_acq_rel) == 1)
        UNLOCK(lock->write_lock);
}

void WriteLock(RWLock *lock)
{
    if(ATOMIC_ADD(&lock->write_count, 1, almemory_order_acq_rel) == 0)
        LOCK(lock->read_lock);
    LOCK(lock->write_lock);
}

void WriteUnlock(RWLock *lock)
{
    UNLOCK(lock->write_lock);
    if(ATOMIC_SUB(&lock->write_count, 1, almemory_order_acq_rel) == 1)
        UNLOCK(lock->read_lock);
}
