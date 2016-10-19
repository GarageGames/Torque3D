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
#include "T3D/levelInfo.h"

#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "scene/sceneManager.h"
#include "lighting/advanced/advancedLightManager.h"
#include "lighting/advanced/advancedLightBinManager.h"
#include "sfx/sfxAmbience.h"
#include "sfx/sfxSoundscape.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTypes.h"
#include "console/engineAPI.h"
#include "math/mathIO.h"

#include "torqueConfig.h"
#include "T3D/accumulationVolume.h"

IMPLEMENT_CO_NETOBJECT_V1(LevelInfo);

ConsoleDocClass( LevelInfo,
   "@brief Stores and controls the rendering and status information for a game level.\n\n"   
   
   "@tsexample\n"
   "new LevelInfo(theLevelInfo)\n"
   "{\n"
   "  visibleDistance = \"1000\";\n"
   "  fogColor = \"0.6 0.6 0.7 1\";\n"
   "  fogDensity = \"0\";\n"
   "  fogDensityOffset = \"700\";\n"
   "  fogAtmosphereHeight = \"0\";\n"
   "  canvasClearColor = \"0 0 0 255\";\n"
   "  canSaveDynamicFields = \"1\";\n"
   "  levelName = \"Blank Room\";\n"
   "  desc0 = \"A blank room ready to be populated with Torque objects.\";\n"
   "  Enabled = \"1\";\n"
   "};\n"
   "@endtsexample\n"
   "@ingroup enviroMisc\n"
);


/// The color used to clear the canvas.
/// @see GuiCanvas
extern ColorI gCanvasClearColor;

/// @see DecalManager
extern F32 gDecalBias;

/// @see AccumulationVolume
extern GFXTexHandle gLevelAccuMap;

/// Default SFXAmbience used to reset the global soundscape.
static SFXAmbience sDefaultAmbience;


//-----------------------------------------------------------------------------

LevelInfo::LevelInfo()
   :  mWorldSize( 10000.0f ),
      mNearClip( 0.1f ),
      mVisibleDistance( 1000.0f ),
      mVisibleGhostDistance ( 0 ),
      mDecalBias( 0.0015f ),
      mCanvasClearColor( 255, 0, 255, 255 ),
      mAmbientLightBlendPhase( 1.f ),
      mSoundAmbience( NULL ),
      mSoundDistanceModel( SFXDistanceModelLinear ),
      mSoundscape( NULL )
{
   mFogData.density = 0.0f;
   mFogData.densityOffset = 0.0f;
   mFogData.atmosphereHeight = 0.0f;
   mFogData.color.set( 0.5f, 0.5f, 0.5f, 1.0f ),

   mNetFlags.set( ScopeAlways | Ghostable );

   mAdvancedLightmapSupport = false;
   mAccuTextureName = "";
   mAccuTexture = NULL;

   // Register with the light manager activation signal, and we need to do it first
   // so the advanced light bin manager can be instructed about MRT lightmaps
   LightManager::smActivateSignal.notify(this, &LevelInfo::_onLMActivate, 0.01f);
}

//-----------------------------------------------------------------------------

LevelInfo::~LevelInfo()
{
   LightManager::smActivateSignal.remove(this, &LevelInfo::_onLMActivate);
   if (!mAccuTexture.isNull())
   {
      mAccuTexture.free();
      gLevelAccuMap.free();
   }
}

//-----------------------------------------------------------------------------

