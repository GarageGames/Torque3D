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

#ifndef _SHADOWMAPPASS_H_
#define _SHADOWMAPPASS_H_

#ifndef _RENDERPASSMANAGER_H_
#include "renderInstance/renderPassManager.h"
#endif
#ifndef _RENDERMESHMGR_H_
#include "renderInstance/renderMeshMgr.h"
#endif
#ifndef _LIGHTINFO_H_
#include "lighting/lightInfo.h"
#endif
#ifndef _SHADOW_COMMON_H_
#include "lighting/shadowMap/shadowCommon.h"
#endif

class RenderMeshMgr;
class LightShadowMap;
class LightManager;
class ShadowMapManager;
class BaseMatInstance;
class RenderObjectMgr;
class RenderTerrainMgr;
class PlatformTimer;
class ShadowRenderPassManager;
class DynamicShadowRenderPassManager;

/// ShadowMapPass, this is plugged into the SceneManager to generate 
/// ShadowMaps for the scene.
class ShadowMapPass
{
public:

   ShadowMapPass() {}   // Only called by ConsoleSystem
   ShadowMapPass(LightManager* LightManager, ShadowMapManager* ShadowManager);
   virtual ~ShadowMapPass();

   //
   // SceneRenderPass interface
   //

   /// Called to render a scene.
   void render(   SceneManager *sceneGraph, 
                  const SceneRenderState *diffuseState, 
                  U32 objectMask );

   /// Return the type of pass this is
   virtual const String& getPassType() const { return PassTypeName; };

   /// Return our sort value. (Go first in order to have shadow maps available for RIT_Objects)
   virtual F32 getSortValue() const { return 0.0f; }

   virtual bool geometryOnly() const { return true; }

   static const String PassTypeName;


   /// Used to for debugging performance by disabling
   /// shadow updates and rendering.
   static bool smDisableShadows;

   static bool smDisableShadowsEditor;
   static bool smDisableShadowsPref;

   /// distance moved per frame before forcing a shadow update
   static F32 smShadowsTeleportDist;
   /// angle turned per frame before forcing a shadow update
   static F32 smShadowsTurnRate;

private:

   static U32 smActiveShadowMaps;
   static U32 smUpdatedShadowMaps;
   static U32 smNearShadowMaps;
   static U32 smShadowMapsDrawCalls;
   static U32 smShadowMapPolyCount;
   static U32 smRenderTargetChanges;
   static U32 smShadowPoolTexturesCount;
   static F32 smShadowPoolMemory;

   /// The milliseconds alotted for shadow map updates
   /// on a per frame basis.
   static U32 smRenderBudgetMs;

   PlatformTimer *mTimer;

   LightInfoList mLights;
   U32 mActiveLights;
   SimObjectPtr<ShadowRenderPassManager> mShadowRPM;
   SimObjectPtr<DynamicShadowRenderPassManager> mDynamicShadowRPM;
   LightManager* mLightManager;
   ShadowMapManager* mShadowManager;
   Point3F mPrevCamPos;
   Point3F mPrevCamRot;
   F32 mPrevCamFov;
};

class ShadowRenderPassManager : public RenderPassManager
{
   typedef RenderPassManager Parent;
public:
   ShadowRenderPassManager() : Parent() {}

   /// Add a RenderInstance to the list
   virtual void addInst( RenderInst *inst );
};

class DynamicShadowRenderPassManager : public RenderPassManager
{
	typedef RenderPassManager Parent;
public:
	DynamicShadowRenderPassManager() : Parent() {}

	/// Add a RenderInstance to the list
	virtual void addInst(RenderInst *inst);
};

#endif // _SHADOWMAPPASS_H_
