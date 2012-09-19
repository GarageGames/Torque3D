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

#include "platform/platform.h"
#include "core/util/tVector.h"

#include "platform/profiler.h"


#ifdef TORQUE_DEBUG_GUARD

bool VectorResize(U32 *aSize, U32 *aCount, void **arrayPtr, U32 newCount, U32 elemSize,
                  const char* fileName,
                  const U32   lineNum)
{
   PROFILE_SCOPE( VectorResize );

   if (newCount > 0) 
   {
      U32 blocks = newCount / VectorBlockSize;
      if (newCount % VectorBlockSize)
         blocks++;
      S32 mem_size = blocks * VectorBlockSize * elemSize;

      const char* pUseFileName = fileName != NULL ? fileName : __FILE__;
      U32 useLineNum           = fileName != NULL ? lineNum  : __LINE__;

      if (*arrayPtr != NULL)
         *arrayPtr = dRealloc_r(*arrayPtr, mem_size, pUseFileName, useLineNum);
      else
         *arrayPtr = dMalloc_r(mem_size, pUseFileName, useLineNum);

      AssertFatal( *arrayPtr, "VectorResize - Allocation failed." );

      *aCount = newCount;
      *aSize = blocks * VectorBlockSize;
      return true;
   }

   if (*arrayPtr) 
   {
      dFree(*arrayPtr);
      *arrayPtr = 0;
   }

   *aSize  = 0;
   *aCount = 0;
   return true;
}

#else

bool VectorResize(U32 *aSize, U32 *aCount, void **arrayPtr, U32 newCount, U32 elemSize)
{
   PROFILE_SCOPE( VectorResize );

   if (newCount > 0)
   {
      U32 blocks = newCount / VectorBlockSize;
      if (newCount % VectorBlockSize)
         blocks++;
      S32 mem_size = blocks * VectorBlockSize * elemSize;
      *arrayPtr = *arrayPtr ? dRealloc(*arrayPtr,mem_size) :
         dMalloc(mem_size);

      *aCount = newCount;
      *aSize = blocks * VectorBlockSize;
      return true;
   }

   if (*arrayPtr) 
   {
      dFree(*arrayPtr);
      *arrayPtr = 0;
   }

   *aSize = 0;
   *aCount = 0;
   return true;
}

#endif
