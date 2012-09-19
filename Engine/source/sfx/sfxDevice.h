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

#ifndef _SFXDEVICE_H_
#define _SFXDEVICE_H_

#ifndef _PLATFORM_H_
   #include "platform/platform.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif
#ifndef _SFXCOMMON_H_
   #include "sfx/sfxCommon.h"
#endif
#ifndef _THREADSAFEREF_H_
   #include "platform/threads/threadSafeRefCount.h"
#endif


class SFXProvider;
class SFXListener;
class SFXBuffer;
class SFXVoice;
class SFXProfile;
class SFXDevice;
class SFXStream;
class SFXDescription;



/// Abstract base class for back-end sound API implementations.
class SFXDevice
{
   public:
   
      typedef void Parent;
      
      /// Device capability flags.
      enum ECaps
      {
         CAPS_Reverb          = BIT( 0 ),    ///< Device supports reverb environments.
         CAPS_VoiceManagement = BIT( 1 ),    ///< Device manages voices on its own; deactivates virtualization code in SFX system.
         CAPS_Occlusion       = BIT( 2 ),    ///< Device has its own sound occlusion handling (SFXOcclusionManager).
         CAPS_DSPEffects      = BIT( 3 ),    ///< Device implements DSP effects (SFXDSPManager).
         CAPS_MultiListener   = BIT( 4 ),    ///< Device supports multiple listeners.
         CAPS_FMODDesigner    = BIT( 5 ),    ///< FMOD Designer support.
      };
      
   protected:

      typedef Vector< SFXBuffer* > BufferVector;
      typedef Vector< SFXVoice* > VoiceVector;
      
      typedef BufferVector::iterator BufferIterator;
      typedef VoiceVector::iterator VoiceIterator;

      SFXDevice( const String& name, SFXProvider* provider, bool useHardware, S32 maxBuffers );

      /// The name of this device.
      String mName;

      /// The provider which created this device.
      SFXProvider* mProvider;

      /// Should the device try to use hardware processing.
      bool mUseHardware;

      /// The maximum playback buffers this device will use.
      S32 mMaxBuffers;

      /// Current set of sound buffers.
      BufferVector mBuffers;

      /// Current set of voices.
      VoiceVector mVoices;

      /// Device capabilities.
      U32 mCaps;

      /// Current number of buffers.  Reflected in $SFX::Device::numBuffers.
      U32 mStatNumBuffers;

      /// Current number of voices.  Reflected in $SFX::Device::numVoices.
      U32 mStatNumVoices;

      /// Current total memory size of sound buffers.  Reflected in $SFX::Device::numBufferBytes.
      U32 mStatNumBufferBytes;
      
      /// Register a buffer with the device.
      /// This also triggers the buffer's stream packet request chain.
      void _addBuffer( SFXBuffer* buffer );

      /// Unregister the given buffer.
      void _removeBuffer( SFXBuffer* buffer );

      /// Register a voice with the device.
      void _addVoice( SFXVoice* voice );

      /// Unregister the given voice.
      virtual void _removeVoice( SFXVoice* buffer );

      /// Release all resources tied to the device.  Can be called repeatedly
      /// without harm.  It is meant for device destructors that will severe
      /// the connection to the sound API and thus need all resources freed
      /// before the base destructor is called.
      void _releaseAllResources();

public:

      virtual ~SFXDevice();

      /// Returns the provider which created this device.
      SFXProvider* getProvider() const { return mProvider; }

      /// Is the device set to use hardware processing.
      bool getUseHardware() const { return mUseHardware; }

      /// The maximum number of playback buffers this device will use.
      S32 getMaxBuffers() const { return mMaxBuffers; }

      /// Returns the name of this device.
      const String& getName() const { return mName; }

      /// Return the device capability flags.
      U32 getCaps() const { return mCaps; }

      /// Tries to create a new sound buffer.  If creation fails
      /// freeing another buffer will usually allow a new one to 
      /// be created.
      ///
      /// @param stream          The sound data stream.
      /// @param description     The playback configuration.
      ///
      /// @return Returns a new buffer or NULL if one cannot be created.
      ///
      virtual SFXBuffer* createBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description ) = 0;

      /// Create a sound buffer directly for a file.  This is for
      /// devices that implemented their own custom file loading.
      ///
      /// @note Only implemented on specific SFXDevices.
      /// @return Return a new buffer or NULL.
      virtual SFXBuffer* createBuffer( const String& fileName, SFXDescription* description ) { return NULL; }

      /// Tries to create a new voice.
      ///
      /// @param is3d True if the voice should have 3D sound enabled.
      /// @param buffer The sound data to play by the voice.
      ///
      /// @return Returns a new voice or NULL if one cannot be created.
      virtual SFXVoice* createVoice( bool is3D, SFXBuffer* buffer ) = 0;
      
      /// Set the rolloff curve to be used by distance attenuation of 3D sounds.
      virtual void setDistanceModel( SFXDistanceModel model ) {}
      
      /// Set the scale factor to use for doppler effects on 3D sounds.
      virtual void setDopplerFactor( F32 factor ) {}
      
      /// Set the rolloff scale factor for distance attenuation of 3D sounds.
      virtual void setRolloffFactor( F32 factor ) {}
      
      /// Set the global reverb environment.
      virtual void setReverb( const SFXReverbProperties& reverb ) {}
      
      /// Reset the global reverb environment to its default.
      virtual void resetReverb() {}

      /// Set the number of concurrent listeners on the device.
      ///
      /// @note On devices that do not support multiple listeners, any value
      ///   other than 1 will be ignored.
      virtual void setNumListeners( U32 num ) {}
      
      /// Set the properties of the given listener.
      ///
      /// @note On devices that do not support multiple listeners, only setting
      ///   the properties on index=0 will have a effect.
      virtual void setListener( U32 index, const SFXListenerProperties& listener ) {}
            
      /// Return the current total number of sound buffers.
      U32 getBufferCount() const { return mBuffers.size(); }

      /// Return the current total number of voices.
      U32 getVoiceCount() const { return mVoices.size(); }

      /// Called from SFXSystem to do any updates the device may need to make.
      virtual void update();
};


#endif // _SFXDEVICE_H_
