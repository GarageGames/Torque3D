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

#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include "platform/platform.h"
#include "platform/threads/mutex.h"
#include "platform/threads/thread.h"
// TODO: examine & dump errno if pthread_* funcs fail. ( only in debug build )

class PlatformMutexData
{
public:
   pthread_mutex_t   mMutex;
   bool              locked;
   U32         lockedByThread;
};

Mutex::Mutex(void)
{
   int ok;
   mData = new PlatformMutexData;
   pthread_mutexattr_t attr;
   ok = pthread_mutexattr_init(&attr);
   ok = pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
   ok = pthread_mutex_init(&(mData->mMutex),&attr);
   AssertFatal(ok == 0, "Mutex() failed: pthread_mutex_init() failed.");
   
   mData->locked = false;
   mData->lockedByThread = 0;
}

Mutex::~Mutex()
{
   int ok;
   ok = pthread_mutex_destroy( &(mData->mMutex) );
   AssertFatal(ok == 0, "~Mutex() failed: pthread_mutex_destroy() failed.");
   
   delete mData;
}
 
bool Mutex::lock( bool block)
{
   int ok;

   if(block)
   {
      ok = pthread_mutex_lock( &(mData->mMutex) );
      AssertFatal( ok != EINVAL, "Mutex::lockMutex() failed: invalid mutex.");
      AssertFatal( ok != EDEADLK, "Mutex::lockMutex() failed: system detected a deadlock!");
      AssertFatal( ok == 0, "Mutex::lockMutex() failed: pthread_mutex_lock() failed -- unknown reason.");
   } 
   else {
      ok = pthread_mutex_trylock( &(mData->mMutex) );
      // returns EBUSY if mutex was locked by another thread,
      // returns EINVAL if mutex was not a valid mutex pointer,
      // returns 0 if lock succeeded.
      AssertFatal( ok != EINVAL, "Mutex::lockMutex(non blocking) failed: invalid mutex.");
      if( ok != 0 )
         return false;

      AssertFatal( ok == 0, "Mutex::lockMutex(non blocking) failed: pthread_mutex_trylock() failed -- unknown reason.");
   }

   mData->locked = true;
   mData->lockedByThread = ThreadManager::getCurrentThreadId();
   return true;
}

void Mutex::unlock()
{
   int ok;
   ok = pthread_mutex_unlock( &(mData->mMutex) );
   AssertFatal( ok == 0, "Mutex::unlockMutex() failed: pthread_mutex_unlock() failed.");
   mData->locked = false;
   mData->lockedByThread = 0;
}
