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
#include "terrain/terrCollision.h"

#include "terrain/terrData.h"
#include "collision/abstractPolyList.h"
#include "collision/collision.h"


const F32 TerrainThickness = 0.5f;
static const U32 MaxExtent = 256;
#define MAX_FLOAT 1e20f


//----------------------------------------------------------------------------

Convex sTerrainConvexList;

// Number of vertices followed by point index
S32 sVertexList[5][5] = {
   { 3, 1,2,3 },  // 135 B
   { 3, 0,1,3 },  // 135 A
   { 3, 0,2,3 },  // 45 B
   { 3, 0,1,2 },  // 45 A
   { 4, 0,1,2,3 } // Convex square
};

// Number of edges followed by edge index pairs
S32 sEdgeList45[16][11] = {
   { 0 },                  //
   { 0 },
   { 0 },
   { 1, 0,1 },             // 0-1
   { 0 },
   { 1, 0,1 },             // 0-2
   { 1, 0,1 },             // 1-2
   { 3, 0,1,1,2,2,0 },     // 0-1,1-2,2-0
   { 0 },
   { 0,},                  //
   { 0 },
   { 1, 0,1 },             // 0-1,
   { 0, },                 //
   { 1, 0,1 },             // 0-2,
   { 1, 0,1 },             // 1-2
   { 3, 0,1,1,2,0,2 },
};

S32 sEdgeList135[16][11] = {
   { 0 },
   { 0 },
   { 0 },
   { 1, 0,1 },             // 0-1
   { 0 },
   { 0 },
   { 1, 0,1 },             // 1-2
   { 2, 0,1,1,2 },         // 0-1,1-2
   { 0 },
   { 0, },                 //
   { 1, 0,1 },             // 1-3
   { 2, 0,1,1,2 },         // 0-1,1-3,
   { 0 },                  //
   { 0 },                  //
   { 2, 0,1,2,0 },         // 1-2,3-1
   { 3, 0,1,1,2,1,3 },
};

// On split squares, the FaceA diagnal is also removed
S32 sEdgeList45A[16][11] = {
   { 0 },                  //
   { 0 },
   { 0 },
   { 1, 0,1 },             // 0-1
   { 0 },
   { 0 },                  //
   { 1, 0,1 },             // 1-2
   { 2, 0,1,1,2 },         // 0-1,1-2
   { 0 },
   { 0,},                  //
   { 0 },
   { 1, 0,1 },             // 0-1
   { 0, },                 //
   { 0, 0,1 },             //
   { 1, 0,1 },             // 1-2
   { 3, 0,1,1,2 },
};

S32 sEdgeList135A[16][11] = {
   { 0 },
   { 0 },
   { 0 },
   { 1, 0,1 },             // 0-1
   { 0 },
   { 0 },
   { 1, 0,1 },             // 1-2
   { 2, 0,1,1,2 },         // 0-1,1-2
   { 0 },
   { 0 },                  //
   { 0 },                  //
   { 1, 0,1 },             // 0-1
   { 0 },                  //
   { 0 },                  //
   { 1, 0,1 },             // 1-2
   { 3, 0,1,1,2 },
};


// Number of faces followed by normal index and vertices
S32 sFaceList45[16][9] = {
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 1, 0,0,1,2 },
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 1, 1,0,1,2 },
   { 0 },
   { 2, 0,0,1,2, 1,0,2,3 },
};

S32 sFaceList135[16][9] = {
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 0 },
   { 1, 0,0,1,2 },
   { 0 },
   { 0 },
   { 1, 1,0,1,2 },
   { 2, 0,0,1,3, 1,1,2,3 },
};


TerrainConvex::TerrainConvex() 
{
   mType = TerrainConvexType; 
}

TerrainConvex::TerrainConvex( const TerrainConvex &cv ) 
{
   mType = TerrainConvexType;

   // Only a partial copy...
   mObject = cv.mObject;
   split45 = cv.split45;
   squareId = cv.squareId;
   material = cv.material;
   point[0] = cv.point[0];
   point[1] = cv.point[1];
   point[2] = cv.point[2];
   point[3] = cv.point[3];
   normal[0] = cv.normal[0];
   normal[1] = cv.normal[1];
   box = cv.box;
}

