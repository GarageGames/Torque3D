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

#include "interior/interior.h"
#include "math/mSphere.h"
#include "scene/sceneObject.h"
#include "collision/abstractPolyList.h"

#include "core/frameAllocator.h"
#include "platform/profiler.h"

namespace {

//--------------------------------------
// Custom on plane version to reduce numerical inaccuracies in the GJK stuff.
//  copied from terrCollision.cc
inline bool isOnPlane(const Point3F& p, const PlaneF& plane)
{
   F32 dist = mDot(plane,p) + plane.d;
   return mFabs(dist) < 0.1f;
}

} // namespace {}


//--------------------------------------------------------------------------
//-------------------------------------- SurfaceHash
const U32 csgHashSize = 4096;
class SurfaceHash
{
  private:
   U32 hash(U32 i) { return i & (csgHashSize - 1); }

  public:
   U32   mSurfaceArray[csgHashSize];
   U32   mNumSurfaces;

   U32   mHashTable[csgHashSize];

  public:
   SurfaceHash() 
   {
      AssertFatal(isPow2(csgHashSize), "Error, size must be power of 2");
   }

   void clear() 
   {
      dMemset(mHashTable, 0xFF, sizeof(mHashTable));
      mNumSurfaces = 0;
   }

   void insert(U32 surface);
};

inline void SurfaceHash::insert(U32 surface)
{
   AssertFatal(surface != 0xFFFFFFFF, "Hm, bad assumption.  0xFFFFFFFF is a valid surface index?");
   if (mNumSurfaces >= csgHashSize) 
   {
      AssertFatal(false, "Error, exceeded surfaceCount restriction!");
      return;
   }

   U32 probe = hash(surface);
   U32 initialProbe = probe;
   while (mHashTable[probe] != 0xFFFFFFFF) 
   {
      // If it's already in the table, bail.
      if (mHashTable[probe] == surface)
         return;

      probe = (probe + 1) % csgHashSize;
      AssertFatal(probe != initialProbe, "Hm, wraparound?");
   }

   // If we're here, then the probe failed, and the index isn't in the hash
   //  table
   mHashTable[probe] = surface;
   mSurfaceArray[mNumSurfaces++] = surface;
}


class InteriorPolytope
{
   // Convex Polyhedron
  public:
   struct Vertex 
   {
      Point3F point;
      // Temp BSP clip info
      S32 side;
   };
   struct Edge 
   {
      S32 vertex[2];
      S32 face[2];
      S32 next;
   };
   struct Face
   {
      S32 original;
      // Temp BSP clip info
      S32 vertex;
   };
   struct Volume
   {
      S32 edgeList;
   };
   struct StackElement
   {
      S32 edgeList;
      U32 popCount;
      U32 nodeIndex;
   };

   typedef Vector<Edge> EdgeList;
   typedef Vector<Face> FaceList;
   typedef Vector<Vertex> VertexList;
   typedef Vector<Volume> VolumeList;
   typedef Vector<StackElement> VolumeStack;

   //
   S32        sideCount;
   EdgeList   mEdgeList;
   FaceList   mFaceList;
   VertexList mVertexList;
   VolumeList mVolumeList;

  public:
   //
   InteriorPolytope();
   void clear();
   bool intersect(const PlaneF& plane,const Point3F& sp,const Point3F& ep);
   void buildBox(const Box3F& box, const MatrixF& transform, const Point3F& scale);
   inline bool didIntersect()  { return mVolumeList.size() > 1; }
};

//----------------------------------------------------------------------------
// Box should be axis aligned in the transform space provided.

InteriorPolytope::InteriorPolytope()
{
   mVertexList.reserve(256);
   mFaceList.reserve(200);
   mEdgeList.reserve(100);
   mVolumeList.reserve(20);
   sideCount = 0;
}

void InteriorPolytope::clear()
{
   mVertexList.clear();
   mFaceList.clear();
   mEdgeList.clear();
   mVolumeList.clear();
   sideCount = 0;
}

void InteriorPolytope::buildBox(const Box3F& box, const MatrixF& transform, const Point3F&)
{
   // Initial vertices
   mVertexList.setSize(8);
   mVertexList[0].point = Point3F(box.minExtents.x, box.minExtents.y, box.minExtents.z);
   mVertexList[1].point = Point3F(box.minExtents.x, box.maxExtents.y, box.minExtents.z);
   mVertexList[2].point = Point3F(box.maxExtents.x, box.maxExtents.y, box.minExtents.z);
   mVertexList[3].point = Point3F(box.maxExtents.x, box.minExtents.y, box.minExtents.z);
   mVertexList[4].point = Point3F(box.minExtents.x, box.minExtents.y, box.maxExtents.z);
   mVertexList[5].point = Point3F(box.minExtents.x, box.maxExtents.y, box.maxExtents.z);
   mVertexList[6].point = Point3F(box.maxExtents.x, box.maxExtents.y, box.maxExtents.z);
   mVertexList[7].point = Point3F(box.maxExtents.x, box.minExtents.y, box.maxExtents.z);
   S32 i;
   for (i = 0; i < 8; i++) 
   {
      transform.mulP(mVertexList[i].point);
      mVertexList[i].side = 0;
   }

   // Initial faces
   mFaceList.setSize(6);
   for (S32 f = 0; f < 6; f++) 
   {
      Face& face = mFaceList[f];
      face.original = true;
      face.vertex = 0;
   }

   // Initial edges
   mEdgeList.setSize(12);
   Edge* edge = mEdgeList.begin();
   S32 nextEdge = 0;
   for (i = 0; i < 4; i++) 
   {
      S32 n = (i == 3)? 0: i + 1;
      S32 p = (i == 0)? 3: i - 1;
      edge->vertex[0] = i;
      edge->vertex[1] = n;
      edge->face[0] = i;
      edge->face[1] = 4;
      edge->next = ++nextEdge;
      edge++;
      edge->vertex[0] = 4 + i;
      edge->vertex[1] = 4 + n;
      edge->face[0] = i;
      edge->face[1] = 5;
      edge->next = ++nextEdge;
      edge++;
      edge->vertex[0] = i;
      edge->vertex[1] = 4 + i;
      edge->face[0] = i;
      edge->face[1] = p;
      edge->next = ++nextEdge;
      edge++;
   }
   edge[-1].next = -1;

   // Volume
   mVolumeList.setSize(1);
   Volume& volume = mVolumeList.last();
   volume.edgeList = 0;
   sideCount = 0;
}

bool InteriorPolytope::intersect(const PlaneF& plane,const Point3F& sp,const Point3F& ep)
{
   // If den == 0 then the line and plane are parallel.
   F32 den;
   Point3F dt = ep - sp;
   if ((den = plane.x * dt.x + plane.y * dt.y + plane.z * dt.z) == 0.0f)
      return false;

   // Save the values in sp since the memory may go away after the increment
   F32 sp_x = sp.x;
   F32 sp_y = sp.y;
   F32 sp_z = sp.z;

   mVertexList.increment();
   Vertex& v = mVertexList.last();
   F32 s = -(plane.x * sp_x + plane.y * sp_y + plane.z * sp_z + plane.d) / den;
   v.point.x = sp_x + dt.x * s;
   v.point.y = sp_y + dt.y * s;
   v.point.z = sp_z + dt.z * s;
   v.side = 0;
   return true;
}


