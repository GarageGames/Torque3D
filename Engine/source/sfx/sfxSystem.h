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

#ifndef _SFXSYSTEM_H_
#define _SFXSYSTEM_H_

#ifndef _SFXCOMMON_H_
   #include "sfx/sfxCommon.h"
#endif
#ifndef _TSIGNAL_H_
   #include "core/util/tSignal.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif
#ifndef _THREADSAFEREFCOUNT_H_
   #include "platform/threads/threadSafeRefCount.h"
#endif


class SFXTrack;
class SFXDevice;
class SFXProfile;
class SFXStream;
class SFXAmbience;
class SFXSoundscapeManager;
class SFXSource;
class SFXSound;
class SFXBuffer;
class SFXDescription;


/// SFX system events that can be received notifications on.
enum SFXSystemEventType
{
   /// SFX is being updated.
   SFXSystemEvent_Update,
   
   /// New SFXDevice has been created.
   SFXSystemEvent_CreateDevice,
   
   /// SFXDevice is about to be destroyed.
   SFXSystemEvent_DestroyDevice,
};

/// SFXSystemPlugins are used to allow other subsystems hook into core functionality
/// of the sound system.
class SFXSystemPlugin
{
   public:
   
      ///
      virtual void update() {}
   
      ///
      virtual SFXSource* createSource( SFXTrack* track ) { return NULL; }
      
      /// Filter the given reverb setup before it is set up on the device.  This
      /// allows to, for example, modify the current reverb depending on listener
      /// location.
      virtual void filterReverb( SFXReverbProperties& reverb ) {}
};


/// This class provides access to the sound system.
///
/// There are a few script preferences that are used by
/// the sound providers.
///
///   $pref::SFX::frequency - This is the playback frequency
///   for the primary sound buffer used for mixing.  Although
///   most providers will reformat on the fly, for best quality
///   and performance match your sound files to this setting.
///
///   $pref::SFX::bitrate - This is the playback bitrate for
///   the primary sound buffer used for mixing.  Although most
///   providers will reformat on the fly, for best quality
///   and performance match your sound files to this setting.
///
class SFXSystem
{
      friend class SFXSound;           // _assignVoices
      friend class SFXSource;          // _onAddSource, _onRemoveSource.
      friend class SFXProfile;         // _createBuffer.

   public:
   
      typedef Signal< void( SFXSystemEventType event ) > EventSignalType;
      typedef Vector< SFXSource* > SFXSourceVector;
      typedef Vector< SFXSound* > SFXSoundVector;
      
   protected:

      /// The one and only instance of the SFXSystem.
      static SFXSystem* smSingleton;

      /// The protected constructor.
      ///
      /// @see SFXSystem::init()
      ///
      SFXSystem();

      /// The non-virtual destructor.  You shouldn't
      /// ever need to overload this class.
      ~SFXSystem();

      /// The current output sound device initialized
      /// and ready to play back.
      SFXDevice* mDevice;
      
      ///
      SFXSoundVector mSounds;

      /// This is used to keep track of play once sources
      /// that must be released when they stop playing.
      SFXSourceVector mPlayOnceSources;
            
      /// The last time the sources got an update.
      U32 mLastSourceUpdateTime;
      
      ///
      U32 mLastAmbientUpdateTime;
      
      ///
      U32 mLastParameterUpdateTime;

      /// The distance model used for rolloff curve computation on 3D sounds.
      SFXDistanceModel mDistanceModel;
      
      /// The current doppler scale factor.
      F32 mDopplerFactor;
      
      /// The current curve rolloff factor.
      F32 mRolloffFactor;
            
      /// The current position and orientation of all listeners.
      Vector< SFXListenerProperties > mListeners;
      
      /// Current global reverb properties.
      SFXReverbProperties mReverb;
      
      /// SFX system event signal.
      EventSignalType mEventSignal;
            
      /// Ambient soundscape manager.
      SFXSoundscapeManager* mSoundscapeMgr;
      
      /// List of plugins currently linked to the SFX system.
      Vector< SFXSystemPlugin* > mPlugins;

      /// @name Stats
      ///
      /// Stats reported back to the console for tracking performance.
      ///
      /// @{

      S32 mStatNumSources;
      S32 mStatNumSounds;
      S32 mStatNumPlaying;
      S32 mStatNumCulled;
      S32 mStatNumVoices;
      S32 mStatSourceUpdateTime;
      S32 mStatParameterUpdateTime;
      S32 mStatAmbientUpdateTime;

