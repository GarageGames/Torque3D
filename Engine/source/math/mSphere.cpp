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

#include "platform/platform.h"
#include "math/mSphere.h"

#include "math/mMatrix.h"


bool SphereF::intersectsRay( const Point3F &start, const Point3F &end ) const
{
   MatrixF worldToObj( true );
   worldToObj.setPosition( center );
   worldToObj.inverse();

   VectorF dir = end - start;
   dir.normalize();

   Point3F tmpStart = start;
   worldToObj.mulP( tmpStart ); 

   //Compute A, B and C coefficients
   F32 a = mDot(dir, dir);
   F32 b = 2 * mDot(dir, tmpStart);
   F32 c = mDot(tmpStart, tmpStart) - (radius * radius);

   //Find discriminant
   F32 disc = b * b - 4 * a * c;

   // if discriminant is negative there are no real roots, so return 
   // false as ray misses sphere
   if ( disc < 0 )
      return false;

   // compute q as described above
   F32 distSqrt = mSqrt( disc );
   F32 q;
   if ( b < 0 )
      q = (-b - distSqrt)/2.0;
   else
      q = (-b + distSqrt)/2.0;

   // compute t0 and t1
   F32 t0 = q / a;
   F32 t1 = c / q;

   // make sure t0 is smaller than t1
   if ( t0 > t1 )
   {
      // if t0 is bigger than t1 swap them around
      F32 temp = t0;
      t0 = t1;
      t1 = temp;
   }

   // This function doesn't use it
   // but t would be the interpolant
   // value for getting the exact
   // intersection point, by interpolating
   // start to end by t.
   F32 t = 0;
   TORQUE_UNUSED(t);

   // if t1 is less than zero, the object is in the ray's negative direction
   // and consequently the ray misses the sphere
   if ( t1 < 0 )
      return false;

   // if t0 is less than zero, the intersection point is at t1
   if ( t0 < 0 ) // t = t1;     
      return true;
   else // else the intersection point is at t0
      return true; // t = t0;
}
