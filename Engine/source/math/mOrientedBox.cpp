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
#include "math/mOrientedBox.h"

#include "math/mMatrix.h"



//-----------------------------------------------------------------------------

bool OrientedBox3F::isContained( const Point3F& point ) const
{
   Point3F distToCenter = point - getCenter();
   for( U32 i = 0; i < 3; ++ i )
   {
      F32 coeff = mDot( distToCenter, getAxis( i ) );
      if( mFabs( coeff ) > getHalfExtents()[ i ] )
         return false;
   }

   return true;
}

//-----------------------------------------------------------------------------

void OrientedBox3F::set( const MatrixF& transform, const Point3F& extents )
{
   mCenter = transform.getPosition();

   mAxes[ RightVector ] = transform.getRightVector();
   mAxes[ ForwardVector ] = transform.getForwardVector();
   mAxes[ UpVector ] = transform.getUpVector();

   mHalfExtents = extents * 0.5f;

   _initPoints();
}

//-----------------------------------------------------------------------------

void OrientedBox3F::set( const MatrixF& transform, const Box3F& aabb )
{
   mCenter = aabb.getCenter();
   transform.mulP( mCenter );

   mAxes[ RightVector ] = transform.getRightVector();
   mAxes[ ForwardVector ] = transform.getForwardVector();
   mAxes[ UpVector ] = transform.getUpVector();

   mHalfExtents[ 0 ] = aabb.len_x() / 2.f;
   mHalfExtents[ 1 ] = aabb.len_y() / 2.f;
   mHalfExtents[ 2 ] = aabb.len_z() / 2.f;

   _initPoints();
}

//-----------------------------------------------------------------------------

void OrientedBox3F::_initPoints()
{
   const Point3F right = mAxes[ RightVector ] * mHalfExtents.x;
   const Point3F forward = mAxes[ ForwardVector ] * mHalfExtents.y;
   const Point3F up = mAxes[ UpVector ] * mHalfExtents.z;

   mPoints[ NearBottomLeft ] = mCenter - forward - right - up;
   mPoints[ NearBottomRight ] = mCenter - forward + right - up;
   mPoints[ NearTopLeft ] = mCenter - forward - right + up;
   mPoints[ NearTopRight ] = mCenter - forward + right + up;

   mPoints[ FarBottomLeft ] = mCenter + forward - right - up;
   mPoints[ FarBottomRight ] = mCenter + forward + right - up;
   mPoints[ FarTopLeft ] = mCenter + forward - right + up;
   mPoints[ FarTopRight ] = mCenter + forward + right + up;
}
