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

#include "platformX86UNIX/platformX86UNIX.h"
#include "platform/threads/semaphore.h"
#include <unistd.h>     
#include <sys/types.h>  
#include <errno.h>      
#include <semaphore.h> 
#include <time.h>

struct PlatformSemaphore
{
   sem_t semaphore;
   bool initialized;

   PlatformSemaphore(S32 initialCount)
   {
	   initialized = true;
	   if (sem_init(&semaphore, 0, initialCount) == -1) {
		   initialized = false;
			AssertFatal(0, "PlatformSemaphore constructor - Failed to create Semaphore.");
	   } 
   }

   ~PlatformSemaphore()
   {
       sem_destroy(&semaphore);
	   initialized = false;
   }
};

Semaphore::Semaphore(S32 initialCount)
{
  mData = new PlatformSemaphore(initialCount);
}

Semaphore::~Semaphore()
{
  AssertFatal(mData, "Semaphore destructor - Invalid semaphore.");
  delete mData;
}

bool Semaphore::acquire(bool block, S32 timeoutMS)
{
   AssertFatal(mData && mData->initialized, "Semaphore::acquire - Invalid semaphore.");
   if (block)
   {
      //SDL was removed so I do not now if this still holds true or not with OS calls but my guess is they are used underneath SDL anyway 
      // Semaphore acquiring is different from the MacOS/Win realization because SDL_SemWaitTimeout() with "infinite" timeout can be too heavy on some platforms.
      // (see "man SDL_SemWaitTimeout(3)" for more info)
      // "man" states to avoid the use of SDL_SemWaitTimeout at all, but at current stage this looks like a valid and working solution, so keeping it this way.
      // [bank / Feb-2010]
      if (timeoutMS == -1)
      {
         if (sem_wait(&mData->semaphore) < 0)
            AssertFatal(false, "Semaphore::acquire - Wait failed.");
      }
      else
      {
	     //convert timeoutMS to timespec
         timespec ts;
		 if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
			 AssertFatal(false, "Semaphore::acquire - clock_realtime failed.");
		 }
		 ts.tv_sec += timeoutMS / 1000;
		 ts.tv_nsec += (timeoutMS % 1000) * 1000;

         if (sem_timedwait(&mData->semaphore, &ts) < 0)
            AssertFatal(false, "Semaphore::acquire - Wait with timeout failed.");
      }
      return (true);
   }
   else
   {
      int res = sem_trywait(&mData->semaphore);
      return (res == 0);
   }
}

void Semaphore::release()
{
   AssertFatal(mData, "Semaphore::releaseSemaphore - Invalid semaphore.");
   sem_post(&mData->semaphore);
}
