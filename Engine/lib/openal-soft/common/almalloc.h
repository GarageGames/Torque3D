#ifndef AL_MALLOC_H
#define AL_MALLOC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Minimum alignment required by posix_memalign. */
#define DEF_ALIGN sizeof(void*)

void *al_malloc(size_t alignment, size_t size);
void *al_calloc(size_t alignment, size_t size);
void al_free(void *ptr);

size_t al_get_page_size(void);

/**
 * Returns non-0 if the allocation function has direct alignment handling.
 * Otherwise, the standard malloc is used with an over-allocation and pointer
 * offset strategy.
 */
int al_is_sane_alignment_allocator(void);

#ifdef __cplusplus
}
#endif

#endif /* AL_MALLOC_H */
