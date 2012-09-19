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

#include "scene/sgUtil.h"
#include "math/mRect.h"
#include "math/mMatrix.h"
#include "math/mPlane.h"

namespace {

// Static state for sgComputeNewFrustum
//
Point3F sgCamPoint;
MatrixF sgWSToOSMatrix;
MatrixF sgProjMatrix;
PlaneF  sgOSPlaneFar;
PlaneF  sgOSPlaneXMin;
PlaneF  sgOSPlaneXMax;
PlaneF  sgOSPlaneYMin;
PlaneF  sgOSPlaneYMax;


void clipToPlane(Point3F* points, U32& rNumPoints, const PlaneF& rPlane)
{
   S32 start = -1;
   for (U32 i = 0; i < rNumPoints; i++) {
      if (rPlane.whichSide(points[i]) == PlaneF::Front) {
         start = i;
         break;
      }
   }

   // Nothing was in front of the plane...
   if (start == -1) {
      rNumPoints = 0;
      return;
   }

   Point3F finalPoints[128];
   U32  numFinalPoints = 0;

   U32 baseStart = start;
   U32 end       = (start + 1) % rNumPoints;

   while (end != baseStart) {
      const Point3F& rStartPoint = points[start];
      const Point3F& rEndPoint   = points[end];

      PlaneF::Side fSide = rPlane.whichSide(rStartPoint);
      PlaneF::Side eSide = rPlane.whichSide(rEndPoint);

      S32 code = fSide * 3 + eSide;
      switch (code) {
        case 4:   // f f
        case 3:   // f o
        case 1:   // o f
        case 0:   // o o
         // No Clipping required
         finalPoints[numFinalPoints++] = points[start];
         start = end;
         end   = (end + 1) % rNumPoints;
         break;


        case 2: { // f b
            // In this case, we emit the front point, Insert the intersection,
            //  and advancing to point to first point that is in front or on...
            //
            finalPoints[numFinalPoints++] = points[start];

            Point3F vector = rEndPoint - rStartPoint;
            F32 t        = -(rPlane.distToPlane(rStartPoint) / mDot(rPlane, vector));

            Point3F intersection = rStartPoint + (vector * t);
            finalPoints[numFinalPoints++] = intersection;

            U32 endSeek = (end + 1) % rNumPoints;
            while (rPlane.whichSide(points[endSeek]) == PlaneF::Back)
               endSeek = (endSeek + 1) % rNumPoints;

            end   = endSeek;
            start = (end + (rNumPoints - 1)) % rNumPoints;

            const Point3F& rNewStartPoint = points[start];
            const Point3F& rNewEndPoint   = points[end];

            vector = rNewEndPoint - rNewStartPoint;
            t = -(rPlane.distToPlane(rNewStartPoint) / mDot(rPlane, vector));

            intersection = rNewStartPoint + (vector * t);
            points[start] = intersection;
         }
         break;

        case -1: {// o b
            // In this case, we emit the front point, and advance to point to first
            //  point that is in front or on...
            //
            finalPoints[numFinalPoints++] = points[start];

            U32 endSeek = (end + 1) % rNumPoints;
            while (rPlane.whichSide(points[endSeek]) == PlaneF::Back)
               endSeek = (endSeek + 1) % rNumPoints;

            end   = endSeek;
            start = (end + (rNumPoints - 1)) % rNumPoints;

            const Point3F& rNewStartPoint = points[start];
            const Point3F& rNewEndPoint   = points[end];

            Point3F vector = rNewEndPoint - rNewStartPoint;
            F32 t        = -(rPlane.distToPlane(rNewStartPoint) / mDot(rPlane, vector));

            Point3F intersection = rNewStartPoint + (vector * t);
            points[start] = intersection;
         }
         break;

        case -2:  // b f
        case -3:  // b o
        case -4:  // b b
         // In the algorithm used here, this should never happen...
         AssertISV(false, "SGUtil::clipToPlane: error in polygon clipper");
         break;

        default:
         AssertFatal(false, "SGUtil::clipToPlane: bad outcode");
         break;
      }

   }

   // Emit the last point.
   finalPoints[numFinalPoints++] = points[start];
   AssertFatal(numFinalPoints >= 3, avar("Error, this shouldn't happen!  Invalid winding in clipToPlane: %d", numFinalPoints));

   // Copy the new rWinding, and we're set!
   //
   dMemcpy(points, finalPoints, numFinalPoints * sizeof(Point3F));
   rNumPoints = numFinalPoints;
   AssertISV(rNumPoints <= 128, "MaxWindingPoints exceeded in scenegraph.  Fatal error.");
}


void fixupViewport(const F64*   oldFrustum,
                   const RectI& oldViewport,
                   F64*         newFrustum,
                   RectI&       newViewport)
{
   F64 widthV  = newFrustum[1] - newFrustum[0];
   F64 heightV = newFrustum[3] - newFrustum[2];

   F64 fx0 = (newFrustum[0] - oldFrustum[0]) / (oldFrustum[1] - oldFrustum[0]);
   F64 fx1 = (oldFrustum[1] - newFrustum[1]) / (oldFrustum[1] - oldFrustum[0]);

   F64 dV0 = F64(oldViewport.point.x) + fx0 * F64(oldViewport.extent.x);
   F64 dV1 = F64(oldViewport.point.x +
                 oldViewport.extent.x) - fx1 * F64(oldViewport.extent.x);

   F64 fdV0 = mFloor(dV0);
   F64 cdV1 = mCeil(dV1);

   F64 new0 = newFrustum[0] - ((dV0 - fdV0) * (widthV / F64(oldViewport.extent.x)));
   F64 new1 = newFrustum[1] + ((cdV1 - dV1) * (widthV / F64(oldViewport.extent.x)));

   newFrustum[0] = new0;
   newFrustum[1] = new1;

   newViewport.point.x  = S32(fdV0);
   newViewport.extent.x = S32(cdV1) - newViewport.point.x;

   F64 fy0 = (oldFrustum[3] - newFrustum[3]) / (oldFrustum[3] - oldFrustum[2]);
   F64 fy1 = (newFrustum[2] - oldFrustum[2]) / (oldFrustum[3] - oldFrustum[2]);

   dV0 = F64(oldViewport.point.y) + fy0 * F64(oldViewport.extent.y);
   dV1 = F64(oldViewport.point.y + oldViewport.extent.y) - fy1 * F64(oldViewport.extent.y);
   fdV0 = mFloor(dV0);
   cdV1 = mCeil(dV1);

   new0 = newFrustum[2] - ((cdV1 - dV1) * (heightV / F64(oldViewport.extent.y)));
   new1 = newFrustum[3] + ((dV0 - fdV0) * (heightV / F64(oldViewport.extent.y)));
   newFrustum[2] = new0;
   newFrustum[3] = new1;

   newViewport.point.y  = S32(fdV0);
   newViewport.extent.y = S32(cdV1) - newViewport.point.y;
}

bool projectClipAndBoundWinding(const SGWinding& rWinding, F64* pResult)
{
   AssertFatal(rWinding.numPoints >= 3, "Error, that's not a winding!");

   static Point3F windingPoints[128];
   U32 i;
   for (i = 0; i < rWinding.numPoints; i++)
      windingPoints[i] = rWinding.points[i];
   U32 numPoints = rWinding.numPoints;

   clipToPlane(windingPoints, numPoints, sgOSPlaneFar);
   if (numPoints != 0)
      clipToPlane(windingPoints, numPoints, sgOSPlaneXMin);
   if (numPoints != 0)
      clipToPlane(windingPoints, numPoints, sgOSPlaneXMax);
   if (numPoints != 0)
      clipToPlane(windingPoints, numPoints, sgOSPlaneYMin);
   if (numPoints != 0)
      clipToPlane(windingPoints, numPoints, sgOSPlaneYMax);

   if (numPoints == 0)
      return false;

   Point4F projPoint;
   for (i = 0; i < numPoints; i++) {
      projPoint.set(windingPoints[i].x, windingPoints[i].y, windingPoints[i].z, 1.0);
      sgProjMatrix.mul(projPoint);

      AssertFatal(projPoint.w != 0.0, "Error, that's bad! (Point projected with non-zero w.)");
      projPoint.x /= projPoint.w;
      projPoint.y /= projPoint.w;

      if (projPoint.x < pResult[0])
         pResult[0] = projPoint.x;
      if (projPoint.x > pResult[1])
         pResult[1] = projPoint.x;
      if (projPoint.y < pResult[2])
         pResult[2] = projPoint.y;
      if (projPoint.y > pResult[3])
         pResult[3] = projPoint.y;
   }

   if (pResult[0] < -1.0f) pResult[0] = -1.0f;
   if (pResult[2] < -1.0f) pResult[2] = -1.0f;
   if (pResult[1] > 1.0f)  pResult[1] =  1.0f;
   if (pResult[3] > 1.0f)  pResult[3] =  1.0f;

   return true;
}

} // namespace { }


