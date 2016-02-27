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
#include "lighting/shadowMap/lightShadowMap.h"

#include "lighting/shadowMap/shadowMapManager.h"
#include "lighting/shadowMap/shadowMatHook.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/gfxOcclusionQuery.h"
#include "gfx/gfxCardProfile.h"
#include "gfx/sim/debugDraw.h"
#include "materials/materialDefinition.h"
#include "materials/baseMatInstance.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "scene/zones/sceneZoneSpace.h"
#include "lighting/lightManager.h"
#include "math/mathUtils.h"
#include "shaderGen/shaderGenVars.h"
#include "core/util/safeDelete.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"
#include "materials/shaderData.h"
#include "core/module.h"

// Used for creation in ShadowMapParams::getOrCreateShadowMap()
#include "lighting/shadowMap/singleLightShadowMap.h"
#include "lighting/shadowMap/pssmLightShadowMap.h"
#include "lighting/shadowMap/cubeLightShadowMap.h"
#include "lighting/shadowMap/dualParaboloidLightShadowMap.h"

// Remove this when the shader constants are reworked better
#include "lighting/advanced/advancedLightManager.h"
#include "lighting/advanced/advancedLightBinManager.h"

// TODO: Some cards (Justin's GeForce 7x series) barf on the integer format causing
// filtering artifacts. These can (sometimes) be resolved by switching the format
// to FP16 instead of Int16.
const GFXFormat LightShadowMap::ShadowMapFormat = GFXFormatR32F; // GFXFormatR8G8B8A8;

bool LightShadowMap::smDebugRenderFrustums;
F32 LightShadowMap::smShadowTexScalar = 1.0f;

Vector<LightShadowMap*> LightShadowMap::smUsedShadowMaps;
Vector<LightShadowMap*> LightShadowMap::smShadowMaps;

GFX_ImplementTextureProfile( ShadowMapProfile,
                              GFXTextureProfile::DiffuseMap,
                              GFXTextureProfile::PreserveSize | 
                              GFXTextureProfile::RenderTarget |
                              GFXTextureProfile::Pooled,
                              GFXTextureProfile::NONE );

GFX_ImplementTextureProfile( ShadowMapZProfile,
                             GFXTextureProfile::DiffuseMap, 
                             GFXTextureProfile::PreserveSize | 
                             GFXTextureProfile::NoMipmap | 
                             GFXTextureProfile::ZTarget |
                             GFXTextureProfile::Pooled,
                             GFXTextureProfile::NONE );


LightShadowMap::LightShadowMap( LightInfo *light )
   :  mWorldToLightProj( true ),
      mLight( light ),
      mTexSize( 0 ),
      mLastShader( NULL ),
      mLastUpdate( 0 ),
      mLastCull( 0 ),
      mIsViewDependent( false ),
      mLastScreenSize( 0.0f ),
      mLastPriority( 0.0f ),
      mIsDynamic( false )
{
   GFXTextureManager::addEventDelegate( this, &LightShadowMap::_onTextureEvent );

   mTarget = GFX->allocRenderToTextureTarget();
   smShadowMaps.push_back( this );
   mStaticRefreshTimer = PlatformTimer::create();
   mDynamicRefreshTimer = PlatformTimer::create();
}

LightShadowMap::~LightShadowMap()
{
   mTarget = NULL;
   SAFE_DELETE(mStaticRefreshTimer);
   SAFE_DELETE(mDynamicRefreshTimer);

   releaseTextures();

   smShadowMaps.remove( this );
   smUsedShadowMaps.remove( this );

   GFXTextureManager::removeEventDelegate( this, &LightShadowMap::_onTextureEvent );
}

void LightShadowMap::releaseAllTextures()
{
   PROFILE_SCOPE( LightShadowMap_ReleaseAllTextures );

   for ( U32 i=0; i < smShadowMaps.size(); i++ )
      smShadowMaps[i]->releaseTextures();
}

