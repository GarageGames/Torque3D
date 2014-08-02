//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#include "platform/threads/mutex.h"
#include "platform/threads/thread.h"

TEST(Mutex, BasicSynchronization)
{
   // We test various scenarios wrt to locking and unlocking, in a single
   // thread, just to make sure our basic primitives are working in the
   // most basic case.
   void *mutex1 = Mutex::createMutex();
   EXPECT_TRUE(mutex1 != NULL)
      << "First Mutex::createMutex call failed - that's pretty bad!";

   // This mutex is intentionally unused.
   void *mutex2 = Mutex::createMutex();
   EXPECT_TRUE(mutex2 != NULL)
      << "Second Mutex::createMutex call failed - that's pretty bad, too!";

   EXPECT_TRUE(Mutex::lockMutex(mutex1, false))
      << "Nonblocking call to brand new mutex failed - should not be.";
   EXPECT_TRUE(Mutex::lockMutex(mutex1, true))
      << "Failed relocking a mutex from the same thread - should be able to do this.";

   // Try to acquire the mutex from another thread.
   struct thread
   {
      static void body(void* mutex)
      {
         // We should not be able to lock the mutex from a separate thread, but
         // we don't want to block either.
         EXPECT_FALSE(Mutex::lockMutex(mutex, false));
      }
   };
   Thread thread(&thread::body, mutex1);
   thread.start();
   thread.join();

   // Unlock & kill mutex 1
   Mutex::unlockMutex(mutex1);
   Mutex::unlockMutex(mutex1);
   Mutex::destroyMutex(mutex1);

   // Kill mutex2, which was never touched.
   Mutex::destroyMutex(mutex2);
}

#endif