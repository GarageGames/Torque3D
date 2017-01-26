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
#include "T3D/fx/precipitation.h"

#include "math/mathIO.h"
#include "console/consoleTypes.h"
#include "console/typeValidators.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "lighting/lightInfo.h"
#include "lighting/lightManager.h"
#include "materials/shaderData.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/player.h"
#include "core/stream/bitStream.h"
#include "platform/profiler.h"
#include "renderInstance/renderPassManager.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxSource.h"
#include "sfx/sfxTypes.h"
#include "console/engineAPI.h"
#include "particleEmitter.h"

static const U32 dropHitMask = 
   TerrainObjectType |
   WaterObjectType |
   StaticShapeObjectType;

IMPLEMENT_CO_NETOBJECT_V1(Precipitation);
IMPLEMENT_CO_DATABLOCK_V1(PrecipitationData);

ConsoleDocClass( Precipitation,
   "@brief Defines a precipitation based storm (rain, snow, etc).\n\n"

   "The Precipitation effect works by creating many 'drops' within a fixed size "
   "box. This box can be configured to move around with the camera (to simulate "
   "level-wide precipitation), or to remain in a fixed position (to simulate "
   "localized precipitation). When #followCam is true, the box containing the "
   "droplets can be thought of as centered on the camera then pushed slightly "
   "forward in the direction the camera is facing so most of the box is in "
   "front of the camera (allowing more drops to be visible on screen at once).\n\n"

   "The effect can also be configured to create a small 'splash' whenever a drop "
   "hits another world object.\n\n"

   "@tsexample\n"
   "// The following is added to a level file (.mis) by the World Editor\n"
   "new Precipitation( TheRain )\n"
   "{\n"
   "   dropSize = \"0.5\";\n"
   "   splashSize = \"0.5\";\n"
   "   splashMS = \"250\";\n"
   "   animateSplashes = \"1\";\n"
   "   dropAnimateMS = \"0\";\n"
   "   fadeDist = \"0\";\n"
   "   fadeDistEnd = \"0\";\n"
   "   useTrueBillboards = \"0\";\n"
   "   useLighting = \"0\";\n"
   "   glowIntensity = \"0 0 0 0\";\n"
   "   reflect = \"0\";\n"
   "   rotateWithCamVel = \"1\";\n"
   "   doCollision = \"1\";\n"
   "   hitPlayers = \"0\";\n"
   "   hitVehicles = \"0\";\n"
   "   followCam = \"1\";\n"
   "   useWind = \"0\";\n"
   "   minSpeed = \"1.5\";\n"
   "   maxSpeed = \"2\";\n"
   "   minMass = \"0.75\";\n"
   "   maxMass = \"0.85\";\n"
   "   useTurbulence = \"0\";\n"
   "   maxTurbulence = \"0.1\";\n"
   "   turbulenceSpeed = \"0.2\";\n"
   "   numDrops = \"1024\";\n"
   "   boxWidth = \"200\";\n"
   "   boxHeight = \"100\";\n"
   "   dataBlock = \"HeavyRain\";\n"
   "};\n"
   "@endtsexample\n"
   "@ingroup FX\n"
   "@ingroup Atmosphere\n"
   "@see PrecipitationData\n"
);

ConsoleDocClass( PrecipitationData,
   "@brief Defines the droplets used in a storm (raindrops, snowflakes, etc).\n\n"
   "@tsexample\n"
   "datablock PrecipitationData( HeavyRain )\n"
   "{\n"
   "   soundProfile = \"HeavyRainSound\";\n"
   "   dropTexture = \"art/environment/precipitation/rain\";\n"
   "   splashTexture = \"art/environment/precipitation/water_splash\";\n"
   "   dropsPerSide = 4;\n"
   "   splashesPerSide = 2;\n"
   "};\n"
   "@endtsexample\n"
   "@ingroup FX\n"
   "@ingroup Atmosphere\n"
   "@see Precipitation\n"
);


//----------------------------------------------------------
// PrecipitationData
//----------------------------------------------------------
PrecipitationData::PrecipitationData()
{
   soundProfile      = NULL;

   mDropName         = StringTable->EmptyString();
   mDropShaderName   = StringTable->EmptyString();
   mSplashName       = StringTable->EmptyString();
   mSplashShaderName = StringTable->EmptyString();

   mDropsPerSide     = 4;
   mSplashesPerSide  = 2;
}

void PrecipitationData::initPersistFields()
{
   addField( "soundProfile", TYPEID< SFXTrack >(), Offset(soundProfile, PrecipitationData),
      "Looping SFXProfile effect to play while Precipitation is active." );
   addField( "dropTexture", TypeFilename, Offset(mDropName, PrecipitationData),
      "@brief Texture filename for drop particles.\n\n"
      "The drop texture can contain several different drop sub-textures "
      "arranged in a grid. There must be the same number of rows as columns. A "
      "random frame will be chosen for each drop." );
   addField( "dropShader", TypeString, Offset(mDropShaderName, PrecipitationData),
      "The name of the shader used for raindrops." );
   addField( "splashTexture", TypeFilename, Offset(mSplashName, PrecipitationData),
      "@brief Texture filename for splash particles.\n\n"
      "The splash texture can contain several different splash sub-textures "
      "arranged in a grid. There must be the same number of rows as columns. A "
      "random frame will be chosen for each splash." );
   addField( "splashShader", TypeString, Offset(mSplashShaderName, PrecipitationData),
      "The name of the shader used for splashes." );
   addField( "dropsPerSide", TypeS32, Offset(mDropsPerSide, PrecipitationData),
      "@brief How many rows and columns are in the raindrop texture.\n\n"
      "For example, if the texture has 16 raindrops arranged in a grid, this "
      "field should be set to 4." );
   addField( "splashesPerSide", TypeS32, Offset(mSplashesPerSide, PrecipitationData),
      "@brief How many rows and columns are in the splash texture.\n\n"
      "For example, if the texture has 9 splashes arranged in a grid, this "
      "field should be set to 3." );

   Parent::initPersistFields();
}

bool PrecipitationData::preload( bool server, String &errorStr )
{
   if( Parent::preload( server, errorStr) == false)
      return false;

   if( !server && !sfxResolve( &soundProfile, errorStr ) )
      return false;

   return true;
}

void PrecipitationData::packData(BitStream* stream)
{
   Parent::packData(stream);

   sfxWrite( stream, soundProfile );

   stream->writeString(mDropName);
   stream->writeString(mDropShaderName);
   stream->writeString(mSplashName);
   stream->writeString(mSplashShaderName);
   stream->write(mDropsPerSide);
   stream->write(mSplashesPerSide);
}

void PrecipitationData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   sfxRead( stream, &soundProfile );

   mDropName = stream->readSTString();
   mDropShaderName = stream->readSTString();
   mSplashName = stream->readSTString();
   mSplashShaderName = stream->readSTString();
   stream->read(&mDropsPerSide);
   stream->read(&mSplashesPerSide);
}