U32 LightShadowMap::releaseUnusedTextures()
{
   PROFILE_SCOPE( LightShadowMap_ReleaseUnusedTextures );

   const U32 currTime = Sim::getCurrentTime();
   const U32 purgeTime = 1000;

   for ( U32 i=0; i < smUsedShadowMaps.size(); )
   {
      LightShadowMap *lsm = smUsedShadowMaps[i];

      // If the shadow has not been culled in a while then
      // release its textures for other shadows to use.
      if (  currTime > ( lsm->mLastCull + purgeTime ) )
      {
         // Internally this will remove the map from the used
         // list, so don't increment the loop.
         lsm->releaseTextures();         
         continue;
      }

      i++;
   }

   return smUsedShadowMaps.size();
}

void LightShadowMap::_onTextureEvent( GFXTexCallbackCode code )
{
   if ( code == GFXZombify )
      releaseTextures();

   // We don't initialize here as we want the textures
   // to be reallocated when the shadow becomes visible.
}

void LightShadowMap::calcLightMatrices( MatrixF &outLightMatrix, const Frustum &viewFrustum )
{
   // Create light matrix, set projection

   switch ( mLight->getType() )
   {
   case LightInfo::Vector :
      {
         const ShadowMapParams *p = mLight->getExtended<ShadowMapParams>();

         // Calculate the bonding box of the shadowed area 
         // we're interested in... this is the shadow box 
         // transformed by the frustum transform.
         Box3F viewBB( -p->shadowDistance, -p->shadowDistance, -p->shadowDistance,
                        p->shadowDistance, p->shadowDistance, p->shadowDistance );
         viewFrustum.getTransform().mul( viewBB );

         // Calculate a light "projection" matrix.
         MatrixF lightMatrix = MathUtils::createOrientFromDir(mLight->getDirection());
         outLightMatrix = lightMatrix;
         static MatrixF rotMat(EulerF( (M_PI_F / 2.0f), 0.0f, 0.0f));
         lightMatrix.mul( rotMat );

         // This is the box in lightspace
         Box3F lightViewBB(viewBB);
         lightMatrix.mul(lightViewBB);

         // Now, let's position our light based on the lightViewBB
         Point3F newLightPos(viewBB.getCenter());
         F32 sceneDepth = lightViewBB.maxExtents.z - lightViewBB.minExtents.z;
         newLightPos += mLight->getDirection() * ((-sceneDepth / 2.0f)-1.0f);         // -1 for the nearplane
         outLightMatrix.setPosition(newLightPos);

         // Update light info
         mLight->setRange( sceneDepth );
         mLight->setPosition( newLightPos );

         // Set our ortho projection
         F32 width = (lightViewBB.maxExtents.x - lightViewBB.minExtents.x) / 2.0f;
         F32 height = (lightViewBB.maxExtents.y - lightViewBB.minExtents.y) / 2.0f;

         width = getMax(width, height);

         GFX->setOrtho(-width, width, -width, width, 1.0f, sceneDepth, true);


         // TODO: Width * 2... really isn't that pixels being used as
         // meters?  Is a real physical metric of scene depth better?         
         //SceneManager::setVisibleDistance(width * 2.0f);

#if 0
         DebugDrawer::get()->drawFrustum(viewFrustum, ColorF(1.0f, 0.0f, 0.0f));
         DebugDrawer::get()->drawBox(viewBB.minExtents, viewBB.maxExtents, ColorF(0.0f, 1.0f, 0.0f));
         DebugDrawer::get()->drawBox(lightViewBB.minExtents, lightViewBB.maxExtents, ColorF(0.0f, 0.0f, 1.0f));
         DebugDrawer::get()->drawBox(newLightPos - Point3F(1,1,1), newLightPos + Point3F(1,1,1), ColorF(1,1,0));
         DebugDrawer::get()->drawLine(newLightPos, newLightPos + mLight.mDirection*3.0f, ColorF(0,1,1));

         Point3F a(newLightPos);
         Point3F b(newLightPos);
         Point3F offset(width, height,0.0f);
         a -= offset;
         b += offset;
         DebugDrawer::get()->drawBox(a, b, ColorF(0.5f, 0.5f, 0.5f));
#endif
      }
      break;
   case LightInfo::Spot :
      {
         outLightMatrix = mLight->getTransform();
         F32 fov = mDegToRad( mLight->getOuterConeAngle() );
         F32 farDist = mLight->getRange().x;
         F32 nearDist = farDist * 0.01f;

         F32 left, right, top, bottom;
         MathUtils::makeFrustum( &left, &right, &top, &bottom, fov, 1.0f, nearDist );
         GFX->setFrustum( left, right, bottom, top, nearDist, farDist );
      }
      break;
   default:
      AssertFatal(false, "Unsupported light type!");
   }
}

