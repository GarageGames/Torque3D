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
#include "platform/threads/threadSafeDeque.h"
#include "platform/threads/thread.h"
#include "core/util/tVector.h"
#include "console/console.h"


#ifndef TORQUE_SHIPPING

using namespace UnitTesting;

#define TEST( x ) test( ( x ), "FAIL: " #x )
#define XTEST( t, x ) t->test( ( x ), "FAIL: " #x )


// Test deque without concurrency.

CreateUnitTest( TestThreadSafeDequeSerial, "Platform/ThreadSafeDeque/Serial" )
{
   void test1()
   {
      ThreadSafeDeque< char > deque;
      String str = "teststring";

      for( U32 i = 0; i < str.length(); ++ i )
         deque.pushBack( str[ i ] );

      TEST( !deque.isEmpty() );

      for( U32 i = 0; i < str.length(); ++ i )
      {
         char ch;
         TEST( deque.tryPopFront( ch ) && ch == str[ i ] );
      }
   }

   void test2()
   {
      ThreadSafeDeque< char > deque;
      String str = "teststring";

      const char* p1 = str.c_str() + 4;
      const char* p2 = p1 + 1;
      while( *p2 )
      {
         deque.pushFront( *p1 );
         deque.pushBack( *p2 );

         -- p1;
         ++ p2;
      }

#ifdef TORQUE_DEBUG
      deque.dumpDebug();
#endif

      for( U32 i = 0; i < str.length(); ++ i )
      {
         char ch;
         TEST( deque.tryPopFront( ch ) && ch == str[ i ] );
      }
   }

   void test3()
   {
      ThreadSafeDeque< char > deque;
      String str = "teststring";

      const char* p1 = str.c_str() + 4;
      const char* p2 = p1 + 1;
      while( *p2 )
      {
         deque.pushFront( *p1 );
         deque.pushBack( *p2 );

         -- p1;
         ++ p2;
      }

#ifdef TORQUE_DEBUG
      deque.dumpDebug();
#endif

      for( S32 i = ( str.length() - 1 ); i >= 0; -- i )
      {
         char ch;
         TEST( deque.tryPopBack( ch ) && ch == str[ i ] );
      }
   }

   void test4()
   {
      ThreadSafeDeque< char > deque;
      char ch;

      TEST( deque.isEmpty() );
      
      deque.pushFront( 'a' );
      TEST( !deque.isEmpty() );
      TEST( deque.tryPopFront( ch ) );
      TEST( ch == 'a' );
      
      deque.pushBack( 'a' );
      TEST( !deque.isEmpty() );
      TEST( deque.tryPopFront( ch ) );
      TEST( ch == 'a' );

      deque.pushBack( 'a' );
      TEST( !deque.isEmpty() );
      TEST( deque.tryPopBack( ch ) );
      TEST( ch == 'a' );

      deque.pushFront( 'a' );
      TEST( !deque.isEmpty() );
      TEST( deque.tryPopBack( ch ) );
      TEST( ch == 'a' );
   }

   void run()
   {
      test1();
      test2();
      test3();
      test4();
   }
};

// Test deque in a concurrent setting.

CreateUnitTest( TestThreadSafeDequeConcurrentSimple, "Platform/ThreadSafeDeque/ConcurrentSimple" )
{
public:
   typedef TestThreadSafeDequeConcurrentSimple TestType;

   enum
   {
      DEFAULT_NUM_VALUES = 100000,
   };

   struct Value : public ThreadSafeRefCount< Value >
   {
      U32 mIndex;
      U32 mTick;

      Value() {}
      Value( U32 index, U32 tick )
         : mIndex( index ), mTick( tick ) {}
   };

   typedef ThreadSafeRef< Value > ValueRef;

   struct Deque : public ThreadSafeDeque< ValueRef >
   {
      typedef ThreadSafeDeque<ValueRef> Parent;

      U32 mPushIndex;
      U32 mPopIndex;

      Deque()
         : mPushIndex( 0 ), mPopIndex( 0 ) {}

      void pushBack( const ValueRef& value )
      {
         AssertFatal( value->mIndex == mPushIndex, "index out of line" );
         mPushIndex ++;
         Parent::pushBack( value );
      }
      bool tryPopFront( ValueRef& outValue )
      {
         if( Parent::tryPopFront( outValue ) )
         {
            AssertFatal( outValue->mIndex == mPopIndex, "index out of line" );
            mPopIndex ++;
            return true;
         }
         else
            return false;
      }
   };

   Deque mDeque;
   Vector< U32 > mValues;

   struct ProducerThread : public Thread
   {
      ProducerThread( TestType* test )
         : Thread( 0, test ) {}

      virtual void run( void* arg )
      {
         _setName( "ProducerThread" );
         Platform::outputDebugString( "Starting ProducerThread" );
         
         TestType* test = ( TestType* ) arg;

         for( U32 i = 0; i < test->mValues.size(); ++ i )
         {
            U32 tick = Platform::getRealMilliseconds();
            test->mValues[ i ] = tick;

            ValueRef val = new Value( i, tick );
            test->mDeque.pushBack( val );
         }
         Platform::outputDebugString( "Stopping ProducerThread" );
      }
   };
   struct ConsumerThread : public Thread
   {
      ConsumerThread( TestType* test )
         : Thread( 0, test ) {}

      virtual void run( void* arg )
      {
         _setName( "ConsumerThread" );
         Platform::outputDebugString( "Starting CosumerThread" );
         TestType* t = ( TestType* ) arg;

         for( U32 i = 0; i < t->mValues.size(); ++ i )
         {
            ValueRef value;
            while( !t->mDeque.tryPopFront( value ) );

            XTEST( t, value->mIndex == i );
            XTEST( t, t->mValues[ i ] == value->mTick );
         }
         Platform::outputDebugString( "Stopping ConsumerThread" );
      }
   };

   void run()
   {
      U32 numValues = Con::getIntVariable( "$testThreadSafeDeque::numValues", DEFAULT_NUM_VALUES );
      mValues.setSize( numValues );

      ProducerThread pThread( this );
      ConsumerThread cThread( this );
      
      pThread.start();
      cThread.start();
      
      pThread.join();
      cThread.join();

      mValues.clear();
   }
};

