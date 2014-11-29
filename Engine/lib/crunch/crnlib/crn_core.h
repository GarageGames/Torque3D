// File: crn_core.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

#if defined(WIN32) && defined(_MSC_VER)
   #pragma warning (disable: 4201) // nonstandard extension used : nameless struct/union
   #pragma warning (disable: 4127) // conditional expression is constant
   #pragma warning (disable: 4793) // function compiled as native
#endif

#if defined(WIN32)

#if 0
   #ifdef NDEBUG
      // Ensure checked iterators are disabled.
      #define _SECURE_SCL 0
      #define _HAS_ITERATOR_DEBUGGING 0
   #endif

   #ifndef _DLL
      // If we're using the DLL form of the run-time libs, we're also going to be enabling exceptions because we'll be building CLR apps.
      // Otherwise, we disable exceptions for a small (up to 5%) speed boost.
      #define _HAS_EXCEPTIONS 0
   #endif
#endif

   //#define _CRT_SECURE_NO_WARNINGS
   #define NOMINMAX

   #define CRNLIB_PLATFORM_PC 1

   #ifdef _WIN64
      #define CRNLIB_PLATFORM_PC_X64 1
   #else
      #define CRNLIB_PLATFORM_PC_X86 1
   #endif

   #define CRNLIB_USE_WIN32_API 1

   #ifdef _WIN64
      #define CRNLIB_PLATFORM_PC_X64 1
      #define CRNLIB_64BIT_POINTERS 1
      #define CRNLIB_CPU_HAS_64BIT_REGISTERS 1
      #define CRNLIB_LITTLE_ENDIAN_CPU 1
   #else
      #define CRNLIB_PLATFORM_PC_X86 1
      #define CRNLIB_64BIT_POINTERS 0
      #define CRNLIB_CPU_HAS_64BIT_REGISTERS 0
      #define CRNLIB_LITTLE_ENDIAN_CPU 1
   #endif
#endif

#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <algorithm>
#include <locale>

#ifdef min
   #undef min
#endif

#ifdef max
   #undef max
#endif

#define CRNLIB_FALSE (0)
#define CRNLIB_TRUE (1)
#define CRNLIB_MAX_PATH (260)

#ifdef _DEBUG
   #define CRNLIB_BUILD_DEBUG
#else
   #define CRNLIB_BUILD_RELEASE

   #ifndef NDEBUG
      #define NDEBUG
   #endif
#endif

#include "crn_platform.h"

#if defined(WIN32)
   #include "crn_mutex.h"
#endif

#include "crn_assert.h"
#include "crn_types.h"
#include "crn_helpers.h"
#include "crn_traits.h"
#include "crn_mem.h"
#include "crn_math.h"
#include "crn_utils.h"
#include "crn_hash.h"
#include "crn_vector.h"
#include "crn_win32_timer.h"
#include "crn_win32_threading.h"
#include "crn_dynamic_string.h"
#include "crn_dynamic_wstring.h"


