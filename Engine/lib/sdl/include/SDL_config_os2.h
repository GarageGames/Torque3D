/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2019 Sam Lantinga <slouken@libsdl.org>

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

#ifndef SDL_config_os2_h_
#define SDL_config_os2_h_
#define SDL_config_h_

#include "SDL_platform.h"

#define SDL_AUDIO_DRIVER_DUMMY 1
#define SDL_AUDIO_DRIVER_DISK 1

#define SDL_POWER_DISABLED  1
#define SDL_JOYSTICK_DISABLED 1
#define SDL_HAPTIC_DISABLED 1
/*#undef SDL_JOYSTICK_HIDAPI */

#define SDL_SENSOR_DUMMY 1
#define SDL_VIDEO_DRIVER_DUMMY 1

/* Enable OpenGL support */
/* #undef SDL_VIDEO_OPENGL */

/* Enable Vulkan support */
/* #undef SDL_VIDEO_VULKAN */

#define SDL_LOADSO_DISABLED 1
#define SDL_THREADS_DISABLED 1
#define SDL_TIMERS_DISABLED 1
#define SDL_FILESYSTEM_DUMMY 1

/* Enable assembly routines */
#define SDL_ASSEMBLY_ROUTINES 1

/* #undef HAVE_LIBSAMPLERATE_H */

/* Enable dynamic libsamplerate support */
/* #undef SDL_LIBSAMPLERATE_DYNAMIC */

#define HAVE_LIBC 1

#define HAVE_SYS_TYPES_H 1
#define HAVE_STDIO_H 1
#define STDC_HEADERS 1
#define HAVE_STDLIB_H 1
#define HAVE_STDARG_H 1
#define HAVE_STDDEF_H 1
#define HAVE_MALLOC_H 1
#define HAVE_MEMORY_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_WCHAR_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
#define HAVE_LIMITS_H 1
#define HAVE_CTYPE_H 1
#define HAVE_MATH_H 1
#define HAVE_FLOAT_H 1
#define HAVE_SIGNAL_H 1

#define HAVE_MALLOC 1
#define HAVE_CALLOC 1
#define HAVE_REALLOC 1
#define HAVE_FREE 1
#if defined(__WATCOMC__)
#define HAVE__FSEEKI64 1
#define HAVE__FTELLI64 1
#endif
#define HAVE_ALLOCA 1
#define HAVE_GETENV 1
#define HAVE_SETENV 1
#define HAVE_PUTENV 1
#define HAVE_QSORT 1
#define HAVE_ABS 1
#define HAVE_BCOPY 1
#define HAVE_MEMSET 1
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMCMP 1
#define HAVE_WCSLEN 1
#define HAVE_WCSLCPY 1
#define HAVE_WCSLCAT 1
#define HAVE_WCSCMP 1
#define HAVE_STRLEN 1
#define HAVE_STRLCPY 1
#define HAVE_STRLCAT 1
#define HAVE__STRREV 1
#define HAVE__STRUPR 1
#define HAVE__STRLWR 1
#define HAVE_INDEX 1
#define HAVE_RINDEX 1
#define HAVE_STRCHR 1
#define HAVE_STRRCHR 1
#define HAVE_STRSTR 1
#define HAVE_ITOA 1
#define HAVE__LTOA 1
#define HAVE__ULTOA 1
#define HAVE_STRTOL 1
#define HAVE_STRTOUL 1
#define HAVE__I64TOA 1
#define HAVE__UI64TOA 1
#define HAVE_STRTOLL 1
#define HAVE_STRTOULL 1
#define HAVE_STRTOD 1
#define HAVE_ATOI 1
#define HAVE_ATOF 1
#define HAVE_STRCMP 1
#define HAVE_STRNCMP 1
#define HAVE_STRICMP 1
#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1
#define HAVE_SSCANF 1
#define HAVE_SNPRINTF 1
#define HAVE_VSNPRINTF 1
#define HAVE_SETJMP 1
#define HAVE_ACOS 1
/* #undef HAVE_ACOSF */
#define HAVE_ASIN 1
/* #undef HAVE_ASINF */
#define HAVE_ATAN 1
#define HAVE_ATAN2 1
/* #undef HAVE_ATAN2F */
#define HAVE_CEIL 1
/* #undef HAVE_CEILF */
/* #undef HAVE_COPYSIGN */
/* #undef HAVE_COPYSIGNF */
#define HAVE_COS 1
/* #undef HAVE_COSF */
#define HAVE_EXP 1
/* #undef HAVE_EXPF */
#define HAVE_FABS 1
/* #undef HAVE_FABSF */
#define HAVE_FLOOR 1
/* #undef HAVE_FLOORF */
#define HAVE_FMOD 1
/* #undef HAVE_FMODF */
#define HAVE_LOG 1
/* #undef HAVE_LOGF */
#define HAVE_LOG10 1
/* #undef HAVE_LOG10F */
#define HAVE_POW 1
/* #undef HAVE_POWF */
#define HAVE_SIN 1
/* #undef HAVE_SINF */
/* #undef HAVE_SCALBN */
/* #undef HAVE_SCALBNF */
#define HAVE_SQRT 1
/* #undef HAVE_SQRTF */
#define HAVE_TAN 1
/* #undef HAVE_TANF */

#endif /* SDL_config_os2_h_ */
