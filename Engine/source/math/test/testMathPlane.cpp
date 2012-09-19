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
#include "math/mPlane.h"
#include "math/mRandom.h"


#ifndef TORQUE_SHIPPING

using namespace UnitTesting;

#define TEST( x ) test( ( x ), "FAIL: " #x )
#define XTEST( t, x ) t->test( ( x ), "FAIL: " #x )

CreateUnitTest( TestMathPlane, "Math/Plane" )
{
   static F32 randF()
   {
      return gRandGen.randF( -1.f, 1.f );
   }

   void test_whichSide()
   {
      for( U32 i = 0; i < 100; ++ i )
      {
         Point3F position( randF(), randF(), randF() );
         Point3F normal( randF(), randF(), randF() );

         PlaneF p1( position, normal );

         TEST( p1.whichSide( position + normal ) == PlaneF::Front );
         TEST( p1.whichSide( position + ( - normal ) ) == PlaneF::Back );
         TEST( p1.whichSide( position ) == PlaneF::On );
      }
   }

   void test_distToPlane()
   {
      for( U32 i = 0; i < 100; ++ i )
      {
         Point3F position( randF(), randF(), randF() );
         Point3F normal( randF(), randF(), randF() );

         PlaneF p1( position, normal );

         TEST( mIsEqual( p1.distToPlane( position + normal ), normal.len() ) );
         TEST( mIsEqual( p1.distToPlane( position + ( - normal ) ), - normal.len() ) );
         TEST( mIsZero( p1.distToPlane( position ) ) );
      }
   }

   void run()
   {
      test_whichSide();
      test_distToPlane();
   }
};

#endif // !TORQUE_SHIPPING