//----------------------------------------------------------
// Precipitation!
//----------------------------------------------------------
Precipitation::Precipitation()
{
   mTypeMask |= ProjectileObjectType;

   mDataBlock = NULL;

   mTexCoords = NULL;
   mSplashCoords = NULL;

   mDropShader = NULL;
   mDropHandle = NULL;

   mSplashShader = NULL;
   mSplashHandle = NULL;

   mDropHead   = NULL;
   mSplashHead = NULL;
   mNumDrops   = 1024;
   mPercentage = 1.0;

   mMinSpeed   = 1.5;
   mMaxSpeed   = 2.0;

   mFollowCam = true;

   mLastRenderFrame = 0;

   mDropHitMask = 0;

   mDropSize          = 0.5;
   mSplashSize        = 0.5;
   mUseTrueBillboards = false;
   mSplashMS          = 250;

   mAnimateSplashes  = true;
   mDropAnimateMS    = 0;

   mUseLighting = false;
   mGlowIntensity = ColorF( 0,0,0,0 );

   mReflect = false;

   mUseWind = false;

   mBoxWidth   = 200;
   mBoxHeight  = 100;
   mFadeDistance = 0;
   mFadeDistanceEnd = 0;

   mMinMass    = 0.75f;
   mMaxMass    = 0.85f;

   mMaxTurbulence = 0.1f;
   mTurbulenceSpeed = 0.2f;
   mUseTurbulence = false;

   mRotateWithCamVel = true;

   mDoCollision = true;
   mDropHitPlayers = false;
   mDropHitVehicles = false;

   mStormData.valid = false;
   mStormData.startPct = 0;
   mStormData.endPct = 0;
   mStormData.startTime = 0;
   mStormData.totalTime = 0;

   mTurbulenceData.valid = false;
   mTurbulenceData.startTime = 0;
   mTurbulenceData.totalTime = 0;
   mTurbulenceData.startMax = 0;
   mTurbulenceData.startSpeed = 0;
   mTurbulenceData.endMax = 0;
   mTurbulenceData.endSpeed = 0;

   mAmbientSound = NULL;

   mDropShaderModelViewSC = NULL;
   mDropShaderFadeStartEndSC = NULL;
   mDropShaderCameraPosSC = NULL;
   mDropShaderAmbientSC = NULL;

   mSplashShaderModelViewSC = NULL;
   mSplashShaderFadeStartEndSC = NULL;
   mSplashShaderCameraPosSC = NULL;
   mSplashShaderAmbientSC = NULL;

   mMaxVBDrops = 5000;
}

Precipitation::~Precipitation()
{
   SAFE_DELETE_ARRAY(mTexCoords);
   SAFE_DELETE_ARRAY(mSplashCoords);
}

void Precipitation::inspectPostApply()
{
   if (mFollowCam)
   {
      setGlobalBounds();
   }
   else
   {
      mObjBox.minExtents = -Point3F(mBoxWidth/2, mBoxWidth/2, mBoxHeight/2);
      mObjBox.maxExtents = Point3F(mBoxWidth/2, mBoxWidth/2, mBoxHeight/2);
   }

   resetWorldBox();
   setMaskBits(DataMask);
}

void Precipitation::setTransform(const MatrixF & mat)
{
   Parent::setTransform(mat);

   setMaskBits(TransformMask);
}

//--------------------------------------------------------------------------
// Console stuff...
//--------------------------------------------------------------------------

IRangeValidator ValidNumDropsRange(1, 100000);

void Precipitation::initPersistFields()
{
   addGroup("Precipitation");

      addFieldV( "numDrops", TypeS32, Offset(mNumDrops, Precipitation), &ValidNumDropsRange,
         "@brief Maximum number of drops allowed to exist in the precipitation "
         "box at any one time.\n\n"
         "The actual number of drops in the effect depends on the current "
         "percentage, which can change over time using modifyStorm()." );

      addField( "boxWidth", TypeF32, Offset(mBoxWidth, Precipitation),
         "Width and depth (horizontal dimensions) of the precipitation box." );

      addField( "boxHeight", TypeF32, Offset(mBoxHeight, Precipitation),
         "Height (vertical dimension) of the precipitation box." );

   endGroup("Precipitation");

   addGroup("Rendering");

      addField( "dropSize", TypeF32, Offset(mDropSize, Precipitation),
         "Size of each drop of precipitation. This will scale the texture." );

      addField( "splashSize", TypeF32, Offset(mSplashSize, Precipitation),
         "Size of each splash animation when a drop collides with another surface." );

      addField( "splashMS", TypeS32, Offset(mSplashMS, Precipitation),
         "Lifetime of splashes in milliseconds." );

      addField( "animateSplashes", TypeBool, Offset(mAnimateSplashes, Precipitation),
         "Set to true to enable splash animations when drops collide with other surfaces." );

      addField( "dropAnimateMS", TypeS32, Offset(mDropAnimateMS, Precipitation),
         "@brief Length (in milliseconds) to display each drop frame.\n\n"
         "If #dropAnimateMS <= 0, drops select a single random frame at creation "
         "that does not change throughout the drop's lifetime. If #dropAnimateMS "
         "> 0, each drop cycles through the the available frames in the drop "
         "texture at the given rate." );

      addField( "fadeDist", TypeF32, Offset(mFadeDistance, Precipitation),
         "The distance at which drops begin to fade out." );

      addField( "fadeDistEnd", TypeF32, Offset(mFadeDistanceEnd, Precipitation),
         "The distance at which drops are completely faded out." );

      addField( "useTrueBillboards", TypeBool, Offset(mUseTrueBillboards, Precipitation),
         "Set to true to make drops true (non axis-aligned) billboards." );

      addField( "useLighting", TypeBool, Offset(mUseLighting, Precipitation),
         "Set to true to enable shading of the drops and splashes by the sun color." );

      addField( "glowIntensity", TypeColorF, Offset(mGlowIntensity, Precipitation),
         "Set to 0 to disable the glow or or use it to control the intensity of each channel." );

      addField( "reflect", TypeBool, Offset(mReflect, Precipitation),
         "@brief This enables precipitation rendering during reflection passes.\n\n"
         "@note This is expensive." );

      addField( "rotateWithCamVel",  TypeBool, Offset(mRotateWithCamVel, Precipitation),
         "Set to true to include the camera velocity when calculating drop "
         "rotation speed." );

   endGroup("Rendering");

   addGroup("Collision");

      addField( "doCollision", TypeBool, Offset(mDoCollision, Precipitation),
         "@brief Allow drops to collide with world objects.\n\n"
         "If #animateSplashes is true, drops that collide with another object "
         "will produce a simple splash animation.\n"
         "@note This can be expensive as each drop will perform a raycast when "
         "it is created to determine where it will hit." );

      addField( "hitPlayers", TypeBool, Offset(mDropHitPlayers, Precipitation),
         "Allow drops to collide with Player objects; only valid if #doCollision is true." );

      addField( "hitVehicles", TypeBool, Offset(mDropHitVehicles, Precipitation),
         "Allow drops to collide with Vehicle objects; only valid if #doCollision is true." );

   endGroup("Collision");

   addGroup("Movement");

      addField( "followCam", TypeBool, Offset(mFollowCam, Precipitation),
         "@brief Controls whether the Precipitation system follows the camera "
         "or remains where it is first placed in the scene.\n\n"
         "Set to true to make it seem like it is raining everywhere in the "
         "level (ie. the Player will always be in the rain). Set to false "
         "to have a single area affected by rain (ie. the Player can move in "
         "and out of the rainy area)." );

      addField( "useWind", TypeBool, Offset(mUseWind, Precipitation),
         "Controls whether drops are affected by wind.\n"
         "@see ForestWindEmitter" );

      addField( "minSpeed", TypeF32, Offset(mMinSpeed, Precipitation),
         "@brief Minimum speed at which a drop will fall.\n\n"
         "On creation, the drop will be assigned a random speed between #minSpeed "
         "and #maxSpeed." );

      addField( "maxSpeed", TypeF32, Offset(mMaxSpeed, Precipitation),
         "@brief Maximum speed at which a drop will fall.\n\n"
         "On creation, the drop will be assigned a random speed between #minSpeed "
         "and #maxSpeed." );

      addField( "minMass", TypeF32, Offset(mMinMass, Precipitation),
         "@brief Minimum mass of a drop.\n\n"
         "Drop mass determines how strongly the drop is affected by wind and "
         "turbulence. On creation, the drop will be assigned a random speed "
         "between #minMass and #minMass." );

      addField( "maxMass", TypeF32, Offset(mMaxMass, Precipitation),
         "@brief Maximum mass of a drop.\n\n"
         "Drop mass determines how strongly the drop is affected by wind and "
         "turbulence. On creation, the drop will be assigned a random speed "
         "between #minMass and #minMass." );

   endGroup("Movement");

   addGroup("Turbulence");

      addField( "useTurbulence", TypeBool, Offset(mUseTurbulence, Precipitation),
         "Check to enable turbulence. This causes precipitation drops to spiral "
         "while falling." );

      addField( "maxTurbulence", TypeF32, Offset(mMaxTurbulence, Precipitation),
         "Radius at which precipitation drops spiral when turbulence is enabled." );

      addField( "turbulenceSpeed", TypeF32, Offset(mTurbulenceSpeed, Precipitation),
         "Speed at which precipitation drops spiral when turbulence is enabled." );

   endGroup("Turbulence");

   Parent::initPersistFields();
}

