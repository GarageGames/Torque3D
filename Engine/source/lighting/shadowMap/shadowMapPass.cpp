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
#include "lighting/shadowMap/shadowMapPass.h"

#include "lighting/shadowMap/lightShadowMap.h"
#include "lighting/shadowMap/shadowMapManager.h"
#include "lighting/lightManager.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "renderInstance/renderPassManager.h"
#include "renderInstance/renderObjectMgr.h"
#include "renderInstance/renderMeshMgr.h"
#include "renderInstance/renderTerrainMgr.h"
#include "renderInstance/renderImposterMgr.h"
#include "core/util/safeDelete.h"
#include "console/consoleTypes.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDebugEvent.h"
#include "platform/platformTimer.h"


const String ShadowMapPass::PassTypeName("ShadowMap");

U32 ShadowMapPass::smActiveShadowMaps = 0;
U32 ShadowMapPass::smUpdatedShadowMaps = 0;
U32 ShadowMapPass::smNearShadowMaps = 0;
U32 ShadowMapPass::smShadowMapsDrawCalls = 0;
U32 ShadowMapPass::smShadowMapPolyCount = 0;
U32 ShadowMapPass::smRenderTargetChanges = 0;
U32 ShadowMapPass::smShadowPoolTexturesCount = 0.;
F32 ShadowMapPass::smShadowPoolMemory = 0.0f;

bool ShadowMapPass::smDisableShadows = false;
bool ShadowMapPass::smDisableShadowsEditor = false;
bool ShadowMapPass::smDisableShadowsPref = false;

/// We have a default 8ms render budget for shadow rendering.
U32 ShadowMapPass::smRenderBudgetMs = 8;

ShadowMapPass::ShadowMapPass(LightManager* lightManager, ShadowMapManager* shadowManager)
{
   mLightManager = lightManager;
   mShadowManager = shadowManager;
   mShadowRPM = new ShadowRenderPassManager();
   mShadowRPM->assignName( "ShadowRenderPassManager" );
   mShadowRPM->registerObject();
   Sim::getRootGroup()->addObject( mShadowRPM );

   // Setup our render pass manager

   mShadowRPM->addManager( new RenderMeshMgr(RenderPassManager::RIT_Mesh, 0.3f, 0.3f) );
   //mShadowRPM->addManager( new RenderObjectMgr() );
   mShadowRPM->addManager( new RenderTerrainMgr( 0.5f, 0.5f )  );
   mShadowRPM->addManager( new RenderImposterMgr( 0.6f, 0.6f )  );

   mActiveLights = 0;

   mTimer = PlatformTimer::create();

   Con::addVariable( "$ShadowStats::activeMaps", TypeS32, &smActiveShadowMaps,
      "The shadow stats showing the active number of shadow maps.\n"
      "@ingroup AdvancedLighting\n" );

   Con::addVariable( "$ShadowStats::updatedMaps", TypeS32, &smUpdatedShadowMaps,
      "The shadow stats showing the number of shadow maps updated this frame.\n"
      "@ingroup AdvancedLighting\n" );

   Con::addVariable( "$ShadowStats::nearMaps", TypeS32, &smNearShadowMaps,
      "The shadow stats showing the number of shadow maps that are close enough to be updated very frame.\n"
      "@ingroup AdvancedLighting\n" );

   Con::addVariable( "$ShadowStats::drawCalls", TypeS32, &smShadowMapsDrawCalls,
      "The shadow stats showing the number of draw calls in shadow map renders for this frame.\n"
      "@ingroup AdvancedLighting\n" );

   Con::addVariable( "$ShadowStats::polyCount", TypeS32, &smShadowMapPolyCount,
      "The shadow stats showing the number of triangles in shadow map renders for this frame.\n"
      "@ingroup AdvancedLighting\n" );

   Con::addVariable( "$ShadowStats::rtChanges", TypeS32, &smRenderTargetChanges,
      "The shadow stats showing the number of render target changes for shadow maps in this frame.\n"
      "@ingroup AdvancedLighting\n" );

   Con::addVariable( "$ShadowStats::poolTexCount", TypeS32, &smShadowPoolTexturesCount,
      "The shadow stats showing the number of shadow textures in the shadow texture pool.\n"
      "@ingroup AdvancedLighting\n" );

   Con::addVariable( "$ShadowStats::poolTexMemory", TypeF32, &smShadowPoolMemory,
      "The shadow stats showing the approximate texture memory usage of the shadow map texture pool.\n"
      "@ingroup AdvancedLighting\n" );
}

