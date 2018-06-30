# CMake build file list for OpenAL

CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

PROJECT(OpenAL)

IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
    CMAKE_POLICY(SET CMP0005 NEW)
    IF(POLICY CMP0020)
        CMAKE_POLICY(SET CMP0020 NEW)
    ENDIF(POLICY CMP0020)
    IF(POLICY CMP0042)
        CMAKE_POLICY(SET CMP0042 NEW)
    ENDIF(POLICY CMP0042)
    IF(POLICY CMP0054)
        CMAKE_POLICY(SET CMP0054 NEW)
    ENDIF(POLICY CMP0054)
ENDIF(COMMAND CMAKE_POLICY)

SET(CMAKE_MODULE_PATH "${OpenAL_SOURCE_DIR}/cmake")

INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckSharedFunctionExists)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckTypeSize)
include(CheckStructHasMember)
include(CheckFileOffsetBits)
include(GNUInstallDirs)

SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)


OPTION(ALSOFT_DLOPEN  "Check for the dlopen API for loading optional libs"  ON)

OPTION(ALSOFT_WERROR  "Treat compile warnings as errors"      OFF)

OPTION(ALSOFT_UTILS          "Build and install utility programs"         ON)
OPTION(ALSOFT_NO_CONFIG_UTIL "Disable building the alsoft-config utility" OFF)

OPTION(ALSOFT_EXAMPLES  "Build and install example programs"  ON)
OPTION(ALSOFT_TESTS     "Build and install test programs"     ON)

OPTION(ALSOFT_CONFIG "Install alsoft.conf sample configuration file" ON)
OPTION(ALSOFT_HRTF_DEFS "Install HRTF definition files" ON)
OPTION(ALSOFT_AMBDEC_PRESETS "Install AmbDec preset files" ON)
OPTION(ALSOFT_INSTALL "Install headers and libraries" ON)

if(DEFINED SHARE_INSTALL_DIR)
    message(WARNING "SHARE_INSTALL_DIR is deprecated.  Use the variables provided by the GNUInstallDirs module instead")
    set(CMAKE_INSTALL_DATADIR "${SHARE_INSTALL_DIR}")
endif()

if(DEFINED LIB_SUFFIX)
    message(WARNING "LIB_SUFFIX is deprecated.  Use the variables provided by the GNUInstallDirs module instead")
endif()


SET(CPP_DEFS ) # C pre-process, not C++
SET(INC_PATHS )
SET(C_FLAGS )
SET(LINKER_FLAGS )
SET(EXTRA_LIBS )

IF(WIN32)
    SET(CPP_DEFS ${CPP_DEFS} _WIN32 _WIN32_WINNT=0x0502)

    OPTION(ALSOFT_BUILD_ROUTER  "Build the router (EXPERIMENTAL; creates OpenAL32.dll and soft_oal.dll)"  OFF)

    # This option is mainly for static linking OpenAL Soft into another project
    # that already defines the IDs. It is up to that project to ensure all
    # required IDs are defined.
    OPTION(ALSOFT_NO_UID_DEFS "Do not define GUIDs, IIDs, CLSIDs, or PropertyKeys" OFF)

    IF(MINGW)
        OPTION(ALSOFT_BUILD_IMPORT_LIB "Build an import .lib using dlltool (requires sed)" ON)
        IF(NOT DLLTOOL)
            IF(HOST)
                SET(DLLTOOL "${HOST}-dlltool")
            ELSE()
                SET(DLLTOOL "dlltool")
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()


# QNX's gcc do not uses /usr/include and /usr/lib pathes by default
IF ("${CMAKE_C_PLATFORM_ID}" STREQUAL "QNX")
    SET(INC_PATHS ${INC_PATHS} /usr/include)
    SET(LINKER_FLAGS ${LINKER_FLAGS} -L/usr/lib)
ENDIF()

IF(NOT LIBTYPE)
    SET(LIBTYPE SHARED)
ENDIF()

SET(LIB_MAJOR_VERSION "1")
SET(LIB_MINOR_VERSION "18")
SET(LIB_REVISION "2")
SET(LIB_VERSION "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_REVISION}")

SET(EXPORT_DECL "")
SET(ALIGN_DECL "")


CHECK_TYPE_SIZE("long" SIZEOF_LONG)
CHECK_TYPE_SIZE("long long" SIZEOF_LONG_LONG)


CHECK_C_COMPILER_FLAG(-std=c11 HAVE_STD_C11)
IF(HAVE_STD_C11)
    SET(CMAKE_C_FLAGS "-std=c11 ${CMAKE_C_FLAGS}")
ELSE()
    CHECK_C_COMPILER_FLAG(-std=c99 HAVE_STD_C99)
    IF(HAVE_STD_C99)
        SET(CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS}")
    ENDIF()
ENDIF()

