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
#include "sfx/sfxSystem.h"

#include "sfx/sfxProvider.h"
#include "sfx/sfxDevice.h"
#include "sfx/sfxInternal.h"
#include "sfx/sfxSource.h"
#include "sfx/sfxProfile.h"
#include "sfx/sfxDescription.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxPlayList.h"
#include "sfx/sfxSound.h"
#include "sfx/sfxController.h"
#include "sfx/sfxSoundscape.h"

#include "console/console.h"
#include "console/engineAPI.h"
#include "T3D/gameBase/processList.h"
#include "platform/profiler.h"
#include "platform/platformTimer.h"
#include "core/util/autoPtr.h"
#include "core/module.h"

#include "sfx/media/sfxWavStream.h"
#ifdef TORQUE_OGGVORBIS
   #include "sfx/media/sfxVorbisStream.h"
#endif


MODULE_BEGIN( SFX )

   MODULE_INIT_BEFORE( Sim )
   MODULE_SHUTDOWN_BEFORE( Sim ) // Make sure all SimObjects disappear in time.

   MODULE_INIT
   {
      SFXSystem::init();
   }
   
   MODULE_SHUTDOWN
   {
      SFXSystem::destroy();
   }

MODULE_END;


SFXSystem* SFXSystem::smSingleton = NULL;


// Excludes Null and Blocked as these are not passed out to the control layer.
ImplementEnumType( SFXStatus,
   "Playback status of sound source.\n"
   "@ingroup SFX" )
   { SFXStatusPlaying, "Playing",
      "The source is currently playing." },
   { SFXStatusStopped, "Stopped",
      "Playback of the source is stopped.  When transitioning to Playing state, playback will start at the beginning "
         "of the source." },
   { SFXStatusPaused, "Paused",
      "Playback of the source is paused.  Resuming playback will play from the current playback position." },
EndImplementEnumType;

ImplementEnumType( SFXDistanceModel,
   "Type of volume distance attenuation curve.\n"
   "The distance model determines the falloff curve applied to the volume of 3D sounds over distance.\n\n"
   "@ref SFXSource_volume\n\n"
   "@ref SFX_3d\n\n"
   "@ingroup SFX" )
   { SFXDistanceModelLinear, "Linear",
      "Volume attenuates linearly from the references distance onwards to max distance where it reaches zero." },
   { SFXDistanceModelLogarithmic, "Logarithmic", 
      "Volume attenuates logarithmically starting from the reference distance and halving every reference distance step from there on. "
      "Attenuation stops at max distance but volume won't reach zero." },
EndImplementEnumType;

ImplementEnumType( SFXChannel,
   "Channels are individual properties of sound sources that may be animated over time.\n\n"
   "@see SFXParameter\n\n"
   "@ref SFX_interactive\n\n"
   "@ingroup SFX" )
   { SFXChannelVolume,             "Volume",
      "Channel controls volume level of attached sound sources.\n"
      "@see SFXDescription::volume" },
   { SFXChannelPitch,              "Pitch",
      "Channel controls pitch of attached sound sources.\n"
      "@see SFXDescription::pitch" },
   { SFXChannelPriority,           "Priority",
      "Channel controls virtualizaton priority level of attached sound sources.\n"
      "@see SFXDescription::priority" },
   { SFXChannelPositionX,          "PositionX",
      "Channel controls X coordinate of 3D sound position of attached sources." },
   { SFXChannelPositionY,          "PositionY",
      "Channel controls Y coordinate of 3D sound position of attached sources." },
   { SFXChannelPositionZ,          "PositionZ",
      "Channel controls Z coordinate of 3D sound position of attached sources." },
   { SFXChannelRotationX,          "RotationX",
      "Channel controls X rotation (in degrees) of 3D sound orientation of attached sources." },
   { SFXChannelRotationY,          "RotationY",
      "Channel controls Y rotation (in degrees) of 3D sound orientation of attached sources." },
   { SFXChannelRotationZ,          "RotationZ",
      "Channel controls Z rotation (in degrees) of 3D sound orientation of attached sources." },
   { SFXChannelVelocityX,          "VelocityX",
      "Channel controls X coordinate of 3D sound velocity vector of attached sources." },
   { SFXChannelVelocityY,          "VelocityY",
      "Channel controls Y coordinate of 3D sound velocity vector of attached sources." },
   { SFXChannelVelocityZ,          "VelocityZ",
      "Channel controls Z coordinate of 3D sound velocity vector of attached sources." },
   { SFXChannelMinDistance,        "ReferenceDistance",
      "Channel controls reference distance of 3D sound of attached sources.\n"
      "@see SFXDescription::referenceDistance" },
   { SFXChannelMaxDistance,        "MaxDistance",
      "Channel controls max volume attenuation distance of 3D sound of attached sources.\n"
      "@see SFXDescription::maxDistance" },
   { SFXChannelConeInsideAngle,    "ConeInsideAngle",
      "Channel controls angle (in degrees) of 3D sound inner volume cone of attached sources.\n"
      "@see SFXDescription::coneInsideAngle" },
   { SFXChannelConeOutsideAngle,   "ConeOutsideAngle",
      "Channel controls angle (in degrees) of 3D sound outer volume cone of attached sources.\n"
      "@see SFXDescription::coneOutsideAngle" },
   { SFXChannelConeOutsideVolume,  "ConeOutsideVolume",
      "Channel controls volume outside of 3D sound outer cone of attached sources.\n"
      "@see SFXDescription::coneOutsideVolume" },
   { SFXChannelCursor,             "Cursor",
      "Channel controls playback cursor of attached sound sources.\n\n"
      "@note Be aware that different types of sound sources interpret play cursor positions differently "
         "or do not actually have play cursors (these sources will ignore the channel)." },
   { SFXChannelStatus,             "Status",
      "Channel controls playback status of attached sound sources.\n\n"
      "The channel's value is rounded down to the nearest integer and interpreted in the following way:\n"
      "- 1: Play\n"
      "- 2: Stop\n"
      "- 3: Pause\n\n" },
   { SFXChannelUser0,              "User0",
      "Channel available for custom use.  By default ignored by sources.\n\n"
      "@note For FMOD Designer event sources (SFXFMODEventSource), this channel is used for event parameters "
         "defined in FMOD Designer and should not be used otherwise.\n\n"
      "@see SFXSource::onParameterValueChange" },
   { SFXChannelUser1,              "User1",
      "Channel available for custom use.  By default ignored by sources.\n\n"
      "@see SFXSource::onParameterValueChange" },
   { SFXChannelUser2,              "User2",
      "Channel available for custom use.  By default ignored by sources.\n\n"
      "@see SFXSource::onParameterValueChange" },
   { SFXChannelUser3,              "User3",
      "Channel available for custom use.  By default ignored by sources.\n\n"
      "@see SFXSource::onParameterValueChange" },
EndImplementEnumType;


// Constants.
static const U32 sDeviceCapsReverb = SFXDevice::CAPS_Reverb;
static const U32 sDeviceCapsVoiceManagement = SFXDevice::CAPS_VoiceManagement;
static const U32 sDeviceCapsOcclusion = SFXDevice::CAPS_Occlusion;
static const U32 sDeviceCapsDSPEffects = SFXDevice::CAPS_DSPEffects;
static const U32 sDeviceCapsMultiListener = SFXDevice::CAPS_MultiListener;
static const U32 sDeviceCapsFMODDesigner = SFXDevice::CAPS_FMODDesigner;

static const U32 sDeviceInfoProvider = 0;
static const U32 sDeviceInfoName = 1;
static const U32 sDeviceInfoUseHardware = 2;
static const U32 sDeviceInfoMaxBuffers = 3;
static const U32 sDeviceInfoCaps = 4;


//-----------------------------------------------------------------------------