CreateUnitTest( TestThreadSafeDequeConcurrent, "Platform/ThreadSafeDeque/Concurrent" )
{
public:
   typedef TestThreadSafeDequeConcurrent TestType;

   enum
   {
      DEFAULT_NUM_VALUES = 100000,
      DEFAULT_NUM_CONSUMERS = 10,
      DEFAULT_NUM_PRODUCERS = 10
   };

   struct Value : public ThreadSafeRefCount< Value >
   {
      U32 mIndex;
      U32 mTick;

      Value() {}
      Value( U32 index, U32 tick )
         : mIndex( index ), mTick( tick ) {}
   };

   typedef ThreadSafeRef< Value > ValueRef;

   U32 mProducerIndex;
   U32 mConsumerIndex;
   ThreadSafeDeque< ValueRef > mDeque;
   Vector< U32 > mValues;

   struct ProducerThread : public Thread
   {
      ProducerThread( TestType* test )
         : Thread( 0, test ) {}

      virtual void run( void* arg )
      {
         _setName( "ProducerThread" );
         Platform::outputDebugString( "Starting ProducerThread" );
         TestType* test = ( TestType* ) arg;

         while( 1 )
         {
            U32 index = test->mProducerIndex;
            if( index == test->mValues.size() )
               break;
               
            if( dCompareAndSwap( test->mProducerIndex, index, index + 1 ) )
            {
               U32 tick = Platform::getRealMilliseconds();
               test->mValues[ index ] = tick;

               ValueRef val = new Value( index, tick );
               test->mDeque.pushBack( val );
            }
         }
         Platform::outputDebugString( "Stopping ProducerThread" );
      }
   };
   struct ConsumerThread : public Thread
   {
      ConsumerThread( TestType* test )
         : Thread( 0, test ) {}

      virtual void run( void* arg )
      {
         _setName( "ConsumerThread" );
         Platform::outputDebugString( "Starting ConsumerThread" );
         TestType* t = ( TestType* ) arg;

         while( t->mConsumerIndex < t->mValues.size() )
         {
            ValueRef value;
            if( t->mDeque.tryPopFront( value ) )
            {
               dFetchAndAdd( t->mConsumerIndex, 1 );
               XTEST( t, t->mValues[ value->mIndex ] == value->mTick );
               t->mValues[ value->mIndex ] = 0;
            }
         }
         
         Platform::outputDebugString( "Stopping ConsumerThread" );
      }
   };

   void run()
   {
      U32 numValues = Con::getIntVariable( "$testThreadSafeDeque::numValues", DEFAULT_NUM_VALUES );
      U32 numConsumers = Con::getIntVariable( "$testThreadSafeDeque::numConsumers", DEFAULT_NUM_CONSUMERS );
      U32 numProducers = Con::getIntVariable( "$testThreadSafeDeque::numProducers", DEFAULT_NUM_PRODUCERS );

      mProducerIndex = 0;
      mConsumerIndex = 0;
      mValues.setSize( numValues );

      U32 tick = Platform::getRealMilliseconds();
      for( U32 i = 0; i < numValues; ++ i )
         mValues[ i ] = tick;

      Vector< ProducerThread* > producers;
      Vector< ConsumerThread* > consumers;

      producers.setSize( numProducers );
      consumers.setSize( numConsumers );

      for( U32 i = 0; i < numProducers; ++ i )
      {
         producers[ i ] = new ProducerThread( this );
         producers[ i ]->start();
      }
      for( U32 i = 0; i < numConsumers; ++ i )
      {
         consumers[ i ] = new ConsumerThread( this );
         consumers[ i ]->start();
      }

      for( U32 i = 0; i < numProducers; ++ i )
      {
         producers[ i ]->join();
         delete producers[ i ];
      }
      for( U32 i = 0; i < numConsumers; ++ i )
      {
         consumers[ i ]->join();
         delete consumers[ i ];
      }

      for( U32 i = 0; i < mValues.size(); ++ i )
         TEST( mValues[ i ] == 0 );

      mValues.clear();
   }
};

#endif // !TORQUE_SHIPPING