//-----------------------------------
// Console methods...
DefineEngineMethod(Precipitation, setPercentage, void, (F32 percentage), (1.0f),
   "Sets the maximum number of drops in the effect, as a percentage of #numDrops.\n"
   "The change occurs instantly (use modifyStorm() to change the number of drops "
   "over a period of time.\n"
   "@param percentage New maximum number of drops value (as a percentage of "
   "#numDrops). Valid range is 0-1.\n"
   "@tsexample\n"
   "%percentage = 0.5;  // The percentage, from 0 to 1, of the maximum drops to display\n"
   "%precipitation.setPercentage( %percentage );\n"
   "@endtsexample\n"
   "@see modifyStorm\n" )
{
   object->setPercentage(percentage);
}

DefineEngineMethod(Precipitation, modifyStorm, void, (F32 percentage, F32 seconds), (1.0f, 5.0f),
   "Smoothly change the maximum number of drops in the effect (from current "
   "value to #numDrops * @a percentage).\n"
   "This method can be used to simulate a storm building or fading in intensity "
   "as the number of drops in the Precipitation box changes.\n"
   "@param percentage New maximum number of drops value (as a percentage of "
   "#numDrops). Valid range is 0-1.\n"
   "@param seconds Length of time (in seconds) over which to increase the drops "
   "percentage value. Set to 0 to change instantly.\n"
   "@tsexample\n"
   "%percentage = 0.5;  // The percentage, from 0 to 1, of the maximum drops to display\n"
   "%seconds = 5.0;     // The length of time over which to make the change.\n"
   "%precipitation.modifyStorm( %percentage, %seconds );\n"
   "@endtsexample\n" )
{
   object->modifyStorm(percentage, S32(seconds * 1000.0f));
}

DefineEngineMethod(Precipitation, setTurbulence, void, (F32 max, F32 speed, F32 seconds), (1.0f, 5.0f, 5.0f),
   "Smoothly change the turbulence parameters over a period of time.\n"
   "@param max New #maxTurbulence value. Set to 0 to disable turbulence.\n"
   "@param speed New #turbulenceSpeed value.\n"
   "@param seconds Length of time (in seconds) over which to interpolate the "
   "turbulence settings. Set to 0 to change instantly.\n"
   "@tsexample\n"
   "%turbulence = 0.5;     // Set the new turbulence value. Set to 0 to disable turbulence.\n"
   "%speed = 5.0;          // The new speed of the turbulance effect.\n"
   "%seconds = 5.0;        // The length of time over which to make the change.\n"
   "%precipitation.setTurbulence( %turbulence, %speed, %seconds );\n"
   "@endtsexample\n" )
{
   object->setTurbulence( max, speed, S32(seconds * 1000.0f));
}

//--------------------------------------------------------------------------
// Backend
//--------------------------------------------------------------------------
bool Precipitation::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (mFollowCam)
   {
      setGlobalBounds();
   }
   else
   {
      mObjBox.minExtents = -Point3F(mBoxWidth/2, mBoxWidth/2, mBoxHeight/2);
      mObjBox.maxExtents = Point3F(mBoxWidth/2, mBoxWidth/2, mBoxHeight/2);
   }
   resetWorldBox();

   if (isClientObject())
   {
      fillDropList();
      initRenderObjects();
      initMaterials();
   }

   addToScene();

   return true;
}

void Precipitation::onRemove()
{
   removeFromScene();
   Parent::onRemove();

   SFX_DELETE( mAmbientSound );

   if (isClientObject())
      killDropList();
}

bool Precipitation::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   mDataBlock = dynamic_cast<PrecipitationData*>( dptr );
   if ( !mDataBlock || !Parent::onNewDataBlock( dptr, reload ) )
      return false;

   if (isClientObject())
   {
      SFX_DELETE( mAmbientSound );

      if ( mDataBlock->soundProfile )
      {
         mAmbientSound = SFX->createSource( mDataBlock->soundProfile, &getTransform() );
         if ( mAmbientSound )
            mAmbientSound->play();
      }

      initRenderObjects();
      initMaterials();
   }

   scriptOnNewDataBlock();
   return true;
}

void Precipitation::initMaterials()
{
   AssertFatal(isClientObject(), "Precipitation is setting materials on the server - BAD!");

   if(!mDataBlock)
      return;

   PrecipitationData *pd = (PrecipitationData*)mDataBlock;

   mDropHandle = NULL;
   mSplashHandle = NULL;
   mDropShader = NULL;
   mSplashShader = NULL;

   if( dStrlen(pd->mDropName) > 0 && !mDropHandle.set(pd->mDropName, &GFXDefaultStaticDiffuseProfile, avar("%s() - mDropHandle (line %d)", __FUNCTION__, __LINE__)) )
      Con::warnf("Precipitation::initMaterials - failed to locate texture '%s'!", pd->mDropName);

   if ( dStrlen(pd->mDropShaderName) > 0 )
   {
      ShaderData *shaderData;
      if ( Sim::findObject( pd->mDropShaderName, shaderData ) )
         mDropShader = shaderData->getShader();

      if( !mDropShader )
         Con::warnf( "Precipitation::initMaterials - could not find shader '%s'!", pd->mDropShaderName );
      else
      {
         mDropShaderConsts = mDropShader->allocConstBuffer();
         mDropShaderModelViewSC = mDropShader->getShaderConstHandle("$modelView");
         mDropShaderFadeStartEndSC = mDropShader->getShaderConstHandle("$fadeStartEnd");
         mDropShaderCameraPosSC = mDropShader->getShaderConstHandle("$cameraPos");
         mDropShaderAmbientSC = mDropShader->getShaderConstHandle("$ambient");
      }
   }

   if( dStrlen(pd->mSplashName) > 0 && !mSplashHandle.set(pd->mSplashName, &GFXDefaultStaticDiffuseProfile, avar("%s() - mSplashHandle (line %d)", __FUNCTION__, __LINE__)) )
      Con::warnf("Precipitation::initMaterials - failed to locate texture '%s'!", pd->mSplashName);

   if ( dStrlen(pd->mSplashShaderName) > 0 )
   {
      ShaderData *shaderData;
      if ( Sim::findObject( pd->mSplashShaderName, shaderData ) )
         mSplashShader = shaderData->getShader();

      if( !mSplashShader )
         Con::warnf( "Precipitation::initMaterials - could not find shader '%s'!", pd->mSplashShaderName );
      else
      {
         mSplashShaderConsts = mSplashShader->allocConstBuffer();
         mSplashShaderModelViewSC = mSplashShader->getShaderConstHandle("$modelView");
         mSplashShaderFadeStartEndSC = mSplashShader->getShaderConstHandle("$fadeStartEnd");
         mSplashShaderCameraPosSC = mSplashShader->getShaderConstHandle("$cameraPos");
         mSplashShaderAmbientSC = mSplashShader->getShaderConstHandle("$ambient");
      }
   }
}

