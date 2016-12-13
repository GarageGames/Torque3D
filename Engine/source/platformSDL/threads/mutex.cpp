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
#include "platform/threads/mutex.h"
#include "core/util/safeDelete.h"

#include <SDL.h>
#include <SDL_thread.h>

struct PlatformMutexData
{
   SDL_mutex *mutex;
};

Mutex::Mutex()
{
   mData = new PlatformMutexData;
   mData->mutex = SDL_CreateMutex();
}

Mutex::~Mutex()
{
   AssertFatal(mData, "Mutex::destroyMutex: invalid mutex");
   SDL_DestroyMutex(mData->mutex);
   SAFE_DELETE(mData);
}

bool Mutex::lock(bool block)
{
   if(mData == NULL)
      return false;
   if(block)
   {
      return SDL_LockMutex(mData->mutex) == 0;
   }
   else
   {
      return SDL_TryLockMutex(mData->mutex) == 0;
   }
}

void Mutex::unlock()
{
   if(mData == NULL)
      return;
   SDL_UnlockMutex(mData->mutex);
}
