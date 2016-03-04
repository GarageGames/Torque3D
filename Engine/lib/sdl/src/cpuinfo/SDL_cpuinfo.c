/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#ifdef TEST_MAIN
#include "SDL_config.h"
#else
#include "../SDL_internal.h"
#endif

#if defined(__WIN32__)
#include "../core/windows/SDL_windows.h"
#endif

/* CPU feature detection for SDL */

#include "SDL_cpuinfo.h"

#ifdef HAVE_SYSCONF
#include <unistd.h>
#endif
#ifdef HAVE_SYSCTLBYNAME
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
#if defined(__MACOSX__) && (defined(__ppc__) || defined(__ppc64__))
#include <sys/sysctl.h>         /* For AltiVec check */
#elif defined(__OpenBSD__) && defined(__powerpc__)
#include <sys/param.h>
#include <sys/sysctl.h> /* For AltiVec check */
#include <machine/cpu.h>
#elif SDL_ALTIVEC_BLITTERS && HAVE_SETJMP
#include <signal.h>
#include <setjmp.h>
#endif

#define CPU_HAS_RDTSC   0x00000001
#define CPU_HAS_ALTIVEC 0x00000002
#define CPU_HAS_MMX     0x00000004
#define CPU_HAS_3DNOW   0x00000008
#define CPU_HAS_SSE     0x00000010
#define CPU_HAS_SSE2    0x00000020
#define CPU_HAS_SSE3    0x00000040
#define CPU_HAS_SSE41   0x00000100
#define CPU_HAS_SSE42   0x00000200
#define CPU_HAS_AVX     0x00000400

#if SDL_ALTIVEC_BLITTERS && HAVE_SETJMP && !__MACOSX__ && !__OpenBSD__
/* This is the brute force way of detecting instruction sets...
   the idea is borrowed from the libmpeg2 library - thanks!
 */
static jmp_buf jmpbuf;
static void
illegal_instruction(int sig)
{
    longjmp(jmpbuf, 1);
}
#endif /* HAVE_SETJMP */

static SDL_INLINE int
CPU_haveCPUID(void)
{
    int has_CPUID = 0;
/* *INDENT-OFF* */
#if defined(__GNUC__) && defined(i386)
    __asm__ (
"        pushfl                      # Get original EFLAGS             \n"
"        popl    %%eax                                                 \n"
"        movl    %%eax,%%ecx                                           \n"
"        xorl    $0x200000,%%eax     # Flip ID bit in EFLAGS           \n"
"        pushl   %%eax               # Save new EFLAGS value on stack  \n"
"        popfl                       # Replace current EFLAGS value    \n"
"        pushfl                      # Get new EFLAGS                  \n"
"        popl    %%eax               # Store new EFLAGS in EAX         \n"
"        xorl    %%ecx,%%eax         # Can not toggle ID bit,          \n"
"        jz      1f                  # Processor=80486                 \n"
"        movl    $1,%0               # We have CPUID support           \n"
"1:                                                                    \n"
    : "=m" (has_CPUID)
    :
    : "%eax", "%ecx"
    );
#elif defined(__GNUC__) && defined(__x86_64__)
/* Technically, if this is being compiled under __x86_64__ then it has 
   CPUid by definition.  But it's nice to be able to prove it.  :)      */
    __asm__ (
"        pushfq                      # Get original EFLAGS             \n"
"        popq    %%rax                                                 \n"
"        movq    %%rax,%%rcx                                           \n"
"        xorl    $0x200000,%%eax     # Flip ID bit in EFLAGS           \n"
"        pushq   %%rax               # Save new EFLAGS value on stack  \n"
"        popfq                       # Replace current EFLAGS value    \n"
"        pushfq                      # Get new EFLAGS                  \n"
"        popq    %%rax               # Store new EFLAGS in EAX         \n"
"        xorl    %%ecx,%%eax         # Can not toggle ID bit,          \n"
"        jz      1f                  # Processor=80486                 \n"
"        movl    $1,%0               # We have CPUID support           \n"
"1:                                                                    \n"
    : "=m" (has_CPUID)
    :
    : "%rax", "%rcx"
    );
#elif (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)
    __asm {
        pushfd                      ; Get original EFLAGS
        pop     eax
        mov     ecx, eax
        xor     eax, 200000h        ; Flip ID bit in EFLAGS
        push    eax                 ; Save new EFLAGS value on stack
        popfd                       ; Replace current EFLAGS value
        pushfd                      ; Get new EFLAGS
        pop     eax                 ; Store new EFLAGS in EAX
        xor     eax, ecx            ; Can not toggle ID bit,
        jz      done                ; Processor=80486
        mov     has_CPUID,1         ; We have CPUID support
done:
    }
#elif defined(_MSC_VER) && defined(_M_X64)
    has_CPUID = 1;
#elif defined(__sun) && defined(__i386)
    __asm (
"       pushfl                 \n"
"       popl    %eax           \n"
"       movl    %eax,%ecx      \n"
"       xorl    $0x200000,%eax \n"
"       pushl   %eax           \n"
"       popfl                  \n"
"       pushfl                 \n"
"       popl    %eax           \n"
"       xorl    %ecx,%eax      \n"
"       jz      1f             \n"
"       movl    $1,-8(%ebp)    \n"
"1:                            \n"
    );
#elif defined(__sun) && defined(__amd64)
    __asm (
"       pushfq                 \n"
"       popq    %rax           \n"
"       movq    %rax,%rcx      \n"
"       xorl    $0x200000,%eax \n"
"       pushq   %rax           \n"
"       popfq                  \n"
"       pushfq                 \n"
"       popq    %rax           \n"
"       xorl    %ecx,%eax      \n"
"       jz      1f             \n"
"       movl    $1,-8(%rbp)    \n"
"1:                            \n"
    );
#endif
/* *INDENT-ON* */
    return has_CPUID;
}