U32 Precipitation::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag( !mFollowCam && mask & TransformMask))
      stream->writeAffineTransform(mObjToWorld);

   if (stream->writeFlag(mask & DataMask))
   {
      stream->write(mDropSize);
      stream->write(mSplashSize);
      stream->write(mSplashMS);
      stream->write(mDropAnimateMS);
      stream->write(mNumDrops);
      stream->write(mMinSpeed);
      stream->write(mMaxSpeed);
      stream->write(mBoxWidth);
      stream->write(mBoxHeight);
      stream->write(mMinMass);
      stream->write(mMaxMass);
      stream->write(mMaxTurbulence);
      stream->write(mTurbulenceSpeed);
      stream->write(mFadeDistance);
      stream->write(mFadeDistanceEnd);
      stream->write(mGlowIntensity.red);
      stream->write(mGlowIntensity.green);
      stream->write(mGlowIntensity.blue);
      stream->write(mGlowIntensity.alpha);
      stream->writeFlag(mReflect);
      stream->writeFlag(mRotateWithCamVel);
      stream->writeFlag(mDoCollision);
      stream->writeFlag(mDropHitPlayers);
      stream->writeFlag(mDropHitVehicles);
      stream->writeFlag(mUseTrueBillboards);
      stream->writeFlag(mUseTurbulence);
      stream->writeFlag(mUseLighting);
      stream->writeFlag(mUseWind);
      stream->writeFlag(mFollowCam);
      stream->writeFlag(mAnimateSplashes);
   }

   if (stream->writeFlag(!(mask & DataMask) && (mask & TurbulenceMask)))
   {
      stream->write(mTurbulenceData.endMax);
      stream->write(mTurbulenceData.endSpeed);
      stream->write(mTurbulenceData.totalTime);
   }

   if (stream->writeFlag(mask & PercentageMask))
   {
      stream->write(mPercentage);
   }

   if (stream->writeFlag(!(mask & ~(DataMask | PercentageMask | StormMask)) && (mask & StormMask)))
   {
      stream->write(mStormData.endPct);
      stream->write(mStormData.totalTime);
   }

   return 0;
}

void Precipitation::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag())
   {
      MatrixF mat;
      stream->readAffineTransform(&mat);
      Parent::setTransform(mat);
   }

   U32 oldDrops = U32(mNumDrops * mPercentage);
   if (stream->readFlag())
   {
      stream->read(&mDropSize);
      stream->read(&mSplashSize);
      stream->read(&mSplashMS);
      stream->read(&mDropAnimateMS);
      stream->read(&mNumDrops);
      stream->read(&mMinSpeed);
      stream->read(&mMaxSpeed);
      stream->read(&mBoxWidth);
      stream->read(&mBoxHeight);
      stream->read(&mMinMass);
      stream->read(&mMaxMass);
      stream->read(&mMaxTurbulence);
      stream->read(&mTurbulenceSpeed);
      stream->read(&mFadeDistance);
      stream->read(&mFadeDistanceEnd);
      stream->read(&mGlowIntensity.red);
      stream->read(&mGlowIntensity.green);
      stream->read(&mGlowIntensity.blue);
      stream->read(&mGlowIntensity.alpha);
      mReflect = stream->readFlag();
      mRotateWithCamVel = stream->readFlag();
      mDoCollision = stream->readFlag();
      mDropHitPlayers = stream->readFlag();
      mDropHitVehicles = stream->readFlag();
      mUseTrueBillboards = stream->readFlag();
      mUseTurbulence = stream->readFlag();
      mUseLighting = stream->readFlag();
      mUseWind = stream->readFlag();
      mFollowCam = stream->readFlag();
      mAnimateSplashes = stream->readFlag();

      mDropHitMask = dropHitMask | 
         ( mDropHitPlayers ? PlayerObjectType : 0 ) | 
         ( mDropHitVehicles ? VehicleObjectType : 0 );

      mTurbulenceData.valid = false;
   }

   if (stream->readFlag())
   {
      F32 max, speed;
      U32 ms;
      stream->read(&max);
      stream->read(&speed);
      stream->read(&ms);
      setTurbulence( max, speed, ms );
   }

   if (stream->readFlag())
   {
      F32 pct;
      stream->read(&pct);
      setPercentage(pct);
   }

   if (stream->readFlag())
   {
      F32 pct;
      U32 time;
      stream->read(&pct);
      stream->read(&time);
      modifyStorm(pct, time);
   }

   AssertFatal(isClientObject(), "Precipitation::unpackUpdate() should only be called on the client!");

   U32 newDrops = U32(mNumDrops * mPercentage);
   if (oldDrops != newDrops)
   {
      fillDropList();
      initRenderObjects();
   }

   if (mFollowCam)
   {
      setGlobalBounds();
   }
   else
   {
      mObjBox.minExtents = -Point3F(mBoxWidth/2, mBoxWidth/2, mBoxHeight/2);
      mObjBox.maxExtents = Point3F(mBoxWidth/2, mBoxWidth/2, mBoxHeight/2);
   }

   resetWorldBox();
}

//--------------------------------------------------------------------------
// Support functions
//--------------------------------------------------------------------------
VectorF Precipitation::getWindVelocity()
{
   // The WindManager happens to set global-wind velocity here, it is not just for particles.
   return mUseWind ? ParticleEmitter::mWindVelocity : Point3F::Zero;
}

void Precipitation::fillDropList()
{
   AssertFatal(isClientObject(), "Precipitation is doing stuff on the server - BAD!");

   F32 density = Con::getFloatVariable("$pref::precipitationDensity", 1.0f);
   U32 newDropCount = (U32)(mNumDrops * mPercentage * density);
   U32 dropCount = 0;

   if (newDropCount == 0)
      killDropList();

   if (mDropHead)
   {
      Raindrop* curr = mDropHead;
      while (curr)
      {
         dropCount++;
         curr = curr->next;
         if (dropCount == newDropCount && curr)
         {
            //delete the remaining drops
            Raindrop* next = curr->next;
            curr->next = NULL;
            while (next)
            {
               Raindrop* last = next;
               next = next->next;
               last->next = NULL;
               destroySplash(last);
               delete last;
            }
            break;
         }
      }
   }

   if (dropCount < newDropCount)
   {
      //move to the end
      Raindrop* curr = mDropHead;
      if (curr)
      {
         while (curr->next)
            curr = curr->next;
      }
      else
      {
         mDropHead = curr = new Raindrop;
         spawnNewDrop(curr);
         dropCount++;
      }

      //and add onto it
      while (dropCount < newDropCount)
      {
         curr->next = new Raindrop;
         curr = curr->next;
         spawnNewDrop(curr);
         dropCount++;
      }
   }
}

