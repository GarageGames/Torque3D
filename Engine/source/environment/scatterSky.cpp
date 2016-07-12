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
#include "scatterSky.h"

#include "core/stream/bitStream.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "sim/netConnection.h"
#include "math/util/sphereMesh.h"
#include "math/mathUtils.h"
#include "math/util/matrixSet.h"
#include "scene/sceneRenderState.h"
#include "lighting/lightInfo.h"
#include "gfx/sim/gfxStateBlockData.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/sim/cubemapData.h"
#include "materials/shaderData.h"
#include "materials/materialManager.h"
#include "materials/baseMatInstance.h"
#include "materials/sceneData.h"
#include "environment/timeOfDay.h"


ConsoleDocClass( ScatterSky,
   "@brief Represents both the sun and sky for scenes with a dynamic time of day.\n\n"

   "%ScatterSky renders as a dome shaped mesh which is camera relative and always overhead. "
   "It is intended to be part of the background of your scene and renders before all "
   "other objects types.\n\n"

   "%ScatterSky is designed for outdoor scenes which need to transition fluidly "
   "between radically different times of day. It will respond to time changes "
   "originating from a TimeOfDay object or the elevation field can be directly "
   "adjusted.\n\n"

   "During day, %ScatterSky uses atmosphereic sunlight scattering "
   "aproximations to generate a sky gradient and sun corona. It also calculates "
   "the fog color, ambient color, and sun color, which are used for scene "
   "lighting. This is user controlled by fields within the ScatterSky group.\n\n"

   "During night, %ScatterSky supports can transition to a night sky cubemap and "
   "moon sprite. The user can control this and night time colors used for scene "
   "lighting with fields within the Night group.\n\n"

   "A scene with a ScatterSky should not have any other sky or sun objects "
   "as it already fulfills both roles.\n\n"

   "%ScatterSky is intended to be used with CloudLayer and TimeOfDay as part of "
   "a scene with dynamic lighting. Having a %ScatterSky without a changing "
   "time of day would unnecessarily give up artistic control compared and fillrate "
   "compared to a SkyBox + Sun setup.\n\n"

   "@ingroup Atmosphere"
);


IMPLEMENT_CO_NETOBJECT_V1(ScatterSky);

const F32 ScatterSky::smEarthRadius = (6378.0f * 1000.0f);
const F32 ScatterSky::smAtmosphereRadius = 200000.0f;
const F32 ScatterSky::smViewerHeight = 1.0f;

ScatterSky::ScatterSky()
{
   mPrimCount = 0;
   mVertCount = 0;


   // Rayleigh scattering constant.
   mRayleighScattering = 0.0035f;
   mRayleighScattering4PI = mRayleighScattering * 4.0f * M_PI_F;

   // Mie scattering constant.
   mMieScattering = 0.0045f;
   mMieScattering4PI = mMieScattering * 4.0f * M_PI_F;

   // Overall scatter scalar.
   mSkyBrightness = 25.0f;

   // The Mie phase asymmetry factor.
   mMiePhaseAssymetry = -0.75f;

   mSphereInnerRadius = 1.0f;
   mSphereOuterRadius = 1.0f * 1.025f;
   mScale = 1.0f / (mSphereOuterRadius - mSphereInnerRadius);

   // 650 nm for red
   // 570 nm for green
   // 475 nm for blue
   mWavelength.set( 0.650f, 0.570f, 0.475f, 0 );

   mWavelength4[0] = mPow(mWavelength[0], 4.0f);
   mWavelength4[1] = mPow(mWavelength[1], 4.0f);
   mWavelength4[2] = mPow(mWavelength[2], 4.0f);

   mRayleighScaleDepth = 0.25f;
   mMieScaleDepth = 0.1f;

   mAmbientColor.set( 0, 0, 0, 1.0f );
   mAmbientScale.set( 1.0f, 1.0f, 1.0f, 1.0f );

   mSunColor.set( 0, 0, 0, 1.0f );
   mSunScale = ColorF::WHITE;

   mFogColor.set( 0, 0, 0, 1.0f );
   mFogScale = ColorF::WHITE;

   mExposure = 1.0f;
   mNightInterpolant = 0;
   mZOffset = 0.0f;

   mShader = NULL;

   mTimeOfDay = 0;

   mSunAzimuth = 0.0f;
   mSunElevation = 35.0f;

   mMoonAzimuth = 0.0f;
   mMoonElevation = 45.0f;

   mBrightness = 1.0f;

   mCastShadows = true;
   mStaticRefreshFreq = 8;
   mDynamicRefreshFreq = 8;
   mDirty = true;

   mLight = LightManager::createLightInfo();
   mLight->setType( LightInfo::Vector );

   mFlareData = NULL;
   mFlareState.clear();
   mFlareScale = 1.0f;

   mMoonEnabled = true;
   mMoonScale = 0.2f;
   mMoonTint.set( 0.192157f, 0.192157f, 0.192157f, 1.0f );
   MathUtils::getVectorFromAngles( mMoonLightDir, 0.0f, 45.0f );
   mMoonLightDir.normalize();
   mMoonLightDir = -mMoonLightDir;
   mNightCubemap = NULL;
   mNightColor.set( 0.0196078f, 0.0117647f, 0.109804f, 1.0f );
   mNightFogColor = mNightColor;
   mUseNightCubemap = false;
   mSunSize = 1.0f;

   mMoonMatInst = NULL;

   mNetFlags.set( Ghostable | ScopeAlways );
   mTypeMask |= EnvironmentObjectType | LightObjectType | StaticObjectType;

   _generateSkyPoints();

   mMatrixSet = reinterpret_cast<MatrixSet *>(dMalloc_aligned(sizeof(MatrixSet), 16));
   constructInPlace(mMatrixSet);

   mColorizeAmt = 0;
   mColorize.set(0,0,0);
}

ScatterSky::~ScatterSky()
{
   SAFE_DELETE( mLight );
   SAFE_DELETE( mMoonMatInst );

   dFree_aligned(mMatrixSet);
}

bool ScatterSky::onAdd()
{
   PROFILE_SCOPE(ScatterSky_onAdd);

   // onNewDatablock for the server is called here
   // for the client it is called in unpackUpdate

   if ( !Parent::onAdd() )
      return false;

   if ( isClientObject() )
      TimeOfDay::getTimeOfDayUpdateSignal().notify( this, &ScatterSky::_updateTimeOfDay );

   setGlobalBounds();
   resetWorldBox();

   addToScene();

   if ( isClientObject() )
   {
      _initMoon();
      Sim::findObject( mNightCubemapName, mNightCubemap );
   }

   return true;
}