      /// @}

      /// Called to reprioritize and reassign buffers as
      /// sources change state, volumes are adjusted, and 
      /// the listener moves around.
      ///
      /// @see SFXSource::_update()
      ///
      void _updateSources();

      /// This called to reprioritize and reassign
      /// voices to sources.
      void _assignVoices();
      
      ///
      void _assignVoice( SFXSound* sound );

      ///
      void _sortSounds( const SFXListenerProperties& listener );

      /// Called from SFXSource::onAdd to register the source.
      void _onAddSource( SFXSource* source );
                       
      /// Called from SFXSource::onRemove to unregister the source.
      void _onRemoveSource( SFXSource* source );

      /// Called from SFXProfile to create a device specific
      /// sound buffer used in conjunction with a voice in playback.
      SFXBuffer* _createBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description );
      
      /// Load file directly through SFXDevice.  Depends on
      /// availability with selected SFXDevice.
      ///
      /// @return Return new buffer or NULL.
      SFXBuffer* _createBuffer( const String& filename, SFXDescription* description );
      
      ///
      SFXDevice* _getDevice() const { return mDevice; }

   public:

      /// Returns the one an only instance of the SFXSystem 
      /// unless it hasn't been initialized or its been disabled
      /// in your build.
      ///
      /// For convienence you can use the SFX-> macro as well.
      ///
      /// @see SFXSystem::init()
      /// @see SFX
      static SFXSystem* getSingleton() { return smSingleton; }

      /// This is called from initialization to prepare the
      /// sound system singleton.  This also includes registering
      /// common resource types and initializing available sound
      /// providers.
      static void init();

      /// This is called after Sim::shutdown() in shutdownLibraries()
      /// to free the sound system singlton.  After this the SFX
      /// singleton is null and any call to it will crash.
      static void destroy();

      /// This is only public so that it can be called by 
      /// the game update loop.  It updates the current 
      /// device and all sources.
      void _update();
      
      /// Register the given plugin with the system.
      void addPlugin( SFXSystemPlugin* plugin );
      
      /// Unregister the given plugin with the system.
      void removePlugin( SFXSystemPlugin* plugin );
      
      /// @name Device Management
      /// @{

      /// This initializes a new device.
      ///
      /// @param providerName    The name of the provider.
      /// @param deviceName      The name of the provider device.
      /// @param useHardware     Toggles the use of hardware processing when available.
      /// @param maxBuffers      The maximum buffers for this device to use or -1 
      ///                        for the device to pick its own reasonable default.
      /// @param changeDevice    Allows this to change the current device to a new one
      /// @return Returns true if the device was created.
      bool createDevice(   const String& providerName, 
                           const String& deviceName, 
                           bool useHardware,
                           S32 maxBuffers,
                           bool changeDevice = false);

      /// Returns the current device information or NULL if no
      /// device is present.  The information string is in the
      /// following format:
      /// 
      /// Provider Name\tDevice Name\tUse Hardware\tMax Buffers
      String getDeviceInfoString();

      /// This destroys the current device.  All sources loose their
      /// playback buffers, but otherwise continue to function.
      void deleteDevice();

      /// Returns true if a device is allocated.
      bool hasDevice() const { return mDevice != NULL; }
      
      /// @}
      
      /// @name Source Creation
      /// @{

      /// Used to create new sound sources from a sound profile.  The
      /// returned source is in a stopped state and ready for playback.
      /// Use the SFX_DELETE macro to free the source when your done.
      ///
      /// @note The track must have at least the same lifetime as the
      ///   source.  If the description disappears while the source is still
      ///   there, the source will go with it.
      ///
      /// @param profile   The sound profile for the created source.
      /// @param transform The optional transform if creating a 3D source.
      /// @param velocity  The optional doppler velocity if creating a 3D source.
      ///
      /// @return The sound source or NULL if an error occured.
      SFXSource* createSource(  SFXTrack* track, 
                                const MatrixF* transform = NULL, 
                                const VectorF* velocity = NULL );

      /// Used to create a streaming sound source from a user supplied
      /// stream object.  
      ///
      /// It is only intended for memory based streams.  For sound file
      /// streaming use createSource() with a streaming SFXProfile.
      ///
      /// Use the SFX_DELETE macro to free the source when your done.
      ///
      /// @note The description must have at least the same lifetime as the
      ///   sound.  If the description disappears while the source is still
      ///   there, the sound will go with it.
      ///
      /// @param stream    The stream used to create the sound buffer.  It
      ///                  must exist for the lifetime of the source and will
      ///                  have its reference count decremented when the source
      ///                  is destroyed.
      ///
      /// @param description  The sound description to apply to the source.
      ///
      /// @return The sound source or NULL if an error occured.
      SFXSound* createSourceFromStream(   const ThreadSafeRef< SFXStream >& stream,
                                          SFXDescription* description );

      /// Creates a source which when it finishes playing will auto delete
      /// itself.  Be aware that the returned SFXSource pointer should only
      /// be used for error checking or immediate setting changes.  It may
      /// be deleted as soon as the next system tick.
      ///
      /// @param profile   The sound profile for the created source.
      /// @param transform The optional transform if creating a 3D source.
      /// @param velocity  The optional doppler velocity if creating a 3D source.
      ///
      /// @return The sound source or NULL if an error occured.
      SFXSource* playOnce( SFXTrack* track, 
                           const MatrixF* transform = NULL,
                           const VectorF* velocity = NULL,
                           F32 fadeInTime = -1.f );
      SFXSource* playOnce( SFXProfile* profile,
                           const MatrixF* transform = NULL,
                           const VectorF* velocity = NULL,
                           F32 fadeInTime = -1.f )
      { // Avoids having to require inclusion of sfxProfile.h
         return playOnce( ( SFXTrack* ) profile, transform, velocity, fadeInTime );
      }
      
      /// Stop the source and delete it.  This method will take care of
      /// the fade-out time that the source may need before it will actually
      /// stop and may be deleted.
      void stopAndDeleteSource( SFXSource* source );
      
      /// Mark source for deletion when it is moving into stopped state.
      /// This method is useful to basically make a source a play-once source
      /// after the fact.
      void deleteWhenStopped( SFXSource* source );
      
      /// @}
            
      /// @}
      
      /// @name Listeners
      /// @{

      /// Return the number of listeners currently configured.
      U32 getNumListeners() const { return mListeners.size(); }
      
      /// Set the number of concurrent listeners.
      /// @note It depends on the selected device if more than one listener is actually supported.
      void setNumListeners( U32 num );

      /// Set the property of the given listener.
      const SFXListenerProperties& getListener( U32 index = 0 ) const { return mListeners[ index ]; }
      
      /// Set the 3D attributes of the given listener.
      void setListener( U32 index, const MatrixF& transform, const Point3F& velocity );
      void setListener( U32 index, const SFXListenerProperties& properties )
      {
         setListener( index, properties.getTransform(), properties.getVelocity() );
      }
      
      /// @}
      
      /// @name 3D Sound Configuration
      /// {
      
      /// Return the curve model currently used distance attenuation of positional sounds.
      SFXDistanceModel getDistanceModel() const { return mDistanceModel; }
      
      ///
      void setDistanceModel( SFXDistanceModel model );
      
      ///
      F32 getDopplerFactor() const { return mDopplerFactor; }
      
      ///
      void setDopplerFactor( F32 factor );
      
      ///
      F32 getRolloffFactor() const { return mRolloffFactor; }
      
      ///
      void setRolloffFactor( F32 factor );
      
      ///
      const SFXReverbProperties& getReverb() const { return mReverb; }
      
      ///
      void setReverb( const SFXReverbProperties& reverb );
      
      /// @}
      
      ///
      SFXSoundscapeManager* getSoundscapeManager() const { return mSoundscapeMgr; }
      
      /// Dump information about all current SFXSources to the console or
      /// to the given StringBuilder.
      void dumpSources( StringBuilder* toString = NULL, bool excludeGroups = true );
      
      /// Return the SFX system event signal.
      EventSignalType& getEventSignal() { return mEventSignal; }
      
      /// Notify the SFX system that the given description has changed.
      /// All sources currently using the description will be updated.
      void notifyDescriptionChanged( SFXDescription* description);
      
      ///
      void notifyTrackChanged( SFXTrack* track );
};


/// Less verbose macro for accessing the SFX singleton.  This
/// should be the prefered method for accessing the system.
///
/// @see SFXSystem
/// @see SFXSystem::getSingleton()
///
#define SFX SFXSystem::getSingleton()


#endif // _SFXSYSTEM_H_
