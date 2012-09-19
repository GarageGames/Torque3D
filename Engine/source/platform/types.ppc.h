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

#ifndef _TYPESPPC_H_
#define _TYPESPPC_H_

///< Calling convention
#define FN_CDECL

// size_t is needed to overload new
// size_t tends to be OS and compiler specific and may need to 
// be if/def'ed in the future
typedef unsigned long  dsize_t;


/** Platform dependent file date-time structure.  The defination of this structure
  * will likely be different for each OS platform.
  * On the PPC is a 64-bit structure for storing the date/time for a file
  */
  
// 64-bit structure for storing the date/time for a file
// The date and time, specified in seconds since the unix epoch.
// NOTE: currently, this is only 32-bits in value, so the upper 32 are all zeroes.
typedef U64 FileTime;

#ifndef NULL
#  define NULL (0)
#endif


#endif //_TYPESPPC_H_