#if defined(__GNUC__) && defined(i386)
#define cpuid(func, a, b, c, d) \
    __asm__ __volatile__ ( \
"        pushl %%ebx        \n" \
"        cpuid              \n" \
"        movl %%ebx, %%esi  \n" \
"        popl %%ebx         \n" : \
            "=a" (a), "=S" (b), "=c" (c), "=d" (d) : "a" (func))
#elif defined(__GNUC__) && defined(__x86_64__)
#define cpuid(func, a, b, c, d) \
    __asm__ __volatile__ ( \
"        pushq %%rbx        \n" \
"        cpuid              \n" \
"        movq %%rbx, %%rsi  \n" \
"        popq %%rbx         \n" : \
            "=a" (a), "=S" (b), "=c" (c), "=d" (d) : "a" (func))
#elif (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)
#define cpuid(func, a, b, c, d) \
    __asm { \
        __asm mov eax, func \
        __asm cpuid \
        __asm mov a, eax \
        __asm mov b, ebx \
        __asm mov c, ecx \
        __asm mov d, edx \
}
#elif defined(_MSC_VER) && defined(_M_X64)
#define cpuid(func, a, b, c, d) \
{ \
    int CPUInfo[4]; \
    __cpuid(CPUInfo, func); \
    a = CPUInfo[0]; \
    b = CPUInfo[1]; \
    c = CPUInfo[2]; \
    d = CPUInfo[3]; \
}
#else
#define cpuid(func, a, b, c, d) \
    a = b = c = d = 0
#endif

static SDL_INLINE int
CPU_getCPUIDFeatures(void)
{
    int features = 0;
    int a, b, c, d;

    cpuid(0, a, b, c, d);
    if (a >= 1) {
        cpuid(1, a, b, c, d);
        features = d;
    }
    return features;
}

static SDL_INLINE int
CPU_haveRDTSC(void)
{
    if (CPU_haveCPUID()) {
        return (CPU_getCPUIDFeatures() & 0x00000010);
    }
    return 0;
}

