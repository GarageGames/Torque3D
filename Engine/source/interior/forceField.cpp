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
#include "interior/forceField.h"

#include "core/stream/stream.h"
#include "math/mathIO.h"
#include "console/console.h"
#include "collision/abstractPolyList.h"
#include "scene/sceneObject.h"


ForceField::ForceField()
{
   VECTOR_SET_ASSOCIATION( mTriggers );
   VECTOR_SET_ASSOCIATION( mPlanes );
   VECTOR_SET_ASSOCIATION( mPoints );
   VECTOR_SET_ASSOCIATION( mBSPNodes );
   VECTOR_SET_ASSOCIATION( mBSPSolidLeaves );
   VECTOR_SET_ASSOCIATION( mSolidLeafSurfaces );
   VECTOR_SET_ASSOCIATION( mWindings );
   VECTOR_SET_ASSOCIATION( mSurfaces );

   mPreppedForRender = false;
}

ForceField::~ForceField()
{
   mPreppedForRender = false;
}


bool ForceField::prepForRendering()
{
   if (mPreppedForRender == true)
      return true;

   mPreppedForRender = true;
   return true;
}


void ForceField::render(const ColorF& rColor, const F32 fade)
{
}


//--------------------------------------------------------------------------
//-------------------------------------- Persistence interfaces
//
const U32 ForceField::smFileVersion = 0;

bool ForceField::read(Stream& stream)
{
   AssertFatal(stream.hasCapability(Stream::StreamRead), "ForceField::read: non-read capable stream passed");
   AssertFatal(stream.getStatus() == Stream::Ok, "ForceField::read: Error, stream in inconsistent state");

   U32 i;

   // Version this stream
   U32 fileVersion;
   stream.read(&fileVersion);
   if (fileVersion != smFileVersion) {
      Con::errorf(ConsoleLogEntry::General, "ForceField::read: incompatible file version found.");
      return false;
   }

   mName = stream.readSTString();
   U32 numTriggers;
   stream.read(&numTriggers);
   mTriggers.setSize(numTriggers);
   for (i = 0; i < mTriggers.size(); i++)
      mTriggers[i] = stream.readSTString();

   // Geometry factors...
   mathRead(stream, &mBoundingBox);
   mathRead(stream, &mBoundingSphere);

   // Now read in our data vectors.
   U32 vectorSize;
   // mPlanes
   readPlaneVector(stream);

   // mPoints
   stream.read(&vectorSize);
   mPoints.setSize(vectorSize);
   for (i = 0; i < mPoints.size(); i++)
      mathRead(stream, &mPoints[i]);

   // mBSPNodes;
   stream.read(&vectorSize);
   mBSPNodes.setSize(vectorSize);
   for (i = 0; i < mBSPNodes.size(); i++) {
      stream.read(&mBSPNodes[i].planeIndex);
      stream.read(&mBSPNodes[i].frontIndex);
      stream.read(&mBSPNodes[i].backIndex);
   }

   // mBSPSolidLeaves
   stream.read(&vectorSize);
   mBSPSolidLeaves.setSize(vectorSize);
   for (i = 0; i < mBSPSolidLeaves.size(); i++) {
      stream.read(&mBSPSolidLeaves[i].surfaceIndex);
      stream.read(&mBSPSolidLeaves[i].surfaceCount);
   }

   // mWindings
   stream.read(&vectorSize);
   mWindings.setSize(vectorSize);
   for (i = 0; i < mWindings.size(); i++) {
      stream.read(&mWindings[i]);
   }

   // mSurfaces
   stream.read(&vectorSize);
   mSurfaces.setSize(vectorSize);
   for (i = 0; i < mSurfaces.size(); i++) {
      stream.read(&mSurfaces[i].windingStart);
      stream.read(&mSurfaces[i].windingCount);
      stream.read(&mSurfaces[i].planeIndex);
      stream.read(&mSurfaces[i].surfaceFlags);
      stream.read(&mSurfaces[i].fanMask);
   }

   // mSolidLeafSurfaces
   stream.read(&vectorSize);
   mSolidLeafSurfaces.setSize(vectorSize);
   for (i = 0; i < mSolidLeafSurfaces.size(); i++) {
      stream.read(&mSolidLeafSurfaces[i]);
   }

   stream.read(&mColor);

   return stream.getStatus() == Stream::Ok;
}