void Precipitation::initRenderObjects()
{
   AssertFatal(isClientObject(), "Precipitation is doing stuff on the server - BAD!");

   SAFE_DELETE_ARRAY(mTexCoords);
   SAFE_DELETE_ARRAY(mSplashCoords);

   if (!mDataBlock)
      return;

   mTexCoords = new Point2F[4*mDataBlock->mDropsPerSide*mDataBlock->mDropsPerSide];

   // Setup the texcoords for the drop texture.
   // The order of the coords when animating is...
   //
   //   +---+---+---+
   //   | 1 | 2 | 3 |
   //   |---|---|---+
   //   | 4 | 5 | 6 |
   //   +---+---+---+
   //   | 7 | etc...
   //   +---+
   //
   U32 count = 0;
   for (U32 v = 0; v < mDataBlock->mDropsPerSide; v++)
   {
      F32 y1 = (F32) v / mDataBlock->mDropsPerSide;
      F32 y2 = (F32)(v+1) / mDataBlock->mDropsPerSide;
      for (U32 u = 0; u < mDataBlock->mDropsPerSide; u++)
      {
         F32 x1 = (F32) u / mDataBlock->mDropsPerSide;
         F32 x2 = (F32)(u+1) / mDataBlock->mDropsPerSide;

         mTexCoords[4*count+0].x = x1;
         mTexCoords[4*count+0].y = y1;

         mTexCoords[4*count+1].x = x2;
         mTexCoords[4*count+1].y = y1;

         mTexCoords[4*count+2].x = x2;
         mTexCoords[4*count+2].y = y2;

         mTexCoords[4*count+3].x = x1;
         mTexCoords[4*count+3].y = y2;
         count++;
      }
   }

   count = 0;
   mSplashCoords = new Point2F[4*mDataBlock->mSplashesPerSide*mDataBlock->mSplashesPerSide];
   for (U32 v = 0; v < mDataBlock->mSplashesPerSide; v++)
   {
      F32 y1 = (F32) v / mDataBlock->mSplashesPerSide;
      F32 y2 = (F32)(v+1) / mDataBlock->mSplashesPerSide;
      for (U32 u = 0; u < mDataBlock->mSplashesPerSide; u++)
      {
         F32 x1 = (F32) u / mDataBlock->mSplashesPerSide;
         F32 x2 = (F32)(u+1) / mDataBlock->mSplashesPerSide;

         mSplashCoords[4*count+0].x = x1;
         mSplashCoords[4*count+0].y = y1;

         mSplashCoords[4*count+1].x = x2;
         mSplashCoords[4*count+1].y = y1;

         mSplashCoords[4*count+2].x = x2;
         mSplashCoords[4*count+2].y = y2;

         mSplashCoords[4*count+3].x = x1;
         mSplashCoords[4*count+3].y = y2;
         count++;
      }
   }

   // Cap the number of precipitation drops so that we don't blow out the max verts
   mMaxVBDrops = getMin( (U32)mNumDrops, ( GFX->getMaxDynamicVerts() / 4 ) - 1 );

   // If we have no drops then skip allocating anything!
   if ( mMaxVBDrops == 0 )
      return;

   // Create a volitile vertex buffer which
   // we'll lock and fill every frame.
   mRainVB.set(GFX, mMaxVBDrops * 4, GFXBufferTypeDynamic);

   // Init the index buffer for rendering the
   // entire or a partially filled vb.
   mRainIB.set(GFX, mMaxVBDrops * 6, 0, GFXBufferTypeStatic);
   U16 *idxBuff;
   mRainIB.lock(&idxBuff, NULL, NULL, NULL);
   for( U32 i=0; i < mMaxVBDrops; i++ )
   {
      //
      // The vertex pattern in the VB for each 
      // particle is as follows...
      //
      //     0----1
      //     |\   |
      //     | \  |
      //     |  \ |
      //     |   \|
      //     3----2
      //
      // We setup the index order below to ensure
      // sequential, cache friendly, access.
      //
      U32 offset = i * 4;
      idxBuff[i*6+0] = 0 + offset;
      idxBuff[i*6+1] = 1 + offset;
      idxBuff[i*6+2] = 2 + offset;
      idxBuff[i*6+3] = 2 + offset;
      idxBuff[i*6+4] = 3 + offset;
      idxBuff[i*6+5] = 0 + offset;
   }
   mRainIB.unlock();
}

void Precipitation::killDropList()
{
   AssertFatal(isClientObject(), "Precipitation is doing stuff on the server - BAD!");

   Raindrop* curr = mDropHead;
   while (curr)
   {
      Raindrop* next = curr->next;
      delete curr;
      curr = next;
   }
   mDropHead = NULL;
   mSplashHead = NULL;
}

void Precipitation::spawnDrop(Raindrop *drop)
{
   PROFILE_START(PrecipSpawnDrop);
   AssertFatal(isClientObject(), "Precipitation is doing stuff on the server - BAD!");

   drop->velocity = Platform::getRandom() * (mMaxSpeed - mMinSpeed) + mMinSpeed;

   drop->position.x = Platform::getRandom() * mBoxWidth;
   drop->position.y = Platform::getRandom() * mBoxWidth;

   // The start time should be randomized so that 
   // all the drops are not animating at the same time.
   drop->animStartTime = (SimTime)(Platform::getVirtualMilliseconds() * Platform::getRandom());

   if (mDropAnimateMS <= 0 && mDataBlock)
      drop->texCoordIndex = (U32)(Platform::getRandom() * ((F32)mDataBlock->mDropsPerSide*mDataBlock->mDropsPerSide - 0.5));

   drop->valid = true;
   drop->time = Platform::getRandom() * M_2PI;
   drop->mass = Platform::getRandom() * (mMaxMass - mMinMass) + mMinMass;
   PROFILE_END();
}

void Precipitation::spawnNewDrop(Raindrop *drop)
{
   AssertFatal(isClientObject(), "Precipitation is doing stuff on the server - BAD!");

   spawnDrop(drop);
   drop->position.z = Platform::getRandom() * mBoxHeight - (mBoxHeight / 2);
}

void Precipitation::wrapDrop(Raindrop *drop, const Box3F &box, const U32 currTime, const VectorF &windVel)
{
   //could probably be slightly optimized to get rid of the while loops
   if (drop->position.z < box.minExtents.z)
   {
      spawnDrop(drop);
      drop->position.x += box.minExtents.x;
      drop->position.y += box.minExtents.y;
      while (drop->position.z < box.minExtents.z)
         drop->position.z += mBoxHeight;
      findDropCutoff(drop, box, windVel);
   }
   else if (drop->position.z > box.maxExtents.z)
   {
      while (drop->position.z > box.maxExtents.z)
         drop->position.z -= mBoxHeight;
      findDropCutoff(drop, box, windVel);
   }
   else if (drop->position.x < box.minExtents.x)
   {
      while (drop->position.x < box.minExtents.x)
         drop->position.x += mBoxWidth;
      findDropCutoff(drop, box, windVel);
   }
   else if (drop->position.x > box.maxExtents.x)
   {
      while (drop->position.x > box.maxExtents.x)
         drop->position.x -= mBoxWidth;
      findDropCutoff(drop, box, windVel);
   }
   else if (drop->position.y < box.minExtents.y)
   {
      while (drop->position.y < box.minExtents.y)
         drop->position.y += mBoxWidth;
      findDropCutoff(drop, box, windVel);
   }
   else if (drop->position.y > box.maxExtents.y)
   {
      while (drop->position.y > box.maxExtents.y)
         drop->position.y -= mBoxWidth;
      findDropCutoff(drop, box, windVel);
   }
}

