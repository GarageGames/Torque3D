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
#include "collision/vertexPolyList.h"


VertexPolyList::VertexPolyList()
{
   VECTOR_SET_ASSOCIATION(mVertexList);
   mVertexList.reserve(100);

   mCurrObject       = NULL;
   mBaseMatrix       = MatrixF::Identity;
   mMatrix           = MatrixF::Identity;
   mTransformMatrix  = MatrixF::Identity;
   mScale.set(1.0f, 1.0f, 1.0f);

   mPlaneTransformer.setIdentity();

   mInterestNormalRegistered = false;
}

void VertexPolyList::clear()
{
   mVertexList.clear();
}

const PlaneF& VertexPolyList::getIndexedPlane(const U32 index)
{
   static const PlaneF dummy( 0, 0, 0, -1 );
   return dummy;
}

U32 VertexPolyList::addPoint( const Point3F &p )
{
   // Apply the transform
   Point3F tp = p * mScale;
   mMatrix.mulP( tp );

   Vector<Point3F>::iterator iter = mVertexList.begin();
   for ( ; iter != mVertexList.end(); iter++ )
   {
      if ( iter->equal( tp ) )
         return iter - mVertexList.begin();
   }

   mVertexList.push_back( tp );
   return mVertexList.size() - 1;
}
