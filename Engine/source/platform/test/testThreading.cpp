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
#include "platform/threads/thread.h"
#include "platform/threads/semaphore.h"
#include "platform/threads/mutex.h"
#include "unit/test.h"
#include "core/util/tVector.h"
#include "console/console.h"

using namespace UnitTesting;

class ThreadTestHarness
{
   U32 mStartTime, mEndTime, mCleanupTime;
   void (*mThreadBody)(void*);
   S32 mThreadCount;
   Thread **mThreads;
   
public:
   ThreadTestHarness()
   {
      mStartTime = mEndTime = mCleanupTime = 0;
      mThreadBody = NULL;
      mThreadCount = 1;
      mThreads = NULL;
   }

   void startThreads(void (*threadBody)(void*), void *arg, U32 threadCount)
   {
      mThreadCount = threadCount;
      mThreadBody = threadBody;
      
      // Start up threadCount threads...
      mThreads = new Thread*[threadCount];
      
      mStartTime = Platform::getRealMilliseconds();
      
      //Con::printf("   Running with %d threads...", threadCount);
      for(S32 i=0; i<mThreadCount; i++)
      {
         mThreads[i] = new Thread(threadBody, arg);
         mThreads[i]->start();
      }
   }

   void waitForThreadExit(U32 checkFrequencyMs)
   {
      // And wait for them to complete.
      bool someAlive = true;
      S32 liveCount = mThreadCount;
      
      while(someAlive)
      {
         //Con::printf("      - Sleeping for %dms with %d live threads.", checkFrequencyMs, liveCount);
         Platform::sleep(checkFrequencyMs);

         someAlive = false;
         liveCount = 0;
         
         for(S32 i=0; i<mThreadCount; i++)
         {
            if(!mThreads[i]->isAlive())
               continue;

            someAlive = true;
            liveCount++;
         }

      }
      
      mEndTime = Platform::getRealMilliseconds();

      // Clean up memory at this point.
      for(S32 i=0; i<mThreadCount; i++)
         delete mThreads[i];
      delete[] mThreads;
      
      // Make sure we didn't take a long time to complete.
      mCleanupTime = Platform::getRealMilliseconds();

      // And dump some stats.
      Con::printf("   Took approximately %dms (+/- %dms) to run %d threads, and %dms to cleanup.", 
                  (mEndTime - mStartTime),
                  checkFrequencyMs,
                  mThreadCount,
                  mCleanupTime - mEndTime);
   }

};

CreateUnitTest( ThreadSanityCheck, "Platform/Threads/BasicSanity")
{
   const static S32 amountOfWork = 100;
   const static S32 numberOfThreads = 8;
   
   static void threadBody(void *)
   {
      S32 work = 0x381f4fd3;
      // Spin on some work, then exit.
      for(S32 i=0; i<amountOfWork; i++)
      {
         // Do a little computation...
         work ^= (i + work | amountOfWork);
         
         // And sleep a slightly variable bit.
         Platform::sleep(10 + ((work+i) % 10));
      }
   }
   
   void runNThreads(S32 threadCount)
   {
      ThreadTestHarness tth;
      
      tth.startThreads(&threadBody, NULL, threadCount);
      tth.waitForThreadExit(32);
   }

   void run()
   {
      for(S32 i=0; i<numberOfThreads; i++)
         runNThreads(i);
   }
};

CreateUnitTest( MutexStressTest, "Platform/Threads/MutexStress")
{
   const static S32 numberOfLocks = 100;
   const static S32 numberOfThreads = 4;
   
   void *mMutex;
   
   static void threadBody(void *mutex)
   {
      // Acquire the mutex numberOfLocks times. Sleep for 1ms, acquire, sleep, release.
      S32 lockCount = numberOfLocks;
      while(lockCount--)
      {
         Platform::sleep(1);
         Mutex::lockMutex(mutex, true);
         Platform::sleep(1);
         Mutex::unlockMutex(mutex);
      }
   }
   
   void runNThreads(S32 threadCount)
   {
      ThreadTestHarness tth;
      
      mMutex = Mutex::createMutex();
      
      tth.startThreads(&threadBody, mMutex, threadCount);
      
      // We fudge the wait period to be about the expected time assuming
      // perfect execution speed.
      tth.waitForThreadExit(32); //threadCount * 2 * numberOfLocks + 100);
      
      Mutex::destroyMutex(mMutex);
   }

   void run()
   {
      for(S32 i=0; i<numberOfThreads; i++)
         runNThreads(i);
   }
};