void ScatterSky::onRemove()
{
   removeFromScene();

   if ( isClientObject() )
      TimeOfDay::getTimeOfDayUpdateSignal().remove( this, &ScatterSky::_updateTimeOfDay );

   Parent::onRemove();
}

void ScatterSky::_conformLights()
{
   _initCurves();

   F32 val = mCurves[0].getVal( mTimeOfDay );
   mNightInterpolant = 1.0f - val;

   VectorF lightDirection;
   F32 brightness;

   // Build the light direction from the azimuth and elevation.
   F32 yaw = mDegToRad(mClampF(mSunAzimuth,0,359));
   F32 pitch = mDegToRad(mClampF(mSunElevation,-360,+360));
   MathUtils::getVectorFromAngles(lightDirection, yaw, pitch);
   lightDirection.normalize();
   mSunDir = -lightDirection;

   yaw = mDegToRad(mClampF(mMoonAzimuth,0,359));
   pitch = mDegToRad(mClampF(mMoonElevation,-360,+360));
   MathUtils::getVectorFromAngles( mMoonLightDir, yaw, pitch );
   mMoonLightDir.normalize();
   mMoonLightDir = -mMoonLightDir;

   brightness = mCurves[2].getVal( mTimeOfDay );

   if ( mNightInterpolant >= 1.0f )
      lightDirection = -mMoonLightDir;

   mLight->setDirection( -lightDirection );
   mLight->setBrightness( brightness * mBrightness );
   mLightDir = lightDirection;

   // Have to do interpolation
   // after the light direction is set
   // otherwise the sun color will be invalid.
   _interpolateColors();

   mLight->setAmbient( mAmbientColor );
   mLight->setColor( mSunColor );
   mLight->setCastShadows( mCastShadows );
   mLight->setStaticRefreshFreq(mStaticRefreshFreq);
   mLight->setDynamicRefreshFreq(mDynamicRefreshFreq);

   FogData fog = getSceneManager()->getFogData();
   fog.color = mFogColor;
   getSceneManager()->setFogData( fog );
}

void ScatterSky::submitLights( LightManager *lm, bool staticLighting )
{
   if ( mDirty )
   {
      _conformLights();
      mDirty = false;
   }

   // The sun is a special light and needs special registration.
   lm->setSpecialLight( LightManager::slSunLightType, mLight );
}

void ScatterSky::setAzimuth( F32 azimuth )
{
   mSunAzimuth = azimuth;
   mDirty = true;
   setMaskBits( TimeMask );
}

void ScatterSky::setElevation( F32 elevation )
{
   mSunElevation = elevation;

   while( elevation < 0 )
      elevation += 360.0f;

   while( elevation >= 360.0f )
      elevation -= 360.0f;

   mTimeOfDay = elevation / 180.0f;
   mDirty = true;
   setMaskBits( TimeMask );
}

void ScatterSky::inspectPostApply()
{
   mDirty = true;
   setMaskBits( 0xFFFFFFFF );
}

void ScatterSky::initPersistFields()
{
   addGroup( "ScatterSky",
      "Only azimuth and elevation are networked fields. To trigger a full update of all other fields use the applyChanges ConsoleMethod." );

      addField( "skyBrightness",       TypeF32,    Offset( mSkyBrightness, ScatterSky ),
         "Global brightness and intensity applied to the sky and objects in the level." );

      addField( "sunSize",             TypeF32,    Offset( mSunSize, ScatterSky ),
         "Affects the size of the sun's disk." );

      addField( "colorizeAmount",      TypeF32,    Offset( mColorizeAmt, ScatterSky ),
         "Controls how much the alpha component of colorize brigthens the sky. Setting to 0 returns default behavior." );

      addField( "colorize",            TypeColorF, Offset( mColorize, ScatterSky ),
         "Tints the sky the color specified, the alpha controls the brigthness. The brightness is multipled by the value of colorizeAmt." );

      addField( "rayleighScattering",  TypeF32,    Offset( mRayleighScattering, ScatterSky ),
         "Controls how blue the atmosphere is during the day." );

      addField( "sunScale",            TypeColorF, Offset( mSunScale, ScatterSky ),
         "Modulates the directional color of sunlight." );

      addField( "ambientScale",        TypeColorF, Offset( mAmbientScale, ScatterSky ),
         "Modulates the ambient color of sunlight." );

      addField( "fogScale",            TypeColorF, Offset( mFogScale, ScatterSky ),
         "Modulates the fog color. Note that this overrides the LevelInfo.fogColor "
         "property, so you should not use LevelInfo.fogColor if the level contains "
         "a ScatterSky object." );

      addField( "exposure",            TypeF32,    Offset( mExposure, ScatterSky ),
         "Controls the contrast of the sky and sun during daytime." );

      addField( "zOffset",             TypeF32,     Offset( mZOffset, ScatterSky ),  
         "Offsets the scatterSky to avoid canvas rendering. Use 5000 or greater for the initial adjustment" );  

   endGroup( "ScatterSky" );

   addGroup( "Orbit" );

      addProtectedField( "azimuth", TypeF32, Offset( mSunAzimuth, ScatterSky ), &ScatterSky::ptSetAzimuth, &defaultProtectedGetFn,
         "The horizontal angle of the sun measured clockwise from the positive Y world axis. This field is networked." );

      addProtectedField( "elevation", TypeF32, Offset( mSunElevation, ScatterSky ), &ScatterSky::ptSetElevation, &defaultProtectedGetFn,
         "The elevation angle of the sun above or below the horizon. This field is networked." );

      addField( "moonAzimuth", TypeF32, Offset( mMoonAzimuth, ScatterSky ),
         "The horizontal angle of the moon measured clockwise from the positive Y world axis. This is not animated by time or networked." );

      addField( "moonElevation", TypeF32, Offset( mMoonElevation, ScatterSky ),
         "The elevation angle of the moon above or below the horizon. This is not animated by time or networked." );

   endGroup( "Orbit" );

   // We only add the basic lighting options that all lighting
   // systems would use... the specific lighting system options
   // are injected at runtime by the lighting system itself.

   addGroup( "Lighting" );

      addField( "castShadows", TypeBool, Offset( mCastShadows, ScatterSky ),
         "Enables/disables shadows cast by objects due to ScatterSky light." );

      addField("staticRefreshFreq", TypeS32, Offset(mStaticRefreshFreq, ScatterSky), "static shadow refresh rate (milliseconds)");
      addField("dynamicRefreshFreq", TypeS32, Offset(mDynamicRefreshFreq, ScatterSky), "dynamic shadow refresh rate (milliseconds)");

      addField( "brightness", TypeF32, Offset( mBrightness, ScatterSky ),
         "The brightness of the ScatterSky's light object." );

   endGroup( "Lighting" );

   addGroup( "Misc" );

      addField( "flareType", TYPEID< LightFlareData >(), Offset( mFlareData, ScatterSky ),
         "Datablock for the flare produced by the ScatterSky." );

      addField( "flareScale", TypeF32, Offset( mFlareScale, ScatterSky ),
         "Changes the size and intensity of the flare." );

   endGroup( "Misc" );

   addGroup( "Night" );

      addField( "nightColor", TypeColorF, Offset( mNightColor, ScatterSky ),
         "The ambient color during night. Also used for the sky color if useNightCubemap is false." );

      addField( "nightFogColor", TypeColorF, Offset( mNightFogColor, ScatterSky ),
         "The fog color during night." );

      addField( "moonEnabled", TypeBool, Offset( mMoonEnabled, ScatterSky ),
         "Enable or disable rendering of the moon sprite during night." );

      addField( "moonMat", TypeMaterialName, Offset( mMoonMatName, ScatterSky ),
         "Material for the moon sprite." );

      addField( "moonScale", TypeF32, Offset( mMoonScale, ScatterSky ),
         "Controls size the moon sprite renders, specified as a fractional amount of the screen height." );

      addField( "moonLightColor", TypeColorF, Offset( mMoonTint, ScatterSky ),
         "Color of light cast by the directional light during night." );

      addField( "useNightCubemap", TypeBool, Offset( mUseNightCubemap, ScatterSky ),
         "Transition to the nightCubemap during night. If false we use nightColor." );

      addField( "nightCubemap", TypeCubemapName, Offset( mNightCubemapName, ScatterSky ),
         "Cubemap visible during night." );

   endGroup( "Night" );

   // Now inject any light manager specific fields.
   LightManager::initLightFields();

   Parent::initPersistFields();
}