Box3F TerrainConvex::getBoundingBox() const
{
   return box;
}

Box3F TerrainConvex::getBoundingBox(const MatrixF&, const Point3F& ) const
{
   // Function should not be called....
   return box;
}

Point3F TerrainConvex::support(const VectorF& v) const
{
   S32 *vp;
   if (halfA)
      vp = square ? sVertexList[(split45 << 1) | 1]: sVertexList[4];
   else
      vp = square ? sVertexList[(split45 << 1)]    : sVertexList[4];

   S32 *ve = vp + vp[0] + 1;
   const Point3F *bp = &point[vp[1]];
   F32 bd = mDot(*bp,v);
   for (vp += 2; vp < ve; vp++) {
      const Point3F* cp = &point[*vp];
      F32 dd = mDot(*cp,v);
      if (dd > bd) {
         bd = dd;
         bp = cp;
      }
   }
   return *bp;
}

inline bool isOnPlane(Point3F& p,PlaneF& plane)
{
   F32 dist = mDot(plane,p) + plane.d;
   return dist < 0.1 && dist > -0.1;
}

void TerrainConvex::getFeatures(const MatrixF& mat,const VectorF& n, ConvexFeature* cf)
{
   U32 i;
   cf->material = 0;
   cf->object = mObject;

   // Plane is normal n + support point
   PlaneF plane;
   plane.set(support(n),n);
   S32 vertexCount = cf->mVertexList.size();

   // Emit vertices on the plane
   S32* vertexListPointer;
   if (halfA)
      vertexListPointer = square ? sVertexList[(split45 << 1) | 1]: sVertexList[4];
   else
      vertexListPointer = square ? sVertexList[(split45 << 1)]    : sVertexList[4];

   S32 pm = 0;
   S32 numVerts = *vertexListPointer;
   vertexListPointer += 1;
   for (i = 0; i < numVerts; i++)
   {
      Point3F& cp = point[vertexListPointer[i]];
      cf->mVertexList.increment();
      mat.mulP(cp,&cf->mVertexList.last());
      pm |= 1 << vertexListPointer[i];
   }

   // Emit Edges
   S32* ep = (square && halfA)?
      (split45 ? sEdgeList45A[pm]: sEdgeList135A[pm]):
      (split45 ? sEdgeList45[pm]: sEdgeList135[pm]);

   S32 numEdges = *ep;
   S32 edgeListStart = cf->mEdgeList.size();
   cf->mEdgeList.increment(numEdges);
   ep += 1;
   for (i = 0; i < numEdges; i++)
   {
      cf->mEdgeList[edgeListStart + i].vertex[0] = vertexCount + ep[i * 2 + 0];
      cf->mEdgeList[edgeListStart + i].vertex[1] = vertexCount + ep[i * 2 + 1];
   }

   // Emit faces
   S32* fp = split45 ? sFaceList45[pm]: sFaceList135[pm];
   S32 numFaces = *fp;
   fp += 1;
   S32 faceListStart = cf->mFaceList.size();
   cf->mFaceList.increment(numFaces);
   for (i = 0; i < numFaces; i++)
   {
      ConvexFeature::Face& face = cf->mFaceList[faceListStart + i];
      face.normal = normal[fp[i * 4 + 0]];
      face.vertex[0] = vertexCount + fp[i * 4 + 1];
      face.vertex[1] = vertexCount + fp[i * 4 + 2];
      face.vertex[2] = vertexCount + fp[i * 4 + 3];
   }
}


