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

#ifdef _MSC_VER
#if _MSC_VER < 1700
#define for if(false) {} else for   ///< Hack to work around Microsoft VC's non-C++ compliance on variable scoping
#endif
#endif

#endif //_NTYPES_H_