CHECK_CXX_COMPILER_FLAG(-std=c++11 HAVE_STD_CXX11)
IF(HAVE_STD_CXX11)
    SET(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
ENDIF()

if(NOT WIN32)
    # Check if _POSIX_C_SOURCE and _XOPEN_SOURCE needs to be set for POSIX functions
    CHECK_SYMBOL_EXISTS(posix_memalign stdlib.h HAVE_POSIX_MEMALIGN_DEFAULT)
    IF(NOT HAVE_POSIX_MEMALIGN_DEFAULT)
        SET(OLD_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
        SET(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600")
        CHECK_SYMBOL_EXISTS(posix_memalign stdlib.h HAVE_POSIX_MEMALIGN_POSIX)
        IF(NOT HAVE_POSIX_MEMALIGN_POSIX)
            SET(CMAKE_REQUIRED_FLAGS ${OLD_REQUIRED_FLAGS})
        ELSE()
            SET(CPP_DEFS ${CPP_DEFS} _POSIX_C_SOURCE=200112L _XOPEN_SOURCE=600)
        ENDIF()
    ENDIF()
    UNSET(OLD_REQUIRED_FLAGS)
ENDIF()

# Set defines for large file support
CHECK_FILE_OFFSET_BITS()
IF(_FILE_OFFSET_BITS)
    SET(CPP_DEFS ${CPP_DEFS} "_FILE_OFFSET_BITS=${_FILE_OFFSET_BITS}")
    SET(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -D_FILE_OFFSET_BITS=${_FILE_OFFSET_BITS}")
ENDIF()
SET(CPP_DEFS ${CPP_DEFS} _LARGEFILE_SOURCE _LARGE_FILES)
SET(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -D_LARGEFILE_SOURCE -D_LARGE_FILES")

# MSVC may need workarounds for C99 restrict and inline
IF(MSVC)
    # TODO: Once we truly require C99, these restrict and inline checks should go
    # away.
    CHECK_C_SOURCE_COMPILES("int *restrict foo;
                             int main() {return 0;}" HAVE_RESTRICT)
    IF(NOT HAVE_RESTRICT)
        SET(CPP_DEFS ${CPP_DEFS} "restrict=")
        SET(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Drestrict=")
    ENDIF()

    CHECK_C_SOURCE_COMPILES("inline void foo(void) { }
                             int main() {return 0;}" HAVE_INLINE)
    IF(NOT HAVE_INLINE)
        CHECK_C_SOURCE_COMPILES("__inline void foo(void) { }
                                 int main() {return 0;}" HAVE___INLINE)
        IF(NOT HAVE___INLINE)
            MESSAGE(FATAL_ERROR "No inline keyword found, please report!")
        ENDIF()

        SET(CPP_DEFS ${CPP_DEFS} inline=__inline)
        SET(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Dinline=__inline")
    ENDIF()
ENDIF()

# Make sure we have C99-style inline semantics with GCC (4.3 or newer).
IF(CMAKE_COMPILER_IS_GNUCC)
    SET(CMAKE_C_FLAGS "-fno-gnu89-inline ${CMAKE_C_FLAGS}")

    SET(OLD_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
    # Force no inlining for the next test.
    SET(CMAKE_REQUIRED_FLAGS "${OLD_REQUIRED_FLAGS} -fno-inline")

    CHECK_C_SOURCE_COMPILES("extern inline int foo() { return 0; }
                             int main() {return foo();}" INLINE_IS_C99)
    IF(NOT INLINE_IS_C99)
        MESSAGE(FATAL_ERROR "Your compiler does not seem to have C99 inline semantics!
                             Please update your compiler for better C99 compliance.")
    ENDIF()

    SET(CMAKE_REQUIRED_FLAGS "${OLD_REQUIRED_FLAGS}")
ENDIF()

# Check if we have a proper timespec declaration
CHECK_STRUCT_HAS_MEMBER("struct timespec" tv_sec time.h HAVE_STRUCT_TIMESPEC)
IF(HAVE_STRUCT_TIMESPEC)
    # Define it here so we don't have to include config.h for it
    SET(CPP_DEFS ${CPP_DEFS} HAVE_STRUCT_TIMESPEC)
ENDIF()

# Some systems may need libatomic for C11 atomic functions to work
SET(OLD_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
SET(CMAKE_REQUIRED_LIBRARIES ${OLD_REQUIRED_LIBRARIES} atomic)
CHECK_C_SOURCE_COMPILES("#include <stdatomic.h>
int _Atomic foo = ATOMIC_VAR_INIT(0);
int main()
{
    return atomic_fetch_add(&foo, 2);
}"
HAVE_LIBATOMIC)
IF(NOT HAVE_LIBATOMIC)
    SET(CMAKE_REQUIRED_LIBRARIES "${OLD_REQUIRED_LIBRARIES}")
ELSE()
    SET(EXTRA_LIBS atomic ${EXTRA_LIBS})
ENDIF()
UNSET(OLD_REQUIRED_LIBRARIES)

# Include liblog for Android logging
CHECK_LIBRARY_EXISTS(log __android_log_print "" HAVE_LIBLOG)
IF(HAVE_LIBLOG)
    SET(EXTRA_LIBS log ${EXTRA_LIBS})
    SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} log)
ENDIF()

# Check if we have C99 bool
CHECK_C_SOURCE_COMPILES(
"int main(int argc, char *argv[])
 {
     volatile _Bool ret;
     ret = (argc > 1) ? 1 : 0;
     return ret ? -1 : 0;
 }"
HAVE_C99_BOOL)

# Check if we have C11 static_assert
CHECK_C_SOURCE_COMPILES(
"int main()
 {
     _Static_assert(sizeof(int) == sizeof(int), \"What\");
     return 0;
 }"
HAVE_C11_STATIC_ASSERT)

# Check if we have C11 alignas
CHECK_C_SOURCE_COMPILES(
"_Alignas(16) int foo;
 int main()
 {
     return 0;
 }"
HAVE_C11_ALIGNAS)

# Check if we have C11 _Atomic
CHECK_C_SOURCE_COMPILES(
"#include <stdatomic.h>
 int _Atomic foo = ATOMIC_VAR_INIT(0);
 int main()
 {
     atomic_fetch_add(&foo, 2);
     return 0;
 }"
HAVE_C11_ATOMIC)

# Add definitions, compiler switches, etc.
INCLUDE_DIRECTORIES("${OpenAL_SOURCE_DIR}/include" "${OpenAL_SOURCE_DIR}/common" "${OpenAL_BINARY_DIR}")

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
ENDIF()
IF(NOT CMAKE_DEBUG_POSTFIX)
    SET(CMAKE_DEBUG_POSTFIX "" CACHE STRING
        "Library postfix for debug builds. Normally left blank."
        FORCE)
ENDIF()

IF(MSVC)
    SET(CPP_DEFS ${CPP_DEFS} _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
    SET(C_FLAGS ${C_FLAGS} /wd4098)

    IF(NOT DXSDK_DIR)
        STRING(REGEX REPLACE "\\\\" "/" DXSDK_DIR "$ENV{DXSDK_DIR}")
    ELSE()
        STRING(REGEX REPLACE "\\\\" "/" DXSDK_DIR "${DXSDK_DIR}")
    ENDIF()
    IF(DXSDK_DIR)
        MESSAGE(STATUS "Using DirectX SDK directory: ${DXSDK_DIR}")
    ENDIF()

    OPTION(FORCE_STATIC_VCRT "Force /MT for static VC runtimes" OFF)
    IF(FORCE_STATIC_VCRT)
        FOREACH(flag_var
                CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
            IF(${flag_var} MATCHES "/MD")
                STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            ENDIF()
        ENDFOREACH(flag_var)
    ENDIF()
ELSE()
    SET(C_FLAGS ${C_FLAGS} -Winline -Wall)
    CHECK_C_COMPILER_FLAG(-Wextra HAVE_W_EXTRA)
    IF(HAVE_W_EXTRA)
        SET(C_FLAGS ${C_FLAGS} -Wextra)
    ENDIF()

    IF(ALSOFT_WERROR)
        SET(C_FLAGS ${C_FLAGS} -Werror)
    ENDIF()

    # We want RelWithDebInfo to actually include debug stuff (define _DEBUG
    # instead of NDEBUG)
    FOREACH(flag_var  CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        IF(${flag_var} MATCHES "-DNDEBUG")
            STRING(REGEX REPLACE "-DNDEBUG" "-D_DEBUG" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH()

    CHECK_C_COMPILER_FLAG(-fno-math-errno HAVE_FNO_MATH_ERRNO)
    IF(HAVE_FNO_MATH_ERRNO)
        SET(C_FLAGS ${C_FLAGS} -fno-math-errno)
    ENDIF()

    CHECK_C_SOURCE_COMPILES("int foo() __attribute__((destructor));
                             int main() {return 0;}" HAVE_GCC_DESTRUCTOR)

    option(ALSOFT_STATIC_LIBGCC "Force -static-libgcc for static GCC runtimes" OFF)
    if(ALSOFT_STATIC_LIBGCC)
        set(OLD_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
        set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} -static-libgcc)
        check_c_source_compiles(
"#include <stdlib.h>
int main()
{
    return 0;
}"
            HAVE_STATIC_LIBGCC_SWITCH
        )
        if(HAVE_STATIC_LIBGCC_SWITCH)
            SET(LINKER_FLAGS ${LINKER_FLAGS} -static-libgcc)
        endif()
        set(CMAKE_REQUIRED_LIBRARIES ${OLD_REQUIRED_LIBRARIES})
        unset(OLD_REQUIRED_LIBRARIES)
    endif()
ENDIF()

# Set visibility/export options if available
IF(WIN32)
    SET(EXPORT_DECL "__declspec(dllexport)")
    IF(NOT MINGW)
        SET(ALIGN_DECL "__declspec(align(x))")
    ELSE()
        SET(ALIGN_DECL "__declspec(aligned(x))")
    ENDIF()
ELSE()
    SET(OLD_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
    # Yes GCC, really don't accept visibility modes you don't support
    SET(CMAKE_REQUIRED_FLAGS "${OLD_REQUIRED_FLAGS} -Wattributes -Werror")

    CHECK_C_SOURCE_COMPILES("int foo() __attribute__((visibility(\"protected\")));
                             int main() {return 0;}" HAVE_GCC_PROTECTED_VISIBILITY)
    IF(HAVE_GCC_PROTECTED_VISIBILITY)
        SET(EXPORT_DECL "__attribute__((visibility(\"protected\")))")
    ELSE()
        CHECK_C_SOURCE_COMPILES("int foo() __attribute__((visibility(\"default\")));
                                 int main() {return 0;}" HAVE_GCC_DEFAULT_VISIBILITY)
        IF(HAVE_GCC_DEFAULT_VISIBILITY)
            SET(EXPORT_DECL "__attribute__((visibility(\"default\")))")
        ENDIF()
    ENDIF()

    IF(HAVE_GCC_PROTECTED_VISIBILITY OR HAVE_GCC_DEFAULT_VISIBILITY)
        CHECK_C_COMPILER_FLAG(-fvisibility=hidden HAVE_VISIBILITY_HIDDEN_SWITCH)
        IF(HAVE_VISIBILITY_HIDDEN_SWITCH)
            SET(C_FLAGS ${C_FLAGS} -fvisibility=hidden)
        ENDIF()
    ENDIF()

    CHECK_C_SOURCE_COMPILES("int foo __attribute__((aligned(16)));
                             int main() {return 0;}" HAVE_ATTRIBUTE_ALIGNED)
    IF(HAVE_ATTRIBUTE_ALIGNED)
        SET(ALIGN_DECL "__attribute__((aligned(x)))")
    ENDIF()

    SET(CMAKE_REQUIRED_FLAGS "${OLD_REQUIRED_FLAGS}")
ENDIF()

CHECK_C_SOURCE_COMPILES("
int main()
{
    float *ptr;
    ptr = __builtin_assume_aligned(ptr, 16);
    return 0;
}" HAVE___BUILTIN_ASSUME_ALIGNED)
IF(HAVE___BUILTIN_ASSUME_ALIGNED)
    SET(ASSUME_ALIGNED_DECL "__builtin_assume_aligned(x, y)")
ELSE()
    SET(ASSUME_ALIGNED_DECL "x")
ENDIF()

SET(SSE_SWITCH "")
SET(SSE2_SWITCH "")
SET(SSE3_SWITCH "")
SET(SSE4_1_SWITCH "")
SET(FPU_NEON_SWITCH "")

CHECK_C_COMPILER_FLAG(-msse HAVE_MSSE_SWITCH)
IF(HAVE_MSSE_SWITCH)
    SET(SSE_SWITCH "-msse")
ENDIF()
CHECK_C_COMPILER_FLAG(-msse2 HAVE_MSSE2_SWITCH)
IF(HAVE_MSSE2_SWITCH)
    SET(SSE2_SWITCH "-msse2")
ENDIF()
CHECK_C_COMPILER_FLAG(-msse3 HAVE_MSSE3_SWITCH)
IF(HAVE_MSSE3_SWITCH)
    SET(SSE3_SWITCH "-msse3")
ENDIF()
CHECK_C_COMPILER_FLAG(-msse4.1 HAVE_MSSE4_1_SWITCH)
IF(HAVE_MSSE4_1_SWITCH)
    SET(SSE4_1_SWITCH "-msse4.1")
ENDIF()
CHECK_C_COMPILER_FLAG(-mfpu=neon HAVE_MFPU_NEON_SWITCH)
IF(HAVE_MFPU_NEON_SWITCH)
    SET(FPU_NEON_SWITCH "-mfpu=neon")
ENDIF()

CHECK_C_SOURCE_COMPILES("int foo(const char *str, ...) __attribute__((format(printf, 1, 2)));
                         int main() {return 0;}" HAVE_GCC_FORMAT)

CHECK_INCLUDE_FILE(stdbool.h HAVE_STDBOOL_H)
CHECK_INCLUDE_FILE(stdalign.h HAVE_STDALIGN_H)
CHECK_INCLUDE_FILE(malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILE(dirent.h HAVE_DIRENT_H)
CHECK_INCLUDE_FILE(strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILE(cpuid.h HAVE_CPUID_H)
CHECK_INCLUDE_FILE(intrin.h HAVE_INTRIN_H)
CHECK_INCLUDE_FILE(sys/sysconf.h HAVE_SYS_SYSCONF_H)
CHECK_INCLUDE_FILE(fenv.h HAVE_FENV_H)
CHECK_INCLUDE_FILE(float.h HAVE_FLOAT_H)
CHECK_INCLUDE_FILE(ieeefp.h HAVE_IEEEFP_H)
CHECK_INCLUDE_FILE(guiddef.h HAVE_GUIDDEF_H)
IF(NOT HAVE_GUIDDEF_H)
    CHECK_INCLUDE_FILE(initguid.h HAVE_INITGUID_H)
ENDIF()

# Some systems need libm for some of the following math functions to work
SET(MATH_LIB )
CHECK_LIBRARY_EXISTS(m pow "" HAVE_LIBM)
IF(HAVE_LIBM)
    SET(MATH_LIB ${MATH_LIB} m)
    SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} m)
ENDIF()

# Check for the dlopen API (for dynamicly loading backend libs)
IF(ALSOFT_DLOPEN)
    CHECK_LIBRARY_EXISTS(dl dlopen "" HAVE_LIBDL)
    IF(HAVE_LIBDL)
        SET(EXTRA_LIBS dl ${EXTRA_LIBS})
        SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} dl)
    ENDIF()

    CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)
ENDIF()

# Check for a cpuid intrinsic
IF(HAVE_CPUID_H)
    CHECK_C_SOURCE_COMPILES("#include <cpuid.h>
        int main()
        {
            unsigned int eax, ebx, ecx, edx;
            return __get_cpuid(0, &eax, &ebx, &ecx, &edx);
        }" HAVE_GCC_GET_CPUID)
ENDIF()
IF(HAVE_INTRIN_H)
    CHECK_C_SOURCE_COMPILES("#include <intrin.h>
        int main()
        {
            int regs[4];
            __cpuid(regs, 0);
            return regs[0];
        }" HAVE_CPUID_INTRINSIC)
    CHECK_C_SOURCE_COMPILES("#include <intrin.h>
        int main()
        {
            unsigned long idx = 0;
            _BitScanForward64(&idx, 1);
            return idx;
        }" HAVE_BITSCANFORWARD64_INTRINSIC)
    CHECK_C_SOURCE_COMPILES("#include <intrin.h>
        int main()
        {
            unsigned long idx = 0;
            _BitScanForward(&idx, 1);
            return idx;
        }" HAVE_BITSCANFORWARD_INTRINSIC)
ENDIF()

CHECK_SYMBOL_EXISTS(sysconf          unistd.h HAVE_SYSCONF)
CHECK_SYMBOL_EXISTS(aligned_alloc    stdlib.h HAVE_ALIGNED_ALLOC)
CHECK_SYMBOL_EXISTS(posix_memalign   stdlib.h HAVE_POSIX_MEMALIGN)
CHECK_SYMBOL_EXISTS(_aligned_malloc  malloc.h HAVE__ALIGNED_MALLOC)
CHECK_SYMBOL_EXISTS(proc_pidpath     libproc.h HAVE_PROC_PIDPATH)
CHECK_SYMBOL_EXISTS(lrintf math.h HAVE_LRINTF)
CHECK_SYMBOL_EXISTS(modff  math.h HAVE_MODFF)
CHECK_SYMBOL_EXISTS(log2f  math.h HAVE_LOG2F)
CHECK_SYMBOL_EXISTS(cbrtf  math.h HAVE_CBRTF)

IF(HAVE_FLOAT_H)
    CHECK_SYMBOL_EXISTS(_controlfp float.h HAVE__CONTROLFP)
    CHECK_SYMBOL_EXISTS(__control87_2 float.h HAVE___CONTROL87_2)
ENDIF()

CHECK_FUNCTION_EXISTS(stat HAVE_STAT)
CHECK_FUNCTION_EXISTS(strtof HAVE_STRTOF)
CHECK_FUNCTION_EXISTS(strcasecmp HAVE_STRCASECMP)
IF(NOT HAVE_STRCASECMP)
    CHECK_FUNCTION_EXISTS(_stricmp HAVE__STRICMP)
    IF(NOT HAVE__STRICMP)
        MESSAGE(FATAL_ERROR "No case-insensitive compare function found, please report!")
    ENDIF()

    SET(CPP_DEFS ${CPP_DEFS} strcasecmp=_stricmp)
ENDIF()

CHECK_FUNCTION_EXISTS(strncasecmp HAVE_STRNCASECMP)
IF(NOT HAVE_STRNCASECMP)
    CHECK_FUNCTION_EXISTS(_strnicmp HAVE__STRNICMP)
    IF(NOT HAVE__STRNICMP)
        MESSAGE(FATAL_ERROR "No case-insensitive size-limitted compare function found, please report!")
    ENDIF()

    SET(CPP_DEFS ${CPP_DEFS} strncasecmp=_strnicmp)
ENDIF()

CHECK_SYMBOL_EXISTS(strnlen string.h HAVE_STRNLEN)
CHECK_SYMBOL_EXISTS(snprintf stdio.h HAVE_SNPRINTF)
IF(NOT HAVE_SNPRINTF)
    CHECK_FUNCTION_EXISTS(_snprintf HAVE__SNPRINTF)
    IF(NOT HAVE__SNPRINTF)
        MESSAGE(FATAL_ERROR "No snprintf function found, please report!")
    ENDIF()

    SET(CPP_DEFS ${CPP_DEFS} snprintf=_snprintf)
ENDIF()

CHECK_SYMBOL_EXISTS(isfinite math.h HAVE_ISFINITE)
IF(NOT HAVE_ISFINITE)
    CHECK_FUNCTION_EXISTS(finite HAVE_FINITE)
    IF(NOT HAVE_FINITE)
        CHECK_FUNCTION_EXISTS(_finite HAVE__FINITE)
        IF(NOT HAVE__FINITE)
            MESSAGE(FATAL_ERROR "No isfinite function found, please report!")
        ENDIF()
        SET(CPP_DEFS ${CPP_DEFS} isfinite=_finite)
    ELSE()
        SET(CPP_DEFS ${CPP_DEFS} isfinite=finite)
    ENDIF()
ENDIF()

CHECK_SYMBOL_EXISTS(isnan math.h HAVE_ISNAN)
IF(NOT HAVE_ISNAN)
    CHECK_FUNCTION_EXISTS(_isnan HAVE__ISNAN)
    IF(NOT HAVE__ISNAN)
        MESSAGE(FATAL_ERROR "No isnan function found, please report!")
    ENDIF()

    SET(CPP_DEFS ${CPP_DEFS} isnan=_isnan)
ENDIF()


# Check if we have Windows headers
SET(OLD_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS})
SET(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -D_WIN32_WINNT=0x0502)
CHECK_INCLUDE_FILE(windows.h HAVE_WINDOWS_H)
SET(CMAKE_REQUIRED_DEFINITIONS ${OLD_REQUIRED_DEFINITIONS})
UNSET(OLD_REQUIRED_DEFINITIONS)

IF(NOT HAVE_WINDOWS_H)
    CHECK_SYMBOL_EXISTS(gettimeofday sys/time.h HAVE_GETTIMEOFDAY)
    IF(NOT HAVE_GETTIMEOFDAY)
        MESSAGE(FATAL_ERROR "No timing function found!")
    ENDIF()

    CHECK_SYMBOL_EXISTS(nanosleep time.h HAVE_NANOSLEEP)
    IF(NOT HAVE_NANOSLEEP)
        MESSAGE(FATAL_ERROR "No sleep function found!")
    ENDIF()

    # We need pthreads outside of Windows
    CHECK_INCLUDE_FILE(pthread.h HAVE_PTHREAD_H)
    IF(NOT HAVE_PTHREAD_H)
        MESSAGE(FATAL_ERROR "PThreads is required for non-Windows builds!")
    ENDIF()
    # Some systems need pthread_np.h to get recursive mutexes
    CHECK_INCLUDE_FILES("pthread.h;pthread_np.h" HAVE_PTHREAD_NP_H)

    CHECK_C_COMPILER_FLAG(-pthread HAVE_PTHREAD)
    IF(HAVE_PTHREAD)
        SET(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -pthread")
        SET(C_FLAGS ${C_FLAGS} -pthread)
        SET(LINKER_FLAGS ${LINKER_FLAGS} -pthread)
    ENDIF()

    CHECK_LIBRARY_EXISTS(pthread pthread_create "" HAVE_LIBPTHREAD)
    IF(HAVE_LIBPTHREAD)
        SET(EXTRA_LIBS pthread ${EXTRA_LIBS})
    ENDIF()

    CHECK_SYMBOL_EXISTS(pthread_setschedparam pthread.h HAVE_PTHREAD_SETSCHEDPARAM)

    IF(HAVE_PTHREAD_NP_H)
        CHECK_SYMBOL_EXISTS(pthread_setname_np "pthread.h;pthread_np.h" HAVE_PTHREAD_SETNAME_NP)
        IF(NOT HAVE_PTHREAD_SETNAME_NP)
            CHECK_SYMBOL_EXISTS(pthread_set_name_np "pthread.h;pthread_np.h" HAVE_PTHREAD_SET_NAME_NP)
        ELSE()
            CHECK_C_SOURCE_COMPILES("
#include <pthread.h>
#include <pthread_np.h>
int main()
{
    pthread_setname_np(\"testname\");
    return 0;
}"
                PTHREAD_SETNAME_NP_ONE_PARAM
            )
            CHECK_C_SOURCE_COMPILES("
#include <pthread.h>
#include <pthread_np.h>
int main()
{
    pthread_setname_np(pthread_self(), \"%s\", \"testname\");
    return 0;
}"
                PTHREAD_SETNAME_NP_THREE_PARAMS
            )
        ENDIF()
        CHECK_SYMBOL_EXISTS(pthread_mutexattr_setkind_np "pthread.h;pthread_np.h" HAVE_PTHREAD_MUTEXATTR_SETKIND_NP)
    ELSE()
        CHECK_SYMBOL_EXISTS(pthread_setname_np pthread.h HAVE_PTHREAD_SETNAME_NP)
        IF(NOT HAVE_PTHREAD_SETNAME_NP)
            CHECK_SYMBOL_EXISTS(pthread_set_name_np pthread.h HAVE_PTHREAD_SET_NAME_NP)
        ELSE()
            CHECK_C_SOURCE_COMPILES("
#include <pthread.h>
int main()
{
    pthread_setname_np(\"testname\");
    return 0;
}"
                PTHREAD_SETNAME_NP_ONE_PARAM
            )
            CHECK_C_SOURCE_COMPILES("
#include <pthread.h>
int main()
{
    pthread_setname_np(pthread_self(), \"%s\", \"testname\");
    return 0;
}"
                PTHREAD_SETNAME_NP_THREE_PARAMS
            )
        ENDIF()
        CHECK_SYMBOL_EXISTS(pthread_mutexattr_setkind_np pthread.h HAVE_PTHREAD_MUTEXATTR_SETKIND_NP)
    ENDIF()

    CHECK_SYMBOL_EXISTS(pthread_mutex_timedlock pthread.h HAVE_PTHREAD_MUTEX_TIMEDLOCK)

    CHECK_LIBRARY_EXISTS(rt clock_gettime "" HAVE_LIBRT)
    IF(HAVE_LIBRT)
        SET(EXTRA_LIBS rt ${EXTRA_LIBS})
    ENDIF()
ENDIF()

CHECK_SYMBOL_EXISTS(getopt unistd.h HAVE_GETOPT)

# Check for a 64-bit type
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
IF(NOT HAVE_STDINT_H)
    IF(HAVE_WINDOWS_H)
        CHECK_C_SOURCE_COMPILES("#define _WIN32_WINNT 0x0502
                                 #include <windows.h>
                                 __int64 foo;
                                 int main() {return 0;}" HAVE___INT64)
    ENDIF()
    IF(NOT HAVE___INT64)
        IF(NOT SIZEOF_LONG MATCHES "8")
            IF(NOT SIZEOF_LONG_LONG MATCHES "8")
                MESSAGE(FATAL_ERROR "No 64-bit types found, please report!")
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()


SET(COMMON_OBJS
    common/align.h
    common/almalloc.c
    common/almalloc.h
    common/atomic.c
    common/atomic.h
    common/bool.h
    common/math_defs.h
    common/rwlock.c
    common/rwlock.h
    common/static_assert.h
    common/threads.c
    common/threads.h
    common/uintmap.c
    common/uintmap.h
)
SET(OPENAL_OBJS
    OpenAL32/Include/bs2b.h
    OpenAL32/Include/alMain.h
    OpenAL32/Include/alu.h

    OpenAL32/Include/alAuxEffectSlot.h
    OpenAL32/alAuxEffectSlot.c
    OpenAL32/Include/alBuffer.h
    OpenAL32/alBuffer.c
    OpenAL32/Include/alEffect.h
    OpenAL32/alEffect.c
    OpenAL32/Include/alError.h
    OpenAL32/alError.c
    OpenAL32/alExtension.c
    OpenAL32/Include/alFilter.h
    OpenAL32/alFilter.c
    OpenAL32/Include/alListener.h
    OpenAL32/alListener.c
    OpenAL32/Include/alSource.h
    OpenAL32/alSource.c
    OpenAL32/alState.c
    OpenAL32/event.c
    OpenAL32/Include/sample_cvt.h
    OpenAL32/sample_cvt.c
)
SET(ALC_OBJS
    Alc/ALc.c
    Alc/ALu.c
    Alc/alconfig.c
    Alc/alconfig.h
    Alc/bs2b.c
    Alc/converter.c
    Alc/converter.h
    Alc/inprogext.h
    Alc/mastering.c
    Alc/mastering.h
    Alc/ringbuffer.c
    Alc/ringbuffer.h
    Alc/effects/chorus.c
    Alc/effects/compressor.c
    Alc/effects/dedicated.c
    Alc/effects/distortion.c
    Alc/effects/echo.c
    Alc/effects/equalizer.c
    Alc/effects/modulator.c
    Alc/effects/null.c
    Alc/effects/pshifter.c
    Alc/effects/reverb.c
    Alc/filters/defs.h
    Alc/filters/filter.c
    Alc/filters/nfc.c
    Alc/filters/nfc.h
    Alc/filters/splitter.c
    Alc/filters/splitter.h
    Alc/helpers.c
    Alc/alstring.h
    Alc/compat.h
    Alc/cpu_caps.h
    Alc/fpu_modes.h
    Alc/logging.h
    Alc/vector.h
    Alc/hrtf.c
    Alc/hrtf.h
    Alc/uhjfilter.c
    Alc/uhjfilter.h
    Alc/ambdec.c
    Alc/ambdec.h
    Alc/bformatdec.c
    Alc/bformatdec.h
    Alc/panning.c
    Alc/polymorphism.h
    Alc/mixvoice.c
    Alc/mixer/defs.h
    Alc/mixer/mixer_c.c
)


SET(CPU_EXTS "Default")
SET(HAVE_SSE        0)
SET(HAVE_SSE2       0)
SET(HAVE_SSE3       0)
SET(HAVE_SSE4_1     0)
SET(HAVE_NEON       0)

SET(HAVE_ALSA       0)
SET(HAVE_OSS        0)
SET(HAVE_SOLARIS    0)
SET(HAVE_SNDIO      0)
SET(HAVE_QSA        0)
SET(HAVE_DSOUND     0)
SET(HAVE_WASAPI     0)
SET(HAVE_WINMM      0)
SET(HAVE_PORTAUDIO  0)
SET(HAVE_PULSEAUDIO 0)
SET(HAVE_COREAUDIO  0)
SET(HAVE_OPENSL     0)
SET(HAVE_WAVE       0)
SET(HAVE_SDL2       0)

# Check for SSE support
OPTION(ALSOFT_REQUIRE_SSE "Require SSE support" OFF)
CHECK_INCLUDE_FILE(xmmintrin.h HAVE_XMMINTRIN_H "${SSE_SWITCH}")
IF(HAVE_XMMINTRIN_H)
    OPTION(ALSOFT_CPUEXT_SSE "Enable SSE support" ON)
    IF(ALSOFT_CPUEXT_SSE)
        IF(ALIGN_DECL OR HAVE_C11_ALIGNAS)
            SET(HAVE_SSE 1)
            SET(ALC_OBJS  ${ALC_OBJS} Alc/mixer/mixer_sse.c)
            IF(SSE_SWITCH)
                SET_SOURCE_FILES_PROPERTIES(Alc/mixer/mixer_sse.c PROPERTIES
                                            COMPILE_FLAGS "${SSE_SWITCH}")
            ENDIF()
            SET(CPU_EXTS "${CPU_EXTS}, SSE")
        ENDIF()
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_SSE AND NOT HAVE_SSE)
    MESSAGE(FATAL_ERROR "Failed to enabled required SSE CPU extensions")
ENDIF()

OPTION(ALSOFT_REQUIRE_SSE2 "Require SSE2 support" OFF)
CHECK_INCLUDE_FILE(emmintrin.h HAVE_EMMINTRIN_H "${SSE2_SWITCH}")
IF(HAVE_EMMINTRIN_H)
    OPTION(ALSOFT_CPUEXT_SSE2 "Enable SSE2 support" ON)
    IF(HAVE_SSE AND ALSOFT_CPUEXT_SSE2)
        IF(ALIGN_DECL OR HAVE_C11_ALIGNAS)
            SET(HAVE_SSE2 1)
            SET(ALC_OBJS  ${ALC_OBJS} Alc/mixer/mixer_sse2.c)
            IF(SSE2_SWITCH)
                SET_SOURCE_FILES_PROPERTIES(Alc/mixer/mixer_sse2.c PROPERTIES
                                            COMPILE_FLAGS "${SSE2_SWITCH}")
            ENDIF()
            SET(CPU_EXTS "${CPU_EXTS}, SSE2")
        ENDIF()
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_SSE2 AND NOT HAVE_SSE2)
    MESSAGE(FATAL_ERROR "Failed to enable required SSE2 CPU extensions")
ENDIF()

OPTION(ALSOFT_REQUIRE_SSE2 "Require SSE3 support" OFF)
CHECK_INCLUDE_FILE(pmmintrin.h HAVE_PMMINTRIN_H "${SSE3_SWITCH}")
IF(HAVE_EMMINTRIN_H)
    OPTION(ALSOFT_CPUEXT_SSE3 "Enable SSE3 support" ON)
    IF(HAVE_SSE2 AND ALSOFT_CPUEXT_SSE3)
        IF(ALIGN_DECL OR HAVE_C11_ALIGNAS)
            SET(HAVE_SSE3 1)
            SET(ALC_OBJS  ${ALC_OBJS} Alc/mixer/mixer_sse3.c)
            IF(SSE2_SWITCH)
                SET_SOURCE_FILES_PROPERTIES(Alc/mixer/mixer_sse3.c PROPERTIES
                                            COMPILE_FLAGS "${SSE3_SWITCH}")
            ENDIF()
            SET(CPU_EXTS "${CPU_EXTS}, SSE3")
        ENDIF()
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_SSE3 AND NOT HAVE_SSE3)
    MESSAGE(FATAL_ERROR "Failed to enable required SSE3 CPU extensions")
ENDIF()

OPTION(ALSOFT_REQUIRE_SSE4_1 "Require SSE4.1 support" OFF)
CHECK_INCLUDE_FILE(smmintrin.h HAVE_SMMINTRIN_H "${SSE4_1_SWITCH}")
IF(HAVE_SMMINTRIN_H)
    OPTION(ALSOFT_CPUEXT_SSE4_1 "Enable SSE4.1 support" ON)
    IF(HAVE_SSE2 AND ALSOFT_CPUEXT_SSE4_1)
        IF(ALIGN_DECL OR HAVE_C11_ALIGNAS)
            SET(HAVE_SSE4_1 1)
            SET(ALC_OBJS  ${ALC_OBJS} Alc/mixer/mixer_sse41.c)
            IF(SSE4_1_SWITCH)
                SET_SOURCE_FILES_PROPERTIES(Alc/mixer/mixer_sse41.c PROPERTIES
                                            COMPILE_FLAGS "${SSE4_1_SWITCH}")
            ENDIF()
            SET(CPU_EXTS "${CPU_EXTS}, SSE4.1")
        ENDIF()
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_SSE4_1 AND NOT HAVE_SSE4_1)
    MESSAGE(FATAL_ERROR "Failed to enable required SSE4.1 CPU extensions")
ENDIF()

# Check for ARM Neon support
OPTION(ALSOFT_REQUIRE_NEON "Require ARM Neon support" OFF)
CHECK_INCLUDE_FILE(arm_neon.h HAVE_ARM_NEON_H ${FPU_NEON_SWITCH})
IF(HAVE_ARM_NEON_H)
    OPTION(ALSOFT_CPUEXT_NEON "Enable ARM Neon support" ON)
    IF(ALSOFT_CPUEXT_NEON)
        SET(HAVE_NEON 1)
        SET(ALC_OBJS  ${ALC_OBJS} Alc/mixer/mixer_neon.c)
        IF(FPU_NEON_SWITCH)
            SET_SOURCE_FILES_PROPERTIES(Alc/mixer/mixer_neon.c PROPERTIES
                                        COMPILE_FLAGS "${FPU_NEON_SWITCH}")
        ENDIF()
        SET(CPU_EXTS "${CPU_EXTS}, Neon")
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_NEON AND NOT HAVE_NEON)
    MESSAGE(FATAL_ERROR "Failed to enabled required ARM Neon CPU extensions")
ENDIF()


IF(WIN32 OR HAVE_DLFCN_H)
    SET(IS_LINKED "")
    MACRO(ADD_BACKEND_LIBS _LIBS)
    ENDMACRO()
ELSE()
    SET(IS_LINKED " (linked)")
    MACRO(ADD_BACKEND_LIBS _LIBS)
        SET(EXTRA_LIBS ${_LIBS} ${EXTRA_LIBS})
    ENDMACRO()
ENDIF()

SET(BACKENDS "")
SET(ALC_OBJS  ${ALC_OBJS}
    Alc/backends/base.c
    Alc/backends/base.h
    # Default backends, always available
    Alc/backends/loopback.c
    Alc/backends/null.c
)

# Check ALSA backend
OPTION(ALSOFT_REQUIRE_ALSA "Require ALSA backend" OFF)
FIND_PACKAGE(ALSA)
IF(ALSA_FOUND)
    OPTION(ALSOFT_BACKEND_ALSA "Enable ALSA backend" ON)
    IF(ALSOFT_BACKEND_ALSA)
        SET(HAVE_ALSA 1)
        SET(BACKENDS  "${BACKENDS} ALSA${IS_LINKED},")
        SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/alsa.c)
        ADD_BACKEND_LIBS(${ALSA_LIBRARIES})
        SET(INC_PATHS ${INC_PATHS} ${ALSA_INCLUDE_DIRS})
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_ALSA AND NOT HAVE_ALSA)
    MESSAGE(FATAL_ERROR "Failed to enabled required ALSA backend")
ENDIF()

# Check OSS backend
OPTION(ALSOFT_REQUIRE_OSS "Require OSS backend" OFF)
FIND_PACKAGE(OSS)
IF(OSS_FOUND)
    OPTION(ALSOFT_BACKEND_OSS "Enable OSS backend" ON)
    IF(ALSOFT_BACKEND_OSS)
        SET(HAVE_OSS 1)
        SET(BACKENDS  "${BACKENDS} OSS,")
        SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/oss.c)
        IF(OSS_LIBRARIES)
            SET(EXTRA_LIBS ${OSS_LIBRARIES} ${EXTRA_LIBS})
        ENDIF()
        SET(INC_PATHS ${INC_PATHS} ${OSS_INCLUDE_DIRS})
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_OSS AND NOT HAVE_OSS)
    MESSAGE(FATAL_ERROR "Failed to enabled required OSS backend")
ENDIF()

# Check Solaris backend
OPTION(ALSOFT_REQUIRE_SOLARIS "Require Solaris backend" OFF)
FIND_PACKAGE(AudioIO)
IF(AUDIOIO_FOUND)
    OPTION(ALSOFT_BACKEND_SOLARIS "Enable Solaris backend" ON)
    IF(ALSOFT_BACKEND_SOLARIS)
        SET(HAVE_SOLARIS 1)
        SET(BACKENDS  "${BACKENDS} Solaris,")
        SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/solaris.c)
        SET(INC_PATHS ${INC_PATHS} ${AUDIOIO_INCLUDE_DIRS})
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_SOLARIS AND NOT HAVE_SOLARIS)
    MESSAGE(FATAL_ERROR "Failed to enabled required Solaris backend")
ENDIF()

# Check SndIO backend
OPTION(ALSOFT_REQUIRE_SNDIO "Require SndIO backend" OFF)
FIND_PACKAGE(SoundIO)
IF(SOUNDIO_FOUND)
    OPTION(ALSOFT_BACKEND_SNDIO "Enable SndIO backend" ON)
    IF(ALSOFT_BACKEND_SNDIO)
        SET(HAVE_SNDIO 1)
        SET(BACKENDS  "${BACKENDS} SndIO (linked),")
        SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/sndio.c)
        SET(EXTRA_LIBS ${SOUNDIO_LIBRARIES} ${EXTRA_LIBS})
        SET(INC_PATHS ${INC_PATHS} ${SOUNDIO_INCLUDE_DIRS})
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_SNDIO AND NOT HAVE_SNDIO)
    MESSAGE(FATAL_ERROR "Failed to enabled required SndIO backend")
ENDIF()

# Check QSA backend
OPTION(ALSOFT_REQUIRE_QSA "Require QSA backend" OFF)
FIND_PACKAGE(QSA)
IF(QSA_FOUND)
    OPTION(ALSOFT_BACKEND_QSA "Enable QSA backend" ON)
    IF(ALSOFT_BACKEND_QSA)
        SET(HAVE_QSA 1)
        SET(BACKENDS  "${BACKENDS} QSA (linked),")
        SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/qsa.c)
        SET(EXTRA_LIBS ${QSA_LIBRARIES} ${EXTRA_LIBS})
        SET(INC_PATHS ${INC_PATHS} ${QSA_INCLUDE_DIRS})
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_QSA AND NOT HAVE_QSA)
    MESSAGE(FATAL_ERROR "Failed to enabled required QSA backend")
ENDIF()

# Check Windows-only backends
OPTION(ALSOFT_REQUIRE_WINMM "Require Windows Multimedia backend" OFF)
OPTION(ALSOFT_REQUIRE_DSOUND "Require DirectSound backend" OFF)
OPTION(ALSOFT_REQUIRE_WASAPI "Require WASAPI backend" OFF)
IF(HAVE_WINDOWS_H)
    SET(OLD_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS})
    SET(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -D_WIN32_WINNT=0x0502)

    # Check MMSystem backend
    CHECK_INCLUDE_FILES("windows.h;mmsystem.h" HAVE_MMSYSTEM_H)
    IF(HAVE_MMSYSTEM_H)
        CHECK_SHARED_FUNCTION_EXISTS(waveOutOpen "windows.h;mmsystem.h" winmm "" HAVE_LIBWINMM)
        IF(HAVE_LIBWINMM)
            OPTION(ALSOFT_BACKEND_WINMM "Enable Windows Multimedia backend" ON)
            IF(ALSOFT_BACKEND_WINMM)
                SET(HAVE_WINMM 1)
                SET(BACKENDS  "${BACKENDS} WinMM,")
                SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/winmm.c)
                SET(EXTRA_LIBS winmm ${EXTRA_LIBS})
            ENDIF()
        ENDIF()
    ENDIF()

    # Check DSound backend
    FIND_PACKAGE(DSound)
    IF(DSOUND_FOUND)
        OPTION(ALSOFT_BACKEND_DSOUND "Enable DirectSound backend" ON)
        IF(ALSOFT_BACKEND_DSOUND)
            SET(HAVE_DSOUND 1)
            SET(BACKENDS  "${BACKENDS} DirectSound${IS_LINKED},")
            SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/dsound.c)
            ADD_BACKEND_LIBS(${DSOUND_LIBRARIES})
            SET(INC_PATHS ${INC_PATHS} ${DSOUND_INCLUDE_DIRS})
        ENDIF()
    ENDIF()

    # Check for WASAPI backend
    CHECK_INCLUDE_FILE(mmdeviceapi.h HAVE_MMDEVICEAPI_H)
    IF(HAVE_MMDEVICEAPI_H)
        OPTION(ALSOFT_BACKEND_WASAPI "Enable WASAPI backend" ON)
        IF(ALSOFT_BACKEND_WASAPI)
            SET(HAVE_WASAPI 1)
            SET(BACKENDS  "${BACKENDS} WASAPI,")
            SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/wasapi.c)
        ENDIF()
    ENDIF()

    SET(CMAKE_REQUIRED_DEFINITIONS ${OLD_REQUIRED_DEFINITIONS})
    UNSET(OLD_REQUIRED_DEFINITIONS)
ENDIF()
IF(ALSOFT_REQUIRE_WINMM AND NOT HAVE_WINMM)
    MESSAGE(FATAL_ERROR "Failed to enabled required WinMM backend")
ENDIF()
IF(ALSOFT_REQUIRE_DSOUND AND NOT HAVE_DSOUND)
    MESSAGE(FATAL_ERROR "Failed to enabled required DSound backend")
ENDIF()
IF(ALSOFT_REQUIRE_WASAPI AND NOT HAVE_WASAPI)
    MESSAGE(FATAL_ERROR "Failed to enabled required WASAPI backend")
ENDIF()

# Check PortAudio backend
OPTION(ALSOFT_REQUIRE_PORTAUDIO "Require PortAudio backend" OFF)
FIND_PACKAGE(PortAudio)
IF(PORTAUDIO_FOUND)
    OPTION(ALSOFT_BACKEND_PORTAUDIO "Enable PortAudio backend" ON)
    IF(ALSOFT_BACKEND_PORTAUDIO)
        SET(HAVE_PORTAUDIO 1)
        SET(BACKENDS  "${BACKENDS} PortAudio${IS_LINKED},")
        SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/portaudio.c)
        ADD_BACKEND_LIBS(${PORTAUDIO_LIBRARIES})
        SET(INC_PATHS ${INC_PATHS} ${PORTAUDIO_INCLUDE_DIRS})
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_PORTAUDIO AND NOT HAVE_PORTAUDIO)
    MESSAGE(FATAL_ERROR "Failed to enabled required PortAudio backend")
ENDIF()

# Check PulseAudio backend
OPTION(ALSOFT_REQUIRE_PULSEAUDIO "Require PulseAudio backend" OFF)
FIND_PACKAGE(PulseAudio)
IF(PULSEAUDIO_FOUND)
    OPTION(ALSOFT_BACKEND_PULSEAUDIO "Enable PulseAudio backend" ON)
    IF(ALSOFT_BACKEND_PULSEAUDIO)
        SET(HAVE_PULSEAUDIO 1)
        SET(BACKENDS  "${BACKENDS} PulseAudio${IS_LINKED},")
        SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/pulseaudio.c)
        ADD_BACKEND_LIBS(${PULSEAUDIO_LIBRARIES})
        SET(INC_PATHS ${INC_PATHS} ${PULSEAUDIO_INCLUDE_DIRS})
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_PULSEAUDIO AND NOT HAVE_PULSEAUDIO)
    MESSAGE(FATAL_ERROR "Failed to enabled required PulseAudio backend")
ENDIF()

# Check JACK backend
OPTION(ALSOFT_REQUIRE_JACK "Require JACK backend" OFF)
FIND_PACKAGE(JACK)
IF(JACK_FOUND)
    OPTION(ALSOFT_BACKEND_JACK "Enable JACK backend" ON)
    IF(ALSOFT_BACKEND_JACK)
        SET(HAVE_JACK 1)
        SET(BACKENDS  "${BACKENDS} JACK${IS_LINKED},")
        SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/jack.c)
        ADD_BACKEND_LIBS(${JACK_LIBRARIES})
        SET(INC_PATHS ${INC_PATHS} ${JACK_INCLUDE_DIRS})
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_JACK AND NOT HAVE_JACK)
    MESSAGE(FATAL_ERROR "Failed to enabled required JACK backend")
ENDIF()

# Check CoreAudio backend
OPTION(ALSOFT_REQUIRE_COREAUDIO "Require CoreAudio backend" OFF)
FIND_LIBRARY(COREAUDIO_FRAMEWORK
             NAMES CoreAudio
             PATHS /System/Library/Frameworks
)
IF(COREAUDIO_FRAMEWORK)
    OPTION(ALSOFT_BACKEND_COREAUDIO "Enable CoreAudio backend" ON)
    IF(ALSOFT_BACKEND_COREAUDIO)
        SET(HAVE_COREAUDIO 1)
        SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/coreaudio.c)
        SET(BACKENDS  "${BACKENDS} CoreAudio,")
        SET(EXTRA_LIBS ${COREAUDIO_FRAMEWORK} ${EXTRA_LIBS})
        SET(EXTRA_LIBS /System/Library/Frameworks/AudioUnit.framework ${EXTRA_LIBS})
        SET(EXTRA_LIBS /System/Library/Frameworks/ApplicationServices.framework ${EXTRA_LIBS})

        # Some versions of OSX may need the AudioToolbox framework. Add it if
        # it's found.
        FIND_LIBRARY(AUDIOTOOLBOX_LIBRARY
                     NAMES AudioToolbox
                     PATHS ~/Library/Frameworks
                           /Library/Frameworks
                           /System/Library/Frameworks
                    )
        IF(AUDIOTOOLBOX_LIBRARY)
            SET(EXTRA_LIBS ${AUDIOTOOLBOX_LIBRARY} ${EXTRA_LIBS})
        ENDIF()
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_COREAUDIO AND NOT HAVE_COREAUDIO)
    MESSAGE(FATAL_ERROR "Failed to enabled required CoreAudio backend")
ENDIF()

# Check for OpenSL (Android) backend
OPTION(ALSOFT_REQUIRE_OPENSL "Require OpenSL backend" OFF)
CHECK_INCLUDE_FILES("SLES/OpenSLES.h;SLES/OpenSLES_Android.h" HAVE_SLES_OPENSLES_ANDROID_H)
IF(HAVE_SLES_OPENSLES_ANDROID_H)
    CHECK_SHARED_FUNCTION_EXISTS(slCreateEngine "SLES/OpenSLES.h" OpenSLES "" HAVE_LIBOPENSLES)
    IF(HAVE_LIBOPENSLES)
        OPTION(ALSOFT_BACKEND_OPENSL "Enable OpenSL backend" ON)
        IF(ALSOFT_BACKEND_OPENSL)
            SET(HAVE_OPENSL 1)
            SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/opensl.c)
            SET(BACKENDS  "${BACKENDS} OpenSL,")
            SET(EXTRA_LIBS OpenSLES ${EXTRA_LIBS})
        ENDIF()
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_OPENSL AND NOT HAVE_OPENSL)
    MESSAGE(FATAL_ERROR "Failed to enabled required OpenSL backend")
