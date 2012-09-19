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

#include <CoreServices/CoreServices.h>
#include "platform/platform.h"
#include "platform/threads/semaphore.h"

class PlatformSemaphore
{
public:
   MPSemaphoreID mSemaphore;

   PlatformSemaphore(S32 initialCount)
   {
      OSStatus err = MPCreateSemaphore(S32_MAX - 1, initialCount, &mSemaphore);
      AssertFatal(err == noErr, "Failed to allocate semaphore!");
   }

   ~PlatformSemaphore()
   {
      OSStatus err = MPDeleteSemaphore(mSemaphore);
      AssertFatal(err == noErr, "Failed to destroy semaphore!");
   }
};

Semaphore::Semaphore(S32 initialCount)
{
   mData = new PlatformSemaphore(initialCount);
}

Semaphore::~Semaphore()
{
   AssertFatal(mData && mData->mSemaphore, "Semaphore::destroySemaphore: invalid semaphore");
   delete mData;
}

bool Semaphore::acquire( bool block, S32 timeoutMS )
{
   AssertFatal(mData && mData->mSemaphore, "Semaphore::acquireSemaphore: invalid semaphore");
   OSStatus err = MPWaitOnSemaphore(mData->mSemaphore, block ? ( timeoutMS == -1 ? kDurationForever : timeoutMS ) : kDurationImmediate);
   return(err == noErr);
}

void Semaphore::release()
{
   AssertFatal(mData && mData->mSemaphore, "Semaphore::releaseSemaphore: invalid semaphore");
   OSStatus err = MPSignalSemaphore(mData->mSemaphore);
   AssertFatal(err == noErr, "Failed to release semaphore!");
}