static SDL_INLINE int
CPU_haveAltiVec(void)
{
    volatile int altivec = 0;
#if (defined(__MACOSX__) && (defined(__ppc__) || defined(__ppc64__))) || (defined(__OpenBSD__) && defined(__powerpc__))
#ifdef __OpenBSD__
    int selectors[2] = { CTL_MACHDEP, CPU_ALTIVEC };
#else
    int selectors[2] = { CTL_HW, HW_VECTORUNIT };
#endif
    int hasVectorUnit = 0;
    size_t length = sizeof(hasVectorUnit);
    int error = sysctl(selectors, 2, &hasVectorUnit, &length, NULL, 0);
    if (0 == error)
        altivec = (hasVectorUnit != 0);
#elif SDL_ALTIVEC_BLITTERS && HAVE_SETJMP
    void (*handler) (int sig);
    handler = signal(SIGILL, illegal_instruction);
    if (setjmp(jmpbuf) == 0) {
        asm volatile ("mtspr 256, %0\n\t" "vand %%v0, %%v0, %%v0"::"r" (-1));
        altivec = 1;
    }
    signal(SIGILL, handler);
#endif
    return altivec;
}

static SDL_INLINE int
CPU_haveMMX(void)
{
    if (CPU_haveCPUID()) {
        return (CPU_getCPUIDFeatures() & 0x00800000);
    }
    return 0;
}

static SDL_INLINE int
CPU_have3DNow(void)
{
    if (CPU_haveCPUID()) {
        int a, b, c, d;

        cpuid(0x80000000, a, b, c, d);
        if (a >= 0x80000001) {
            cpuid(0x80000001, a, b, c, d);
            return (d & 0x80000000);
        }
    }
    return 0;
}

static SDL_INLINE int
CPU_haveSSE(void)
{
    if (CPU_haveCPUID()) {
        return (CPU_getCPUIDFeatures() & 0x02000000);
    }
    return 0;
}

static SDL_INLINE int
CPU_haveSSE2(void)
{
    if (CPU_haveCPUID()) {
        return (CPU_getCPUIDFeatures() & 0x04000000);
    }
    return 0;
}

static SDL_INLINE int
CPU_haveSSE3(void)
{
    if (CPU_haveCPUID()) {
        int a, b, c, d;

        cpuid(0, a, b, c, d);
        if (a >= 1) {
            cpuid(1, a, b, c, d);
            return (c & 0x00000001);
        }
    }
    return 0;
}

static SDL_INLINE int
CPU_haveSSE41(void)
{
    if (CPU_haveCPUID()) {
        int a, b, c, d;

        cpuid(1, a, b, c, d);
        if (a >= 1) {
            cpuid(1, a, b, c, d);
            return (c & 0x00080000);
        }
    }
    return 0;
}

static SDL_INLINE int
CPU_haveSSE42(void)
{
    if (CPU_haveCPUID()) {
        int a, b, c, d;

        cpuid(1, a, b, c, d);
        if (a >= 1) {
            cpuid(1, a, b, c, d);
            return (c & 0x00100000);
        }
    }
    return 0;
}

static SDL_INLINE int
CPU_haveAVX(void)
{
    if (CPU_haveCPUID()) {
        int a, b, c, d;

        cpuid(1, a, b, c, d);
        if (a >= 1) {
            cpuid(1, a, b, c, d);
            return (c & 0x10000000);
        }
    }
    return 0;
}

static int SDL_CPUCount = 0;

int
SDL_GetCPUCount(void)
{
    if (!SDL_CPUCount) {
#if defined(HAVE_SYSCONF) && defined(_SC_NPROCESSORS_ONLN)
        if (SDL_CPUCount <= 0) {
            SDL_CPUCount = (int)sysconf(_SC_NPROCESSORS_ONLN);
        }
#endif
#ifdef HAVE_SYSCTLBYNAME
        if (SDL_CPUCount <= 0) {
            size_t size = sizeof(SDL_CPUCount);
            sysctlbyname("hw.ncpu", &SDL_CPUCount, &size, NULL, 0);
        }
#endif
#ifdef __WIN32__
        if (SDL_CPUCount <= 0) {
            SYSTEM_INFO info;
            GetSystemInfo(&info);
            SDL_CPUCount = info.dwNumberOfProcessors;
        }
#endif
        /* There has to be at least 1, right? :) */
        if (SDL_CPUCount <= 0) {
            SDL_CPUCount = 1;
        }
    }
    return SDL_CPUCount;
}

