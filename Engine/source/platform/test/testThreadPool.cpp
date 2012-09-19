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
#include "platform/threads/threadPool.h"
#include "console/console.h"
#include "core/util/tVector.h"

#ifndef TORQUE_SHIPPING

using namespace UnitTesting;

#define TEST( x ) test( ( x ), "FAIL: " #x )

// Simple test that creates and verifies an array of numbers using
// thread pool work items.

CreateUnitTest( TestThreadPool, "Platform/ThreadPool/Simple" )
{
   enum { DEFAULT_NUM_ITEMS = 4000 };
   
   static Vector< U32 > results;
   
   struct TestItem : public ThreadPool::WorkItem
   {
         typedef ThreadPool::WorkItem Parent;
         
         U32 mIndex;
         
         TestItem( U32 index )
            : mIndex( index ) {}
      
      protected:
         virtual void execute()
         {
            results[ mIndex ] = mIndex;
         }
   };
   
   void run()
   {
      U32 numItems = Con::getIntVariable( "$testThreadPool::numValues", DEFAULT_NUM_ITEMS );
      ThreadPool* pool = &ThreadPool::GLOBAL();
      results.setSize( numItems );

      for( U32 i = 0; i < numItems; ++ i )
         results[ i ] = U32( -1 );
      
      for( U32 i = 0; i < numItems; ++ i )
      {
         ThreadSafeRef< TestItem > item( new TestItem( i ) );
         pool->queueWorkItem( item );
      }
      
      pool->flushWorkItems();
      
      for( U32 i = 0; i < numItems; ++ i )
         test( results[ i ] == i, "result mismatch" );
         
      results.clear();
   }
};

Vector< U32 > TestThreadPool::results( __FILE__, __LINE__ );

#endif // !TORQUE_SHIPPING
