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

#include "platformWin32/platformWin32.h"
#include "platform/threads/semaphore.h"

class PlatformSemaphore
{
public:
   HANDLE   *semaphore;

   PlatformSemaphore(S32 initialCount)
   {
      semaphore = new HANDLE;
      *semaphore = CreateSemaphore(0, initialCount, S32_MAX, 0);
   }

   ~PlatformSemaphore()
   {
      CloseHandle(*(HANDLE*)(semaphore));
      delete semaphore;
      semaphore = NULL;
   }
};

Semaphore::Semaphore(S32 initialCount)
{
   mData = new PlatformSemaphore(initialCount);
}

Semaphore::~Semaphore()
{
   AssertFatal(mData && mData->semaphore, "Semaphore::destroySemaphore: invalid semaphore");
   delete mData;
}

bool Semaphore::acquire(bool block, S32 timeoutMS)
{
   AssertFatal(mData && mData->semaphore, "Semaphore::acquireSemaphore: invalid semaphore");
   if(block)
   {
      WaitForSingleObject(*(HANDLE*)(mData->semaphore), timeoutMS != -1 ? timeoutMS : INFINITE );
      return(true);
   }
   else
   {
      DWORD result = WaitForSingleObject(*(HANDLE*)(mData->semaphore), 0);
      return(result == WAIT_OBJECT_0);
   }
}

void Semaphore::release()
{
   AssertFatal(mData && mData->semaphore, "Semaphore::releaseSemaphore: invalid semaphore");
   ReleaseSemaphore(*(HANDLE*)(mData->semaphore), 1, 0);
}