//--------------------------------------------------------------------------
void Interior::collisionFanFromSurface(const Surface& rSurface, U32* fanIndices, U32* numIndices) const
{
   U32 tempIndices[32];

   tempIndices[0] = 0;
   U32 idx = 1;
   U32 i;
   
   for (i = 1; i < rSurface.windingCount; i += 2)
      tempIndices[idx++] = i;

   for (i = ((rSurface.windingCount - 1) & (~0x1)); i > 0; i -= 2)
      tempIndices[idx++] = i;

   idx = 0;
   for (i = 0; i < rSurface.windingCount; i++) 
   {
      if (rSurface.fanMask & (1 << i)) 
      {
         fanIndices[idx++] = mWindings[rSurface.windingStart + tempIndices[i]];
      }
   }
   *numIndices = idx;
}

void Interior::fullWindingFromSurface(const Surface& rSurface, U32* fanIndices, U32* numIndices) const
{
   U32 tempIndices[32];

   tempIndices[0] = 0;
   U32 idx = 1;
   U32 i;

   for (i = 1; i < rSurface.windingCount; i += 2)
      tempIndices[idx++] = i;

   for (i = ((rSurface.windingCount - 1) & (~0x1)); i > 0; i -= 2)
      tempIndices[idx++] = i;

   idx = 0;
   for (i = 0; i < rSurface.windingCount; i++)
      fanIndices[idx++] = mWindings[rSurface.windingStart + tempIndices[i]];

   *numIndices = idx;
}


bool Interior::castRay_r(const U32      node,
                         const U16      planeIndex,
                         const Point3F& s,
                         const Point3F& e,
                         RayInfo*       info)
{
   if (isBSPLeafIndex(node) == false) 
   {
      const IBSPNode& rNode = mBSPNodes[node];
      const PlaneF& rPlane  = getPlane(rNode.planeIndex);

      const PlaneF::Side sSide = rPlane.whichSide(s);
      const PlaneF::Side eSide = rPlane.whichSide(e);

      switch (PlaneSwitchCode(sSide, eSide)) 
      {
         case PlaneSwitchCode(PlaneF::Front, PlaneF::Front):
         case PlaneSwitchCode(PlaneF::Front, PlaneF::On):
         case PlaneSwitchCode(PlaneF::On,    PlaneF::Front):
            return castRay_r(rNode.frontIndex, planeIndex, s, e, info);
            break;

         case PlaneSwitchCode(PlaneF::On,   PlaneF::Back):
         case PlaneSwitchCode(PlaneF::Back, PlaneF::On):
         case PlaneSwitchCode(PlaneF::Back, PlaneF::Back):
            return castRay_r(rNode.backIndex, planeIndex, s, e, info);
            break;

         case PlaneSwitchCode(PlaneF::On, PlaneF::On):
            // Line lies on the plane
            if (isBSPLeafIndex(rNode.backIndex) == false) 
            {
               if (castRay_r(rNode.backIndex, planeIndex, s, e, info))
                  return true;
            }

            if (isBSPLeafIndex(rNode.frontIndex) == false) 
            {
               if (castRay_r(rNode.frontIndex, planeIndex, s, e, info))
                  return true;
            }

            return false;
            break;

         case PlaneSwitchCode(PlaneF::Front, PlaneF::Back): 
         {
            Point3F ip;
            F32     intersectT = rPlane.intersect(s, e);
            AssertFatal(intersectT != PARALLEL_PLANE, "Error, this should never happen in this case!");
            ip.interpolate(s, e, intersectT);
            if (castRay_r(rNode.frontIndex, planeIndex, s, ip, info))
               return true;
            return castRay_r(rNode.backIndex, rNode.planeIndex, ip, e, info);
         }
            break;

         case PlaneSwitchCode(PlaneF::Back, PlaneF::Front): 
         {
            Point3F ip;
            F32     intersectT = rPlane.intersect(s, e);
            AssertFatal(intersectT != PARALLEL_PLANE, "Error, this should never happen in this case!");
            ip.interpolate(s, e, intersectT);
            if (castRay_r(rNode.backIndex, planeIndex, s, ip, info))
               return true;
            return castRay_r(rNode.frontIndex, rNode.planeIndex, ip, e, info);
         }
            break;

         default:
            AssertFatal(false, "Misunderstood switchCode in Interior::castRay_r");
            return false;
      }
   }

   if (isBSPSolidLeaf(node)) 
   {
      // DMM: Set material info here.. material info? hahaha

      info->point  = s;

      if (planeIndex != U16(-1)) 
      {
         const PlaneF& rPlane = getPlane(planeIndex);
         info->normal = rPlane;
         if (planeIsFlipped(planeIndex)) 
         {
            info->normal.neg();
            if (rPlane.whichSide(e) == PlaneF::Back)
               info->normal.neg();
         }
         else 
         {
            if (rPlane.whichSide(e) == PlaneF::Front)
               info->normal.neg();
         }
      }
      else 
      {
         // Point started in solid;
         if (s == e) 
         {
            info->normal.set(0.0f, 0.0f, 1.0f);
         } 
         else 
         {
            info->normal = s - e;
            info->normal.normalize();
         }
      }

      // ok.. let's get it on! get the face that was hit
      info->face = U32(-1);

      U32 numStaticSurfaces = 0;

      const IBSPLeafSolid & rLeaf = mBSPSolidLeaves[getBSPSolidLeafIndex(node)];
      for(U32 i = 0; i < rLeaf.surfaceCount; i++)
      {
         U32 surfaceIndex = mSolidLeafSurfaces[rLeaf.surfaceIndex + i];
         if(isNullSurfaceIndex(surfaceIndex))
         {
            const NullSurface & rSurface = mNullSurfaces[getNullSurfaceIndex(surfaceIndex)];
            if (rSurface.surfaceFlags & SurfaceStaticMesh)
               numStaticSurfaces++;

            continue;
         }

         const Surface & rSurface = mSurfaces[surfaceIndex];
         if(!areEqualPlanes(rSurface.planeIndex, planeIndex))
            continue;

         PlaneF plane = getPlane(rSurface.planeIndex);
         if(planeIsFlipped(rSurface.planeIndex))
            plane.neg();

         // unfan this surface
         U32 winding[32];
         U32 windingCount;
         collisionFanFromSurface(rSurface, winding, &windingCount);

         // inside this surface?
         bool inside = true;
         for(U32 j = 0; inside && (j < windingCount); j++)
         {
            U32 k = (j+1) % windingCount;

            Point3F vec1 = mPoints[winding[k]].point - mPoints[winding[j]].point;
            Point3F vec2 = info->point - mPoints[winding[j]].point;

            Point3F cross;
            mCross(vec2, vec1, &cross);

            if(mDot(plane, cross) < 0.f)
               inside = false;
         }

         if(inside)
         {
            info->face = surfaceIndex;
            info->material = mMaterialList->getMaterialInst( rSurface.textureIndex );
            break;
         }
      }

      if (Interior::smLightingCastRays && numStaticSurfaces == rLeaf.surfaceCount && numStaticSurfaces > 0)
         return false;

      return true;
   }
   return false;
}

bool Interior::castRay(const Point3F& s, const Point3F& e, RayInfo* info)
{
   // DMM: Going to need normal here eventually.
   bool hit = castRay_r(0, U16(-1), s, e, info);
   if (hit) 
   {
      Point3F vec = e - s;
      F32 len = vec.len();
      if (len < 1e-6f) 
      {
         info->t = 0.0f;
      }
      else 
      {
         vec /= len;
         info->t = mDot(info->point - s, vec) / len;
      }
   }

   AssertFatal(!hit || (info->normal.z == info->normal.z), "NaN returned from castRay.\n\nPlease talk to DMM if you are running this in the debugger");

   return hit;
}