/* Oh, such a sweet sweet trick, just not very useful. :) */
static const char *
SDL_GetCPUType(void)
{
    static char SDL_CPUType[13];

    if (!SDL_CPUType[0]) {
        int i = 0;
        int a, b, c, d;

        if (CPU_haveCPUID()) {
            cpuid(0x00000000, a, b, c, d);
            SDL_CPUType[i++] = (char)(b & 0xff); b >>= 8;
            SDL_CPUType[i++] = (char)(b & 0xff); b >>= 8;
            SDL_CPUType[i++] = (char)(b & 0xff); b >>= 8;
            SDL_CPUType[i++] = (char)(b & 0xff); b >>= 8;
            SDL_CPUType[i++] = (char)(d & 0xff); d >>= 8;
            SDL_CPUType[i++] = (char)(d & 0xff); d >>= 8;
            SDL_CPUType[i++] = (char)(d & 0xff); d >>= 8;
            SDL_CPUType[i++] = (char)(d & 0xff); d >>= 8;
            SDL_CPUType[i++] = (char)(c & 0xff); c >>= 8;
            SDL_CPUType[i++] = (char)(c & 0xff); c >>= 8;
            SDL_CPUType[i++] = (char)(c & 0xff); c >>= 8;
            SDL_CPUType[i++] = (char)(c & 0xff); c >>= 8;
        }
        if (!SDL_CPUType[0]) {
            SDL_strlcpy(SDL_CPUType, "Unknown", sizeof(SDL_CPUType));
        }
    }
    return SDL_CPUType;
}


#ifdef TEST_MAIN  /* !!! FIXME: only used for test at the moment. */
static const char *
SDL_GetCPUName(void)
{
    static char SDL_CPUName[48];

    if (!SDL_CPUName[0]) {
        int i = 0;
        int a, b, c, d;

        if (CPU_haveCPUID()) {
            cpuid(0x80000000, a, b, c, d);
            if (a >= 0x80000004) {
                cpuid(0x80000002, a, b, c, d);
                SDL_CPUName[i++] = (char)(a & 0xff); a >>= 8;
                SDL_CPUName[i++] = (char)(a & 0xff); a >>= 8;
                SDL_CPUName[i++] = (char)(a & 0xff); a >>= 8;
                SDL_CPUName[i++] = (char)(a & 0xff); a >>= 8;
                SDL_CPUName[i++] = (char)(b & 0xff); b >>= 8;
                SDL_CPUName[i++] = (char)(b & 0xff); b >>= 8;
                SDL_CPUName[i++] = (char)(b & 0xff); b >>= 8;
                SDL_CPUName[i++] = (char)(b & 0xff); b >>= 8;
                SDL_CPUName[i++] = (char)(c & 0xff); c >>= 8;
                SDL_CPUName[i++] = (char)(c & 0xff); c >>= 8;
                SDL_CPUName[i++] = (char)(c & 0xff); c >>= 8;
                SDL_CPUName[i++] = (char)(c & 0xff); c >>= 8;
                SDL_CPUName[i++] = (char)(d & 0xff); d >>= 8;
                SDL_CPUName[i++] = (char)(d & 0xff); d >>= 8;
                SDL_CPUName[i++] = (char)(d & 0xff); d >>= 8;
                SDL_CPUName[i++] = (char)(d & 0xff); d >>= 8;
                cpuid(0x80000003, a, b, c, d);
                SDL_CPUName[i++] = (char)(a & 0xff); a >>= 8;
                SDL_CPUName[i++] = (char)(a & 0xff); a >>= 8;
                SDL_CPUName[i++] = (char)(a & 0xff); a >>= 8;
                SDL_CPUName[i++] = (char)(a & 0xff); a >>= 8;
                SDL_CPUName[i++] = (char)(b & 0xff); b >>= 8;
                SDL_CPUName[i++] = (char)(b & 0xff); b >>= 8;
                SDL_CPUName[i++] = (char)(b & 0xff); b >>= 8;
                SDL_CPUName[i++] = (char)(b & 0xff); b >>= 8;
                SDL_CPUName[i++] = (char)(c & 0xff); c >>= 8;
                SDL_CPUName[i++] = (char)(c & 0xff); c >>= 8;
                SDL_CPUName[i++] = (char)(c & 0xff); c >>= 8;
                SDL_CPUName[i++] = (char)(c & 0xff); c >>= 8;
                SDL_CPUName[i++] = (char)(d & 0xff); d >>= 8;
                SDL_CPUName[i++] = (char)(d & 0xff); d >>= 8;
                SDL_CPUName[i++] = (char)(d & 0xff); d >>= 8;
                SDL_CPUName[i++] = (char)(d & 0xff); d >>= 8;
                cpuid(0x80000004, a, b, c, d);
                SDL_CPUName[i++] = (char)(a & 0xff); a >>= 8;
                SDL_CPUName[i++] = (char)(a & 0xff); a >>= 8;
                SDL_CPUName[i++] = (char)(a & 0xff); a >>= 8;
                SDL_CPUName[i++] = (char)(a & 0xff); a >>= 8;
                SDL_CPUName[i++] = (char)(b & 0xff); b >>= 8;
                SDL_CPUName[i++] = (char)(b & 0xff); b >>= 8;
                SDL_CPUName[i++] = (char)(b & 0xff); b >>= 8;
                SDL_CPUName[i++] = (char)(b & 0xff); b >>= 8;
                SDL_CPUName[i++] = (char)(c & 0xff); c >>= 8;
                SDL_CPUName[i++] = (char)(c & 0xff); c >>= 8;
                SDL_CPUName[i++] = (char)(c & 0xff); c >>= 8;
                SDL_CPUName[i++] = (char)(c & 0xff); c >>= 8;
                SDL_CPUName[i++] = (char)(d & 0xff); d >>= 8;
                SDL_CPUName[i++] = (char)(d & 0xff); d >>= 8;
                SDL_CPUName[i++] = (char)(d & 0xff); d >>= 8;
                SDL_CPUName[i++] = (char)(d & 0xff); d >>= 8;
            }
        }
        if (!SDL_CPUName[0]) {
            SDL_strlcpy(SDL_CPUName, "Unknown", sizeof(SDL_CPUName));
        }
    }
    return SDL_CPUName;
}
#endif