void TerrainConvex::getPolyList(AbstractPolyList* list)
{
   list->setTransform(&mObject->getTransform(), mObject->getScale());
   list->setObject(mObject);

   // Emit vertices
   U32 array[4];
   U32 curr = 0;

   S32 numVerts;
   S32* vertsStart;
   if (halfA)
   {
      numVerts   = square ?  sVertexList[(split45 << 1) | 1][0] :  sVertexList[4][0];
      vertsStart = square ? &sVertexList[(split45 << 1) | 1][1] : &sVertexList[4][1];
   }
   else
   {
      numVerts   = square ?  sVertexList[(split45 << 1)][0] :  sVertexList[4][0];
      vertsStart = square ? &sVertexList[(split45 << 1)][1] : &sVertexList[4][1];
   }

   S32 pointMask = 0;
   for (U32 i = 0; i < numVerts; i++) {
      const Point3F& cp = point[vertsStart[i]];
      array[curr++] = list->addPoint(cp);
      pointMask |= (1 << vertsStart[i]);
   }

   S32  numFaces  = split45 ?  sFaceList45[pointMask][0] :  sFaceList135[pointMask][0];
   S32* faceStart = split45 ? &sFaceList45[pointMask][1] : &sFaceList135[pointMask][1];
   for (U32 j = 0; j < numFaces; j++) {
      S32 plane = faceStart[0];
      S32 v0    = faceStart[1];
      S32 v1    = faceStart[2];
      S32 v2    = faceStart[3];

      list->begin(0, plane);
      list->vertex(array[v0]);
      list->vertex(array[v1]);
      list->vertex(array[v2]);
      list->plane(array[v0], array[v1], array[v2]);
      list->end();

      faceStart += 4;
   }
}


//----------------------------------------------------------------------------

void TerrainBlock::buildConvex(const Box3F& box,Convex* convex)
{
   PROFILE_SCOPE( TerrainBlock_buildConvex );
   
   sTerrainConvexList.collectGarbage();

   // First check to see if the query misses the 
   // terrain elevation range.
   const Point3F &terrainPos = getPosition();
   if (  box.maxExtents.z - terrainPos.z < -TerrainThickness || 
         box.minExtents.z - terrainPos.z > fixedToFloat( mFile->getMaxHeight() ) )
      return;

   // Transform the bounding sphere into the object's coord space.  Note that this
   // not really optimal.
   Box3F osBox = box;
   mWorldToObj.mul(osBox);
   AssertWarn(mObjScale == Point3F(1, 1, 1), "Error, handle the scale transform on the terrain");

   S32 xStart = (S32)mFloor( osBox.minExtents.x / mSquareSize );
   S32 xEnd   = (S32)mCeil ( osBox.maxExtents.x / mSquareSize );
   S32 yStart = (S32)mFloor( osBox.minExtents.y / mSquareSize );
   S32 yEnd   = (S32)mCeil ( osBox.maxExtents.y / mSquareSize );
   S32 xExt = xEnd - xStart;
   if (xExt > MaxExtent)
      xExt = MaxExtent;

   U16 heightMax = floatToFixed(osBox.maxExtents.z);
   U16 heightMin = (osBox.minExtents.z < 0)? 0: floatToFixed(osBox.minExtents.z);

   const U32 BlockMask = mFile->mSize - 1;

   for ( S32 y = yStart; y < yEnd; y++ ) 
   {
      S32 yi = y & BlockMask;

      //
      for ( S32 x = xStart; x < xEnd; x++ ) 
      {
         S32 xi = x & BlockMask;

         const TerrainSquare *sq = mFile->findSquare( 0, xi, yi );

         if ( x != xi || y != yi )
            continue;

         // holes only in the primary terrain block
         if (  ( ( sq->flags & TerrainSquare::Empty ) && x == xi && y == yi ) ||
               sq->minHeight > heightMax || 
               sq->maxHeight < heightMin )
            continue;

         U32 sid = (x << 16) + (y & ((1 << 16) - 1));
         Convex *cc = 0;

         // See if the square already exists as part of the working set.
         CollisionWorkingList& wl = convex->getWorkingList();
         for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext)
            if (itr->mConvex->getType() == TerrainConvexType &&
                static_cast<TerrainConvex*>(itr->mConvex)->squareId == sid) {
               cc = itr->mConvex;
               break;
            }

         if (cc)
            continue;

         // Create a new convex.
         TerrainConvex* cp = new TerrainConvex;
         sTerrainConvexList.registerObject(cp);
         convex->addToWorkingList(cp);
         cp->halfA = true;
         cp->square = 0;
         cp->mObject = this;
         cp->squareId = sid;
         cp->material = mFile->getLayerIndex( xi, yi );
         cp->box.minExtents.set((F32)(x * mSquareSize), (F32)(y * mSquareSize), fixedToFloat( sq->minHeight ));
         cp->box.maxExtents.x = cp->box.minExtents.x + mSquareSize;
         cp->box.maxExtents.y = cp->box.minExtents.y + mSquareSize;
         cp->box.maxExtents.z = fixedToFloat( sq->maxHeight );
         mObjToWorld.mul(cp->box);

         // Build points
         Point3F* pos = cp->point;
         for (S32 i = 0; i < 4 ; i++,pos++) {
            S32 dx = i >> 1;
            S32 dy = dx ^ (i & 1);
            pos->x = (F32)((x + dx) * mSquareSize);
            pos->y = (F32)((y + dy) * mSquareSize);
            pos->z = fixedToFloat( mFile->getHeight(xi + dx, yi + dy) );
         }

         // Build normals, then split into two Convex objects if the
         // square is concave
         if ((cp->split45 = sq->flags & TerrainSquare::Split45) == true) {
            VectorF *vp = cp->point;
            mCross(vp[0] - vp[1],vp[2] - vp[1],&cp->normal[0]);
            cp->normal[0].normalize();
            mCross(vp[2] - vp[3],vp[0] - vp[3],&cp->normal[1]);
            cp->normal[1].normalize();
            if (mDot(vp[3] - vp[1],cp->normal[0]) > 0) {
               TerrainConvex* nc = new TerrainConvex(*cp);
               sTerrainConvexList.registerObject(nc);
               convex->addToWorkingList(nc);
               nc->halfA = false;
               nc->square = cp;
               cp->square = nc;
            }
         }
         else {
            VectorF *vp = cp->point;
            mCross(vp[3] - vp[0],vp[1] - vp[0],&cp->normal[0]);
            cp->normal[0].normalize();
            mCross(vp[1] - vp[2],vp[3] - vp[2],&cp->normal[1]);
            cp->normal[1].normalize();
            if (mDot(vp[2] - vp[0],cp->normal[0]) > 0) {
               TerrainConvex* nc = new TerrainConvex(*cp);
               sTerrainConvexList.registerObject(nc);
               convex->addToWorkingList(nc);
               nc->halfA = false;
               nc->square = cp;
               cp->square = nc;
            }
         }
      }
   }
}

