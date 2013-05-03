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
#include "lighting/common/blobShadow.h"

#include "gfx/primBuilder.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/bitmap/gBitmap.h"
#include "math/mathUtils.h"
#include "lighting/lightInfo.h"
#include "lighting/lightingInterfaces.h"
#include "T3D/shapeBase.h"
#include "scene/sceneManager.h"
#include "lighting/lightManager.h"
#include "ts/tsMesh.h"

DepthSortList BlobShadow::smDepthSortList;
GFXTexHandle BlobShadow::smGenericShadowTexture = NULL;
S32 BlobShadow::smGenericShadowDim = 32;
U32 BlobShadow::smShadowMask = TerrainObjectType;
F32 BlobShadow::smGenericRadiusSkew = 0.4f; // shrink radius of shape when it always uses generic shadow...

Box3F gBlobShadowBox;
SphereF gBlobShadowSphere;
Point3F gBlobShadowPoly[4];

//--------------------------------------------------------------

BlobShadow::BlobShadow(SceneObject* parentObject, LightInfo* light, TSShapeInstance* shapeInstance)
{
   mParentObject = parentObject;
   mShapeBase = dynamic_cast<ShapeBase*>(parentObject);
   mParentLight = light;
   mShapeInstance = shapeInstance;
   mRadius = 0.0f;
   mLastRenderTime = 0;
   mDepthBias = -0.0002f;  

   generateGenericShadowBitmap(smGenericShadowDim);
   setupStateBlocks();
}

void BlobShadow::setupStateBlocks()
{
   GFXStateBlockDesc sh;
   sh.cullDefined = true;
   sh.cullMode = GFXCullNone;
   sh.zDefined = true;
   sh.zEnable = true;
   sh.zWriteEnable = false;
   
   sh.zBias = mDepthBias; 
   sh.blendDefined = true;
   sh.blendEnable = true;
   sh.blendSrc = GFXBlendSrcAlpha;
   sh.blendDest = GFXBlendInvSrcAlpha;
   sh.alphaDefined = true;
   sh.alphaTestEnable = true;
   sh.alphaTestFunc = GFXCmpGreater;
   sh.alphaTestRef = 0;
   sh.samplersDefined = true;
   sh.samplers[0] = GFXSamplerStateDesc::getClampLinear();
   mShadowSB = GFX->createStateBlock(sh);
}

BlobShadow::~BlobShadow()
{
   mShadowBuffer = NULL;
}

bool BlobShadow::shouldRender(F32 camDist)
{
   Point3F lightDir;

   if (mShapeBase && mShapeBase->getFadeVal() < TSMesh::VISIBILITY_EPSILON)
      return false;

   F32 shadowLen = 10.0f * mShapeInstance->getShape()->radius;
   Point3F pos = mShapeInstance->getShape()->center;

   // this is a bit of a hack...move generic shadows towards feet/base of shape
   pos *= 0.5f;
   pos.convolve(mParentObject->getScale());
   mParentObject->getRenderTransform().mulP(pos);
   if(mParentLight->getType() == LightInfo::Vector)
   {
      lightDir = mParentLight->getDirection();
   }
   else
   {
      lightDir = pos - mParentLight->getPosition();
      lightDir.normalize();
   }

   // pos is where shadow will be centered (in world space)
   setRadius(mShapeInstance, mParentObject->getScale());
   bool render = prepare(pos, lightDir, shadowLen);
   return render;
}

void BlobShadow::generateGenericShadowBitmap(S32 dim)
{
   if(smGenericShadowTexture)
      return;
   GBitmap * bitmap = new GBitmap(dim,dim,false,GFXFormatR8G8B8A8);
   U8 * bits = bitmap->getWritableBits();
   dMemset(bits, 0, dim*dim*4);
   S32 center = dim >> 1;
   F32 invRadiusSq = 1.0f / (F32)(center*center);
   F32 tmpF;
   for (S32 i=0; i<dim; i++)
   {
      for (S32 j=0; j<dim; j++)
      {
         tmpF = (F32)((i-center)*(i-center)+(j-center)*(j-center)) * invRadiusSq;
         U8 val = tmpF>0.99f ? 0 : (U8)(180.0f*(1.0f-tmpF)); // 180 out of 255 max
         bits[(i*dim*4)+(j*4)+3] = val;
      }
   }

   smGenericShadowTexture.set( bitmap, &GFXDefaultStaticDiffuseProfile, true, "BlobShadow" );
}