void LevelInfo::initPersistFields()
{
   addGroup( "Visibility" );

      addField( "nearClip", TypeF32, Offset( mNearClip, LevelInfo ), "Closest distance from the camera's position to render the world." );
      addField( "visibleDistance", TypeF32, Offset( mVisibleDistance, LevelInfo ), "Furthest distance from the camera's position to render the world." );
      addField( "visibleGhostDistance", TypeF32, Offset( mVisibleGhostDistance, LevelInfo ), "Furthest distance from the camera's position to render players. Defaults to visibleDistance." );
      addField( "decalBias", TypeF32, Offset( mDecalBias, LevelInfo ),
         "NearPlane bias used when rendering Decal and DecalRoad. This should be tuned to the visibleDistance in your level." );

   endGroup( "Visibility" );

   addGroup( "Fog" );

      addField( "fogColor", TypeColorF, Offset( mFogData.color, LevelInfo ),
         "The default color for the scene fog." );

      addField( "fogDensity", TypeF32, Offset( mFogData.density, LevelInfo ),
         "The 0 to 1 density value for the exponential fog falloff." );

      addField( "fogDensityOffset", TypeF32, Offset( mFogData.densityOffset, LevelInfo ),
         "An offset from the camera in meters for moving the start of the fog effect." );

      addField( "fogAtmosphereHeight", TypeF32, Offset( mFogData.atmosphereHeight, LevelInfo ),
         "A height in meters for altitude fog falloff." );

   endGroup( "Fog" );

   addGroup( "LevelInfo" );

      addField( "canvasClearColor", TypeColorI, Offset( mCanvasClearColor, LevelInfo ),
         "The color used to clear the background before the scene or any GUIs are rendered." );

   endGroup( "LevelInfo" );

   addGroup( "Lighting" );

      addField( "ambientLightBlendPhase", TypeF32, Offset( mAmbientLightBlendPhase, LevelInfo ),
         "Number of seconds it takes to blend from one ambient light color to a different one." );

      addField( "ambientLightBlendCurve", TypeEaseF, Offset( mAmbientLightBlendCurve, LevelInfo ),
         "Interpolation curve to use for blending from one ambient light color to a different one." );

      addField( "advancedLightmapSupport", TypeBool, Offset( mAdvancedLightmapSupport, LevelInfo ),
         "Enable expanded support for mixing static and dynamic lighting (more costly)" );

      addProtectedField("AccuTexture", TypeStringFilename, Offset(mAccuTextureName, LevelInfo),
         &_setLevelAccuTexture, &defaultProtectedGetFn, "Accumulation texture.");

   endGroup( "Lighting" );
   
   addGroup( "Sound" );
   
      addField( "soundAmbience", TypeSFXAmbienceName, Offset( mSoundAmbience, LevelInfo ), "The global ambient sound environment." );
      addField( "soundDistanceModel", TypeSFXDistanceModel, Offset( mSoundDistanceModel, LevelInfo ), "The distance attenuation model to use." );
   
   endGroup( "Sound" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void LevelInfo::inspectPostApply()
{
   _updateSceneGraph();
   setMaskBits( 0xFFFFFFFF );
   
   Parent::inspectPostApply();
}

//-----------------------------------------------------------------------------

U32 LevelInfo::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   stream->write( mNearClip );
   stream->write( mVisibleDistance );
   stream->write( mDecalBias );

   stream->write( mFogData.density );
   stream->write( mFogData.densityOffset );
   stream->write( mFogData.atmosphereHeight );
   stream->write( mFogData.color );

   stream->write( mCanvasClearColor );
   stream->write( mWorldSize );

   stream->writeFlag( mAdvancedLightmapSupport );
   stream->write( mAmbientLightBlendPhase );
   mathWrite( *stream, mAmbientLightBlendCurve );

   sfxWrite( stream, mSoundAmbience );
   stream->writeInt( mSoundDistanceModel, 1 );

   stream->write(mAccuTextureName);
   return retMask;
}

//-----------------------------------------------------------------------------

void LevelInfo::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   Parent::unpackUpdate(conn, stream);

   stream->read( &mNearClip );
   stream->read( &mVisibleDistance );
   stream->read( &mDecalBias );

   stream->read( &mFogData.density );
   stream->read( &mFogData.densityOffset );
   stream->read( &mFogData.atmosphereHeight );
   stream->read( &mFogData.color );

   stream->read( &mCanvasClearColor );
   stream->read( &mWorldSize );

   mAdvancedLightmapSupport = stream->readFlag();
   stream->read( &mAmbientLightBlendPhase );
   mathRead( *stream, &mAmbientLightBlendCurve );

   String errorStr;
   if( !sfxReadAndResolve( stream, &mSoundAmbience, errorStr ) )
      Con::errorf( "%s", errorStr.c_str() );
   mSoundDistanceModel = ( SFXDistanceModel ) stream->readInt( 1 );
   
   if( isProperlyAdded() )
   {
      _updateSceneGraph();
      
      if( mSoundscape )
      {
         if( mSoundAmbience )
            mSoundscape->setAmbience( mSoundAmbience );
         else
            mSoundscape->setAmbience( &sDefaultAmbience );
      }

      SFX->setDistanceModel( mSoundDistanceModel );
   }
   stream->read(&mAccuTextureName);
   setLevelAccuTexture(mAccuTextureName);
}

