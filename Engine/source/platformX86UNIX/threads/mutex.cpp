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

#include "console/console.h"
#include "platformX86UNIX/platformX86UNIX.h"
#include "platform/threads/mutex.h"
#include "core/util/safeDelete.h"

#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

struct PlatformMutexData
{
   pthread_mutex_t mutex;
};

Mutex::Mutex()
{
   mData = new PlatformMutexData;
   pthread_mutexattr_t attr;

   pthread_mutexattr_init(&attr);
   pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);

   pthread_mutex_init(&mData->mutex, &attr);
}

Mutex::~Mutex()
{
   AssertFatal(mData, "Mutex::destroyMutex: invalid mutex");
   pthread_mutex_destroy(&mData->mutex);
   SAFE_DELETE(mData);
}

bool Mutex::lock(bool block)
{
   if(mData == NULL)
      return false;
   if(block)
   {
	   return pthread_mutex_lock(&mData->mutex) == 0;
   }
   else
   {
	   return pthread_mutex_trylock(&mData->mutex) == 0;
   }
}

void Mutex::unlock()
{
   if(mData == NULL)
      return;
   pthread_mutex_unlock(&mData->mutex);
}