//--------------------------------------------------------------

void BlobShadow::setLightMatrices(const Point3F & lightDir, const Point3F & pos)
{
   AssertFatal(mDot(lightDir,lightDir)>0.0001f,"BlobShadow::setLightDir: light direction must be a non-zero vector.");

   // construct light matrix
   Point3F x,z;
   if (mFabs(lightDir.z)>0.001f)
   {
      // mCross(Point3F(1,0,0),lightDir,&z);
      z.x = 0.0f;
      z.y =  lightDir.z;
      z.z = -lightDir.y;
      z.normalize();
      mCross(lightDir,z,&x);
   }
   else
   {
      mCross(lightDir,Point3F(0,0,1),&x);
      x.normalize();
      mCross(x,lightDir,&z);
   }

   mLightToWorld.identity();
   mLightToWorld.setColumn(0,x);
   mLightToWorld.setColumn(1,lightDir);
   mLightToWorld.setColumn(2,z);
   mLightToWorld.setColumn(3,pos);

   mWorldToLight = mLightToWorld;
   mWorldToLight.inverse();
}

void BlobShadow::setRadius(F32 radius)
{
   mRadius = radius;
}

void BlobShadow::setRadius(TSShapeInstance * shapeInstance, const Point3F & scale)
{
   const Box3F & bounds = shapeInstance->getShape()->bounds;
   F32 dx = 0.5f * (bounds.maxExtents.x-bounds.minExtents.x) * scale.x;
   F32 dy = 0.5f * (bounds.maxExtents.y-bounds.minExtents.y) * scale.y;
   F32 dz = 0.5f * (bounds.maxExtents.z-bounds.minExtents.z) * scale.z;
   mRadius = mSqrt(dx*dx+dy*dy+dz*dz);
}


//--------------------------------------------------------------

bool BlobShadow::prepare(const Point3F & pos, Point3F lightDir, F32 shadowLen)
{
   if (mPartition.empty())
   {
      // --------------------------------------
      // 1.
      F32 dirMult = (1.0f) * (1.0f);
      if (dirMult < 0.99f)
      {
         lightDir.z *= dirMult;
         lightDir.z -= 1.0f - dirMult;
      }
      lightDir.normalize();
      shadowLen *= (1.0f) * (1.0f);

      // --------------------------------------
      // 2. get polys
      F32 radius = mRadius;
      radius *= smGenericRadiusSkew;
      buildPartition(pos,lightDir,radius,shadowLen);
   }
   if (mPartition.empty())
      // no need to draw shadow if nothing to cast it onto
      return false;

   return true;
}

//--------------------------------------------------------------

