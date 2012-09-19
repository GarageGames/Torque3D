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

#ifndef _BLOBSHADOW_H_
#define _BLOBSHADOW_H_

#include "collision/depthSortList.h"
#include "scene/sceneObject.h"
#include "ts/tsShapeInstance.h"
#include "lighting/common/shadowBase.h"

class ShapeBase;
class LightInfo;

class BlobShadow : public ShadowBase
{
   F32 mRadius;
   F32 mInvShadowDistance;
   MatrixF mLightToWorld;
   MatrixF mWorldToLight;

   Vector<DepthSortList::Poly> mPartition;
   Vector<Point3F> mPartitionVerts;
   GFXVertexBufferHandle<GFXVertexPCT> mShadowBuffer;

   static U32 smShadowMask;

   static DepthSortList smDepthSortList;
   static GFXTexHandle smGenericShadowTexture;
   static F32 smGenericRadiusSkew;
   static S32 smGenericShadowDim;

   U32 mLastRenderTime;

   static void collisionCallback(SceneObject*,void *);

private:
   SceneObject* mParentObject;
   ShapeBase* mShapeBase;
   LightInfo* mParentLight;
   TSShapeInstance* mShapeInstance;
   GFXStateBlockRef mShadowSB;
   F32 mDepthBias;

   void setupStateBlocks();
   void setLightMatrices(const Point3F & lightDir, const Point3F & pos);
   void buildPartition(const Point3F & p, const Point3F & lightDir, F32 radius, F32 shadowLen);
public:

   BlobShadow(SceneObject* parentobject, LightInfo* light, TSShapeInstance* shapeinstance);
   ~BlobShadow();

   void setRadius(F32 radius);
   void setRadius(TSShapeInstance *, const Point3F & scale);

   bool prepare(const Point3F & pos, Point3F lightDir, F32 shadowLen);

   bool shouldRender(F32 camDist);

   void update( const SceneRenderState *state ) {}
   void render( F32 camDist, const TSRenderState &rdata );
   U32 getLastRenderTime() const { return mLastRenderTime; }

   static void generateGenericShadowBitmap(S32 dim);
   static void deleteGenericShadowBitmap();
};

#endif