//-----------------------------------------------------------------------------

bool LevelInfo::onAdd()
{
   if ( !Parent::onAdd() )
      return false;
      
   // If no sound ambience has been set, default to
   // 'AudioAmbienceDefault'.
      
   if( !mSoundAmbience )
      Sim::findObject( "AudioAmbienceDefault", mSoundAmbience );
      
   // Set up sound on client.
   
   if( isClientObject() )
   {
      SFX->setDistanceModel( mSoundDistanceModel );
      
      // Set up the global ambient soundscape.
      
      mSoundscape = SFX->getSoundscapeManager()->getGlobalSoundscape();
      if( mSoundAmbience )
         mSoundscape->setAmbience( mSoundAmbience );
   }

   _updateSceneGraph();

   return true;
}

//-----------------------------------------------------------------------------

void LevelInfo::onRemove()
{
   if( mSoundscape )
      mSoundscape->setAmbience( &sDefaultAmbience );

   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void LevelInfo::_updateSceneGraph()
{
   // Clamp above zero before setting on the sceneGraph.
   // If we don't we get serious crashes.
   if ( mNearClip <= 0.0f )
      mNearClip = 0.001f;

   SceneManager* scene = isClientObject() ? gClientSceneGraph : gServerSceneGraph;
   
   scene->setNearClip( mNearClip );
   scene->setVisibleDistance( mVisibleDistance );
   scene->setVisibleGhostDistance( mVisibleGhostDistance );

   gDecalBias = mDecalBias;

   // Set ambient lighting properties.

   scene->setAmbientLightTransitionTime( mAmbientLightBlendPhase * 1000.f );
   scene->setAmbientLightTransitionCurve( mAmbientLightBlendCurve );

   // Copy our AirFogData into the sceneGraph.
   scene->setFogData( mFogData );

   // If the level info specifies that MRT pre-pass should be used in this scene
   // enable it via the appropriate light manager
   // (Basic lighting doesn't do anything different right now)
#ifndef TORQUE_DEDICATED
   if(isClientObject())
      _onLMActivate(LIGHTMGR->getId(), true);
#endif

   // TODO: This probably needs to be moved.
   gCanvasClearColor = mCanvasClearColor;
}

//-----------------------------------------------------------------------------

void LevelInfo::_onLMActivate(const char *lm, bool enable)
{
#ifndef TORQUE_DEDICATED
   // Advanced light manager
   if(enable && String(lm) == String("ADVLM"))
   {
      AssertFatal(dynamic_cast<AdvancedLightManager *>(LIGHTMGR), "Bad light manager type!");
      AdvancedLightManager *lightMgr = static_cast<AdvancedLightManager *>(LIGHTMGR);
      lightMgr->getLightBinManager()->MRTLightmapsDuringPrePass(mAdvancedLightmapSupport);
   }
#endif
}

bool LevelInfo::_setLevelAccuTexture(void *object, const char *index, const char *data)
{
   LevelInfo* volume = reinterpret_cast< LevelInfo* >(object);
   volume->setLevelAccuTexture(data);
   return false;
}


void LevelInfo::setLevelAccuTexture(const String& name)
{
   mAccuTextureName = name;
   if (isClientObject() && mAccuTextureName.isNotEmpty())
   {
      mAccuTexture.set(mAccuTextureName, &GFXDefaultStaticDiffuseProfile, "AccumulationVolume::mAccuTexture");
      if (mAccuTexture.isNull())
         Con::warnf("AccumulationVolume::setTexture - Unable to load texture: %s", mAccuTextureName.c_str());
      else
         gLevelAccuMap = mAccuTexture;
   }
   AccumulationVolume::refreshVolumes();
}