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
#include "interior/interior.h"

#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "gfx/bitmap/gBitmap.h"
#include "math/mMatrix.h"
#include "math/mRect.h"
#include "core/bitVector.h"
#include "core/frameAllocator.h"
#include "scene/sgUtil.h"
#include "platform/profiler.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxTextureHandle.h"
#include "materials/materialList.h"
#include "materials/matInstance.h"
#include "materials/materialManager.h"
#include "renderInstance/renderPassManager.h"
#include "materials/processedMaterial.h"
#include "materials/materialFeatureTypes.h"

U32  Interior::smRenderMode            = 0;
bool Interior::smFocusedDebug          = false;
bool Interior::smUseVertexLighting     = false;
bool Interior::smLightingCastRays      = false;
bool Interior::smLightingBuildPolyList = false;

// These are setup by setupActivePolyList
U16*            sgActivePolyList      = NULL;
U32             sgActivePolyListSize  = 0;
U16*            sgEnvironPolyList     = NULL;
U32             sgEnvironPolyListSize = 0;
U16*            sgFogPolyList         = NULL;
U32             sgFogPolyListSize     = 0;
bool            sgFogActive           = false;

// Always the same size as the mPoints array
Point2F*        sgFogTexCoords        = NULL;

class PlaneRange
{
public:
   U32 start;
   U32 count;
};

namespace {

struct PortalRenderInfo
{
   bool  render;

   F64   frustum[4];
   RectI viewport;
};

//-------------------------------------- Rendering state variables.
Point3F sgCamPoint;
F64     sgStoredFrustum[6];
RectI   sgStoredViewport;

Vector<PortalRenderInfo> sgZoneRenderInfo(__FILE__, __LINE__);

// Takes OS coords to clip space...
MatrixF sgWSToOSMatrix;
MatrixF sgProjMatrix;

PlaneF  sgOSPlaneFar;
PlaneF  sgOSPlaneXMin;
PlaneF  sgOSPlaneXMax;
PlaneF  sgOSPlaneYMin;
PlaneF  sgOSPlaneYMax;

struct ZoneRect {
   RectD rect;
   bool  active;
};

Vector<ZoneRect> sgZoneRects(__FILE__, __LINE__);

//-------------------------------------- Little utility functions
RectD outlineRects(const Vector<RectD>& rects)
{
   F64 minx =  1e10;
   F64 maxx = -1e10;
   F64 miny =  1e10;
   F64 maxy = -1e10;

   for (S32 i = 0; i < rects.size(); i++) 
   {
      if (rects[i].point.x < minx)
         minx = rects[i].point.x;
      if (rects[i].point.y < miny)
         miny = rects[i].point.y;

      if (rects[i].point.x + rects[i].extent.x > maxx)
         maxx = rects[i].point.x + rects[i].extent.x;
      if (rects[i].point.y + rects[i].extent.y > maxy)
         maxy = rects[i].point.y + rects[i].extent.y;
   }

   return RectD(minx, miny, maxx - minx, maxy - miny);
}

void insertZoneRects(ZoneRect& rZoneRect, const RectD* rects, const U32 numRects)
{
   F64 minx =  1e10;
   F64 maxx = -1e10;
   F64 miny =  1e10;
   F64 maxy = -1e10;

   for (U32 i = 0; i < numRects; i++) {
      if (rects[i].point.x < minx)
         minx = rects[i].point.x;
      if (rects[i].point.y < miny)
         miny = rects[i].point.y;

      if (rects[i].point.x + rects[i].extent.x > maxx)
         maxx = rects[i].point.x + rects[i].extent.x;
      if (rects[i].point.y + rects[i].extent.y > maxy)
         maxy = rects[i].point.y + rects[i].extent.y;
   }

   if (rZoneRect.active == false && numRects != 0) {
      rZoneRect.rect = RectD(minx, miny, maxx - minx, maxy - miny);
      rZoneRect.active = true;
   } else {
      if (rZoneRect.rect.point.x < minx)
         minx = rZoneRect.rect.point.x;
      if (rZoneRect.rect.point.y < miny)
         miny = rZoneRect.rect.point.y;

      if (rZoneRect.rect.point.x + rZoneRect.rect.extent.x > maxx)
         maxx = rZoneRect.rect.point.x + rZoneRect.rect.extent.x;
      if (rZoneRect.rect.point.y + rZoneRect.rect.extent.y > maxy)
         maxy = rZoneRect.rect.point.y + rZoneRect.rect.extent.y;

      rZoneRect.rect = RectD(minx, miny, maxx - minx, maxy - miny);
   }
}



void fixupViewport(PortalRenderInfo& rInfo)
{
   F64 widthV  = rInfo.frustum[1] - rInfo.frustum[0];
   F64 heightV = rInfo.frustum[3] - rInfo.frustum[2];

   F64 fx0 = (rInfo.frustum[0] - sgStoredFrustum[0]) / (sgStoredFrustum[1] - sgStoredFrustum[0]);
   F64 fx1 = (sgStoredFrustum[1] - rInfo.frustum[1]) / (sgStoredFrustum[1] - sgStoredFrustum[0]);

   F64 dV0 = F64(sgStoredViewport.point.x) + fx0 * F64(sgStoredViewport.extent.x);
   F64 dV1 = F64(sgStoredViewport.point.x +
                 sgStoredViewport.extent.x) - fx1 * F64(sgStoredViewport.extent.x);

   F64 fdV0 = getMax(mFloorD(dV0), F64(sgStoredViewport.point.x));
   F64 cdV1 = getMin(mCeilD(dV1), F64(sgStoredViewport.point.x + sgStoredViewport.extent.x));

   // If the width is 1 pixel, we need to widen it up a bit...
   if ((cdV1 - fdV0) <= 1.0)
      cdV1 = fdV0 + 1;
   AssertFatal((fdV0 >= sgStoredViewport.point.x &&
                cdV1 <= sgStoredViewport.point.x + sgStoredViewport.extent.x),
               "Out of bounds viewport bounds");

   F64 new0 = rInfo.frustum[0] - ((dV0 - fdV0) * (widthV / F64(sgStoredViewport.extent.x)));
   F64 new1 = rInfo.frustum[1] + ((cdV1 - dV1) * (widthV / F64(sgStoredViewport.extent.x)));

   rInfo.frustum[0] = new0;
   rInfo.frustum[1] = new1;

   rInfo.viewport.point.x  = S32(fdV0);
   rInfo.viewport.extent.x = S32(cdV1) - rInfo.viewport.point.x;

   F64 fy0 = (sgStoredFrustum[3] - rInfo.frustum[3]) / (sgStoredFrustum[3] - sgStoredFrustum[2]);
   F64 fy1 = (rInfo.frustum[2] - sgStoredFrustum[2]) / (sgStoredFrustum[3] - sgStoredFrustum[2]);

   dV0 = F64(sgStoredViewport.point.y) + fy0 * F64(sgStoredViewport.extent.y);
   dV1 = F64(sgStoredViewport.point.y +
             sgStoredViewport.extent.y) - fy1 * F64(sgStoredViewport.extent.y);
   fdV0 = getMax(mFloorD(dV0), F64(sgStoredViewport.point.y));
   cdV1 = getMin(mCeilD(dV1), F64(sgStoredViewport.point.y + sgStoredViewport.extent.y));

   // If the width is 1 pixel, we need to widen it up a bit...
   if ((cdV1 - fdV0) <= 1.0)
      cdV1 = fdV0 + 1;
   // GFX2_RENDER_MERGE
   // Need to fix this properly but for now *HACK*
#ifndef TORQUE_OS_MAC
   AssertFatal((fdV0 >= sgStoredViewport.point.y &&
                cdV1 <= sgStoredViewport.point.y + sgStoredViewport.extent.y),
               "Out of bounds viewport bounds");
#endif

   new0 = rInfo.frustum[2] - ((cdV1 - dV1) * (heightV / F64(sgStoredViewport.extent.y)));
   new1 = rInfo.frustum[3] + ((dV0 - fdV0) * (heightV / F64(sgStoredViewport.extent.y)));
   rInfo.frustum[2] = new0;
   rInfo.frustum[3] = new1;

   rInfo.viewport.point.y  = S32(fdV0);
   rInfo.viewport.extent.y = S32(cdV1) - rInfo.viewport.point.y;
}

RectD convertToRectD(const F64 inResult[4])
{
   F64 minx = ((inResult[0] + 1.0f) / 2.0f) * (sgStoredFrustum[1] - sgStoredFrustum[0]) + sgStoredFrustum[0];
   F64 maxx = ((inResult[2] + 1.0f) / 2.0f) * (sgStoredFrustum[1] - sgStoredFrustum[0]) + sgStoredFrustum[0];

   F64 miny = ((inResult[1] + 1.0f) / 2.0f) * (sgStoredFrustum[3] - sgStoredFrustum[2]) + sgStoredFrustum[2];
   F64 maxy = ((inResult[3] + 1.0f) / 2.0f) * (sgStoredFrustum[3] - sgStoredFrustum[2]) + sgStoredFrustum[2];

   return RectD(minx, miny, (maxx - minx), (maxy - miny));
}

void convertToFrustum(PortalRenderInfo& zrInfo, const RectD& finalRect)
{
   zrInfo.frustum[0] = finalRect.point.x;                      // left
   zrInfo.frustum[1] = finalRect.point.x + finalRect.extent.x; // right
   zrInfo.frustum[2] = finalRect.point.y;                      // bottom
   zrInfo.frustum[3] = finalRect.point.y + finalRect.extent.y; // top

   fixupViewport(zrInfo);
}

} // namespace {}