ShadowMapPass::~ShadowMapPass()
{
   SAFE_DELETE( mTimer );

   if ( mShadowRPM )
      mShadowRPM->deleteObject();
}

void ShadowMapPass::render(   SceneManager *sceneManager, 
                              const SceneRenderState *diffuseState, 
                              U32 objectMask )
{
   PROFILE_SCOPE( ShadowMapPass_Render );

   // Prep some shadow rendering stats.
   smActiveShadowMaps = 0;
   smUpdatedShadowMaps = 0;
   smNearShadowMaps = 0;
   GFXDeviceStatistics stats;
   stats.start( GFX->getDeviceStatistics() );

   // NOTE: The lights were already registered by SceneManager.

   // Update mLights
   mLights.clear();
   mLightManager->getAllUnsortedLights( &mLights );
   mActiveLights = mLights.size();

   // Use the per-frame incremented time for
   // priority updates and to track when the 
   // shadow was last updated.
   const U32 currTime = Sim::getCurrentTime();

   // First do a loop thru the lights setting up the shadow
   // info array for this pass.
   Vector<LightShadowMap*> shadowMaps;
   shadowMaps.reserve( mActiveLights );
   for ( U32 i = 0; i < mActiveLights; i++ )
   {
      ShadowMapParams *params = mLights[i]->getExtended<ShadowMapParams>();

      // Before we do anything... skip lights without shadows.      
      if ( !mLights[i]->getCastShadows() || smDisableShadows )
         continue;

      LightShadowMap *lsm = params->getOrCreateShadowMap();

      // First check the visiblity query... if it wasn't 
      // visible skip it.
      if ( lsm->wasOccluded() )
         continue;

      // Any shadow that is visible is counted as being 
      // active regardless if we update it or not.
      ++smActiveShadowMaps;

      // Do a priority update for this shadow.
      lsm->updatePriority( diffuseState, currTime );

      shadowMaps.push_back( lsm );
   }

   // Now sort the shadow info by priority.
   shadowMaps.sort( LightShadowMap::cmpPriority );

   GFXDEBUGEVENT_SCOPE( ShadowMapPass_Render, ColorI::RED );

   // Use a timer for tracking our shadow rendering 
   // budget to ensure a high precision results.
   mTimer->getElapsedMs();
   mTimer->reset();

   for ( U32 i = 0; i < shadowMaps.size(); i++ )
   {
      LightShadowMap *lsm = shadowMaps[i];

      {
         GFXDEBUGEVENT_SCOPE( ShadowMapPass_Render_Shadow, ColorI::RED );

         mShadowManager->setLightShadowMap( lsm );
         lsm->render( mShadowRPM, diffuseState );
         ++smUpdatedShadowMaps;
      }

      // View dependent shadows or ones that are covering the entire
      // screen are updated every frame no matter the time left in
      // our shadow rendering budget.
      if ( lsm->isViewDependent() || lsm->getLastScreenSize() >= 1.0f )
      {
         ++smNearShadowMaps;
         continue;
      }

      // See if we're over our frame budget for shadow 
      // updates... give up completely in that case.
      if ( mTimer->getElapsedMs() > smRenderBudgetMs )
         break;
   }

   // Cleanup old unused textures.
   LightShadowMap::releaseUnusedTextures();

   // Update the stats.
   stats.end( GFX->getDeviceStatistics() );
   smShadowMapsDrawCalls = stats.mDrawCalls;
   smShadowMapPolyCount = stats.mPolyCount;
   smRenderTargetChanges = stats.mRenderTargetChanges;
   smShadowPoolTexturesCount = ShadowMapProfile.getStats().activeCount;
   smShadowPoolMemory = ( ShadowMapProfile.getStats().activeBytes / 1024.0f ) / 1024.0f;

   // The NULL here is importaint as having it around
   // will cause extra work in AdvancedLightManager::setLightInfo().
   mShadowManager->setLightShadowMap( NULL );
}

void ShadowRenderPassManager::addInst( RenderInst *inst )
{
   PROFILE_SCOPE(ShadowRenderPassManager_addInst);

   if ( inst->type == RIT_Mesh )
   {
      MeshRenderInst *meshRI = static_cast<MeshRenderInst*>( inst );
      if ( !meshRI->matInst )
         return;

      const BaseMaterialDefinition *mat = meshRI->matInst->getMaterial();
      if ( !mat->castsShadows() || mat->isTranslucent() )
      {
         // Do not add this instance, return here and avoid the default behavior
         // of calling up to Parent::addInst()
         return;
      }
   }

   Parent::addInst(inst);
}