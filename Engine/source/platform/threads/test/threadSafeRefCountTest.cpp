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
#include "platform/threads/threadSafeRefCount.h"
#include "platform/threads/thread.h"
#include "core/util/tVector.h"
#include "console/console.h"

FIXTURE(ThreadSafeRefCount)
{
public:
   struct TestObjectDtor : public ThreadSafeRefCount<TestObjectDtor>
   {
      bool &flag;
      TestObjectDtor(bool &f) : flag(f)
      {
         flag = false;
      }
      ~TestObjectDtor()
      {
         flag = true;
      }
   };
   typedef ThreadSafeRef<TestObjectDtor> TestObjectDtorRef;

   enum
   {
      NUM_ADD_REFS_PER_THREAD = 10,
      NUM_EXTRA_REFS_PER_THREAD = 10,
      NUM_THREADS = 10
   };
   
   class TestObject : public ThreadSafeRefCount<TestObject> {};
   typedef ThreadSafeRef<TestObject> TestObjectRef;
   
   class TestThread : public Thread
   {
   public:
      TestObjectRef mRef;
      Vector<TestObjectRef> mExtraRefs;

      TestThread(TestObjectRef ref) : mRef(ref) {}

      void run(void* arg)
      {
         if (!arg)
         {
            // Create references.
            for (U32 i = 0; i < NUM_ADD_REFS_PER_THREAD; i++)
               mRef->addRef();

            mExtraRefs.setSize(NUM_EXTRA_REFS_PER_THREAD);
            for (U32 i = 0; i < NUM_EXTRA_REFS_PER_THREAD; i++)
               mExtraRefs[i] = mRef;
         }
         else
         {
            // Clear references.
            mExtraRefs.clear();
            for (U32 i = 0; i < NUM_ADD_REFS_PER_THREAD; i++)
               mRef->release();
         }
      } 
   };

};

TEST_FIX(ThreadSafeRefCount, Serial)
{
   bool deleted = false;
   TestObjectDtorRef ref1 = new TestObjectDtor(deleted);
   ASSERT_FALSE(deleted);
   EXPECT_FALSE(ref1->isShared());
   EXPECT_TRUE(ref1 != NULL);

   TestObjectDtorRef ref2 = ref1;
   EXPECT_TRUE(ref1->isShared());
   EXPECT_TRUE(ref2->isShared());
   EXPECT_EQ(ref1, ref2);

   ref1 = NULL;
   EXPECT_FALSE(ref2->isShared());

   ref2 = NULL;
   ASSERT_TRUE(deleted);
}

TEST_FIX(ThreadSafeRefCount, Concurrent)
{
   TestObjectRef mRef = new TestObject;
   EXPECT_EQ(2, mRef->getRefCount()); // increments of 2

   Vector<TestThread*> threads;
   threads.setSize(NUM_THREADS);

   // Create threads.
   for (U32 i = 0; i < NUM_THREADS; i++)
      threads[i] = new TestThread(mRef);

   // Run phase 1: create references.
   for (U32 i = 0; i < NUM_THREADS; i++)
      threads[i]->start(NULL);

   // Wait for completion.
   for (U32 i = 0; i < NUM_THREADS; i++)
      threads[i]->join();

   EXPECT_EQ(2 + ((1 + NUM_ADD_REFS_PER_THREAD + NUM_EXTRA_REFS_PER_THREAD) * NUM_THREADS * 2),
             mRef->getRefCount());

   // Run phase 2: release references.
   for (U32 i = 0; i < NUM_THREADS; i++)
      threads[i]->start((void*) 1);

   // Wait for completion.
   for (U32 i = 0; i < NUM_THREADS; i++)
   {
      threads[i]->join();
      delete threads[i];
   }

   EXPECT_EQ(2, mRef->getRefCount()); // increments of two

   mRef = NULL;
}

TEST_FIX(ThreadSafeRefCount, Tagging)
{
   TestObjectRef ref;
   EXPECT_FALSE(ref.isTagged());
   EXPECT_FALSE(bool(ref));
   EXPECT_FALSE(bool(ref.ptr()));

   EXPECT_TRUE(ref.trySetFromTo(ref, NULL));
   EXPECT_FALSE(ref.isTagged());

   EXPECT_TRUE(ref.trySetFromTo(ref, NULL, TestObjectRef::TAG_Set));
   EXPECT_TRUE(ref.isTagged());
   EXPECT_TRUE(ref.trySetFromTo(ref, NULL, TestObjectRef::TAG_Set));
   EXPECT_TRUE(ref.isTagged());

   EXPECT_TRUE(ref.trySetFromTo(ref, NULL, TestObjectRef::TAG_Unset));
   EXPECT_FALSE(ref.isTagged());
   EXPECT_TRUE(ref.trySetFromTo(ref, NULL, TestObjectRef::TAG_Unset));
   EXPECT_FALSE(ref.isTagged());

   EXPECT_TRUE(ref.trySetFromTo(ref, NULL, TestObjectRef::TAG_SetOrFail));
   EXPECT_TRUE(ref.isTagged());
   EXPECT_FALSE(ref.trySetFromTo(ref, NULL, TestObjectRef::TAG_SetOrFail));
   EXPECT_TRUE(ref.isTagged());
   EXPECT_FALSE(ref.trySetFromTo(ref, NULL, TestObjectRef::TAG_FailIfSet));

   EXPECT_TRUE(ref.trySetFromTo(ref, NULL, TestObjectRef::TAG_UnsetOrFail));
   EXPECT_FALSE(ref.isTagged());
   EXPECT_FALSE(ref.trySetFromTo(ref, NULL, TestObjectRef::TAG_UnsetOrFail));
   EXPECT_FALSE(ref.isTagged());
   EXPECT_FALSE(ref.trySetFromTo(ref, NULL, TestObjectRef::TAG_FailIfUnset));

   TestObjectRef objectA = new TestObject;
   TestObjectRef objectB = new TestObject;

   EXPECT_FALSE(objectA->isShared());
   EXPECT_FALSE(objectB->isShared());

   ref = objectA;
   EXPECT_FALSE(ref.isTagged());
   EXPECT_TRUE(ref == objectA);
   EXPECT_TRUE(ref == objectA.ptr());
   EXPECT_TRUE(objectA->isShared());

   EXPECT_TRUE(ref.trySetFromTo(objectA, objectB, TestObjectRef::TAG_Set));
   EXPECT_TRUE(ref.isTagged());
   EXPECT_EQ(ref, objectB);
   EXPECT_EQ(ref, objectB.ptr());
   EXPECT_TRUE(objectB->isShared());
   EXPECT_FALSE(objectA->isShared());

   EXPECT_TRUE(ref.trySetFromTo(ref, objectA));
   EXPECT_TRUE(ref.isTagged());
   EXPECT_EQ(ref, objectA);
   EXPECT_EQ(ref, objectA.ptr());
}

#endif