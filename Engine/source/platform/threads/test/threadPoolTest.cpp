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

   // Wait for all items to complete.
   pool->flushWorkItems();

   // Verify.
   for (U32 i = 0; i < numItems; i++)
      EXPECT_EQ(results[i], i) << "result mismatch";
   results.clear();
}

#endif