SFXSystem::SFXSystem()
   :  mDevice( NULL ),
      mLastSourceUpdateTime( 0 ),
      mLastAmbientUpdateTime( 0 ),
      mLastParameterUpdateTime( 0 ),
      mStatNumSources( 0 ),
      mStatNumSounds( 0 ),
      mStatNumPlaying( 0 ),
      mStatNumCulled( 0 ),
      mStatNumVoices( 0 ),
      mStatSourceUpdateTime( 0 ),
      mStatParameterUpdateTime( 0 ),
      mStatAmbientUpdateTime( 0 ),
      mDistanceModel( SFXDistanceModelLinear ),
      mDopplerFactor( 0.5 ),
      mRolloffFactor( 1.0 ),
      mSoundscapeMgr( NULL )
{
   VECTOR_SET_ASSOCIATION( mSounds );
   VECTOR_SET_ASSOCIATION( mPlayOnceSources );
   VECTOR_SET_ASSOCIATION( mPlugins );
   VECTOR_SET_ASSOCIATION( mListeners );
   
   // Always at least one listener.
   
   mListeners.increment();
   
   // Register stat variables.

   Con::addVariable( "SFX::numSources", TypeS32, &mStatNumSources,
      "Number of SFXSource type objects that are currently instantiated.\n"
      "@ingroup SFX" );
   Con::addVariable( "SFX::numSounds", TypeS32, &mStatNumSounds,
      "Number of SFXSound type objects (i.e. actual single-file sounds) that are currently instantiated.\n"
      "@ingroup SFX" );
   Con::addVariable( "SFX::numPlaying", TypeS32, &mStatNumPlaying,
      "Number of SFXSources that are currently in playing state.\n"
      "@ingroup SFX" );
   Con::addVariable( "SFX::numCulled", TypeS32, &mStatNumCulled,
      "Number of SFXSounds that are currently in virtualized playback mode.\n"
      "@ref SFXSound_virtualization\n\n"
      "@ingroup SFX" );
   Con::addVariable( "SFX::numVoices", TypeS32, &mStatNumVoices,
      "Number of voices that are currently allocated on the sound device.\n"
      "@ingroup SFX" );
   Con::addVariable( "SFX::sourceUpdateTime", TypeS32, &mStatSourceUpdateTime,
      "Milliseconds spent on the last SFXSource update loop.\n"
      "@ref SFX_updating\n\n"
      "@ingroup SFX" );
   Con::addVariable( "SFX::parameterUpdateTime", TypeS32, &mStatParameterUpdateTime,
      "Milliseconds spent on the last SFXParameter update loop.\n"
      "@ref SFX_updating\n\n"
      "@ref SFX_interactive\n\n"
      "@ingroup SFX" );
   Con::addVariable( "SFX::ambientUpdateTime", TypeS32, &mStatAmbientUpdateTime,
      "Milliseconds spent on the last ambient audio update.\n"
      "@ref SFX_updating\n\n"
      "@ref SFX_ambient\n\n"
      "@ingroup SFX" );
   
   // Register constants.
   
   Con::addConstant( "$SFX::DEVICE_CAPS_REVERB", TypeS32, &sDeviceCapsReverb,
      "Sound device capability flag indicating that the sound device supports reverb.\n\n"
      "@note Currently only FMOD implements this.\n\n"
      "@see sfxGetDeviceInfo\n\n"
      "@ref SFX_reverb\n\n"
      "@ingroup SFX" );
   Con::addConstant( "$SFX::DEVICE_CAPS_VOICEMANAGEMENT", TypeS32, &sDeviceCapsVoiceManagement,
      "Sound device capability flag indicating that the sound device implements its own voice virtualization.\n\n"
      "For these devices, the sound system will deactivate its own voice management and leave voice "
         "virtualization entirely to the device.\n\n"
      "@note Currently only FMOD implements this.\n\n"
      "@see sfxGetDeviceInfo\n\n"
      "@ref SFXSound_virtualization\n\n"
      "@ingroup SFX" );
   Con::addConstant( "$SFX::DEVICE_CAPS_OCCLUSION", TypeS32, &sDeviceCapsOcclusion,
      "Sound device capability flag indicating that the sound device implements sound occlusion.\n\n"
      "@note This is not yet used by the sound system.\n\n"
      "@see sfxGetDeviceInfo\n\n"
      "@ingroup SFX" );
   Con::addConstant( "$SFX::DEVICE_CAPS_DSPEFFECTS", TypeS32, &sDeviceCapsDSPEffects,
      "Sound device capability flag indicating that the sound device supports adding DSP effect chains to sounds.\n\n"
      "@see sfxGetDeviceInfo\n\n"
      "@note This is not yet used by the sound system.\n\n"
      "@see sfxGetDeviceInfo\n\n"
      "@ingroup SFX" );
   Con::addConstant( "$SFX::DEVICE_CAPS_MULTILISTENER", TypeS32, &sDeviceCapsMultiListener,
      "Sound device capability flag indicating that the sound device supports multiple concurrent listeners.\n\n"
      "@note Currently only FMOD implements this.\n\n"
      "@see sfxGetDeviceInfo\n\n"
      "@ingroup SFX" );
   Con::addConstant( "$SFX::DEVICE_CAPS_FMODDESIGNER", TypeS32, &sDeviceCapsFMODDesigner,
      "Sound device capability flag indicating that the sound device supports FMOD Designer audio projects.\n\n"
      "@note This is exclusive to FMOD.  If the FMOD Event DLLs are in place and could be successfully loaded, this "
         "flag will be set after initializating an FMOD audio device.\n\n"
      "@see sfxGetDeviceInfo\n\n"
      "@ref FMOD_designer\n\n"
      "@ingroup SFX" );
      
   Con::addConstant( "$SFX::DEVICE_INFO_PROVIDER", TypeS32, &sDeviceInfoProvider,
      "Index of sound provider field in device info string.\n\n"
      "@see sfxGetDeviceInfo\n\n"
      "@see sfxGetAvailableDevices\n\n"
      "@ingroup SFX" );
   Con::addConstant( "$SFX::DEVICE_INFO_NAME", TypeS32, &sDeviceInfoName,
      "Index of device name field in device info string.\n\n"
      "@see sfxGetDeviceInfo\n\n"
      "@see sfxGetAvailableDevices\n\n"
      "@ingroup SFX" );
   Con::addConstant( "$SFX::DEVICE_INFO_USEHARDWARE", TypeS32, &sDeviceInfoUseHardware,
      "Index of use hardware flag in device info string.\n\n"
      "@see sfxGetDeviceInfo\n\n"
      "@see sfxGetAvailableDevices\n\n"
      "@ingroup SFX" );
   Con::addConstant( "$SFX::DEVICE_INFO_MAXBUFFERS", TypeS32, &sDeviceInfoMaxBuffers,
      "Index of buffer limit number in device info string.\n\n"
      "@see sfxGetDeviceInfo\n\n"
      "@see sfxGetAvailableDevices\n\n"
      "@ingroup SFX" );
   Con::addConstant( "$SFX::DEVICE_INFO_CAPS", TypeS32, &sDeviceInfoMaxBuffers,
      "Index of device capability flags in device info string.\n\n"
      "@see sfxGetDeviceInfo\n\n"
      "@see sfxGetAvailableDevices\n\n"
      "@ingroup SFX" );

   // Create subsystems.
   
   mSoundscapeMgr = new SFXSoundscapeManager();
}

//-----------------------------------------------------------------------------

SFXSystem::~SFXSystem()
{
   // Unregister stat variables.
   
   Con::removeVariable( "SFX::numSources" );
   Con::removeVariable( "SFX::numSounds" );
   Con::removeVariable( "SFX::numPlaying" );
   Con::removeVariable( "SFX::numCulled" );
   Con::removeVariable( "SFX::numVoices" );
   Con::removeVariable( "SFX::sourceUpdateTime" );
   Con::removeVariable( "SFX::parameterUpdateTime" );
   Con::removeVariable( "SFX::ambientUpdateTime" );
   
   // Cleanup any remaining sources.
   
   if( Sim::getSFXSourceSet() )
      Sim::getSFXSourceSet()->deleteAllObjects();

   mSounds.clear();
   mPlayOnceSources.clear();
   mListeners.clear();
   
   // Delete subsystems.
   
   if( mSoundscapeMgr )
      SAFE_DELETE( mSoundscapeMgr );
      
   // Delete device if we still have one.
   
   deleteDevice();
}

//-----------------------------------------------------------------------------

void SFXSystem::init()
{
   AssertWarn( smSingleton == NULL, "SFX has already been initialized!" );

   SFXProvider::initializeAllProviders();

   // Register the streams and resources.  Note that 
   // the order here does matter!
   SFXFileStream::registerExtension( ".wav", ( SFXFILESTREAM_CREATE_FN ) SFXWavStream::create );
   #ifdef TORQUE_OGGVORBIS
      SFXFileStream::registerExtension( ".ogg", ( SFXFILESTREAM_CREATE_FN ) SFXVorbisStream::create );
   #endif
   
   // Create the stream thread pool.
   
   SFXInternal::SFXThreadPool::createSingleton();

   // Note: If you have provider specific file types you should
   // register them in the provider initialization.

   // Create the system.
   smSingleton = new SFXSystem();   
}

//-----------------------------------------------------------------------------

void SFXSystem::destroy()
{
   AssertWarn( smSingleton != NULL, "SFX has not been initialized!" );

   SFXFileStream::unregisterExtension( ".wav" );
   #ifdef TORQUE_OGGVORBIS
      SFXFileStream::unregisterExtension( ".ogg" );
   #endif

   delete smSingleton;
   smSingleton = NULL;
   
   // Destroy the stream thread pool

   SFXInternal::SFXThreadPool::deleteSingleton();
}

//-----------------------------------------------------------------------------

void SFXSystem::addPlugin( SFXSystemPlugin* plugin )
{
   for( U32 i = 0; i < mPlugins.size(); ++ i )
      AssertFatal( mPlugins[ i ] != plugin, "SFXSystem::addPlugin - plugin already added to the system!" );
      
   mPlugins.push_back( plugin );
}

