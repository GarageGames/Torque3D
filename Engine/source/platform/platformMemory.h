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

#ifndef _TORQUE_PLATFORM_PLATFORMMEMORY_H_
#define _TORQUE_PLATFORM_PLATFORMMEMORY_H_

#include "platform/platform.h"

namespace Memory
{
   enum EFlag
   {
      FLAG_Debug,
      FLAG_Global,
      FLAG_Static
   };

   struct Info
   {
      U32         mAllocNumber;
      U32         mAllocSize;
      const char* mFileName;
      U32         mLineNumber;
      bool        mIsArray;
      bool        mIsGlobal;
      bool        mIsStatic;
   };

   void        checkPtr( void* ptr );
   void        flagCurrentAllocs( EFlag flag = FLAG_Debug );
   void        ensureAllFreed();
   void        dumpUnflaggedAllocs(const char *file, EFlag flag = FLAG_Debug );
   S32         countUnflaggedAllocs(const char *file, S32 *outUnflaggedRealloc = NULL, EFlag flag = FLAG_Debug );
   dsize_t     getMemoryUsed();
   dsize_t     getMemoryAllocated();
   void        getMemoryInfo( void* ptr, Info& info );
   void        validate();
}

#endif // _TORQUE_PLATFORM_PLATFORMMEMORY_H_
