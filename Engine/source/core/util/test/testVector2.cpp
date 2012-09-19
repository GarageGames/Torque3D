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
#include "console/console.h"
#include "core/util/tVector.h"


#ifndef TORQUE_SHIPPING

using namespace UnitTesting;

#define TEST( x ) test( ( x ), "FAIL: " #x )
#define XTEST( t, x ) t->test( ( x ), "FAIL: " #x )

CreateUnitTest( TestVector, "Util/Vector" )
{
   bool dtorVals[ 10 ];
   struct Dtor
   {
      bool* ptr;
      Dtor() {}
      Dtor( bool* ptr )
         : ptr( ptr ) { *ptr = false; }
      ~Dtor()
      {
         *ptr = true;
      }
   };
   void testDestruction()
   {
      Vector< Dtor > v;
      
      for( U32 i = 0; i < 9; ++ i )
         v.push_back( Dtor( &dtorVals[ i ] ) );
         
      v.decrement();
      v.decrement( 2 );
      v.pop_back();
      v.increment();
      v.last() = Dtor( &dtorVals[ 9 ] );
      v.clear();
      
      TEST( dtorVals[ 0 ] );
      TEST( dtorVals[ 1 ] );
      TEST( dtorVals[ 2 ] );
      TEST( dtorVals[ 3 ] );
      TEST( dtorVals[ 4 ] );
      TEST( dtorVals[ 5 ] );
      TEST( dtorVals[ 6 ] );
      TEST( dtorVals[ 7 ] );
      TEST( dtorVals[ 8 ] );
      TEST( dtorVals[ 9 ] );
   }
   
   static S32 QSORT_CALLBACK sortInts( const int* a, const int* b )
   {
      int av = *a;
      int bv = *b;
      
      if( av < bv )
         return -1;
      else if( av > bv )
         return 1;
      else
         return 0;
   }
   
   void testSort()
   {
      Vector< int > v;
      
      v.push_back( 0 );
      v.push_back( 10 );
      v.push_back( 2 );
      v.push_back( 3 );
      v.push_back( 14 );
      v.push_back( 4 );
      v.push_back( 12 );
      v.push_back( 6 );
      v.push_back( 16 );
      v.push_back( 7 );
      v.push_back( 8 );
      v.push_back( 1 );
      v.push_back( 11 );
      v.push_back( 5 );
      v.push_back( 13 );
      v.push_back( 9 );
      v.push_back( 15 );
      
      v.sort( sortInts );
      
      TEST( v[ 0 ] == 0 );
      TEST( v[ 1 ] == 1 );
      TEST( v[ 2 ] == 2 );
      TEST( v[ 3 ] == 3 );
      TEST( v[ 4 ] == 4 );
      TEST( v[ 5 ] == 5 );
      TEST( v[ 6 ] == 6 );
      TEST( v[ 7 ] == 7 );
      TEST( v[ 8 ] == 8 );
      TEST( v[ 9 ] == 9 );
      TEST( v[ 10 ] == 10 );
      TEST( v[ 11 ] == 11 );
      TEST( v[ 12 ] == 12 );
      TEST( v[ 13 ] == 13 );
      TEST( v[ 14 ] == 14 );
      TEST( v[ 15 ] == 15 );
      TEST( v[ 16 ] == 16 );
   }
   
   void run()
   {
      testSort();
      testDestruction();
   }
};

#endif
