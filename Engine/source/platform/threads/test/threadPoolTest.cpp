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
#include "platform/threads/threadPool.h"
#include "console/console.h"
#include "core/util/tVector.h"

FIXTURE(ThreadPool)
{
public:
   // Represents a single unit of work. In this test we just set an element in
   // a result vector.
   struct TestItem : public ThreadPool::WorkItem
   {
      U32 mIndex;
      Vector<U32>& mResults;
      TestItem(U32 index, Vector<U32>& results)
         : mIndex(index), mResults(results) {}

   protected:
      virtual void execute()
      {
         mResults[mIndex] = mIndex;
      }
   };

   // A worker that delays for some time. We'll use this to test the ThreadPool's
   // synchronous and asynchronous operations.
   struct DelayItem : public ThreadPool::WorkItem
   {
      U32 ms;
      DelayItem(U32 _ms) : ms(_ms) {}

   protected:
      virtual void execute()
      {
         Platform::sleep(ms);
      }
   };
};

TEST_FIX(ThreadPool, BasicAPI)
{
   // Construct the vector of results from the work items.
   const U32 numItems = 100;
   Vector<U32> results(__FILE__, __LINE__);
   results.setSize(numItems);
   for (U32 i = 0; i < numItems; i++)
      results[i] = U32(-1);

   // Launch the work items.
   ThreadPool* pool = &ThreadPool::GLOBAL();
   for (U32 i = 0; i < numItems; i++)
   {
      ThreadSafeRef<TestItem> item(new TestItem(i, results));
      pool->queueWorkItem(item);
   }

   pool->waitForAllItems();

   // Verify.
   for (U32 i = 0; i < numItems; i++)
      EXPECT_EQ(results[i], i) << "result mismatch";
   results.clear();
}

TEST_FIX(ThreadPool, Asynchronous)
{
   const U32 delay = 500; //ms

   // Launch a single delaying work item.
   ThreadPool* pool = &ThreadPool::GLOBAL();
   ThreadSafeRef<DelayItem> item(new DelayItem(delay));
   pool->queueWorkItem(item);

   // The thread should not yet be finished.
   EXPECT_EQ(false, item->hasExecuted());

   // Wait til the item should have completed.
   Platform::sleep(delay * 2);

   EXPECT_EQ(true, item->hasExecuted());
}

TEST_FIX(ThreadPool, Synchronous)
{
   const U32 delay = 500; //ms

   // Launch a single delaying work item.
   ThreadPool* pool = &ThreadPool::GLOBAL();
   ThreadSafeRef<DelayItem> item(new DelayItem(delay));
   pool->queueWorkItem(item);

   // Wait for the item to complete.
   pool->waitForAllItems();

   EXPECT_EQ(true, item->hasExecuted());
}

#endif