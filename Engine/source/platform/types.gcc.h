//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _TYPESGCC_H
#define _TYPESGCC_H


// For additional information on GCC predefined macros
// http://gcc.gnu.org/onlinedocs/gcc-3.0.2/cpp.html


//--------------------------------------
// Types
#if defined(TORQUE_X86)
typedef signed long long    S64;
typedef unsigned long long  U64;
#else
typedef signed long    S64;
typedef unsigned long  U64;
#endif


//--------------------------------------
// Compiler Version
#define TORQUE_COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)


//--------------------------------------
// Identify the compiler string

#if defined(__MINGW32__)
#  define TORQUE_COMPILER_STRING "GCC (MinGW)"
#  define TORQUE_COMPILER_MINGW
#elif defined(__CYGWIN__)
#  define TORQUE_COMPILER_STRING "GCC (Cygwin)"
#  define TORQUE_COMPILER_MINGW
#else
#  define TORQUE_COMPILER_STRING "GCC "
#endif


//--------------------------------------
// Identify the Operating System
#if defined(_WIN64)
#  define TORQUE_OS_STRING "Win64"
#  define TORQUE_OS_WIN
#  define TORQUE_OS_WIN64
#  include "platform/types.win.h"
#elif defined(__WIN32__) || defined(_WIN32)
#  define TORQUE_OS_STRING "Win32"
#  define TORQUE_OS_WIN
#  define TORQUE_OS_WIN32
#  define TORQUE_SUPPORTS_NASM
#  define TORQUE_SUPPORTS_GCC_INLINE_X86_ASM
#  include "platform/types.win.h"

#elif defined(SN_TARGET_PS3)
#  define TORQUE_OS_STRING "PS3"
#  define TORQUE_OS_PS3
#  include "platform/types.posix.h"

#elif defined(linux) || defined(LINUX)
#  define TORQUE_OS_STRING "Linux"
#  define TORQUE_OS_LINUX
//#  define TORQUE_SUPPORTS_NASM
//#  define TORQUE_SUPPORTS_GCC_INLINE_X86_ASM
#  include "platform/types.posix.h"

#elif defined(__OpenBSD__)
#  define TORQUE_OS_STRING "OpenBSD"
#  define TORQUE_OS_OPENBSD
#  define TORQUE_SUPPORTS_NASM
#  define TORQUE_SUPPORTS_GCC_INLINE_X86_ASM
#  include "platform/types.posix.h"

#elif defined(__FreeBSD__)
#  define TORQUE_OS_STRING "FreeBSD"
#  define TORQUE_OS_FREEBSD
#  define TORQUE_SUPPORTS_NASM
#  define TORQUE_SUPPORTS_GCC_INLINE_X86_ASM
#  include "platform/types.posix.h"

#elif defined(__APPLE__)
#  define TORQUE_OS_STRING "MacOS X"
#  define TORQUE_OS_MAC
#  include "platform/types.mac.h"
#  if defined(i386)
// Disabling ASM on XCode for shared library build code relocation issues
// This could be reconfigured for static builds, though minimal impact
//#     define TORQUE_SUPPORTS_NASM
#  endif
#else 
#  error "GCC: Unsupported Operating System"
#endif


//--------------------------------------
// Identify the CPU
#if defined(i386)
#  define TORQUE_CPU_STRING "Intel x86"
#  define TORQUE_CPU_X86
#  define TORQUE_LITTLE_ENDIAN

#elif defined(__x86_64__)
#  define TORQUE_CPU_STRING "Intel x64"
#  define TORQUE_CPU_X64
#  define TORQUE_LITTLE_ENDIAN

#elif defined(__ppc__)
#  define TORQUE_CPU_STRING "PowerPC"
#  define TORQUE_CPU_PPC
#  define TORQUE_BIG_ENDIAN

#elif defined(SN_TARGET_PS3)
#  define TORQUE_CPU_STRING "PowerPC"
#  define TORQUE_CPU_PPC
#  define TORQUE_BIG_ENDIAN

#else
#  error "GCC: Unsupported Target CPU"
#endif

#ifndef Offset
/// Offset macro:
/// Calculates the location in memory of a given member x of class cls from the
/// start of the class.  Need several definitions to account for various
/// flavors of GCC.

// now, for each compiler type, define the Offset macros that should be used.
// The Engine code usually uses the Offset macro, but OffsetNonConst is needed
// when a variable is used in the indexing of the member field (see
// TSShapeConstructor::initPersistFields for an example)

// compiler is non-GCC, or gcc < 3
#if (__GNUC__ < 3)
#define Offset(x, cls) _Offset_Normal(x, cls)
#define OffsetNonConst(x, cls) _Offset_Normal(x, cls)

// compiler is GCC 3 with minor version less than 4
#elif defined(TORQUE_COMPILER_GCC) && (__GNUC__ == 3) && (__GNUC_MINOR__ < 4)
#define Offset(x, cls) _Offset_Variant_1(x, cls)
#define OffsetNonConst(x, cls) _Offset_Variant_1(x, cls)

// compiler is GCC 3 with minor version greater than 4
#elif defined(TORQUE_COMPILER_GCC) && (__GNUC__ == 3) && (__GNUC_MINOR__ >= 4)
#include <stddef.h>
#define Offset(x, cls) _Offset_Variant_2(x, cls)
#define OffsetNonConst(x, cls) _Offset_Variant_1(x, cls)

// compiler is GCC 4
#elif defined(TORQUE_COMPILER_GCC) && (__GNUC__ == 4)
#include <stddef.h>
#define Offset(x, cls) _Offset_Normal(x, cls)
#define OffsetNonConst(x, cls) _Offset_Variant_1(x, cls)

#endif
#endif

#endif // INCLUDED_TYPES_GCC_H