ENDIF()

# Check for SDL2 backend
OPTION(ALSOFT_REQUIRE_SDL2 "Require SDL2 backend" OFF)
FIND_PACKAGE(SDL2)
IF(SDL2_FOUND)
    # Off by default, since it adds a runtime dependency
    OPTION(ALSOFT_BACKEND_SDL2 "Enable SDL2 backend" OFF)
    IF(ALSOFT_BACKEND_SDL2)
        SET(HAVE_SDL2 1)
        SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/sdl2.c)
        SET(BACKENDS  "${BACKENDS} SDL2,")
        SET(EXTRA_LIBS ${SDL2_LIBRARY} ${EXTRA_LIBS})
        SET(INC_PATHS ${INC_PATHS} ${SDL2_INCLUDE_DIR})
    ENDIF()
ENDIF()
IF(ALSOFT_REQUIRE_SDL2 AND NOT SDL2_FOUND)
    MESSAGE(FATAL_ERROR "Failed to enabled required SDL2 backend")
ENDIF()

# Optionally enable the Wave Writer backend
OPTION(ALSOFT_BACKEND_WAVE "Enable Wave Writer backend" ON)
IF(ALSOFT_BACKEND_WAVE)
    SET(HAVE_WAVE 1)
    SET(ALC_OBJS  ${ALC_OBJS} Alc/backends/wave.c)
    SET(BACKENDS  "${BACKENDS} WaveFile,")