int
SDL_GetCPUCacheLineSize(void)
{
    const char *cpuType = SDL_GetCPUType();

    if (SDL_strcmp(cpuType, "GenuineIntel") == 0) {
        int a, b, c, d;

        cpuid(0x00000001, a, b, c, d);
        return (((b >> 8) & 0xff) * 8);
    } else if (SDL_strcmp(cpuType, "AuthenticAMD") == 0) {
        int a, b, c, d;

        cpuid(0x80000005, a, b, c, d);
        return (c & 0xff);
    } else {
        /* Just make a guess here... */
        return SDL_CACHELINE_SIZE;
    }
}

static Uint32 SDL_CPUFeatures = 0xFFFFFFFF;

static Uint32
SDL_GetCPUFeatures(void)
{
    if (SDL_CPUFeatures == 0xFFFFFFFF) {
        SDL_CPUFeatures = 0;
        if (CPU_haveRDTSC()) {
            SDL_CPUFeatures |= CPU_HAS_RDTSC;
        }
        if (CPU_haveAltiVec()) {
            SDL_CPUFeatures |= CPU_HAS_ALTIVEC;
        }
        if (CPU_haveMMX()) {
            SDL_CPUFeatures |= CPU_HAS_MMX;
        }
        if (CPU_have3DNow()) {
            SDL_CPUFeatures |= CPU_HAS_3DNOW;
        }
        if (CPU_haveSSE()) {
            SDL_CPUFeatures |= CPU_HAS_SSE;
        }
        if (CPU_haveSSE2()) {
            SDL_CPUFeatures |= CPU_HAS_SSE2;
        }
        if (CPU_haveSSE3()) {
            SDL_CPUFeatures |= CPU_HAS_SSE3;
        }
        if (CPU_haveSSE41()) {
            SDL_CPUFeatures |= CPU_HAS_SSE41;
        }
        if (CPU_haveSSE42()) {
            SDL_CPUFeatures |= CPU_HAS_SSE42;
        }
        if (CPU_haveAVX()) {
            SDL_CPUFeatures |= CPU_HAS_AVX;
        }
    }
    return SDL_CPUFeatures;
}

