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
#include "math/mMath.h"
#include "console/console.h"
#include "collision/extrudedPolyList.h"
#include "math/mPolyhedron.h"
#include "collision/collision.h"

// Minimum distance from a face
F32 ExtrudedPolyList::FaceEpsilon = 0.01f;

// Value used to compare collision times
F32 ExtrudedPolyList::EqualEpsilon = 0.0001f;

ExtrudedPolyList::ExtrudedPolyList()
{
   VECTOR_SET_ASSOCIATION(mVertexList);
   VECTOR_SET_ASSOCIATION(mIndexList);
   VECTOR_SET_ASSOCIATION(mExtrudedList);
   VECTOR_SET_ASSOCIATION(mPlaneList);
   VECTOR_SET_ASSOCIATION(mPolyPlaneList);

   mVelocity.set(0.0f,0.0f,0.0f);
   mIndexList.reserve(128);
   mVertexList.reserve(64);
   mPolyPlaneList.reserve(64);
   mPlaneList.reserve(64);
   mCollisionList = 0;
}

ExtrudedPolyList::~ExtrudedPolyList()
{
}

//----------------------------------------------------------------------------

bool ExtrudedPolyList::isEmpty() const
{
   return mCollisionList->getCount() == 0;
}


//----------------------------------------------------------------------------

void ExtrudedPolyList::extrude(const Polyhedron& pt, const VectorF& vector)
{
   // Clear state
   mIndexList.clear();
   mVertexList.clear();
   mPlaneList.clear();
   mPolyPlaneList.clear();

   // Determine which faces will be extruded.
   mExtrudedList.setSize(pt.planeList.size());

   for (U32 f = 0; f < pt.planeList.size(); f++) 
   {
      const PlaneF& face = pt.planeList[f];
      ExtrudedFace& eface = mExtrudedList[f];
      F32 dot = mDot(face,vector);
      eface.active = dot > EqualEpsilon;
      
      if (eface.active) 
      {
         eface.maxDistance = dot;
         eface.plane = face;
         eface.planeMask = BIT(mPlaneList.size());

         // Add the face as a plane to clip against.
         mPlaneList.increment(2);
         PlaneF* plane = mPlaneList.end() - 2;
         plane[0] = plane[1] = face;
         plane[0].invert();
      }
   }

   // Produce extruded planes for bounding and internal edges
   for (U32 e = 0; e < pt.edgeList.size(); e++) 
   {
      Polyhedron::Edge const& edge = pt.edgeList[e];
      ExtrudedFace& ef1 = mExtrudedList[edge.face[0]];
      ExtrudedFace& ef2 = mExtrudedList[edge.face[1]];
      if (ef1.active || ef2.active) 
      {

         // Assumes that the edge points are clockwise
         // for face[0].
         const Point3F& p1 = pt.pointList[edge.vertex[1]];
         const Point3F &p2 = pt.pointList[edge.vertex[0]];
         Point3F p3 = p2 + vector;

         mPlaneList.increment(2);
         PlaneF* plane = mPlaneList.end() - 2;
         plane[0].set(p3,p2,p1);
         plane[1] = plane[0];
         plane[1].invert();

         U32 pmask = BIT(mPlaneList.size()-2);
         ef1.planeMask |= pmask;
         ef2.planeMask |= pmask << 1;
      }
   }
}


//----------------------------------------------------------------------------

void ExtrudedPolyList::setCollisionList(CollisionList* info)
{
   mCollisionList = info;
   mCollisionList->clear();
   mCollisionList->setTime( 2.0f );
}

//----------------------------------------------------------------------------

void ExtrudedPolyList::adjustCollisionTime()
{
   if( !mCollisionList->getCount() )
      return;

   mCollisionList->setTime( mClampF( mCollisionList->getTime(), 0.f, 1.f ) );
}


//----------------------------------------------------------------------------

U32 ExtrudedPolyList::addPoint(const Point3F& p)
{
   mVertexList.increment();
   Vertex& v = mVertexList.last();

   v.point.x = p.x * mScale.x;
   v.point.y = p.y * mScale.y;
   v.point.z = p.z * mScale.z;
   mMatrix.mulP(v.point);

   // Build the plane mask, planes come in pairs
   v.mask = 0;
   for (U32 i = 0; i < mPlaneList.size(); i ++)
      if (mPlaneList[i].distToPlane(v.point) >= 0.f)
         v.mask |= BIT(i);

   return mVertexList.size() - 1;
}


U32 ExtrudedPolyList::addPlane(const PlaneF& plane)
{
   mPolyPlaneList.increment();
   mPlaneTransformer.transform(plane, mPolyPlaneList.last());

   return mPolyPlaneList.size() - 1;
}


//----------------------------------------------------------------------------

void ExtrudedPolyList::begin(BaseMatInstance* material, U32 /*surfaceKey*/)
{
   mPoly.object = mCurrObject;
   mPoly.material = material;
   mIndexList.clear();
}

