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

#ifndef _PROJECTEDSHADOW_H_
#define _PROJECTEDSHADOW_H_

#ifndef _DEPTHSORTLIST_H_
#include "collision/depthSortList.h"
#endif
#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _TSSHAPEINSTANCE_H_
#include "ts/tsShapeInstance.h"
#endif
#ifndef _LIGHTINGSYSTEM_SHADOWBASE_H_
#include "lighting/common/shadowBase.h"
#endif

class ShapeBase;
class LightInfo;
class DecalData;
class DecalInstance;
class RenderPassManager;
class PostEffect;
class RenderMeshMgr;
class CustomMaterial;
class BaseMatInstance;
class MaterialParameterHandle;


GFX_DeclareTextureProfile( BLProjectedShadowProfile );
GFX_DeclareTextureProfile( BLProjectedShadowZProfile );

class ProjectedShadow : public ShadowBase
{

protected:


   /// This parameter is used to
   /// adjust the far plane out for our
   /// orthographic render in order to 
   /// force our object towards one end of the
   /// the eye space depth range.
   static F32 smDepthAdjust;

   F32 mRadius;
   MatrixF mWorldToLight;
   U32 mLastRenderTime;

   F32 mShadowLength;

   F32 mScore;
   bool mUpdateTexture;

   Point3F mLastObjectScale;
   Point3F mLastObjectPosition;
   VectorF mLastLightDir;

   DecalData *mDecalData;
   DecalInstance *mDecalInstance;

   SceneObject *mParentObject;
   ShapeBase *mShapeBase;

   MaterialParameterHandle *mCasterPositionSC;
   MaterialParameterHandle *mShadowLengthSC;

   static SimObjectPtr<RenderPassManager> smRenderPass;

   static SimObjectPtr<PostEffect> smShadowFilter;

   static RenderPassManager* _getRenderPass();

   GFXTexHandle mShadowTexture;
   GFXTextureTargetRef mRenderTarget;

   GFXTextureObject* _getDepthTarget( U32 width, U32 height );
   void _renderToTexture( F32 camDist, const TSRenderState &rdata );

   bool _updateDecal( const SceneRenderState *sceneState );

   void _calcScore( const SceneRenderState *state );
 
   /// Returns a spotlight shadow material for use when
   /// rendering meshes into the projected shadow.
   static BaseMatInstance* _getShadowMaterial( BaseMatInstance *inMat );

public:

   /// @see DecalData
   static float smFadeStartPixelSize;
   static float smFadeEndPixelSize;

   ProjectedShadow( SceneObject *object );
   virtual ~ProjectedShadow();

   bool shouldRender( const SceneRenderState *state );

   void update( const SceneRenderState *state );
   void render( F32 camDist, const TSRenderState &rdata );
   U32 getLastRenderTime() const { return mLastRenderTime; }
   const F32 getScore() const { return mScore; }

};

#endif // _PROJECTEDSHADOW_H_