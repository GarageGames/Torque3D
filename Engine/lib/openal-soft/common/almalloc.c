
#include "config.h"

#include "almalloc.h"

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#else
#include <unistd.h>
#endif


#ifdef __GNUC__
#define LIKELY(x) __builtin_expect(!!(x), !0)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x) (!!(x))
#define UNLIKELY(x) (!!(x))
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

size_t al_get_page_size(void)
{
    static size_t psize = 0;
    if(UNLIKELY(!psize))
    {
#ifdef HAVE_SYSCONF
#if defined(_SC_PAGESIZE)
        if(!psize) psize = sysconf(_SC_PAGESIZE);
#elif defined(_SC_PAGE_SIZE)
        if(!psize) psize = sysconf(_SC_PAGE_SIZE);
#endif
#endif /* HAVE_SYSCONF */
#ifdef _WIN32
        if(!psize)
        {
            SYSTEM_INFO sysinfo;
            memset(&sysinfo, 0, sizeof(sysinfo));

            GetSystemInfo(&sysinfo);
            psize = sysinfo.dwPageSize;
        }
#endif
        if(!psize) psize = DEF_ALIGN;
    }
    return psize;
}

int al_is_sane_alignment_allocator(void)
{
#if defined(HAVE_ALIGNED_ALLOC) || defined(HAVE_POSIX_MEMALIGN) || defined(HAVE__ALIGNED_MALLOC)
    return 1;
#else
    return 0;
#endif
}