ENDIF()

# This is always available
SET(BACKENDS  "${BACKENDS} Null")


FIND_PACKAGE(Git)
IF(GIT_FOUND AND EXISTS "${OpenAL_SOURCE_DIR}/.git")
    # Get the current working branch and its latest abbreviated commit hash
    ADD_CUSTOM_TARGET(build_version
        ${CMAKE_COMMAND} -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
                         -D LIB_VERSION=${LIB_VERSION}
                         -D SRC=${OpenAL_SOURCE_DIR}/version.h.in
                         -D DST=${OpenAL_BINARY_DIR}/version.h
                         -P ${OpenAL_SOURCE_DIR}/version.cmake
        WORKING_DIRECTORY "${OpenAL_SOURCE_DIR}"
        VERBATIM
    )
ELSE()
    SET(GIT_BRANCH "UNKNOWN")
    SET(GIT_COMMIT_HASH "unknown")
    CONFIGURE_FILE(
        "${OpenAL_SOURCE_DIR}/version.h.in"
        "${OpenAL_BINARY_DIR}/version.h")
ENDIF()

SET(NATIVE_SRC_DIR  "${OpenAL_SOURCE_DIR}/native-tools/")
SET(NATIVE_BIN_DIR  "${OpenAL_BINARY_DIR}/native-tools/")
FILE(MAKE_DIRECTORY "${NATIVE_BIN_DIR}")