void Interior::buildPolyList_r(InteriorPolytope& polytope,
                               SurfaceHash& hash)
{
   // Submit the first volume of the poly tope to the bsp tree
   static InteriorPolytope::VolumeStack stack;
   stack.reserve(256);
   stack.setSize(1);
   stack.last().edgeList  = polytope.mVolumeList[0].edgeList;
   stack.last().nodeIndex = 0;
   stack.last().popCount  = 0;

   static Vector<U16> collPlanes;
   collPlanes.reserve(64);
   collPlanes.clear();

   while (stack.empty() == false) 
   {
      InteriorPolytope::StackElement volume = stack.last();
      stack.pop_back();

      if (isBSPLeafIndex(volume.nodeIndex)) 
      {
         if (isBSPSolidLeaf(volume.nodeIndex)) 
         {
            // Export the polys from this node.
            const IBSPLeafSolid& rLeaf = mBSPSolidLeaves[getBSPSolidLeafIndex(volume.nodeIndex)];
            for (U32 i = 0; i < rLeaf.surfaceCount; i++) 
            {
               U32 surfaceIndex = mSolidLeafSurfaces[rLeaf.surfaceIndex + i];
               if (isNullSurfaceIndex(surfaceIndex)) 
               {
                  // Is a NULL surface
                  const NullSurface& rSurface = mNullSurfaces[getNullSurfaceIndex(surfaceIndex)];
                  for (U32 j = 0; j < collPlanes.size(); j++) 
                  {
                     if (areEqualPlanes(rSurface.planeIndex, collPlanes[j]) == true) 
                     {
                        hash.insert(surfaceIndex);
                        break;
                     }
                  }
               }
               else 
               {
                  const Surface& rSurface = mSurfaces[surfaceIndex];
                  for (U32 j = 0; j < collPlanes.size(); j++) 
                  {
                     if (areEqualPlanes(rSurface.planeIndex, collPlanes[j]) == true) 
                     {
                        hash.insert(surfaceIndex);
                        break;
                     }
                  }
               }
            }
         }

         if (volume.popCount)
            for (U32 i = 0; i < volume.popCount; i++)
               collPlanes.pop_back();

         continue;
      }

      const IBSPNode& rNode = mBSPNodes[volume.nodeIndex];

      // New front and back faces
      polytope.mFaceList.increment(2);
      InteriorPolytope::Face& frontFace = polytope.mFaceList[polytope.mFaceList.size() - 1];
      InteriorPolytope::Face& backFace  = polytope.mFaceList[polytope.mFaceList.size() - 2];

      backFace.original = frontFace.original = false;
      backFace.vertex   = frontFace.vertex = 0;

      // New front and back volumes
      InteriorPolytope::StackElement frontVolume,backVolume;
      frontVolume.edgeList = backVolume.edgeList = -1;

      PlaneF plane = getPlane(rNode.planeIndex);
      if (planeIsFlipped(rNode.planeIndex))
         plane.neg();

      S32 startVertex = polytope.mVertexList.size();

      // Test & clip all the edges
      S32 sideBase = ++polytope.sideCount << 1;
      for (S32 i = volume.edgeList; i >= 0; i = polytope.mEdgeList[i].next) 
      {

         // Copy into tmp first to avoid problems with the array.
         InteriorPolytope::Edge edge = polytope.mEdgeList[i];

         InteriorPolytope::Vertex& v0 = polytope.mVertexList[edge.vertex[0]];
         if (v0.side < sideBase)
            v0.side = sideBase + ((plane.distToPlane(v0.point) >= 0)? 0: 1);

         InteriorPolytope::Vertex& v1 = polytope.mVertexList[edge.vertex[1]];
         if (v1.side < sideBase)
            v1.side = sideBase + ((plane.distToPlane(v1.point) >= 0)? 0: 1);

         if (v0.side != v1.side) 
         {
            S32 s = v0.side - sideBase;
            polytope.intersect(plane,v0.point,v1.point);

            // Split the edge into each volume
            polytope.mEdgeList.increment(2);
            InteriorPolytope::Edge& e0 = polytope.mEdgeList.last();
            e0.next = frontVolume.edgeList;
            frontVolume.edgeList = polytope.mEdgeList.size() - 1;

            InteriorPolytope::Edge& e1 = *(&e0 - 1);
            e1.next = backVolume.edgeList;
            backVolume.edgeList = frontVolume.edgeList - 1;

            e0.vertex[0] = edge.vertex[s];
            e1.vertex[0] = edge.vertex[s ^ 1];
            e0.vertex[1] = e1.vertex[1] = polytope.mVertexList.size() - 1;
            e0.face[0] = e1.face[0] = edge.face[0];
            e0.face[1] = e1.face[1] = edge.face[1];

            // Add new edges on the plane, one to each volume
            for (S32 f = 0; f < 2; f++) 
            {
               InteriorPolytope::Face& face = polytope.mFaceList[edge.face[f]];
               if (face.vertex < startVertex) 
               {
                  face.vertex = polytope.mVertexList.size() - 1;
               }
               else 
               {
                  polytope.mEdgeList.increment(2);
                  InteriorPolytope::Edge& e0 = polytope.mEdgeList.last();
                  e0.next = frontVolume.edgeList;
                  frontVolume.edgeList = polytope.mEdgeList.size() - 1;

                  InteriorPolytope::Edge& e1 = *(&e0 - 1);
                  e1.next = backVolume.edgeList;
                  backVolume.edgeList = frontVolume.edgeList - 1;

                  e1.vertex[0] = e0.vertex[0] = face.vertex;
                  e1.vertex[1] = e0.vertex[1] = polytope.mVertexList.size() - 1;
                  e1.face[0] = e0.face[0] = edge.face[f];
                  e1.face[1] = polytope.mFaceList.size() - 1;
                  e0.face[1] = e1.face[1] - 1;
               }
            }
         }
         else 
         {
            if (v0.side == sideBase) 
            {
               polytope.mEdgeList.push_back(edge);
               InteriorPolytope::Edge& ne = polytope.mEdgeList.last();
               ne.next = frontVolume.edgeList;
               frontVolume.edgeList = polytope.mEdgeList.size() - 1;
            }
            else 
            {
               polytope.mEdgeList.push_back(edge);
               InteriorPolytope::Edge& ne = polytope.mEdgeList.last();
               ne.next = backVolume.edgeList;
               backVolume.edgeList = polytope.mEdgeList.size() - 1;
            }
         }
      }

      // Push the front and back nodes
      if (frontVolume.edgeList >= 0 && backVolume.edgeList >= 0) 
      {
         collPlanes.push_back(rNode.planeIndex);

         frontVolume.nodeIndex = rNode.frontIndex;
         frontVolume.popCount  = volume.popCount + 1;
         stack.push_back(frontVolume);

         backVolume.nodeIndex = rNode.backIndex;
         backVolume.popCount = 0;
         stack.push_back(backVolume);
      }
      else if (frontVolume.edgeList >= 0) 
      {
         frontVolume.nodeIndex = rNode.frontIndex;
         frontVolume.popCount  = volume.popCount;
         stack.push_back(frontVolume);
      }
      else if (backVolume.edgeList >= 0) 
      {
         backVolume.nodeIndex = rNode.backIndex;
         backVolume.popCount = volume.popCount;
         stack.push_back(backVolume);
      }
      else
      {
         // Pop off our own planes...
         if (volume.popCount)
            for (U32 i = 0; i < volume.popCount; i++)
               collPlanes.pop_back();
      }
   }

   AssertFatal(collPlanes.size() == 0, "Unbalanced stack!");
}

