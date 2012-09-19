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
#include "collision/collision.h"
#include "collision/polytope.h"

//----------------------------------------------------------------------------

Polytope::Polytope()
{
   VECTOR_SET_ASSOCIATION(mEdgeList);
   VECTOR_SET_ASSOCIATION(mFaceList);
   VECTOR_SET_ASSOCIATION(mVertexList);
   VECTOR_SET_ASSOCIATION(mVolumeList);

   mVertexList.reserve(100);
   mFaceList.reserve(200);
   mEdgeList.reserve(100);
   mVolumeList.reserve(20);
   sideCount = 0;
}


//----------------------------------------------------------------------------
// Box should be axis aligned in the transform space provided.

void Polytope::buildBox(const MatrixF& transform,const Box3F& box)
{
   // Box is assumed to be axis aligned in the source space.
   // Transform into geometry space
   Point3F xvec,yvec,zvec,min;
   transform.getColumn(0,&xvec);
   xvec *= box.len_x();
   transform.getColumn(1,&yvec);
   yvec *= box.len_y();
   transform.getColumn(2,&zvec);
   zvec *= box.len_z();
   transform.mulP(box.minExtents,&min);

   // Initial vertices
   mVertexList.setSize(8);
   mVertexList[0].point = min;
   mVertexList[1].point = min + yvec;
   mVertexList[2].point = min + xvec + yvec;
   mVertexList[3].point = min + xvec;
   mVertexList[4].point = mVertexList[0].point + zvec;
   mVertexList[5].point = mVertexList[1].point + zvec;
   mVertexList[6].point = mVertexList[2].point + zvec;
   mVertexList[7].point = mVertexList[3].point + zvec;
   S32 i;
   for (i = 0; i < 8; i++)
      mVertexList[i].side = 0;

   // Initial faces
   mFaceList.setSize(6);
   for (S32 f = 0; f < 6; f++) {
      Face& face = mFaceList[f];
      face.original = true;
      face.vertex = 0;
   }

   mFaceList[0].plane.set(mVertexList[0].point,xvec);
   mFaceList[0].plane.invert();
   mFaceList[1].plane.set(mVertexList[2].point,yvec);
   mFaceList[2].plane.set(mVertexList[2].point,xvec);
   mFaceList[3].plane.set(mVertexList[0].point,yvec);
   mFaceList[3].plane.invert();
   mFaceList[4].plane.set(mVertexList[0].point,zvec);
   mFaceList[4].plane.invert();
   mFaceList[5].plane.set(mVertexList[4].point,zvec);

   // Initial edges
   mEdgeList.setSize(12);
   Edge* edge = mEdgeList.begin();
   S32 nextEdge = 0;
   for (i = 0; i < 4; i++) {
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
   volume.material = -1;
   volume.object = 0;
   sideCount = 0;
}


//----------------------------------------------------------------------------

void Polytope::intersect(SimObject* object,const BSPNode* root)
{
   AssertFatal(mVolumeList.size() > 0,"Polytope::intersect: Missing initial volume");

   // Always clips the first volume against the BSP
   VolumeStack stack;
   stack.reserve(50);
   stack.increment();
   stack.last().edgeList = mVolumeList[0].edgeList;
   stack.last().node = root;

   while (!stack.empty()) {
      StackElement volume = stack.last();
      stack.pop_back();

      // If it's a solid node, export the volume
      const BSPNode* node = volume.node;
      if (!node->backNode && !node->frontNode) {
         mVolumeList.increment();
         Volume& vol = mVolumeList.last();
         vol.object = object;
         vol.material = node->material;
         vol.edgeList = volume.edgeList;
         continue;
      }

      // New front and back faces
      mFaceList.increment(2);
      Face& frontFace = mFaceList.last();
      Face& backFace = *(&frontFace - 1);

      backFace.original = frontFace.original = false;
      backFace.vertex = frontFace.vertex = 0;
      backFace.plane = frontFace.plane = node->plane;
      backFace.plane.invert();

      // New front and back volumes
      StackElement frontVolume,backVolume;
      frontVolume.edgeList = backVolume.edgeList = -1;

      const PlaneF& plane = node->plane;
      S32 startVertex = mVertexList.size();

      // Test & clip all the edges
      S32 sideBase = ++sideCount << 1;
      for (S32 i = volume.edgeList; i >= 0; i = mEdgeList[i].next) {

         // Copy into tmp first to avoid problems with the array.
         Edge edge = mEdgeList[i];

         Vertex& v0 = mVertexList[edge.vertex[0]];
         if (v0.side < sideBase)
            v0.side = sideBase + ((plane.distToPlane(v0.point) >= 0)? 0: 1);
         Vertex& v1 = mVertexList[edge.vertex[1]];
         if (v1.side < sideBase)
            v1.side = sideBase + ((plane.distToPlane(v1.point) >= 0)? 0: 1);

         if (v0.side != v1.side) {
            S32 s = v0.side - sideBase;
            intersect(plane,v0.point,v1.point);

            // Split the edge into each volume
            mEdgeList.increment(2);
            Edge& e0 = mEdgeList.last();
            e0.next = frontVolume.edgeList;
            frontVolume.edgeList = mEdgeList.size() - 1;

            Edge& e1 = *(&e0 - 1);
            e1.next = backVolume.edgeList;
            backVolume.edgeList = frontVolume.edgeList - 1;

            e0.vertex[0] = edge.vertex[s];
            e1.vertex[0] = edge.vertex[s ^ 1];
            e0.vertex[1] = e1.vertex[1] = mVertexList.size() - 1;
            e0.face[0] = e1.face[0] = edge.face[0];
            e0.face[1] = e1.face[1] = edge.face[1];

            // Add new edges on the plane, one to each volume
            for (S32 f = 0; f < 2; f++) {
               Face& face = mFaceList[edge.face[f]];
               if (face.vertex < startVertex)
                  face.vertex = mVertexList.size() - 1;
               else {
                  mEdgeList.increment(2);
                  Edge& e0 = mEdgeList.last();
                  e0.next = frontVolume.edgeList;
                  frontVolume.edgeList = mEdgeList.size() - 1;

                  Edge& e1 = *(&e0 - 1);
                  e1.next = backVolume.edgeList;
                  backVolume.edgeList = frontVolume.edgeList - 1;

                  e1.vertex[0] = e0.vertex[0] = face.vertex;
                  e1.vertex[1] = e0.vertex[1] = mVertexList.size() - 1;
                  e1.face[0] = e0.face[0] = edge.face[f];
                  e1.face[1] = mFaceList.size() - 1;
                  e0.face[1] = e1.face[1] - 1;
               }
            }
         }
         else
            if (v0.side == sideBase) {
               mEdgeList.push_back(edge);
               Edge& ne = mEdgeList.last();
               ne.next = frontVolume.edgeList;
               frontVolume.edgeList = mEdgeList.size() - 1;
            }
            else {
               mEdgeList.push_back(edge);
               Edge& ne = mEdgeList.last();
               ne.next = backVolume.edgeList;
               backVolume.edgeList = mEdgeList.size() - 1;
            }
      }

      // Push the front and back nodes
      if (node->frontNode && frontVolume.edgeList >= 0) {
         frontVolume.node = node->frontNode;
         stack.push_back(frontVolume);
      }
      if (node->backNode && backVolume.edgeList >= 0) {
         backVolume.node = node->backNode;
         stack.push_back(backVolume);
      }
   }
}


//----------------------------------------------------------------------------

bool Polytope::intersect(const PlaneF& plane,const Point3F& sp,const Point3F& ep)
{
   // If den == 0 then the line and plane are parallel.
   F32 den;
   Point3F dt = ep - sp;
   if ((den = plane.x * dt.x + plane.y * dt.y + plane.z * dt.z) == 0)
      return false;

   mVertexList.increment();
   Vertex& v = mVertexList.last();
   F32 s = -(plane.x * sp.x + plane.y * sp.y + plane.z * sp.z + plane.d) / den;
   v.point.x = sp.x + dt.x * s;
   v.point.y = sp.y + dt.y * s;
   v.point.z = sp.z + dt.z * s;
   v.side = 0;
   return true;
}

//----------------------------------------------------------------------------

void Polytope::extrudeFace(int faceIdx,const VectorF& vec,Polytope* out)
{
   // Assumes the face belongs to the first volume.
   out->mVertexList.clear();
   out->mFaceList.clear();
   out->mEdgeList.clear();
   out->mVolumeList.clear();
   sideCount++;

   // Front & end faces
   Face nface;
   nface.original = true;
   nface.vertex = 0;
   nface.plane = mFaceList[faceIdx].plane;
   out->mFaceList.setSize(2);
   out->mFaceList[0] = out->mFaceList[1] = nface;
   out->mFaceList[0].plane.invert();

   for (S32 e = mVolumeList[0].edgeList; e >= 0; e = mEdgeList[e].next) {
      Edge& edge = mEdgeList[e];
      if (edge.face[0] == faceIdx || edge.face[1] == faceIdx) {

         // Build face for this edge
         // Should think about calulating the plane
         S32 fi = out->mFaceList.size();
         out->mFaceList.push_back(nface);

         // Reserve 4 entries to make sure the ve[] pointers
         // into the list don't get invalidated.
         out->mEdgeList.reserve(out->mEdgeList.size() + 4);
         Edge* ve[2];

         // Build edges for each vertex
         for (S32 v = 0; v < 2; v++) {
            if (mVertexList[edge.vertex[v]].side < sideCount) {
               mVertexList[edge.vertex[v]].side = sideCount + out->mEdgeList.size();

               out->mVertexList.increment(2);
               out->mVertexList.end()[-1] =
                  out->mVertexList.end()[-2] =
                     mVertexList[edge.vertex[v]];
               out->mVertexList.last().point += vec;

               out->mEdgeList.increment();
               Edge& ne = out->mEdgeList.last();
               ne.next = out->mEdgeList.size();
               ne.vertex[1] = out->mVertexList.size() - 1;
               ne.vertex[0] = ne.vertex[1] - 1;
               ne.face[0] = ne.face[1] = -1;
               ve[v] = &ne;
            }
            else {
               S32 ei = mVertexList[edge.vertex[v]].side - sideCount;
               ve[v] = &out->mEdgeList[ei];
            }

            // Edge should share this face
            if (ve[v]->face[0] == -1)
               ve[v]->face[0] = fi;
            else
               ve[v]->face[1] = fi;
         }

         // Build parallel edges
         out->mEdgeList.increment(2);
         for (S32 i = 0; i < 2; i++ ) {
            Edge& ne = out->mEdgeList.end()[i - 2];
            ne.next = out->mEdgeList.size() - 1 + i;
            ne.vertex[0] = ve[0]->vertex[i];
            ne.vertex[1] = ve[1]->vertex[i];
            ne.face[0] = i;
            ne.face[1] = fi;
         }
      }
   }

   out->mEdgeList.last().next = -1;
   out->mVolumeList.increment();
   Volume& nv = out->mVolumeList.last();
   nv.edgeList = 0;
   nv.object = 0;
   nv.material = -1;
}


//----------------------------------------------------------------------------

bool Polytope::findCollision(const VectorF& vec,Polytope::Collision *best)
{
   if (mVolumeList.size() <= 1)
      return false;
   if (!best->object)
      best->distance = 1.0E30f;
   int bestVertex = -1;
   Polytope::Volume* bestVolume = NULL;
   sideCount++;

   // Find the closest point
   for (Volume* vol = mVolumeList.begin() + 1;
         vol < mVolumeList.end(); vol++) {
      for (S32 e = vol->edgeList; e >= 0; e = mEdgeList[e].next) {
         Edge& edge = mEdgeList[e];
         if (mFaceList[edge.face[0]].original &&
               mFaceList[edge.face[1]].original)
            continue;
         for (S32 v = 0; v < 2; v++) {
            S32 vi = edge.vertex[v];
            Vertex& vr = mVertexList[vi];
            if (vr.side != sideCount) {
               vr.side = sideCount;
               F32 dist = mDot(vr.point,vec);
               if (dist < best->distance) {
                  best->distance = dist;
                  bestVertex = vi;
                  bestVolume = vol;
               }
            }
         }
      }
   }

   if (bestVertex == -1)
      return false;

   // Fill in the return value
   best->point = mVertexList[bestVertex].point;
   best->object = bestVolume->object;
   best->material = bestVolume->material;

   // Pick the best face
   F32 bestFaceDot = 1;
   for (S32 e = bestVolume->edgeList; e >= 0; e = mEdgeList[e].next) {
      Edge& edge = mEdgeList[e];
      if (edge.vertex[0] == bestVertex || edge.vertex[1] == bestVertex) {
         for (S32 f = 0; f < 2; f++) {
            Face& tf = mFaceList[edge.face[f]];
            F32 fd = mDot(tf.plane,vec);
            if (fd < bestFaceDot) {
               bestFaceDot = fd;
               best->plane = tf.plane;
            }
         }
      }
   }
   return true;
}