//------------------------------------------------------------------------------
//-------------------------------------- IMPLEMENTATION
//
Interior::Interior()
{
   mMaterialList  = NULL;

   mHasTranslucentMaterials = false;

   mLMHandle = LM_HANDLE(-1);

   // By default, no alarm state, no animated light states
   mHasAlarmState = false;

   mNumLightStateEntries = 0;
   mNumTriggerableLights = 0;

   mPreppedForRender = false;;
   mSearchTag = 0;

   mLightMapBorderSize = 0;

#ifndef TORQUE_SHIPPING
   mDebugShader = NULL;

   mDebugShaderModelViewSC = NULL;
   mDebugShaderShadeColorSC = NULL;
#endif

   // Bind our vectors
   VECTOR_SET_ASSOCIATION(mPlanes);
   VECTOR_SET_ASSOCIATION(mPoints);
   VECTOR_SET_ASSOCIATION(mBSPNodes);
   VECTOR_SET_ASSOCIATION(mBSPSolidLeaves);
   VECTOR_SET_ASSOCIATION(mWindings);
   VECTOR_SET_ASSOCIATION(mTexGenEQs);
   VECTOR_SET_ASSOCIATION(mLMTexGenEQs);
   VECTOR_SET_ASSOCIATION(mWindingIndices);
   VECTOR_SET_ASSOCIATION(mSurfaces);
   VECTOR_SET_ASSOCIATION(mNullSurfaces);
   VECTOR_SET_ASSOCIATION(mSolidLeafSurfaces);
   VECTOR_SET_ASSOCIATION(mZones);
   VECTOR_SET_ASSOCIATION(mZonePlanes);
   VECTOR_SET_ASSOCIATION(mZoneSurfaces);
   VECTOR_SET_ASSOCIATION(mZonePortalList);
   VECTOR_SET_ASSOCIATION(mPortals);
   //VECTOR_SET_ASSOCIATION(mSubObjects);
   VECTOR_SET_ASSOCIATION(mLightmaps);
   VECTOR_SET_ASSOCIATION(mLightmapKeep);
   VECTOR_SET_ASSOCIATION(mNormalLMapIndices);
   VECTOR_SET_ASSOCIATION(mAlarmLMapIndices);
   VECTOR_SET_ASSOCIATION(mAnimatedLights);
   VECTOR_SET_ASSOCIATION(mLightStates);
   VECTOR_SET_ASSOCIATION(mStateData);
   VECTOR_SET_ASSOCIATION(mStateDataBuffer);
   VECTOR_SET_ASSOCIATION(mNameBuffer);
   VECTOR_SET_ASSOCIATION(mConvexHulls);
   VECTOR_SET_ASSOCIATION(mConvexHullEmitStrings);
   VECTOR_SET_ASSOCIATION(mHullIndices);
   VECTOR_SET_ASSOCIATION(mHullEmitStringIndices);
   VECTOR_SET_ASSOCIATION(mHullSurfaceIndices);
   VECTOR_SET_ASSOCIATION(mHullPlaneIndices);
   VECTOR_SET_ASSOCIATION(mPolyListPlanes);
   VECTOR_SET_ASSOCIATION(mPolyListPoints);
   VECTOR_SET_ASSOCIATION(mPolyListStrings);
   VECTOR_SET_ASSOCIATION(mCoordBinIndices);

   VECTOR_SET_ASSOCIATION(mVehicleConvexHulls);
   VECTOR_SET_ASSOCIATION(mVehicleConvexHullEmitStrings);
   VECTOR_SET_ASSOCIATION(mVehicleHullIndices);
   VECTOR_SET_ASSOCIATION(mVehicleHullEmitStringIndices);
   VECTOR_SET_ASSOCIATION(mVehicleHullSurfaceIndices);
   VECTOR_SET_ASSOCIATION(mVehicleHullPlaneIndices);
   VECTOR_SET_ASSOCIATION(mVehiclePolyListPlanes);
   VECTOR_SET_ASSOCIATION(mVehiclePolyListPoints);
   VECTOR_SET_ASSOCIATION(mVehiclePolyListStrings);
   VECTOR_SET_ASSOCIATION(mVehiclePoints);
   VECTOR_SET_ASSOCIATION(mVehicleNullSurfaces);
   VECTOR_SET_ASSOCIATION(mVehiclePlanes);
}

Interior::~Interior()
{
   U32 i;
   delete mMaterialList;
   mMaterialList = NULL;

   if(mLMHandle != LM_HANDLE(-1))
      gInteriorLMManager.removeInterior(mLMHandle);

   for(i = 0; i < mLightmaps.size(); i++)
   {
      delete mLightmaps[i];
      mLightmaps[i] = NULL;
   }

   for(i = 0; i < mMatInstCleanupList.size(); i++)
   {
      delete mMatInstCleanupList[i];
      mMatInstCleanupList[i] = NULL;
   }

   for(S32 i=0; i<mStaticMeshes.size(); i++)
      delete mStaticMeshes[i];
}


//--------------------------------------------------------------------------
bool Interior::prepForRendering(const char* path)
{
   if(mPreppedForRender == true)
      return true;

   // Before we load the material list we temporarily remove
   // some special texture names so that we don't get bogus
   // texture load warnings in the console.
   const Vector<String> &matNames = mMaterialList->getMaterialNameList();
   Vector<String> originalNames = matNames;

   for (U32 i = 0; i < matNames.size(); i++)
   {
      if (matNames[i].equal("NULL", String::NoCase) ||
          matNames[i].equal("ORIGIN", String::NoCase) ||
          matNames[i].equal("TRIGGER", String::NoCase) ||
          matNames[i].equal("FORCEFIELD", String::NoCase) ||
          matNames[i].equal("EMITTER", String::NoCase) )
      {
         mMaterialList->setMaterialName(i, String());
      }
   }

   String relPath = Platform::makeRelativePathName(path, Platform::getCurrentDirectory());

   // Load the material list
   mMaterialList->setTextureLookupPath(relPath);
   mMaterialList->mapMaterials();

   // Grab all the static meshes and load any textures that didn't originate
   // from inside the DIF.
   for(S32 i=0; i<mStaticMeshes.size(); i++)
      mStaticMeshes[i]->materialList->setTextureLookupPath(relPath);

   // Now restore the material names since someone later may
   // count on the special texture names being present.
   for (U32 i = 0; i < originalNames.size(); i++)
      mMaterialList->setMaterialName(i, originalNames[i]);


   fillSurfaceTexMats();
   createZoneVBs();
   cloneMatInstances();
   createReflectPlanes();
   initMatInstances();

   // lightmap manager steals the lightmaps here...
   gInteriorLMManager.addInterior(mLMHandle, mLightmaps.size(), this);
   AssertFatal(!mLightmaps.size(), "Failed to process lightmaps");

   for(U32 i=0; i<mStaticMeshes.size(); i++)
      mStaticMeshes[i]->prepForRendering(relPath);

   GFXStateBlockDesc sh;

#ifndef TORQUE_SHIPPING
   // First create a default state block with
   // texturing turned off
   mInteriorDebugNoneSB = GFX->createStateBlock(sh);

   // Create a state block for portal rendering that
   // doesn't have backface culling enabled
   sh.cullDefined = true;
   sh.cullMode = GFXCullNone;

   mInteriorDebugPortalSB = GFX->createStateBlock(sh);

   // Reset our cull mode to the default
   sh.cullMode = GFXCullCCW;
#endif

   // Next turn on the first texture channel
   sh.samplersDefined = true;
   sh.samplers[0].textureColorOp = GFXTOPModulate;

#ifndef TORQUE_SHIPPING
   mInteriorDebugTextureSB = GFX->createStateBlock(sh);

   sh.samplers[1].textureColorOp = GFXTOPModulate;

   mInteriorDebugTwoTextureSB = GFX->createStateBlock(sh);
#endif

   // Lastly create a standard rendering state block
   sh.samplers[2].textureColorOp = GFXTOPModulate;

   sh.samplers[0].magFilter = GFXTextureFilterLinear;
   sh.samplers[0].minFilter = GFXTextureFilterLinear;

   mInteriorSB = GFX->createStateBlock(sh);

   mPreppedForRender = true;

   return true;
}


void Interior::setupAveTexGenLength()
{
/*
   F32 len = 0;
   for (U32 i = 0; i < mSurfaces.size(); i++)
   {
      // We're going to assume that most textures don't have separate scales for
      //  x and y...
      F32 lenx = mTexGenEQs[mSurfaces[i].texGenIndex].planeX.len();
      len += F32((*mMaterialList)[mSurfaces[i].textureIndex].getWidth()) * lenx;
   }
   len /= F32(mSurfaces.size());
   mAveTexGenLength = len;
*/
}


//--------------------------------------------------------------------------
bool Interior::traverseZones(SceneCullingState*    state,
                             const Frustum& frustum,
                             S32            containingZone,
                             S32            baseZone,
                             U32            zoneOffset,
                             const MatrixF& OSToWS,
                             const Point3F& objScale,
                             const bool     dontRestrictOutside,
                             const bool     flipClipPlanes,
                             Frustum&       outFrustum)
{
   // Store off the viewport and frustum
   sgStoredViewport = state->getCameraState().getViewport();
   if( dontRestrictOutside )
   {
      sgStoredFrustum[0] = state->getFrustum().getNearLeft();
      sgStoredFrustum[1] = state->getFrustum().getNearRight();
      sgStoredFrustum[2] = state->getFrustum().getNearBottom();
      sgStoredFrustum[3] = state->getFrustum().getNearTop();
      sgStoredFrustum[4] = state->getFrustum().getNearDist();
      sgStoredFrustum[5] = state->getFrustum().getFarDist();
   }
   else
   {
      sgStoredFrustum[0] = frustum.getNearLeft();
      sgStoredFrustum[1] = frustum.getNearRight();
      sgStoredFrustum[2] = frustum.getNearBottom();
      sgStoredFrustum[3] = frustum.getNearTop();
      sgStoredFrustum[4] = frustum.getNearDist();
      sgStoredFrustum[5] = frustum.getFarDist();
   }
   
   sgProjMatrix = state->getCameraState().getProjectionMatrix();

   MatrixF finalModelView = state->getCameraState().getWorldViewMatrix();
   finalModelView.mul(OSToWS);
   finalModelView.scale(Point3F(objScale.x, objScale.y, objScale.z));
   sgProjMatrix.mul(finalModelView);

   finalModelView.inverse();
   finalModelView.mulP(Point3F(0, 0, 0), &sgCamPoint);
   sgWSToOSMatrix = finalModelView;

   // do the zone traversal
   sgZoneRenderInfo.setSize(mZones.size());
   zoneTraversal(baseZone, flipClipPlanes);

   // Copy out the information for all zones but the outside zone.
   for(U32 i = 1; i < mZones.size(); i++)
   {
      AssertFatal(zoneOffset != 0xFFFFFFFF, "Error, this should never happen!");
      U32 globalIndex = i + zoneOffset - 1;

      if( sgZoneRenderInfo[ i ].render )
      {
         state->addCullingVolumeToZone(
            globalIndex,
            SceneCullingVolume::Includer,
            Frustum(
               frustum.isOrtho(),
               sgZoneRenderInfo[ i ].frustum[ 0 ],
               sgZoneRenderInfo[ i ].frustum[ 1 ],
               sgZoneRenderInfo[ i ].frustum[ 3 ],
               sgZoneRenderInfo[ i ].frustum[ 2 ],
               frustum.getNearDist(),
               frustum.getFarDist(),
               frustum.getTransform()
            )
         );
      }
   }

   destroyZoneRectVectors();

   // If zone 0 is rendered, then we return true...
   bool continueOut = sgZoneRenderInfo[ 0 ].render;
   if( continueOut )
      outFrustum = Frustum(
         frustum.isOrtho(),
         sgZoneRenderInfo[ 0 ].frustum[ 0 ],
         sgZoneRenderInfo[ 0 ].frustum[ 1 ],
         sgZoneRenderInfo[ 0 ].frustum[ 3 ],
         sgZoneRenderInfo[ 0 ].frustum[ 2 ],
         frustum.getNearDist(),
         frustum.getFarDist(),
         frustum.getTransform()
      );

   return sgZoneRenderInfo[0].render;
}


//------------------------------------------------------------------------------
S32 Interior::getZoneForPoint(const Point3F& rPoint) const
{
   const IBSPNode* pNode = &mBSPNodes[0];

   while (true) {
      F32 dist = getPlane(pNode->planeIndex).distToPlane(rPoint);
      if (planeIsFlipped(pNode->planeIndex))
         dist = -dist;

      U32 traverseIndex;
      if (dist >= 0)
         traverseIndex = pNode->frontIndex;
      else
         traverseIndex = pNode->backIndex;

      if (isBSPLeafIndex(traverseIndex)) {
         if (isBSPSolidLeaf(traverseIndex)) {
            return -1;
         } else {
            U16 zone = getBSPEmptyLeafZone(traverseIndex);
            if (zone == 0x0FFF)
               return -1;
            else
               return zone;
         }
      }

      pNode = &mBSPNodes[traverseIndex];
   }
}