void LightShadowMap::releaseTextures()
{
   mShadowMapTex = NULL;
   mDebugTarget.setTexture( NULL );
   mLastUpdate = 0;
   smUsedShadowMaps.remove( this );
}

void LightShadowMap::setDebugTarget( const String &name )
{
   mDebugTarget.registerWithName( name );
   mDebugTarget.setTexture( mShadowMapTex );
}

GFXTextureObject* LightShadowMap::_getDepthTarget( U32 width, U32 height )
{
   // Get a depth texture target from the pooled profile
   // which is returned as a temporary.
   GFXTexHandle depthTex( width, height, GFXFormatD24S8, &ShadowMapZProfile, 
      "LightShadowMap::_getDepthTarget()" );

   return depthTex;
}

bool LightShadowMap::setTextureStage( U32 currTexFlag, LightingShaderConstants* lsc )
{
   if ( currTexFlag == Material::DynamicLight && !isDynamic() )
   {
      S32 reg = lsc->mShadowMapSC->getSamplerRegister();

      if ( reg != -1 )
         GFX->setTexture( reg, mShadowMapTex);

      return true;
   } else if ( currTexFlag == Material::DynamicShadowMap && isDynamic() )
   {
      S32 reg = lsc->mDynamicShadowMapSC->getSamplerRegister();

      if ( reg != -1 )
         GFX->setTexture( reg, mShadowMapTex);

      return true;
   }
   else if ( currTexFlag == Material::DynamicLightMask )
   {
      S32 reg = lsc->mCookieMapSC->getSamplerRegister();
   	if ( reg != -1 )
      {
         ShadowMapParams *p = mLight->getExtended<ShadowMapParams>();

         if ( lsc->mCookieMapSC->getType() == GFXSCT_SamplerCube )
            GFX->setCubeTexture( reg, p->getCookieCubeTex() );
         else
      	   GFX->setTexture( reg, p->getCookieTex() );
      }

      return true;
   }

   return false;
}

void LightShadowMap::render(  RenderPassManager* renderPass,
                              const SceneRenderState *diffuseState,
                              bool _dynamic)
{
    //  control how often shadow maps are refreshed
    if (!_dynamic && (mStaticRefreshTimer->getElapsedMs() < getLightInfo()->getStaticRefreshFreq()))
        return;
    mStaticRefreshTimer->reset();

    if (_dynamic && (mDynamicRefreshTimer->getElapsedMs() < getLightInfo()->getDynamicRefreshFreq()))
        return;
    mDynamicRefreshTimer->reset();

   mDebugTarget.setTexture( NULL );
   _render( renderPass, diffuseState );
   mDebugTarget.setTexture( mShadowMapTex );

   // Add it to the used list unless we're been updated.
   if ( !mLastUpdate )
   {
      AssertFatal( !smUsedShadowMaps.contains( this ), "LightShadowMap::render - Used shadow map inserted twice!" );
      smUsedShadowMaps.push_back( this );
   }

   mLastUpdate = Sim::getCurrentTime();
}

BaseMatInstance* LightShadowMap::getShadowMaterial( BaseMatInstance *inMat ) const
{
   // See if we have an existing material hook.
   ShadowMaterialHook *hook = static_cast<ShadowMaterialHook*>( inMat->getHook( ShadowMaterialHook::Type ) );
   if ( !hook )
   {
      // Create a hook and initialize it using the incoming material.
      hook = new ShadowMaterialHook;
      hook->init( inMat );
      inMat->addHook( hook );
   }

   return hook->getShadowMat( getShadowType() );
}

