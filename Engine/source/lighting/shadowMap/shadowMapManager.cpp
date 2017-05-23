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
#include "lighting/shadowMap/shadowMapManager.h"

#include "lighting/shadowMap/shadowMapPass.h"
#include "lighting/shadowMap/lightShadowMap.h"
#include "materials/materialManager.h"
#include "lighting/lightManager.h"
#include "core/util/safeDelete.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxTextureManager.h"
#include "core/module.h"
#include "console/consoleTypes.h"


GFX_ImplementTextureProfile(ShadowMapTexProfile,
                            GFXTextureProfile::DiffuseMap, 
                            GFXTextureProfile::PreserveSize | GFXTextureProfile::Dynamic , 
                            GFXTextureProfile::NONE);


MODULE_BEGIN( ShadowMapManager )

   MODULE_INIT
   {
      ManagedSingleton< ShadowMapManager >::createSingleton();
   }
   
   MODULE_SHUTDOWN
   {
      ManagedSingleton< ShadowMapManager >::deleteSingleton();
   }

MODULE_END;


AFTER_MODULE_INIT( Sim )
{
   Con::addVariable( "$pref::Shadows::textureScalar",	
      TypeF32, &LightShadowMap::smShadowTexScalar,
      "@brief Used to scale the shadow texture sizes.\n"
      "This can reduce the shadow quality and texture memory overhead or increase them.\n"
      "@ingroup AdvancedLighting\n" );
   Con::NotifyDelegate callabck( &LightShadowMap::releaseAllTextures );
   Con::addVariableNotify( "$pref::Shadows::textureScalar", callabck );

   Con::addVariable( "$pref::Shadows::disable", 
      TypeBool, &ShadowMapPass::smDisableShadowsPref,
      "Used to disable all shadow rendering.\n"
      "@ingroup AdvancedLighting\n" );

   Con::addVariable( "$Shadows::disable", 
      TypeBool, &ShadowMapPass::smDisableShadowsEditor,
      "Used by the editor to disable all shadow rendering.\n"
      "@ingroup AdvancedLighting\n" );

   Con::NotifyDelegate shadowCallback( &ShadowMapManager::updateShadowDisable );
   Con::addVariableNotify( "$pref::Shadows::disable", shadowCallback );
   Con::addVariableNotify( "$Shadows::disable", shadowCallback );

   Con::addVariable("$pref::Shadows::teleportDist",
      TypeF32, &ShadowMapPass::smShadowsTeleportDist,
      "Minimum distance moved per frame to determine that we are teleporting.\n");
   Con::addVariableNotify("$pref::Shadows::teleportDist", shadowCallback);

   Con::addVariable("$pref::Shadows::turnRate",
      TypeF32, &ShadowMapPass::smShadowsTurnRate,
      "Minimum angle moved per frame to determine that we are turning quickly.\n");
   Con::addVariableNotify("$pref::Shadows::turnRate", shadowCallback);
}

Signal<void(void)> ShadowMapManager::smShadowDeactivateSignal;


ShadowMapManager::ShadowMapManager() 
:  mShadowMapPass(NULL), 
   mCurrentShadowMap(NULL),
   mCurrentDynamicShadowMap(NULL),
   mIsActive(false)
{
}

ShadowMapManager::~ShadowMapManager()
{
}

void ShadowMapManager::setLightShadowMapForLight( LightInfo *light )
{
   ShadowMapParams *params = light->getExtended<ShadowMapParams>();
   if ( params )
   {
      mCurrentShadowMap = params->getShadowMap();
      mCurrentDynamicShadowMap = params->getShadowMap(true);
   }
   else 
   {
      mCurrentShadowMap = NULL;
      mCurrentDynamicShadowMap = NULL;
   }
}

void ShadowMapManager::activate()
{
   ShadowManager::activate();

   if (!getSceneManager())
   {
      Con::errorf("This world has no scene manager!  Shadow manager not activating!");
      return;
   }

   mShadowMapPass = new ShadowMapPass(LIGHTMGR, this);

   getSceneManager()->getPreRenderSignal().notify( this, &ShadowMapManager::_onPreRender, 0.01f );
   GFXTextureManager::addEventDelegate( this, &ShadowMapManager::_onTextureEvent );

   mIsActive = true;
}

void ShadowMapManager::deactivate()
{
   GFXTextureManager::removeEventDelegate( this, &ShadowMapManager::_onTextureEvent );
   getSceneManager()->getPreRenderSignal().remove( this, &ShadowMapManager::_onPreRender );

   SAFE_DELETE(mShadowMapPass);
   mTapRotationTex = NULL;

   // Clean up our shadow texture memory.
   LightShadowMap::releaseAllTextures();
   TEXMGR->cleanupPool();

   mIsActive = false;

   ShadowManager::deactivate();
}

void ShadowMapManager::_onPreRender( SceneManager *sg, const SceneRenderState *state )
{
   if ( mShadowMapPass && state->isDiffusePass() )
      mShadowMapPass->render( sg, state, (U32)-1 );
}

void ShadowMapManager::_onTextureEvent( GFXTexCallbackCode code )
{
   if ( code == GFXZombify )
      mTapRotationTex = NULL;
}

GFXTextureObject* ShadowMapManager::getTapRotationTex()
{
   if ( mTapRotationTex.isValid() )
      return mTapRotationTex;

   mTapRotationTex.set( 64, 64, GFXFormatR8G8B8A8, &ShadowMapTexProfile, 
                        "ShadowMapManager::getTapRotationTex" );

   GFXLockedRect *rect = mTapRotationTex.lock();
   U8 *f = rect->bits;
   F32 angle;
   for( U32 i = 0; i < 64*64; i++, f += 4 )
   {         
      // We only pack the rotations into the red
      // and green channels... the rest are empty.
      angle = M_2PI_F * gRandGen.randF();
      f[0] = U8_MAX * ( ( 1.0f + mSin( angle ) ) * 0.5f );
      f[1] = U8_MAX * ( ( 1.0f + mCos( angle ) ) * 0.5f );
      f[2] = 0;
      f[3] = 0;
   }

   mTapRotationTex.unlock();

   return mTapRotationTex;
}

void ShadowMapManager::updateShadowDisable()
{
   bool disable = false;

   if ( ShadowMapPass::smDisableShadowsEditor || ShadowMapPass::smDisableShadowsPref )
      disable = true;

   if ( disable != ShadowMapPass::smDisableShadows)
   {
      ShadowMapPass::smDisableShadows = disable;
      smShadowDeactivateSignal.trigger();
   }
}
