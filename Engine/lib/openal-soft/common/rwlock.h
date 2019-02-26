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
    ATOMIC_FLAG read_lock;
    ATOMIC_FLAG read_entry_lock;
    ATOMIC_FLAG write_lock;
} RWLock;
#define RWLOCK_STATIC_INITIALIZE { ATOMIC_INIT_STATIC(0), ATOMIC_INIT_STATIC(0),         \
                                   ATOMIC_FLAG_INIT, ATOMIC_FLAG_INIT, ATOMIC_FLAG_INIT }

void RWLockInit(RWLock *lock);
void ReadLock(RWLock *lock);
void ReadUnlock(RWLock *lock);
void WriteLock(RWLock *lock);
void WriteUnlock(RWLock *lock);

#ifdef __cplusplus
}
#endif

#endif /* AL_RWLOCK_H */