void Precipitation::findDropCutoff(Raindrop *drop, const Box3F &box, const VectorF &windVel)
{
   PROFILE_START(PrecipFindDropCutoff);
   AssertFatal(isClientObject(), "Precipitation is doing stuff on the server - BAD!");

   if (mDoCollision)
   {
      VectorF velocity = windVel / drop->mass - VectorF(0, 0, drop->velocity);
      velocity.normalize();

      Point3F end   = drop->position + 100 * velocity;
      Point3F start = drop->position - (mFollowCam ? 500.0f : 0.0f) * velocity;

      if (!mFollowCam)
      {
         mObjToWorld.mulP(start);
         mObjToWorld.mulP(end);
      }

      // Look for a collision... make sure we don't 
      // collide with backfaces.
      RayInfo rInfo;
      if (getContainer()->castRay(start, end, mDropHitMask, &rInfo))
      {
         // TODO: Add check to filter out hits on backfaces.

         if (!mFollowCam)
            mWorldToObj.mulP(rInfo.point);

         drop->hitPos = rInfo.point;
         drop->hitType = rInfo.object->getTypeMask();
      }
      else
         drop->hitPos = Point3F(0,0,-1000);

      drop->valid = drop->position.z > drop->hitPos.z;
   }
   else
   {
      drop->hitPos = Point3F(0,0,-1000);
      drop->valid = true;
   }
   PROFILE_END();
}

void Precipitation::createSplash(Raindrop *drop)
{
   if (!mDataBlock)
      return;

   PROFILE_START(PrecipCreateSplash);
   if (drop != mSplashHead && !(drop->nextSplashDrop || drop->prevSplashDrop))
   {
      if (!mSplashHead)
      {
         mSplashHead = drop;
         drop->prevSplashDrop = NULL;
         drop->nextSplashDrop = NULL;
      }
      else
      {
         mSplashHead->prevSplashDrop = drop;
         drop->nextSplashDrop = mSplashHead;
         drop->prevSplashDrop = NULL;
         mSplashHead = drop;
      }
   }

   drop->animStartTime = Platform::getVirtualMilliseconds();

   if (!mAnimateSplashes)
      drop->texCoordIndex = (U32)(Platform::getRandom() * ((F32)mDataBlock->mSplashesPerSide*mDataBlock->mSplashesPerSide - 0.5));

   PROFILE_END();
}

void Precipitation::destroySplash(Raindrop *drop)
{
   PROFILE_START(PrecipDestroySplash);
   if (drop == mSplashHead)
   {
      mSplashHead = mSplashHead->nextSplashDrop;
   }

   if (drop->nextSplashDrop)
      drop->nextSplashDrop->prevSplashDrop = drop->prevSplashDrop;
   if (drop->prevSplashDrop)
      drop->prevSplashDrop->nextSplashDrop = drop->nextSplashDrop;

   drop->nextSplashDrop = NULL;
   drop->prevSplashDrop = NULL;

   PROFILE_END();
}

//--------------------------------------------------------------------------
// Processing
//--------------------------------------------------------------------------
void Precipitation::setPercentage(F32 pct)
{
   mPercentage = mClampF(pct, 0, 1);
   mStormData.valid = false;

   if (isServerObject()) 
   {
      setMaskBits(PercentageMask);
   }
}

void Precipitation::modifyStorm(F32 pct, U32 ms)
{
   if ( ms == 0 ) 
   {
      setPercentage( pct );
      return;
   }

   pct = mClampF(pct, 0, 1);
   mStormData.endPct = pct;
   mStormData.totalTime = ms;

   if (isServerObject())
   {
      setMaskBits(StormMask);
      return;
   }

   mStormData.startTime = Platform::getVirtualMilliseconds();
   mStormData.startPct = mPercentage;
   mStormData.valid = true;
}

void Precipitation::setTurbulence(F32 max, F32 speed, U32 ms)
{
   if ( ms == 0 && !isServerObject() ) 
   {
      mUseTurbulence = max > 0;
      mMaxTurbulence = max;
      mTurbulenceSpeed = speed;
      return;
   }

   mTurbulenceData.endMax = max;
   mTurbulenceData.endSpeed = speed;
   mTurbulenceData.totalTime = ms;

   if (isServerObject())
   {
      setMaskBits(TurbulenceMask);
      return;
   }

   mTurbulenceData.startTime = Platform::getVirtualMilliseconds();
   mTurbulenceData.startMax = mMaxTurbulence;
   mTurbulenceData.startSpeed = mTurbulenceSpeed;
   mTurbulenceData.valid = true;
}

void Precipitation::interpolateTick(F32 delta)
{
   AssertFatal(isClientObject(), "Precipitation is doing stuff on the server - BAD!");

   // If we're not being seen then the simulation
   // is paused and we don't need any interpolation.
   if (mLastRenderFrame != ShapeBase::sLastRenderFrame)
      return;

   PROFILE_START(PrecipInterpolate);

   const F32 dt = 1-delta;
   const VectorF windVel = dt * getWindVelocity();
   const F32 turbSpeed = dt * mTurbulenceSpeed;

   Raindrop* curr = mDropHead;
   VectorF turbulence;
   F32 renderTime;

   while (curr)
   {
      if (!curr->valid || !curr->toRender)
      {
         curr = curr->next;
         continue;
      }

      if (mUseTurbulence)
      {
         renderTime = curr->time + turbSpeed;
         turbulence.x = windVel.x + ( mSin(renderTime) * mMaxTurbulence );
         turbulence.y = windVel.y + ( mCos(renderTime) * mMaxTurbulence );
         turbulence.z = windVel.z;
         curr->renderPosition = curr->position + turbulence / curr->mass;
      }
      else
         curr->renderPosition = curr->position + windVel / curr->mass;

      curr->renderPosition.z -= dt * curr->velocity;

      curr = curr->next;
   }
   PROFILE_END();
}

