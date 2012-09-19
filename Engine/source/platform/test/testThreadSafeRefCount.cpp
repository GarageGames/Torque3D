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

#include "unit/test.h"
#include "platform/threads/threadSafeRefCount.h"
#include "platform/threads/thread.h"
#include "core/util/tVector.h"
#include "console/console.h"

#ifndef TORQUE_SHIPPING

using namespace UnitTesting;

#define TEST( x ) test( ( x ), "FAIL: " #x )

CreateUnitTest( TestThreadSafeRefCountSerial, "Platform/ThreadSafeRefCount/Serial" )
{
   struct TestObject : public ThreadSafeRefCount< TestObject >
   {
      static bool smDeleted;

      TestObject()
      {
         smDeleted = false;
      }
      ~TestObject()
      {
         smDeleted = true;
      }
   };

   typedef ThreadSafeRef< TestObject > TestObjectRef;

   void run()
   {
      TestObjectRef ref1 = new TestObject;
      TEST( !ref1->isShared() );
      TEST( ref1 != NULL );

      TestObjectRef ref2 = ref1;
      TEST( ref1->isShared() );
      TEST( ref2->isShared() );
      TEST( ref1 == ref2 );

      ref1 = NULL;
      TEST( !ref2->isShared() );

      ref2 = NULL;
      TEST( TestObject::smDeleted );
   }
};

bool TestThreadSafeRefCountSerial::TestObject::smDeleted;

CreateUnitTest( TestThreadSafeRefCountConcurrent, "Platform/ThreadSafeRefCount/Concurrent" )
{
public:
   typedef TestThreadSafeRefCountConcurrent TestType;
   enum
   {
      NUM_ADD_REFS_PER_THREAD = 1000,
      NUM_EXTRA_REFS_PER_THREAD = 1000,
      NUM_THREADS = 10
   };
   
   class TestObject : public ThreadSafeRefCount< TestObject >
   {
      public:
   };
   
   ThreadSafeRef< TestObject > mRef;
   
   class TestThread : public Thread
   {
      public:
         TestType* mTest;
         Vector< ThreadSafeRef< TestObject > > mExtraRefs;
         
         TestThread( TestType* test )
            : mTest( test ) {}
         
         void run( void* arg )
         {
            if( !arg )
            {
               for( U32 i = 0; i < NUM_ADD_REFS_PER_THREAD; ++ i )
                  mTest->mRef->addRef();
                  
               mExtraRefs.setSize( NUM_EXTRA_REFS_PER_THREAD );
               for( U32 i = 0; i < NUM_EXTRA_REFS_PER_THREAD; ++ i )
                  mExtraRefs[ i ] = mTest->mRef;
            }
            else
            {
               mExtraRefs.clear();
               
               for( U32 i = 0; i < NUM_ADD_REFS_PER_THREAD; ++ i )
                  mTest->mRef->release();
            }
         } 
   };
   
   void run()
   {
      mRef = new TestObject;
      TEST( mRef->getRefCount() == 2 ); // increments of 2
      
      Vector< TestThread* > threads;
      threads.setSize( NUM_THREADS );
      
      // Create threads.
      for( U32 i = 0; i < NUM_THREADS; ++ i )
         threads[ i ] = new TestThread( this );
         
      // Run phase 1: create references.
      for( U32 i = 0; i < NUM_THREADS; ++ i )
         threads[ i ]->start( NULL );
               
      // Wait for completion.
      for( U32 i = 0; i < NUM_THREADS; ++ i )
         threads[ i ]->join();
         
      Con::printf( "REF: %i", mRef->getRefCount() );
      TEST( mRef->getRefCount() == 2 + ( ( NUM_ADD_REFS_PER_THREAD + NUM_EXTRA_REFS_PER_THREAD ) * NUM_THREADS * 2 ) );

      // Run phase 2: release references.
      for( U32 i = 0; i < NUM_THREADS; ++ i )
         threads[ i ]->start( ( void* ) 1 );
         
      // Wait for completion.
      for( U32 i = 0; i < NUM_THREADS; ++ i )
      {
         threads[ i ]->join();
         delete threads[ i ];
      }
      
      TEST( mRef->getRefCount() == 2 ); // increments of two

      mRef = NULL;
   }
};

CreateUnitTest( TestThreadSafeRefCountTagging, "Platform/ThreadSafeRefCount/Tagging" )
{
   struct TestObject : public ThreadSafeRefCount< TestObject > {};

   typedef ThreadSafeRef< TestObject > TestObjectRef;

   void run()
   {
      TestObjectRef ref;

      TEST( !ref.isTagged() );
      TEST( !ref );
      TEST( !ref.ptr() );
      
      TEST( ref.trySetFromTo( ref, NULL ) );
      TEST( !ref.isTagged() );

      TEST( ref.trySetFromTo( ref, NULL, TestObjectRef::TAG_Set ) );
      TEST( ref.isTagged() );
      TEST( ref.trySetFromTo( ref, NULL, TestObjectRef::TAG_Set ) );
      TEST( ref.isTagged() );

      TEST( ref.trySetFromTo( ref, NULL, TestObjectRef::TAG_Unset ) );
      TEST( !ref.isTagged() );
      TEST( ref.trySetFromTo( ref, NULL, TestObjectRef::TAG_Unset ) );
      TEST( !ref.isTagged() );

      TEST( ref.trySetFromTo( ref, NULL, TestObjectRef::TAG_SetOrFail ) );
      TEST( ref.isTagged() );
      TEST( !ref.trySetFromTo( ref, NULL, TestObjectRef::TAG_SetOrFail ) );
      TEST( ref.isTagged() );
      TEST( !ref.trySetFromTo( ref, NULL, TestObjectRef::TAG_FailIfSet ) );

      TEST( ref.trySetFromTo( ref, NULL, TestObjectRef::TAG_UnsetOrFail ) );
      TEST( !ref.isTagged() );
      TEST( !ref.trySetFromTo( ref, NULL, TestObjectRef::TAG_UnsetOrFail ) );
      TEST( !ref.isTagged() );
      TEST( !ref.trySetFromTo( ref, NULL, TestObjectRef::TAG_FailIfUnset ) );

      TestObjectRef objectA = new TestObject;
      TestObjectRef objectB = new TestObject;

      TEST( !objectA->isShared() );
      TEST( !objectB->isShared() );

      ref = objectA;
      TEST( !ref.isTagged() );
      TEST( ref == objectA );
      TEST( ref == objectA.ptr() );
      TEST( objectA->isShared() );

      TEST( ref.trySetFromTo( objectA, objectB, TestObjectRef::TAG_Set ) );
      TEST( ref.isTagged() );
      TEST( ref == objectB );
      TEST( ref == objectB.ptr() );
      TEST( objectB->isShared() );
      TEST( !objectA->isShared() );

      TEST( ref.trySetFromTo( ref, objectA ) );
      TEST( ref.isTagged() );
      TEST( ref == objectA );
      TEST( ref == objectA.ptr() );
   }
};

#endif // !TORQUE_SHIPPING