U32 ScatterSky::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if ( stream->writeFlag( mask & TimeMask ) )
   {
      stream->write( mSunAzimuth );
      stream->write( mSunElevation );
   }

   if ( stream->writeFlag( mask & UpdateMask ) )
   {
      stream->write( mRayleighScattering );
      mRayleighScattering4PI = mRayleighScattering * 4.0f * M_PI_F;

      stream->write( mRayleighScattering4PI );

      stream->write( mMieScattering );
      mMieScattering4PI = mMieScattering * 4.0f * M_PI_F;

      stream->write( mMieScattering4PI );

      stream->write( mSunSize );

      stream->write( mSkyBrightness );

      stream->write( mMiePhaseAssymetry );

      stream->write( mSphereInnerRadius );
      stream->write( mSphereOuterRadius );

      stream->write( mScale );

      stream->write( mWavelength );

      stream->write( mWavelength4[0] );
      stream->write( mWavelength4[1] );
      stream->write( mWavelength4[2] );

      stream->write( mRayleighScaleDepth );
      stream->write( mMieScaleDepth );

      stream->write( mNightColor );
      stream->write( mNightFogColor );
      stream->write( mAmbientScale );
      stream->write( mSunScale );
      stream->write( mFogScale );
      stream->write( mColorizeAmt );
      stream->write( mColorize );

      stream->write( mExposure );

      stream->write( mZOffset );

      stream->write( mBrightness );

      stream->writeFlag( mCastShadows );
      stream->write(mStaticRefreshFreq);
      stream->write(mDynamicRefreshFreq);

      stream->write( mFlareScale );

      if ( stream->writeFlag( mFlareData ) )
      {
         stream->writeRangedU32( mFlareData->getId(),
                                 DataBlockObjectIdFirst,
                                 DataBlockObjectIdLast );
      }

      stream->writeFlag( mMoonEnabled );
      stream->write( mMoonMatName );
      stream->write( mMoonScale );
      stream->write( mMoonTint );
      stream->writeFlag( mUseNightCubemap );
      stream->write( mNightCubemapName );

      stream->write( mMoonAzimuth );
      stream->write( mMoonElevation );

      mLight->packExtended( stream );
   }

   return retMask;
}

void ScatterSky::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   if ( stream->readFlag() ) // TimeMask
   {
      F32 temp = 0;
      stream->read( &temp );
      setAzimuth( temp );

      stream->read( &temp );
      setElevation( temp );
   }

   if ( stream->readFlag() ) // UpdateMask
   {
      stream->read( &mRayleighScattering );
      stream->read( &mRayleighScattering4PI );

      stream->read( &mMieScattering );
      stream->read( &mMieScattering4PI );

      stream->read( &mSunSize );

      stream->read( &mSkyBrightness );

      stream->read( &mMiePhaseAssymetry );

      stream->read( &mSphereInnerRadius );
      stream->read( &mSphereOuterRadius );

      stream->read( &mScale );

      ColorF tmpColor( 0, 0, 0 );

      stream->read( &tmpColor );

      stream->read( &mWavelength4[0] );
      stream->read( &mWavelength4[1] );
      stream->read( &mWavelength4[2] );

      stream->read( &mRayleighScaleDepth );
      stream->read( &mMieScaleDepth );

      stream->read( &mNightColor );
      stream->read( &mNightFogColor );
      stream->read( &mAmbientScale );
      stream->read( &mSunScale );
      stream->read( &mFogScale );
      F32 colorizeAmt;
      stream->read( &colorizeAmt );

      if(mColorizeAmt != colorizeAmt) {
         mColorizeAmt = colorizeAmt;
         mShader = NULL; //forces shader refresh
      }

      stream->read( &mColorize );


      if ( tmpColor != mWavelength )
      {
         mWavelength = tmpColor;
         mWavelength4[0] = mPow(mWavelength[0], 4.0f);
         mWavelength4[1] = mPow(mWavelength[1], 4.0f);
         mWavelength4[2] = mPow(mWavelength[2], 4.0f);
      }

      stream->read( &mExposure );

      stream->read( &mZOffset );

      stream->read( &mBrightness );

      mCastShadows = stream->readFlag();
      stream->read(&mStaticRefreshFreq);
      stream->read(&mDynamicRefreshFreq);

      stream->read( &mFlareScale );

      if ( stream->readFlag() )
      {
         SimObjectId id = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
         LightFlareData *datablock = NULL;

         if ( Sim::findObject( id, datablock ) )
            mFlareData = datablock;
         else
         {
            con->setLastError( "ScatterSky::unpackUpdate() - invalid LightFlareData!" );
            mFlareData = NULL;
         }
      }
      else
         mFlareData = NULL;

      mMoonEnabled = stream->readFlag();
      stream->read( &mMoonMatName );
      stream->read( &mMoonScale );
      stream->read( &mMoonTint );
      mUseNightCubemap = stream->readFlag();
      stream->read( &mNightCubemapName );

      stream->read( &mMoonAzimuth );
      stream->read( &mMoonElevation );

      mLight->unpackExtended( stream );

      if ( isProperlyAdded() )
      {
         mDirty = true;
         _initMoon();
         Sim::findObject( mNightCubemapName, mNightCubemap );
      }
   }
}