bool Interior::buildPolyList(AbstractPolyList* list,
                             const Box3F&      box,
                             const MatrixF&    transform,
                             const Point3F&    scale)
{
   Box3F testBox;
   MatrixF toItr;
   if (!list->getMapping(&toItr,&testBox))
   {
      // this list doesn't do this, use world space box and transform
      testBox = box;
      toItr = transform;
   }

   // construct an interior space box from testBox and toItr
   // source space may be world space, or may be something else...
   // that's up to the list
   // Note: transform maps to interior, but scale is itr -> world...
   //       that is why we divide by scale below...
   F32 * f = toItr;

   F32 xx = mFabs(f[0]); F32 xy = mFabs(f[4]); F32 xz = mFabs(f[8]);
   F32 yx = mFabs(f[1]); F32 yy = mFabs(f[5]); F32 yz = mFabs(f[9]);
   F32 zx = mFabs(f[2]); F32 zy = mFabs(f[6]); F32 zz = mFabs(f[10]);

   F32 xlen = testBox.maxExtents.x - testBox.minExtents.x;
   F32 ylen = testBox.maxExtents.y - testBox.minExtents.y;
   F32 zlen = testBox.maxExtents.z - testBox.minExtents.z;

   F32 invScalex = 1.0f/scale.x;
   F32 invScaley = 1.0f/scale.y;
   F32 invScalez = 1.0f/scale.z;

   F32 xrad = (xx * xlen + yx * ylen + zx * zlen) * invScalex;
   F32 yrad = (xy * xlen + yy * ylen + zy * zlen) * invScaley;
   F32 zrad = (xz * xlen + yz * ylen + zz * zlen) * invScalez;

   Box3F interiorBox;
   testBox.getCenter(&interiorBox.minExtents);
   toItr.mulP(interiorBox.minExtents);

   interiorBox.minExtents.x *= invScalex;
   interiorBox.minExtents.y *= invScaley;
   interiorBox.minExtents.z *= invScalez;
   
   interiorBox.maxExtents = interiorBox.minExtents;

   interiorBox.minExtents.x -= xrad;
   interiorBox.minExtents.y -= yrad;
   interiorBox.minExtents.z -= zrad;
   
   interiorBox.maxExtents.x += xrad;
   interiorBox.maxExtents.y += yrad;
   interiorBox.maxExtents.z += zrad;

   U32 waterMark = FrameAllocator::getWaterMark();

   U16* hulls = (U16*)FrameAllocator::alloc(mConvexHulls.size() * sizeof(U16));
   U32 numHulls = 0;

   getIntersectingHulls(interiorBox,hulls, &numHulls);

   if (numHulls == 0) 
   {
      FrameAllocator::setWaterMark(waterMark);
      return false;
   }

   // we've found all the hulls that intersect the lists interior space bounding box...
   // now cull out those hulls which don't intersect the oriented bounding box...

   Point3F radii = testBox.maxExtents - testBox.minExtents;
   radii *= 0.5f;
   radii.x *= invScalex;
   radii.y *= invScaley;
   radii.z *= invScalez;

   // adjust toItr transform so that origin of source space is box center
   // Note:  center of interior box will be = to transformed center of testBox
   Point3F center = interiorBox.minExtents + interiorBox.maxExtents;
   center *= 0.5f;
   toItr.setColumn(3,center); // (0,0,0) now goes where box center used to...

   for (S32 i=0; i<numHulls; i++)
   {
      const ConvexHull & hull = mConvexHulls[hulls[i]];
      Box3F hullBox(hull.minX,hull.minY,hull.minZ,hull.maxX,hull.maxY,hull.maxZ);
      if (!hullBox.collideOrientedBox(radii,toItr))
         // oriented bounding boxes don't intersect...
         continue;

      // test for static mesh removal...
      if(hull.staticMesh && Interior::smLightingBuildPolyList)
         continue;

      for (S32 j=0; j<hull.surfaceCount; j++)
      {
         U32 surfaceIndex = mHullSurfaceIndices[j+hull.surfaceStart];
         if (isNullSurfaceIndex(surfaceIndex))
         {
            // Is a NULL surface
            const Interior::NullSurface& rSurface = mNullSurfaces[getNullSurfaceIndex(surfaceIndex)];
            U32 array[32];

            list->begin(0, rSurface.planeIndex);
            for (U32 k = 0; k < rSurface.windingCount; k++)
            {
               array[k] = list->addPoint(mPoints[mWindings[rSurface.windingStart + k]].point);
               list->vertex(array[k]);
            }

            list->plane(getFlippedPlane(rSurface.planeIndex));
            list->end();
         }
         else
         {
            const Interior::Surface& rSurface = mSurfaces[surfaceIndex];
            U32 array[32];
            U32 fanVerts[32];
            U32 numVerts;

            collisionFanFromSurface(rSurface, fanVerts, &numVerts);

            list->begin(0, rSurface.planeIndex);
            for (U32 k = 0; k < numVerts; k++)
            {
               array[k] = list->addPoint(mPoints[fanVerts[k]].point);
               list->vertex(array[k]);
            }
            list->plane(getFlippedPlane(rSurface.planeIndex));
            list->end();
         }
      }
   }

   FrameAllocator::setWaterMark(waterMark);
   return !list->isEmpty();
}


//---------------------------------------------------------------------------------
//-------------------------------------- Zone scan.  Not really collision, but hey.
//
struct Edge
{
   U16   p1;
   U16   p2;
   Edge(U16 o, U16 t) : p1(o), p2(t) { }
};

struct EdgeList
{

   Vector<Point3F> points;
   Vector<Edge>    edges;
};