//-----------------------------------------------------------------------------

void SFXSystem::removePlugin( SFXSystemPlugin* plugin )
{
   for( U32 i = 0; i < mPlugins.size(); ++ i )
      if( mPlugins[ i ] == plugin )
      {
         mPlugins.erase_fast( i );
         break;
      }
}

//-----------------------------------------------------------------------------

bool SFXSystem::createDevice( const String& providerName, const String& deviceName, bool useHardware, S32 maxBuffers, bool changeDevice )
{
   // Make sure we don't have a device already.
   
   if( mDevice && !changeDevice )
      return false;

   // Lookup the provider.
   
   SFXProvider* provider = SFXProvider::findProvider( providerName );
   if( !provider )
      return false;

   // If we have already created this device and are using it then no need to do anything.
   
   if( mDevice
       && providerName.equal( mDevice->getProvider()->getName(), String::NoCase )
       && deviceName.equal( mDevice->getName(), String::NoCase )
       && useHardware == mDevice->getUseHardware() )
      return true;

   // If we have an existing device remove it.
   
   if( mDevice )
      deleteDevice();

   // Create the new device..
   
   mDevice = provider->createDevice( deviceName, useHardware, maxBuffers );
   if( !mDevice )
   {
      Con::errorf( "SFXSystem::createDevice - failed creating %s device '%s'", providerName.c_str(), deviceName.c_str() );
      return false;
   }
   
   // Print capabilities.

   Con::printf( "SFXSystem::createDevice - created %s device '%s'", providerName.c_str(), deviceName.c_str() );
   if( mDevice->getCaps() & SFXDevice::CAPS_Reverb )
      Con::printf( "   CAPS_Reverb" );
   if( mDevice->getCaps() & SFXDevice::CAPS_VoiceManagement )
      Con::printf( "   CAPS_VoiceManagement" );
   if( mDevice->getCaps() & SFXDevice::CAPS_Occlusion )
      Con::printf( "\tCAPS_Occlusion" );
   if( mDevice->getCaps() & SFXDevice::CAPS_MultiListener )
      Con::printf( "\tCAPS_MultiListener" );
      
   // Set defaults.
   
   mDevice->setNumListeners( getNumListeners() );
   mDevice->setDistanceModel( mDistanceModel );
   mDevice->setDopplerFactor( mDopplerFactor );
   mDevice->setRolloffFactor( mRolloffFactor );
   mDevice->setReverb( mReverb );
      
   // Signal system.

   getEventSignal().trigger( SFXSystemEvent_CreateDevice );
   
   return true;
}

//-----------------------------------------------------------------------------

String SFXSystem::getDeviceInfoString()
{
   // Make sure we have a valid device.
   if( !mDevice )
      return String();

   return String::ToString( "%s\t%s\t%s\t%d\t%d",
      mDevice->getProvider()->getName().c_str(),
      mDevice->getName().c_str(),
      mDevice->getUseHardware() ? "1" : "0",
      mDevice->getMaxBuffers(),
      mDevice->getCaps() );
}

//-----------------------------------------------------------------------------

void SFXSystem::deleteDevice()
{
   // Make sure we have a valid device.
   if ( !mDevice )
      return;

   // Put all playing sounds into virtualized playback mode.  Where this fails,
   // stop the source.
   
   for( U32 i = 0; i < mSounds.size(); ++ i )
   {
      SFXSound* sound = mSounds[ i ];
      if( sound->hasVoice() && !sound->_releaseVoice() )
         sound->stop();
   }

   // Signal everyone who cares that the
   // device is being deleted.
   getEventSignal().trigger( SFXSystemEvent_DestroyDevice );

   // Free the device which should delete all
   // the active voices and buffers.
   delete mDevice;
   mDevice = NULL;
}

//-----------------------------------------------------------------------------

SFXSource* SFXSystem::createSource( SFXTrack* track,
                                    const MatrixF* transform, 
                                    const VectorF* velocity )
{
   if( !track )
      return NULL;
            
   SFXSource* source = NULL;

   // Try creating through a plugin first so that they
   // always get the first shot and may override our
   // logic here.
   
   for( U32 i = 0; !source && i < mPlugins.size(); ++ i )
      source = mPlugins[ i ]->createSource( track );
      
   // If that failed, try our own logic using the track
   // types that we know about.
   
   if( !source )
   {
      if( !mDevice )
      {
         Con::errorf( "SFXSystem::createSource() - no device initialized!" );
         return NULL;
      }
      
      if( dynamic_cast< SFXPlayList* >( track ) )
      {
         // Create a playback controller for the track.
         
         SFXPlayList* playList = static_cast< SFXPlayList* >( track );
         source = SFXController::_create( playList );
      }
      else if( dynamic_cast< SFXProfile* >( track ) )
      {
         // Create a sound.
         
         SFXProfile* profile = static_cast< SFXProfile* >( track );
         source = SFXSound::_create( mDevice, profile );
         if( !source )
         {
            Con::errorf( 
               "SFXSystem::createSource() - Failed to create sound!\n"
               "  Profile: %s\n"
               "  Filename: %s",
               profile->getName(),
               profile->getSoundFileName().c_str() );
         }
      }
      else
      {
         Con::errorf( "SFXSystem::createSource - cannot create source for %i (%s) of type '%s'",
            track->getId(), track->getName(), track->getClassName() );
         Con::errorf( "SFXSystem::createSource - maybe you are using the wrong SFX provider for this type of track" );
         return NULL;
      }
   }
   
   if( source )
   {
      if( transform )
         source->setTransform( *transform );
      if( velocity )
         source->setVelocity( *velocity );
   }
      
   return source;
}

//-----------------------------------------------------------------------------

SFXSound* SFXSystem::createSourceFromStream( const ThreadSafeRef< SFXStream >& stream,
                                              SFXDescription* description )
{
   AssertFatal( mDevice, "SFXSystem::createSourceFromStream() - no device initialized!" );

   // We sometimes get null values from script... fail in that case.

   if( !stream || !description )
      return NULL;

   // Create the sound.

   SFXSound* sound = SFXSound::_create( mDevice, stream, description );
   if( !sound )
      return NULL;

   return sound;
}

//-----------------------------------------------------------------------------

void SFXSystem::stopAndDeleteSource( SFXSource* source )
{
   if( source->getFadeOutTime() > 0.f )
   {
      // Fade-out.  Stop and put on play-once list to
      // ensure deletion when the source actually stops.
      
      source->stop();
      deleteWhenStopped( source );
   }
   else
   {
      // No fade-out.  Just stop and delete the source.
      
      source->stop();
      SFX_DELETE( source );
   }
}

//-----------------------------------------------------------------------------

void SFXSystem::deleteWhenStopped( SFXSource* source )
{
   // If the source isn't already on the play-once source list,
   // put it there now.
   
   Vector< SFXSource* >::iterator iter = find( mPlayOnceSources.begin(), mPlayOnceSources.end(), source );
   if( iter == mPlayOnceSources.end() )
      mPlayOnceSources.push_back( source );
}

//-----------------------------------------------------------------------------

void SFXSystem::_onAddSource( SFXSource* source )
{
   if( dynamic_cast< SFXSound* >( source ) )
   {
      SFXSound* sound = static_cast< SFXSound* >( source );
      mSounds.push_back( sound );
      
      mStatNumSounds = mSounds.size();
   }

   // Update the stats.
   mStatNumSources ++;
}

//-----------------------------------------------------------------------------

void SFXSystem::_onRemoveSource( SFXSource* source )
{
   // Check if it was a play once source.
   
   Vector< SFXSource* >::iterator iter = find( mPlayOnceSources.begin(), mPlayOnceSources.end(), source );
   if ( iter != mPlayOnceSources.end() )
      mPlayOnceSources.erase_fast( iter );

   // Update the stats.
   
   mStatNumSources --;
   
   if( dynamic_cast< SFXSound* >( source ) )
   {
      SFXSoundVector::iterator iter = find( mSounds.begin(), mSounds.end(), static_cast< SFXSound* >( source ) );
      mSounds.erase_fast( iter );
         
      mStatNumSounds = mSounds.size();
   }
}

//-----------------------------------------------------------------------------

SFXBuffer* SFXSystem::_createBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description )
{
   // The buffers are created by the active
   // device... without one we cannot do anything.
   if ( !mDevice )
   {
      Con::errorf( "SFXSystem::_createBuffer - No sound device initialized!!" );
      return NULL;
   }

   // Some sanity checking for streaming.  If the stream isn't at least three packets
   // in size given the current settings in "description", then turn off streaming.
   // The device code *will* mess up if the stream input fails to match certain metrics.
   // Just disabling streaming when it doesn't make sense is easier than complicating the
   // device logic to account for bad metrics.

   bool streamFlag = description->mIsStreaming;
   if( description->mIsStreaming
       && stream->getDuration() < description->mStreamPacketSize * 1000 * SFXInternal::SFXAsyncQueue::DEFAULT_STREAM_QUEUE_LENGTH )
      description->mIsStreaming = false;

   SFXBuffer* buffer = mDevice->createBuffer( stream, description );

   description->mIsStreaming = streamFlag; // restore in case we stomped it
   return buffer;
}