void ScatterSky::prepRenderImage( SceneRenderState *state )
{
   // Only render into diffuse and reflect passes.

   if( !state->isDiffusePass() &&
       !state->isReflectPass() )
      return;

   // Regular sky render instance.
   RenderPassManager* renderPass = state->getRenderPass();
   ObjectRenderInst *ri = renderPass->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind( this, &ScatterSky::_render );
   ri->type = RenderPassManager::RIT_Sky;
   ri->defaultKey = 10;
   ri->defaultKey2 = 0;
   renderPass->addInst(ri);

   // Debug render instance.
   /*
   if ( Con::getBoolVariable( "$ScatterSky::debug", false ) )
   {
      ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind( this, &ScatterSky::_debugRender );
      ri->type = RenderPassManager::RIT_Editor;
      state->getRenderPass()->addInst( ri );
   }
   */

   // Light flare effect render instance.
   if ( mFlareData && mNightInterpolant != 1.0f )
   {
      mFlareState.fullBrightness = mBrightness;
      mFlareState.scale = mFlareScale;
      mFlareState.lightInfo = mLight;

      Point3F lightPos = state->getCameraPosition() - state->getFarPlane() * mLight->getDirection() * 0.9f;
      mFlareState.lightMat.identity();
      mFlareState.lightMat.setPosition( lightPos );

      F32 dist = ( lightPos - state->getCameraPosition( ) ).len( );
      F32 coronaScale = 0.5f;
      F32 screenRadius = GFX->getViewport( ).extent.y * coronaScale * 0.5f;
      mFlareState.worldRadius = screenRadius * dist / state->getWorldToScreenScale( ).y;

      mFlareData->prepRender( state, &mFlareState );
   }

   // Render instances for Night effects.
   if ( mNightInterpolant <= 0.0f )
      return;

   // Render instance for Moon sprite.
   if ( mMoonEnabled && mMoonMatInst )
   {
      mMatrixSet->setSceneView(GFX->getWorldMatrix());
      mMatrixSet->setSceneProjection(GFX->getProjectionMatrix());
      mMatrixSet->setWorld(GFX->getWorldMatrix());

      ObjectRenderInst *ri = renderPass->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind( this, &ScatterSky::_renderMoon );
      ri->type = RenderPassManager::RIT_Sky;
      // Render after sky objects and before CloudLayer!
      ri->defaultKey = 5;
      ri->defaultKey2 = 0;
      renderPass->addInst(ri);
   }
}

bool ScatterSky::_initShader()
{
   ShaderData *shaderData;
   if ( !Sim::findObject( "ScatterSkyShaderData", shaderData ) )
   {
      Con::warnf( "ScatterSky::_initShader - failed to locate shader ScatterSkyShaderData!" );
      return false;
   }
      Vector<GFXShaderMacro> macros;
   if ( mColorizeAmt )
      macros.push_back( GFXShaderMacro( "USE_COLORIZE" ) );

   mShader = shaderData->getShader( macros );

   if ( !mShader )
      return false;

   if ( mStateBlock.isNull() )
   {
      GFXStateBlockData *data = NULL;
      if ( !Sim::findObject( "ScatterSkySBData", data ) )
         Con::warnf( "ScatterSky::_initShader - failed to locate ScatterSkySBData!" );
      else
         mStateBlock = GFX->createStateBlock( data->getState() );
     }

   if ( !mStateBlock )
      return false;

   mShaderConsts = mShader->allocConstBuffer();
   mModelViewProjSC = mShader->getShaderConstHandle( "$modelView" );

   // Camera height, cam height squared, scale and scale over depth.
   mMiscSC = mShader->getShaderConstHandle( "$misc" );

   // Inner and out radius, and inner and outer radius squared.
   mSphereRadiiSC = mShader->getShaderConstHandle( "$sphereRadii" );

   // Rayleigh sun brightness, mie sun brightness and 4 * PI * coefficients.
   mScatteringCoefficientsSC = mShader->getShaderConstHandle( "$scatteringCoeffs" );
   mCamPosSC = mShader->getShaderConstHandle( "$camPos" );
   mLightDirSC = mShader->getShaderConstHandle( "$lightDir" );
   mSunDirSC = mShader->getShaderConstHandle( "$sunDir" );
   mNightColorSC = mShader->getShaderConstHandle( "$nightColor" );
   mInverseWavelengthSC = mShader->getShaderConstHandle( "$invWaveLength" );
   mNightInterpolantAndExposureSC = mShader->getShaderConstHandle( "$nightInterpAndExposure" );
   mUseCubemapSC = mShader->getShaderConstHandle( "$useCubemap" );
   mColorizeSC = mShader->getShaderConstHandle( "$colorize" );

   return true;
}