U32 LightShadowMap::getBestTexSize( U32 scale ) const
{
   const ShadowMapParams *params = mLight->getExtended<ShadowMapParams>();
   
   // The view dependent shadows don't scale by screen size. 
   U32 texSize;
   if ( isViewDependent() )
      texSize = params->texSize;
   else
      texSize = params->texSize * getMin( 1.0f, mLastScreenSize );

   // Apply the shadow texture scale and make
   // sure this is a power of 2.
   texSize = getNextPow2( texSize * smShadowTexScalar );

   // Get the max texture size this card supports and
   // scale it down... ensuring the final texSize can
   // be scaled up that many times and not go over
   // the card maximum.
   U32 maxTexSize = GFX->getCardProfiler()->queryProfile( "maxTextureSize", 2048 );
   if ( scale > 1 )
      maxTexSize >>= ( scale - 1 );

   // Never let the shadow texture get smaller than 16x16 as
   // it just makes the pool bigger and the fillrate savings
   // are less and leass as we get smaller.
   texSize = mClamp( texSize, (U32)16, maxTexSize );

   // Return it.
   return texSize;
}

void LightShadowMap::updatePriority( const SceneRenderState *state, U32 currTimeMs )
{
   PROFILE_SCOPE( LightShadowMap_updatePriority );

   mLastCull = currTimeMs;

   if ( isViewDependent() )
   {
      mLastScreenSize = 1.0f;
      mLastPriority = F32_MAX;
      return;
   }

   U32 timeSinceLastUpdate = currTimeMs - mLastUpdate;

   const Point3F &camPt = state->getCameraPosition();
   F32 range = mLight->getRange().x;
   F32 dist;

   if ( mLight->getType() == LightInfo::Spot )
   {
      // We treat the cone as a cylinder to get the
      // approximate projection distance.

      Point3F endPt = mLight->getPosition() + ( mLight->getDirection() * range );
      Point3F nearPt = MathUtils::mClosestPointOnSegment( mLight->getPosition(), endPt, camPt );
      dist = ( camPt - nearPt ).len();

      F32 radius = range * mSin( mDegToRad( mLight->getOuterConeAngle() * 0.5f ) );
      dist -= radius;
   }
   else
      dist = SphereF( mLight->getPosition(), range ).distanceTo( camPt );

   // Get the approximate screen size of the light.
   mLastScreenSize = state->projectRadius( dist, range );
   mLastScreenSize /= state->getViewport().extent.y;

   // Update the priority.
   mLastPriority = mPow( mLastScreenSize * 50.0f, 2.0f );   
   mLastPriority += timeSinceLastUpdate;
   mLastPriority *= mLight->getPriority();
}

S32 QSORT_CALLBACK LightShadowMap::cmpPriority( LightShadowMap *const *lsm1, LightShadowMap *const *lsm2 )
{
   F32 diff = (*lsm1)->getLastPriority() - (*lsm2)->getLastPriority(); 
   return diff > 0.0f ? -1 : ( diff < 0.0f ? 1 : 0 );
}

void LightShadowMap::_debugRender( SceneRenderState* shadowRenderState )
{
   #ifdef TORQUE_DEBUG

   // Skip if light does not have debug rendering enabled.
   if( !getLightInfo()->isDebugRenderingEnabled() )
      return;

   DebugDrawer* drawer = DebugDrawer::get();
   if( !drawer )
      return;

   if( smDebugRenderFrustums )
      shadowRenderState->getCullingState().debugRenderCullingVolumes();

   #endif
}


