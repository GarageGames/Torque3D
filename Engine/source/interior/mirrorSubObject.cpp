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

#include "interior/mirrorSubObject.h"
#include "interior/interiorInstance.h"
#include "interior/interior.h"
#include "materials/materialList.h"
#include "core/stream/stream.h"
#include "scene/sgUtil.h"

IMPLEMENT_CONOBJECT(MirrorSubObject);

ConsoleDocClass( MirrorSubObject,
   "@deprecated Dysfunctional"
   "@internal"
);

//--------------------------------------------------------------------------
MirrorSubObject::MirrorSubObject()
{
   mTypeMask = StaticObjectType;

   mInitialized = false;
   mWhite       = NULL;
}

MirrorSubObject::~MirrorSubObject()
{
   delete mWhite;
   mWhite = NULL;
}

//--------------------------------------------------------------------------
void MirrorSubObject::initPersistFields()
{
   Parent::initPersistFields();

   //
}

//--------------------------------------------------------------------------
/*
void MirrorSubObject::renderObject(SceneRenderState* state, SceneRenderImage* image)
{
}
*/

//--------------------------------------------------------------------------
void MirrorSubObject::transformModelview(const U32 portalIndex, const MatrixF& oldMV, MatrixF* pNewMV)
{
   AssertFatal(isInitialized() == true, "Error, we should have been initialized by this point!");
   AssertFatal(portalIndex == 0, "Error, we only have one portal!");

   *pNewMV = oldMV;
   pNewMV->mul(mReflectionMatrix);
}


//--------------------------------------------------------------------------
void MirrorSubObject::transformPosition(const U32 portalIndex, Point3F& ioPosition)
{
   AssertFatal(isInitialized() == true, "Error, we should have been initialized by this point!");
   AssertFatal(portalIndex == 0, "Error, we only have one portal!");

   mReflectionMatrix.mulP(ioPosition);
}


//--------------------------------------------------------------------------
bool MirrorSubObject::computeNewFrustum(const U32      portalIndex,
                                        const Frustum  &oldFrustum,
                                        const F64      nearPlane,
                                        const F64      farPlane,
                                        const RectI&   oldViewport,
                                        F64            *newFrustum,
                                        RectI&         newViewport,
                                        const bool     flippedMatrix)
{
   AssertFatal(isInitialized() == true, "Error, we should have been initialized by this point!");
   AssertFatal(portalIndex == 0, "Error, mirrortests only have one portal!");

   Interior* interior = getInstance()->getDetailLevel(mDetailLevel);
   
   static Vector<SGWinding> mirrorWindings;
   mirrorWindings.setSize(surfaceCount);

   for (U32 i = 0; i < surfaceCount; i++) {
      SGWinding& rSGWinding             = mirrorWindings[i];
      const Interior::Surface& rSurface = interior->mSurfaces[surfaceStart + i];

      U32 fanIndices[32];
      U32 numFanIndices = 0;
      interior->collisionFanFromSurface(rSurface, fanIndices, &numFanIndices);

      for (U32 j = 0; j < numFanIndices; j++)
         rSGWinding.points[j] = interior->mPoints[fanIndices[j]].point;
      rSGWinding.numPoints = numFanIndices;
   }

   MatrixF finalModelView;
   finalModelView.mul(getSOTransform());
   finalModelView.scale(getSOScale());

   return sgComputeNewFrustum(oldFrustum, nearPlane, farPlane,
                              oldViewport,
                              mirrorWindings.address(), mirrorWindings.size(),
                              finalModelView,
                              newFrustum, newViewport,
                              flippedMatrix);
}


//--------------------------------------------------------------------------
void MirrorSubObject::openPortal(const U32   portalIndex,
                                 SceneRenderState* pCurrState,
                                 SceneRenderState* pParentState)
{

}


//--------------------------------------------------------------------------
void MirrorSubObject::closePortal(const U32   portalIndex,
                                  SceneRenderState* pCurrState,
                                  SceneRenderState* pParentState)
{
}


//--------------------------------------------------------------------------
void MirrorSubObject::getWSPortalPlane(const U32 portalIndex, PlaneF* pPlane)
{
   AssertFatal(portalIndex == 0, "Error, mirrortests only have one portal!");

   Interior* interior = getInstance()->getDetailLevel(mDetailLevel);
   const Interior::Surface& rSurface = interior->mSurfaces[surfaceStart];

   PlaneF temp = interior->getPlane(rSurface.planeIndex);
   if (Interior::planeIsFlipped(rSurface.planeIndex))
      temp.neg();

   mTransformPlane(getSOTransform(), getSOScale(), temp, pPlane);
}


