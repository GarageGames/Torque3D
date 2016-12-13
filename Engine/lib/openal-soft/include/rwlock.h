#ifndef AL_RWLOCK_H
#define AL_RWLOCK_H

#include "bool.h"
#include "atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    RefCount read_count;
    RefCount write_count;
    ATOMIC(int) read_lock;
    ATOMIC(int) read_entry_lock;
    ATOMIC(int) write_lock;
} RWLock;
#define RWLOCK_STATIC_INITIALIZE { ATOMIC_INIT_STATIC(0), ATOMIC_INIT_STATIC(0),         \
                                   ATOMIC_INIT_STATIC(false), ATOMIC_INIT_STATIC(false), \
                                   ATOMIC_INIT_STATIC(false) }

void RWLockInit(RWLock *lock);
void ReadLock(RWLock *lock);
void ReadUnlock(RWLock *lock);
void WriteLock(RWLock *lock);
void WriteUnlock(RWLock *lock);

#ifdef __cplusplus
}
#endif

#endif /* AL_RWLOCK_H */