LightingShaderConstants::LightingShaderConstants() 
   :  mInit( false ),
      mShader( NULL ),
      mLightParamsSC(NULL), 
      mLightSpotParamsSC(NULL), 
      mLightPositionSC(NULL),
      mLightDiffuseSC(NULL), 
      mLightAmbientSC(NULL), 
      mLightInvRadiusSqSC(NULL),
      mLightSpotDirSC(NULL),
      mLightSpotAngleSC(NULL),
      mLightSpotFalloffSC(NULL),
      mShadowMapSC(NULL), 
      mDynamicShadowMapSC(NULL), 
      mShadowMapSizeSC(NULL), 
      mCookieMapSC(NULL),
      mRandomDirsConst(NULL),
      mShadowSoftnessConst(NULL), 
      mAtlasXOffsetSC(NULL), 
      mAtlasYOffsetSC(NULL),
      mAtlasScaleSC(NULL), 
      mFadeStartLength(NULL), 
      mOverDarkFactorPSSM(NULL), 
      mTapRotationTexSC(NULL),

      mWorldToLightProjSC(NULL), 
      mViewToLightProjSC(NULL),
      mScaleXSC(NULL), 
      mScaleYSC(NULL),
      mOffsetXSC(NULL), 
      mOffsetYSC(NULL), 
      mFarPlaneScalePSSM(NULL),

      mDynamicWorldToLightProjSC(NULL),
      mDynamicViewToLightProjSC(NULL),
      mDynamicScaleXSC(NULL),
      mDynamicScaleYSC(NULL),
      mDynamicOffsetXSC(NULL),
      mDynamicOffsetYSC(NULL),
      mDynamicFarPlaneScalePSSM(NULL)
{
}

LightingShaderConstants::~LightingShaderConstants()
{
   if (mShader.isValid())
   {
      mShader->getReloadSignal().remove( this, &LightingShaderConstants::_onShaderReload );
      mShader = NULL;
   }
}

void LightingShaderConstants::init(GFXShader* shader)
{
   if (mShader.getPointer() != shader)
   {
      if (mShader.isValid())
         mShader->getReloadSignal().remove( this, &LightingShaderConstants::_onShaderReload );

      mShader = shader;
      mShader->getReloadSignal().notify( this, &LightingShaderConstants::_onShaderReload );
   }

   mLightParamsSC = shader->getShaderConstHandle("$lightParams");
   mLightSpotParamsSC = shader->getShaderConstHandle("$lightSpotParams");

   // NOTE: These are the shader constants used for doing lighting 
   // during the forward pass.  Do not confuse these for the prepass
   // lighting constants which are used from AdvancedLightBinManager.
   mLightPositionSC = shader->getShaderConstHandle( ShaderGenVars::lightPosition );
   mLightDiffuseSC = shader->getShaderConstHandle( ShaderGenVars::lightDiffuse );
   mLightAmbientSC = shader->getShaderConstHandle( ShaderGenVars::lightAmbient );
   mLightInvRadiusSqSC = shader->getShaderConstHandle( ShaderGenVars::lightInvRadiusSq );
   mLightSpotDirSC = shader->getShaderConstHandle( ShaderGenVars::lightSpotDir );
   mLightSpotAngleSC = shader->getShaderConstHandle( ShaderGenVars::lightSpotAngle );
   mLightSpotFalloffSC = shader->getShaderConstHandle( ShaderGenVars::lightSpotFalloff );

   mShadowMapSC = shader->getShaderConstHandle("$shadowMap");
   mDynamicShadowMapSC = shader->getShaderConstHandle("$dynamicShadowMap");
   mShadowMapSizeSC = shader->getShaderConstHandle("$shadowMapSize");

   mCookieMapSC = shader->getShaderConstHandle("$cookieMap");

   mShadowSoftnessConst = shader->getShaderConstHandle("$shadowSoftness");
   mAtlasXOffsetSC = shader->getShaderConstHandle("$atlasXOffset");
   mAtlasYOffsetSC = shader->getShaderConstHandle("$atlasYOffset");
   mAtlasScaleSC = shader->getShaderConstHandle("$atlasScale");

   mFadeStartLength = shader->getShaderConstHandle("$fadeStartLength");
   mOverDarkFactorPSSM = shader->getShaderConstHandle("$overDarkPSSM");
   mTapRotationTexSC = shader->getShaderConstHandle( "$gTapRotationTex" );

   mWorldToLightProjSC = shader->getShaderConstHandle("$worldToLightProj");
   mViewToLightProjSC = shader->getShaderConstHandle("$viewToLightProj");
   mScaleXSC = shader->getShaderConstHandle("$scaleX");
   mScaleYSC = shader->getShaderConstHandle("$scaleY");
   mOffsetXSC = shader->getShaderConstHandle("$offsetX");
   mOffsetYSC = shader->getShaderConstHandle("$offsetY");
   mFarPlaneScalePSSM = shader->getShaderConstHandle("$farPlaneScalePSSM");

   mDynamicWorldToLightProjSC = shader->getShaderConstHandle("$dynamicWorldToLightProj");
   mDynamicViewToLightProjSC = shader->getShaderConstHandle("$dynamicViewToLightProj");
   mDynamicScaleXSC = shader->getShaderConstHandle("$dynamicScaleX");
   mDynamicScaleYSC = shader->getShaderConstHandle("$dynamicScaleY");
   mDynamicOffsetXSC = shader->getShaderConstHandle("$dynamicOffsetX");
   mDynamicOffsetYSC = shader->getShaderConstHandle("$dynamicOffsetY");
   mDynamicFarPlaneScalePSSM = shader->getShaderConstHandle("$dynamicFarPlaneScalePSSM");

   mInit = true;
}