//-----------------------------------------------------------------------------

SFXBuffer* SFXSystem::_createBuffer( const String& filename, SFXDescription* description )
{
   if( !mDevice )
   {
      Con::errorf( "SFXSystem::_createBuffer - No sound device initialized!!" );
      return NULL;
   }
      
   return mDevice->createBuffer( filename, description );
}

//-----------------------------------------------------------------------------

SFXSource* SFXSystem::playOnce(  SFXTrack* track,
                                 const MatrixF *transform,
                                 const VectorF *velocity,
                                 F32 fadeInTime )
{
   // We sometimes get null profiles... nothing to play without a profile!
   if( !track )
      return NULL;

   SFXSource *source = createSource( track, transform, velocity );
   if( source )
   {
      mPlayOnceSources.push_back( source ); 
      source->play( fadeInTime );
   }

   return source;
}

//-----------------------------------------------------------------------------

void SFXSystem::_update()
{
   PROFILE_SCOPE( SFXSystem_Update );
   
   getEventSignal().trigger( SFXSystemEvent_Update );
   
   for( U32 i = 0; i < mPlugins.size(); ++ i )
      mPlugins[ i ]->update();
   
   const U32 SOURCE_UPDATE_MS = TickMs * 2;
   const U32 PARAMETER_UPDATE_MS = TickMs * 3;
   const U32 AMBIENT_UPDATE_MS = TickMs * 4;
   
   static AutoPtr< PlatformTimer > sTimer;
   if( sTimer.isNull() )
      sTimer = PlatformTimer::create();

   // The update of the sources can be a bit expensive
   // and it does not need to be updated every frame.
   const U32 currentTime = Platform::getRealMilliseconds();
   if( ( currentTime - mLastSourceUpdateTime ) >= SOURCE_UPDATE_MS )
   {
      S32 tick = sTimer->getElapsedMs();
      
      _updateSources();
      mLastSourceUpdateTime = currentTime;
      
      mStatSourceUpdateTime = ( sTimer->getElapsedMs() - tick );
   }
   if( ( currentTime - mLastParameterUpdateTime ) >= PARAMETER_UPDATE_MS )
   {
      S32 tick = sTimer->getElapsedMs();

      SimSet* set = Sim::getSFXParameterGroup();
      for( SimSet::iterator iter = set->begin(); iter != set->end(); ++ iter )
      {
         SFXParameter* parameter = dynamic_cast< SFXParameter* >( *iter );
         if( parameter )
            parameter->update();
      }
         
      mLastParameterUpdateTime = currentTime;
      mStatParameterUpdateTime = ( sTimer->getElapsedMs() - tick );
   }
   if( mSoundscapeMgr && ( currentTime - mLastAmbientUpdateTime ) >= AMBIENT_UPDATE_MS )
   {
      S32 tick = sTimer->getElapsedMs();

      mSoundscapeMgr->update();
      mLastAmbientUpdateTime = currentTime;

      mStatAmbientUpdateTime = ( sTimer->getElapsedMs() - tick );
   }

   // If we have a device then update it.
   if( mDevice )
      mDevice->update();
}

//-----------------------------------------------------------------------------

void SFXSystem::_updateSources()
{
   PROFILE_SCOPE( SFXSystem_UpdateSources );

   SimSet* sources = Sim::getSFXSourceSet();
   if( !sources )
      return;

   // Check the status of the sources here once.
   // 
   // NOTE: We do not use iterators in this loop because
   // SFXControllers can add to the source list during the
   // loop.
   //
   mStatNumPlaying = 0;
   for( S32 i=0; i < sources->size(); i++ )
   {
      SFXSource *source = dynamic_cast< SFXSource* >( sources->at( i ) );
      if ( source )
      {
         source->update();
         if( source->getStatus() == SFXStatusPlaying )
            ++ mStatNumPlaying;
      }
   }

   // First check to see if any play once sources have
   // finished playback... delete them.
   
   for( SFXSourceVector::iterator iter = mPlayOnceSources.begin(); iter != mPlayOnceSources.end();  )
   {
      SFXSource* source = *iter;

      if(   source->getLastStatus() == SFXStatusStopped &&
            source->getSavedStatus() != SFXStatusPlaying )
      {
         int index = iter - mPlayOnceSources.begin();

         // Erase it from the vector first, so that onRemoveSource
         // doesn't do it during cleanup and screw up our loop here!
         mPlayOnceSources.erase_fast( iter );
         source->deleteObject();

         iter = mPlayOnceSources.begin() + index;
         continue;
      }

      ++ iter;
   }

   
   if( mDevice )
   {
      // Reassign buffers to the sounds (if voices are managed by
      // us instead of by the device).
      
      if( !( mDevice->getCaps() & SFXDevice::CAPS_VoiceManagement ) )
         _assignVoices();

      // Update the voice count stat.
      mStatNumVoices = mDevice->getVoiceCount();
   }
}

//-----------------------------------------------------------------------------

void SFXSystem::_sortSounds( const SFXListenerProperties& listener )
{   
   PROFILE_SCOPE( SFXSystem_SortSounds );
   
   // Sort the source vector by the attenuated 
   // volume and priorities.  This leaves us
   // with the loudest and highest priority sounds 
   // at the front of the vector.
   
   dQsort( ( void* ) mSounds.address(), mSounds.size(), sizeof( SFXSound* ), SFXSound::qsortCompare );
}

//-----------------------------------------------------------------------------

void SFXSystem::_assignVoices()
{
   AssertFatal( getNumListeners() == 1, "SFXSystem::_assignVoices() - must only have a single listener" );
   PROFILE_SCOPE( SFXSystem_AssignVoices );

   mStatNumVoices = 0;
   mStatNumCulled = 0;
   
   if( !mDevice )
      return;
      
   // Sort the sources in the SFX source set by priority.  This also
   // updates each soures effective volume first.
   
   _sortSounds( getListener() );

   // We now make sure that the sources closest to the 
   // listener, the ones at the top of the source list,
   // have a device buffer to play thru.
   
   mStatNumCulled = 0;
   for( SFXSoundVector::iterator iter = mSounds.begin(); iter != mSounds.end(); ++ iter )
   {
      SFXSound* sound = *iter;

      // Non playing sources (paused or stopped) are at the
      // end of the vector, so when i encounter one i know 
      // that nothing else in the vector needs buffer assignment.
      
		if( !sound->isPlaying() )
         break;

      // If the source is outside it's max range we can
      // skip it as well, so that we don't waste cycles
      // setting up a buffer for something we won't hear.
      
      if( sound->getAttenuatedVolume() <= 0.0f )
      {
         ++ mStatNumCulled;
         continue;
      }

      // If the source has a voice then we can skip it.
      
      if( sound->hasVoice() )
         continue;

      // Ok let the device try to assign a new voice for 
      // this source... this may fail if we're out of voices.
      
      if( sound->_allocVoice( mDevice ) )
         continue;

      // The device couldn't assign a new voice, so we go through
      // local priority sounds and try to steal a voice.
      
      for( SFXSoundVector::iterator hijack = mSounds.end() - 1; hijack != iter; -- hijack )
      {
         SFXSound* other = *hijack;
         
         if( other->hasVoice() )
         {
            // If the sound is a suitable candidate, try to steal
            // its voice.  While the sound definitely is lower down the chain
            // in the total priority ordering, we don't want to steal voices
            // from sounds that are clearly audible as that results in noticable
            // sound pops.
            
            if( (    other->getAttenuatedVolume() < 0.1     // Very quiet or maybe not even audible.
                  || !other->isPlaying()                    // Not playing so not audible anyways.
                  || other->getPosition() == 0 )            // Not yet started playing.
                && other->_releaseVoice() )
               break;
         }
		}

      // Ok try to assign a voice once again!
      
      if( sound->_allocVoice( mDevice ) )
         continue;

      // If the source still doesn't have a buffer... well
      // tough cookies.  It just cannot be heard yet, maybe
      // it can in the next update.
      
      mStatNumCulled ++;
	}

   // Update the voice count stat.
   mStatNumVoices = mDevice->getVoiceCount();
}

//-----------------------------------------------------------------------------

void SFXSystem::_assignVoice( SFXSound* sound )
{
   if( !mDevice )
      return;
      
   // Make sure all properties are up-to-date.
   
   sound->_update();

   // If voices are managed by the device, just let the sound
   // allocate a voice on it.  Otherwise, do a voice allocation pass
   // on all our active sounds.
      
   if( mDevice->getCaps() & SFXDevice::CAPS_VoiceManagement )
      sound->_allocVoice( mDevice );
   else
      _assignVoices();

   // Update the voice count stat.
   mStatNumVoices = mDevice->getVoiceCount();
}