void Interior::scanZoneNew(InteriorPolytope& polytope,
                           U16*           zones,
                           U32*           numZones)
{
   PROFILE_START(InteriorScanZoneNew);
   // Submit the first volume of the poly tope to the bsp tree
   static InteriorPolytope::VolumeStack stack;
   stack.reserve(128);
   stack.setSize(1);
   stack.last().edgeList  = polytope.mVolumeList[0].edgeList;
   stack.last().nodeIndex = 0;
   stack.last().popCount  = 0;

   static Vector<U16> collPlanes;
   collPlanes.reserve(64);
   collPlanes.clear();

   while (stack.empty() == false) 
   {

      InteriorPolytope::StackElement volume = stack.last();
      stack.pop_back();

      if (isBSPLeafIndex(volume.nodeIndex)) 
      {
         if (isBSPEmptyLeaf(volume.nodeIndex)) 
         {
            U16 zone = getBSPEmptyLeafZone(volume.nodeIndex);
            if (zone != 0x0FFF) 
            {
               bool insert = true;
               for (U32 i = 0; i < *numZones; i++) 
               {
                  if (zones[i] == zone) 
                  {
                     insert = false;
                     break;
                  }
               }
               if (insert)
               {
                  zones[*numZones] = zone;
                  (*numZones)++;
               }
            }
         }

         if (volume.popCount)
            for (U32 i = 0; i < volume.popCount; i++)
               collPlanes.pop_back();

         continue;
      }

      const IBSPNode& rNode = mBSPNodes[volume.nodeIndex];
      if ((rNode.terminalZone & U16(0x8000)) != 0)
      {
         // Hah!  we don't need to search any further
         U16 zone = rNode.terminalZone & (~0x8000);
         if (zone != 0x7FFF && zone != 0x0FFF)
         {
            bool insert = true;
            for (U32 i = 0; i < *numZones; i++) 
            {
               if (zones[i] == zone) 
               {
                  insert = false;
                  break;
               }
            }
            if (insert)
            {
               zones[*numZones] = zone;
               (*numZones)++;
            }
         }

         if (volume.popCount)
            for (U32 i = 0; i < volume.popCount; i++)
               collPlanes.pop_back();

         continue;
      }

      // New front and back faces
      polytope.mFaceList.increment(2);
      InteriorPolytope::Face& frontFace = polytope.mFaceList.last();
      InteriorPolytope::Face& backFace = *(&frontFace - 1);

      backFace.original = frontFace.original = false;
      backFace.vertex = frontFace.vertex = 0;

      // New front and back volumes
      InteriorPolytope::StackElement frontVolume,backVolume;
      frontVolume.edgeList = backVolume.edgeList = -1;

      PlaneF plane = getFlippedPlane(rNode.planeIndex);

      S32 startVertex = polytope.mVertexList.size();

      // Test & clip all the edges
      S32 sideBase = ++polytope.sideCount << 1;
      AssertFatal(sideBase != 0, "Well, crap.");
      for (S32 i = volume.edgeList; i >= 0; i = polytope.mEdgeList[i].next)
      {
         // Copy into tmp first to avoid problems with the array.
         InteriorPolytope::Edge edge = polytope.mEdgeList[i];

         InteriorPolytope::Vertex& v0 = polytope.mVertexList[edge.vertex[0]];
         if (v0.side < sideBase)
            v0.side = sideBase + ((plane.distToPlane(v0.point) >= 0)? 0: 1);
         InteriorPolytope::Vertex& v1 = polytope.mVertexList[edge.vertex[1]];
         if (v1.side < sideBase)
            v1.side = sideBase + ((plane.distToPlane(v1.point) >= 0)? 0: 1);

         if (v0.side != v1.side) 
         {
            S32 s = v0.side - sideBase;
            polytope.intersect(plane,v0.point,v1.point);

            // Split the edge into each volume
            polytope.mEdgeList.increment(2);
            InteriorPolytope::Edge& e0 = polytope.mEdgeList.last();
            e0.next = frontVolume.edgeList;
            frontVolume.edgeList = polytope.mEdgeList.size() - 1;

            InteriorPolytope::Edge& e1 = *(&e0 - 1);
            e1.next = backVolume.edgeList;
            backVolume.edgeList = frontVolume.edgeList - 1;

            e0.vertex[0] = edge.vertex[s];
            e1.vertex[0] = edge.vertex[s ^ 1];
            e0.vertex[1] = e1.vertex[1] = polytope.mVertexList.size() - 1;
            e0.face[0] = e1.face[0] = edge.face[0];
            e0.face[1] = e1.face[1] = edge.face[1];

            // Add new edges on the plane, one to each volume
            for (S32 f = 0; f < 2; f++) 
            {
               InteriorPolytope::Face& face = polytope.mFaceList[edge.face[f]];
               if (face.vertex < startVertex) 
               {
                  face.vertex = polytope.mVertexList.size() - 1;
               } 
               else 
               {
                  polytope.mEdgeList.increment(2);
                  InteriorPolytope::Edge& e0 = polytope.mEdgeList.last();
                  e0.next = frontVolume.edgeList;
                  frontVolume.edgeList = polytope.mEdgeList.size() - 1;

                  InteriorPolytope::Edge& e1 = *(&e0 - 1);
                  e1.next = backVolume.edgeList;
                  backVolume.edgeList = frontVolume.edgeList - 1;

                  e1.vertex[0] = e0.vertex[0] = face.vertex;
                  e1.vertex[1] = e0.vertex[1] = polytope.mVertexList.size() - 1;
                  e1.face[0] = e0.face[0] = edge.face[f];
                  e1.face[1] = polytope.mFaceList.size() - 1;
                  e0.face[1] = e1.face[1] - 1;
               }
            }
         }
         else 
         {
            if (v0.side == sideBase) 
            {
               polytope.mEdgeList.push_back(edge);
               InteriorPolytope::Edge& ne = polytope.mEdgeList.last();
               ne.next = frontVolume.edgeList;
               frontVolume.edgeList = polytope.mEdgeList.size() - 1;
            }
            else 
            {
               polytope.mEdgeList.push_back(edge);
               InteriorPolytope::Edge& ne = polytope.mEdgeList.last();
               ne.next = backVolume.edgeList;
               backVolume.edgeList = polytope.mEdgeList.size() - 1;
            }
         }
      }

      // Push the front and back nodes
      if (frontVolume.edgeList >= 0 && backVolume.edgeList >= 0) 
      {
         collPlanes.push_back(rNode.planeIndex);

         frontVolume.nodeIndex = rNode.frontIndex;
         frontVolume.popCount  = volume.popCount + 1;
         stack.push_back(frontVolume);

         backVolume.nodeIndex = rNode.backIndex;
         backVolume.popCount = 0;
         stack.push_back(backVolume);
      }
      else if (frontVolume.edgeList >= 0) 
      {
         frontVolume.nodeIndex = rNode.frontIndex;
         frontVolume.popCount  = volume.popCount;
         stack.push_back(frontVolume);
      }
      else if (backVolume.edgeList >= 0) 
      {
         backVolume.nodeIndex = rNode.backIndex;
         backVolume.popCount = volume.popCount;
         stack.push_back(backVolume);
      }
      else 
      {
         // Pop off our own planes...
         if (volume.popCount)
            for (U32 i = 0; i < volume.popCount; i++)
               collPlanes.pop_back();
      }
   }

   AssertFatal(collPlanes.size() == 0, "Unbalanced stack!");
   PROFILE_END();
}

void Interior::scanZone_r(const U32      node,
                          const Point3F& center,
                          const Point3F& axisx,
                          const Point3F& axisy,
                          const Point3F& axisz,
                          U16*           zones,
                          U32*           numZones)
{
   if (isBSPLeafIndex(node) == false) 
   {
      const IBSPNode& rNode = mBSPNodes[node];
      const PlaneF& rPlane  = getPlane(rNode.planeIndex);

      PlaneF::Side side = rPlane.whichSideBox(center, axisx, axisy, axisz);
      if (planeIsFlipped(rNode.planeIndex))
         side = PlaneF::Side(-S32(side));

      switch (side) 
      {
      case PlaneF::Front:
         scanZone_r(rNode.frontIndex, center, axisx, axisy, axisz, zones, numZones);
         break;

      case PlaneF::Back:
         scanZone_r(rNode.backIndex, center, axisx, axisy, axisz, zones, numZones);
         break;

      case PlaneF::On:
         scanZone_r(rNode.frontIndex, center, axisx, axisy, axisz, zones, numZones);
         scanZone_r(rNode.backIndex, center, axisx, axisy, axisz, zones, numZones);
         break;

      default:
         AssertFatal(false, "Misunderstood switchCode in Interior::zoneRay_r");
      }

      return;
   }

   if (isBSPEmptyLeaf(node)) 
   {
      U16 zone = getBSPEmptyLeafZone(node);
      if (zone != 0x0FFF) 
      {
         for (U32 i = 0; i < *numZones; i++) 
         {
            if (zones[i] == zone)
               return;
         }

         zones[*numZones] = zone;
         (*numZones)++;
      }
   }
}

