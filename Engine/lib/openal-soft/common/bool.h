#ifndef AL_BOOL_H
#define AL_BOOL_H

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

#ifndef bool
#ifdef HAVE_C99_BOOL
#define bool _Bool
#else
#define bool int
#endif
#define false 0
#define true 1
#endif

#endif /* AL_BOOL_H */