bool ForceField::write(Stream& stream) const
{
   AssertFatal(stream.hasCapability(Stream::StreamWrite), "Interior::write: non-write capable stream passed");
   AssertFatal(stream.getStatus() == Stream::Ok, "Interior::write: Error, stream in inconsistent state");

   U32 i;

   // Version this stream
   stream.write(smFileVersion);

   stream.writeString(mName);
   stream.write(mTriggers.size());
   for (i = 0; i < mTriggers.size(); i++)
      stream.writeString(mTriggers[i]);

   mathWrite(stream, mBoundingBox);
   mathWrite(stream, mBoundingSphere);

   // Now write out our data vectors.  Remember, for cross-platform capability, no
   //  structure writing is allowed...

   // mPlanes
   writePlaneVector(stream);

   // mPoints
   stream.write(mPoints.size());
   for (i = 0; i < mPoints.size(); i++)
      mathWrite(stream, mPoints[i]);

   // mBSPNodes;
   stream.write(mBSPNodes.size());
   for (i = 0; i < mBSPNodes.size(); i++) {
      stream.write(mBSPNodes[i].planeIndex);
      stream.write(mBSPNodes[i].frontIndex);
      stream.write(mBSPNodes[i].backIndex);
   }

   // mBSPSolidLeaves
   stream.write(mBSPSolidLeaves.size());
   for (i = 0; i < mBSPSolidLeaves.size(); i++) {
      stream.write(mBSPSolidLeaves[i].surfaceIndex);
      stream.write(mBSPSolidLeaves[i].surfaceCount);
   }

   // mWindings
   stream.write(mWindings.size());
   for (i = 0; i < mWindings.size(); i++) {
      stream.write(mWindings[i]);
   }

   // mSurfaces
   stream.write(mSurfaces.size());
   for (i = 0; i < mSurfaces.size(); i++) {
      stream.write(mSurfaces[i].windingStart);
      stream.write(mSurfaces[i].windingCount);
      stream.write(mSurfaces[i].planeIndex);
      stream.write(mSurfaces[i].surfaceFlags);
      stream.write(mSurfaces[i].fanMask);
   }

   // mSolidLeafSurfaces
   stream.write(mSolidLeafSurfaces.size());
   for (i = 0; i < mSolidLeafSurfaces.size(); i++) {
      stream.write(mSolidLeafSurfaces[i]);
   }

   stream.write(mColor);

   return stream.getStatus() == Stream::Ok;
}

bool ForceField::writePlaneVector(Stream& stream) const
{
   // This is pretty slow, but who cares?
   //
   Vector<Point3F> uniqueNormals(mPlanes.size());
   Vector<U16>  uniqueIndices(mPlanes.size());

   U32 i;

   for (i = 0; i < mPlanes.size(); i++) {
      bool inserted = false;
      for (U32 j = 0; j < uniqueNormals.size(); j++) {
         if (mPlanes[i] == uniqueNormals[j]) {
            // Hah!  Already have this one...
            uniqueIndices.push_back(j);
            inserted = true;
            break;
         }
      }

      if (inserted == false) {
         // Gotta do it ourselves...
         uniqueIndices.push_back(uniqueNormals.size());
         uniqueNormals.push_back(Point3F(mPlanes[i].x, mPlanes[i].y, mPlanes[i].z));
      }
   }

   // Ok, what we have now, is a list of unique normals, a set of indices into
   //  that vector, and the distances that we still have to write out by hand.
   //  Hop to it!
   stream.write(uniqueNormals.size());
   for (i = 0; i < uniqueNormals.size(); i++)
      mathWrite(stream, uniqueNormals[i]);

   stream.write(mPlanes.size());
   for (i = 0; i < mPlanes.size(); i++) {
      stream.write(uniqueIndices[i]);
      stream.write(mPlanes[i].d);
   }

   return (stream.getStatus() == Stream::Ok);
}

bool ForceField::readPlaneVector(Stream& stream)
{
   Vector<Point3F> normals;
   U32 vectorSize;

   stream.read(&vectorSize);
   normals.setSize(vectorSize);
   U32 i;
   for (i = 0; i < normals.size(); i++)
      mathRead(stream, &normals[i]);

   U16 index;
   stream.read(&vectorSize);
   mPlanes.setSize(vectorSize);
   for (i = 0; i < mPlanes.size(); i++) {
      stream.read(&index);
      stream.read(&mPlanes[i].d);
      mPlanes[i].x = normals[index].x;
      mPlanes[i].y = normals[index].y;
      mPlanes[i].z = normals[index].z;
   }

   return (stream.getStatus() == Stream::Ok);
}


//--------------------------------------------------------------------------
//-------------------------------------- Collision support.  Essentially
//                                        copied from the interiorCollision
//
void ForceField::collisionFanFromSurface(const Surface& rSurface, U32* fanIndices, U32* numIndices) const
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
   for (i = 0; i < rSurface.windingCount; i++) {
      if (rSurface.fanMask & (1 << i)) {
         fanIndices[idx++] = mWindings[rSurface.windingStart + tempIndices[i]];
      }
   }
   *numIndices = idx;
}

bool ForceField::castRay(const Point3F& s, const Point3F& e, RayInfo* info)
{
   bool hit = castRay_r(0, s, e, info);
   if (hit) {
      Point3F vec = e - s;
      F32 len = vec.len();
      vec /= len;
      info->t = mDot(info->point - s, vec) / len;
   }

   return hit;
}