void ExtrudedPolyList::plane(U32 v1, U32 v2, U32 v3)
{
   mPoly.plane.set(mVertexList[v1].point,
                   mVertexList[v2].point,
                   mVertexList[v3].point);
                   
   // We hope this isn't needed but we're leaving it in anyway -- BJG/EGH
   mPoly.plane.normalizeSafe();
}

void ExtrudedPolyList::plane(const PlaneF& p)
{
   mPlaneTransformer.transform(p, mPoly.plane);
}

void ExtrudedPolyList::plane(const U32 index)
{
   AssertFatal(index < mPolyPlaneList.size(), "Out of bounds index!");
   mPoly.plane = mPolyPlaneList[index];
}

const PlaneF& ExtrudedPolyList::getIndexedPlane(const U32 index)
{
   AssertFatal(index < mPolyPlaneList.size(), "Out of bounds index!");
   return mPolyPlaneList[index];
}


void ExtrudedPolyList::vertex(U32 vi)
{
   mIndexList.push_back(vi);
}

void ExtrudedPolyList::end()
{
   // Anything facing away from the mVelocity is rejected  (and also
   // cap to max collisions)
   if (mDot(mPoly.plane, mNormalVelocity) > 0.f ||
      mCollisionList->getCount() >= CollisionList::MaxCollisions)
      return;

   // Test the built up poly (stored in mPoly) against all our extruded
   // faces.
   U32           cFaceCount = 0;
   ExtrudedFace* cFace[30];
   bool          cEdgeColl[30];
   ExtrudedFace* face = mExtrudedList.begin();
   ExtrudedFace* end = mExtrudedList.end();

   for (; face != end; face++)
   {
      // Skip inactive..
      if (!face->active)
         continue;

      // Update the dot product.
      face->faceDot = -mDot(face->plane,mPoly.plane);
      
      // Skip it if we're facing towards...
      if(face->faceDot <= 0.f)
         continue;

      // Test, and skip if colliding.
      if (!testPoly(*face))
         continue;
 
      // Note collision.
      cFace[cFaceCount]       = face;
      cEdgeColl[cFaceCount++] = false;
   }

   if (!cFaceCount)
   {
      face = mExtrudedList.begin();
      end  = mExtrudedList.end();
      for (; face != end; face++)
      {
         // Don't need to do dot product second time, so just check if it's
         // active (we already did the dot product in the previous loop).
         if (!face->active)
            continue;

         // Skip it if we're facing away...
         if(face->faceDot > 0.f)
            continue;

         // Do collision as above.
         if (!testPoly(*face))
            continue;

         // Note the collision.
         cFace[cFaceCount]       = face;
         cEdgeColl[cFaceCount++] = true;
      }
   }

   // If we STILL don't have any collisions, just skip out.
   if (!cFaceCount)
      return;

   // Pick the best collision face based on best alignment with respective
   // face.
   face = cFace[0];
   bool edge = cEdgeColl[0];
   for (U32 f = 1; f < cFaceCount; f++)
   {
      if (cFace[f]->faceDot <= face->faceDot)
         continue;

      face = cFace[f];
      edge = cEdgeColl[f];
   }

   // Add it to the collision list if it's better and/or equal
   // to our current best.

   // Don't add it to the collision list if it's too far away.
   if (face->time > mCollisionList->getTime() + EqualEpsilon || face->time >= 1.0)
      return;

   if (face->time < mCollisionList->getTime() - EqualEpsilon) 
   {
      // If this is significantly closer than before, then clear out the
      // list, as it's a better match than the old stuff.
      mCollisionList->clear();
      mCollisionList->setTime( face->time );
      mCollisionList->setMaxHeight( face->height );
   }
   else 
   {
      // Otherwise, just update some book-keeping stuff.
      if ( face->height > mCollisionList->getMaxHeight() )
         mCollisionList->setMaxHeight( face->height );
   }

   // Note the collision in our collision list.
   Collision& collision = mCollisionList->increment();
   collision.point    = face->point;
   collision.faceDot  = face->faceDot;
   collision.face     = face - mExtrudedList.begin();
   collision.object   = mPoly.object;
   collision.normal   = mPoly.plane;
   collision.material = mPoly.material;
}


//----------------------------------------------------------------------------