SDL_bool
SDL_HasRDTSC(void)
{
    if (SDL_GetCPUFeatures() & CPU_HAS_RDTSC) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

SDL_bool
SDL_HasAltiVec(void)
{
    if (SDL_GetCPUFeatures() & CPU_HAS_ALTIVEC) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

SDL_bool
SDL_HasMMX(void)
{
    if (SDL_GetCPUFeatures() & CPU_HAS_MMX) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

SDL_bool
SDL_Has3DNow(void)
{
    if (SDL_GetCPUFeatures() & CPU_HAS_3DNOW) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

SDL_bool
SDL_HasSSE(void)
{
    if (SDL_GetCPUFeatures() & CPU_HAS_SSE) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

SDL_bool
SDL_HasSSE2(void)
{
    if (SDL_GetCPUFeatures() & CPU_HAS_SSE2) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

SDL_bool
SDL_HasSSE3(void)
{
    if (SDL_GetCPUFeatures() & CPU_HAS_SSE3) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

SDL_bool
SDL_HasSSE41(void)
{
    if (SDL_GetCPUFeatures() & CPU_HAS_SSE41) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

SDL_bool
SDL_HasSSE42(void)
{
    if (SDL_GetCPUFeatures() & CPU_HAS_SSE42) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

SDL_bool
SDL_HasAVX(void)
{
    if (SDL_GetCPUFeatures() & CPU_HAS_AVX) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

static int SDL_SystemRAM = 0;

int
SDL_GetSystemRAM(void)
{
    if (!SDL_SystemRAM) {
#if defined(HAVE_SYSCONF) && defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
        if (SDL_SystemRAM <= 0) {
            SDL_SystemRAM = (int)((Sint64)sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE) / (1024*1024));
        }
#endif
#ifdef HAVE_SYSCTLBYNAME
        if (SDL_SystemRAM <= 0) {
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#ifdef HW_REALMEM
            int mib[2] = {CTL_HW, HW_REALMEM};
#else
            /* might only report up to 2 GiB */
            int mib[2] = {CTL_HW, HW_PHYSMEM};
#endif /* HW_REALMEM */
#else
            int mib[2] = {CTL_HW, HW_MEMSIZE};
#endif /* __FreeBSD__ || __FreeBSD_kernel__ */
            Uint64 memsize = 0;
            size_t len = sizeof(memsize);
            
            if (sysctl(mib, 2, &memsize, &len, NULL, 0) == 0) {
                SDL_SystemRAM = (int)(memsize / (1024*1024));
            }
        }
#endif
#ifdef __WIN32__
        if (SDL_SystemRAM <= 0) {
            MEMORYSTATUSEX stat;
            stat.dwLength = sizeof(stat);
            if (GlobalMemoryStatusEx(&stat)) {
                SDL_SystemRAM = (int)(stat.ullTotalPhys / (1024 * 1024));
            }
        }
#endif
    }
    return SDL_SystemRAM;
}


#ifdef TEST_MAIN

#include <stdio.h>

int
main()
{
    printf("CPU count: %d\n", SDL_GetCPUCount());
    printf("CPU type: %s\n", SDL_GetCPUType());
    printf("CPU name: %s\n", SDL_GetCPUName());
    printf("CacheLine size: %d\n", SDL_GetCPUCacheLineSize());
    printf("RDTSC: %d\n", SDL_HasRDTSC());
    printf("Altivec: %d\n", SDL_HasAltiVec());
    printf("MMX: %d\n", SDL_HasMMX());
    printf("3DNow: %d\n", SDL_Has3DNow());
    printf("SSE: %d\n", SDL_HasSSE());
    printf("SSE2: %d\n", SDL_HasSSE2());
    printf("SSE3: %d\n", SDL_HasSSE3());
    printf("SSE4.1: %d\n", SDL_HasSSE41());
    printf("SSE4.2: %d\n", SDL_HasSSE42());
    printf("AVX: %d\n", SDL_HasAVX());
    printf("RAM: %d MB\n", SDL_GetSystemRAM());
    return 0;
}

#endif /* TEST_MAIN */

/* vi: set ts=4 sw=4 expandtab: */
