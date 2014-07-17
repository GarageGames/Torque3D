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
#include "core/util/tFixedSizeDeque.h"

TEST(FixedSizeDeque, FixedSizeDeque)
{
   enum { DEQUE_SIZE = 3 };
   FixedSizeDeque< U32 > deque( DEQUE_SIZE );

   EXPECT_EQ( deque.capacity(), DEQUE_SIZE );
   EXPECT_EQ( deque.size(), 0 );

   deque.pushFront( 1 );
   EXPECT_EQ( deque.capacity(), ( DEQUE_SIZE - 1 ) );
   EXPECT_EQ( deque.size(), 1 );
   EXPECT_FALSE( deque.isEmpty() );

   deque.pushBack( 2 );
   EXPECT_EQ( deque.capacity(), ( DEQUE_SIZE - 2 ) );
   EXPECT_EQ( deque.size(), 2 );

   EXPECT_EQ( deque.popFront(), 1 );
   EXPECT_EQ( deque.popFront(), 2 );
   EXPECT_TRUE( deque.isEmpty() );
};

#endif