SET(BIN2H_COMMAND  "${NATIVE_BIN_DIR}bin2h")
SET(BSINCGEN_COMMAND  "${NATIVE_BIN_DIR}bsincgen")
ADD_CUSTOM_COMMAND(OUTPUT "${BIN2H_COMMAND}" "${BSINCGEN_COMMAND}"
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" "${NATIVE_SRC_DIR}"
    COMMAND ${CMAKE_COMMAND} -E remove "${BIN2H_COMMAND}" "${BSINCGEN_COMMAND}"
    COMMAND ${CMAKE_COMMAND} --build . --config "Release"
    WORKING_DIRECTORY "${NATIVE_BIN_DIR}"
    DEPENDS "${NATIVE_SRC_DIR}CMakeLists.txt"
    IMPLICIT_DEPENDS C "${NATIVE_SRC_DIR}bin2h.c"
                     C "${NATIVE_SRC_DIR}bsincgen.c"
    VERBATIM
)
ADD_CUSTOM_TARGET(native-tools
    DEPENDS "${BIN2H_COMMAND}" "${BSINCGEN_COMMAND}"
    VERBATIM
)

option(ALSOFT_EMBED_HRTF_DATA "Embed the HRTF data files (increases library footprint)" OFF)
if(ALSOFT_EMBED_HRTF_DATA)
    MACRO(make_hrtf_header FILENAME VARNAME)
        SET(infile  "${OpenAL_SOURCE_DIR}/hrtf/${FILENAME}")
        SET(outfile  "${OpenAL_BINARY_DIR}/${FILENAME}.h")

        ADD_CUSTOM_COMMAND(OUTPUT "${outfile}"
            COMMAND "${BIN2H_COMMAND}" "${infile}" "${outfile}" ${VARNAME}
            DEPENDS native-tools "${infile}"
            VERBATIM
        )
        SET(ALC_OBJS  ${ALC_OBJS} "${outfile}")
    ENDMACRO()

    make_hrtf_header(default-44100.mhr "hrtf_default_44100")
    make_hrtf_header(default-48000.mhr "hrtf_default_48000")
