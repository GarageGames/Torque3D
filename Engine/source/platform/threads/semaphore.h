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

#ifndef _PLATFORM_THREAD_SEMAPHORE_H_
#define _PLATFORM_THREAD_SEMAPHORE_H_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

// Forward ref used by platform code
class PlatformSemaphore;

class Semaphore
{
protected:
   PlatformSemaphore *mData;

public:
   /// Create a semaphore. initialCount defaults to 1.
   Semaphore(S32 initialCount = 1);
   /// Delete a semaphore, ignoring it's count.
   ~Semaphore();

   /// Acquire the semaphore, decrementing its count.
   /// if the initial count is less than 1, block until it goes above 1, then acquire.
   /// Returns true if the semaphore was acquired, false if the semaphore could
   /// not be acquired and block was false.
   bool acquire(bool block = true, S32 timeoutMS = -1);

   /// Release the semaphore, incrementing its count.
   /// Never blocks.
   void release();
};

#endif
