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
#include "platform/threads/thread.h"

TEST(Thread, CallbackAPI)
{
#define VALUE_TO_SET 10

   // This struct exists just so we can define run as a local function.
   struct thread
   {
      // Do some work we can observe.
      static void body(void* arg)
      {
         U32* value = reinterpret_cast<U32*>(arg);
         *value = VALUE_TO_SET;
      }
   };

   // Test most basic Thread API functions.
   U32 value = ~VALUE_TO_SET;
   Thread thread(&thread::body, reinterpret_cast<void*>(&value));
   thread.start();
   EXPECT_TRUE(thread.isAlive());
   thread.join();
   EXPECT_FALSE(thread.isAlive());

   EXPECT_EQ(value, VALUE_TO_SET)
      << "Thread did not set expected value!";

#undef VALUE_TO_SET
}

TEST(Thread, InheritanceAPI)
{
#define VALUE_TO_SET 10

   // This struct exists just so we can define run as a local function.
   struct thread : public Thread
   {
      U32* mPtr;
      thread(U32* ptr): mPtr(ptr) {}

      // Do some work we can observe.
      virtual void run(void*)
      {
         *mPtr = VALUE_TO_SET;
      }
   };

   // Test most basic Thread API functions.
   U32 value = ~VALUE_TO_SET;
   thread thread(&value);
   thread.start();
   EXPECT_TRUE(thread.isAlive());
   thread.join();
   EXPECT_FALSE(thread.isAlive());

   EXPECT_EQ(value, VALUE_TO_SET)
      << "Thread did not set expected value!";

#undef VALUE_TO_SET
}

#endif