static inline void swap(U32*& a,U32*& b)
{
   U32* t = b;
   b = a;
   a = t;
}

static void clrbuf(U32* p, U32 s)
{
   U32* e = p + s;
   while (p != e)
      *p++ = U32_MAX;
}

bool TerrainBlock::buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF&)
{
	PROFILE_SCOPE( TerrainBlock_buildPolyList );

   // First check to see if the query misses the 
   // terrain elevation range.
   const Point3F &terrainPos = getPosition();
   if (  box.maxExtents.z - terrainPos.z < -TerrainThickness || 
         box.minExtents.z - terrainPos.z > fixedToFloat( mFile->getMaxHeight() ) )
      return false;

   // Transform the bounding sphere into the object's coord 
   // space.  Note that this is really optimal.
   Box3F osBox = box;
   mWorldToObj.mul(osBox);
   AssertWarn(mObjScale == Point3F::One, "Error, handle the scale transform on the terrain");

   // Setup collision state data
   polyList->setTransform(&getTransform(), getScale());
   polyList->setObject(this);

   S32 xStart = (S32)mFloor( osBox.minExtents.x / mSquareSize );
   S32 xEnd   = (S32)mCeil ( osBox.maxExtents.x / mSquareSize );
   S32 yStart = (S32)mFloor( osBox.minExtents.y / mSquareSize );
   S32 yEnd   = (S32)mCeil ( osBox.maxExtents.y / mSquareSize );
   if ( xStart < 0 )
      xStart = 0;
   S32 xExt = xEnd - xStart;
   if ( xExt > MaxExtent )
      xExt = MaxExtent;
   xEnd = xStart + xExt;

   U32 heightMax = floatToFixed(osBox.maxExtents.z);
   U32 heightMin = (osBox.minExtents.z < 0.0f)? 0.0f: floatToFixed(osBox.minExtents.z);

   // Index of shared points
   U32 bp[(MaxExtent + 1) * 2],*vb[2];
   vb[0] = &bp[0];
   vb[1] = &bp[xExt + 1];
   clrbuf(vb[1],xExt + 1);

   const U32 BlockMask = mFile->mSize - 1;

   bool emitted = false;
   for (S32 y = yStart; y < yEnd; y++) 
   {
      S32 yi = y & BlockMask;

      swap(vb[0],vb[1]);
      clrbuf(vb[1],xExt + 1);

      F32 wy1 = y * mSquareSize, wy2 = (y + 1) * mSquareSize;
      if(context == PLC_Navigation &&
         ((wy1 > osBox.maxExtents.y && wy2 > osBox.maxExtents.y) ||
          (wy1 < osBox.minExtents.y && wy2 < osBox.minExtents.y)))
         continue;

      //
      for (S32 x = xStart; x < xEnd; x++) 
      {
         S32 xi = x & BlockMask;
         const TerrainSquare *sq = mFile->findSquare( 0, xi, yi );

         F32 wx1 = x * mSquareSize, wx2 = (x + 1) * mSquareSize;
         if(context == PLC_Navigation &&
            ((wx1 > osBox.maxExtents.x && wx2 > osBox.maxExtents.x) ||
             (wx1 < osBox.minExtents.x && wx2 < osBox.minExtents.x)))
            continue;

         if ( x != xi || y != yi )
            continue;

         // holes only in the primary terrain block
         if (  ( ( sq->flags & TerrainSquare::Empty ) && x == xi && y == yi ) || 
               sq->minHeight > heightMax || 
               sq->maxHeight < heightMin )
            continue;

         emitted = true;

         // Add the missing points
         U32 vi[5];
         for (int i = 0; i < 4 ; i++) 
         {
            S32 dx = i >> 1;
            S32 dy = dx ^ (i & 1);
            U32* vp = &vb[dy][x - xStart + dx];
            if (*vp == U32_MAX) 
            {
               Point3F pos;
               pos.x = (F32)((x + dx) * mSquareSize);
               pos.y = (F32)((y + dy) * mSquareSize);
               pos.z = fixedToFloat( mFile->getHeight(xi + dx, yi + dy) );
               *vp = polyList->addPoint(pos);
            }
            vi[i] = *vp;
         }

         U32* vp = &vi[0];
         if ( !( sq->flags & TerrainSquare::Split45 ) )
            vi[4] = vi[0], vp++;

         BaseMatInstance *material = NULL; //getMaterialInst( xi, yi );
         U32 surfaceKey = ((xi << 16) + yi) << 1;
         polyList->begin(material,surfaceKey);
         polyList->vertex(vp[0]);
         polyList->vertex(vp[1]);
         polyList->vertex(vp[2]);
         polyList->plane(vp[0],vp[1],vp[2]);
         polyList->end();
         polyList->begin(material,surfaceKey + 1);
         polyList->vertex(vp[0]);
         polyList->vertex(vp[2]);
         polyList->vertex(vp[3]);
         polyList->plane(vp[0],vp[2],vp[3]);
         polyList->end();
      }
   }

   return emitted;
}

