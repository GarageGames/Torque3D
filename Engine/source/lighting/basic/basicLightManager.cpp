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
#include "lighting/basic/basicLightManager.h"

#include "platform/platformTimer.h"
#include "console/simSet.h"
#include "console/consoleTypes.h"
#include "core/module.h"
#include "core/util/safeDelete.h"
#include "materials/processedMaterial.h"
#include "shaderGen/shaderFeature.h"
#include "lighting/basic/basicSceneObjectLightingPlugin.h"
#include "shaderGen/shaderGenVars.h"
#include "gfx/gfxShader.h"
#include "materials/sceneData.h"
#include "materials/materialParameters.h"
#include "materials/materialManager.h"
#include "materials/materialFeatureTypes.h"
#include "math/util/frustum.h"
#include "scene/sceneObject.h"
#include "renderInstance/renderPrePassMgr.h"
#include "shaderGen/featureMgr.h"
#include "shaderGen/HLSL/shaderFeatureHLSL.h"
#include "shaderGen/HLSL/bumpHLSL.h"
#include "shaderGen/HLSL/pixSpecularHLSL.h"
#include "lighting/basic/blTerrainSystem.h"
#include "lighting/common/projectedShadow.h"

#if defined( TORQUE_OPENGL )
#include "shaderGen/GLSL/shaderFeatureGLSL.h"
#include "shaderGen/GLSL/bumpGLSL.h"
#include "shaderGen/GLSL/pixSpecularGLSL.h"
#endif


MODULE_BEGIN( BasicLightManager )

   MODULE_SHUTDOWN_AFTER( Scene )

   MODULE_INIT
   {
      ManagedSingleton< BasicLightManager >::createSingleton();
   }
   
   MODULE_SHUTDOWN
   {
      ManagedSingleton< BasicLightManager >::deleteSingleton();
   }

MODULE_END;


U32 BasicLightManager::smActiveShadowPlugins = 0;
U32 BasicLightManager::smShadowsUpdated = 0;
U32 BasicLightManager::smElapsedUpdateMs = 0;

F32 BasicLightManager::smProjectedShadowFilterDistance = 40.0f;

static S32 QSORT_CALLBACK comparePluginScores( const void *a, const void *b )
{
   const BasicSceneObjectLightingPlugin *A = *((BasicSceneObjectLightingPlugin**)a);
   const BasicSceneObjectLightingPlugin *B = *((BasicSceneObjectLightingPlugin**)b);     
   
   F32 dif = B->getScore() - A->getScore();
   return (S32)mFloor( dif );
}

BasicLightManager::BasicLightManager()
   : LightManager( "Basic Lighting", "BLM" ),
     mLastShader(NULL),
     mLastConstants(NULL)
{
   mTimer = PlatformTimer::create();
   
   mTerrainSystem = new blTerrainSystem;
   
   getSceneLightingInterface()->registerSystem( mTerrainSystem );

   Con::addVariable( "$BasicLightManagerStats::activePlugins", 
      TypeS32, &smActiveShadowPlugins,
      "The number of active Basic Lighting SceneObjectLightingPlugin objects this frame.\n"
      "@ingroup BasicLighting\n" );

   Con::addVariable( "$BasicLightManagerStats::shadowsUpdated", 
      TypeS32, &smShadowsUpdated,
      "The number of Basic Lighting shadows updated this frame.\n"
      "@ingroup BasicLighting\n" );

   Con::addVariable( "$BasicLightManagerStats::elapsedUpdateMs", 
      TypeS32, &smElapsedUpdateMs,
      "The number of milliseconds spent this frame updating Basic Lighting shadows.\n"
      "@ingroup BasicLighting\n" );

   Con::addVariable( "$BasicLightManager::shadowFilterDistance", 
      TypeF32, &smProjectedShadowFilterDistance,
      "The maximum distance in meters that projected shadows will get soft filtering.\n"
      "@ingroup BasicLighting\n" );

   Con::addVariable( "$pref::ProjectedShadow::fadeStartPixelSize", 
      TypeF32, &ProjectedShadow::smFadeStartPixelSize,
      "A size in pixels at which BL shadows begin to fade out. "
      "This should be a larger value than fadeEndPixelSize.\n"
      "@see DecalData\n"
      "@ingroup BasicLighting\n" );

   Con::addVariable( "$pref::ProjectedShadow::fadeEndPixelSize", 
      TypeF32, &ProjectedShadow::smFadeEndPixelSize,
      "A size in pixels at which BL shadows are fully faded out. "
      "This should be a smaller value than fadeStartPixelSize.\n"
      "@see DecalData\n"
      "@ingroup BasicLighting\n" );
}

BasicLightManager::~BasicLightManager()
{
   mLastShader = NULL;
   mLastConstants = NULL;

   for (LightConstantMap::Iterator i = mConstantLookup.begin(); i != mConstantLookup.end(); i++)
   {
      if (i->value)
         SAFE_DELETE(i->value);
   }
   mConstantLookup.clear();

   if (mTimer)
      SAFE_DELETE( mTimer );

   SAFE_DELETE( mTerrainSystem );
}