//-----------------------------------------------------------------------------

void SFXSystem::setDistanceModel( SFXDistanceModel model )
{
   const bool changed = ( model != mDistanceModel );
   
   mDistanceModel = model;
   if( mDevice && changed )
      mDevice->setDistanceModel( model );
}

//-----------------------------------------------------------------------------

void SFXSystem::setDopplerFactor( F32 factor )
{
   const bool changed = ( factor != mDopplerFactor );
   
   mDopplerFactor = factor;
   if( mDevice && changed )
      mDevice->setDopplerFactor( factor );
}

//-----------------------------------------------------------------------------

void SFXSystem::setRolloffFactor( F32 factor )
{
   const bool changed = ( factor != mRolloffFactor );
   
   mRolloffFactor = factor;
   if( mDevice && changed )
      mDevice->setRolloffFactor( factor );
}

//-----------------------------------------------------------------------------

void SFXSystem::setReverb( const SFXReverbProperties& reverb )
{
   mReverb = reverb;
   
   // Allow the plugins to adjust the reverb.
   
   for( U32 i = 0; i < mPlugins.size(); ++ i )
      mPlugins[ i ]->filterReverb( mReverb );
      
   // Pass it on to the device.
   
   if( mDevice )
      mDevice->setReverb( mReverb );
}

//-----------------------------------------------------------------------------

void SFXSystem::setNumListeners( U32 num )
{
   // If we are set to a single listener, just accept this as
   // we always support this no matter what.
   
   if( num == 1 )
   {
      mListeners.setSize( 1 );
      if( mDevice )
         mDevice->setNumListeners( 1 );
      return;
   }
   
   // If setting to multiple listeners, make sure that the device
   // both supports multiple listeners and implements its own voice
   // management (as our voice virtualization does not work with more
   // than a single listener).
      
   if(    !mDevice || !( mDevice->getCaps() & SFXDevice::CAPS_MultiListener )
       || !( mDevice->getCaps() & SFXDevice::CAPS_VoiceManagement ) )
   {
      Con::errorf( "SFXSystem::setNumListeners() - multiple listeners not supported on current configuration" );
      return;
   }
   
   mListeners.setSize( num );
   if( mDevice )
      mDevice->setNumListeners( num );
}

//-----------------------------------------------------------------------------

void SFXSystem::setListener( U32 index, const MatrixF& transform, const Point3F& velocity )
{
   if( index >= mListeners.size() )
      return;
      
   mListeners[ index ] = SFXListenerProperties( transform, velocity );
   
   if( mDevice )
      mDevice->setListener( index, mListeners[ index ] );
}

//-----------------------------------------------------------------------------

void SFXSystem::notifyDescriptionChanged( SFXDescription* description )
{
   SimSet* set = Sim::getSFXSourceSet();
   for( SimSet::iterator iter = set->begin(); iter != set->end(); ++ iter )
   {
      SFXSource* source = dynamic_cast< SFXSource* >( *iter );
      if( source && source->getDescription() == description )
         source->notifyDescriptionChanged();
   }
}

//-----------------------------------------------------------------------------

void SFXSystem::notifyTrackChanged( SFXTrack* track )
{
   SimSet* set = Sim::getSFXSourceSet();
   for( SimSet::iterator iter = set->begin(); iter != set->end(); ++ iter )
   {
      SFXSource* source = dynamic_cast< SFXSource* >( *iter );
      if( source && source->getTrack() == track )
         source->notifyTrackChanged();
   }
}

//-----------------------------------------------------------------------------

void SFXSystem::dumpSources( StringBuilder* toString, bool excludeGroups )
{
   SimSet* sources = Sim::getSFXSourceSet();
   if( !sources )
      return;
      
   bool isFirst = true;
   for( SimSet::iterator iter = sources->begin(); iter != sources->end(); ++ iter )
   {
      SFXSource* source = dynamic_cast< SFXSource* >( *iter );      
      if( !source )
         continue;

      bool isGroup = typeid( *source ) == typeid( SFXSource );
      if( isGroup && excludeGroups )
         continue;

      bool isPlayOnce = false;
      for( U32 j = 0; j < mPlayOnceSources.size(); ++ j )
         if( mPlayOnceSources[ j ] == source )
         {
            isPlayOnce = true;
            break;
         }
         
      SFXSource* sourceGroup = source->getSourceGroup();

      SFXSound* sound = dynamic_cast< SFXSound* >( source );
      SFXController* controller = dynamic_cast< SFXController* >( source );

      if( toString )
         toString->format( "%s%5i: type=%s, status=%s, blocked=%s, volume=%.2f, priority=%.2f, virtual=%s, looping=%s, 3d=%s, group=%s, playtime=%.2f, playOnce=%s, streaming=%s, hasVoice=%s, track=%s",
            ( isFirst ? "" : "\n" ),
            source->getId(),
            ( isGroup ? "group" : sound ? "sound" : controller ? "list" : "other" ),
            source->isPlaying()
            ? "playing"
            : source->isPaused()
            ? "paused"
            : source->isStopped()
            ? "stopped"
            : "unknown",
            ( sound && sound->isBlocked() ? "1" : "0" ),
            source->getAttenuatedVolume(),
            source->getEffectivePriority(),
            ( sound && sound->isVirtualized() ? "1" : "0" ),
            ( sound && sound->isLooping() ) ? "1" : "0",
            source->getDescription()->mIs3D ? "1" : "0",
            sourceGroup ? sourceGroup->getName() : "",
            source->getElapsedPlayTimeCurrentCycle(),
            isPlayOnce ? "1" : "0",
            ( sound && sound->isStreaming() ? "1" : "0" ),
            ( sound && sound->hasVoice() ? "1" : "0" ),
            source->getTrack() ? source->getTrack()->getName() : ""
         );
      else
         Con::printf( "%5i: type=%s, status=%s, blocked=%s, volume=%.2f, priority=%.2f, virtual=%s, looping=%s, 3d=%s, group=%s, playtime=%.2f, playOnce=%s, streaming=%s, hasVoice=%s, track=%s",
            source->getId(),
            ( isGroup ? "group" : sound ? "sound" : controller ? "list" : "other" ),
            source->isPlaying()
            ? "playing"
            : source->isPaused()
            ? "paused"
            : source->isStopped()
            ? "stopped"
            : "unknown",
            ( sound && sound->isBlocked() ? "1" : "0" ),
            source->getAttenuatedVolume(),
            source->getEffectivePriority(),
            ( sound && sound->isVirtualized() ? "1" : "0" ),
            ( sound && sound->isLooping() ) ? "1" : "0",
            source->getDescription()->mIs3D ? "1" : "0",
            sourceGroup ? sourceGroup->getName() : "",
            source->getElapsedPlayTimeCurrentCycle(),
            isPlayOnce ? "1" : "0",
            ( sound && sound->isStreaming() ? "1" : "0" ),
            ( sound && sound->hasVoice() ? "1" : "0" ),
            source->getTrack() ? source->getTrack()->getName() : ""
         );
         
      isFirst = false;
   }
}

