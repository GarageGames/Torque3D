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

#include "platform/threads/threadSafeDeque.h"
#include "platform/threads/thread.h"
#include "core/util/tVector.h"
#include "console/console.h"

FIXTURE(ThreadSafeDeque)
{
public:
   // Used by the concurrent test.
   struct Value : public ThreadSafeRefCount<Value>
   {
      U32 mIndex;
      U32 mTick;

      Value() {}
      Value(U32 index, U32 tick)
         : mIndex(index), mTick(tick) {}
   };

   typedef ThreadSafeRef<Value> ValueRef;

   struct Deque : public ThreadSafeDeque<ValueRef>
   {
      typedef ThreadSafeDeque<ValueRef> Parent;

      U32 mPushIndex;
      U32 mPopIndex;

      Deque()
         : mPushIndex(0), mPopIndex(0) {}

      void pushBack(const ValueRef& value)
      {
         EXPECT_EQ(value->mIndex, mPushIndex) << "index out of line";
         mPushIndex++;
         Parent::pushBack(value);
      }

      bool tryPopFront(ValueRef& outValue)
      {
         if(Parent::tryPopFront(outValue))
         {
            EXPECT_EQ(outValue->mIndex, mPopIndex) << "index out of line";
            mPopIndex++;
            return true;
         }
         else
            return false;
      }
   };

   struct ProducerThread : public Thread
   {
      Vector<U32>& mValues;
      Deque& mDeque;
      ProducerThread(Vector<U32>& values, Deque& deque)
         : mValues(values), mDeque(deque) {}

      virtual void run(void*)
      {
         for(U32 i = 0; i < mValues.size(); i++)
         {
            U32 tick = Platform::getRealMilliseconds();
            mValues[i] = tick;

            ValueRef val = new Value(i, tick);
            mDeque.pushBack(val);
         }
      }
   };

   struct ConsumerThread : public Thread
   {
      Vector<U32>& mValues;
      Deque& mDeque;
      ConsumerThread(Vector<U32>& values, Deque& deque)
         : mValues(values), mDeque(deque) {}

      virtual void run(void*)
      {
         for(U32 i = 0; i < mValues.size(); i++)
         {
            ValueRef value;
            while(!mDeque.tryPopFront(value));

            EXPECT_EQ(i, value->mIndex);
            EXPECT_EQ(value->mTick, mValues[i]);
         }
      }
   };
};

// Test deque without concurrency.
TEST_FIX(ThreadSafeDeque, PopFront)
{
   ThreadSafeDeque<char> deque;
   String str = "teststring";

   for(U32 i = 0; i < str.length(); i++)
      deque.pushBack(str[i]);

   EXPECT_FALSE(deque.isEmpty());

   char ch;
   for(U32 i = 0; i < str.length(); i++)
   {
      EXPECT_TRUE(deque.tryPopFront(ch));
      EXPECT_EQ(str[i], ch);
   }

   ASSERT_TRUE(deque.isEmpty());
}

TEST_FIX(ThreadSafeDeque, PopBack)
{
   ThreadSafeDeque<char> deque;
   String str = "teststring";

   const char* p1 = str.c_str() + 4;
   const char* p2 = p1 + 1;
   while(*p2)
   {
      deque.pushFront(*p1);
      deque.pushBack(*p2);
      --p1;
      ++p2;
   }

   char ch;
   for(S32 i = str.length()-1; i >= 0; i--)
   {
      EXPECT_TRUE(deque.tryPopBack(ch));
      EXPECT_EQ(str[i], ch);
   }

   ASSERT_TRUE(deque.isEmpty());
}

// Test deque in a concurrent setting.
TEST_FIX(ThreadSafeDeque, Concurrent1)
{
   const U32 NumValues = 100;

   Deque mDeque;
   Vector<U32> mValues;

   mValues.setSize(NumValues);

   ProducerThread pThread(mValues, mDeque);
   ConsumerThread cThread(mValues, mDeque);

   pThread.start();
   cThread.start();

   pThread.join();
   cThread.join();

   mValues.clear();
};

#endif