bool BasicLightManager::isCompatible() const
{
   // As long as we have some shaders this works.
   return GFX->getPixelShaderVersion() > 1.0;
}

void BasicLightManager::activate( SceneManager *sceneManager )
{
   Parent::activate( sceneManager );

   if( GFX->getAdapterType() == OpenGL )
   {
      #if defined( TORQUE_OPENGL ) 
         FEATUREMGR->registerFeature( MFT_LightMap, new LightmapFeatGLSL );
         FEATUREMGR->registerFeature( MFT_ToneMap, new TonemapFeatGLSL );
         FEATUREMGR->registerFeature( MFT_NormalMap, new BumpFeatGLSL );
         FEATUREMGR->registerFeature( MFT_RTLighting, new RTLightingFeatGLSL );
         FEATUREMGR->registerFeature( MFT_PixSpecular, new PixelSpecularGLSL );
      #endif
   }
   else
   {
      #if defined( TORQUE_OS_WIN )
         FEATUREMGR->registerFeature( MFT_LightMap, new LightmapFeatHLSL );
         FEATUREMGR->registerFeature( MFT_ToneMap, new TonemapFeatHLSL );
         FEATUREMGR->registerFeature( MFT_NormalMap, new BumpFeatHLSL );
         FEATUREMGR->registerFeature( MFT_RTLighting, new RTLightingFeatHLSL );
         FEATUREMGR->registerFeature( MFT_PixSpecular, new PixelSpecularHLSL );
      #endif
   }

   FEATUREMGR->unregisterFeature( MFT_MinnaertShading );
   FEATUREMGR->unregisterFeature( MFT_SubSurface );

   // First look for the prepass bin...
   RenderPrePassMgr *prePassBin = _findPrePassRenderBin();

   /*
   // If you would like to use forward shading, and have a linear depth pre-pass
   // than un-comment this code block.
   if ( !prePassBin )
   {
      Vector<GFXFormat> formats;
      formats.push_back( GFXFormatR32F );
      formats.push_back( GFXFormatR16F );
      formats.push_back( GFXFormatR8G8B8A8 );
      GFXFormat linearDepthFormat = GFX->selectSupportedFormat( &GFXDefaultRenderTargetProfile,
         formats,
         true,
         false );

      // Uncomment this for a no-color-write z-fill pass. 
      //linearDepthFormat = GFXFormat_COUNT;

      prePassBin = new RenderPrePassMgr( linearDepthFormat != GFXFormat_COUNT, linearDepthFormat );
      prePassBin->registerObject();
      rpm->addManager( prePassBin );
   }
   */
   mPrePassRenderBin = prePassBin;

   // If there is a prepass bin
   MATMGR->setPrePassEnabled( mPrePassRenderBin.isValid() );
   sceneManager->setPostEffectFog( mPrePassRenderBin.isValid() && mPrePassRenderBin->getTargetChainLength() > 0  );

   // Tell the material manager that we don't use prepass.
   MATMGR->setPrePassEnabled( false );

   GFXShader::addGlobalMacro( "TORQUE_BASIC_LIGHTING" );

   // Hook into the SceneManager prerender signal.
   sceneManager->getPreRenderSignal().notify( this, &BasicLightManager::_onPreRender );

   // Last thing... let everyone know we're active.
   smActivateSignal.trigger( getId(), true );
}

void BasicLightManager::deactivate()
{
   Parent::deactivate();

   mLastShader = NULL;
   mLastConstants = NULL;

   for (LightConstantMap::Iterator i = mConstantLookup.begin(); i != mConstantLookup.end(); i++)
   {
      if (i->value)
         SAFE_DELETE(i->value);
   }
   mConstantLookup.clear();

   if ( mPrePassRenderBin )
      mPrePassRenderBin->deleteObject();
   mPrePassRenderBin = NULL;

   GFXShader::removeGlobalMacro( "TORQUE_BASIC_LIGHTING" );

   // Remove us from the prerender signal.
   getSceneManager()->getPreRenderSignal().remove( this, &BasicLightManager::_onPreRender );

   // Now let everyone know we've deactivated.
   smActivateSignal.trigger( getId(), false );
}