void ScatterSky::_initVBIB()
{
   // Vertex Buffer...
   U32 vertStride = 50;
   U32 strideMinusOne = vertStride - 1;
   mVertCount = vertStride * vertStride;
   mPrimCount = strideMinusOne * strideMinusOne * 2;

   Point3F vertScale( 16.0f, 16.0f, 4.0f );

   F32 zOffset = -( mCos( mSqrt( 1.0f ) ) + 0.01f );

   mVB.set( GFX, mVertCount, GFXBufferTypeStatic );
   GFXVertexP *pVert = mVB.lock();
   if(!pVert) return;

   for ( U32 y = 0; y < vertStride; y++ )
   {
      F32 v = ( (F32)y / (F32)strideMinusOne - 0.5f ) * 2.0f;

      for ( U32 x = 0; x < vertStride; x++ )
      {
         F32 u = ( (F32)x / (F32)strideMinusOne - 0.5f ) * 2.0f;

         F32 sx = u;
         F32 sy = v;
         F32 sz = (mCos( mSqrt( sx*sx + sy*sy ) ) * 1.0f) + zOffset;
         //F32 sz = 1.0f;
         pVert->point.set( sx, sy, sz );
         pVert->point *= vertScale;

         pVert->point.normalize();
         pVert->point *= 200000.0f;

         pVert++;
      }
   }

   mVB.unlock();

   // Primitive Buffer...
   mPrimBuffer.set( GFX, mPrimCount * 3, mPrimCount, GFXBufferTypeStatic );

   U16 *pIdx = NULL;
   mPrimBuffer.lock(&pIdx);
   U32 curIdx = 0;

   for ( U32 y = 0; y < strideMinusOne; y++ )
   {
      for ( U32 x = 0; x < strideMinusOne; x++ )
      {
         U32 offset = x + y * vertStride;

         pIdx[curIdx] = offset;
         curIdx++;
         pIdx[curIdx] = offset + 1;
         curIdx++;
         pIdx[curIdx] = offset + vertStride + 1;
         curIdx++;

         pIdx[curIdx] = offset;
         curIdx++;
         pIdx[curIdx] = offset + vertStride + 1;
         curIdx++;
         pIdx[curIdx] = offset + vertStride;
         curIdx++;
      }
   }

   mPrimBuffer.unlock();
}

void ScatterSky::_initMoon()
{
   if ( isServerObject() )
      return;

   if ( mMoonMatInst )
      SAFE_DELETE( mMoonMatInst );

   if ( mMoonMatName.isNotEmpty() )
      mMoonMatInst = MATMGR->createMatInstance( mMoonMatName, MATMGR->getDefaultFeatures(), getGFXVertexFormat<GFXVertexPCT>() );
}

void ScatterSky::_initCurves()
{
   if ( mCurves->getSampleCount() > 0 )
      return;

   // Takes time of day (0-2) and returns
   // the night interpolant (0-1) day/night factor.
   // moonlight = 0, sunlight > 0
   mCurves[0].clear();
   mCurves[0].addPoint( 0.0f, 0.5f );// Sunrise
   mCurves[0].addPoint( 0.025f, 1.0f );//
   mCurves[0].addPoint( 0.975f, 1.0f );//
   mCurves[0].addPoint( 1.0f, 0.5f );//Sunset
   mCurves[0].addPoint( 1.02f, 0.0f );//Sunlight ends
   mCurves[0].addPoint( 1.98f, 0.0f );//Sunlight begins
   mCurves[0].addPoint( 2.0f, 0.5f );// Sunrise

    //  Takes time of day (0-2) and returns mieScattering factor
   //   Regulates the size of the sun's disk
   mCurves[1].clear();
   mCurves[1].addPoint( 0.0f, 0.0006f );
   mCurves[1].addPoint( 0.01f, 0.00035f );
   mCurves[1].addPoint( 0.03f, 0.00023f );
   mCurves[1].addPoint( 0.1f, 0.00022f );
   mCurves[1].addPoint( 0.2f, 0.00043f );
   mCurves[1].addPoint( 0.3f, 0.00062f );
   mCurves[1].addPoint( 0.4f, 0.0008f );
   mCurves[1].addPoint( 0.5f, 0.00086f );// High noon
   mCurves[1].addPoint( 0.6f, 0.0008f );
   mCurves[1].addPoint( 0.7f, 0.00062f );
   mCurves[1].addPoint( 0.8f, 0.00043f );
   mCurves[1].addPoint( 0.9f, 0.00022f );
   mCurves[1].addPoint( 0.97f, 0.00023f );
   mCurves[1].addPoint( 0.99f, 0.00035f );
   mCurves[1].addPoint( 1.0f, 0.0006f );
   mCurves[1].addPoint( 2.0f, 0.0006f );

   // Takes time of day and returns brightness
   // Controls sunlight and moonlight brightness
   mCurves[2].clear();
   mCurves[2].addPoint( 0.0f, 0.2f );// Sunrise
   mCurves[2].addPoint( 0.1f, 1.0f );
   mCurves[2].addPoint( 0.9f, 1.0f );// Sunset
   mCurves[2].addPoint( 1.008f, 0.0f );//Adjust end of sun's reflection
   mCurves[2].addPoint( 1.02001f, 0.0f );
   mCurves[2].addPoint( 1.05f, 0.5f );// Turn brightness up for moonlight
   mCurves[2].addPoint( 1.93f, 0.5f );
   mCurves[2].addPoint( 1.97999f, 0.0f );// No brightness when sunlight starts
   mCurves[2].addPoint( 1.992f, 0.0f );//Adjust start of sun's reflection
   mCurves[2].addPoint( 2.0f, 0.2f ); // Sunrise

   // Interpolation of day/night color sets
   // 0/1  ambient/nightcolor
   // 0 = day colors only anytime
   // 1 = night colors only anytime
   // between 0 and 1 renders both color sets anytime

   mCurves[3].clear();
   mCurves[3].addPoint( 0.0f, 0.8f );//Sunrise
   mCurves[3].addPoint( 0.1f, 0.0f );
   mCurves[3].addPoint( 0.99f, 0.0f );
   mCurves[3].addPoint( 1.0f, 0.8f );// Sunset
   mCurves[3].addPoint( 1.01999f, 1.0f );//
   mCurves[3].addPoint( 1.98001f, 1.0f );// Sunlight begins with full night colors
   mCurves[3].addPoint( 2.0f, 0.8f );  //Sunrise

   //  Takes time of day (0-2) and returns smoothing factor
   //  Interpolates between mMoonTint color and mNightColor

   mCurves[4].clear();
   mCurves[4].addPoint( 0.0f, 1.0f );
   mCurves[4].addPoint( 0.96f, 1.0f );
   mCurves[4].addPoint( 1.01999f, 0.5f );
   mCurves[4].addPoint( 1.02001f, 0.5f );
   mCurves[4].addPoint( 1.08f, 1.0f );
   mCurves[4].addPoint( 1.92f, 1.0f );
   mCurves[4].addPoint( 1.97999f, 0.5f );
   mCurves[4].addPoint( 1.98001f, 0.5f );
   mCurves[4].addPoint( 2.0f, 1.0f );
}
void ScatterSky::_updateTimeOfDay( TimeOfDay *timeOfDay, F32 time )
{
   setElevation( timeOfDay->getElevationDegrees() );
   setAzimuth( timeOfDay->getAzimuthDegrees() );
}