bool ForceField::castRay_r(const U16      node,
                         const Point3F& s,
                         const Point3F& e,
                         RayInfo*       info)
{
   if (isBSPLeafIndex(node) == false) {
      const IBSPNode& rNode = mBSPNodes[node];
      const PlaneF& rPlane  = getPlane(rNode.planeIndex);

      PlaneF::Side sSide = rPlane.whichSide(s);
      PlaneF::Side eSide = rPlane.whichSide(e);

      switch (PlaneSwitchCode(sSide, eSide)) {
        case PlaneSwitchCode(PlaneF::Front, PlaneF::Front):
        case PlaneSwitchCode(PlaneF::Front, PlaneF::On):
        case PlaneSwitchCode(PlaneF::On,    PlaneF::Front):
         return castRay_r(rNode.frontIndex, s, e, info);
         break;

        case PlaneSwitchCode(PlaneF::On,   PlaneF::Back):
        case PlaneSwitchCode(PlaneF::Back, PlaneF::On):
        case PlaneSwitchCode(PlaneF::Back, PlaneF::Back):
         return castRay_r(rNode.backIndex, s, e, info);
         break;

        case PlaneSwitchCode(PlaneF::On, PlaneF::On):
         // Line lies on the plane
         if (isBSPLeafIndex(rNode.backIndex) == false) {
            if (castRay_r(rNode.backIndex, s, e, info))
               return true;
         }
         if (isBSPLeafIndex(rNode.frontIndex) == false) {
            if (castRay_r(rNode.frontIndex, s, e, info))
               return true;
         }
         return false;
         break;

        case PlaneSwitchCode(PlaneF::Front, PlaneF::Back): {
            Point3F ip;
            F32     intersectT = rPlane.intersect(s, e);
            AssertFatal(intersectT != PARALLEL_PLANE, "Error, this should never happen in this case!");
            ip.interpolate(s, e, intersectT);
            if (castRay_r(rNode.frontIndex, s, ip, info))
               return true;
            return castRay_r(rNode.backIndex, ip, e, info);
         }
         break;

        case PlaneSwitchCode(PlaneF::Back, PlaneF::Front): {
            Point3F ip;
            F32     intersectT = rPlane.intersect(s, e);
            AssertFatal(intersectT != PARALLEL_PLANE, "Error, this should never happen in this case!");
            ip.interpolate(s, e, intersectT);
            if (castRay_r(rNode.backIndex, s, ip, info))
               return true;
            return castRay_r(rNode.frontIndex, ip, e, info);
         }
         break;

        default:
         AssertFatal(false, "Misunderstood switchCode in ForceField::castRay_r");
         return false;
      }
   }

   if (isBSPSolidLeaf(node)) {
      // DMM: Set material info here
      info->point = s;
      return true;
   }
   return false;
}

void ForceField::buildPolyList_r(const U16 node, Vector<U16>& collPlanes, AbstractPolyList* list, SphereF& s)
{
   if (isBSPLeafIndex(node) == false) {
      const IBSPNode& rNode = mBSPNodes[node];
      const PlaneF& rPlane  = getPlane(rNode.planeIndex);

      F32 dist = rPlane.distToPlane(s.center);
      if (mFabs(dist) <= s.radius) {
         // Have to do both, and push the plane back on the list...
         collPlanes.push_back(rNode.planeIndex);
         buildPolyList_r(rNode.frontIndex, collPlanes, list, s);
         buildPolyList_r(rNode.backIndex, collPlanes, list, s);
         collPlanes.pop_back();
      } else if (dist > 0.0f) {
         buildPolyList_r(rNode.frontIndex, collPlanes, list, s);
      } else {
         buildPolyList_r(rNode.backIndex, collPlanes, list, s);
      }
      return;
   }

   if (isBSPSolidLeaf(node)) {
      const IBSPLeafSolid& rLeaf = mBSPSolidLeaves[getBSPSolidLeafIndex(node)];
      for (U32 i = 0; i < rLeaf.surfaceCount; i++) {
         U32 surfaceIndex = mSolidLeafSurfaces[rLeaf.surfaceIndex + i];
         const Surface& rSurface = mSurfaces[surfaceIndex];
         for (U32 j = 0; j < collPlanes.size(); j++) {
            if (areEqualPlanes(rSurface.planeIndex, collPlanes[j]) == true) {

               U32 fanVerts[32];
               U32 numVerts;
               collisionFanFromSurface(rSurface, fanVerts, &numVerts);

               // DMM: Material here
               list->begin(0, rSurface.planeIndex);

               U32 vertStart = list->addPoint(mPoints[fanVerts[0]]);
               list->vertex(vertStart);
               for (U32 k = 1; k < numVerts; k++) {
                  list->addPoint(mPoints[fanVerts[k]]);
                  list->vertex(vertStart + k);
               }
               list->plane(vertStart, vertStart + 1, vertStart + 2);
               list->end();

               break;
            }
         }

      }
   }
}

bool ForceField::buildPolyList(AbstractPolyList* list, SphereF& sphere)
{
   Vector<U16> planes;
   buildPolyList_r(0, planes, list, sphere);
   AssertFatal(planes.size() == 0, "Error, unbalanced plane stack!");

   return !list->isEmpty();
}