//--------------------------------------------------------------------------
static void itrClipToPlane(Point3F* points, U32& rNumPoints, const PlaneF& rPlane)
{
   S32 start = -1;
   for(U32 i = 0; i < rNumPoints; i++)
   {
      if(rPlane.whichSide(points[i]) == PlaneF::Front)
      {
         start = i;
         break;
      }
   }

   // Nothing was in front of the plane...
   if(start == -1)
   {
      rNumPoints = 0;
      return;
   }

   static Point3F finalPoints[128];
   U32  numFinalPoints = 0;

   U32 baseStart = start;
   U32 end       = (start + 1) % rNumPoints;

   while(end != baseStart)
   {
      const Point3F& rStartPoint = points[start];
      const Point3F& rEndPoint   = points[end];

      PlaneF::Side fSide = rPlane.whichSide(rStartPoint);
      PlaneF::Side eSide = rPlane.whichSide(rEndPoint);

      S32 code = fSide * 3 + eSide;
      switch(code)
      {
      case 4:   // f f
      case 3:   // f o
      case 1:   // o f
      case 0:   // o o
         // No Clipping required
         finalPoints[numFinalPoints++] = points[start];
         start = end;
         end   = (end + 1) % rNumPoints;
         break;


      case 2: // f b
         {
            // In this case, we emit the front point, Insert the intersection,
            //  and advancing to point to first point that is in front or on...
            //
            finalPoints[numFinalPoints++] = points[start];

            Point3F vector = rEndPoint - rStartPoint;
            F32 t        = -(rPlane.distToPlane(rStartPoint) / mDot(rPlane, vector));

            Point3F intersection = rStartPoint + (vector * t);
            finalPoints[numFinalPoints++] = intersection;

            U32 endSeek = (end + 1) % rNumPoints;
            while(rPlane.whichSide(points[endSeek]) == PlaneF::Back)
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

      case -1: // o b
         {
            // In this case, we emit the front point, and advance to point to first
            //  point that is in front or on...
            //
            finalPoints[numFinalPoints++] = points[start];

            U32 endSeek = (end + 1) % rNumPoints;
            while(rPlane.whichSide(points[endSeek]) == PlaneF::Back)
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
         AssertISV(false, "CSGPlane::clipWindingToPlaneFront: error in polygon clipper");
         break;

      default:
         AssertFatal(false, "CSGPlane::clipWindingToPlaneFront: bad outcode");
         break;
      }

   }

   // Emit the last point.
   finalPoints[numFinalPoints++] = points[start];
   AssertFatal(numFinalPoints >= 3, avar("Error, this shouldn't happen!  Invalid winding in itrClipToPlane: %d", numFinalPoints));

   // Copy the new rWinding, and we're set!
   //
   dMemcpy(points, finalPoints, numFinalPoints * sizeof(Point3F));
   rNumPoints = numFinalPoints;
   AssertISV(rNumPoints <= 128, "Increase maxWindingPoints.  Talk to DMoore");
}

bool Interior::projectClipAndBoundFan(U32 fanIndex, F64* pResult)
{
   const TriFan& rFan = mWindingIndices[fanIndex];

   static Point3F windingPoints[128];
   U32 numPoints = rFan.windingCount;
   U32 i;
   for(i = 0; i < numPoints; i++)
      windingPoints[i] = mPoints[mWindings[rFan.windingStart + i]].point;

   itrClipToPlane(windingPoints, numPoints, sgOSPlaneFar);
   if(numPoints != 0)
      itrClipToPlane(windingPoints, numPoints, sgOSPlaneXMin);
   if(numPoints != 0)
      itrClipToPlane(windingPoints, numPoints, sgOSPlaneXMax);
   if(numPoints != 0)
      itrClipToPlane(windingPoints, numPoints, sgOSPlaneYMin);
   if(numPoints != 0)
      itrClipToPlane(windingPoints, numPoints, sgOSPlaneYMax);

   if(numPoints == 0)
   {
      pResult[0] =
      pResult[1] =
      pResult[2] =
      pResult[3] = 0.0f;
      return false;
   }

   F32 minX = 1e10;
   F32 maxX = -1e10;
   F32 minY = 1e10;
   F32 maxY = -1e10;
   static Point4F projPoints[128];
   for(i = 0; i < numPoints; i++)
   {
      projPoints[i].set(windingPoints[i].x, windingPoints[i].y, windingPoints[i].z, 1.0);
      sgProjMatrix.mul(projPoints[i]);

      AssertFatal(projPoints[i].w != 0.0, "Error, that's bad!");
      projPoints[i].x /= projPoints[i].w;
      projPoints[i].y /= projPoints[i].w;

      if(projPoints[i].x < minX)
         minX = projPoints[i].x;
      if(projPoints[i].x > maxX)
         maxX = projPoints[i].x;
      if(projPoints[i].y < minY)
         minY = projPoints[i].y;
      if(projPoints[i].y > maxY)
         maxY = projPoints[i].y;
   }

   if(minX < -1.0f) minX = -1.0f;
   if(minY < -1.0f) minY = -1.0f;
   if(maxX > 1.0f)  maxX = 1.0f;
   if(maxY > 1.0f)  maxY = 1.0f;

   pResult[0] = minX;
   pResult[1] = minY;
   pResult[2] = maxX;
   pResult[3] = maxY;
   return true;
}

void Interior::createZoneRectVectors()
{
   sgZoneRects.setSize(mZones.size());
   for(U32 i = 0; i < mZones.size(); i++)
      sgZoneRects[i].active = false;
}

void Interior::destroyZoneRectVectors()
{

}

void Interior::traverseZone(const RectD* inputRects, const U32 numInputRects, U32 currZone, Vector<U32>& zoneStack)
{
   PROFILE_START(InteriorTraverseZone);
   // First, we push onto our rect list all the inputRects...
   insertZoneRects(sgZoneRects[currZone], inputRects, numInputRects);

   // A portal is a valid traversal if the camera point is on the
   //  same side of it's plane as the zone.  It must then pass the
   //  clip/project test.
   U32 i;
   const Zone& rZone = mZones[currZone];
   for(i = rZone.portalStart; i < U32(rZone.portalStart + rZone.portalCount); i++)
   {
      const Portal& rPortal = mPortals[mZonePortalList[i]];
      AssertFatal(U32(rPortal.zoneFront) == currZone || U32(rPortal.zoneBack) == currZone,
                  "Portal doesn't reference this zone?");

      S32 camSide = getPlane(rPortal.planeIndex).whichSide(sgCamPoint);
      if(planeIsFlipped(rPortal.planeIndex))
         camSide = -camSide;
      S32  zoneSide  = (U32(rPortal.zoneFront) == currZone) ? 1 : -1;
      U16 otherZone = (U32(rPortal.zoneFront) == currZone) ? rPortal.zoneBack : rPortal.zoneFront;

      // Make sure this isn't a free floating portal...
      if(otherZone == currZone)
         continue;

      // Make sure we haven't encountered this zone already in this traversal
      bool onStack = false;
      for(U32 i = 0; i < zoneStack.size(); i++)
      {
         if(otherZone == zoneStack[i])
         {
            onStack = true;
            break;
         }
      }
      if(onStack == true)
         continue;

      if(camSide == zoneSide)
      {
         // Can traverse.  Note: special case PlaneF::On
         //  here to prevent possible w == 0 problems and infinite recursion
//          Vector<RectD> newRects;
//          VECTOR_SET_ASSOCIATION(newRects);

         // We're abusing the heck out of the allocator here.
         U32 waterMark = FrameAllocator::getWaterMark();
         RectD* newRects = (RectD*)FrameAllocator::alloc(1);
         U32 numNewRects = 0;

         for(S32 j = 0; j < rPortal.triFanCount; j++)
         {
            F64 result[4];
            if(projectClipAndBoundFan(rPortal.triFanStart + j, result))
            {
               // Have a good rect from this.
               RectD possible = convertToRectD(result);

               for(U32 k = 0; k < numInputRects; k++)
               {
                  RectD copy = possible;
                  if(copy.intersect(inputRects[k]))
                     newRects[numNewRects++] = copy;
               }
            }
         }

         if(numNewRects != 0)
         {
            FrameAllocator::alloc((sizeof(RectD) * numNewRects) - 1);

            U32 prevStackSize = zoneStack.size();
            zoneStack.push_back(currZone);
            traverseZone(newRects, numNewRects, otherZone, zoneStack);
            zoneStack.pop_back();
            AssertFatal(zoneStack.size() == prevStackSize, "Error, stack size changed!");
         }
         FrameAllocator::setWaterMark(waterMark);
      }
      else if(camSide == PlaneF::On)
      {
         U32 waterMark = FrameAllocator::getWaterMark();
         RectD* newRects = (RectD*)FrameAllocator::alloc(numInputRects * sizeof(RectD));
         dMemcpy(newRects, inputRects, sizeof(RectD) * numInputRects);

         U32 prevStackSize = zoneStack.size();
         zoneStack.push_back(currZone);
         traverseZone(newRects, numInputRects, otherZone, zoneStack);
         zoneStack.pop_back();
         AssertFatal(zoneStack.size() == prevStackSize, "Error, stack size changed!");
         FrameAllocator::setWaterMark(waterMark);
      }
   }
   PROFILE_END();
}

void Interior::zoneTraversal(S32 baseZone, const bool flipClip)
{
   PROFILE_START(InteriorZoneTraversal);
   // If we're in solid, render everything...
   if(baseZone == -1)
   {
      for(U32 i = 0; i < mZones.size(); i++)
      {
         sgZoneRenderInfo[i].render = true;

         sgZoneRenderInfo[i].frustum[0] = sgStoredFrustum[0];
         sgZoneRenderInfo[i].frustum[1] = sgStoredFrustum[1];
         sgZoneRenderInfo[i].frustum[2] = sgStoredFrustum[2];
         sgZoneRenderInfo[i].frustum[3] = sgStoredFrustum[3];
         sgZoneRenderInfo[i].viewport   = sgStoredViewport;
      }
      PROFILE_END();
      return;
   }

   // Otherwise, we're going to have to do some work...
   createZoneRectVectors();
   U32 i;
   for(i = 0; i < mZones.size(); i++)
      sgZoneRenderInfo[i].render = false;

   // Create the object space clipping planes...
   sgComputeOSFrustumPlanes(sgStoredFrustum,
                            sgWSToOSMatrix,
                            sgCamPoint,
                            sgOSPlaneFar,
                            sgOSPlaneXMin,
                            sgOSPlaneXMax,
                            sgOSPlaneYMin,
                            sgOSPlaneYMax);

   if(flipClip == true)
   {
      sgOSPlaneXMin.neg();
      sgOSPlaneXMax.neg();
      sgOSPlaneYMin.neg();
      sgOSPlaneYMax.neg();
   }
   // First, the current zone gets the full clipRect, and marked as rendering...
   static const F64 fullResult[4] = { -1, -1, 1, 1};

   static Vector<U32> zoneStack;
   zoneStack.clear();
   VECTOR_SET_ASSOCIATION(zoneStack);

   RectD baseRect = convertToRectD(fullResult);
   traverseZone(&baseRect, 1, baseZone, zoneStack);

   for(i = 0; i < mZones.size(); i++)
   {
      if(sgZoneRects[i].active == true)
      {
         sgZoneRenderInfo[i].render = true;
         convertToFrustum(sgZoneRenderInfo[i], sgZoneRects[i].rect);
      }
   }
   PROFILE_END();
}


void mergeSurfaceVectors(const U16* from0,
                         const U32  size0,
                         const U16* from1,
                         const U32  size1,
                         U16*       output,
                         U32*       outputSize)
{
   U32 pos0 = 0;
   U32 pos1 = 0;
   U32 outputCount = 0;
   while(pos0 < size0 && pos1 < size1)
   {
      if(from0[pos0] < from1[pos1])
      {
         output[outputCount++] = from0[pos0++];
      }
      else if(from0[pos0] == from1[pos1])
      {
         // Equal, output one, and inc both counts
         output[outputCount++] = from0[pos0++];
         pos1++;
      }
      else
      {
         output[outputCount++] = from1[pos1++];
      }
   }
   AssertFatal(pos0 == size0 || pos1 == size1, "Error, one of these must have reached the end!");

   // Copy the dregs...
   if(pos0 != size0)
   {
      dMemcpy(&output[outputCount], &from0[pos0], sizeof(U16) * (size0 - pos0));
      outputCount += size0 - pos0;
   }
   else if(pos1 != size1)
   {
      dMemcpy(&output[outputCount], &from1[pos1], sizeof(U16) * (size1 - pos1));
      outputCount += size1 - pos1;
   }

   *outputSize = outputCount;
}

// Remove any collision hulls, interval trees, etc...
//
void Interior::purgeLODData()
{
   mConvexHulls.clear();
   mHullIndices.clear();
   mHullEmitStringIndices.clear();
   mHullSurfaceIndices.clear();
   mCoordBinIndices.clear();
   mConvexHullEmitStrings.clear();
   for(U32 i = 0; i < NumCoordBins * NumCoordBins; i++)
   {
      mCoordBins[i].binStart = 0;
      mCoordBins[i].binCount = 0;
   }
}

// Build an OptimizedPolyList that represents this Interior's mesh
void Interior::buildExportPolyList(OptimizedPolyList& polys, MatrixF* mat, Point3F* scale)
{
   MatrixF saveMat;
   Point3F saveScale;
   polys.getTransform(&saveMat, &saveScale);

   if (mat)
   {
      if (scale)
         polys.setTransform(mat, *scale);
      else
         polys.setTransform(mat, Point3F(1.0f, 1.0f, 1.0f));
   }

   // Create one TSMesh per zone
   for (U32 i = 0; i < mZones.size(); i++)
   {
      const Interior::Zone& zone = mZones[i];

      // Gather some data
      for (U32 j = 0; j < zone.surfaceCount; j++)
      {
         U32 sdx = mZoneSurfaces[zone.surfaceStart + j];

         const Interior::Surface& surface = mSurfaces[sdx];

         // Snag the MaterialInstance
         BaseMatInstance *matInst = mMaterialList->getMaterialInst( surface.textureIndex );

         // Start a poly
         polys.begin(matInst, j, OptimizedPolyList::TriangleStrip);

         // Set its plane
         PlaneF plane = getFlippedPlane(surface.planeIndex);
         polys.plane(plane);

         // Get its texGen so that we can calculate uvs
         Interior::TexGenPlanes texGens = mTexGenEQs[surface.texGenIndex];
         texGens.planeY.invert();

         // Loop through and add the verts and uvs
         for (U32 k = 0; k < surface.windingCount; k++)
         {
            // Get our point
            U32 vdx = mWindings[surface.windingStart + k];
            const Point3F& pt = mPoints[vdx].point;

            // Get our uv
            Point2F uv;
            uv.x = texGens.planeX.distToPlane(pt);
            uv.y = texGens.planeY.distToPlane(pt);

            Point3F normal = getPointNormal(sdx, k);

            polys.vertex(pt, normal, uv);
         }

         polys.end();
      }
   }

   polys.setTransform(&saveMat, saveScale);
}


struct TempProcSurface
{
   U32 numPoints;
   U32 pointIndices[32];
   U16 planeIndex;
   U8  mask;
};

struct PlaneGrouping
{
   U32 numPlanes;
   U16 planeIndices[32];
   U8  mask;
};


//--------------------------------------------------------------------------
void Interior::processHullPolyLists()
{
   Vector<U16>             planeIndices(256, __FILE__, __LINE__);
   Vector<U32>             pointIndices(256, __FILE__, __LINE__);
   Vector<U8>              pointMasks(256, __FILE__, __LINE__);
   Vector<U8>              planeMasks(256, __FILE__, __LINE__);
   Vector<TempProcSurface> tempSurfaces(128, __FILE__, __LINE__);
   Vector<PlaneGrouping>   planeGroups(32, __FILE__, __LINE__);

   // Reserve space in the vectors
   {
      mPolyListStrings.setSize(0);
      mPolyListStrings.reserve(128 << 10);

      mPolyListPoints.setSize(0);
      mPolyListPoints.reserve(32 << 10);

      mPolyListPlanes.setSize(0);
      mPolyListPlanes.reserve(16 << 10);
   }

   for(U32 i = 0; i < mConvexHulls.size(); i++)
   {
      U32 j, k, l, m;

      ConvexHull& rHull = mConvexHulls[i];

      planeIndices.setSize(0);
      pointIndices.setSize(0);
      tempSurfaces.setSize(0);

      // Extract all the surfaces from this hull into our temporary processing format
      {
         for(j = 0; j < rHull.surfaceCount; j++)
         {
            tempSurfaces.increment();
            TempProcSurface& temp = tempSurfaces.last();

            U32 surfaceIndex = mHullSurfaceIndices[j + rHull.surfaceStart];
            if(isNullSurfaceIndex(surfaceIndex))
            {
               const NullSurface& rSurface = mNullSurfaces[getNullSurfaceIndex(surfaceIndex)];

               temp.planeIndex = rSurface.planeIndex;
               temp.numPoints  = rSurface.windingCount;
               for(k = 0; k < rSurface.windingCount; k++)
                  temp.pointIndices[k] = mWindings[rSurface.windingStart + k];
            }
            else
            {
               const Surface& rSurface = mSurfaces[surfaceIndex];

               temp.planeIndex = rSurface.planeIndex;
               collisionFanFromSurface(rSurface, temp.pointIndices, &temp.numPoints);
            }
         }
      }

      // First order of business: extract all unique planes and points from
      //  the list of surfaces...
      {
         for(j = 0; j < tempSurfaces.size(); j++)
         {
            const TempProcSurface& rSurface = tempSurfaces[j];

            bool found = false;
            for(k = 0; k < planeIndices.size() && !found; k++)
            {
               if(rSurface.planeIndex == planeIndices[k])
                  found = true;
            }
            if(!found)
               planeIndices.push_back(rSurface.planeIndex);

            for(k = 0; k < rSurface.numPoints; k++)
            {
               found = false;
               for(l = 0; l < pointIndices.size(); l++)
               {
                  if(pointIndices[l] == rSurface.pointIndices[k])
                     found = true;
               }
               if(!found)
                  pointIndices.push_back(rSurface.pointIndices[k]);
            }
         }
      }

      // Now that we have all the unique points and planes, remap the surfaces in
      //  terms of the offsets into the unique point list...
      {
         for(j = 0; j < tempSurfaces.size(); j++)
         {
            TempProcSurface& rSurface = tempSurfaces[j];

            // Points
            for(k = 0; k < rSurface.numPoints; k++)
            {
               bool found = false;
               for(l = 0; l < pointIndices.size(); l++)
               {
                  if(pointIndices[l] == rSurface.pointIndices[k])
                  {
                     rSurface.pointIndices[k] = l;
                     found = true;
                     break;
                  }
               }
               AssertISV(found, "Error remapping point indices in interior collision processing");
            }
         }
      }

      // Ok, at this point, we have a list of unique points, unique planes, and the
      //  surfaces all remapped in those terms.  We need to check our error conditions
      //  that will make sure that we can properly encode this hull:
      {
         AssertISV(planeIndices.size() < 256,   "Error, > 256 planes on an interior hull");
         AssertISV(pointIndices.size() < 63356, "Error, > 65536 points on an interior hull");
         AssertISV(tempSurfaces.size() < 256,   "Error, > 256 surfaces on an interior hull");
      }

      // Now we group the planes together, and merge the closest groups until we're left
      //  with <= 8 groups
      {
         planeGroups.setSize(planeIndices.size());
         for(j = 0; j < planeIndices.size(); j++)
         {
            planeGroups[j].numPlanes       = 1;
            planeGroups[j].planeIndices[0] = planeIndices[j];
         }

         while(planeGroups.size() > 8)
         {
            // Find the two closest groups.  If mdp(i, j) is the value of the
            //  largest pairwise dot product that can be computed from the vectors
            //  of group i, and group j, then the closest group pair is the one
            //  with the smallest value of mdp.
            F32 currmin = 2;
            S32 firstGroup  = -1;
            S32 secondGroup = -1;

            for(j = 0; j < planeGroups.size(); j++)
            {
               PlaneGrouping& first = planeGroups[j];
               for(k = j + 1; k < planeGroups.size(); k++)
               {
                  PlaneGrouping& second = planeGroups[k];

                  F32 max = -2;
                  for(l = 0; l < first.numPlanes; l++)
                  {
                     for(m = 0; m < second.numPlanes; m++)
                     {
                        Point3F firstNormal = getPlane(first.planeIndices[l]);
                        if(planeIsFlipped(first.planeIndices[l]))
                           firstNormal.neg();
                        Point3F secondNormal = getPlane(second.planeIndices[m]);
                        if(planeIsFlipped(second.planeIndices[m]))
                           secondNormal.neg();

                        F32 dot = mDot(firstNormal, secondNormal);
                        if(dot > max)
                           max = dot;
                     }
                  }

                  if(max < currmin)
                  {
                     currmin = max;
                     firstGroup  = j;
                     secondGroup = k;
                  }
               }
            }
            AssertFatal(firstGroup != -1 && secondGroup != -1, "Error, unable to find a suitable pairing?");

            // Merge first and second
            PlaneGrouping& to   = planeGroups[firstGroup];
            PlaneGrouping& from = planeGroups[secondGroup];
            while(from.numPlanes != 0)
            {
               to.planeIndices[to.numPlanes++] = from.planeIndices[from.numPlanes - 1];
               from.numPlanes--;
            }

            // And remove the merged group
            planeGroups.erase(secondGroup);
         }
         AssertFatal(planeGroups.size() <= 8, "Error, too many plane groupings!");


         // Assign a mask to each of the plane groupings
         for(j = 0; j < planeGroups.size(); j++)
            planeGroups[j].mask = (1 << j);
      }

      // Now, assign the mask to each of the temp polys
      {
         for(j = 0; j < tempSurfaces.size(); j++)
         {
            bool assigned = false;
            for(k = 0; k < planeGroups.size() && !assigned; k++)
            {
               for(l = 0; l < planeGroups[k].numPlanes; l++)
               {
                  if(planeGroups[k].planeIndices[l] == tempSurfaces[j].planeIndex)
                  {
                     tempSurfaces[j].mask = planeGroups[k].mask;
                     assigned = true;
                     break;
                  }
               }
            }
            AssertFatal(assigned, "Error, missed a plane somewhere in the hull poly list!");
         }
      }

      // Copy the appropriate group mask to the plane masks
      {
         planeMasks.setSize(planeIndices.size());
         dMemset(planeMasks.address(), 0, planeMasks.size() * sizeof(U8));

         for(j = 0; j < planeIndices.size(); j++)
         {
            bool found = false;
            for(k = 0; k < planeGroups.size() && !found; k++)
            {
               for(l = 0; l < planeGroups[k].numPlanes; l++)
               {
                  if(planeGroups[k].planeIndices[l] == planeIndices[j])
                  {
                     planeMasks[j] = planeGroups[k].mask;
                     found = true;
                     break;
                  }
               }
            }
            AssertFatal(planeMasks[j] != 0, "Error, missing mask for plane!");
         }
      }

      // And whip through the points, constructing the total mask for that point
      {
         pointMasks.setSize(pointIndices.size());
         dMemset(pointMasks.address(), 0, pointMasks.size() * sizeof(U8));

         for(j = 0; j < pointIndices.size(); j++)
         {
            for(k = 0; k < tempSurfaces.size(); k++)
            {
               for(l = 0; l < tempSurfaces[k].numPoints; l++)
               {
                  if(tempSurfaces[k].pointIndices[l] == j)
                  {
                     pointMasks[j] |= tempSurfaces[k].mask;
                     break;
                  }
               }
            }
            AssertFatal(pointMasks[j] != 0, "Error, point must exist in at least one surface!");
         }
      }

      // Create the emit strings, and we're done!
      {
         // Set the range of planes
         rHull.polyListPlaneStart = mPolyListPlanes.size();
         mPolyListPlanes.setSize(rHull.polyListPlaneStart + planeIndices.size());
         for(j = 0; j < planeIndices.size(); j++)
            mPolyListPlanes[j + rHull.polyListPlaneStart] = planeIndices[j];

         // Set the range of points
         rHull.polyListPointStart = mPolyListPoints.size();
         mPolyListPoints.setSize(rHull.polyListPointStart + pointIndices.size());
         for(j = 0; j < pointIndices.size(); j++)
            mPolyListPoints[j + rHull.polyListPointStart] = pointIndices[j];

         // Now the emit string.  The emit string goes like: (all fields are bytes)
         //  NumPlanes (PLMask) * NumPlanes
         //  NumPointsHi NumPointsLo (PtMask) * NumPoints
         //  NumSurfaces
         //   (NumPoints SurfaceMask PlOffset (PtOffsetHi PtOffsetLo) * NumPoints) * NumSurfaces
         //
         U32 stringLen  = 1 + planeIndices.size();
         stringLen     += 2 + pointIndices.size();
         stringLen     += 1;
         for(j = 0; j < tempSurfaces.size(); j++)
            stringLen += 1 + 1 + 1 + (tempSurfaces[j].numPoints * 2);

         rHull.polyListStringStart = mPolyListStrings.size();
         mPolyListStrings.setSize(rHull.polyListStringStart + stringLen);

         U8* pString = &mPolyListStrings[rHull.polyListStringStart];
         U32 currPos = 0;

         // Planes
         pString[currPos++] = planeIndices.size();
         for(j = 0; j < planeIndices.size(); j++)
            pString[currPos++] = planeMasks[j];

         // Points
         pString[currPos++] = (pointIndices.size() >> 8) & 0xFF;
         pString[currPos++] = (pointIndices.size() >> 0) & 0xFF;
         for(j = 0; j < pointIndices.size(); j++)
            pString[currPos++] = pointMasks[j];

         // Surfaces
         pString[currPos++] = tempSurfaces.size();
         for(j = 0; j < tempSurfaces.size(); j++)
         {
            pString[currPos++] = tempSurfaces[j].numPoints;
            pString[currPos++] = tempSurfaces[j].mask;

            bool found = false;
            for(k = 0; k < planeIndices.size(); k++)
            {
               if(planeIndices[k] == tempSurfaces[j].planeIndex)
               {
                  pString[currPos++] = k;
                  found = true;
                  break;
               }
            }
            AssertFatal(found, "Error, missing planeindex!");

            for(k = 0; k < tempSurfaces[j].numPoints; k++)
            {
               pString[currPos++] = (tempSurfaces[j].pointIndices[k] >> 8) & 0xFF;
               pString[currPos++] = (tempSurfaces[j].pointIndices[k] >> 0) & 0xFF;
            }
         }
         AssertFatal(currPos == stringLen, "Error, mismatched string length!");
      }
   } // for (i = 0; i < mConvexHulls.size(); i++)

   // Compact the used vectors
   {
      mPolyListStrings.compact();
      mPolyListPoints.compact();
      mPolyListPlanes.compact();
   }
}

//--------------------------------------------------------------------------
void Interior::processVehicleHullPolyLists()
{
   Vector<U16>             planeIndices(256, __FILE__, __LINE__);
   Vector<U32>             pointIndices(256, __FILE__, __LINE__);
   Vector<U8>              pointMasks(256, __FILE__, __LINE__);
   Vector<U8>              planeMasks(256, __FILE__, __LINE__);
   Vector<TempProcSurface> tempSurfaces(128, __FILE__, __LINE__);
   Vector<PlaneGrouping>   planeGroups(32, __FILE__, __LINE__);

   // Reserve space in the vectors
   {
      mVehiclePolyListStrings.setSize(0);
      mVehiclePolyListStrings.reserve(128 << 10);

      mVehiclePolyListPoints.setSize(0);
      mVehiclePolyListPoints.reserve(32 << 10);

      mVehiclePolyListPlanes.setSize(0);
      mVehiclePolyListPlanes.reserve(16 << 10);
   }

   for(U32 i = 0; i < mVehicleConvexHulls.size(); i++)
   {
      U32 j, k, l, m;

      ConvexHull& rHull = mVehicleConvexHulls[i];

      planeIndices.setSize(0);
      pointIndices.setSize(0);
      tempSurfaces.setSize(0);

      // Extract all the surfaces from this hull into our temporary processing format
      {
         for(j = 0; j < rHull.surfaceCount; j++)
         {
            tempSurfaces.increment();
            TempProcSurface& temp = tempSurfaces.last();

            U32 surfaceIndex = mVehicleHullSurfaceIndices[j + rHull.surfaceStart];
            const NullSurface& rSurface = mVehicleNullSurfaces[getVehicleNullSurfaceIndex(surfaceIndex)];

            temp.planeIndex = rSurface.planeIndex;
            temp.numPoints  = rSurface.windingCount;
            for(k = 0; k < rSurface.windingCount; k++)
               temp.pointIndices[k] = mVehicleWindings[rSurface.windingStart + k];
         }
      }

      // First order of business: extract all unique planes and points from
      //  the list of surfaces...
      {
         for(j = 0; j < tempSurfaces.size(); j++)
         {
            const TempProcSurface& rSurface = tempSurfaces[j];

            bool found = false;
            for(k = 0; k < planeIndices.size() && !found; k++)
            {
               if(rSurface.planeIndex == planeIndices[k])
                  found = true;
            }
            if(!found)
               planeIndices.push_back(rSurface.planeIndex);

            for(k = 0; k < rSurface.numPoints; k++)
            {
               found = false;
               for(l = 0; l < pointIndices.size(); l++)
               {
                  if(pointIndices[l] == rSurface.pointIndices[k])
                     found = true;
               }
               if(!found)
                  pointIndices.push_back(rSurface.pointIndices[k]);
            }
         }
      }

      // Now that we have all the unique points and planes, remap the surfaces in
      //  terms of the offsets into the unique point list...
      {
         for(j = 0; j < tempSurfaces.size(); j++)
         {
            TempProcSurface& rSurface = tempSurfaces[j];

            // Points
            for(k = 0; k < rSurface.numPoints; k++)
            {
               bool found = false;
               for(l = 0; l < pointIndices.size(); l++)
               {
                  if(pointIndices[l] == rSurface.pointIndices[k])
                  {
                     rSurface.pointIndices[k] = l;
                     found = true;
                     break;
                  }
               }
               AssertISV(found, "Error remapping point indices in interior collision processing");
            }
         }
      }

      // Ok, at this point, we have a list of unique points, unique planes, and the
      //  surfaces all remapped in those terms.  We need to check our error conditions
      //  that will make sure that we can properly encode this hull:
      {
         AssertISV(planeIndices.size() < 256,   "Error, > 256 planes on an interior hull");
         AssertISV(pointIndices.size() < 63356, "Error, > 65536 points on an interior hull");
         AssertISV(tempSurfaces.size() < 256,   "Error, > 256 surfaces on an interior hull");
      }

      // Now we group the planes together, and merge the closest groups until we're left
      //  with <= 8 groups
      {
         planeGroups.setSize(planeIndices.size());
         for(j = 0; j < planeIndices.size(); j++)
         {
            planeGroups[j].numPlanes       = 1;
            planeGroups[j].planeIndices[0] = planeIndices[j];
         }

         while(planeGroups.size() > 8)
         {
            // Find the two closest groups.  If mdp(i, j) is the value of the
            //  largest pairwise dot product that can be computed from the vectors
            //  of group i, and group j, then the closest group pair is the one
            //  with the smallest value of mdp.
            F32 currmin = 2;
            S32 firstGroup  = -1;
            S32 secondGroup = -1;

            for(j = 0; j < planeGroups.size(); j++)
            {
               PlaneGrouping& first = planeGroups[j];
               for(k = j + 1; k < planeGroups.size(); k++)
               {
                  PlaneGrouping& second = planeGroups[k];

                  F32 max = -2;
                  for(l = 0; l < first.numPlanes; l++)
                  {
                     for(m = 0; m < second.numPlanes; m++)
                     {
                        Point3F firstNormal = mVehiclePlanes[first.planeIndices[l]];
                        Point3F secondNormal = mVehiclePlanes[second.planeIndices[m]];

                        F32 dot = mDot(firstNormal, secondNormal);
                        if(dot > max)
                           max = dot;
                     }
                  }

                  if(max < currmin)
                  {
                     currmin = max;
                     firstGroup  = j;
                     secondGroup = k;
                  }
               }
            }
            AssertFatal(firstGroup != -1 && secondGroup != -1, "Error, unable to find a suitable pairing?");

            // Merge first and second
            PlaneGrouping& to   = planeGroups[firstGroup];
            PlaneGrouping& from = planeGroups[secondGroup];
            while(from.numPlanes != 0)
            {
               to.planeIndices[to.numPlanes++] = from.planeIndices[from.numPlanes - 1];
               from.numPlanes--;
            }

            // And remove the merged group
            planeGroups.erase(secondGroup);
         }
         AssertFatal(planeGroups.size() <= 8, "Error, too many plane groupings!");


         // Assign a mask to each of the plane groupings
         for(j = 0; j < planeGroups.size(); j++)
            planeGroups[j].mask = (1 << j);
      }

      // Now, assign the mask to each of the temp polys
      {
         for(j = 0; j < tempSurfaces.size(); j++)
         {
            bool assigned = false;
            for(k = 0; k < planeGroups.size() && !assigned; k++)
            {
               for(l = 0; l < planeGroups[k].numPlanes; l++)
               {
                  if(planeGroups[k].planeIndices[l] == tempSurfaces[j].planeIndex)
                  {
                     tempSurfaces[j].mask = planeGroups[k].mask;
                     assigned = true;
                     break;
                  }
               }
            }
            AssertFatal(assigned, "Error, missed a plane somewhere in the hull poly list!");
         }
      }

      // Copy the appropriate group mask to the plane masks
      {
         planeMasks.setSize(planeIndices.size());
         dMemset(planeMasks.address(), 0, planeMasks.size() * sizeof(U8));

         for(j = 0; j < planeIndices.size(); j++)
         {
            bool found = false;
            for(k = 0; k < planeGroups.size() && !found; k++)
            {
               for(l = 0; l < planeGroups[k].numPlanes; l++)
               {
                  if(planeGroups[k].planeIndices[l] == planeIndices[j])
                  {
                     planeMasks[j] = planeGroups[k].mask;
                     found = true;
                     break;
                  }
               }
            }
            AssertFatal(planeMasks[j] != 0, "Error, missing mask for plane!");
         }
      }

      // And whip through the points, constructing the total mask for that point
      {
         pointMasks.setSize(pointIndices.size());
         dMemset(pointMasks.address(), 0, pointMasks.size() * sizeof(U8));

         for(j = 0; j < pointIndices.size(); j++)
         {
            for(k = 0; k < tempSurfaces.size(); k++)
            {
               for(l = 0; l < tempSurfaces[k].numPoints; l++)
               {
                  if(tempSurfaces[k].pointIndices[l] == j)
                  {
                     pointMasks[j] |= tempSurfaces[k].mask;
                     break;
                  }
               }
            }
            AssertFatal(pointMasks[j] != 0, "Error, point must exist in at least one surface!");
         }
      }

      // Create the emit strings, and we're done!
      {
         // Set the range of planes
         rHull.polyListPlaneStart = mVehiclePolyListPlanes.size();
         mVehiclePolyListPlanes.setSize(rHull.polyListPlaneStart + planeIndices.size());
         for(j = 0; j < planeIndices.size(); j++)
            mVehiclePolyListPlanes[j + rHull.polyListPlaneStart] = planeIndices[j];

         // Set the range of points
         rHull.polyListPointStart = mVehiclePolyListPoints.size();
         mVehiclePolyListPoints.setSize(rHull.polyListPointStart + pointIndices.size());
         for(j = 0; j < pointIndices.size(); j++)
            mVehiclePolyListPoints[j + rHull.polyListPointStart] = pointIndices[j];

         // Now the emit string.  The emit string goes like: (all fields are bytes)
         //  NumPlanes (PLMask) * NumPlanes
         //  NumPointsHi NumPointsLo (PtMask) * NumPoints
         //  NumSurfaces
         //   (NumPoints SurfaceMask PlOffset (PtOffsetHi PtOffsetLo) * NumPoints) * NumSurfaces
         //
         U32 stringLen  = 1 + planeIndices.size();
         stringLen     += 2 + pointIndices.size();
         stringLen     += 1;
         for(j = 0; j < tempSurfaces.size(); j++)
            stringLen += 1 + 1 + 1 + (tempSurfaces[j].numPoints * 2);

         rHull.polyListStringStart = mVehiclePolyListStrings.size();
         mVehiclePolyListStrings.setSize(rHull.polyListStringStart + stringLen);

         U8* pString = &mVehiclePolyListStrings[rHull.polyListStringStart];
         U32 currPos = 0;

         // Planes
         pString[currPos++] = planeIndices.size();
         for(j = 0; j < planeIndices.size(); j++)
            pString[currPos++] = planeMasks[j];

         // Points
         pString[currPos++] = (pointIndices.size() >> 8) & 0xFF;
         pString[currPos++] = (pointIndices.size() >> 0) & 0xFF;
         for(j = 0; j < pointIndices.size(); j++)
            pString[currPos++] = pointMasks[j];

         // Surfaces
         pString[currPos++] = tempSurfaces.size();
         for(j = 0; j < tempSurfaces.size(); j++)
         {
            pString[currPos++] = tempSurfaces[j].numPoints;
            pString[currPos++] = tempSurfaces[j].mask;

            bool found = false;
            for(k = 0; k < planeIndices.size(); k++)
            {
               if(planeIndices[k] == tempSurfaces[j].planeIndex)
               {
                  pString[currPos++] = k;
                  found = true;
                  break;
               }
            }
            AssertFatal(found, "Error, missing planeindex!");

            for(k = 0; k < tempSurfaces[j].numPoints; k++)
            {
               pString[currPos++] = (tempSurfaces[j].pointIndices[k] >> 8) & 0xFF;
               pString[currPos++] = (tempSurfaces[j].pointIndices[k] >> 0) & 0xFF;
            }
         }
         AssertFatal(currPos == stringLen, "Error, mismatched string length!");
      }
   } // for (i = 0; i < mConvexHulls.size(); i++)

   // Compact the used vectors
   {
      mVehiclePolyListStrings.compact();
      mVehiclePolyListPoints.compact();
      mVehiclePolyListPlanes.compact();
   }
}




//--------------------------------------------------------------------------
void ZoneVisDeterminer::runFromState(SceneRenderState* state, U32 offset, U32 parentZone)
{
   mMode            = FromState;
   mState           = state;
   mZoneRangeOffset = offset;
   mParentZone      = parentZone;
}

void ZoneVisDeterminer::runFromRects(SceneRenderState* state, U32 offset, U32 parentZone)
{
   mMode            = FromRects;
   mState           = state;
   mZoneRangeOffset = offset;
   mParentZone      = parentZone;
}

bool ZoneVisDeterminer::isZoneVisible(const U32 zone) const
{
   if(zone == 0)
      return mState->getCullingState().getZoneState(mParentZone).isZoneVisible();

   if(mMode == FromState)
   {
      return mState->getCullingState().getZoneState(zone + mZoneRangeOffset - 1).isZoneVisible();
   }
   else
   {
      return sgZoneRenderInfo[zone].render;
   }
}



//--------------------------------------------------------------------------
// storeSurfaceVerts -
// Need to store the verts for every surface because the uv mapping changes
// per vertex per surface.
//--------------------------------------------------------------------------
void Interior::storeSurfVerts( Vector<U16> &masterIndexList,
                               Vector<VertexBufferTempIndex> &tempIndexList,
                               Vector<GFXVertexPNTTB> &verts,
                               U32 numIndices,
                               Surface &surface,
                               U32 surfaceIndex )
{
   U32 startIndex = tempIndexList.size() - numIndices;
   U32 startVert = verts.size();

   Vector<U32> vertMap;

   for( U32 i=0; i<numIndices; i++ )
   {
      // check if vertex is already stored for this surface
      bool alreadyStored = false;

      for( U32 j=0; j<i; j++ )
      {
         if( tempIndexList[startIndex+i].index == tempIndexList[startIndex+j].index )
         {
            alreadyStored = true;
            break;
         }
      }

      if( alreadyStored )
      {
         for( U32 a=0; a<vertMap.size(); a++ )
         {
            // find which vertex is indexed
            if( vertMap[a] == tempIndexList[startIndex+i].index )
            {
               // store the index
               masterIndexList.push_back( startVert + a );
               break;
            }
         }
      }
      else
      {
         // store the vertex
         GFXVertexPNTTB vert;
         VertexBufferTempIndex &ind = tempIndexList[startIndex+i];
         vert.point = mPoints[ind.index].point;

         vert.normal = ind.normal;

         fillVertex( vert, surface, surfaceIndex );
         verts.push_back( vert );

         // store the index
         masterIndexList.push_back( verts.size() - 1 );

         // maintain mapping of old indices to new indices
         vertMap.push_back( ind.index );
      }

   }

}


//--------------------------------------------------------------------------
// storeRenderNode
//--------------------------------------------------------------------------
void Interior::storeRenderNode(  RenderNode &node,
                                 ZoneRNList &RNList,
                                 Vector<GFXPrimitive> &primInfoList,
                                 Vector<U16> &indexList,
                                 Vector<GFXVertexPNTTB> &verts,
                                 U32 &startIndex,
                                 U32 &startVert )
{

   GFXPrimitive pnfo;

   if( !node.matInst )
   {
      String name = mMaterialList->getMaterialName( node.baseTexIndex );

      if (!name.equal("NULL", String::NoCase) &&
          !name.equal("ORIGIN", String::NoCase) &&
          !name.equal("TRIGGER", String::NoCase) &&
          !name.equal("FORCEFIELD", String::NoCase) &&
          !name.equal("EMITTER", String::NoCase) )
      {
         Con::errorf( "material unmapped: %s", name.c_str() );
      }

      node.matInst = MATMGR->getWarningMatInstance();
   }


   // find min index
   pnfo.minIndex = U32(-1);

   for( U32 i=startIndex; i<indexList.size(); i++ )
   {
      if( indexList[i] < pnfo.minIndex )
      {
         pnfo.minIndex = indexList[i];
      }
   }

   pnfo.numPrimitives = (indexList.size() - startIndex) / 3;
   pnfo.startIndex = startIndex;
   pnfo.numVertices = verts.size() - startVert;
   pnfo.type = GFXTriangleList;
   startIndex = indexList.size();
   startVert = verts.size();

   if( pnfo.numPrimitives > 0 )
   {
      primInfoList.push_back( pnfo );

      node.primInfoIndex = primInfoList.size() - 1;
      RNList.renderNodeList.push_back( node );
   }

}

//--------------------------------------------------------------------------
// fill vertex
//--------------------------------------------------------------------------
void Interior::fillVertex( GFXVertexPNTTB &vert, Surface &surface, U32 surfaceIndex )
{
   TexGenPlanes texPlanes = mTexGenEQs[surface.texGenIndex];

   vert.texCoord.x = texPlanes.planeX.x * vert.point.x +
                     texPlanes.planeX.y * vert.point.y +
                     texPlanes.planeX.z * vert.point.z +
                     texPlanes.planeX.d;


   vert.texCoord.y = texPlanes.planeY.x * vert.point.x +
                     texPlanes.planeY.y * vert.point.y +
                     texPlanes.planeY.z * vert.point.z +
                     texPlanes.planeY.d;

   texPlanes = mLMTexGenEQs[surfaceIndex];

   vert.texCoord2.x = texPlanes.planeX.x * vert.point.x +
                      texPlanes.planeX.y * vert.point.y +
                      texPlanes.planeX.z * vert.point.z +
                      texPlanes.planeX.d;


   vert.texCoord2.y = texPlanes.planeY.x * vert.point.x +
                      texPlanes.planeY.y * vert.point.y +
                      texPlanes.planeY.z * vert.point.z +
                      texPlanes.planeY.d;

   // vert normal and N already set
   vert.T = surface.T - vert.normal * mDot(vert.normal, surface.T);
   vert.T.normalize();

   mCross(vert.normal, vert.T, &vert.B);
   vert.B *= (mDot(vert.B, surface.B) < 0.0F) ? -1.0F : 1.0F;
}

//--------------------------------------------------------------------------
// Create vertex (and index) buffers for each zone
//--------------------------------------------------------------------------
void Interior::createZoneVBs()
{
   if( mVertBuff )
   {
      return;
   }

   // create one big-ass vertex buffer to contain all verts
   // drawIndexedPrimitive() calls can render subsets of the big-ass buffer

   Vector<GFXVertexPNTTB> verts;
   Vector<U16> indices;

   U32 startIndex = 0;
   U32 startVert = 0;


   Vector<GFXPrimitive> primInfoList;


   // fill index list first, then fill verts
   for( U32 i=0; i<mZones.size(); i++ )
   {

      ZoneRNList RNList;
      RenderNode node;


      U16 curTexIndex = 0;
      U8 curLightMapIndex = U8(-1);

      Vector<VertexBufferTempIndex> tempIndices;
      tempIndices.setSize(0);



      for( U32 j=0; j<mZones[i].surfaceCount; j++ )
      {
         U32 surfaceIndex = mZoneSurfaces[mZones[i].surfaceStart + j];
         Surface& surface = mSurfaces[ surfaceIndex ];
         U32 *surfIndices = &mWindings[surface.windingStart];

		 //surface.VBIndexStart = indices.size();
		 //surface.primIndex = primInfoList.size();

         BaseMatInstance *matInst = mMaterialList->getMaterialInst( surface.textureIndex );
         Material* pMat = dynamic_cast<Material*>(matInst->getMaterial());
         if( pMat && pMat->mPlanarReflection ) continue;

         node.exterior = surface.surfaceFlags & SurfaceOutsideVisible;

         // fill in node info on first time through
         if( j==0 )
         {
            node.baseTexIndex = surface.textureIndex;
            node.matInst = matInst;
            curTexIndex = node.baseTexIndex;
            node.lightMapIndex = mNormalLMapIndices[surfaceIndex];
            curLightMapIndex = node.lightMapIndex;
         }

         // check for material change
         if( surface.textureIndex != curTexIndex ||
             mNormalLMapIndices[surfaceIndex] != curLightMapIndex )
         {
            storeRenderNode( node, RNList, primInfoList, indices, verts, startIndex, startVert );

            tempIndices.setSize( 0 );

            // set new material info
            U16 baseTex = surface.textureIndex;
            U8 lmIndex = mNormalLMapIndices[surfaceIndex];

            if( baseTex != curTexIndex )
            {
               node.baseTexIndex = baseTex;
               node.matInst = mMaterialList->getMaterialInst( baseTex );
            }
            else
            {
               node.baseTexIndex = NULL;
            }


            node.lightMapIndex = lmIndex;

            curTexIndex = baseTex;
            curLightMapIndex = lmIndex;

         }


         // NOTE, can put this in storeSurfVerts()
         U32 tempStartIndex = tempIndices.size();
			
			U32 nPrim = 0;
         U32 last = 2;
         while(last < surface.windingCount)
         {
            // First
            tempIndices.push_back( VertexBufferTempIndex(surfIndices[last-2], getPointNormal(surfaceIndex, last-2)) );
            tempIndices.push_back( VertexBufferTempIndex(surfIndices[last-1], getPointNormal(surfaceIndex, last-1)) );
            tempIndices.push_back( VertexBufferTempIndex(surfIndices[last-0], getPointNormal(surfaceIndex, last)) );
            last++;
				nPrim++;

            if(last == surface.windingCount)
               break;

            // Second
            tempIndices.push_back( VertexBufferTempIndex(surfIndices[last-1], getPointNormal(surfaceIndex, last-1)) );
            tempIndices.push_back( VertexBufferTempIndex(surfIndices[last-2], getPointNormal(surfaceIndex, last-2)) );
            tempIndices.push_back( VertexBufferTempIndex(surfIndices[last-0], getPointNormal(surfaceIndex, last)) );
            last++;
				nPrim++;
         }

			U32 dStartVert = verts.size();
			GFXPrimitive* p = &surface.surfaceInfo;
    		p->startIndex = indices.size(); //tempStartIndex;

			// Normal render info
         storeSurfVerts( indices, tempIndices, verts, tempIndices.size() - tempStartIndex,
                         surface, surfaceIndex );			

			// Debug render info
			p->type = GFXTriangleList;
			p->numVertices = verts.size() - dStartVert;
			p->numPrimitives = nPrim;
			p->minIndex = indices[p->startIndex];
			for (U32 i = p->startIndex; i < p->startIndex + nPrim * 3; i++) {
				if (indices[i] < p->minIndex) {
					p->minIndex = indices[i];
				}
			}
		}

      // store remaining index list
      storeRenderNode( node, RNList, primInfoList, indices, verts, startIndex, startVert );

      mZoneRNList.push_back( RNList );
   }

   // It is possible that we have no zones or have no surfaces in our zones (static meshes only)
   if (verts.size() == 0)
      return;

   // create vertex buffer
   mVertBuff.set(GFX, verts.size(), GFXBufferTypeStatic);
   GFXVertexPNTTB *vbVerts = mVertBuff.lock();

   dMemcpy( vbVerts, verts.address(), verts.size() * sizeof( GFXVertexPNTTB ) );

   mVertBuff.unlock();

   // create primitive buffer
   U16 *ibIndices;
   GFXPrimitive *piInput;
   mPrimBuff.set(GFX, indices.size(), primInfoList.size(), GFXBufferTypeStatic);
   mPrimBuff.lock(&ibIndices, &piInput);

   dMemcpy( ibIndices, indices.address(), indices.size() * sizeof(U16) );
   dMemcpy( piInput, primInfoList.address(), primInfoList.size() * sizeof(GFXPrimitive) );

   mPrimBuff.unlock();

}


#define SMALL_FLOAT (1e-12)

//--------------------------------------------------------------------------
// Get the texture space matrice for a point on a surface
//--------------------------------------------------------------------------
void Interior::getTexMat(U32 surfaceIndex, U32 pointOffset, Point3F& T, Point3F& N, Point3F& B)
{
   Surface& surface = mSurfaces[surfaceIndex];

   if (mFileVersion >= 11)
   {
      // There is a one-to-one mapping of mWindings and mTexMatIndices
      U32 texMatIndex = mTexMatIndices[surface.windingStart + pointOffset];
      TexMatrix& texMat = mTexMatrices[texMatIndex];

      T = mNormals[texMat.T];
      N = mNormals[texMat.N];
      B = mNormals[texMat.B];
   }
   else
   {
      T = surface.T - surface.N * mDot(surface.N, surface.T);
      N = surface.N;

      mCross(surface.N, T, &B);
      B *= (mDot(B, surface.B) < 0.0F) ? -1.0f : 1.0f;
   }

   return;
}

//--------------------------------------------------------------------------
// Fill in texture space matrices for each surface
//--------------------------------------------------------------------------
void Interior::fillSurfaceTexMats()
{
   for( U32 i=0; i<mSurfaces.size(); i++ )
   {
      Surface &surface = mSurfaces[i];

      const PlaneF & plane = getPlane(surface.planeIndex);

      Point3F planeNorm = plane;
      if( planeIsFlipped( surface.planeIndex ) )
      {
         planeNorm = -planeNorm;
      }

      GFXVertexPNTTB pts[3];
      pts[0].point = mPoints[mWindings[surface.windingStart + 1]].point;
      pts[1].point = mPoints[mWindings[surface.windingStart + 0]].point;
      pts[2].point = mPoints[mWindings[surface.windingStart + 2]].point;

      TexGenPlanes texPlanes = mTexGenEQs[surface.texGenIndex];


      for( U32 j=0; j<3; j++ )
      {
         pts[j].texCoord.x = texPlanes.planeX.x * pts[j].point.x +
                             texPlanes.planeX.y * pts[j].point.y +
                             texPlanes.planeX.z * pts[j].point.z +
                             texPlanes.planeX.d;


         pts[j].texCoord.y = texPlanes.planeY.x * pts[j].point.x +
                             texPlanes.planeY.y * pts[j].point.y +
                             texPlanes.planeY.z * pts[j].point.z +
                             texPlanes.planeY.d;

      }


      Point3F edge1, edge2;
      Point3F cp;
      Point3F S,T,SxT;

      // x, s, t
      edge1.set( pts[1].point.x - pts[0].point.x, pts[1].texCoord.x - pts[0].texCoord.x, pts[1].texCoord.y - pts[0].texCoord.y );
      edge2.set( pts[2].point.x - pts[0].point.x, pts[2].texCoord.x - pts[0].texCoord.x, pts[2].texCoord.y - pts[0].texCoord.y );

      mCross( edge1, edge2, &cp );
      if( fabs(cp.x) > SMALL_FLOAT )
      {
         S.x = -cp.y / cp.x;
         T.x = -cp.z / cp.x;
      }

      edge1.set( pts[1].point.y - pts[0].point.y, pts[1].texCoord.x - pts[0].texCoord.x, pts[1].texCoord.y - pts[0].texCoord.y );
      edge2.set( pts[2].point.y - pts[0].point.y, pts[2].texCoord.x - pts[0].texCoord.x, pts[2].texCoord.y - pts[0].texCoord.y );

      mCross( edge1, edge2, &cp );
      if( fabs(cp.x) > SMALL_FLOAT )
      {
         S.y = -cp.y / cp.x;
         T.y = -cp.z / cp.x;
      }

      edge1.set( pts[1].point.z - pts[0].point.z, pts[1].texCoord.x - pts[0].texCoord.x, pts[1].texCoord.y - pts[0].texCoord.y );
      edge2.set( pts[2].point.z - pts[0].point.z, pts[2].texCoord.x - pts[0].texCoord.x, pts[2].texCoord.y - pts[0].texCoord.y );

      mCross( edge1, edge2, &cp );
      if( fabs(cp.x) > SMALL_FLOAT )
      {
         S.z = -cp.y / cp.x;
         T.z = -cp.z / cp.x;
      }

      S.normalizeSafe();
      T.normalizeSafe();
      mCross( S, T, &SxT );


      if( mDot( SxT, planeNorm ) < 0.0 )
      {
         SxT = -SxT;
      }

      surface.T = S;
      surface.B = T;
      surface.N = SxT;
      surface.normal = planeNorm;
   }

}

//--------------------------------------------------------------------------
// Clone material instances - if a texture (material) exists on both the
// inside and outside of an interior, it needs to create two material
// instances - one for the inside, and one for the outside.  The reason is
// that the light direction maps only exist on the inside of the interior.
//--------------------------------------------------------------------------
void Interior::cloneMatInstances()
{
   Vector< BaseMatInstance *> outsideMats;
   Vector< BaseMatInstance *> insideMats;

   // store pointers to mat lists
   for( U32 i=0; i<getNumZones(); i++ )
   {
      for( U32 j=0; j<mZoneRNList[i].renderNodeList.size(); j++ )
      {
         RenderNode &node = mZoneRNList[i].renderNodeList[j];

         if( !node.matInst ) continue;

         if( node.exterior )
         {
            // insert only if it's not already there
            U32 k;
            for( k=0; k<outsideMats.size(); k++ )
            {
               if( node.matInst == outsideMats[k] ) break;
            }
            if( k == outsideMats.size() )
            {
               outsideMats.push_back( node.matInst );
            }
         }
         else
         {
            // insert only if it's not already there
            U32 k;
            for( k=0; k<insideMats.size(); k++ )
            {
               if( node.matInst == insideMats[k] ) break;
            }
            if( k == insideMats.size() )
            {
               insideMats.push_back( node.matInst );
            }
         }
      }
   }

   // for all materials that exist both inside and outside,
   // clone them so they can have separate material instances
   for( U32 i=0; i<outsideMats.size(); i++ )
   {
      for( U32 j=0; j<insideMats.size(); j++ )
      {
         if( outsideMats[i] == insideMats[j] )
         {
            // GFX2_RENDER_MERGE
            Material *mat = dynamic_cast<Material*>(outsideMats[i]->getMaterial());

            if (mat)
            {
               BaseMatInstance *newMat = mat->createMatInstance();
               mMatInstCleanupList.push_back( newMat );

               // go through and find the inside version and replace it
               // with the new one.
               for( U32 k=0; k<getNumZones(); k++ )
               {
                  for( U32 l=0; l<mZoneRNList[k].renderNodeList.size(); l++ )
                  {
                     RenderNode &node = mZoneRNList[k].renderNodeList[l];
                     if( !node.exterior )
                     {
                        if( node.matInst == outsideMats[i] )
                        {
                           node.matInst = newMat;
                        }
                     }
                  }
               }
            }
         }

      }
   }
}

//--------------------------------------------------------------------------
// Intialize material instances
//--------------------------------------------------------------------------
void Interior::initMatInstances()
{
   mHasTranslucentMaterials = false;

   for( U32 i=0; i<getNumZones(); i++ )
   {
      for( U32 j=0; j<mZoneRNList[i].renderNodeList.size(); j++ )
      {
         RenderNode &node = mZoneRNList[i].renderNodeList[j];

         BaseMatInstance *mat = node.matInst;
         if( mat )
         {
            // Note: We disabled this as it was keeping the prepass lighting
            // from being applied to interiors... this fixes that.
            //fd.features[MFT_RTLighting] = false;

            // If this node has a lightmap index,
            // we need to bind the override features delegate
            // in order to ensure the MFT_LightMap feature
            // is added for the material.    
            if ( node.lightMapIndex != (U8)-1 )
               mat->getFeaturesDelegate().bind( &Interior::_enableLightMapFeature );
            mat->init( MATMGR->getDefaultFeatures(), getGFXVertexFormat<GFXVertexPNTTB>());

            // We need to know if we have non-zwrite translucent materials 
            // so that we can make the extra renderimage pass for them.
            Material* pMat = dynamic_cast<Material*>(mat->getMaterial());
            if ( pMat )
               mHasTranslucentMaterials |= pMat->mTranslucent && !pMat->mTranslucentZWrite;
         }

      }

      for( U32 j=0; j<mZoneReflectRNList[i].reflectList.size(); j++ )
      {
         ReflectRenderNode &node = mZoneReflectRNList[i].reflectList[j];

         BaseMatInstance *mat = node.matInst;
         if( mat )
         {
            // Note: We disabled this as it was keeping the prepass lighting
            // from being applied to interiors... this fixes that.
            //fd.features[MFT_RTLighting] = false;

            mat->init( MATMGR->getDefaultFeatures(), getGFXVertexFormat<GFXVertexPNTTB>());

            // We need to know if we have non-zwrite translucent materials 
            // so that we can make the extra renderimage pass for them.
            Material* pMat = dynamic_cast<Material*>(mat->getMaterial());
            if ( pMat )
               mHasTranslucentMaterials |= pMat->mTranslucent && !pMat->mTranslucentZWrite;
         }
      }
   }
}

void Interior::_enableLightMapFeature( ProcessedMaterial *mat,
                                       U32 stageNum,
                                       MaterialFeatureData &fd, 
                                       const FeatureSet &features )
{
   if ( mat->getMaterial() )
   {
      fd.features.addFeature( MFT_LightMap );
      fd.features.removeFeature( MFT_ToneMap );
   }
}

//--------------------------------------------------------------------------
// Create the reflect plane list and the nodes of geometry necessary to
// render the reflective surfaces.
//--------------------------------------------------------------------------
void Interior::createReflectPlanes()
{
   Vector<GFXVertexPNTTB> verts;
   Vector<GFXPrimitive>    primInfoList;
   Vector<U16>             indices;

   U32 startIndex = 0;
   U32 startVert = 0;



   for( U32 i=0; i<mZones.size(); i++ )
   {

      ZoneReflectRNList reflectRNList;

      // for each zone:
      // go through list of surfaces, searching for reflection

      for( U32 j=0; j<mZones[i].surfaceCount; j++ )
      {
         U32 surfaceIndex = mZoneSurfaces[mZones[i].surfaceStart + j];
         Surface& surface = mSurfaces[ surfaceIndex ];

         BaseMatInstance *matInst = mMaterialList->getMaterialInst( surface.textureIndex );
         Material* pMat = dynamic_cast<Material*>(matInst->getMaterial());         
         if( !pMat || !pMat->mPlanarReflection ) continue;

         U32 *surfIndices = &mWindings[surface.windingStart];

         // create / fill in GFXPrimitve, verts, indices
         // going to need a new render node
         ReflectRenderNode node;
         node.exterior = surface.surfaceFlags & SurfaceOutsideVisible;
         node.matInst = mMaterialList->getMaterialInst( surface.textureIndex );
         node.lightMapIndex = mNormalLMapIndices[surfaceIndex];


         PlaneF plane;
         plane = getPlane( surface.planeIndex );
         if( planeIsFlipped( surface.planeIndex ) )
         {
            plane.x = -plane.x;
            plane.y = -plane.y;
            plane.z = -plane.z;
            plane.d = -plane.d;
         }
         
         // check if coplanar with existing reflect plane
         //--------------------------------------------------
         S32 rPlaneIdx = -1;
         for( U32 a=0; a<mReflectPlanes.size(); a++ )
         {
            if( fabs( mDot( plane, mReflectPlanes[a] ) ) > 0.999 )
            {
               if( fabs( plane.d - mReflectPlanes[a].d ) < 0.001 )
               {
                  rPlaneIdx = a;
                  break;
               }
            }
         }

         PlaneF refPlane;
         refPlane = plane;

         if( rPlaneIdx < 0 )
         {
            mReflectPlanes.push_back( refPlane );
            node.reflectPlaneIndex = mReflectPlanes.size() - 1;
         }
         else
         {
            node.reflectPlaneIndex = rPlaneIdx;
         }

         // store the indices for the surface
         //--------------------------------------------------
         Vector<VertexBufferTempIndex> tempIndices;
         tempIndices.setSize( 0 );

         U32 tempStartIndex = tempIndices.size();

         U32 last = 2;
         while(last < surface.windingCount)
         {
            // First
            tempIndices.push_back( VertexBufferTempIndex(surfIndices[last-2], getPointNormal(surfaceIndex, last-2)) );
            tempIndices.push_back( VertexBufferTempIndex(surfIndices[last-1], getPointNormal(surfaceIndex, last-1)) );
            tempIndices.push_back( VertexBufferTempIndex(surfIndices[last-0], getPointNormal(surfaceIndex, last)) );
            last++;

            if(last == surface.windingCount)
               break;

            // Second
            tempIndices.push_back( VertexBufferTempIndex(surfIndices[last-1], getPointNormal(surfaceIndex, last-1)) );
            tempIndices.push_back( VertexBufferTempIndex(surfIndices[last-2], getPointNormal(surfaceIndex, last-2)) );
            tempIndices.push_back( VertexBufferTempIndex(surfIndices[last-0], getPointNormal(surfaceIndex, last)) );
            last++;
         }
         
         storeSurfVerts( indices, tempIndices, verts, tempIndices.size() - tempStartIndex,
                         surface, surfaceIndex );


         // store render node and GFXPrimitive
         // each node is a different reflective surface
         // ---------------------------------------------------

         // find min index
         GFXPrimitive pnfo;
         pnfo.minIndex = U32(-1);
         for( U32 k=startIndex; k<indices.size(); k++ )
         {
            if( indices[k] < pnfo.minIndex )
            {
               pnfo.minIndex = indices[k];
            }
         }

         pnfo.numPrimitives = (indices.size() - startIndex) / 3;
         pnfo.startIndex = startIndex;
         pnfo.numVertices = verts.size() - startVert;
         pnfo.type = GFXTriangleList;
         startIndex = indices.size();
         startVert = verts.size();

         primInfoList.push_back( pnfo );
         node.primInfoIndex = primInfoList.size() - 1;
         reflectRNList.reflectList.push_back( node );
      }

      mZoneReflectRNList.push_back( reflectRNList );
   }

   if( mReflectPlanes.size() )
   {
      // copy verts to buffer
      mReflectVertBuff.set(GFX, verts.size(), GFXBufferTypeStatic);
      GFXVertexPNTTB *vbVerts = mReflectVertBuff.lock();
      dMemcpy( vbVerts, verts.address(), verts.size() * sizeof( GFXVertexPNTTB ) );
      mReflectVertBuff.unlock();

      // create primitive buffer
      U16 *ibIndices;
      GFXPrimitive *piInput;
      mReflectPrimBuff.set(GFX, indices.size(), primInfoList.size(), GFXBufferTypeStatic);
      mReflectPrimBuff.lock(&ibIndices, &piInput);
      dMemcpy( ibIndices, indices.address(), indices.size() * sizeof(U16) );
      dMemcpy( piInput, primInfoList.address(), primInfoList.size() * sizeof(GFXPrimitive) );
      mReflectPrimBuff.unlock();
   }


}

void Interior::buildSurfaceZones()
{
	surfaceZones.clear();
	surfaceZones.setSize(mSurfaces.size());

	for(U32 i=0; i<getSurfaceCount(); i++)
	{
		surfaceZones[i] = -1;
	}

	for(U32 z=0; z<mZones.size(); z++)
	{
		Interior::Zone &zone = mZones[z];
		zone.zoneId = z;
		for(U32 s=zone.surfaceStart; s<(zone.surfaceStart + zone.surfaceCount); s++)
		{
			surfaceZones[mZoneSurfaces[s]] = zone.zoneId - 1;
		}
	}
}

const String& Interior::getTargetName( S32 mapToNameIndex ) const
{
	S32 targetCount = mMaterialList->getMaterialNameList().size();

	if(mapToNameIndex < 0 || mapToNameIndex >= targetCount)
		return String::EmptyString;

	return mMaterialList->getMaterialNameList()[mapToNameIndex];
}

S32 Interior::getTargetCount() const
{
	if(!this)
		return -1;

	return mMaterialList->getMaterialNameList().size();

}