void ScatterSky::_render( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   if ( overrideMat || (!mShader && !_initShader()) )
      return;

   GFXTransformSaver saver;

   if ( mVB.isNull() || mPrimBuffer.isNull() )
      _initVBIB();

   GFX->setShader( mShader );
   GFX->setShaderConstBuffer( mShaderConsts );

   Point4F sphereRadii( mSphereOuterRadius, mSphereOuterRadius * mSphereOuterRadius,
                        mSphereInnerRadius, mSphereInnerRadius * mSphereInnerRadius );

   Point4F scatteringCoeffs( mRayleighScattering * mSkyBrightness, mRayleighScattering4PI,
                             mMieScattering * mSkyBrightness, mMieScattering4PI );

   Point4F invWavelength(  1.0f / mWavelength4[0],
                           1.0f / mWavelength4[1],
                           1.0f / mWavelength4[2], 1.0f );

   Point3F camPos( 0, 0, smViewerHeight );
   Point4F miscParams( camPos.z, camPos.z * camPos.z, mScale, mScale / mRayleighScaleDepth );

   Frustum frust = state->getCameraFrustum();
   frust.setFarDist( smEarthRadius + smAtmosphereRadius );
   MatrixF proj( true );
   frust.getProjectionMatrix( &proj );

   Point3F camPos2 = state->getCameraPosition();
   MatrixF xfm(true);
   
   GFX->multWorld(xfm);
   MatrixF xform(proj);//GFX->getProjectionMatrix());
   xform *= GFX->getViewMatrix();
   xform *=  GFX->getWorldMatrix();

   if(state->isReflectPass())
   {
      static MatrixF rotMat(EulerF(0.0, 0.0, M_PI_F));
      xform.mul(rotMat);
      rotMat.set(EulerF(M_PI_F, 0.0, 0.0));
      xform.mul(rotMat);
   }
   xform.setPosition(xform.getPosition() - Point3F(0, 0, mZOffset));

   mShaderConsts->setSafe( mModelViewProjSC, xform );
   mShaderConsts->setSafe( mMiscSC, miscParams );
   mShaderConsts->setSafe( mSphereRadiiSC, sphereRadii );
   mShaderConsts->setSafe( mScatteringCoefficientsSC, scatteringCoeffs );
   mShaderConsts->setSafe( mCamPosSC, camPos );
   mShaderConsts->setSafe( mLightDirSC, mLightDir );
   mShaderConsts->setSafe( mSunDirSC, mSunDir );
   mShaderConsts->setSafe( mNightColorSC, mNightColor );
   mShaderConsts->setSafe( mInverseWavelengthSC, invWavelength );
   mShaderConsts->setSafe( mNightInterpolantAndExposureSC, Point2F( mExposure, mNightInterpolant ) );
   mShaderConsts->setSafe( mColorizeSC, mColorize*mColorizeAmt );

   if ( GFXDevice::getWireframe() )
   {
      GFXStateBlockDesc desc( mStateBlock->getDesc() );
      desc.setFillModeWireframe();
      GFX->setStateBlockByDesc( desc );
   }
   else
      GFX->setStateBlock( mStateBlock );

   if ( mUseNightCubemap && mNightCubemap )
   {
      mShaderConsts->setSafe( mUseCubemapSC, 1.0f );

      if ( !mNightCubemap->mCubemap )
         mNightCubemap->createMap();

      GFX->setCubeTexture( 0, mNightCubemap->mCubemap );
   }
   else
   {
      GFX->setCubeTexture( 0, NULL );
      mShaderConsts->setSafe( mUseCubemapSC, 0.0f );
   }

   GFX->setPrimitiveBuffer( mPrimBuffer );
   GFX->setVertexBuffer( mVB );

   GFX->drawIndexedPrimitive( GFXTriangleList, 0, 0, mVertCount, 0, mPrimCount );
}

void ScatterSky::_debugRender( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   GFXStateBlockDesc desc;
   desc.fillMode = GFXFillSolid;
   desc.setBlend( false, GFXBlendOne, GFXBlendZero );
   desc.setZReadWrite( false, false );
   GFXStateBlockRef sb = GFX->GFX->createStateBlock( desc );

   GFX->setStateBlock( sb );

   PrimBuild::begin( GFXLineStrip, mSkyPoints.size() );
   PrimBuild::color3i( 255, 0, 255 );

   for ( U32 i = 0; i < mSkyPoints.size(); i++ )
   {
      Point3F pnt = mSkyPoints[i];
      pnt.normalize();
      pnt *= 500;
      pnt += state->getCameraPosition();
      PrimBuild::vertex3fv( pnt );
   }

   PrimBuild::end();
}

void ScatterSky::_renderMoon( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   if ( !mMoonMatInst )
      return;

   Point3F moonlightPosition = state->getCameraPosition() - /*mLight->getDirection()*/ mMoonLightDir * state->getFarPlane() * 0.9f;
   F32 dist = (moonlightPosition - state->getCameraPosition()).len();

   // worldRadius = screenRadius * dist / worldToScreen
   // screenRadius = worldRadius / dist * worldToScreen

   //
   F32 screenRadius = GFX->getViewport().extent.y * mMoonScale * 0.5f;
   F32 worldRadius = screenRadius * dist / state->getWorldToScreenScale().y;

   // Calculate Billboard Radius (in world units) to be constant, independent of distance.
   // Takes into account distance, viewport size, and specified size in editor

   F32 BBRadius = worldRadius;


   mMatrixSet->restoreSceneViewProjection();

   if ( state->isReflectPass() )
      mMatrixSet->setProjection( state->getSceneManager()->getNonClipProjection() );

   mMatrixSet->setWorld( MatrixF::Identity );

   // Initialize points with basic info
   Point3F points[4];
   points[0] = Point3F( -BBRadius, 0.0, -BBRadius);
   points[1] = Point3F( -BBRadius, 0.0, BBRadius);
   points[2] = Point3F( BBRadius, 0.0, -BBRadius);
   points[3] = Point3F( BBRadius, 0.0, BBRadius);

   static const Point2F sCoords[4] =
   {
      Point2F( 0.0f, 0.0f ),
      Point2F( 0.0f, 1.0f ),
      Point2F( 1.0f, 0.0f ),
      Point2F( 1.0f, 1.0f )
   };

   // Get info we need to adjust points
   const MatrixF &camView = state->getCameraTransform();

   // Finalize points
   for(S32 i = 0; i < 4; i++)
   {
      // align with camera
      camView.mulV(points[i]);
      // offset
      points[i] += moonlightPosition;
   }

   // Vertex color.
   ColorF moonVertColor( 1.0f, 1.0f, 1.0f, mNightInterpolant );

   // Copy points to buffer.

   GFXVertexBufferHandle< GFXVertexPCT > vb;
   vb.set( GFX, 4, GFXBufferTypeVolatile );
   GFXVertexPCT *pVert = vb.lock();
   if(!pVert) return;

   for ( S32 i = 0; i < 4; i++ )
   {
      pVert->color.set( moonVertColor );
      pVert->point.set( points[i] );
      pVert->texCoord.set( sCoords[i].x, sCoords[i].y );
      pVert++;
   }

   vb.unlock();

   // Setup SceneData struct.

   SceneData sgData;
   sgData.wireframe = GFXDevice::getWireframe();
   sgData.visibility = 1.0f;

   // Draw it

   while ( mMoonMatInst->setupPass( state, sgData ) )
   {
      mMoonMatInst->setTransforms( *mMatrixSet, state );
      mMoonMatInst->setSceneInfo( state, sgData );

      GFX->setVertexBuffer( vb );
      GFX->drawPrimitive( GFXTriangleStrip, 0, 2 );
   }
}

