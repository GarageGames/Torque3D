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
#include "platform/threads/threadSafePriorityQueue.h"
#include "platform/threads/thread.h"
#include "core/util/tVector.h"
#include "console/console.h"

// Test queue without concurrency.
TEST(ThreadSafePriorityQueue, Serial)
{
   const U32 min = 0;
   const U32 max = 9;
   const U32 len = 11;

   U32 indices[len]    = {  2,   7,   4,   6,   1,   5,   3,   8,   6,   9, 0};
   F32 priorities[len] = {0.2, 0.7, 0.4, 0.6, 0.1, 0.5, 0.3, 0.8, 0.6, 0.9, 0};
   
   ThreadSafePriorityQueue<U32, F32, true>  minQueue;
   ThreadSafePriorityQueue<U32, F32, false> maxQueue;

   for(U32 i = 0; i < len; i++)
   {
      minQueue.insert(priorities[i], indices[i]);
      maxQueue.insert(priorities[i], indices[i]);
   }

   EXPECT_FALSE(minQueue.isEmpty());
   EXPECT_FALSE(maxQueue.isEmpty());
   
   U32 index = min;
   for(U32 i = 0; i < len; i++)
   {
      U32 popped;
      EXPECT_TRUE(minQueue.takeNext(popped))
         << "Failed to pop element from minQueue";
      EXPECT_LE(index, popped)
         << "Element from minQueue was not in sort order";
      index = popped;
   }
   
   index = max;
   for(U32 i = 0; i < len; i++)
   {
      U32 popped;
      EXPECT_TRUE(maxQueue.takeNext(popped))
         << "Failed to pop element from maxQueue";
      EXPECT_GE(index, popped)
         << "Element from maxQueue was not in sort order";
      index = popped;
   }
}

// Test queue with concurrency.
TEST(ThreadSafePriorityQueue, Concurrent)
{
#define MIN 0
#define MAX 9
#define LEN 11

   typedef ThreadSafePriorityQueue<U32, F32, true>  MinQueue;
   typedef ThreadSafePriorityQueue<U32, F32, false> MaxQueue;

   struct ProducerThread : public Thread
   {
      MinQueue& minQueue;
      MaxQueue& maxQueue;
      ProducerThread(MinQueue& min, MaxQueue& max)
         : minQueue(min), maxQueue(max) {}

      virtual void run(void*)
      {
         U32 indices[LEN]    = {  2,   7,   4,   6,   1,   5,   3,   8,   6,   9, 0};
         F32 priorities[LEN] = {0.2, 0.7, 0.4, 0.6, 0.1, 0.5, 0.3, 0.8, 0.6, 0.9, 0};

         for(U32 i = 0; i < LEN; i++)
         {
            minQueue.insert(priorities[i], indices[i]);
            maxQueue.insert(priorities[i], indices[i]);
         }
      }
   };

   MinQueue minQueue;
   MaxQueue maxQueue;
   ProducerThread producers[] = {
      ProducerThread(minQueue, maxQueue),
      ProducerThread(minQueue, maxQueue),
      ProducerThread(minQueue, maxQueue)
   };

   const U32 len = sizeof(producers) / sizeof(ProducerThread);
   for(U32 i = 0; i < len; i++)
      producers[i].start();
   for(U32 i = 0; i < len; i++)
      producers[i].join();

   U32 index = MIN;
   for(U32 i = 0; i < LEN * len; i++)
   {
      U32 popped;
      EXPECT_TRUE(minQueue.takeNext(popped))
         << "Failed to pop element from minQueue";
      EXPECT_LE(index, popped)
         << "Element from minQueue was not in sort order";
      index = popped;
   }
   
   index = MAX;
   for(U32 i = 0; i < LEN * len; i++)
   {
      U32 popped;
      EXPECT_TRUE(maxQueue.takeNext(popped))
         << "Failed to pop element from maxQueue";
      EXPECT_GE(index, popped)
         << "Element from maxQueue was not in sort order";
      index = popped;
   }

#undef MIN
#undef MAX
#undef LEN
}

#endif