//=============================================================================
//    Console Functions.
//=============================================================================
// MARK: ---- Console Functions ----

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxGetAvailableDevices, const char*, (),,
   "Get a list of all available sound devices.\n"
   "The return value will be a newline-separated list of entries where each line describes one available sound "
   "device.  Each such line will have the following format:"
   "@verbatim\n"
      "provider TAB device TAB hasHardware TAB numMaxBuffers\n"
   "@endverbatim\n"
   "- provider: The name of the device provider (e.g. \"FMOD\").\n"
   "- device: The name of the device as returned by the device layer.\n"
   "- hasHardware: Whether the device supports hardware mixing or not.\n"
   "- numMaxBuffers: The maximum number of concurrent voices supported by the device's mixer.  If this limit "
      "limit is exceeded, i.e. if there are more active sounds playing at any one time, then voice virtualization "
      "will start culling voices and put them into virtualized playback mode.  Voice virtualization may or may not "
      "be provided by the device itself; if not provided by the device, it will be provided by Torque's sound system.\n\n"
   "@return A newline-separated list of information about all available sound devices.\n"
   "@see sfxCreateDevice\n"
   "@see sfxGetDeviceInfo\n\n"
   "@see $SFX::DEVICE_INFO_PROVIDER\n\n"
   "@see $SFX::DEVICE_INFO_NAME\n\n"
   "@see $SFX::DEVICE_INFO_USEHARDWARE\n\n"
   "@see $SFX::DEVICE_INFO_MAXBUFFERS\n\n"
   "@ref SFX_devices\n"
   "@ingroup SFX" )
{
   char* deviceList = Con::getReturnBuffer( 2048 );
   deviceList[0] = 0;

   SFXProvider* provider = SFXProvider::getFirstProvider();
   while ( provider )
   {
      // List the devices in this provider.
      const SFXDeviceInfoVector& deviceInfo = provider->getDeviceInfo();
      for ( S32 d=0; d < deviceInfo.size(); d++ )
      {
         const SFXDeviceInfo* info = deviceInfo[d];
         dStrcat( deviceList, provider->getName() );
         dStrcat( deviceList, "\t" );
         dStrcat( deviceList, info->name );
         dStrcat( deviceList, "\t" );
         dStrcat( deviceList, info->hasHardware ? "1" : "0" );
         dStrcat( deviceList, "\t" );
         dStrcat( deviceList, Con::getIntArg( info->maxBuffers ) );         
         dStrcat( deviceList, "\n" );
         
         //TODO: caps
      }

      provider = provider->getNextProvider();
   }

   return deviceList;
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxCreateDevice, bool, ( const char* provider, const char* device, bool useHardware, S32 maxBuffers ),,
   "Try to create a new sound device using the given properties.\n"
   "If a sound device is currently initialized, it will be uninitialized first.  However, be aware that in this case, "
   "if this function fails, it will not restore the previously active device but rather leave the sound system in an "
   "uninitialized state.\n\n"
   "Sounds that are already playing while the new device is created will be temporarily transitioned to virtualized "
   "playback and then resume normal playback once the device has been created.\n\n"
   "In the core scripts, sound is automatically set up during startup in the sfxStartup() function.\n\n"
   "@param provider The name of the device provider as returned by sfxGetAvailableDevices().\n"
   "@param device The name of the device as returned by sfxGetAvailableDevices().\n"
   "@param useHardware Whether to enabled hardware mixing on the device or not.  Only relevant if supported by the given device.\n"
   "@param maxBuffers The maximum number of concurrent voices for this device to use or -1 for the device to pick its own reasonable default."
   "@return True if the initialization was successful, false if not.\n"
   "@note This function must be called before any of the sound playback functions can be used.\n"
   "@see sfxGetAvailableDevices\n"
   "@see sfxGetDeviceInfo\n"
   "@see sfxDeleteDevice\n\n"
   "@ref SFX_devices\n"
   "@ingroup SFX" )
{
   return SFX->createDevice( provider, device, useHardware, maxBuffers, true );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxDeleteDevice, void, (),,
   "Delete the currently active sound device and release all its resources.\n"
   "SFXSources that are still playing will be transitioned to virtualized playback mode. "
   "When creating a new device, they will automatically transition back to normal playback.\n\n"
   "In the core scripts, this is done automatically for you during shutdown in the sfxShutdown() function.\n\n"
   "@see sfxCreateDevice\n\n"
   "@ref SFX_devices\n"
   "@ingroup SFX" )
{
   SFX->deleteDevice();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxGetDeviceInfo, const char*, (),,
   "Return information about the currently active sound device.\n"
   "The return value is a tab-delimited string of the following format:\n"
   "@verbatim\n"
      "provider TAB device TAB hasHardware TAB numMaxBuffers TAB caps\n"
   "@endverbatim\n"
   "- provider: The name of the device provider (e.g. \"FMOD\").\n"
   "- device: The name of the device as returned by the device layer.\n"
   "- hasHardware: Whether the device supports hardware mixing or not.\n"
   "- numMaxBuffers: The maximum number of concurrent voices supported by the device's mixer.  If this limit "
      "limit is exceeded, i.e. if there are more active sounds playing at any one time, then voice virtualization "
      "will start culling voices and put them into virtualized playback mode.  Voice virtualization may or may not "
      "be provided by the device itself; if not provided by the device, it will be provided by Torque's sound system.\n"
   "- caps: A bitfield of capability flags.\n\n"
   "@return A tab-separated list of properties of the currently active sound device or the empty string if no sound device has been initialized.\n"
   "@see sfxCreateDevice\n"
   "@see sfxGetAvailableDevices\n\n"
   "@see $SFX::DEVICE_INFO_PROVIDER\n\n"
   "@see $SFX::DEVICE_INFO_NAME\n\n"
   "@see $SFX::DEVICE_INFO_USEHARDWARE\n\n"
   "@see $SFX::DEVICE_INFO_MAXBUFFERS\n\n"
   "@see $SFX::DEVICE_INFO_CAPS\n\n"
   "@see $SFX::DEVICE_CAPS_REVERB\n\n"
   "@see $SFX::DEVICE_CAPS_VOICEMANAGEMENT\n\n"
   "@see $SFX::DEVICE_CAPS_OCCLUSION\n\n"
   "@see $SFX::DEVICE_CAPS_DSPEFFECTS\n\n"
   "@see $SFX::DEVICE_CAPS_MULTILISTENER\n\n"
   "@see $SFX::DEVICE_CAPS_FMODDESIGNER\n\n"
   "@ref SFX_devices\n"
   "@ingroup SFX" )
{
   String deviceInfo = SFX->getDeviceInfoString();
   if( deviceInfo.isEmpty() )
      return "";
      
   return Con::getReturnBuffer( deviceInfo );
}

//-----------------------------------------------------------------------------

static ConsoleDocFragment _sfxCreateSource1(
   "@brief Create a new source that plays the given track.\n\n"
   "The source will be returned in stopped state.  Call SFXSource::play() to start playback.\n\n"
   "In contrast to play-once sources, the source object will not be automatically deleted once playback stops. "
   "Call delete() to release the source object.\n\n"
   "This function will automatically create the right SFXSource type for the given SFXTrack.\n\n"
   "@param track The track the source should play.\n"
   "@return A new SFXSource for playback of the given track or 0 if no source could be created from the given track.\n\n"
   "@note Trying to create a source for a device-specific track type will fail if the currently selected device "
      "does not support the type.  Example: trying to create a source for an FMOD Designer event when not running FMOD.\n\n"
   "@tsexample\n"
   "// Create and play a source from a pre-existing profile:\n"
   "%source = sfxCreateSource( SoundFileProfile );\n"
   "%source.play();\n"
   "@endtsexample\n\n"
   "@ingroup SFX",
   NULL,
   "SFXSource sfxCreateSource( SFXTrack track );" );
static ConsoleDocFragment _sfxCreateSource2(
   "@brief Create a new source that plays the given track and position its 3D sounds source at the given coordinates (if it is a 3D sound).\n\n"
   "The source will be returned in stopped state.  Call SFXSource::play() to start playback.\n\n"
   "In contrast to play-once sources, the source object will not be automatically deleted once playback stops. "
   "Call delete() to release the source object.\n\n"
   "This function will automatically create the right SFXSource type for the given SFXTrack.\n\n"
   "@param track The track the source should play.\n"
   "@param x The X coordinate of the 3D sound position.\n"
   "@param y The Y coordinate of the 3D sound position.\n"
   "@param z The Z coordinate of the 3D sound position.\n"
   "@return A new SFXSource for playback of the given track or 0 if no source could be created from the given track.\n\n"
   "@note Trying to create a source for a device-specific track type will fail if the currently selected device "
      "does not support the type.  Example: trying to create a source for an FMOD Designer event when not running FMOD.\n\n"
   "@tsexample\n"
   "// Create and play a source from a pre-existing profile and position it at (100, 200, 300):\n"
   "%source = sfxCreateSource( SoundFileProfile, 100, 200, 300 );\n"
   "%source.play();\n"
   "@endtsexample\n\n"
   "@ingroup SFX",
   NULL,
   "SFXSource sfxCreateSource( SFXTrack track, float x, float y, float z );" );
static ConsoleDocFragment _sfxCreateSource3(
   "@brief Create a temporary SFXProfile from the given @a description and @a filename and then create and return a new source that plays the profile.\n\n"
   "The source will be returned in stopped state.  Call SFXSource::play() to start playback.\n\n"
   "In contrast to play-once sources, the source object will not be automatically deleted once playback stops. "
   "Call delete() to release the source object.\n\n"
   "@param description The description to use for setting up the temporary SFXProfile.\n"
   "@param filename Path to the sound file to play.\n"
   "@return A new SFXSource for playback of the given track or 0 if no source or no temporary profile could be created.\n\n"
   "@tsexample\n"
   "// Create a source for a music track:\n"
   "%source = sfxCreateSource( AudioMusicLoop2D, \"art/sound/backgroundMusic\" );\n"
   "%source.play();\n"
   "@endtsexample\n\n"
   "@see SFXProfile\n\n"
   "@ingroup SFX",
   NULL,
   "SFXSound sfxCreateSource( SFXDescription description, string filename );" );