CreateUnitTest( MemoryStressTest, "Platform/Threads/MemoryStress")
{
   const static S32 numberOfAllocs = 1000;
   const static S32 minAllocSize = 13;
   const static S32 maxAllocSize = 1024 * 1024;
   const static S32 numberOfThreads = 4;
   
   void *mMutex;
   
   // Cheap little RNG so we can vary our allocations more uniquely per thread.
   static U32 threadRandom(U32 &seed, U32 min, U32 max)
   {
      seed = (1664525 * seed + 1013904223);
      U32 res = seed;
      res %= (max - min);
      return res + min;
   }
   
   static void threadBody(void *mutex)
   {
      // Acquire the mutex numberOfLocks times. Sleep for 1ms, acquire, sleep, release.
      S32 allocCount = numberOfAllocs;
      U32 seed = (U32)((U32)mutex + (U32)&allocCount);
      while(allocCount--)
      {
         U8 *mem = new U8[threadRandom(seed, minAllocSize, maxAllocSize)];
         delete[] mem;
      }
   }
   
   void runNThreads(S32 threadCount)
   {
      ThreadTestHarness tth;
      
      mMutex = Mutex::createMutex();
      
      tth.startThreads(&threadBody, mMutex, threadCount);
      
      // We fudge the wait period to be about the expected time assuming
      // perfect execution speed.
      tth.waitForThreadExit(32);
      
      Mutex::destroyMutex(mMutex);
   }

   void run()
   {
      for(S32 i=0; i<numberOfThreads; i++)
         runNThreads(i);
   }
};

CreateUnitTest( ThreadGymnastics, "Platform/Threads/BasicSynchronization")
{
   void run()
   {
      // We test various scenarios wrt to locking and unlocking, in a single
      // thread, just to make sure our basic primitives are working in the
      // most basic case.
      
      void *mutex1 = Mutex::createMutex();
      test(mutex1, "First Mutex::createMutex call failed - that's pretty bad!");
      
      void *mutex2 = Mutex::createMutex();
      test(mutex2, "Second Mutex::createMutex call failed - that's pretty bad, too!");
      
      test(Mutex::lockMutex(mutex1, false), "Nonblocking call to brand new mutex failed - should not be.");
      test(Mutex::lockMutex(mutex1, true), "Failed relocking a mutex from the same thread - should be able to do this.");
      
      // Unlock & kill mutex 1
      Mutex::unlockMutex(mutex1);
      Mutex::unlockMutex(mutex1);
      Mutex::destroyMutex(mutex1);
      
      // Kill mutex2, which was never touched.
      Mutex::destroyMutex(mutex2);
      
      // Now we can test semaphores.
      Semaphore *sem1 = new Semaphore(1);
      Semaphore *sem2 = new Semaphore(1);

      // Test that we can do non-blocking acquires that succeed.
      test(sem1->acquire(false), "Should succeed at acquiring a new semaphore with count 1.");
      test(sem2->acquire(false), "This one should succeed too, see previous test.");
      
      // Test that we can do non-blocking acquires that fail.
      test(sem1->acquire(false)==false, "Should failed, as we've already got the sem.");
      sem1->release();
      test(sem2->acquire(false)==false, "Should also fail.");
      sem2->release();
      
      // Test that we can do blocking acquires that succeed.
      test(sem1->acquire(true)==true, "Should succeed as we just released.");
      test(sem2->acquire(true)==true, "Should succeed as we just released.");
      
      // Can't test blocking acquires that never happen... :)
      
      // Clean up.
      delete sem1;
      delete sem2;
   }
};