void BasicLightManager::_onPreRender( SceneManager *sceneManger, const SceneRenderState *state )
{
   // Update all our shadow plugins here!
   Vector<BasicSceneObjectLightingPlugin*> *pluginInsts = BasicSceneObjectLightingPlugin::getPluginInstances();

   Vector<BasicSceneObjectLightingPlugin*>::const_iterator pluginIter = (*pluginInsts).begin();
   for ( ; pluginIter != (*pluginInsts).end(); pluginIter++ )
   {
      BasicSceneObjectLightingPlugin *plugin = *pluginIter;
      plugin->updateShadow( (SceneRenderState*)state );
   }

   U32 pluginCount = (*pluginInsts).size();

   // Sort them by the score.
   dQsort( (*pluginInsts).address(), pluginCount, sizeof(BasicSceneObjectLightingPlugin*), comparePluginScores );

   mTimer->getElapsedMs();
   mTimer->reset();
   U32 numUpdated = 0;
   U32 targetMs = 5;

   S32 updateMs = 0;

   pluginIter = (*pluginInsts).begin();
   for ( ; pluginIter != (*pluginInsts).end(); pluginIter++ )
   {
      BasicSceneObjectLightingPlugin *plugin = *pluginIter;

      // If we run out of update time then stop.
      updateMs = mTimer->getElapsedMs();
      if ( updateMs >= targetMs )
         break;

      // NOTE! Fix this all up to past const SceneRenderState!
      plugin->renderShadow( (SceneRenderState*)state );
      numUpdated++;
   }

   smShadowsUpdated = numUpdated;
   smActiveShadowPlugins = pluginCount;
   smElapsedUpdateMs = updateMs;
}

BasicLightManager::LightingShaderConstants::LightingShaderConstants()
   :  mInit( false ),
      mShader( NULL ),
      mLightPosition( NULL ),
      mLightDiffuse( NULL ),
      mLightAmbient( NULL ),
      mLightInvRadiusSq( NULL ),
      mLightSpotDir( NULL ),
      mLightSpotAngle( NULL ),
	  mLightSpotFalloff( NULL )
{
}

BasicLightManager::LightingShaderConstants::~LightingShaderConstants()
{
   if (mShader.isValid())
   {
      mShader->getReloadSignal().remove( this, &LightingShaderConstants::_onShaderReload );
      mShader = NULL;
   }
}

void BasicLightManager::LightingShaderConstants::init(GFXShader* shader)
{
   if (mShader.getPointer() != shader)
   {
      if (mShader.isValid())
         mShader->getReloadSignal().remove( this, &LightingShaderConstants::_onShaderReload );

      mShader = shader;
      mShader->getReloadSignal().notify( this, &LightingShaderConstants::_onShaderReload );
   }

   mLightPosition = shader->getShaderConstHandle( ShaderGenVars::lightPosition );
   mLightDiffuse = shader->getShaderConstHandle( ShaderGenVars::lightDiffuse);
   mLightInvRadiusSq = shader->getShaderConstHandle( ShaderGenVars::lightInvRadiusSq );
   mLightAmbient = shader->getShaderConstHandle( ShaderGenVars::lightAmbient );   
   mLightSpotDir = shader->getShaderConstHandle( ShaderGenVars::lightSpotDir );
   mLightSpotAngle = shader->getShaderConstHandle( ShaderGenVars::lightSpotAngle );
   mLightSpotFalloff = shader->getShaderConstHandle( ShaderGenVars::lightSpotFalloff );

   mInit = true;
}

void BasicLightManager::LightingShaderConstants::_onShaderReload()
{
   if (mShader.isValid())
      init( mShader );
}

void BasicLightManager::setLightInfo(  ProcessedMaterial* pmat, 
                                       const Material* mat, 
                                       const SceneData& sgData, 
                                       const SceneRenderState *state,
                                       U32 pass, 
                                       GFXShaderConstBuffer* shaderConsts ) 
{
   PROFILE_SCOPE( BasicLightManager_SetLightInfo );

   GFXShader *shader = shaderConsts->getShader();

   // Check to see if this is the same shader.  Since we
   // sort by material we should get hit repeatedly by the
   // same one.  This optimization should save us many 
   // hash table lookups.
   if ( mLastShader.getPointer() != shader )
   {
      LightConstantMap::Iterator iter = mConstantLookup.find(shader);   
      if ( iter != mConstantLookup.end() )
      {
         mLastConstants = iter->value;
      } 
      else 
      {     
         LightingShaderConstants* lsc = new LightingShaderConstants();
         mConstantLookup[shader] = lsc;

         mLastConstants = lsc;      
      }

      // Set our new shader
      mLastShader = shader;
   }

   // Make sure that our current lighting constants are initialized
   if (!mLastConstants->mInit)
      mLastConstants->init(shader);

   // NOTE: If you encounter a crash from this point forward
   // while setting a shader constant its probably because the
   // mConstantLookup has bad shaders/constants in it.
   //
   // This is a known crash bug that can occur if materials/shaders
   // are reloaded and the light manager is not reset.
   //
   // We should look to fix this by clearing the table.

   _update4LightConsts( sgData,
                        mLastConstants->mLightPosition,
                        mLastConstants->mLightDiffuse,
                        mLastConstants->mLightAmbient,
                        mLastConstants->mLightInvRadiusSq,
                        mLastConstants->mLightSpotDir,
                        mLastConstants->mLightSpotAngle,
						mLastConstants->mLightSpotFalloff,
                        shaderConsts );
}
