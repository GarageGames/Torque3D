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
#include "renderInstance/renderGlowMgr.h"

#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "materials/sceneData.h"
#include "materials/matInstance.h"
#include "materials/materialFeatureTypes.h"
#include "materials/processedMaterial.h"
#include "postFx/postEffect.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDebugEvent.h"
#include "math/util/matrixSet.h"

IMPLEMENT_CONOBJECT( RenderGlowMgr );


ConsoleDocClass( RenderGlowMgr, 
   "@brief A render bin for the glow pass.\n\n"
   "When the glow buffer PostEffect is enabled this bin gathers mesh render "
   "instances with glow materials and renders them to the #glowbuffer offscreen "
   "render target.\n\n"
   "This render target is then used by the 'GlowPostFx' PostEffect to blur and "
   "render the glowing portions of the screen.\n\n"
   "@ingroup RenderBin\n" );

const MatInstanceHookType RenderGlowMgr::GlowMaterialHook::Type( "Glow" );


RenderGlowMgr::GlowMaterialHook::GlowMaterialHook( BaseMatInstance *matInst )
   : mGlowMatInst( NULL )
{
   mGlowMatInst = (MatInstance*)matInst->getMaterial()->createMatInstance();
   mGlowMatInst->getFeaturesDelegate().bind( &GlowMaterialHook::_overrideFeatures );
   mGlowMatInst->init(  matInst->getRequestedFeatures(), 
                        matInst->getVertexFormat() );
}

RenderGlowMgr::GlowMaterialHook::~GlowMaterialHook()
{
   SAFE_DELETE( mGlowMatInst );
}

void RenderGlowMgr::GlowMaterialHook::_overrideFeatures( ProcessedMaterial *mat,
                                                         U32 stageNum,
                                                         MaterialFeatureData &fd, 
                                                         const FeatureSet &features )
{
   // If this isn't a glow pass... then add the glow mask feature.
   if (  mat->getMaterial() && 
         !mat->getMaterial()->mGlow[stageNum] )
      fd.features.addFeature( MFT_GlowMask );

   // Don't allow fog or HDR encoding on 
   // the glow materials.
   fd.features.removeFeature( MFT_Fog );
   fd.features.removeFeature( MFT_HDROut );
}

RenderGlowMgr::RenderGlowMgr()
   : RenderTexTargetBinManager(  RenderPassManager::RIT_Mesh, 
                                 1.0f, 
                                 1.0f,
                                 GFXFormatR8G8B8A8,
                                 Point2I( 512, 512 ) )
{
   notifyType( RenderPassManager::RIT_Decal );
   notifyType( RenderPassManager::RIT_Translucent );

   mNamedTarget.registerWithName( "glowbuffer" );
   mTargetSizeType = WindowSize;
}

RenderGlowMgr::~RenderGlowMgr()
{
}

PostEffect* RenderGlowMgr::getGlowEffect()
{
   if ( !mGlowEffect )
      mGlowEffect = dynamic_cast<PostEffect*>( Sim::findObject( "GlowPostFx" ) );
   
   return mGlowEffect;
}

bool RenderGlowMgr::isGlowEnabled()
{
   return getGlowEffect() && getGlowEffect()->isEnabled();
}

void RenderGlowMgr::addElement( RenderInst *inst )
{
   // Skip out if we don't have the glow post 
   // effect enabled at this time.
   if ( !isGlowEnabled() )
      return;

   // TODO: We need to get the scene state here in a more reliable
   // manner so we can skip glow in a non-diffuse render pass.
   //if ( !mParentManager->getSceneManager()->getSceneState()->isDiffusePass() )
      //return RenderBinManager::arSkipped;

   // Skip it if we don't have a glowing material.
   BaseMatInstance *matInst = getMaterial( inst );
   if ( !matInst || !matInst->hasGlow() )   
      return;

   internalAddElement(inst);
}

void RenderGlowMgr::render( SceneRenderState *state )
{
   PROFILE_SCOPE( RenderGlowMgr_Render );
   
   if ( !isGlowEnabled() )
      return;

   const U32 binSize = mElementList.size();

   // If this is a non-diffuse pass or we have no objects to
   // render then tell the effect to skip rendering.
   if ( !state->isDiffusePass() || binSize == 0 )
   {
      getGlowEffect()->setSkip( true );
      return;
   }

   GFXDEBUGEVENT_SCOPE( RenderGlowMgr_Render, ColorI::GREEN );

   GFXTransformSaver saver;

   // Tell the superclass we're about to render, preserve contents
   const bool isRenderingToTarget = _onPreRender( state, true );

   // Clear all the buffers to black.
   GFX->clear( GFXClearTarget, ColorI::BLACK, 1.0f, 0);

   // Restore transforms
   MatrixSet &matrixSet = getRenderPass()->getMatrixSet();
   matrixSet.restoreSceneViewProjection();

   // init loop data
   SceneData sgData;
   sgData.init( state, SceneData::GlowBin );

   for( U32 j=0; j<binSize; )
   {
      MeshRenderInst *ri = static_cast<MeshRenderInst*>(mElementList[j].inst);

      setupSGData( ri, sgData );

      BaseMatInstance *mat = ri->matInst;
      GlowMaterialHook *hook = mat->getHook<GlowMaterialHook>();
      if ( !hook )
      {
         hook = new GlowMaterialHook( ri->matInst );
         ri->matInst->addHook( hook );
      }
      BaseMatInstance *glowMat = hook->getMatInstance();

      U32 matListEnd = j;

      while( glowMat && glowMat->setupPass( state, sgData ) )
      {
         U32 a;
         for( a=j; a<binSize; a++ )
         {
            MeshRenderInst *passRI = static_cast<MeshRenderInst*>(mElementList[a].inst);

            if ( newPassNeeded( ri, passRI ) )
               break;

            matrixSet.setWorld(*passRI->objectToWorld);
            matrixSet.setView(*passRI->worldToCamera);
            matrixSet.setProjection(*passRI->projection);
            glowMat->setTransforms(matrixSet, state);

            glowMat->setSceneInfo(state, sgData);
            glowMat->setBuffers(passRI->vertBuff, passRI->primBuff);

            if ( passRI->prim )
               GFX->drawPrimitive( *passRI->prim );
            else
               GFX->drawPrimitive( passRI->primBuffIndex );
         }
         matListEnd = a;
         setupSGData( ri, sgData );
      }

      // force increment if none happened, otherwise go to end of batch
      j = ( j == matListEnd ) ? j+1 : matListEnd;
   }

   // Finish up.
   if ( isRenderingToTarget )
      _onPostRender();

   // Make sure the effect is gonna render.
   getGlowEffect()->setSkip( false );
}
