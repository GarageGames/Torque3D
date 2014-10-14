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
#include "platform/threads/semaphore.h"
#include "platform/threads/thread.h"

TEST(Semaphore, BasicSynchronization)
{
   Semaphore *sem1 = new Semaphore(1);
   Semaphore *sem2 = new Semaphore(1);

   // Test that we can do non-blocking acquires that succeed.
   EXPECT_TRUE(sem1->acquire(false))
      << "Should succeed at acquiring a new semaphore with count 1.";
   EXPECT_TRUE(sem2->acquire(false))
      << "This one should succeed too, see previous test.";

   // Test that we can do non-blocking acquires that fail.
   EXPECT_FALSE(sem1->acquire(false))
      << "Should failed, as we've already got the sem.";
   sem1->release();
   EXPECT_FALSE(sem2->acquire(false))
      << "Should also fail.";
   sem2->release();

   // Test that we can do blocking acquires that succeed.
   EXPECT_TRUE(sem1->acquire(true))
      << "Should succeed as we just released.";
   EXPECT_TRUE(sem2->acquire(true))
      << "Should succeed as we just released.";

   // Clean up.
   delete sem1;
   delete sem2;
}

TEST(Semaphore, MultiThreadSynchronization)
{
   Semaphore semaphore(1);

   struct thread
   {
      // Try to acquire the semaphore from another thread.
      static void body1(void* sem)
      {
         Semaphore *semaphore = reinterpret_cast<Semaphore*>(sem);
         EXPECT_TRUE(semaphore->acquire(true));
         // Note that this semaphore is never released. Bad programmer!
      }
      // One more acquisition should fail!
      static void body2(void* sem)
      {
         Semaphore *semaphore = reinterpret_cast<Semaphore*>(sem);
         EXPECT_FALSE(semaphore->acquire(false));
      }
   };

   Thread thread1(&thread::body1, &semaphore);
   EXPECT_TRUE(semaphore.acquire(true));
   thread1.start();
   semaphore.release();
   thread1.join();

   Thread thread2(&thread::body2, &semaphore);
   thread2.start();
   thread2.join();
}

#endif