bool Interior::scanZones(const Box3F&   box,
                         const MatrixF& transform,
                         U16*           zones,
                         U32*           numZones)
{
   // We don't need an exact answer, so let's just blast a box through
   // the planes and see what we intersect. scanZoneNew is good if you
   // have an exact volume to clip.

   Point3F center;
   box.getCenter(&center);

   Point3F xRad((box.maxExtents.x - box.minExtents.x) * 0.5f, 0.0f, 0.0f);
   Point3F yRad(0.0f, (box.maxExtents.y - box.minExtents.y) * 0.5f, 0.0f);
   Point3F zRad(0.0f, 0.0f, (box.maxExtents.z - box.minExtents.z) * 0.5f);

   transform.mulP(center);
   transform.mulV(xRad);
   transform.mulV(yRad);
   transform.mulV(zRad);

   scanZone_r(0, center, xRad, yRad, zRad, zones, numZones);

   bool outsideToo = false;
   if (*numZones != 0) 
   {
      for (U32 i = 0; i < *numZones; /**/) 
      {
         if (zones[i])
         {
            i++;
            continue;
         }

         outsideToo = true;

         zones[i] = zones[(*numZones) - 1];
         (*numZones)--;
      }
   }
   else 
   {
      // If it ain't in us, it's outside us.
      outsideToo = true;
   }

   return outsideToo;
}


bool Interior::getIntersectingHulls(const Box3F& query, U16* hulls, U32* numHulls)
{
   AssertFatal(*numHulls == 0, "Error, some stuff in the hull vector already!");

   // This is paranoia, and I probably wouldn't do it if the tag was 32 bits, but
   //  a possible collision every 65k searches is just a little too small for comfort
   // DMM
   if (mSearchTag == 0) 
   {
      for (U32 i = 0; i < mConvexHulls.size(); i++)
         mConvexHulls[i].searchTag = 0;
      mSearchTag = 1;
   }
   else 
   {
      mSearchTag++;
   }

   F32 xBinSize = mBoundingBox.len_x() / F32(NumCoordBins);
   F32 yBinSize = mBoundingBox.len_y() / F32(NumCoordBins);

   F32 queryMinX = getMax(mBoundingBox.minExtents.x, query.minExtents.x);
   F32 queryMinY = getMax(mBoundingBox.minExtents.y, query.minExtents.y);
   F32 queryMaxX = getMin(mBoundingBox.maxExtents.x, query.maxExtents.x);
   F32 queryMaxY = getMin(mBoundingBox.maxExtents.y, query.maxExtents.y);

   S32 startX = S32(mFloor((queryMinX - mBoundingBox.minExtents.x) / xBinSize));
   S32 endX   = S32( mCeil((queryMaxX - mBoundingBox.minExtents.x) / xBinSize));
   S32 startY = S32(mFloor((queryMinY - mBoundingBox.minExtents.y) / yBinSize));
   S32 endY   = S32( mCeil((queryMaxY - mBoundingBox.minExtents.y) / yBinSize));

   AssertFatal(startX >= 0, "Error, bad startx");
   AssertFatal(startY >= 0, "Error, bad starty");
   AssertFatal(endX <= NumCoordBins, "Error, bad endX");
   AssertFatal(endY <= NumCoordBins, "Error, bad endY");

   // Handle non-debug case
   startX = getMax(startX, 0);
   endX   = getMin(endX,   NumCoordBins);
   startY = getMax(startY, 0);
   endY   = getMin(endY,   NumCoordBins);

   for (S32 i = startX; i < endX; i++) 
   {
      for (S32 j = startY; j < endY; j++) 
      {
         const CoordBin& rBin = mCoordBins[i * NumCoordBins + j];
         for (U32 k = rBin.binStart; k < rBin.binStart + rBin.binCount; k++) 
         {
            U16 hullIndex = mCoordBinIndices[k];
            ConvexHull& rHull = mConvexHulls[hullIndex];

            // Don't check twice! (We're not Santa Claus.)
            if (rHull.searchTag == mSearchTag)
               continue;
            rHull.searchTag = mSearchTag;

            Box3F qb(rHull.minX, rHull.minY, rHull.minZ, rHull.maxX, rHull.maxY, rHull.maxZ);
            if (query.isOverlapped(qb)) 
            {
               hulls[*numHulls] = hullIndex;
               (*numHulls)++;
            }
         }
      }
   }

   return *numHulls != 0;
}


bool Interior::getIntersectingVehicleHulls(const Box3F& query, U16* hulls, U32* numHulls)
{
   AssertFatal(*numHulls == 0, "Error, some stuff in the hull vector already!");

   for (U16 i = 0; i < mVehicleConvexHulls.size(); i++)
   {
      ConvexHull& rHull = mVehicleConvexHulls[i];
      Box3F qb(rHull.minX, rHull.minY, rHull.minZ, rHull.maxX, rHull.maxY, rHull.maxZ);
      if (query.isOverlapped(qb)) 
      {
         hulls[*numHulls] = i;
         (*numHulls)++;
      }
   }

   return *numHulls != 0;
}

//--------------------------------------------------------------------------
Box3F InteriorConvex::getBoundingBox() const
{
   return getBoundingBox(mObject->getTransform(), mObject->getScale());
}

Box3F InteriorConvex::getBoundingBox(const MatrixF& mat, const Point3F& scale) const
{
   Box3F newBox = box;
   newBox.minExtents.convolve(scale);
   newBox.maxExtents.convolve(scale);
   mat.mul(newBox);
   return newBox;
}

Point3F InteriorConvex::support(const VectorF& v) const
{
   FrameAllocatorMarker fam;

   if (hullId >= 0)
   {
      AssertFatal(hullId < pInterior->mConvexHulls.size(), "Out of bounds hull!");

      const Interior::ConvexHull& rHull = pInterior->mConvexHulls[hullId];

      F32* pDots = (F32*)fam.alloc(sizeof(F32) * rHull.hullCount);
      m_point3F_bulk_dot_indexed(&v.x,
                                 &pInterior->mPoints[0].point.x,
                                 rHull.hullCount,
                                 sizeof(ItrPaddedPoint),
                                 &pInterior->mHullIndices[rHull.hullStart],
                                 pDots);

      U32 index = 0;
      for (U32 i = 1; i < rHull.hullCount; i++) 
      {
         if (pDots[i] > pDots[index])
            index = i;
      }

      return pInterior->mPoints[pInterior->mHullIndices[rHull.hullStart + index]].point;
   }
   else
   {
      S32 actualId = -(hullId + 1);
      AssertFatal(actualId < pInterior->mVehicleConvexHulls.size(), "Out of bounds hull!");

      const Interior::ConvexHull& rHull = pInterior->mVehicleConvexHulls[actualId];

      F32* pDots = (F32*)fam.alloc(sizeof(F32) * rHull.hullCount);
      m_point3F_bulk_dot_indexed(&v.x,
                                 &pInterior->mVehiclePoints[0].point.x,
                                 rHull.hullCount,
                                 sizeof(ItrPaddedPoint),
                                 &pInterior->mVehicleHullIndices[rHull.hullStart],
                                 pDots);

      U32 index = 0;
      for (U32 i = 1; i < rHull.hullCount; i++) 
      {
         if (pDots[i] > pDots[index])
            index = i;
      }

      return pInterior->mVehiclePoints[pInterior->mVehicleHullIndices[rHull.hullStart + index]].point;
   }
}