//--------------------------------------------------------------------------
bool sgComputeNewFrustum(const Frustum    &oldFrustum,
                         const F64        nearPlane,
                         const F64        farPlane,
                         const RectI&     oldViewport,
                         const SGWinding* windings,
                         const U32        numWindings,
                         const MatrixF&   modelview,
                         F64              *newFrustum,
                         RectI&           newViewport,
                         const bool       flippedMatrix)
{
   return false;
}


void sgComputeOSFrustumPlanes(const F64      frustumParameters[6],
                              const MatrixF& worldSpaceToObjectSpace,
                              const Point3F& wsCamPoint,
                              PlaneF&        outFarPlane,
                              PlaneF&        outXMinPlane,
                              PlaneF&        outXMaxPlane,
                              PlaneF&        outYMinPlane,
                              PlaneF&        outYMaxPlane)
{
   // Create the object space clipping planes...
   Point3F ul(frustumParameters[0] * 1000.0, frustumParameters[4] * 1000.0, frustumParameters[3] * 1000.0);
   Point3F ur(frustumParameters[1] * 1000.0, frustumParameters[4] * 1000.0, frustumParameters[3] * 1000.0);
   Point3F ll(frustumParameters[0] * 1000.0, frustumParameters[4] * 1000.0, frustumParameters[2] * 1000.0);
   Point3F lr(frustumParameters[1] * 1000.0, frustumParameters[4] * 1000.0, frustumParameters[2] * 1000.0);
   Point3F farPlane(0, frustumParameters[5], 0);

   worldSpaceToObjectSpace.mulP(ul);
   worldSpaceToObjectSpace.mulP(ur);
   worldSpaceToObjectSpace.mulP(ll);
   worldSpaceToObjectSpace.mulP(lr);
   worldSpaceToObjectSpace.mulP(farPlane);

   outFarPlane.set(farPlane, wsCamPoint - farPlane);
   outXMinPlane.set(wsCamPoint, ul, ll);
   outXMaxPlane.set(wsCamPoint, lr, ur);
   outYMinPlane.set(wsCamPoint, ur, ul);
   outYMaxPlane.set(wsCamPoint, ll, lr);
}

// MM/JF: Added for mirrorSubObject fix.
void sgOrientClipPlanes(
                        PlaneF * planes,
                        const Point3F & camPos,
                        const Point3F & leftUp,
                        const Point3F & leftDown,
                        const Point3F & rightUp,
                        const Point3F & rightDown)
{
	AssertFatal(planes, "orientClipPlanes: NULL planes ptr");
	planes[0].set(camPos,      leftUp,     leftDown);
	planes[1].set(camPos,      rightUp,    leftUp);
	planes[2].set(camPos,      rightDown,  rightUp);
	planes[3].set(camPos,      leftDown,   rightDown);
	planes[4].set(leftUp,      rightUp,    rightDown);

	// clip-planes through mirror portal are inverted
	PlaneF plane(leftUp, rightUp, rightDown);
	if(plane.whichSide(camPos) == PlaneF::Back)
		for(U32 i = 0; i < 5; i++)
			planes[i].invert();
}