//----------------------------------------------------------------------------

static F32 calcInterceptV(F32 vStart, F32 invDeltaV, F32 intercept)
{
   return (intercept - vStart) * invDeltaV;
}

static F32 calcInterceptNone(F32, F32, F32)
{
   return MAX_FLOAT;
}

static F32 (*calcInterceptX)(F32, F32, F32);
static F32 (*calcInterceptY)(F32, F32, F32);

static U32 lineCount;
static Point3F lineStart, lineEnd;

bool TerrainBlock::castRay(const Point3F &start, const Point3F &end, RayInfo *info)
{
	PROFILE_SCOPE( TerrainBlock_castRay );

   if ( !castRayI(start, end, info, false) )
      return false;
      
   // Set intersection point.
   info->setContactPoint( start, end );
   getTransform().mulP( info->point );    // transform to world coordinates for getGridPos

   // Set material at contact point.
   Point2I gridPos = getGridPos( info->point );
   U8 layer = mFile->getLayerIndex( gridPos.x, gridPos.y );
   info->material = mFile->getMaterialMapping( layer );

   return true;
}

bool TerrainBlock::castRayI(const Point3F &start, const Point3F &end, RayInfo *info, bool collideEmpty)
{
   lineCount = 0;
   lineStart = start;
   lineEnd = end;

   info->object = this;

   if(start.x == end.x && start.y == end.y)
   {
      if (end.z == start.z)
         return false;

      F32 height;
      if(!getNormalAndHeight(Point2F(start.x, start.y), &info->normal, &height, true))
         return false;

      F32 t = (height - start.z) / (end.z - start.z);
      if(t < 0 || t > 1)
         return false;
      info->t = t;

      return true;
   }

   F32 invBlockWorldSize = 1 / getWorldBlockSize();

   Point3F pStart(start.x * invBlockWorldSize, start.y * invBlockWorldSize, start.z);
   Point3F pEnd(end.x * invBlockWorldSize, end.y * invBlockWorldSize, end.z);

   S32 blockX = (S32)mFloor(pStart.x);
   S32 blockY = (S32)mFloor(pStart.y);

   S32 dx, dy;

   F32 invDeltaX;
   if(pEnd.x == pStart.x)
   {
      calcInterceptX = calcInterceptNone;
      invDeltaX = 0;
      dx = 0;
   }
   else
   {
      invDeltaX = 1 / (pEnd.x - pStart.x);
      calcInterceptX = calcInterceptV;
      if(pEnd.x < pStart.x)
         dx = -1;
      else
         dx = 1;
   }

   F32 invDeltaY;
   if(pEnd.y == pStart.y)
   {
      calcInterceptY = calcInterceptNone;
      invDeltaY = 0;
      dy = 0;
   }
   else
   {
      invDeltaY = 1 / (pEnd.y - pStart.y);
      calcInterceptY = calcInterceptV;
      if(pEnd.y < pStart.y)
         dy = -1;
      else
         dy = 1;
   }

   const U32 BlockSquareWidth = mFile->mSize;
   const U32 GridLevels = mFile->mGridLevels;

   F32 startT = 0;
   for(;;)
   {
      F32 nextXInt = calcInterceptX(pStart.x, invDeltaX, (F32)(blockX + (dx == 1)));
      F32 nextYInt = calcInterceptY(pStart.y, invDeltaY, (F32)(blockY + (dy == 1)));

      F32 intersectT = 1;

      if(nextXInt < intersectT)
         intersectT = nextXInt;
      if(nextYInt < intersectT)
         intersectT = nextYInt;

      if ( castRayBlock(   pStart, 
                           pEnd, 
                           Point2I( blockX * BlockSquareWidth, 
                                    blockY * BlockSquareWidth ), 
                           GridLevels, 
                           invDeltaX, 
                           invDeltaY, 
                           startT, 
                           intersectT, 
                           info, 
                           collideEmpty ) ) 
      {
         info->normal.z *= BlockSquareWidth * mSquareSize;
         info->normal.normalize();
         return true;
      }

      startT = intersectT;
      if(intersectT >= 1)
         break;
      if(nextXInt < nextYInt)
         blockX += dx;
      else if(nextYInt < nextXInt)
         blockY += dy;
      else
      {
         blockX += dx;
         blockY += dy;
      }
   }

   return false;
}