void InteriorConvex::getFeatures(const MatrixF& mat, const VectorF& n, ConvexFeature* cf)
{
   S32 i;

   cf->material = 0;
   cf->object   = mObject;

   if (hullId >= 0)
   {
      // We find the support ourselves here since we want the index too...
      const Interior::ConvexHull& rHull = pInterior->mConvexHulls[hullId];
      U32 spIndex = 0;
      F32   maxSp = mDot(pInterior->mPoints[pInterior->mHullIndices[rHull.hullStart + spIndex]].point, n);

      for (i = 1; i < rHull.hullCount; i++) 
      {
         U32 index             = pInterior->mHullIndices[rHull.hullStart + i];
         const Point3F& rPoint = pInterior->mPoints[index].point;

         F32 dot = mDot(rPoint, n);
         if (dot > maxSp) 
         {
            spIndex = i;
            maxSp   = dot;
         }
      }

      // Ok, now we have the support point, let's extract the emission string for this
      //  vertex...
      U32 currPos = 0;
      const U8* pString = &pInterior->mConvexHullEmitStrings[pInterior->mHullEmitStringIndices[rHull.hullStart + spIndex]];

      FrameAllocatorMarker fam;
      U32* pRemaps = (U32*)fam.alloc(256 * sizeof(U32));

      // Ok, this is a piece of cake.  Lets dump the points first...
      U32 numPoints = pString[currPos++];
      for (i = 0; i < numPoints; i++) 
      {
         U32 index = pString[currPos++];

         pRemaps[i] = cf->mVertexList.size();
         cf->mVertexList.increment();

         const Point3F& rPoint = pInterior->mPoints[pInterior->mHullIndices[rHull.hullStart + index]].point;
         mat.mulP(rPoint, &cf->mVertexList.last());
      }

      // Then the edges...
      U32 numEdges = pString[currPos++];
      for (i = 0; i < numEdges; i++) 
      {
         U32 index0 = pString[currPos++];
         U32 index1 = pString[currPos++];

         cf->mEdgeList.increment();
         cf->mEdgeList.last().vertex[0] = pRemaps[index0];
         cf->mEdgeList.last().vertex[1] = pRemaps[index1];
      }

      // Then the polys...
      U32 numPolys = pString[currPos++];
      for (i = 0; i < numPolys; i++) 
      {
         U32 vertexCount = pString[currPos++];
         U32 planeIndex  = pString[currPos++];

         U32 verts[3];
         verts[0] = pString[currPos++];
         verts[1] = pString[currPos++];

         AssertFatal( verts[0] < numPoints, "InteriorConvex::getFeatures verts out of range" );

         for (U32 j = 2; j < vertexCount; j++) 
         {
            verts[2] = pString[currPos++];

            // Emit this poly
            cf->mFaceList.increment();
            cf->mFaceList.last().normal = pInterior->getPlane(pInterior->mHullPlaneIndices[rHull.planeStart + planeIndex]);

            AssertFatal( verts[1] < numPoints, "InteriorConvex::getFeatures verts out of range" );
            AssertFatal( verts[2] < numPoints, "InteriorConvex::getFeatures verts out of range" );
               
            cf->mFaceList.last().vertex[0] = pRemaps[verts[0]];
            cf->mFaceList.last().vertex[1] = pRemaps[verts[1]];
            cf->mFaceList.last().vertex[2] = pRemaps[verts[2]];
            PlaneF plane(cf->mVertexList[pRemaps[verts[0]]],
                         cf->mVertexList[pRemaps[verts[1]]],
                         cf->mVertexList[pRemaps[verts[2]]]);
            cf->mFaceList.last().normal = plane;

            // Shift the fan over
            verts[1] = verts[2];
         }
      }
   }
   else
   {
      S32 actualId = -(hullId + 1);
      // We find the support ourselves here since we want the index too...
      const Interior::ConvexHull& rHull = pInterior->mVehicleConvexHulls[actualId];
      U32 spIndex = 0;
      F32   maxSp = mDot(pInterior->mVehiclePoints[pInterior->mVehicleHullIndices[rHull.hullStart + spIndex]].point, n);

      for (i = 1; i < rHull.hullCount; i++) 
      {
         U32 index             = pInterior->mVehicleHullIndices[rHull.hullStart + i];
         const Point3F& rPoint = pInterior->mVehiclePoints[index].point;

         F32 dot = mDot(rPoint, n);
         if (dot > maxSp) 
         {
            spIndex = i;
            maxSp   = dot;
         }
      }

      // Ok, now we have the support point, let's extract the emission string for this
      //  vertex...
      U32 currPos = 0;
      const U8* pString = &pInterior->mVehicleConvexHullEmitStrings[
                              pInterior->mVehicleHullEmitStringIndices[rHull.hullStart + spIndex]
                           ];

      FrameAllocatorMarker fam;
      U32* pRemaps = (U32*)fam.alloc(256 * sizeof(U32));

      // Ok, this is a piece of cake.  Lets dump the points first...
      U32 numPoints = pString[currPos++];
      for (i = 0; i < numPoints; i++) 
      {
         U32 index = pString[currPos++];

         pRemaps[i] = cf->mVertexList.size();
         cf->mVertexList.increment();

         const Point3F& rPoint = pInterior->mVehiclePoints[pInterior->mVehicleHullIndices[rHull.hullStart + index]].point;
         mat.mulP(rPoint, &cf->mVertexList.last());
      }

      // Then the edges...
      U32 numEdges = pString[currPos++];
      for (i = 0; i < numEdges; i++) 
      {
         U32 index0 = pString[currPos++];
         U32 index1 = pString[currPos++];

         cf->mEdgeList.increment();
         cf->mEdgeList.last().vertex[0] = pRemaps[index0];
         cf->mEdgeList.last().vertex[1] = pRemaps[index1];
      }

      // Then the polys...
      U32 numPolys = pString[currPos++];
      for (i = 0; i < numPolys; i++) 
      {
         U32 vertexCount = pString[currPos++];
         U32 planeIndex  = pString[currPos++];

         U32 verts[3];
         verts[0] = pString[currPos++];
         verts[1] = pString[currPos++];

         for (U32 j = 2; j < vertexCount; j++) 
         {
            verts[2] = pString[currPos++];

            // Emit this poly
            cf->mFaceList.increment();

            cf->mFaceList.last().vertex[0] = pRemaps[verts[0]];
            cf->mFaceList.last().vertex[1] = pRemaps[verts[1]];
            cf->mFaceList.last().vertex[2] = pRemaps[verts[2]];
            cf->mFaceList.last().normal = pInterior->getPlane(planeIndex);

            // Shift the fan over
            verts[1] = verts[2];
         }
      }
   }
}