void ScatterSky::_generateSkyPoints()
{
   U32 rings=60, segments=20;//rings=160, segments=20;

   Point3F tmpPoint( 0, 0, 0 );

   // Establish constants used in sphere generation.
   F32 deltaRingAngle = ( M_PI_F / (F32)(rings * 2) );
   F32 deltaSegAngle = ( 2.0f * M_PI_F / (F32)segments );

   // Generate the group of rings for the sphere.
   for( S32 ring = 0; ring < 2; ring++ )
   {
      F32 r0 = mSin( ring * deltaRingAngle );
      F32 y0 = mCos( ring * deltaRingAngle );

      // Generate the group of segments for the current ring.
      for( S32 seg = 0; seg < segments + 1 ; seg++ )
      {
         F32 x0 = r0 * sinf( seg * deltaSegAngle );
         F32 z0 = r0 * cosf( seg * deltaSegAngle );

         tmpPoint.set( x0, z0, y0 );
         tmpPoint.normalizeSafe();

         tmpPoint.x *= smEarthRadius + smAtmosphereRadius;
         tmpPoint.y *= smEarthRadius + smAtmosphereRadius;
         tmpPoint.z *= smEarthRadius + smAtmosphereRadius;
         tmpPoint.z -= smEarthRadius;

         if ( ring == 1 )
            mSkyPoints.push_back( tmpPoint );
      }
   }
}

void ScatterSky::_interpolateColors()
{
   mFogColor.set( 0, 0, 0, 0 );
   mAmbientColor.set( 0, 0, 0, 0 );
   mSunColor.set( 0, 0, 0, 0 );

   _getFogColor( &mFogColor );
   _getAmbientColor( &mAmbientColor );
   _getSunColor( &mSunColor );

   mAmbientColor *= mAmbientScale;
   mSunColor *= mSunScale;
   mFogColor *= mFogScale;

   mMieScattering = (mCurves[1].getVal( mTimeOfDay) * mSunSize ); //Scale the size of the sun's disk

   ColorF moonTemp = mMoonTint;
   ColorF nightTemp = mNightColor;

   moonTemp.interpolate( mNightColor, mMoonTint, mCurves[4].getVal( mTimeOfDay ) );
   nightTemp.interpolate( mMoonTint, mNightColor, mCurves[4].getVal( mTimeOfDay ) );

   mFogColor.interpolate( mFogColor, mNightFogColor, mCurves[3].getVal( mTimeOfDay ) );//mNightInterpolant );
   mFogColor.alpha = 1.0f;

   mAmbientColor.interpolate( mAmbientColor, mNightColor, mCurves[3].getVal( mTimeOfDay ) );//mNightInterpolant );
   mSunColor.interpolate( mSunColor, mMoonTint, mCurves[3].getVal( mTimeOfDay ) );//mNightInterpolant );
}

void ScatterSky::_getSunColor( ColorF *outColor )
{
   PROFILE_SCOPE( ScatterSky_GetSunColor );

   U32 count = 0;
   ColorF tmpColor( 0, 0, 0 );
   VectorF tmpVec( 0, 0, 0 );

   tmpVec = mLightDir;
   tmpVec.x *= smEarthRadius + smAtmosphereRadius;
   tmpVec.y *= smEarthRadius + smAtmosphereRadius;
   tmpVec.z *= smEarthRadius + smAtmosphereRadius;
   tmpVec.z -= smAtmosphereRadius;

   for ( U32 i = 0; i < 10; i++ )
   {
      _getColor( tmpVec, &tmpColor );
      (*outColor) += tmpColor;
      tmpVec.x += (smEarthRadius * 0.5f) + (smAtmosphereRadius * 0.5f);
      count++;
   }

   if ( count > 0 )
      (*outColor) /= count;
}

void ScatterSky::_getAmbientColor( ColorF *outColor )
{
   PROFILE_SCOPE( ScatterSky_GetAmbientColor );

   ColorF tmpColor( 0, 0, 0, 0 );
   U32 count = 0;

   // Disable mieScattering for purposes of calculating the ambient color.
   F32 oldMieScattering = mMieScattering;
   mMieScattering = 0.0f;

   for ( U32 i = 0; i < mSkyPoints.size(); i++ )
   {
      Point3F pnt( mSkyPoints[i] );

      _getColor( pnt, &tmpColor );
      (*outColor) += tmpColor;
      count++;
   }

   if ( count > 0 )
      (*outColor) /= count;
   mMieScattering = oldMieScattering;
}

void ScatterSky::_getFogColor( ColorF *outColor )
{
   PROFILE_SCOPE( ScatterSky_GetFogColor );

   VectorF scatterPos( 0, 0, 0 );

   F32 sunBrightness = mSkyBrightness;
   mSkyBrightness *= 0.25f;

   F32 yaw = 0, pitch = 0, originalYaw = 0;
   VectorF fwd( 0, 1.0f, 0 );
   MathUtils::getAnglesFromVector( fwd, yaw, pitch );
   originalYaw = yaw;
   pitch = mDegToRad( 10.0f );

   ColorF tmpColor( 0, 0, 0 );

   U32 i = 0;
   for ( i = 0; i < 10; i++ )
   {
      MathUtils::getVectorFromAngles( scatterPos, yaw, pitch );

      scatterPos.x *= smEarthRadius + smAtmosphereRadius;
      scatterPos.y *= smEarthRadius + smAtmosphereRadius;
      scatterPos.z *= smEarthRadius + smAtmosphereRadius;
      scatterPos.y -= smEarthRadius;

      _getColor( scatterPos, &tmpColor );
      (*outColor) += tmpColor;

      if ( i <= 5 )
         yaw += mDegToRad( 5.0f );
      else
      {
         originalYaw += mDegToRad( -5.0f );
         yaw = originalYaw;
      }

      yaw = mFmod( yaw, M_2PI_F );
   }

   if ( i > 0 )
      (*outColor) /= i;

   mSkyBrightness = sunBrightness;
}