void BlobShadow::buildPartition(const Point3F & p, const Point3F & lightDir, F32 radius, F32 shadowLen)
{
   setLightMatrices(lightDir,p);

   Point3F extent(2.0f*radius,shadowLen,2.0f*radius);
   smDepthSortList.clear();
   smDepthSortList.set(mWorldToLight,extent);
   smDepthSortList.setInterestNormal(lightDir);

   if (shadowLen<1.0f)
      // no point in even this short of a shadow...
      shadowLen = 1.0f;
   mInvShadowDistance = 1.0f / shadowLen;

   // build world space box and sphere around shadow

   Point3F x,y,z;
   mLightToWorld.getColumn(0,&x);
   mLightToWorld.getColumn(1,&y);
   mLightToWorld.getColumn(2,&z);
   x *= radius;
   y *= shadowLen;
   z *= radius;
   gBlobShadowBox.maxExtents.set(mFabs(x.x)+mFabs(y.x)+mFabs(z.x),
      mFabs(x.y)+mFabs(y.y)+mFabs(z.y),
      mFabs(x.z)+mFabs(y.z)+mFabs(z.z));
   y *= 0.5f;
   gBlobShadowSphere.radius = gBlobShadowBox.maxExtents.len();
   gBlobShadowSphere.center = p + y;
   gBlobShadowBox.minExtents  = y + p - gBlobShadowBox.maxExtents;
   gBlobShadowBox.maxExtents += y + p;

   // get polys

   gClientContainer.findObjects(STATIC_COLLISION_TYPEMASK, BlobShadow::collisionCallback, this);

   // setup partition list
   gBlobShadowPoly[0].set(-radius,0,-radius);
   gBlobShadowPoly[1].set(-radius,0, radius);
   gBlobShadowPoly[2].set( radius,0, radius);
   gBlobShadowPoly[3].set( radius,0,-radius);

   mPartition.clear();
   mPartitionVerts.clear();
   smDepthSortList.depthPartition(gBlobShadowPoly,4,mPartition,mPartitionVerts);

   if(mPartitionVerts.empty())
      return;

   // Find the rough distance of the shadow verts
   // from the object position and use that to scale
   // the visibleAlpha so that the shadow fades out
   // the further away from you it gets
   F32 dist = 0.0f;

   // Calculate the center of the partition verts
   Point3F shadowCenter(0.0f, 0.0f, 0.0f);
   for (U32 i = 0; i < mPartitionVerts.size(); i++)
      shadowCenter += mPartitionVerts[i];

   shadowCenter /= mPartitionVerts.size();

   mLightToWorld.mulP(shadowCenter);

   dist = (p - shadowCenter).len();

   // now set up tverts & colors
   mShadowBuffer.set(GFX, mPartitionVerts.size(), GFXBufferTypeVolatile);
   mShadowBuffer.lock();

   F32 visibleAlpha = 255.0f;
   if (mShapeBase && mShapeBase->getFadeVal())
      visibleAlpha = mClampF(255.0f * mShapeBase->getFadeVal(), 0, 255);
   visibleAlpha *= 1.0f - (dist / gBlobShadowSphere.radius);
   F32 invRadius = 1.0f / radius;
   for (S32 i=0; i<mPartitionVerts.size(); i++)
   {
      Point3F vert = mPartitionVerts[i];
      mShadowBuffer[i].point.set(vert);
      mShadowBuffer[i].color.set(255, 255, 255, visibleAlpha);
      mShadowBuffer[i].texCoord.set(0.5f + 0.5f * mPartitionVerts[i].x * invRadius, 0.5f + 0.5f * mPartitionVerts[i].z * invRadius);
   };

   mShadowBuffer.unlock();
}

//--------------------------------------------------------------

void BlobShadow::collisionCallback(SceneObject * obj, void* thisPtr)
{
   if (obj->getWorldBox().isOverlapped(gBlobShadowBox))
   {
      // only interiors clip...
      ClippedPolyList::allowClipping = (obj->getTypeMask() & LIGHTMGR->getSceneLightingInterface()->mClippingMask) != 0;
      obj->buildPolyList(PLC_Collision,&smDepthSortList,gBlobShadowBox,gBlobShadowSphere);
      ClippedPolyList::allowClipping = true;
   }
}

//--------------------------------------------------------------

void BlobShadow::render( F32 camDist, const TSRenderState &rdata )
{
   mLastRenderTime = Platform::getRealMilliseconds();
   GFX->pushWorldMatrix();
   MatrixF world = GFX->getWorldMatrix();
   world.mul(mLightToWorld);
   GFX->setWorldMatrix(world);

   GFX->disableShaders();

   GFX->setStateBlock(mShadowSB);
   GFX->setTexture(0, smGenericShadowTexture);
   GFX->setVertexBuffer(mShadowBuffer);

   for(U32 p=0; p<mPartition.size(); p++)
      GFX->drawPrimitive(GFXTriangleFan, mPartition[p].vertexStart, (mPartition[p].vertexCount - 2));

   // This is a bad nasty hack which forces the shadow to reconstruct itself every frame.
   mPartition.clear();

   GFX->popWorldMatrix();
}

void BlobShadow::deleteGenericShadowBitmap()
{
   smGenericShadowTexture = NULL;
}