void LightingShaderConstants::_onShaderReload()
{
   if (mShader.isValid())
      init( mShader );
}

MODULE_BEGIN( ShadowMapParams )
MODULE_INIT_BEFORE( LightMapParams )
MODULE_INIT
{
   ShadowMapParams::Type = "ShadowMapParams" ;
}
MODULE_END;

LightInfoExType ShadowMapParams::Type( "" );

ShadowMapParams::ShadowMapParams( LightInfo *light ) 
   :  mLight( light ),
      mShadowMap( NULL ),
      mDynamicShadowMap ( NULL ),
      isDynamic ( true )
{
   attenuationRatio.set( 0.0f, 1.0f, 1.0f );
   shadowType = ShadowType_Spot;
   overDarkFactor.set(2000.0f, 1000.0f, 500.0f, 100.0f);
   numSplits = 4;
   logWeight = 0.91f;
   texSize = 512;
   shadowDistance = 400.0f;
   shadowSoftness = 0.15f;
   fadeStartDist = 0.0f;
   lastSplitTerrainOnly = false;
   mQuery = GFX->createOcclusionQuery();

   _validate();
}

ShadowMapParams::~ShadowMapParams()
{
   SAFE_DELETE( mQuery );
   SAFE_DELETE( mShadowMap );
   SAFE_DELETE( mDynamicShadowMap );
}

void ShadowMapParams::_validate()
{
   switch ( mLight->getType() )
   {
      case LightInfo::Spot:
         shadowType = ShadowType_Spot;
         break;

      case LightInfo::Vector:
         shadowType = ShadowType_PSSM;
         break;

      case LightInfo::Point:
         if ( shadowType < ShadowType_Paraboloid )
            shadowType = ShadowType_DualParaboloidSinglePass;
         break;
      
      default:
         break;
   }

   // The texture sizes for shadows should always
   // be power of 2 in size.
   texSize = getNextPow2( texSize );

   // The maximum shadow texture size setting we're 
   // gonna allow... this doesn't use your hardware 
   // settings as you may be on a lower end system 
   // than your target machine.
   //
   // We apply the hardware specific limits during 
   // shadow rendering.
   //
   U32 maxTexSize = 4096;

   if ( mLight->getType() == LightInfo::Vector )
   {
      numSplits = mClamp( numSplits, 1, 4 );
      
      // Adjust the shadow texture size for the PSSM 
      // based on the split count to keep the total
      // shadow texture size within 4096.
      if ( numSplits == 2 || numSplits == 4 )
         maxTexSize = 2048;
      if ( numSplits == 3 )
         maxTexSize = 1024;
   }
   else
      numSplits = 1;

   // Keep it in a valid range... less than 32 is dumb.
   texSize = mClamp( texSize, 32, maxTexSize );
}