struct TerrLOSStackNode
{
   F32 startT;
   F32 endT;
   Point2I blockPos;
   U32 level;
};

bool TerrainBlock::castRayBlock( const Point3F &pStart, 
                                 const Point3F &pEnd, 
                                 const Point2I &aBlockPos, 
                                 U32 aLevel, 
                                 F32 invDeltaX, 
                                 F32 invDeltaY, 
                                 F32 aStartT, 
                                 F32 aEndT, 
                                 RayInfo *info, 
                                 bool collideEmpty )
{
   const U32 BlockSquareWidth = mFile->mSize;
   const U32 GridLevels = mFile->mGridLevels;
   const U32 BlockMask = mFile->mSize - 1;

   F32 invBlockSize = 1 / F32( BlockSquareWidth );

   static Vector<TerrLOSStackNode> stack;
   stack.setSize( GridLevels * 3 + 1 );
   U32 stackSize = 1;

   stack[0].startT = aStartT;
   stack[0].endT = aEndT;
   stack[0].blockPos = aBlockPos;
   stack[0].level = aLevel;
   
   if( !aBlockPos.isZero() )
      return false;

   while(stackSize--)
   {
      TerrLOSStackNode *sn = stack.address() + stackSize;
      U32 level  = sn->level;
      F32 startT = sn->startT;
      F32 endT   = sn->endT;
      Point2I blockPos = sn->blockPos;

      const TerrainSquare *sq = mFile->findSquare( level, blockPos.x, blockPos.y );

      F32 startZ = startT * (pEnd.z - pStart.z) + pStart.z;
      F32 endZ = endT * (pEnd.z - pStart.z) + pStart.z;

      F32 minHeight = fixedToFloat(sq->minHeight);
      if(startZ <= minHeight && endZ <= minHeight)
         continue;

      F32 maxHeight = fixedToFloat(sq->maxHeight);
      if(startZ >= maxHeight && endZ >= maxHeight)
         continue;

      if (  !collideEmpty && ( sq->flags & TerrainSquare::Empty ) &&
      	  blockPos.x == ( blockPos.x & BlockMask ) && blockPos.y == ( blockPos.y & BlockMask ))
         continue;

      if(level == 0)
      {
         F32 xs = blockPos.x * invBlockSize;
         F32 ys = blockPos.y * invBlockSize;

         F32 zBottomLeft = fixedToFloat( mFile->getHeight(blockPos.x, blockPos.y) );
         F32 zBottomRight= fixedToFloat( mFile->getHeight(blockPos.x + 1, blockPos.y) );
         F32 zTopLeft =    fixedToFloat( mFile->getHeight(blockPos.x, blockPos.y + 1) );
         F32 zTopRight =   fixedToFloat( mFile->getHeight(blockPos.x + 1, blockPos.y + 1) );

         PlaneF p1, p2;
         PlaneF divider;
         Point3F planePoint;

         if(sq->flags & TerrainSquare::Split45)
         {
            p1.set(zBottomLeft - zBottomRight, zBottomRight - zTopRight, invBlockSize);
            p2.set(zTopLeft - zTopRight, zBottomLeft - zTopLeft, invBlockSize);
            planePoint.set(xs, ys, zBottomLeft);
            divider.x = 1;
            divider.y = -1;
            divider.z = 0;
         }
         else
         {
            p1.set(zTopLeft - zTopRight, zBottomRight - zTopRight, invBlockSize);
            p2.set(zBottomLeft - zBottomRight, zBottomLeft - zTopLeft, invBlockSize);
            planePoint.set(xs + invBlockSize, ys, zBottomRight);
            divider.x = 1;
            divider.y = 1;
            divider.z = 0;
         }
         p1.setPoint(planePoint);
         p2.setPoint(planePoint);
         divider.setPoint(planePoint);

         F32 t1 = p1.intersect(pStart, pEnd);
         F32 t2 = p2.intersect(pStart, pEnd);
         F32 td = divider.intersect(pStart, pEnd);

         F32 dStart = divider.distToPlane(pStart);
         F32 dEnd = divider.distToPlane(pEnd);

         // see if the line crosses the divider
         if((dStart >= 0 && dEnd < 0) || (dStart < 0 && dEnd >= 0))
         {
            if(dStart < 0)
            {
               F32 temp = t1;
               t1 = t2;
               t2 = temp;
            }
            if(t1 >= startT && t1 && t1 <= td && t1 <= endT)
            {
               info->t = t1;
               info->normal = p1;
               return true;
            }
            if(t2 >= td && t2 >= startT && t2 <= endT)
            {
               info->t = t2;
               info->normal = p2;
               return true;
            }
         }
         else
         {
            F32 t;
            if(dStart >= 0) {
               t = t1;
               info->normal = p1;
            }
            else {
               t = t2;
               info->normal = p2;
            }
            if(t >= startT && t <= endT)
            {
               info->t = t;
               return true;
            }
         }
         continue;
      }
      S32 subSqWidth = 1 << (level - 1);
      F32 xIntercept = (blockPos.x + subSqWidth) * invBlockSize;
      F32 xInt = calcInterceptX(pStart.x, invDeltaX, xIntercept);
      F32 yIntercept = (blockPos.y + subSqWidth) * invBlockSize;
      F32 yInt = calcInterceptY(pStart.y, invDeltaY, yIntercept);

      F32 startX = startT * (pEnd.x - pStart.x) + pStart.x;
      F32 startY = startT * (pEnd.y - pStart.y) + pStart.y;

      if(xInt < startT)
         xInt = MAX_FLOAT;
      if(yInt < startT)
         yInt = MAX_FLOAT;

      U32 x0 = (startX > xIntercept) * subSqWidth;
      U32 y0 = (startY > yIntercept) * subSqWidth;
      U32 x1 = subSqWidth - x0;
      U32 y1 = subSqWidth - y0;
      U32 nextLevel = level - 1;

      // push the items on the stack in reverse order of processing
      if(xInt > endT && yInt > endT)
      {
         // only test the square the point started in:
         stack[stackSize].blockPos.set(blockPos.x + x0, blockPos.y + y0);
         stack[stackSize].level = nextLevel;
         stackSize++;
      }
      else if(xInt < yInt)
      {
         F32 nextIntersect = endT;
         if(yInt <= endT)
         {
            stack[stackSize].blockPos.set(blockPos.x + x1, blockPos.y + y1);
            stack[stackSize].startT = yInt;
            stack[stackSize].endT = endT;
            stack[stackSize].level = nextLevel;
            nextIntersect = yInt;
            stackSize++;
         }
         stack[stackSize].blockPos.set(blockPos.x + x1, blockPos.y + y0);
         stack[stackSize].startT = xInt;
         stack[stackSize].endT = nextIntersect;
         stack[stackSize].level = nextLevel;

         stack[stackSize+1].blockPos.set(blockPos.x + x0, blockPos.y + y0);
         stack[stackSize+1].startT = startT;
         stack[stackSize+1].endT = xInt;
         stack[stackSize+1].level = nextLevel;
         stackSize += 2;
      }
      else if(yInt < xInt)
      {
         F32 nextIntersect = endT;
         if(xInt <= endT)
         {
            stack[stackSize].blockPos.set(blockPos.x + x1, blockPos.y + y1);
            stack[stackSize].startT = xInt;
            stack[stackSize].endT = endT;
            stack[stackSize].level = nextLevel;
            nextIntersect = xInt;
            stackSize++;
         }
         stack[stackSize].blockPos.set(blockPos.x + x0, blockPos.y + y1);
         stack[stackSize].startT = yInt;
         stack[stackSize].endT = nextIntersect;
         stack[stackSize].level = nextLevel;

         stack[stackSize+1].blockPos.set(blockPos.x + x0, blockPos.y + y0);
         stack[stackSize+1].startT = startT;
         stack[stackSize+1].endT = yInt;
         stack[stackSize+1].level = nextLevel;
         stackSize += 2;
      }
      else
      {
         stack[stackSize].blockPos.set(blockPos.x + x1, blockPos.y + y1);
         stack[stackSize].startT = xInt;
         stack[stackSize].endT = endT;
         stack[stackSize].level = nextLevel;

         stack[stackSize+1].blockPos.set(blockPos.x + x0, blockPos.y + y0);
         stack[stackSize+1].startT = startT;
         stack[stackSize+1].endT = xInt;
         stack[stackSize+1].level = nextLevel;
         stackSize += 2;
      }
   }

   return false;
}
