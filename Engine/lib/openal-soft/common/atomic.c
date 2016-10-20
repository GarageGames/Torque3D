
#include "config.h"

#include "atomic.h"


extern inline void InitRef(RefCount *ptr, uint value);
extern inline uint ReadRef(RefCount *ptr);
extern inline uint IncrementRef(RefCount *ptr);
extern inline uint DecrementRef(RefCount *ptr);
