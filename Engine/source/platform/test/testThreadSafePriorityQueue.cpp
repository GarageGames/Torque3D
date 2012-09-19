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
#include "platform/threads/threadSafePriorityQueue.h"
#include "platform/threads/thread.h"
#include "core/util/tVector.h"
#include "console/console.h"


#ifndef TORQUE_SHIPPING

using namespace UnitTesting;

#define TEST( x ) test( ( x ), "FAIL: " #x )
#define XTEST( t, x ) t->test( ( x ), "FAIL: " #x )


// Test queue without concurrency.

CreateUnitTest( TestThreadSafePriorityQueueSerial, "Platform/ThreadSafePriorityQueue/Serial" )
{
   struct Value
   {
      F32 mPriority;
      U32 mIndex;

      Value() {}
      Value( F32 priority, U32 index )
         : mPriority( priority ), mIndex( index ) {}
   };

   template< bool SORT_MIN_TO_MAX >
   void test1()
   {
      Vector< Value > values;

      values.push_back( Value( 0.2f, 2 ) );
      values.push_back( Value( 0.7f, 7 ) );
      values.push_back( Value( 0.4f, 4 ) );
      values.push_back( Value( 0.6f, 6 ) );
      values.push_back( Value( 0.1f, 1 ) );
      values.push_back( Value( 0.5f, 5 ) );
      values.push_back( Value( 0.3f, 3 ) );
      values.push_back( Value( 0.8f, 8 ) );
      values.push_back( Value( 0.6f, 6 ) );
      values.push_back( Value( 0.9f, 9 ) );
      values.push_back( Value( 0.0f, 0 ) );

      const S32 min = 0;
      const S32 max = 9;

      ThreadSafePriorityQueue< U32, F32, SORT_MIN_TO_MAX > queue;

      for( U32 i = 0; i < values.size(); ++ i )
         queue.insert( values[ i ].mPriority, values[ i ].mIndex );

      TEST( !queue.isEmpty() );

      S32 index;
      if( SORT_MIN_TO_MAX )
         index = min - 1;
      else
         index = max + 1;

      for( U32 i = 0; i < values.size(); ++ i )
      {
         U32 value;
         TEST( queue.takeNext( value ) );

         if( value != index )
         {
            if( SORT_MIN_TO_MAX )
               index ++;
            else
               index --;
         }

         TEST( value == index );
      }
   }

   void run()
   {
      test1< true >();
      test1< false >();
   }
};

// Test queue with concurrency.

CreateUnitTest( TestThreadSafePriorityQueueConcurrent, "Platform/ThreadSafePriorityQueue/Concurrent" )
{
public:
   typedef TestThreadSafePriorityQueueConcurrent TestType;

   enum
   {
      DEFAULT_NUM_VALUES = 100000,
      DEFAULT_NUM_CONSUMERS = 10,
      DEFAULT_NUM_PRODUCERS = 10
   };

   struct Value : public ThreadSafeRefCount< Value >
   {
      U32   mIndex;
      F32   mPriority;
      bool  mCheck;

      Value() : mCheck( false ) {}
      Value( U32 index, F32 priority )
         : mIndex( index ), mPriority( priority ), mCheck( false ) {}
   };

   typedef ThreadSafeRef< Value > ValueRef;

   U32 mProducerIndex;
   U32 mConsumerIndex;
   ThreadSafePriorityQueue< ValueRef > mQueue;
   Vector< ValueRef > mValues;

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
               F32 priority = Platform::getRandom();
               ValueRef val = new Value( index, priority );
               test->mValues[ index ] = val;
               test->mQueue.insert( priority, val );
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
            if( t->mQueue.takeNext( value ) )
            {
               dFetchAndAdd( t->mConsumerIndex, 1 );
               XTEST( t, t->mValues[ value->mIndex ] == value );
               value->mCheck = true;
            }
            else
               Platform::sleep( 5 );
         }
         Platform::outputDebugString( "Stopping ConsumerThread" );
      }
   };

   void run()
   {
      U32 numValues = Con::getIntVariable( "$testThreadSafePriorityQueue::numValues", DEFAULT_NUM_VALUES );
      U32 numConsumers = Con::getIntVariable( "$testThreadSafePriorityQueue::numConsumers", DEFAULT_NUM_CONSUMERS );
      U32 numProducers = Con::getIntVariable( "$testThreadSafePriorityQueue::numProducers", DEFAULT_NUM_PRODUCERS );

      mProducerIndex = 0;
      mConsumerIndex = 0;
      mValues.setSize( numValues );

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
      {
         TEST( mValues[ i ] != NULL );
         if( mValues[ i ] != NULL )
            TEST( mValues[ i ]->mCheck );
      }

      mValues.clear();
   }
};

#endif // !TORQUE_SHIPPING