void Precipitation::processTick(const Move *)
{
   //nothing to do on the server
   if (isServerObject() || mDataBlock == NULL || isHidden())
      return;

   const U32 currTime = Platform::getVirtualMilliseconds();

   // Update the storm if necessary
   if (mStormData.valid)
   {
      F32 t = (currTime - mStormData.startTime) / (F32)mStormData.totalTime;
      if (t >= 1)
      {
         mPercentage = mStormData.endPct;
         mStormData.valid = false;
      }
      else
         mPercentage = mStormData.startPct * (1-t) + mStormData.endPct * t;

      fillDropList();
   }

   // Do we need to update the turbulence?
   if ( mTurbulenceData.valid )
   {
      F32 t = (currTime - mTurbulenceData.startTime) / (F32)mTurbulenceData.totalTime;
      if (t >= 1)
      {
         mMaxTurbulence = mTurbulenceData.endMax;
         mTurbulenceSpeed = mTurbulenceData.endSpeed;
         mTurbulenceData.valid = false;
      }
      else
      {
         mMaxTurbulence = mTurbulenceData.startMax * (1-t) + mTurbulenceData.endMax * t;
         mTurbulenceSpeed = mTurbulenceData.startSpeed * (1-t) + mTurbulenceData.endSpeed * t;
      }

      mUseTurbulence = mMaxTurbulence > 0;
   }

   // If we're not being seen then pause the 
   // simulation.  Precip is generally noisy 
   // enough that no one should notice.
   if (mLastRenderFrame != ShapeBase::sLastRenderFrame)
      return;

   //we need to update positions and do some collision here
   GameConnection* conn = GameConnection::getConnectionToServer();
   if (!conn)
      return; //need connection to server

   ShapeBase* camObj = dynamic_cast<ShapeBase*>(conn->getCameraObject());
   if (!camObj)
      return;

   PROFILE_START(PrecipProcess);

   MatrixF camMat;
   camObj->getEyeTransform(&camMat);

   const F32 camFov = camObj->getCameraFov();

   Point3F camPos, camDir;
   Box3F box;

   if (mFollowCam)
   {
      camMat.getColumn(3, &camPos);

      box = Box3F(camPos.x - mBoxWidth / 2, camPos.y - mBoxWidth / 2, camPos.z - mBoxHeight / 2,
         camPos.x + mBoxWidth / 2, camPos.y + mBoxWidth / 2, camPos.z + mBoxHeight / 2);

      camMat.getColumn(1, &camDir);
      camDir.normalize();
   }
   else
   {
      box = mObjBox;

      camMat.getColumn(3, &camPos);
      mWorldToObj.mulP(camPos);

      camMat.getColumn(1, &camDir);
      camDir.normalize();
      mWorldToObj.mulV(camDir);
   }

   const VectorF windVel = getWindVelocity();
   const F32 fovDot = camFov / 180;

   Raindrop* curr = mDropHead;

   //offset the renderbox in the direction of the camera direction
   //in order to have more of the drops actually rendered
   if (mFollowCam) 
   {
      box.minExtents.x += camDir.x * mBoxWidth / 4;
      box.maxExtents.x += camDir.x * mBoxWidth / 4;
      box.minExtents.y += camDir.y * mBoxWidth / 4;
      box.maxExtents.y += camDir.y * mBoxWidth / 4;
      box.minExtents.z += camDir.z * mBoxHeight / 4;
      box.maxExtents.z += camDir.z * mBoxHeight / 4;
   }

   VectorF lookVec;
   F32 pct;
   const S32 dropCount = mDataBlock->mDropsPerSide*mDataBlock->mDropsPerSide;
   while (curr)
   {
      // Update the position.  This happens even if this
      // is a splash so that the drop respawns when it wraps
      // around to the top again.
      if (mUseTurbulence)
         curr->time += mTurbulenceSpeed;
      curr->position += windVel / curr->mass;
      curr->position.z -= curr->velocity;

      // Wrap the drop if it reaches an edge of the box.
      wrapDrop(curr, box, currTime, windVel);

      // Did the drop pass below the hit position?
      if (curr->valid && curr->position.z < curr->hitPos.z)
      {
         // If this drop was to hit a player or vehicle double
         // check to see if the object has moved out of the way.
         // This keeps us from leaving phantom trails of splashes
         // behind a moving player/vehicle.
         if (curr->hitType & (PlayerObjectType | VehicleObjectType))
         {
            findDropCutoff(curr, box, windVel);

            if (curr->position.z > curr->hitPos.z)
               goto NO_SPLASH; // Ugly, yet simple.
         }

         // The drop is dead.
         curr->valid = false;

         // Convert the drop into a splash or let it 
         // wrap around and respawn in wrapDrop().
         if (mSplashMS > 0)
            createSplash(curr);

         // So ugly... yet simple.
NO_SPLASH:;
      }

      // We do not do cull individual drops when we're not
      // following as it is usually a tight box and all of 
      // the particles are in view.
      if (!mFollowCam)
         curr->toRender = true;
      else
      {
         lookVec = curr->position - camPos;
         curr->toRender = mDot(lookVec, camDir) > fovDot;
      }

      // Do we need to animate the drop?
      if (curr->valid && mDropAnimateMS > 0 && curr->toRender)
      {
         pct = (F32)(currTime - curr->animStartTime) / mDropAnimateMS;
         pct = mFmod(pct, 1);
         curr->texCoordIndex = (U32)(dropCount * pct);
      }

      curr = curr->next;
   }

   //update splashes
   curr = mSplashHead;
   Raindrop *next;
   const S32 splashCount = mDataBlock->mSplashesPerSide * mDataBlock->mSplashesPerSide;
   while (curr)
   {
      pct = (F32)(currTime - curr->animStartTime) / mSplashMS;
      if (pct >= 1.0f)
      {
         next = curr->nextSplashDrop;
         destroySplash(curr);
         curr = next;
         continue;
      }

      if (mAnimateSplashes)
         curr->texCoordIndex = (U32)(splashCount * pct);

      curr = curr->nextSplashDrop;
   }

   PROFILE_END_NAMED(PrecipProcess);
}

//--------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------
void Precipitation::prepRenderImage(SceneRenderState* state)
{
   PROFILE_SCOPE(Precipitation_prepRenderImage);

   // We we have no drops then skip rendering
   // and don't bother with the sound.
   if (mMaxVBDrops == 0)
      return;

   // We do nothing if we're not supposed to be reflected.
   if ( state->isReflectPass() && !mReflect )
      return;

   // This should be sufficient for most objects that don't manage zones, and
   // don't need to return a specialized RenderImage...
   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind(this, &Precipitation::renderObject);
   ri->type = RenderPassManager::RIT_Foliage;
   state->getRenderPass()->addInst( ri );
}

