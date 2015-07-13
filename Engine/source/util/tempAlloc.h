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

#ifndef _TEMPALLOC_H_
#define _TEMPALLOC_H_

#ifndef _PLATFORM_H_
#  include "platform/platform.h"
#endif


template< typename T >
struct TempAlloc
{
   T* ptr;
   U32 size;

   TempAlloc()
      : ptr(0), size(0) {}
   TempAlloc( U32 size )
      : size( size )
   {
      ptr = static_cast<T*>(dMalloc( size * sizeof( T ) ));
   }
   ~TempAlloc()
   {
      if( ptr )
         dFree( ptr );
   }
   operator T*()
   {
      return ptr;
   }

   TempAlloc( const TempAlloc& copyAlloc )
   {
      ptr = copyAlloc.ptr;
      size = copyAlloc.size; 
   }

   inline TempAlloc& operator=(const TempAlloc &copyAlloc)
   {
      ptr = copyAlloc.ptr;
      size = copyAlloc.size;
      return *this;
   }
};

#endif // _TEMPALLOC_H_