endif()

ADD_CUSTOM_COMMAND(OUTPUT "${OpenAL_BINARY_DIR}/bsinc_inc.h"
    COMMAND "${BSINCGEN_COMMAND}" "${OpenAL_BINARY_DIR}/bsinc_inc.h"
    DEPENDS native-tools "${NATIVE_SRC_DIR}bsincgen.c"
    VERBATIM
)
SET(ALC_OBJS  ${ALC_OBJS} "${OpenAL_BINARY_DIR}/bsinc_inc.h")


IF(ALSOFT_UTILS AND NOT ALSOFT_NO_CONFIG_UTIL)
    add_subdirectory(utils/alsoft-config)
ENDIF()
IF(ALSOFT_EXAMPLES)
    IF(NOT SDL2_FOUND)
        FIND_PACKAGE(SDL2)
    ENDIF()
    IF(SDL2_FOUND)
        FIND_PACKAGE(SDL_sound)
        FIND_PACKAGE(FFmpeg COMPONENTS AVFORMAT AVCODEC AVUTIL SWSCALE SWRESAMPLE)
    ENDIF()
ENDIF()

IF(NOT WIN32)
    SET(LIBNAME "openal")
ELSE()
    SET(LIBNAME "OpenAL32")
ENDIF()

# Needed for openal.pc.in
SET(prefix ${CMAKE_INSTALL_PREFIX})
SET(exec_prefix "\${prefix}")
SET(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
SET(bindir "\${exec_prefix}/${CMAKE_INSTALL_BINDIR}")
SET(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
SET(PACKAGE_VERSION "${LIB_VERSION}")
SET(PKG_CONFIG_CFLAGS )
SET(PKG_CONFIG_PRIVATE_LIBS )
IF(LIBTYPE STREQUAL "STATIC")
    SET(PKG_CONFIG_CFLAGS -DAL_LIBTYPE_STATIC)
    FOREACH(FLAG  ${LINKER_FLAGS} ${EXTRA_LIBS} ${MATH_LIB})
        # If this is already a linker flag, or is a full path+file, add it
        # as-is. Otherwise, it's a name intended to be dressed as -lname.
        IF(FLAG MATCHES "^-.*" OR EXISTS "${FLAG}")
            SET(PKG_CONFIG_PRIVATE_LIBS "${PKG_CONFIG_PRIVATE_LIBS} ${FLAG}")
        ELSE()
            SET(PKG_CONFIG_PRIVATE_LIBS "${PKG_CONFIG_PRIVATE_LIBS} -l${FLAG}")
        ENDIF()
    ENDFOREACH()
ENDIF()

# End configuration
CONFIGURE_FILE(
    "${OpenAL_SOURCE_DIR}/config.h.in"
    "${OpenAL_BINARY_DIR}/config.h")
CONFIGURE_FILE(
    "${OpenAL_SOURCE_DIR}/openal.pc.in"
    "${OpenAL_BINARY_DIR}/openal.pc"
    @ONLY)


# Add a static library with common functions used by multiple targets
ADD_LIBRARY(common STATIC ${COMMON_OBJS})
TARGET_COMPILE_DEFINITIONS(common PRIVATE ${CPP_DEFS})
TARGET_COMPILE_OPTIONS(common PRIVATE ${C_FLAGS})


UNSET(HAS_ROUTER)
SET(IMPL_TARGET OpenAL)
SET(COMMON_LIB )
SET(SUBSYS_FLAG )

# Build main library
IF(LIBTYPE STREQUAL "STATIC")
    SET(CPP_DEFS ${CPP_DEFS} AL_LIBTYPE_STATIC)
    IF(WIN32 AND ALSOFT_NO_UID_DEFS)
        SET(CPP_DEFS ${CPP_DEFS} AL_NO_UID_DEFS)
    ENDIF()
    ADD_LIBRARY(OpenAL STATIC ${COMMON_OBJS} ${OPENAL_OBJS} ${ALC_OBJS})
ELSE()
    # Make sure to compile the common code with PIC, since it'll be linked into
    # shared libs that needs it.
    SET_PROPERTY(TARGET common PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    SET(COMMON_LIB common)

    IF(WIN32)
        IF(MSVC)
            SET(SUBSYS_FLAG ${SUBSYS_FLAG} "/SUBSYSTEM:WINDOWS")
        ELSEIF(CMAKE_COMPILER_IS_GNUCC)
            SET(SUBSYS_FLAG ${SUBSYS_FLAG} "-mwindows")
        ENDIF()
    ENDIF()

    IF(WIN32 AND ALSOFT_BUILD_ROUTER)
        ADD_LIBRARY(OpenAL SHARED router/router.c router/router.h router/alc.c router/al.c)
        TARGET_COMPILE_DEFINITIONS(OpenAL
            PRIVATE AL_BUILD_LIBRARY AL_ALEXT_PROTOTYPES ${CPP_DEFS})
        TARGET_COMPILE_OPTIONS(OpenAL PRIVATE ${C_FLAGS})
        TARGET_LINK_LIBRARIES(OpenAL PRIVATE ${LINKER_FLAGS} ${COMMON_LIB})
        SET_TARGET_PROPERTIES(OpenAL PROPERTIES PREFIX "")
        SET_TARGET_PROPERTIES(OpenAL PROPERTIES OUTPUT_NAME ${LIBNAME})
        IF(TARGET build_version)
            ADD_DEPENDENCIES(OpenAL build_version)
        ENDIF()
        SET(HAS_ROUTER 1)

        SET(LIBNAME "soft_oal")
        SET(IMPL_TARGET soft_oal)
    ENDIF()

    ADD_LIBRARY(${IMPL_TARGET} SHARED ${OPENAL_OBJS} ${ALC_OBJS})
    IF(WIN32)
        SET_TARGET_PROPERTIES(${IMPL_TARGET} PROPERTIES PREFIX "")
    ENDIF()
ENDIF()
SET_TARGET_PROPERTIES(${IMPL_TARGET} PROPERTIES OUTPUT_NAME ${LIBNAME}
    VERSION ${LIB_VERSION}
    SOVERSION ${LIB_MAJOR_VERSION}
)
TARGET_COMPILE_DEFINITIONS(${IMPL_TARGET}
    PRIVATE AL_BUILD_LIBRARY AL_ALEXT_PROTOTYPES ${CPP_DEFS})
TARGET_INCLUDE_DIRECTORIES(${IMPL_TARGET}
    PRIVATE "${OpenAL_SOURCE_DIR}/OpenAL32/Include" "${OpenAL_SOURCE_DIR}/Alc" ${INC_PATHS})
TARGET_COMPILE_OPTIONS(${IMPL_TARGET} PRIVATE ${C_FLAGS})
TARGET_LINK_LIBRARIES(${IMPL_TARGET}
    PRIVATE ${LINKER_FLAGS} ${COMMON_LIB} ${EXTRA_LIBS} ${MATH_LIB})
IF(TARGET build_version)
    ADD_DEPENDENCIES(${IMPL_TARGET} build_version)
ENDIF()

IF(WIN32 AND MINGW AND ALSOFT_BUILD_IMPORT_LIB AND NOT LIBTYPE STREQUAL "STATIC")
    FIND_PROGRAM(SED_EXECUTABLE NAMES sed DOC "sed executable")
    FIND_PROGRAM(DLLTOOL_EXECUTABLE NAMES "${DLLTOOL}" DOC "dlltool executable")
    IF(NOT SED_EXECUTABLE OR NOT DLLTOOL_EXECUTABLE)
        MESSAGE(STATUS "")
        IF(NOT SED_EXECUTABLE)
            MESSAGE(STATUS "WARNING: Cannot find sed, disabling .def/.lib generation")
        ENDIF()
        IF(NOT DLLTOOL_EXECUTABLE)
            MESSAGE(STATUS "WARNING: Cannot find dlltool, disabling .def/.lib generation")
        ENDIF()
    ELSE()
        SET_PROPERTY(TARGET OpenAL APPEND_STRING PROPERTY LINK_FLAGS
                        " -Wl,--output-def,OpenAL32.def")
        ADD_CUSTOM_COMMAND(TARGET OpenAL POST_BUILD
            COMMAND "${SED_EXECUTABLE}" -i -e "s/ @[^ ]*//" OpenAL32.def
            COMMAND "${DLLTOOL_EXECUTABLE}" -d OpenAL32.def -l OpenAL32.lib -D OpenAL32.dll
            COMMENT "Stripping ordinals from OpenAL32.def and generating OpenAL32.lib..."
            VERBATIM
        )
    ENDIF()
ENDIF()

IF(ALSOFT_INSTALL)
    INSTALL(TARGETS OpenAL EXPORT OpenAL
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} ${CMAKE_INSTALL_INCLUDEDIR}/AL
    )
    EXPORT(TARGETS OpenAL
           NAMESPACE OpenAL::
           FILE OpenALConfig.cmake)
    INSTALL(EXPORT OpenAL
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OpenAL
            NAMESPACE OpenAL::
            FILE OpenALConfig.cmake)
    INSTALL(FILES include/AL/al.h
                  include/AL/alc.h
                  include/AL/alext.h
                  include/AL/efx.h
                  include/AL/efx-creative.h
                  include/AL/efx-presets.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/AL
    )
    INSTALL(FILES "${OpenAL_BINARY_DIR}/openal.pc"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
    IF(TARGET soft_oal)
        INSTALL(TARGETS soft_oal
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    ENDIF()
ENDIF()


if(HAS_ROUTER)
    message(STATUS "")
    message(STATUS "Building DLL router")
endif()

MESSAGE(STATUS "")
MESSAGE(STATUS "Building OpenAL with support for the following backends:")
MESSAGE(STATUS "    ${BACKENDS}")
MESSAGE(STATUS "")
MESSAGE(STATUS "Building with support for CPU extensions:")
MESSAGE(STATUS "    ${CPU_EXTS}")
MESSAGE(STATUS "")

IF(WIN32)
    IF(NOT HAVE_DSOUND)
        MESSAGE(STATUS "WARNING: Building the Windows version without DirectSound output")
        MESSAGE(STATUS "         This is probably NOT what you want!")
        MESSAGE(STATUS "")
    ENDIF()
ENDIF()

if(ALSOFT_EMBED_HRTF_DATA)
    message(STATUS "Embedding HRTF datasets")
    message(STATUS "")
endif()

# Install alsoft.conf configuration file
IF(ALSOFT_CONFIG)
    INSTALL(FILES alsoftrc.sample
            DESTINATION ${CMAKE_INSTALL_DATADIR}/openal
    )
    MESSAGE(STATUS "Installing sample configuration")
    MESSAGE(STATUS "")
ENDIF()

# Install HRTF definitions
IF(ALSOFT_HRTF_DEFS)
    INSTALL(FILES hrtf/default-44100.mhr
                  hrtf/default-48000.mhr
            DESTINATION ${CMAKE_INSTALL_DATADIR}/openal/hrtf
    )
    MESSAGE(STATUS "Installing HRTF definitions")
    MESSAGE(STATUS "")
ENDIF()

# Install AmbDec presets
IF(ALSOFT_AMBDEC_PRESETS)
    INSTALL(FILES presets/3D7.1.ambdec
                  presets/hexagon.ambdec
                  presets/itu5.1.ambdec
                  presets/itu5.1-nocenter.ambdec
                  presets/rectangle.ambdec
                  presets/square.ambdec
                  presets/presets.txt
            DESTINATION ${CMAKE_INSTALL_DATADIR}/openal/presets
    )
    MESSAGE(STATUS "Installing AmbDec presets")
    MESSAGE(STATUS "")
ENDIF()

IF(ALSOFT_UTILS)
    ADD_EXECUTABLE(openal-info utils/openal-info.c)
    TARGET_COMPILE_OPTIONS(openal-info PRIVATE ${C_FLAGS})
    TARGET_LINK_LIBRARIES(openal-info PRIVATE ${LINKER_FLAGS} OpenAL)

    SET(MAKEHRTF_SRCS  utils/makehrtf.c)
    IF(NOT HAVE_GETOPT)
        SET(MAKEHRTF_SRCS  ${MAKEHRTF_SRCS} utils/getopt.c utils/getopt.h)
    ENDIF()
    ADD_EXECUTABLE(makehrtf ${MAKEHRTF_SRCS})
    TARGET_COMPILE_DEFINITIONS(makehrtf PRIVATE ${CPP_DEFS})
    TARGET_COMPILE_OPTIONS(makehrtf PRIVATE ${C_FLAGS})
    IF(HAVE_LIBM)
        TARGET_LINK_LIBRARIES(makehrtf PRIVATE ${LINKER_FLAGS} m)
    ENDIF()

    IF(ALSOFT_INSTALL)
        INSTALL(TARGETS openal-info makehrtf
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    ENDIF()

    MESSAGE(STATUS "Building utility programs")
    IF(TARGET alsoft-config)
        MESSAGE(STATUS "Building configuration program")
    ENDIF()
    MESSAGE(STATUS "")
ENDIF()

IF(ALSOFT_TESTS)
    SET(TEST_COMMON_OBJS  examples/common/alhelpers.c)

    ADD_EXECUTABLE(altonegen examples/altonegen.c ${TEST_COMMON_OBJS})
    TARGET_COMPILE_DEFINITIONS(altonegen PRIVATE ${CPP_DEFS})
    TARGET_COMPILE_OPTIONS(altonegen PRIVATE ${C_FLAGS})
    TARGET_LINK_LIBRARIES(altonegen PRIVATE ${LINKER_FLAGS} common OpenAL ${MATH_LIB})

    IF(ALSOFT_INSTALL)
        INSTALL(TARGETS altonegen
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    ENDIF()

    MESSAGE(STATUS "Building test programs")
    MESSAGE(STATUS "")
ENDIF()

IF(ALSOFT_EXAMPLES)
    ADD_EXECUTABLE(alrecord examples/alrecord.c)
    TARGET_COMPILE_DEFINITIONS(alrecord PRIVATE ${CPP_DEFS})
    TARGET_COMPILE_OPTIONS(alrecord PRIVATE ${C_FLAGS})
    TARGET_LINK_LIBRARIES(alrecord PRIVATE ${LINKER_FLAGS} common OpenAL)

    IF(ALSOFT_INSTALL)
        INSTALL(TARGETS alrecord
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    ENDIF()

    MESSAGE(STATUS "Building example programs")

    IF(SDL2_FOUND)
        IF(SDL_SOUND_FOUND)
            # Add a static library with common functions used by multiple targets
            ADD_LIBRARY(ex-common STATIC examples/common/alhelpers.c)
            TARGET_COMPILE_DEFINITIONS(ex-common PRIVATE ${CPP_DEFS})
            TARGET_COMPILE_OPTIONS(ex-common PRIVATE ${C_FLAGS})

            ADD_EXECUTABLE(alplay examples/alplay.c)
            TARGET_COMPILE_DEFINITIONS(alplay PRIVATE ${CPP_DEFS})
            TARGET_INCLUDE_DIRECTORIES(alplay
                PRIVATE ${SDL2_INCLUDE_DIR} ${SDL_SOUND_INCLUDE_DIR})
            TARGET_COMPILE_OPTIONS(alplay PRIVATE ${C_FLAGS})
            TARGET_LINK_LIBRARIES(alplay
                PRIVATE ${LINKER_FLAGS} ${SDL_SOUND_LIBRARIES} ${SDL2_LIBRARY} ex-common common
                        OpenAL)

            ADD_EXECUTABLE(alstream examples/alstream.c)
            TARGET_COMPILE_DEFINITIONS(alstream PRIVATE ${CPP_DEFS})
            TARGET_INCLUDE_DIRECTORIES(alstream
                PRIVATE ${SDL2_INCLUDE_DIR} ${SDL_SOUND_INCLUDE_DIR})
            TARGET_COMPILE_OPTIONS(alstream PRIVATE ${C_FLAGS})
            TARGET_LINK_LIBRARIES(alstream
                PRIVATE ${LINKER_FLAGS} ${SDL_SOUND_LIBRARIES} ${SDL2_LIBRARY} ex-common common
                        OpenAL)

            ADD_EXECUTABLE(alreverb examples/alreverb.c)
            TARGET_COMPILE_DEFINITIONS(alreverb PRIVATE ${CPP_DEFS})
            TARGET_INCLUDE_DIRECTORIES(alreverb
                PRIVATE ${SDL2_INCLUDE_DIR} ${SDL_SOUND_INCLUDE_DIR})
            TARGET_COMPILE_OPTIONS(alreverb PRIVATE ${C_FLAGS})
            TARGET_LINK_LIBRARIES(alreverb
                PRIVATE ${LINKER_FLAGS} ${SDL_SOUND_LIBRARIES} ${SDL2_LIBRARY} ex-common common
                        OpenAL)

            ADD_EXECUTABLE(almultireverb examples/almultireverb.c)
            TARGET_COMPILE_DEFINITIONS(almultireverb PRIVATE ${CPP_DEFS})
            TARGET_INCLUDE_DIRECTORIES(almultireverb
                PRIVATE ${SDL2_INCLUDE_DIR} ${SDL_SOUND_INCLUDE_DIR})
            TARGET_COMPILE_OPTIONS(almultireverb PRIVATE ${C_FLAGS})
            TARGET_LINK_LIBRARIES(almultireverb
                PRIVATE ${LINKER_FLAGS} ${SDL_SOUND_LIBRARIES} ${SDL2_LIBRARY} ex-common common
                        OpenAL ${MATH_LIB})

            ADD_EXECUTABLE(allatency examples/allatency.c)
            TARGET_COMPILE_DEFINITIONS(allatency PRIVATE ${CPP_DEFS})
            TARGET_INCLUDE_DIRECTORIES(allatency
                PRIVATE ${SDL2_INCLUDE_DIR} ${SDL_SOUND_INCLUDE_DIR})
            TARGET_COMPILE_OPTIONS(allatency PRIVATE ${C_FLAGS})
            TARGET_LINK_LIBRARIES(allatency
                PRIVATE ${LINKER_FLAGS} ${SDL_SOUND_LIBRARIES} ${SDL2_LIBRARY} ex-common common
                        OpenAL)

            ADD_EXECUTABLE(alloopback examples/alloopback.c)
            TARGET_COMPILE_DEFINITIONS(alloopback PRIVATE ${CPP_DEFS})
            TARGET_INCLUDE_DIRECTORIES(alloopback
                PRIVATE ${SDL2_INCLUDE_DIR} ${SDL_SOUND_INCLUDE_DIR})
            TARGET_COMPILE_OPTIONS(alloopback PRIVATE ${C_FLAGS})
            TARGET_LINK_LIBRARIES(alloopback
                PRIVATE ${LINKER_FLAGS} ${SDL_SOUND_LIBRARIES} ${SDL2_LIBRARY} ex-common common
                        OpenAL ${MATH_LIB})

            ADD_EXECUTABLE(alhrtf examples/alhrtf.c)
            TARGET_COMPILE_DEFINITIONS(alhrtf PRIVATE ${CPP_DEFS})
            TARGET_INCLUDE_DIRECTORIES(alhrtf
                PRIVATE ${SDL2_INCLUDE_DIR} ${SDL_SOUND_INCLUDE_DIR})
            TARGET_COMPILE_OPTIONS(alhrtf PRIVATE ${C_FLAGS})
            TARGET_LINK_LIBRARIES(alhrtf
                PRIVATE ${LINKER_FLAGS} ${SDL_SOUND_LIBRARIES} ${SDL2_LIBRARY} ex-common common
                        OpenAL ${MATH_LIB})

            IF(ALSOFT_INSTALL)
                INSTALL(TARGETS alplay alstream alreverb almultireverb allatency alloopback alhrtf
                        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                )
            ENDIF()

            MESSAGE(STATUS "Building SDL_sound example programs")
        ENDIF()

        SET(FFVER_OK FALSE)
        IF(FFMPEG_FOUND)
            SET(FFVER_OK TRUE)
            IF(AVFORMAT_VERSION VERSION_LESS "57.56.101")
                MESSAGE(STATUS "libavformat is too old! (${AVFORMAT_VERSION}, wanted 57.56.101)")
                SET(FFVER_OK FALSE)
            ENDIF()
            IF(AVCODEC_VERSION VERSION_LESS "57.64.101")
                MESSAGE(STATUS "libavcodec is too old! (${AVCODEC_VERSION}, wanted 57.64.101)")
                SET(FFVER_OK FALSE)
            ENDIF()
            IF(AVUTIL_VERSION VERSION_LESS "55.34.101")
                MESSAGE(STATUS "libavutil is too old! (${AVUTIL_VERSION}, wanted 55.34.101)")
                SET(FFVER_OK FALSE)
            ENDIF()
            IF(SWSCALE_VERSION VERSION_LESS "4.2.100")
                MESSAGE(STATUS "libswscale is too old! (${SWSCALE_VERSION}, wanted 4.2.100)")
                SET(FFVER_OK FALSE)
            ENDIF()
            IF(SWRESAMPLE_VERSION VERSION_LESS "2.3.100")
                MESSAGE(STATUS "libswresample is too old! (${SWRESAMPLE_VERSION}, wanted 2.3.100)")
                SET(FFVER_OK FALSE)
            ENDIF()
        ENDIF()
        IF(FFVER_OK)
            ADD_EXECUTABLE(alffplay examples/alffplay.cpp)
            TARGET_COMPILE_DEFINITIONS(alffplay PRIVATE ${CPP_DEFS})
            TARGET_INCLUDE_DIRECTORIES(alffplay
                PRIVATE ${SDL2_INCLUDE_DIR} ${FFMPEG_INCLUDE_DIRS})
            TARGET_COMPILE_OPTIONS(alffplay PRIVATE ${C_FLAGS})
            TARGET_LINK_LIBRARIES(alffplay
                PRIVATE ${LINKER_FLAGS} ${SDL2_LIBRARY} ${FFMPEG_LIBRARIES} common OpenAL)

            IF(ALSOFT_INSTALL)
                INSTALL(TARGETS alffplay
                        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                )
            ENDIF()
            MESSAGE(STATUS "Building SDL+FFmpeg example programs")
        ENDIF()
        MESSAGE(STATUS "")
    ENDIF()
ENDIF()
