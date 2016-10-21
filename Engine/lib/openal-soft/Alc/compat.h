#ifndef AL_COMPAT_H
#define AL_COMPAT_H

#include "alstring.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

WCHAR *strdupW(const WCHAR *str);

/* Opens a file with standard I/O. The filename is expected to be UTF-8. */
FILE *al_fopen(const char *fname, const char *mode);

#define HAVE_DYNLOAD 1

#else

#define al_fopen fopen

#if defined(HAVE_DLFCN_H) && !defined(IN_IDE_PARSER)
#define HAVE_DYNLOAD 1
#endif

#endif

struct FileMapping {
#ifdef _WIN32
    HANDLE file;
    HANDLE fmap;
#else
    int fd;
#endif
    void *ptr;
    size_t len;
};
struct FileMapping MapFileToMem(const char *fname);
void UnmapFileMem(const struct FileMapping *mapping);

al_string GetProcPath(void);

#ifdef HAVE_DYNLOAD
void *LoadLib(const char *name);
void CloseLib(void *handle);
void *GetSymbol(void *handle, const char *name);
#endif

#endif /* AL_COMPAT_H */
