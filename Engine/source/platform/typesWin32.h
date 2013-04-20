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

#ifndef _TYPESWIN32_H_
#define _TYPESWIN32_H_

#include <cstdint>

// We have to check this.  Since every file will eventually wind up including
//  this header, but not every header includes a windows or system header...
//
#ifndef NULL
#define NULL 0
#endif

// Let's just have this in a nice central location.  Again, since every file
//  will wind up including this file, we can affect compilation most effectively
//  from this location.
//
#define PLATFORM_LITTLE_ENDIAN      ///< Signals this platfrom is Little Endian


#define FN_CDECL __cdecl            ///< Calling convention

//------------------------------------------------------------------------------
//-------------------------------------- Basic Types...

typedef std::int8_t    S8;
typedef std::uint8_t   U8;
typedef std::int16_t  S16;
typedef std::uint16_t U16;
typedef std::int32_t  S32;
typedef std::uint32_t U32;
typedef std::int64_t  S64;
typedef std::uint64_t U64;
typedef float         F32;
typedef double        F64;

// size_t is needed to overload new
// size_t tends to be OS and compiler specific and may need to
// be if/def'ed in the future
typedef unsigned int  dsize_t;

typedef const char* StringTableEntry;

/*  Platform dependent file date-time structure.  The defination of this structure
  * will likely be different for each OS platform.
  */
struct FileTime
{
   U32 v1;
   U32 v2;
};

//------------------------------------------------------------------------------
//-------------------------------------- Type constants...
#define __EQUAL_CONST_F F32(0.000001)                             ///< Constant float epsilon used for F32 comparisons

static const F32 Float_One  = F32(1.0);                           ///< Constant float 1.0
static const F32 Float_Half = F32(0.5);                           ///< Constant float 0.5
static const F32 Float_Zero = F32(0.0);                           ///< Constant float 0.0
static const F32 Float_Pi   = F32(3.14159265358979323846);        ///< Constant float PI
static const F32 Float_2Pi  = F32(2.0 * 3.14159265358979323846);  ///< Constant float 2*PI

static const S8  S8_MIN  = S8(-128);                              ///< Constant Min Limit S8
static const S8  S8_MAX  = S8(127);                               ///< Constant Max Limit S8
static const U8  U8_MAX  = U8(255);                               ///< Constant Max Limit U8

static const S16 S16_MIN = S16(-32768);                           ///< Constant Min Limit S16
static const S16 S16_MAX = S16(32767);                            ///< Constant Max Limit S16
static const U16 U16_MAX = U16(65535);                            ///< Constant Max Limit U16

static const S32 S32_MIN = S32(-2147483647 - 1);                  ///< Constant Min Limit S32
static const S32 S32_MAX = S32(2147483647);                       ///< Constant Max Limit S32
static const U32 U32_MAX = U32(0xffffffff);                       ///< Constant Max Limit U32

static const F32 F32_MIN = F32(1.175494351e-38F);                 ///< Constant Min Limit F32
static const F32 F32_MAX = F32(3.402823466e+38F);                 ///< Constant Max Limit F32


#ifdef _MSC_VER
#if _MSC_VER < 1700
#define for if(false) {} else for   ///< Hack to work around Microsoft VC's non-C++ compliance on variable scoping
#endif
#endif

#endif //_NTYPES_H_