void Precipitation::renderObject(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* overrideMat)
{
   if (overrideMat)
      return;

#ifdef TORQUE_OS_XENON
   return;
#endif

   GameConnection* conn = GameConnection::getConnectionToServer();
   if (!conn)
      return; //need connection to server

   ShapeBase* camObj = dynamic_cast<ShapeBase*>(conn->getCameraObject());
   if (!camObj)
      return; // need camera object

   PROFILE_START(PrecipRender);

   GFX->pushWorldMatrix();

   MatrixF world = GFX->getWorldMatrix();
   MatrixF proj = GFX->getProjectionMatrix();
   if (!mFollowCam)
   {
      world.mul( getRenderTransform() );
      world.scale( getScale() );
      GFX->setWorldMatrix( world );
   }    
   proj.mul(world);

   //GFX2 doesn't require transpose?
   //proj.transpose();

   Point3F camPos  = state->getCameraPosition();
   VectorF camVel  = camObj->getVelocity();
   if (!mFollowCam)
   {
      getRenderWorldTransform().mulP(camPos);
      getRenderWorldTransform().mulV(camVel);
   }
   const VectorF windVel = getWindVelocity();
   const bool useBillboards = mUseTrueBillboards;
   const F32 dropSize = mDropSize;

   Point3F pos;
   VectorF orthoDir, velocity, right, up, rightUp(0.0f, 0.0f, 0.0f), leftUp(0.0f, 0.0f, 0.0f);
   F32 distance = 0;
   GFXVertexPCT* vertPtr = NULL;
   const Point2F *tc;

   // Do this here and we won't have to in the loop!
   if (useBillboards)
   {
      MatrixF camMat = state->getCameraTransform();
      camMat.inverse();
      camMat.getRow(0,&right);
      camMat.getRow(2,&up);
      if (!mFollowCam)
      {
         mWorldToObj.mulV(right);
         mWorldToObj.mulV(up);
      }
      right.normalize();
      up.normalize();
      right *= mDropSize;
      up    *= mDropSize;
      rightUp = right + up;
      leftUp = -right + up;
   }

   // We pass the sunlight as a constant to the 
   // shader.  Once the lighting and shadow systems
   // are added into TSE we can expand this to include
   // the N nearest lights to the camera + the ambient.
   ColorF ambient( 1, 1, 1 );
   if ( mUseLighting )
   {
      const LightInfo *sunlight = LIGHTMGR->getSpecialLight(LightManager::slSunLightType);
      ambient = sunlight->getColor();
   }

   if ( mGlowIntensity.red > 0 ||
      mGlowIntensity.green > 0 ||
      mGlowIntensity.blue > 0 )
   {
      ambient *= mGlowIntensity;
   }

   // Setup render state

   if (mDefaultSB.isNull())
   {
      GFXStateBlockDesc desc;

      desc.zWriteEnable = false;
      desc.setAlphaTest(true, GFXCmpGreaterEqual, 1);
      desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);

      mDefaultSB = GFX->createStateBlock(desc);

      desc.samplersDefined = true;
      desc.samplers[0].textureColorOp =  GFXTOPModulate;
      desc.samplers[0].colorArg1 = GFXTATexture;
      desc.samplers[0].colorArg2 = GFXTADiffuse;
      desc.samplers[0].alphaOp = GFXTOPSelectARG1;
      desc.samplers[0].alphaArg1 = GFXTATexture;

      desc.samplers[1].textureColorOp = GFXTOPDisable;
      desc.samplers[1].alphaOp = GFXTOPDisable;

      mDistantSB = GFX->createStateBlock(desc);
   }

   GFX->setStateBlock(mDefaultSB);

   // Everything is rendered from these buffers.
   GFX->setPrimitiveBuffer(mRainIB);
   GFX->setVertexBuffer(mRainVB);

   // Set the constants used by the shaders.
   if (mDropShader) 
   {
      Point2F fadeStartEnd( mFadeDistance, mFadeDistanceEnd );

      mDropShaderConsts->setSafe(mDropShaderModelViewSC, proj);
      mDropShaderConsts->setSafe(mDropShaderFadeStartEndSC, fadeStartEnd);
      mDropShaderConsts->setSafe(mDropShaderCameraPosSC, camPos);
      mDropShaderConsts->setSafe(mDropShaderAmbientSC, Point3F(ambient.red, ambient.green, ambient.blue));
   }


   if (mSplashShader) 
   {
      Point2F fadeStartEnd( mFadeDistance, mFadeDistanceEnd );

      mSplashShaderConsts->setSafe(mSplashShaderModelViewSC, proj);
      mSplashShaderConsts->setSafe(mSplashShaderFadeStartEndSC, fadeStartEnd);
      mSplashShaderConsts->setSafe(mSplashShaderCameraPosSC, camPos);
      mSplashShaderConsts->setSafe(mSplashShaderAmbientSC, Point3F(ambient.red, ambient.green, ambient.blue));
   }

   // Time to render the drops...
   const Raindrop *curr = mDropHead;
   U32 vertCount = 0;

   GFX->setTexture(0, mDropHandle);

   // Use the shader or setup the pipeline 
   // for fixed function rendering.
   if (mDropShader)
   {
      GFX->setShader( mDropShader );
      GFX->setShaderConstBuffer( mDropShaderConsts );
   }
   else
   {
      GFX->setupGenericShaders(GFXDevice::GSTexture);

      // We don't support distance fade or lighting without shaders.
      GFX->setStateBlock(mDistantSB);
   }

   while (curr)
   {
      // Skip ones that are not drops (hit something and 
      // may have been converted into a splash) or they 
      // are behind the camera.
      if (!curr->valid || !curr->toRender)
      {
         curr = curr->next;
         continue;
      }

      pos = curr->renderPosition;

      // two forms of billboards - true billboards (which we set 
      // above outside this loop) or axis-aligned with velocity
      // (this codeblock) the axis-aligned billboards are aligned
      // with the velocity of the raindrop, and tilted slightly 
      // towards the camera
      if (!useBillboards)
      {
         orthoDir = camPos - pos;
         distance = orthoDir.len();

         // Inline the normalize so we don't 
         // calculate the ortho len twice.
         if (distance > 0.0)
            orthoDir *= 1.0f / distance;
         else
            orthoDir.set( 0, 0, 1 );

         velocity = windVel / curr->mass;

         // We do not optimize this for the "still" case
         // because its not a typical scenario.
         if (mRotateWithCamVel)
            velocity -= camVel / (distance > 2.0f ? distance : 2.0f) * 0.3f;

         velocity.z -= curr->velocity;
         velocity.normalize();

         right = mCross(-velocity, orthoDir);
         right.normalize();
         up    = mCross(orthoDir, right) * 0.5 - velocity * 0.5;
         up.normalize();
         right *= dropSize;
         up    *= dropSize;
         rightUp = right + up;
         leftUp = -right + up;
      }

      // Do we need to relock the buffer?
      if ( !vertPtr )
         vertPtr = mRainVB.lock();
      if(!vertPtr) return;

      // Set the proper texture coords... (it's fun!)
      tc = &mTexCoords[4*curr->texCoordIndex];
      vertPtr->point = pos + leftUp;
      vertPtr->texCoord = *tc;
      tc++;
      vertPtr++;

      vertPtr->point = pos + rightUp;
      vertPtr->texCoord = *tc;
      tc++;
      vertPtr++;

      vertPtr->point = pos - leftUp;
      vertPtr->texCoord = *tc;
      tc++;
      vertPtr++;

      vertPtr->point = pos - rightUp;
      vertPtr->texCoord = *tc;
      tc++;
      vertPtr++;

      // Do we need to render to clear the buffer?
      vertCount += 4;
      if ( (vertCount + 4) >= mRainVB->mNumVerts ) {

         mRainVB.unlock();
         GFX->drawIndexedPrimitive(GFXTriangleList, 0, 0, vertCount, 0, vertCount / 2);
         vertPtr = NULL;
         vertCount = 0;
      }

      curr = curr->next;
   }

   // Do we have stuff left to render?
   if ( vertCount > 0 ) {

      mRainVB.unlock();
      GFX->drawIndexedPrimitive(GFXTriangleList, 0, 0, vertCount, 0, vertCount / 2);
      vertCount = 0;
      vertPtr = NULL;
   }

   // Setup the billboard for the splashes.
   MatrixF camMat = state->getCameraTransform();
   camMat.inverse();
   camMat.getRow(0, &right);
   camMat.getRow(2, &up);
   if (!mFollowCam)
   {
      mWorldToObj.mulV(right);
      mWorldToObj.mulV(up);
   }
   right.normalize();
   up.normalize();
   right *= mSplashSize;
   up *= mSplashSize;
   rightUp = right + up;
   leftUp = -right + up;

   // Render the visible splashes.
   curr = mSplashHead;

   GFX->setTexture(0, mSplashHandle);

   if (mSplashShader)
   {
      GFX->setShader( mSplashShader );
      GFX->setShaderConstBuffer(mSplashShaderConsts);
   }
   else
      GFX->setupGenericShaders(GFXDevice::GSTexture);

   while (curr)
   {
      if (!curr->toRender)
      {
         curr = curr->nextSplashDrop;
         continue;
      }

      pos = curr->hitPos;

      tc = &mSplashCoords[4*curr->texCoordIndex];

      // Do we need to relock the buffer?
      if ( !vertPtr )
         vertPtr = mRainVB.lock();
      if(!vertPtr) return;

      vertPtr->point = pos + leftUp;
      vertPtr->texCoord = *tc;
      tc++;
      vertPtr++;

      vertPtr->point = pos + rightUp;
      vertPtr->texCoord = *tc;
      tc++;
      vertPtr++;

      vertPtr->point = pos - leftUp;
      vertPtr->texCoord = *tc;
      tc++;
      vertPtr++;

      vertPtr->point = pos - rightUp;
      vertPtr->texCoord = *tc;
      tc++;
      vertPtr++;

      // Do we need to flush the buffer by rendering?
      vertCount += 4;
      if ( (vertCount + 4) >= mRainVB->mNumVerts ) {

         mRainVB.unlock();
         GFX->drawIndexedPrimitive(GFXTriangleList, 0, 0, vertCount, 0, vertCount / 2);
         vertPtr = NULL;
         vertCount = 0;
      }

      curr = curr->nextSplashDrop;
   }

   // Do we have stuff left to render?
   if ( vertCount > 0 ) {

      mRainVB.unlock();
      GFX->drawIndexedPrimitive(GFXTriangleList, 0, 0, vertCount, 0, vertCount / 2);
   }

   mLastRenderFrame = ShapeBase::sLastRenderFrame;

   GFX->popWorldMatrix();

   PROFILE_END();
}