CreateUnitTest( SemaphoreWaitTest, "Platform/Threads/SemaphoreWaitTest")
{
   static void threadBody(void *self)
   {
      SemaphoreWaitTest *me = (SemaphoreWaitTest*)self;

      // Wait for the semaphore to get released.
      me->mSemaphore->acquire();
      
      // Increment the counter.
      Mutex::lockMutex(me->mMutex);
      me->mDoneCount++;
      Mutex::unlockMutex(me->mMutex);
      
      // Signal back to the main thread we're done.
      me->mPostbackSemaphore->release();
   }

   Semaphore   *mSemaphore;
   Semaphore   *mPostbackSemaphore;
   void        *mMutex;
   U32         mDoneCount;

   const static int csmThreadCount = 10;

   void run()
   {
      ThreadTestHarness tth;
      
      mDoneCount = 0;
      mSemaphore = new Semaphore(0);
      mPostbackSemaphore = new Semaphore(0);
      mMutex = Mutex::createMutex();
      
      tth.startThreads(&threadBody, this, csmThreadCount);
      
      Platform::sleep(500);

      Mutex::lockMutex(mMutex);
      test(mDoneCount == 0, "no threads should have touched the counter yet.");
      Mutex::unlockMutex(mMutex);
      
      // Let 500 come out.
      for(S32 i=0; i<csmThreadCount/2; i++)
         mSemaphore->release();

      // And wait for 500 postbacks.
      for(S32 i=0; i<csmThreadCount/2; i++)
         mPostbackSemaphore->acquire();

      Mutex::lockMutex(mMutex);
      test(mDoneCount == csmThreadCount / 2, "Didn't get expected number of done threads! (a)");
      Mutex::unlockMutex(mMutex);

      // Ok, now do the rest.
      // Let 500 come out.
      for(S32 i=0; i<csmThreadCount/2; i++)
         mSemaphore->release();

      // And wait for 500 postbacks.
      for(S32 i=0; i<csmThreadCount/2; i++)
         mPostbackSemaphore->acquire();

      Mutex::lockMutex(mMutex);
      test(mDoneCount == csmThreadCount, "Didn't get expected number of done threads! (b)");
      Mutex::unlockMutex(mMutex);
      
      // Wait for the threads to exit - shouldn't have to wait ever though.
      tth.waitForThreadExit(10);

      // Make sure no one touched our data after shutdown time.
      Mutex::lockMutex(mMutex);
      test(mDoneCount == csmThreadCount, "Didn't get expected number of done threads! (c)");
      Mutex::unlockMutex(mMutex);
   }
};

CreateUnitTest( MutexWaitTest, "Platform/Threads/MutexWaitTest")
{
   static void threadBody(void *self)
   {
      MutexWaitTest *me = (MutexWaitTest*)self;

      // Increment the counter. We'll block until the mutex
      // is open.
      Mutex::lockMutex(me->mMutex);
      me->mDoneCount++;
      Mutex::unlockMutex(me->mMutex);
   }

   void *mMutex;
   U32 mDoneCount;

   const static int csmThreadCount = 10;
   
   void run()
   {
      mMutex = Mutex::createMutex();      
      mDoneCount = 0;
      
      // We lock the mutex before we create any threads, so that all the threads
      // block on the mutex. Then we unlock it and let them all work their way
      // through the increment.
      Mutex::lockMutex(mMutex);

      ThreadTestHarness tth;
      tth.startThreads(&threadBody, this, csmThreadCount);
      
      Platform::sleep(5000);

      // Check count is still zero.
      test(mDoneCount == 0, "Uh oh - a thread somehow didn't get blocked by the locked mutex!");
      
      // Open the flood gates...
      Mutex::unlockMutex(mMutex);
      
      // Wait for the threads to all finish executing.
      tth.waitForThreadExit(10);
      
      Mutex::lockMutex(mMutex);
      test(mDoneCount == csmThreadCount, "Hmm - all threads reported done, but we didn't get the expected count.");
      Mutex::unlockMutex(mMutex);
      
      // Kill the mutex.
      Mutex::destroyMutex(mMutex);
   }
};