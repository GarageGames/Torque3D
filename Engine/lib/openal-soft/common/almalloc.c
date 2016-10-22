
#include "config.h"

#include "almalloc.h"

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

void *al_malloc(size_t alignment, size_t size)
{
#if defined(HAVE_ALIGNED_ALLOC)
    size = (size+(alignment-1))&~(alignment-1);
    return aligned_alloc(alignment, size);
#elif defined(HAVE_POSIX_MEMALIGN)
    void *ret;
    if(posix_memalign(&ret, alignment, size) == 0)
        return ret;
    return NULL;
#elif defined(HAVE__ALIGNED_MALLOC)
    return _aligned_malloc(size, alignment);
#else
    char *ret = malloc(size+alignment);
    if(ret != NULL)
    {
        *(ret++) = 0x00;
        while(((ptrdiff_t)ret&(alignment-1)) != 0)
            *(ret++) = 0x55;
    }
    return ret;
#endif
}

void *al_calloc(size_t alignment, size_t size)
{
    void *ret = al_malloc(alignment, size);
    if(ret) memset(ret, 0, size);
    return ret;
}

void al_free(void *ptr)
{
#if defined(HAVE_ALIGNED_ALLOC) || defined(HAVE_POSIX_MEMALIGN)
    free(ptr);
#elif defined(HAVE__ALIGNED_MALLOC)
    _aligned_free(ptr);
#else
    if(ptr != NULL)
    {
        char *finder = ptr;
        do {
            --finder;
        } while(*finder == 0x55);
        free(finder);
    }
#endif
}
