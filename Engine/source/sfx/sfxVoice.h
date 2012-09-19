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

#ifndef _SFXVOICE_H_
#define _SFXVOICE_H_

#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif

#ifndef _TSTREAM_H_
#include "core/stream/tStream.h"
#endif

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif

#ifndef _SFXBUFFER_H_
#include "sfx/sfxBuffer.h"
#endif


namespace SFXInternal {
   class SFXVoiceTimeSource;
   class SFXAynscQueue;
}


/// The voice interface provides for playback of sound buffers and positioning
/// of 3D sounds.
///
/// This abstract class is derived from in the different device layers to implement
/// device-specific playback control.
///
/// The primary responsibility of this class is to mediate between the user requests
/// (play(), stop(), pause(), setPosition()), the buffer (which may change state
/// asynchronously), and the underlying device playback control (_play(), _stop(),
/// _pause(), _seek()).
class SFXVoice :  public StrongRefBase,
                  public IPositionable< U32 >
{
   public:

      typedef void Parent;

      friend class SFXDevice; // _attachToBuffer
      friend class SFXInternal::SFXVoiceTimeSource; // _tell
      friend class SFXInternal::SFXAsyncQueue; // mOffset

   protected:

      /// Current playback status.
      /// @note This is maintained on both the sound update thread as well
      ///   as the main thread.
      mutable volatile SFXStatus mStatus;

      /// Sound data played back by the voice.
      WeakRefPtr< SFXBuffer > mBuffer;

      /// For streaming voices, this keeps track of play start offset
      /// after seeking.  Expressed in number of samples.
      U32 mOffset;

      explicit SFXVoice( SFXBuffer* buffer );

      /// @name Device Control Methods
      /// @{

      /// Return the current playback status (playing, paused, or stopped).  Default
      /// status is stopped.
      virtual SFXStatus _status() const = 0;

      /// Stop playback on the device.
      /// @note Called from both the SFX update thread and the main thread.
      virtual void _stop() = 0;

      /// Start playback on the device.
      /// @note Called from both the SFX update thread and the main thread.
      virtual void _play() = 0;

      /// Pause playback on the device.
      /// @note Called from both the SFX update thread and the main thread.
      virtual void _pause() = 0;

      /// Set the playback cursor on the device.
      /// @note Only used for non-streaming voices.
      virtual void _seek( U32 sample ) = 0;

      /// Get the playback cursor on the device.
      ///
      /// When the voice is playing or paused, this method must return a valid sample position.
      /// When the voice is stopped, the result of this method is undefined.
      ///
      /// For streaming voices that are looping, the sample position must be a total count of the
      /// number of samples played so far which thus includes the count of all cycles before the
      /// current one.  For non-looping voices, this behavior is optional.
      ///
      /// @note This is called for both streaming and non-streaming voices.
      virtual U32 _tell() const = 0;

      /// @}

      /// Hooked up to SFXBuffer::mOnStatusChange of #mBuffer.
      /// @note Called on the SFX update thread.
      virtual void _onBufferStatusChange( SFXBuffer* buffer, SFXBuffer::Status newStatus );

      ///
      void _attachToBuffer();

      /// @name Streaming
      /// The following methods are for streaming voices only.
      /// @{

      /// Reset streaming of the voice by cloning the current streaming source and
      /// letting the resulting stream start from @a sampleStartPos.
      void _resetStream( U32 sampleStartPos, bool triggerUpdate = true );

      /// @}

   public:

      static Signal< void( SFXVoice* ) > smVoiceCreatedSignal;
      static Signal< void( SFXVoice* ) > smVoiceDestroyedSignal;

      /// The destructor.
      virtual ~SFXVoice();

      ///
      const SFXFormat& getFormat() const { return mBuffer->getFormat(); }

      /// Return the current playback position (in number of samples).
      ///
      /// @note For looping sounds, this will return the position in the
      ///   current cycle and not the total number of samples played so far.
      virtual U32 getPosition() const;

      /// Sets the playback position to the given sample count.
      ///
      /// @param sample Offset in number of samples.  This is allowed to use an offset
      ///   accumulated from multiple cycles.  Each cycle will wrap around back to the
      ///   beginning of the buffer.
      virtual void setPosition( U32 sample );

      /// @return the current playback status.
      /// @note For streaming voices, the reaction to for the voice to update its status
      ///   to SFXStatusStopped after the voice has stopped playing depends on the synchronization
      ///   of the underlying device.  If, for example, the underlying device uses periodic updates
      ///   and doesn't have notifications, then a delay up to the total length of the period time
      ///   may occur before the status changes.  Note that in-between the actual playback stopping
      ///   and the voice updating its status, the result of getPosition() is undefined.
      virtual SFXStatus getStatus() const;

      /// Starts playback from the current position.
      virtual void play( bool looping );

      /// Stops playback and moves the position to the start.
      virtual void stop();

      /// Pauses playback.
      virtual void pause();

      /// Sets the position and orientation for a 3d voice.
      virtual void setTransform( const MatrixF &transform ) = 0;

      /// Sets the velocity for a 3d voice.
      virtual void setVelocity( const VectorF &velocity ) = 0;

      /// Sets the minimum and maximum distances for 3d falloff.
      virtual void setMinMaxDistance( F32 min, F32 max ) = 0;

      /// Set the distance attenuation rolloff factor.  Support by device optional.
      virtual void setRolloffFactor( F32 factor ) {}

      /// Sets the volume.
      virtual void setVolume( F32 volume ) = 0;

      /// Sets the pitch scale.
      virtual void setPitch( F32 pitch ) = 0;

      /// Set sound cone of a 3D sound.
      ///
      /// @param innerAngle Inner cone angle in degrees.
      /// @param outerAngle Outer cone angle in degrees.
      /// @param outerVolume Outer volume factor.
      virtual void setCone(   F32 innerAngle, 
                              F32 outerAngle,
                              F32 outerVolume ) = 0;
                              
      /// Set the reverb properties for playback of this sound.
      /// @note Has no effect on devices that do not support reverb.
      virtual void setReverb( const SFXSoundReverbProperties& reverb ) {}
      
      /// Set the priority of this voice.  Default 1.0.
      /// @note Has no effect on devices that do not support voice management.
      virtual void setPriority( F32 priority ) {}
      
      /// Returns true if the voice is virtualized on the device.
      /// @note Always false on devices that do not support voice management.
      virtual bool isVirtual() const { return false; }
};

#endif // _SFXVOICE_H_
