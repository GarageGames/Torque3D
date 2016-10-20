
#include "config.h"

#include "rwlock.h"

#include "bool.h"
#include "atomic.h"
#include "threads.h"


/* A simple spinlock. Yield the thread while the given integer is set by
 * another. Could probably be improved... */
#define LOCK(l) do {                                                          \
    while(ATOMIC_EXCHANGE(int, &(l), true) == true)                            \
        althrd_yield();                                                       \
} while(0)
#define UNLOCK(l) ATOMIC_STORE(&(l), false)


void RWLockInit(RWLock *lock)
{
    InitRef(&lock->read_count, 0);
    InitRef(&lock->write_count, 0);
    ATOMIC_INIT(&lock->read_lock, false);
    ATOMIC_INIT(&lock->read_entry_lock, false);
    ATOMIC_INIT(&lock->write_lock, false);
}

void ReadLock(RWLock *lock)
{
    LOCK(lock->read_entry_lock);
    LOCK(lock->read_lock);
    if(IncrementRef(&lock->read_count) == 1)
        LOCK(lock->write_lock);
    UNLOCK(lock->read_lock);
    UNLOCK(lock->read_entry_lock);
}

void ReadUnlock(RWLock *lock)
{
    if(DecrementRef(&lock->read_count) == 0)
        UNLOCK(lock->write_lock);
}

void WriteLock(RWLock *lock)
{
    if(IncrementRef(&lock->write_count) == 1)
        LOCK(lock->read_lock);
    LOCK(lock->write_lock);
}

void WriteUnlock(RWLock *lock)
{
    UNLOCK(lock->write_lock);
    if(DecrementRef(&lock->write_count) == 0)
        UNLOCK(lock->read_lock);
}