bool ExtrudedPolyList::testPoly(ExtrudedFace& face)
{
   // Build intial inside/outside plane masks
   U32 indexStart = 0;
   U32 indexEnd = mIndexList.size();
   U32 oVertexSize = mVertexList.size();
   U32 oIndexSize = mIndexList.size();

   U32 frontMask = 0,backMask = 0;
   for (U32 i = indexStart; i < indexEnd; i++) 
   {
      U32 mask = mVertexList[mIndexList[i]].mask & face.planeMask;
      frontMask |= mask;
      backMask |= ~mask;
   }

   // Clip the mPoly against the planes that bound the face...
   // Trivial accept if all the vertices are on the backsides of
   // all the planes.
   if (frontMask) 
   {
      // Trivial reject if any plane not crossed has all it's points
      // on the front.
      U32 crossMask = frontMask & backMask;
      if (~crossMask & frontMask)
         return false;

      // Need to do some clipping
      for (U32 p=0; p < mPlaneList.size(); p++) 
      {
         U32 pmask    = BIT(p);
         U32 newStart = mIndexList.size();

         // Only test against this plane if we have something
         // on both sides - otherwise skip.
         if (!(face.planeMask & crossMask & pmask))
            continue;

         U32 i1 = indexEnd - 1;
         U32 mask1 = mVertexList[mIndexList[i1]].mask;

         for (U32 i2 = indexStart; i2 < indexEnd; i2++) 
         {
            const U32 mask2 = mVertexList[mIndexList[i2]].mask;
            if ((mask1 ^ mask2) & pmask)
            {
               // Clip the edge against the plane.
               mVertexList.increment();
               VectorF& v1 = mVertexList[mIndexList[i1]].point;
               VectorF& v2 = mVertexList[mIndexList[i2]].point;
               VectorF vv = v2 - v1;
               F32 t = -mPlaneList[p].distToPlane(v1) / mDot(mPlaneList[p],vv);

               mIndexList.push_back(mVertexList.size() - 1);
               Vertex& iv = mVertexList.last();
               iv.point.x = v1.x + vv.x * t;
               iv.point.y = v1.y + vv.y * t;
               iv.point.z = v1.z + vv.z * t;
               iv.mask = 0;

               // Test against the remaining planes
               for (U32 i = p+1; i < mPlaneList.size(); i ++)
               {
                  if (mPlaneList[i].distToPlane(iv.point) > 0.f)
                     iv.mask |= BIT(i);
               }
            }

            if (!(mask2 & pmask)) 
            {
               U32 index = mIndexList[i2];
               mIndexList.push_back(index);
            }

            mask1 = mask2;
            i1 = i2;
         }

         // Check for degenerate
         indexStart = newStart;
         indexEnd = mIndexList.size();
         if (mIndexList.size() - indexStart < 3) 
         {
            mVertexList.setSize(oVertexSize);
            mIndexList.setSize(oIndexSize);
            return false;
         }
      }
   }

   // Find closest point on the mPoly
   Point3F bp(0.0f, 0.0f, 0.0f);
   F32 bd = 1E30f;
   F32 height = -1E30f;
   for (U32 b = indexStart; b < indexEnd; b++) 
   {
      Vertex& vertex = mVertexList[mIndexList[b]];
      F32 dist = face.plane.distToPlane(vertex.point);
      if (dist <= bd) 
      {
         bd = (dist < 0)? 0: dist;
         bp = vertex.point;
      }
      
      // Since we don't clip against the back plane, we'll
      // only include vertex heights that are within range.
      if (vertex.point.z > height && dist < face.maxDistance)
         height = vertex.point.z;
   }
   
   // hack for not jetting up through the cieling
   F32 fudge = 0.01f;
   F32 fudgeB = 0.2f;
   if(mNormalVelocity.z > 0.0)
   {
      fudge = 0.01f; //0.015;
      fudgeB = 0.2f;
   }
   
   // Do extruded points for back-off.
   F32 oldBd=bd;
   for (U32 b = indexStart; b < indexEnd; b++) 
   {
      Vertex& vertex = mVertexList[mIndexList[b]];
      
      // Extrude out just a tad to make sure we don't end up getting too close to the
      // geometry and getting stuck - but cap it so we don't introduce error into long
      // sweeps.
      F32 dist = face.plane.distToPlane( vertex.point 
                          + Point3F(mPoly.plane) * getMin(face.maxDistance * fudgeB, fudge));

      if (dist <= bd)
      {
         bd = (dist < 0)? 0: dist;
         bp = vertex.point;
      }
   }

   // Remove temporary data
   mVertexList.setSize(oVertexSize);
   mIndexList.setSize(oIndexSize);

   // Don't add it to the collision list if it's worse then our current best.
   if (oldBd >= face.maxDistance)
      return false;

   // Update our info and indicate we should add to the model.
   F32 oldT = oldBd / face.maxDistance;
   F32 pushBackT = bd / face.maxDistance;
   
   if(oldT - pushBackT > 0.1)
      face.time = oldT - fudge;
   else
      face.time = pushBackT;
      
   face.height = height;
   face.point  = bp;
   return true;
}

//--------------------------------------------------------------------------
void ExtrudedPolyList::setVelocity(const VectorF& velocity)
{
   mVelocity = velocity;
   if (velocity.isZero() == false)
   {
      mNormalVelocity = velocity;
      mNormalVelocity.normalize();
      setInterestNormal(mNormalVelocity);
   }
   else
   {
      mNormalVelocity.set(0.0f, 0.0f, 0.0f);
      clearInterestNormal();
   }
}