LightShadowMap* ShadowMapParams::getOrCreateShadowMap(bool _isDynamic)
{
	if (_isDynamic && mDynamicShadowMap)
		return mDynamicShadowMap;

	if (!_isDynamic && mShadowMap)
      return mShadowMap;

   if ( !mLight->getCastShadows() )
      return NULL;

   LightShadowMap* newShadowMap = NULL;

   switch ( mLight->getType() )
   {
      case LightInfo::Spot:
         newShadowMap = new SingleLightShadowMap( mLight );
         break;

      case LightInfo::Vector:
         newShadowMap = new PSSMLightShadowMap( mLight );
         break;

      case LightInfo::Point:

         if ( shadowType == ShadowType_CubeMap )
            newShadowMap = new CubeLightShadowMap( mLight );
         else if ( shadowType == ShadowType_Paraboloid )
            newShadowMap = new ParaboloidLightShadowMap( mLight );
         else
            newShadowMap = new DualParaboloidLightShadowMap( mLight );
         break;
   
      default:
         break;
   }

   if ( _isDynamic )
   {
      newShadowMap->setDynamic( true );
      mDynamicShadowMap = newShadowMap;
      return mDynamicShadowMap;
   }
   else
   {
      newShadowMap->setDynamic(false);
      mShadowMap = newShadowMap;
      return mShadowMap;
   }
}

GFXTextureObject* ShadowMapParams::getCookieTex()
{
   if (  cookie.isNotEmpty() &&
         (  mCookieTex.isNull() || 
            cookie != mCookieTex->getPath() ) )
   {
      mCookieTex.set(   cookie, 
                        &GFXDefaultStaticDiffuseProfile, 
                        "ShadowMapParams::getCookieTex()" );
   }
   else if ( cookie.isEmpty() )
      mCookieTex = NULL;

   return mCookieTex.getPointer();
}

GFXCubemap* ShadowMapParams::getCookieCubeTex()
{
   if (  cookie.isNotEmpty() &&
         (  mCookieCubeTex.isNull() || 
            cookie != mCookieCubeTex->getPath() ) )
   {
      mCookieCubeTex.set( cookie );
   }
   else if ( cookie.isEmpty() )
      mCookieCubeTex = NULL;

   return mCookieCubeTex.getPointer();
}

void ShadowMapParams::set( const LightInfoEx *ex )
{
   // TODO: Do we even need this?
}

void ShadowMapParams::packUpdate( BitStream *stream ) const
{
   // HACK: We need to work out proper parameter 
   // validation when any field changes on the light.

   ((ShadowMapParams*)this)->_validate();

   stream->writeInt( shadowType, 8 );
   
   mathWrite( *stream, attenuationRatio );
   
   stream->write( texSize );

   stream->write( cookie );

   stream->write( numSplits );
   stream->write( logWeight );

   mathWrite(*stream, overDarkFactor);

   stream->write( fadeStartDist );
   stream->writeFlag( lastSplitTerrainOnly );

   stream->write( shadowDistance );

   stream->write( shadowSoftness );   
}

void ShadowMapParams::unpackUpdate( BitStream *stream )
{
   ShadowType newType = (ShadowType)stream->readInt( 8 );
   if ( shadowType != newType )
   {
      // If the shadow type changes delete the shadow
      // map so it can be reallocated on the next render.
      shadowType = newType;
      SAFE_DELETE( mShadowMap );
      SAFE_DELETE( mDynamicShadowMap );
   }

   mathRead( *stream, &attenuationRatio );

   stream->read( &texSize );

   stream->read( &cookie );

   stream->read( &numSplits );
   stream->read( &logWeight );
   mathRead(*stream, &overDarkFactor);

   stream->read( &fadeStartDist );
   lastSplitTerrainOnly = stream->readFlag();

   stream->read( &shadowDistance );   

   stream->read( &shadowSoftness );
}
