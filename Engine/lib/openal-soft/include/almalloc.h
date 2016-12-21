#ifndef AL_MALLOC_H
#define AL_MALLOC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *al_malloc(size_t alignment, size_t size);
void *al_calloc(size_t alignment, size_t size);
void al_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* AL_MALLOC_H */
