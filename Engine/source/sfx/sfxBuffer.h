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

#ifndef _SFXBUFFER_H_
#define _SFXBUFFER_H_

#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif

#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif

#ifndef _TSTREAM_H_
#include "core/stream/tStream.h"
#endif

#ifndef _SFXCOMMON_H_
#include "sfx/sfxCommon.h"
#endif

#ifndef _THREADSAFEREFCOUNT_H_
#include "platform/threads/threadSafeRefCount.h"
#endif


class SFXStream;
class SFXDescription;
class SFXVoice;

namespace SFXInternal {
   class SFXStreamPacket;
   class SFXAsyncStream;
   class SFXAsyncQueue;
   void PurgeDeadBuffers();
}


/// The buffer interface hides the details of how the device holds sound data for playback.
///
/// A sound buffer may either be loaded once completely and then played as needed or it may
/// be progressively streamed from an SFXStream.  In the latter case, there can only be a single
/// voice tied to the buffer.
///
/// @note SFXDevice is the last instance when it comes to ownership
///   of SFXBuffers.  If the SFXDevice goes away, it will take all
///   SFXBuffers with it, regardless of whether there are still strong
///   refs to it.  Use StrongWeakRefPtrs to keep pointers to
///   SFXBuffers!
///
/// @see SFXStream
class SFXBuffer :    public StrongRefBase,
                     public IPolled, // SFXBuffers are periodically updated on the SFX thread.
                     public IOutputStream< SFXInternal::SFXStreamPacket* > // Interface for writing sound data to the buffer.
{
      friend class SFXVoice; // mUniqueVoice
      friend void SFXInternal::PurgeDeadBuffers(); // dtor

   public:

      typedef void Parent;

      /// Status indicators for sound buffers.
      enum Status
      {
         STATUS_Null,      ///< Initial state.
         STATUS_Loading,   ///< Buffer has requested data and is waiting for queue to fill up.
         STATUS_Ready,     ///< Playback queue is fed and ready (non-stream buffers will stop at this state).
         STATUS_Blocked,   ///< Queue is starved and playback thus held until further data is available (streaming buffers only).
         STATUS_AtEnd,     ///< Buffer has read all its streaming data (streaming buffers only).
      };

      /// This signal is triggered from SFXBuffer's destructor so the sound system
      /// can keep track of buffers being released on the device.
      static Signal< void( SFXBuffer* ) > smBufferDestroyedSignal;

   protected:

      typedef ThreadSafeRef< SFXInternal::SFXAsyncStream > SFXAsyncStreamPtr;
      typedef SFXInternal::SFXAsyncQueue* SFXAsyncQueuePtr;

      /// Encapsulates the async I/O state of the sound buffer.
      struct AsyncState : public ThreadSafeRefCount< AsyncState >
      {
         /// The sound packet stream.
         SFXAsyncStreamPtr mStream;

         /// The packet queue that feeds into the actual device buffer.
         /// Only used for streaming buffers; non-streaming buffers directly receive
         /// and upload sound packets without queuing.
         SFXAsyncQueuePtr mQueue;

         AsyncState();
         AsyncState( SFXInternal::SFXAsyncStream* stream );
         ~AsyncState();
      };

      typedef ThreadSafeRef< AsyncState > AsyncStatePtr;

      /// Create a new buffer from @a stream using the parameters in @a description.
      ///
      /// @param stream Sound stream from which to read sound data into the buffer.
      /// @param description Sound setup description.
      /// @param createAsyncState If true, the asynchronous loading state for the buffer will be set up
      ///   in the constructor.  This is mainly useful for the null device which creates dummy buffers that
      ///   do not need the async state to be in place.  All other buffers do.
      SFXBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description, bool createAsyncState = true );
      
      /// Create a buffer with just a description.  This is used by devices who fully take over loading
      /// and streaming.
      SFXBuffer( SFXDescription* description );
      
      virtual ~SFXBuffer();

      /// The buffer readiness status.
      Status mStatus;
      
      /// The sound sample format used by the buffer.
      SFXFormat mFormat;

      /// Total playback time of the associated sound stream in milliseconds.
      /// @note For streaming buffers, this will not correspond to the actual
      ///   playtime of the device buffer.
      U32 mDuration;

      /// If true, this is a continuously streaming buffer.
      bool mIsStreaming;
      
      /// For streaming buffers, tells whether the source stream loops.
      bool mIsLooping;

      /// If true, this buffer can only have a single SFXVoice attached.
      bool mIsUnique;

      /// If true, the buffer is dead and will be deleted.  Can't be in status
      /// for synchronization reasons.
      bool mIsDead;

      /// Pointer to structure keeping the asynchronous I/O state of the buffer.
      /// For non-streaming buffers, this is released as soon as all data is loaded.
      ///
      /// To allow seeking in streaming buffers even after playback has ended,
      /// we do not release the async state of these buffers until the buffer is
      /// actually released itself.  This allows to always access the associated
      /// input stream.
      ///
      /// @note For devices that handle loading/streaming on their own, this will
      ///   not be set.
      AsyncStatePtr mAsyncState;

      /// If this is a unique buffer (i.e. a streaming buffer), then this holds
      /// the reference to the unique voice.
      StrongWeakRefPtr< SFXVoice > mUniqueVoice;

      /// Set the buffer status and trigger mOnStatusChange if the status changes.
      /// @note Called on both the SFX update thread and the main thread.
      void _setStatus( Status status );

      /// Flush all queue state for this buffer on the device.
      /// @note Called on the SFX update thread.
      virtual void _flush() = 0;

   public:

      /// Signal that is triggered when the buffer status changes.
      /// @note This signal is triggered on the same thread that the buffer update
      ///   code runs on.
      Signal< void( SFXBuffer* buffer, Status newStatus ) > mOnStatusChange;

      /// @return The current buffer loading/queue status.
      Status getStatus() const { return mStatus; }

      /// @return The sound sample format used by the buffer.
      const SFXFormat& getFormat() const { return mFormat; }

      /// @return The total playback time of the buffer in milliseconds.
      U32 getDuration() const { return mDuration; }

      /// @return The total number of samples in the buffer.
      U32 getNumSamples() const { return getFormat().getSampleCount( mDuration ); }

      /// @return The number of bytes consumed by this sound buffer.
      virtual U32 getMemoryUsed() const { return 0; }

      /// @return True if the buffer does continuous sound streaming.
      bool isStreaming() const { return mIsStreaming; }

      /// @return True if the buffer is pending deletion.
      bool isDead() const { return mIsDead; }

      /// @return True if the buffer's packet queue is loaded and ready for playback.
      bool isReady() const { return ( getStatus() == STATUS_Ready ); }

      /// @return True if the buffer's packet queue is still loading.
      bool isLoading() const { return ( getStatus() == STATUS_Loading ); }

      /// @return True if the buffer's packet queue has been starved and is waiting for data.
      bool isBlocked() const { return ( getStatus() == STATUS_Blocked ); }

      /// @return True if the buffer has exhausted its source stream
      bool isAtEnd() const { return ( getStatus() == STATUS_AtEnd ); }

      /// @return True if the buffer can only have a single SFXVoice attached to it.
      bool isUnique() const { return mIsUnique; }

      /// Start the async request chain for the buffer.
      void load();

      // IPolled.
      virtual bool update();

      // WeakRefBase.
      virtual void destroySelf();
};

#endif // _SFXBUFFER_H_
