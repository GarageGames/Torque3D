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
#include "collision/concretePolyList.h"

#include "math/mMath.h"
#include "console/console.h"
#include "gfx/gfxDevice.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxStateBlock.h"


//----------------------------------------------------------------------------

ConcretePolyList::ConcretePolyList()
{
   VECTOR_SET_ASSOCIATION(mPolyList);
   VECTOR_SET_ASSOCIATION(mVertexList);
   VECTOR_SET_ASSOCIATION(mIndexList);
   VECTOR_SET_ASSOCIATION(mPolyPlaneList);

   mIndexList.reserve(100);
}

ConcretePolyList::~ConcretePolyList()
{

}


//----------------------------------------------------------------------------
void ConcretePolyList::clear()
{
   // Only clears internal data
   mPolyList.clear();
   mVertexList.clear();
   mIndexList.clear();
   mPolyPlaneList.clear();
}

//----------------------------------------------------------------------------

U32 ConcretePolyList::addPoint(const Point3F& p)
{
   mVertexList.increment();
   Point3F& v = mVertexList.last();
   v.x = p.x * mScale.x;
   v.y = p.y * mScale.y;
   v.z = p.z * mScale.z;
   mMatrix.mulP(v);

   return mVertexList.size() - 1;
}


U32 ConcretePolyList::addPlane(const PlaneF& plane)
{
   mPolyPlaneList.increment();
   mPlaneTransformer.transform(plane, mPolyPlaneList.last());

   return mPolyPlaneList.size() - 1;
}


//----------------------------------------------------------------------------

void ConcretePolyList::begin(BaseMatInstance* material,U32 surfaceKey)
{
   mPolyList.increment();
   Poly& poly = mPolyList.last();
   poly.object = mCurrObject;
   poly.material = material;
   poly.vertexStart = mIndexList.size();
   poly.surfaceKey = surfaceKey;
}


//----------------------------------------------------------------------------

void ConcretePolyList::plane(U32 v1,U32 v2,U32 v3)
{
   mPolyList.last().plane.set(mVertexList[v1],
      mVertexList[v2],mVertexList[v3]);
}

void ConcretePolyList::plane(const PlaneF& p)
{
   mPlaneTransformer.transform(p, mPolyList.last().plane); 
}

void ConcretePolyList::plane(const U32 index)
{
   AssertFatal(index < mPolyPlaneList.size(), "Out of bounds index!");
   mPolyList.last().plane = mPolyPlaneList[index];
}

const PlaneF& ConcretePolyList::getIndexedPlane(const U32 index)
{
   AssertFatal(index < mPolyPlaneList.size(), "Out of bounds index!");
   return mPolyPlaneList[index];
}


//----------------------------------------------------------------------------

void ConcretePolyList::vertex(U32 vi)
{
   mIndexList.push_back(vi);
}


//----------------------------------------------------------------------------

bool ConcretePolyList::isEmpty() const
{
   return mPolyList.empty();
}

void ConcretePolyList::end()
{
   Poly& poly = mPolyList.last();
   poly.vertexCount = mIndexList.size() - poly.vertexStart;
}

void ConcretePolyList::render()
{
   GFXStateBlockDesc solidZDisable;
   solidZDisable.setCullMode( GFXCullNone );
   solidZDisable.setZReadWrite( false, false );
   GFXStateBlockRef sb = GFX->createStateBlock( solidZDisable );
   GFX->setStateBlock( sb );

   PrimBuild::color3i( 255, 0, 255 );

   Poly *p;
   Point3F *pnt;

   for ( p = mPolyList.begin(); p < mPolyList.end(); p++ )
   {
      PrimBuild::begin( GFXLineStrip, p->vertexCount + 1 );      

      for ( U32 i = 0; i < p->vertexCount; i++ )
      {
         pnt = &mVertexList[mIndexList[p->vertexStart + i]];
         PrimBuild::vertex3fv( pnt );
      }

      pnt = &mVertexList[mIndexList[p->vertexStart]];
      PrimBuild::vertex3fv( pnt );

      PrimBuild::end();
   }   
}

void ConcretePolyList::triangulate()
{
   PROFILE_SCOPE( ConcretePolyList_Triangulate );

   // Build into a new polylist and index list.
   //
   // TODO: There are potential performance issues
   // here as we're not reserving enough space for
   // new generated triangles.
   //
   // We need to either over estimate and shrink or 
   // better yet fix vector to internally grow in 
   // large chunks.
   //
   PolyList polyList;
   polyList.reserve( mPolyList.size() );
   IndexList indexList;
   indexList.reserve( mIndexList.size() );
   
   U32 j, numTriangles;

   //
   PolyList::const_iterator polyIter = mPolyList.begin();
   for ( ; polyIter != mPolyList.end(); polyIter++ )
   {
      const Poly &poly = *polyIter;

      // How many triangles in this poly?
      numTriangles = poly.vertexCount - 2;        

      // Build out the triangles.
      for ( j = 0; j < numTriangles; j++ )
      {
         polyList.increment();
         
         Poly &triangle = polyList.last();
         triangle = poly;
         triangle.vertexCount = 3;
         triangle.vertexStart = indexList.size();

         indexList.push_back( mIndexList[ poly.vertexStart ] );
         indexList.push_back( mIndexList[ poly.vertexStart + 1 + j ] );
         indexList.push_back( mIndexList[ poly.vertexStart + 2 + j ] );
      }
   } 

   mPolyList = polyList;
   mIndexList = indexList;
}