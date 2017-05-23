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


#define FN_CDECL __cdecl            ///< Calling convention

// size_t is needed to overload new
// size_t tends to be OS and compiler specific and may need to 
// be if/def'ed in the future

#ifdef _WIN64
typedef unsigned long long  dsize_t;
#else
typedef unsigned int  dsize_t;
#endif // _WIN64


/// Platform dependent file date-time structure.  The definition of this structure
/// will likely be different for each OS platform.
struct FileTime                     
{
   U32 v1;
   U32 v2;
};


#ifndef NULL
#  define NULL 0
#endif


#endif //_TYPESWIN32_H_