F32 ScatterSky::_vernierScale( F32 fCos )
{
   F32 x = 1.0 - fCos;
   return 0.25f * exp( -0.00287f + x * (0.459f + x * (3.83f + x * ((-6.80f + (x * 5.25f))))) );
}

F32 ScatterSky::_getMiePhase( F32 fCos, F32 fCos2, F32 g, F32 g2)
{
   return 1.5f * ((1.0f - g2) / (2.0f + g2)) * (1.0f + fCos2) / mPow(mFabs(1.0f + g2 - 2.0f*g*fCos), 1.5f);
}

F32 ScatterSky::_getRayleighPhase( F32 fCos2 )
{
   return 0.75 + 0.75 * fCos2;
}

void ScatterSky::_getColor( const Point3F &pos, ColorF *outColor )
{
   PROFILE_SCOPE( ScatterSky_GetColor );

   F32 scaleOverScaleDepth = mScale / mRayleighScaleDepth;
   F32 rayleighBrightness = mRayleighScattering * mSkyBrightness;
   F32 mieBrightness = mMieScattering * mSkyBrightness;

   Point3F invWaveLength(  1.0f / mWavelength4[0],
                           1.0f / mWavelength4[1],
                           1.0f / mWavelength4[2] );

   Point3F v3Pos = pos / 6378000.0f;
   v3Pos.z += mSphereInnerRadius;

   Point3F newCamPos( 0, 0, smViewerHeight );

   VectorF v3Ray = v3Pos - newCamPos;
   F32 fFar = v3Ray.len();
   v3Ray / fFar;
   v3Ray.normalizeSafe();

   Point3F v3Start = newCamPos;
   F32 fDepth = mExp( scaleOverScaleDepth * (mSphereInnerRadius - smViewerHeight ) );
   F32 fStartAngle = mDot( v3Ray, v3Start );

   F32 fStartOffset = fDepth * _vernierScale( fStartAngle );

   F32 fSampleLength = fFar / 2.0f;
   F32 fScaledLength = fSampleLength * mScale;
   VectorF v3SampleRay = v3Ray * fSampleLength;
   Point3F v3SamplePoint = v3Start + v3SampleRay * 0.5f;

   Point3F v3FrontColor( 0, 0, 0 );
   for ( U32 i = 0; i < 2; i++ )
   {
      F32 fHeight = v3SamplePoint.len();
      F32 fDepth = mExp( scaleOverScaleDepth * (mSphereInnerRadius - smViewerHeight) );
      F32 fLightAngle = mDot( mLightDir, v3SamplePoint ) / fHeight;
      F32 fCameraAngle = mDot( v3Ray, v3SamplePoint ) / fHeight;

      F32 fScatter = (fStartOffset + fDepth * ( _vernierScale( fLightAngle ) - _vernierScale( fCameraAngle ) ));
      Point3F v3Attenuate( 0, 0, 0 );

      F32 tmp = mExp( -fScatter * (invWaveLength[0] * mRayleighScattering4PI + mMieScattering4PI) );
      v3Attenuate.x = tmp;

      tmp = mExp( -fScatter * (invWaveLength[1] * mRayleighScattering4PI + mMieScattering4PI) );
      v3Attenuate.y = tmp;

      tmp = mExp( -fScatter * (invWaveLength[2] * mRayleighScattering4PI + mMieScattering4PI) );
      v3Attenuate.z = tmp;

      v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
      v3SamplePoint += v3SampleRay;
   }

   Point3F mieColor = v3FrontColor * mieBrightness;
   Point3F rayleighColor = v3FrontColor * (invWaveLength * rayleighBrightness);
   Point3F v3Direction = newCamPos - v3Pos;
   v3Direction.normalize();

   F32 fCos = mDot( mLightDir, v3Direction ) / v3Direction.len();
   F32 fCos2 = fCos * fCos;

   F32 g = -0.991f;
   F32 g2 = g * g;
   F32 miePhase = _getMiePhase( fCos, fCos2, g, g2 );

   Point3F color = rayleighColor + (miePhase * mieColor);
   ColorF tmp( color.x, color.y, color.z, color.y );

   Point3F expColor( 0, 0, 0 );
   expColor.x = 1.0f - exp(-mExposure * color.x);
   expColor.y = 1.0f - exp(-mExposure * color.y);
   expColor.z = 1.0f - exp(-mExposure * color.z);

   tmp.set( expColor.x, expColor.y, expColor.z, 1.0f );

   if ( !tmp.isValidColor() )
   {
      F32 len = expColor.len();
      if ( len > 0 )
         expColor /= len;
   }

   outColor->set( expColor.x, expColor.y, expColor.z, 1.0f );
}

// Static protected field set methods

bool ScatterSky::ptSetElevation( void *object, const char *index, const char *data )
{
   ScatterSky *sky = static_cast<ScatterSky*>( object );
   F32 val = dAtof( data );

   sky->setElevation( val );

   // we already set the field
   return false;
}

bool ScatterSky::ptSetAzimuth( void *object, const char *index, const char *data )
{
   ScatterSky *sky = static_cast<ScatterSky*>( object );
   F32 val = dAtof( data );

   sky->setAzimuth( val );

   // we already set the field
   return false;
}

void ScatterSky::_onSelected()
{
#ifdef TORQUE_DEBUG
   // Enable debug rendering on the light.
   if( isClientObject() )
      mLight->enableDebugRendering( true );
#endif

   Parent::_onSelected();
}

void ScatterSky::_onUnselected()
{
#ifdef TORQUE_DEBUG
   // Disable debug rendering on the light.
   if( isClientObject() )
      mLight->enableDebugRendering( false );
#endif

   Parent::_onUnselected();
}

// ConsoleMethods

DefineEngineMethod( ScatterSky, applyChanges, void, (),,
                   "Apply a full network update of all fields to all clients."
                  )
{
   object->inspectPostApply();
}