//--------------------------------------------------------------------------
U32 MirrorSubObject::getSubObjectKey() const
{
   return InteriorSubObject::MirrorSubObjectKey;
}


bool MirrorSubObject::_readISO(Stream& stream)
{
   AssertFatal(isInitialized() == false, "Error, should not be initialized here!");

   if (Parent::_readISO(stream) == false)
      return false;

   stream.read(&mDetailLevel);
   stream.read(&mZone);
   stream.read(&mAlphaLevel);
   stream.read(&surfaceCount);
   stream.read(&surfaceStart);

   stream.read(&mCentroid.x);
   stream.read(&mCentroid.y);
   stream.read(&mCentroid.z);

   return true;
}


bool MirrorSubObject::_writeISO(Stream& stream) const
{
   if (Parent::_writeISO(stream) == false)
      return false;

   stream.write(mDetailLevel);
   stream.write(mZone);
   stream.write(mAlphaLevel);
   stream.write(surfaceCount);
   stream.write(surfaceStart);

   stream.write(mCentroid.x);
   stream.write(mCentroid.y);
   stream.write(mCentroid.z);

   return true;
}

bool MirrorSubObject::renderDetailDependant() const
{
   return true;
}


U32 MirrorSubObject::getZone() const
{
   return mZone;
}


void MirrorSubObject::setupTransforms()
{
   mInitialized = true;

   // This is really bad, but it's just about the only good place for this...
   if (getInstance()->isClientObject() && mWhite == NULL)
      mWhite = new GFXTexHandle("special/whiteAlpha0", &GFXDefaultStaticDiffuseProfile, avar("%s() - mWhite (line %d)", __FUNCTION__, __LINE__));

   Interior* interior = getInstance()->getDetailLevel(mDetailLevel);
   const Interior::Surface& rSurface = interior->mSurfaces[surfaceStart];

   PlaneF plane = interior->getPlane(rSurface.planeIndex);
   if (Interior::planeIsFlipped(rSurface.planeIndex))
      plane.neg();

   Point3F n(plane.x, plane.y, plane.z);
   Point3F q = n;
   q *= -plane.d;

   MatrixF t(true);
   t.scale(getSOScale());
   t.mul(getSOTransform());

   t.mulV(n);
   t.mulP(q);

   F32* ra = mReflectionMatrix;

   ra[0]  = 1.0f - 2.0f*(n.x*n.x); ra[1]  = 0.0f - 2.0f*(n.x*n.y); ra[2]  = 0.0f - 2.0f*(n.x*n.z); ra[3]  = 0.0f;
   ra[4]  = 0.0f - 2.0f*(n.y*n.x); ra[5]  = 1.0f - 2.0f*(n.y*n.y); ra[6]  = 0.0f - 2.0f*(n.y*n.z); ra[7]  = 0.0f;
   ra[8]  = 0.0f - 2.0f*(n.z*n.x); ra[9]  = 0.0f - 2.0f*(n.z*n.y); ra[10] = 1.0f - 2.0f*(n.z*n.z); ra[11] = 0.0f;

   Point3F qnn = n * mDot(n, q);

   ra[12] = qnn.x * 2.0f;
   ra[13] = qnn.y * 2.0f;
   ra[14] = qnn.z * 2.0f;
   ra[15] = 1.0f;

   // Now, the GGems series (as of v1) uses row vectors (arg)
   mReflectionMatrix.transpose();
}

void MirrorSubObject::noteTransformChange()
{
   setupTransforms();
   Parent::noteTransformChange();
}

InteriorSubObject* MirrorSubObject::clone(InteriorInstance* instance) const
{
   MirrorSubObject* pClone = new MirrorSubObject;

   pClone->mDetailLevel = mDetailLevel;
   pClone->mZone        = mZone;
   pClone->mAlphaLevel  = mAlphaLevel;
   pClone->mCentroid    = mCentroid;
   pClone->surfaceCount = surfaceCount;
   pClone->surfaceStart = surfaceStart;

   pClone->mInteriorInstance = instance;

   return pClone;
}