static ConsoleDocFragment _sfxCreateSource4(
   "@brief Create a temporary SFXProfile from the given @a description and @a filename and then create and return a new source that plays the profile. "
   "Position the sound source at the given coordinates (if it is a 3D sound).\n\n"
   "The source will be returned in stopped state.  Call SFXSource::play() to start playback.\n\n"
   "In contrast to play-once sources, the source object will not be automatically deleted once playback stops. "
   "Call delete() to release the source object.\n\n"
   "@param description The description to use for setting up the temporary SFXProfile.\n"
   "@param filename Path to the sound file to play.\n"
   "@param x The X coordinate of the 3D sound position.\n"
   "@param y The Y coordinate of the 3D sound position.\n"
   "@param z The Z coordinate of the 3D sound position.\n"
   "@return A new SFXSource for playback of the given track or 0 if no source or no temporary profile could be created.\n\n"
   "@tsexample\n"
   "// Create a source for a music track and position it at (100, 200, 300):\n"
   "%source = sfxCreateSource( AudioMusicLoop3D, \"art/sound/backgroundMusic\", 100, 200, 300 );\n"
   "%source.play();\n"
   "@endtsexample\n\n"
   "@see SFXProfile\n\n"
   "@ingroup SFX",
   NULL,
   "SFXSound sfxCreateSource( SFXDescription description, string filename, float x, float y, float z );" );

ConsoleFunction( sfxCreateSource, S32, 2, 6,
                     "( SFXTrack track | ( SFXDescription description, string filename ) [, float x, float y, float z ] ) "
                     "Creates a new paused sound source using a profile or a description "
                     "and filename.  The return value is the source which must be "
                     "released by delete().\n"
                     "@hide" )
{
   SFXDescription* description = NULL;
   SFXTrack* track = dynamic_cast< SFXTrack* >( Sim::findObject( argv[1] ) );
   if ( !track )
   {
      description = dynamic_cast< SFXDescription* >( Sim::findObject( argv[1] ) );
      if ( !description )
      {
         Con::printf( "Unable to locate sound track/description '%s'", argv[1] );
         return 0;
      }
   }

   SFXSource* source = NULL;

   if ( track )
   {
      if ( argc == 2 )
      {
         source = SFX->createSource( track );
      }
      else
      {
         MatrixF transform;
         transform.set( EulerF(0,0,0), Point3F( dAtof(argv[2]), dAtof(argv[3]), dAtof(argv[4])) );
         source = SFX->createSource( track, &transform );
      }
   }
   else if ( description )
   {
      SFXProfile* tempProfile = new SFXProfile( description, StringTable->insert( argv[2] ), true );
      if( !tempProfile->registerObject() )
      {
         Con::errorf( "sfxCreateSource - unable to create profile" );
         delete tempProfile;
      }
      else
      {
         if ( argc == 3 )
         {
            source = SFX->createSource( tempProfile );
         }
         else
         {
            MatrixF transform;
            transform.set(EulerF(0,0,0), Point3F( dAtof(argv[3]),dAtof(argv[4]),dAtof(argv[5]) ));
            source = SFX->createSource( tempProfile, &transform );
         }

         tempProfile->setAutoDelete( true );
      }
   }

   if ( source )
      return source->getId();

   return 0;
}

//-----------------------------------------------------------------------------

static ConsoleDocFragment _sfxPlay1(
   "@brief Start playback of the given source.\n\n"
   "This is the same as calling SFXSource::play() directly.\n\n"
   "@param source The source to start playing.\n\n"
   "@return @a source.\n\n"
   "@tsexample\n"
   "// Create and play a source from a pre-existing profile:\n"
   "%source = sfxCreateSource( SoundFileProfile );\n"
   "%source.play();\n"
   "@endtsexample\n\n"
   "@ingroup SFX",
   NULL,
   "SFXSource sfxPlay( SFXSource source );" );
static ConsoleDocFragment _sfxPlay2(
   "@brief Create a new play-once source for the given @a track and start playback of the source.\n\n"
   "This is equivalent to calling sfxCreateSource() on @track and SFXSource::play() on the resulting source.\n\n"
   "@param track The sound datablock to play.\n\n"
   "@return The newly created play-once source or 0 if the creation failed.\n\n"
   "@ref SFXSource_playonce\n\n"
   "@ingroup SFX",
   NULL,
   "void sfxPlay( SFXTrack track );" );
static ConsoleDocFragment _sfxPlay3(
   "@brief Create a new play-once source for the given @a track, position its 3D sound at the given coordinates (if the track's description "
   "is set up for 3D sound) and start playback of the source.\n\n"
   "This is equivalent to calling sfxCreateSource() on @track and SFXSource::play() on the resulting source.\n\n"
   "@param track The sound datablock to play.\n\n"
   "@param x The X coordinate of the 3D sound position.\n"
   "@param y The Y coordinate of the 3D sound position.\n"
   "@param z The Z coordinate of the 3D sound position.\n"
   "@return The newly created play-once source or 0 if the creation failed.\n\n"
   "@ref SFXSource_playonce\n\n"
   "@ingroup SFX",
   NULL,
   "void sfxPlay( SFXTrack track, float x, float y, float z );" );
   
ConsoleFunction( sfxPlay, S32, 2, 5, "( SFXSource source | ( SFXTrack track [, float x, float y, float z ] ) ) "
   "Start playing the given source or create a new source for the given track and play it.\n"
   "@hide" )
{
   if ( argc == 2 )
   {
      SFXSource* source = dynamic_cast<SFXSource*>( Sim::findObject( argv[1] ) );
      if ( source )
      {
         source->play();
         return source->getId();
      }
   }

   SFXTrack* track = dynamic_cast<SFXTrack*>( Sim::findObject( argv[1] ) );
   if ( !track )
   {
      Con::printf( "Unable to locate sfx track '%s'", argv[1] );
      return 0;
   }

   Point3F pos(0.f, 0.f, 0.f);
   if ( argc == 3 )
      dSscanf( argv[2], "%g %g %g", &pos.x, &pos.y, &pos.z );
   else if(argc == 5)
      pos.set( dAtof(argv[2]), dAtof(argv[3]), dAtof(argv[4]) );

   MatrixF transform;
   transform.set( EulerF(0,0,0), pos );

   SFXSource* source = SFX->playOnce( track, &transform );
   if ( source )
      return source->getId();

   return 0;
}

//-----------------------------------------------------------------------------

static ConsoleDocFragment _sPlayOnce1(
   "@brief Create a play-once source for the given @a track.\n\n"
   "Once playback has finished, the source will be automatically deleted in the next sound system update.\n"
   "@param track The sound datablock.\n"
   "@return A newly created temporary source in \"Playing\" state or 0 if the operation failed.\n\n"
   "@ref SFXSource_playonce\n\n"
   "@ingroup SFX",
   NULL,
   "SFXSource sfxPlayOnce( SFXTrack track );"
);
static ConsoleDocFragment _sPlayOnce2(
   "@brief Create a play-once source for the given given @a track and position the source's 3D sound at the given coordinates "
      "only if the track's description is set up for 3D sound).\n\n"
   "Once playback has finished, the source will be automatically deleted in the next sound system update.\n"
   "@param track The sound datablock.\n"
   "@param x The X coordinate of the 3D sound position.\n"
   "@param y The Y coordinate of the 3D sound position.\n"
   "@param z The Z coordinate of the 3D sound position.\n"
   "@param fadeInTime If >=0, this overrides the SFXDescription::fadeInTime value on the track's description.\n"
   "@return A newly created temporary source in \"Playing\" state or 0 if the operation failed.\n\n"
   "@tsexample\n"
      "// Immediately start playing the given track.  Fade it in to full volume over 5 seconds.\n"
      "sfxPlayOnce( MusicTrack, 0, 0, 0, 5.f );\n"
   "@endtsexample\n\n"
   "@ref SFXSource_playonce\n\n"
   "@ingroup SFX",
   NULL,
   "SFXSource sfxPlayOnce( SFXTrack track, float x, float y, float z, float fadeInTime=-1 );"
);
static ConsoleDocFragment _sPlayOnce3(
   "@brief Create a new temporary SFXProfile from the given @a description and @a filename, then create a play-once source "
      "for it and start playback.\n\n"
   "Once playback has finished, the source will be automatically deleted in the next sound system update.  If not referenced "
      "otherwise by then, the temporary SFXProfile will also be deleted.\n"
   "@param description The description to use for playback.\n"
   "@param filename Path to the sound file to play.\n"
   "@return A newly created temporary source in \"Playing\" state or 0 if the operation failed.\n\n"
   "@tsexample\n"
   "// Play a sound effect file once.\n"
   "sfxPlayOnce( AudioEffects, \"art/sound/weapons/Weapon_pickup\" );\n"
   "@endtsexample\n\n"
   "@ref SFXSource_playonce\n\n"
   "@ingroup SFX",
   NULL,
   "SFXSource sfxPlayOnce( SFXDescription description, string filename );"
);
static ConsoleDocFragment _sPlayOnce4(
   "@brief Create a new temporary SFXProfile from the given @a description and @a filename, then create a play-once source "
      "for it and start playback.  Position the source's 3D sound at the given coordinates (only if the description "
      "is set up for 3D sound).\n\n"
   "Once playback has finished, the source will be automatically deleted in the next sound system update.  If not referenced "
      "otherwise by then, the temporary SFXProfile will also be deleted.\n"
   "@param description The description to use for playback.\n"
   "@param filename Path to the sound file to play.\n"
   "@param x The X coordinate of the 3D sound position.\n"
   "@param y The Y coordinate of the 3D sound position.\n"
   "@param z The Z coordinate of the 3D sound position.\n"
   "@param fadeInTime If >=0, this overrides the SFXDescription::fadeInTime value on the track's description.\n"
   "@return A newly created temporary source in \"Playing\" state or 0 if the operation failed.\n\n"
   "@tsexample\n"
   "// Play a sound effect file once using a 3D sound with a default falloff placed at the origin.\n"
   "sfxPlayOnce( AudioDefault3D, \"art/sound/weapons/Weapon_pickup\", 0, 0, 0 );\n"
   "@endtsexample\n\n"
   "@ref SFXSource_playonce\n\n"
   "@ingroup SFX",
   NULL,
   "SFXSource sfxPlayOnce( SFXDescription description, string filename, float x, float y, float z, float fadeInTime=-1 );"
);

