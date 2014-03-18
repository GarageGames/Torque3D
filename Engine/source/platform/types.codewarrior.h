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

#ifndef INCLUDED_TYPES_CODEWARRIOR_H
#define INCLUDED_TYPES_CODEWARRIOR_H

#pragma once

// If using the IDE detect if DEBUG build was requested
#if __ide_target("Torque-W32-Debug")
   #define TORQUE_DEBUG
#elif __ide_target("Torque-MacCarb-Debug")
   #define TORQUE_DEBUG
#elif __ide_target("Torque-MacX-Debug")
   #define TORQUE_DEBUG
#endif


//--------------------------------------
// Types
typedef signed long long   S64;     ///< Compiler independent Signed 64-bit integer
typedef unsigned long long U64;     ///< Compiler independent Unsigned 64-bit integer



//--------------------------------------
// Compiler Version
#define TORQUE_COMPILER_CODEWARRIOR __MWERKS__

#define TORQUE_COMPILER_STRING "CODEWARRIOR"


//--------------------------------------
// Identify the Operating System
#if defined(_WIN64)
#  define TORQUE_OS_STRING "Win64"
#  define TORQUE_OS_WIN
#  define TORQUE_OS_WIN64
#  include "platform/types.win.h"
#if defined(_WIN32)
#  define TORQUE_OS_STRING "Win32"
#  define TORQUE_OS_WIN
#  define TORQUE_OS_WIN32
#  include "platform/types.win.h"

#elif defined(macintosh) || defined(__APPLE__)
#  define TORQUE_OS_STRING "Mac"
#  define TORQUE_OS_MAC
#  if defined(__MACH__)
#     define TORQUE_OS_MAC
#  endif
#  include "platform/types.ppc.h"
#else 
#  error "CW: Unsupported Operating System"
#endif


//--------------------------------------
// Identify the CPU
#if defined(_M_IX86)
#  define TORQUE_CPU_STRING "x86"
#  define TORQUE_CPU_X86
#  define TORQUE_LITTLE_ENDIAN
#  define TORQUE_SUPPORTS_NASM
#  define TORQUE_SUPPORTS_VC_INLINE_X86_ASM

   // Compiling with the CW IDE we cannot use NASM :(
#  if __ide_target("Torque-W32-Debug")
#     undef TORQUE_SUPPORTS_NASM
#  elif __ide_target("Torque-W32-Release")
#     undef TORQUE_SUPPORTS_NASM
#  endif

#elif defined(__POWERPC__)
#  define TORQUE_CPU_STRING "PowerPC"
#  define TORQUE_CPU_PPC
#  define TORQUE_BIG_ENDIAN

#else
#  error "CW: Unsupported Target CPU"
#endif


#endif // INCLUDED_TYPES_CODEWARRIOR_H