void InteriorConvex::getPolyList(AbstractPolyList* list)
{
   // Setup collision state data
   {
      list->setTransform(&mObject->getTransform(), mObject->getScale());
      list->setObject(mObject);
   }

   if (hullId >= 0)
   {
      // Get our hull
      const Interior::ConvexHull& rHull = pInterior->mConvexHulls[hullId];

      // Build up the lists of points and strings
      const U8* pString = &pInterior->mPolyListStrings[rHull.polyListStringStart];

      U32 currPos    = 0;
      U32 numPlanes  = pString[currPos++];

      // It can happen that a hull has no collision surfaces.  In that case, just bail out
      //  here...
      if (numPlanes == 0)
         return;

      const U8* planeString = &pString[currPos];
      currPos += numPlanes;

      U32 numPoints         = pString[currPos++] << 8;
      numPoints            |= pString[currPos++];
      const U8* pointString = &pString[currPos];
      currPos += numPoints;

      U32 numSurfaces       = pString[currPos++];

      const U16* planeIndices = &pInterior->mPolyListPlanes[rHull.polyListPlaneStart];
      const U32* pointIndices = &pInterior->mPolyListPoints[rHull.polyListPointStart];

      //--------------------------------------
      // At this point, currPos is pointing to the first surface in the string
      //--------------------------------------

      // First thing to do: build the interest mask, by seeing if the list is interested
      //  in our planes...
      U8  interestMask = 0;
      U32 remappedPlaneBase;
      {
         U16 planeIndex = planeIndices[0];
         PlaneF plane = pInterior->getPlane(planeIndex);
         if (Interior::planeIsFlipped(planeIndex))
            plane.neg();

         remappedPlaneBase = list->addPlane(plane);

         if (list->isInterestedInPlane(remappedPlaneBase))
            interestMask |= planeString[0];

         for (U32 i = 1; i < numPlanes; i++) 
         {
            planeIndex = planeIndices[i];
            plane = pInterior->getPlane(planeIndex);
            if (Interior::planeIsFlipped(planeIndex))
               plane.neg();

            list->addPlane(plane);
            if (list->isInterestedInPlane(remappedPlaneBase + i))
               interestMask |= planeString[i];
         }
      }

      // Now, whip through the points, and build up the remap table, adding only
      //  those points that the list is interested in.  Note that we use the frameAllocator
      //  to get enoughMemory to deal with the variable sized remap array
      FrameAllocatorMarker fam;
      U32* pointRemapTable = reinterpret_cast<U32*>(fam.alloc(numPoints * sizeof(U32)));
      {
         for (U32 i = 0; i < numPoints; i++) 
         {
            if ((interestMask & pointString[i]) != 0) 
            {
               const Point3F& rPoint = pInterior->mPoints[pointIndices[i]].point;
               pointRemapTable[i] = list->addPoint(rPoint);
            }
         }
      }

      // Now, whip through the surfaces, checking to make sure that we're interested in
      //  that poly as we go.  At this point, currPos should point to the first surface.
      //  The format of the surface string can be found in interior.cc, in the
      //  processHullPolyLists function
      {
         for (U32 i = 0; i < numSurfaces; i++) 
         {
            U32 snPoints = pString[currPos++];
            U32 sMask    = pString[currPos++];
            U32 sPlane   = pString[currPos++];

            if ((interestMask & sMask) != 0) 
            {
               // Add the poly
               //
               list->begin(0, planeIndices[sPlane]);
               for (U32 j = 0; j < snPoints; j++) 
               {
                  U16 remappedIndex  = pString[currPos++] << 8;
                  remappedIndex     |= pString[currPos++];
                  remappedIndex      = pointRemapTable[remappedIndex];
                  list->vertex(remappedIndex);
               }
               list->plane(remappedPlaneBase + sPlane);

               list->end();
            }
            else 
            {
               // Superflous poly, just skip past the points
               currPos += snPoints * 2;
            }
         }
      }
   }
   else
   {
      S32 actualId = -(hullId + 1);

      // Get our hull
      const Interior::ConvexHull& rHull = pInterior->mVehicleConvexHulls[actualId];

      // Build up the lists of points and strings
      const U8* pString = &pInterior->mVehiclePolyListStrings[rHull.polyListStringStart];
      U32 currPos = 0;

      U32 numPlanes         = pString[currPos++];
      // It can happen that a hull has no collision surfaces.  In that case, just bail out
      //  here...
      if (numPlanes == 0)
         return;

      const U8* planeString = &pString[currPos];
      currPos += numPlanes;

      U32 numPoints         = pString[currPos++] << 8;
      numPoints            |= pString[currPos++];
      const U8* pointString = &pString[currPos];
      currPos += numPoints;

      U32 numSurfaces       = pString[currPos++];

      const U16* planeIndices = &pInterior->mVehiclePolyListPlanes[rHull.polyListPlaneStart];
      const U32* pointIndices = &pInterior->mVehiclePolyListPoints[rHull.polyListPointStart];

      //--------------------------------------
      // At this point, currPos is pointing to the first surface in the string
      //--------------------------------------

      // First thing to do: build the interest mask, by seeing if the list is interested
      //  in our planes...
      U8  interestMask = 0;
      U32 remappedPlaneBase;
      {
         U16 planeIndex = planeIndices[0];
         PlaneF plane = pInterior->getPlane(planeIndex);
         if (Interior::planeIsFlipped(planeIndex))
            plane.neg();

         remappedPlaneBase = list->addPlane(plane);

         if (list->isInterestedInPlane(remappedPlaneBase))
            interestMask |= planeString[0];

         for (U32 i = 1; i < numPlanes; i++) 
         {
            planeIndex = planeIndices[i];
            plane = pInterior->getPlane(planeIndex);
            if (Interior::planeIsFlipped(planeIndex))
               plane.neg();

            list->addPlane(plane);
            if (list->isInterestedInPlane(remappedPlaneBase + i))
               interestMask |= planeString[i];
         }
      }

      // Now, whip through the points, and build up the remap table, adding only
      //  those points that the list is interested in.  Note that we use the frameAllocator
      //  to get enoughMemory to deal with the variable sized remap array
      FrameAllocatorMarker fam;
      U32* pointRemapTable = reinterpret_cast<U32*>(fam.alloc(numPoints * sizeof(U32)));
      {
         for (U32 i = 0; i < numPoints; i++) 
         {
            if ((interestMask & pointString[i]) != 0) 
            {
               const Point3F& rPoint = pInterior->mVehiclePoints[pointIndices[i]].point;
               pointRemapTable[i] = list->addPoint(rPoint);
            }
         }
      }

      // Now, whip through the surfaces, checking to make sure that we're interested in
      //  that poly as we go.  At this point, currPos should point to the first surface.
      //  The format of the surface string can be found in interior.cc, in the
      //  processHullPolyLists function
      {
         for (U32 i = 0; i < numSurfaces; i++) 
         {
            U32 snPoints = pString[currPos++];
            U32 sMask    = pString[currPos++];
            U32 sPlane   = pString[currPos++];

            if ((interestMask & sMask) != 0) 
            {
               // Add the poly
               //
               list->begin(0, planeIndices[sPlane]);
               for (U32 j = 0; j < snPoints; j++) 
               {
                  U16 remappedIndex  = pString[currPos++] << 8;
                  remappedIndex     |= pString[currPos++];
                  remappedIndex      = pointRemapTable[remappedIndex];
                  list->vertex(remappedIndex);
               }
               list->plane(remappedPlaneBase + sPlane);

               list->end();
            }
            else 
            {
               // Superflous poly, just skip past the points
               currPos += snPoints * 2;
            }
         }
      }
   }
}