ConsoleFunction( sfxPlayOnce, S32, 2, 6,
   "SFXSource sfxPlayOnce( ( SFXTrack track | SFXDescription description, string filename ) [, float x, float y, float z, float fadeInTime=-1 ] ) "
   "Create a new play-once source for the given profile or description+filename and start playback of the source.\n"
   "@hide" )
{
   SFXDescription* description = NULL;
   SFXTrack* track = dynamic_cast< SFXTrack* >( Sim::findObject( argv[1] ) );
   if( !track )
   {
      description = dynamic_cast< SFXDescription* >( Sim::findObject( argv[1] ) );
      if( !description )
      {
         Con::errorf( "sfxPlayOnce - Unable to locate sound track/description '%s'", argv[1] );
         return 0;
      }
   }

   SFXSource* source = NULL;
   if( track )
   {
      if( argc == 2 )
         source = SFX->playOnce( track );
      else
      {
         MatrixF transform;
         transform.set( EulerF( 0, 0, 0 ), Point3F( dAtof( argv[ 2 ] ), dAtof( argv[ 3 ] ),dAtof( argv[ 4 ] ) ) );
         F32 fadeInTime = -1.f;
         if( argc > 5 )
            fadeInTime = dAtof( argv[ 5 ] );
         source = SFX->playOnce( track, &transform, NULL, fadeInTime );
      }
   }
   else if( description )
   {
      SFXProfile* tempProfile = new SFXProfile( description, StringTable->insert( argv[2] ), true );
      if( !tempProfile->registerObject() )
      {
         Con::errorf( "sfxPlayOnce - unable to create profile" );
         delete tempProfile;
      }
      else
      {
         if ( argc == 3 )
            source = SFX->playOnce( tempProfile );
         else
         {
            MatrixF transform;
            transform.set(EulerF(0,0,0), Point3F( dAtof(argv[3]),dAtof(argv[4]),dAtof(argv[5]) ));
            F32 fadeInTime = -1.f;
            if( argc > 6 )
               fadeInTime = dAtof( argv[ 6 ] );
            source = SFX->playOnce( tempProfile, &transform, NULL, fadeInTime );
         }
         
         // Set profile to auto-delete when SFXSource releases its reference.
         // Also add to root group so the profile will get deleted when the
         // Sim system is shut down before the SFXSource has played out.

         tempProfile->setAutoDelete( true );
         Sim::getRootGroup()->addObject( tempProfile );
      }
   }

   if( !source )
      return 0;
      
   return source->getId();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxStop, void, ( SFXSource* source ),,
   "Stop playback of the given @a source.\n"
   "This is equivalent to calling SFXSource::stop().\n\n"
   "@param source The source to put into stopped state.\n\n"
   "@ingroup SFX" )
{
   if( source )
      source->stop();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxStopAndDelete, void, ( SFXSource* source ),,
   "Stop playback of the given @a source (if it is not already stopped) and delete the @a source.\n\n"
   "The advantage of this function over directly calling delete() is that it will correctly "
   "handle volume fades that may be configured on the source.  Whereas calling delete() would immediately "
   "stop playback and delete the source, this functionality will wait for the fade-out to play and only then "
   "stop the source and delete it.\n\n"
   "@param source A sound source.\n\n"
   "@ref SFXSource_fades\n\n"
   "@ingroup SFX" )
{
   if( source )
      SFX->stopAndDeleteSource( source );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxDeleteWhenStopped, void, ( SFXSource* source ),,
   "Mark the given @a source for deletion as soon as it moves into stopped state.\n\n"
   "This function will retroactively turn the given @a source into a play-once source (see @ref SFXSource_playonce).\n\n"
   "@param source A sound source.\n\n"
   "@ingroup SFX" )
{
   if( source )
      SFX->deleteWhenStopped( source );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxGetDistanceModel, SFXDistanceModel, (),,
   "Get the falloff curve type currently being applied to 3D sounds.\n\n"
   "@return The current distance model type.\n\n"
   "@ref SFXSource_volume\n\n"
   "@ref SFX_3d\n\n"
   "@ingroup SFX" )
{
   return SFX->getDistanceModel();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxSetDistanceModel, void, ( SFXDistanceModel model ),,
   "Set the falloff curve type to use for distance-based volume attenuation of 3D sounds.\n\n"
   "@param model The distance model to use for 3D sound.\n\n"
   "@note This setting takes effect globally and is applied to all 3D sounds.\n\n"
   "@ingroup SFX" )
{
   SFX->setDistanceModel( model );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxGetDopplerFactor, F32, (),,
   "Get the current global doppler effect setting.\n\n"
   "@return The current global doppler effect scale factor (>=0).\n\n"
   "@see sfxSetDopplerFactor\n\n"
   "@ref SFXSource_doppler\n\n"
   "@ingroup SFX" )
{
   return SFX->getDopplerFactor();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxSetDopplerFactor, void, ( F32 value ),,
   "Set the global doppler effect scale factor.\n"
   "@param value The new doppler shift scale factor.\n"
   "@pre @a value must be >= 0.\n"
   "@see sfxGetDopplerFactor\n\n"
   "@ref SFXSource_doppler\n\n"
   "@ingroup SFX" )
{
   if( value < 0.0f )
   {
      Con::errorf( "sfxSetDopplerFactor - factor must be >0" );
      return;
   }

   SFX->setDopplerFactor( value );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxGetRolloffFactor, F32, (),,
   "Get the current global scale factor applied to volume attenuation of 3D sounds in the logarithmic model.\n"
   "@return The current scale factor for logarithmic 3D sound falloff curves.\n\n"
   "@see sfxGetDistanceModel\n"
   "@see SFXDistanceModel\n\n"
   "@ref SFXSource_volume\n"
   "@ref SFX_3d\n"
   "@ingroup SFX" )
{
   return SFX->getRolloffFactor();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxSetRolloffFactor, void, ( F32 value ),,
   "Set the global scale factor to apply to volume attenuation of 3D sounds in the logarithmic model.\n"
   "@param value The new scale factor for logarithmic 3D sound falloff curves.\n\n"
   "@pre @a value must be > 0.\n"
   "@note This function has no effect if the currently distance model is set to SFXDistanceModel::Linear.\n\n"
   "@see sfxGetDistanceModel\n"
   "@see SFXDistanceModel\n\n"
   "@ref SFXSource_volume\n"
   "@ref SFX_3d\n"
   "@ingroup SFX" )
{
   if( value <= 0.0f )
   {
      Con::errorf( "sfxSetRolloffFactor - factor must be >0" );
      return;
   }

   SFX->setRolloffFactor( value );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxDumpSources, void, ( bool includeGroups ), ( false ),
   "Dump information about all current SFXSource instances to the console.\n"
   "The dump includes information about the playback status for each source, volume levels, virtualization, etc.\n"
   "@param includeGroups If true, direct instances of SFXSources (which represent logical sound groups) will be included. "
      "Otherwise only instances of subclasses of SFXSources are included in the dump.\n"
   "@see SFXSource\n"
   "@see sfxDumpSourcesToString\n"
   "@ingroup SFX" )
{
   SFX->dumpSources( NULL, !includeGroups );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxDumpSourcesToString, const char*, ( bool includeGroups ), ( false ),
   "Dump information about all current SFXSource instances to a string.\n"
   "The dump includes information about the playback status for each source, volume levels, virtualization, etc.\n"
   "@param includeGroups If true, direct instances of SFXSources (which represent logical sound groups) will be included. "
      "Otherwise only instances of subclasses of SFXSources are included in the dump.\n"
   "@return A string containing a dump of information about all currently instantiated SFXSources.\n"
   "@see SFXSource\n"
   "@see sfxDumpSources\n"
   "@ingroup SFX" )
{
   StringBuilder str;
   SFX->dumpSources( &str, !includeGroups );
   
   return Con::getReturnBuffer( str );
}
