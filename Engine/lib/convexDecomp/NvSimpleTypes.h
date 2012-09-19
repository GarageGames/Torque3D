#ifndef NV_SIMPLE_TYPES_H

#define NV_SIMPLE_TYPES_H

/*

NvSimpleTypes.h : Defines basic data types for integers and floats.

*/


/*!
**
** Copyright (c) 2009 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** Portions of this source has been released with the PhysXViewer application, as well as
** Rocket, CreateDynamics, ODF, and as a number of sample code snippets.
**
** If you find this code useful or you are feeling particularily generous I would
** ask that you please go to http://www.amillionpixels.us and make a donation
** to Troy DeMolay.
**
** DeMolay is a youth group for young men between the ages of 12 and 21.
** It teaches strong moral principles, as well as leadership skills and
** public speaking.  The donations page uses the 'pay for pixels' paradigm
** where, in this case, a pixel is only a single penny.  Donations can be
** made for as small as $4 or as high as a $100 block.  Each person who donates
** will get a link to their own site as well as acknowledgement on the
** donations blog located here http://www.amillionpixels.blogspot.com/
**
** If you wish to contact me you can use the following methods:
**
** Skype ID: jratcliff63367
** Yahoo: jratcliff63367
** AOL: jratcliff1961
** email: jratcliffscarab@gmail.com
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifdef __APPLE__
   #include <sys/malloc.h>
#else
   #include <malloc.h>
#endif
#include <assert.h>

#if defined(__APPLE__) || defined(__CELLOS_LV2__) || defined(LINUX)

#ifndef stricmp
#define stricmp(a, b) strcasecmp((a), (b))
#define _stricmp(a, b) strcasecmp((a), (b))
#endif

#endif

#ifdef WIN32
	typedef __int64				NxI64;
	typedef signed int			NxI32;
	typedef signed short		NxI16;
	typedef signed char			NxI8;

	typedef unsigned __int64	NxU64;
	typedef unsigned int		NxU32;
	typedef unsigned short		NxU16;
	typedef unsigned char		NxU8;

	typedef float				NxF32;
	typedef double				NxF64;
		
#elif LINUX
	typedef long long			NxI64;
	typedef signed int			NxI32;
	typedef signed short		NxI16;
	typedef signed char			NxI8;

	typedef unsigned long long	NxU64;
	typedef unsigned int		NxU32;
	typedef unsigned short		NxU16;
	typedef unsigned char		NxU8;

	typedef float				NxF32;
	typedef double				NxF64;

#elif __APPLE__
	typedef long long			NxI64;
	typedef signed int			NxI32;
	typedef signed short		NxI16;
	typedef signed char			NxI8;

	typedef unsigned long long	NxU64;
	typedef unsigned int		NxU32;
	typedef unsigned short		NxU16;
	typedef unsigned char		NxU8;

	typedef float				NxF32;
	typedef double				NxF64;

#elif __CELLOS_LV2__
	typedef long long			NxI64;
	typedef signed int			NxI32;
	typedef signed short		NxI16;
	typedef signed char			NxI8;

	typedef unsigned long long	NxU64;
	typedef unsigned int		NxU32;
	typedef unsigned short		NxU16;
	typedef unsigned char		NxU8;

	typedef float				NxF32;
	typedef double				NxF64;

#elif _XBOX
	typedef __int64				NxI64;
	typedef signed int			NxI32;
	typedef signed short		NxI16;
	typedef signed char			NxI8;

	typedef unsigned __int64	NxU64;
	typedef unsigned int		NxU32;
	typedef unsigned short		NxU16;
	typedef unsigned char		NxU8;

	typedef float				NxF32;
	typedef double				NxF64;

#elif defined(__PPCGEKKO__)
	typedef long long			NxI64;
	typedef signed int			NxI32;
	typedef signed short		NxI16;
	typedef signed char			NxI8;

	typedef unsigned long long	NxU64;
	typedef unsigned int		NxU32;
	typedef unsigned short		NxU16;
	typedef unsigned char		NxU8;

	typedef float				NxF32;
	typedef double				NxF64;

#else
	#error Unknown platform!
#endif

#ifndef NX_INLINE
#define NX_INLINE inline
#define NX_ASSERT assert
#endif


#endif
