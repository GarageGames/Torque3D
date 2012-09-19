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

#ifndef _SFXINTERNAL_H_
#define _SFXINTERNAL_H_

#ifndef _THREADPOOL_H_
   #include "platform/threads/threadPool.h"
#endif
#ifndef _ASYNCUPDATE_H_
   #include "platform/async/asyncUpdate.h"
#endif
#ifndef _ASYNCPACKETSTREAM_H_
   #include "platform/async/asyncPacketStream.h"
#endif
#ifndef _ASYNCPACKETQUEUE_H_
   #include "platform/async/asyncPacketQueue.h"
#endif
#ifndef _SFXSTREAM_H_
   #include "sfx/sfxStream.h"
#endif
#ifndef _SFXBUFFER_H_
   #include "sfx/sfxBuffer.h"
#endif
#ifndef _SFXVOICE_H_
   #include "sfx/sfxVoice.h"
#endif
#ifndef _CONSOLE_H_
   #include "console/console.h"
#endif
#ifndef _TSINGLETON_H_
   #include "core/util/tSingleton.h"
#endif


/// @file
/// Mostly internal definitions for sound stream handling.
/// The code here is used by SFXBuffer for asynchronously loading
/// sample data from sound files, both for streaming buffers
/// as well as for "normal" buffers.
///
/// This is all pretty low-level code here.


namespace SFXInternal {

typedef AsyncUpdateThread SFXUpdateThread;
typedef AsyncUpdateList SFXBufferProcessList;

//--------------------------------------------------------------------------
//    Async sound packets.
//--------------------------------------------------------------------------

/// Sound stream packets are raw byte buffers containing PCM sample data.
class SFXStreamPacket : public AsyncPacket< U8 >
{
   public:

      typedef AsyncPacket< U8 > Parent;

      SFXStreamPacket() {}
      SFXStreamPacket( U8* data, U32 size, bool ownMemory = false )
         : Parent( data, size, ownMemory ) {}

      /// The format of the sound samples in the packet.
      SFXFormat mFormat;

      /// @return the number of samples contained in the packet.
      U32 getSampleCount() const { return ( mSizeActual / mFormat.getBytesPerSample() ); }
};

//--------------------------------------------------------------------------
//    Async SFXStream I/O.
//--------------------------------------------------------------------------

/// Asynchronous sound data stream that delivers sound data
/// in discrete packets.
class SFXAsyncStream : public AsyncPacketBufferedInputStream< SFXStreamRef, SFXStreamPacket >
{
   public:

      typedef AsyncPacketBufferedInputStream< SFXStreamRef, SFXStreamPacket > Parent;

      enum
      {
         /// The number of seconds of sample data to load per streaming packet by default.
         /// Set this reasonably high to ensure the system is able to cope with latencies
         /// in the buffer update chain.
         DEFAULT_STREAM_PACKET_LENGTH = 8
      };

   protected:

      /// If true, the stream reads one packet of silence beyond the
      /// sound streams actual sound data.  This is to avoid wrap-around
      /// playback queues running into old data when there is a delay
      /// in playback being stopped.
      ///
      /// @note The silence packet is <em>not</em> counting towards stream
      ///   playback time.
      bool mReadSilenceAtEnd;

      // AsyncPacketStream.
      virtual SFXStreamPacket* _newPacket( U32 packetSize )
      {
         SFXStreamPacket* packet = Parent::_newPacket( packetSize );
         packet->mFormat = getSourceStream()->getFormat();
         return packet;
      }
      virtual void _requestNext();
      virtual void _onArrival( SFXStreamPacket* const& packet );
      virtual void _newReadItem( PacketReadItemRef& outRef, SFXStreamPacket* packet, U32 numElements )
      {
         if( !this->mNumRemainingSourceElements && mReadSilenceAtEnd )
            packet->mIsLast = false;
         Parent::_newReadItem( outRef, packet, numElements );
      }

   public:

      /// Construct a new async sound stream reading data from "stream".
      ///
      /// @param stream The sound data source stream.
      /// @param isIncremental If true, "stream" is read in packets of "streamPacketLength" size
      ///   each; otherwise the stream is read in a single packet containing the entire stream.
      /// @param streamPacketLength Seconds of sample data to read per streaming packet.  Only
      ///   relevant if "isIncremental" is true.
      /// @param numReadAhead Number of stream packets to read and buffer in advance.
      /// @param isLooping If true, the packet stream infinitely loops over "stream".
      SFXAsyncStream( const SFXStreamRef& stream,
                      bool isIncremental,
                      U32 streamPacketLength = DEFAULT_STREAM_PACKET_LENGTH,
                      U32 numReadAhead = DEFAULT_STREAM_LOOKAHEAD,
                      bool isLooping = false );

      /// Returns true if the stream will read a packet of silence after the actual sound data.
      U32 getReadSilenceAtEnd() const { return mReadSilenceAtEnd; }

      /// Set whether the stream should read one packet of silence past the
      /// actual sound data.  This is useful for situations where continued
      /// playback may run into old data.
      void setReadSilenceAtEnd( bool value ) { mReadSilenceAtEnd = value; }

      /// Return the playback time of a single sound packet in milliseconds.
      /// For non-incremental streams, this will be the duration of the
      /// entire stream.
      U32 getPacketDuration() const
      {
         const SFXFormat& format = getSourceStream()->getFormat();
         return format.getDuration( mPacketSize / format.getBytesPerSample() );
      }
};

//--------------------------------------------------------------------------
//    Voice time source wrapper.
//--------------------------------------------------------------------------

/// Wrapper around SFXVoice that yields the raw underlying sample position
/// rather than the virtualized position returned by SFXVoice::getPosition().
class SFXVoiceTimeSource
{
   public:

      typedef void Parent;

   protected:

      /// The voice to sample the position from.
      SFXVoice* mVoice;

      /// Last position returned by voice.
      mutable U32 mLastPos;

   public:

      SFXVoiceTimeSource( SFXVoice* voice )
         : mVoice( voice ), mLastPos( 0 ) {}

      U32 getPosition() const
      {
         U32 samplePos = mVoice->_tell();

         // The device playback cursor may snap back to an undefined value as soon
         // as all the data has been consumed.  However, for us to be a reliable
         // time source, we can't let that happen so as soon as the device play cursor
         // goes back to a sample position we have already passed, we start reporting an
         // end-of-stream position.

         if( samplePos < mLastPos && mVoice->mBuffer != NULL )
            samplePos = mVoice->mBuffer->getNumSamples();
         else
            mLastPos = samplePos;
         
         return samplePos;
      }
};

//--------------------------------------------------------------------------
//    Async sound packet queue.
//--------------------------------------------------------------------------

/// An async stream queue that writes sound packets to SFXBuffers in sync
/// to the playback of an SFXVoice.
///
/// Sound packet queues use sample counts as tick counts.
class SFXAsyncQueue : public AsyncPacketQueue< SFXStreamPacket*, SFXVoiceTimeSource, SFXBuffer* >
{
   public:

      typedef AsyncPacketQueue< SFXStreamPacket*, SFXVoiceTimeSource, SFXBuffer* > Parent;

      enum
      {
         /// The number of stream packets that the playback queue for streaming
         /// sounds will be sliced into.  This should generally be left at
         /// three since there is an overhead incurred for each additional
         /// segment.  Having three segments gives one segment for current
         /// immediate playback, one segment as intermediate buffer, and one segment
         /// for stream writes.
         DEFAULT_STREAM_QUEUE_LENGTH = 3,
      };

      /// Construct a new sound queue that pushes sound packets to "buffer" in sync
      /// to the playback of "voice".
      ///
      /// @param voice The SFXVoice to synchronize to.
      /// @param buffer The sound buffer to push sound packets to.
      SFXAsyncQueue(    SFXVoice* voice,
                        SFXBuffer* buffer,
                        bool looping = false )
         : Parent(   DEFAULT_STREAM_QUEUE_LENGTH,
                        voice,
                        buffer,
                        ( looping
                          ? 0
                          : ( buffer->getDuration() * ( buffer->getFormat().getSamplesPerSecond() / 1000 ) ) - voice->mOffset ) ) {}
};

//--------------------------------------------------------------------------
//    SFXBuffer with a wrap-around buffering scheme.
//--------------------------------------------------------------------------

/// Buffer that uses wrap-around packet buffering.
///
/// This class automatically coordinates retrieval and submitting of
/// sound packets and also protects against play cursors running beyond
/// the last packet by making sure some silence is submitted after the
/// last packet (does not count towards playback time).
///
/// @note Note that the reaction times of this class depend on the SFXDevice
///   triggering timely SFXBuffer:update() calls.
class SFXWrapAroundBuffer : public SFXBuffer
{
   public:

      typedef SFXBuffer Parent;

   protected:

      /// Absolute byte offset into the sound stream that the next packet write
      /// will occur at.  This is <em>not</em> an offset into the device buffer
      /// in order to allow us to track how far in the source stream we are.
      U32 mWriteOffset;

      /// Size of the device buffer in bytes.
      U32 mBufferSize;

      // SFXBuffer.
      virtual void _flush()
      {
         mWriteOffset = 0;
      }

      /// Copy "length" bytes from "data" into the device at "offset".
      virtual bool _copyData( U32 offset, const U8* data, U32 length ) = 0;

      // SFXBuffer.
      virtual void write( SFXStreamPacket* const* packets, U32 num );

      /// @return the sample position in the sound stream as determined from the
      ///   given buffer offset.
      U32 getSamplePos( U32 bufferOffset ) const
      {
         if( !mBufferSize )
            return ( bufferOffset / getFormat().getBytesPerSample() );
            
         const U32 writeOffset = mWriteOffset; // Concurrent writes on this one.
         const U32 writeOffsetRelative = writeOffset % mBufferSize;
         
         U32 numBufferedBytes;
         if( !writeOffset )
            numBufferedBytes = 0;
         else if( writeOffsetRelative > bufferOffset )
            numBufferedBytes = writeOffsetRelative - bufferOffset;
         else
            // Wrap-around.
            numBufferedBytes = mBufferSize - bufferOffset + writeOffsetRelative;

         const U32 bytePos = writeOffset - numBufferedBytes;
         
         return ( bytePos / getFormat().getBytesPerSample() );
      }

   public:

      SFXWrapAroundBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description );
      SFXWrapAroundBuffer( SFXDescription* description )
         : Parent( description ), mBufferSize( 0 ) {}
         
      virtual U32 getMemoryUsed() const { return mBufferSize; }
};

//--------------------------------------------------------------------------
//    Global state.
//--------------------------------------------------------------------------

enum
{
   /// Soft limit on milliseconds to spend on updating sound buffers
   /// when doing buffer updates on the main thread.
   MAIN_THREAD_PROCESS_TIMEOUT = 512,
   
   /// Default time interval between periodic sound updates in milliseconds.
   /// Only relevant for devices that perform periodic updates.
   DEFAULT_UPDATE_INTERVAL = 512,
};

/// Thread pool for sound I/O.
///
/// We are using a separate pool for sound packets in order to be
/// able to submit packet items from different threads.  This would
/// violate the invariant of the global thread pool that only the
/// main thread is feeding the queues.
///
/// Note that this also means that only at certain very well-defined
/// points is it possible to safely flush the work item queue on this
/// pool.
///
/// @note Don't use this directly but rather use THREAD_POOL() instead.
///   This way, the sound code may be easily switched to using a common
///   pool later on.
class SFXThreadPool : public ThreadPool, public ManagedSingleton< SFXThreadPool >
{
   public:
   
      typedef ThreadPool Parent;
      
      /// Create a ThreadPool called "SFX" with two threads.
      SFXThreadPool()
         : Parent( "SFX", 2 ) {}
         
      // For ManagedSingleton.
      static const char* getSingletonName() { return "SFXThreadPool"; }
};

/// Dedicated thread that does sound buffer updates.
/// May be NULL if sound API used does not do asynchronous buffer
/// updates but rather uses per-frame polling.
///
/// @note SFXDevice automatically polls if this is NULL.
extern ThreadSafeRef< AsyncUpdateThread > gUpdateThread;

/// List of buffers that need updating.
///
/// It depends on the actual device whether this list is processed
/// on a stream update thread or on the main thread.
extern ThreadSafeRef< SFXBufferProcessList > gBufferUpdateList;

/// List of buffers that are pending deletion.
///
/// This is a messy issue.  Buffers with live async states cannot be instantly
/// deleted since they may still be running concurrent updates.  However, they
/// also cannot be deleted on the update thread since the StrongRefBase stuff
/// isn't thread-safe (i.e weak references kept by client code would cause trouble).
///
/// So, what we do is mark buffers for deletion, wait till they surface on the
/// process list and then ping them back to this list to have them deleted by the
/// SFXDevice itself on the main thread.  A bit of overhead but only a fraction of
/// the buffers will ever undergo this procedure.
extern ThreadSafeDeque< SFXBuffer* > gDeadBufferList;

/// Return the thread pool used for SFX work.
inline ThreadPool& THREAD_POOL()
{
   return *( SFXThreadPool::instance() );
}

/// Return the dedicated SFX update thread; NULL if updating on the main thread.
inline ThreadSafeRef< SFXUpdateThread > UPDATE_THREAD()
{
   return gUpdateThread;
}

/// Return the processing list for SFXBuffers that need updating.
inline SFXBufferProcessList& UPDATE_LIST()
{
   return *gBufferUpdateList;
}

/// Trigger an SFX update.
inline bool TriggerUpdate()
{
   ThreadSafeRef< SFXUpdateThread > sfxThread = UPDATE_THREAD();
   if( sfxThread != NULL )
   {
      sfxThread->triggerUpdate();
      return true;
   }
   else
      return false;
}

/// Delete all buffers currently on the dead buffer list.
inline void PurgeDeadBuffers()
{
   SFXBuffer* buffer;
   while( gDeadBufferList.tryPopFront( buffer ) )
      delete buffer;
}

/// Return true if the current thread is the one responsible for doing SFX updates.
inline bool isSFXThread()
{
   ThreadSafeRef< SFXUpdateThread > sfxThread = UPDATE_THREAD();

   U32 threadId;
   if( sfxThread != NULL )
      threadId = sfxThread->getId();
   else
      threadId = ThreadManager::getMainThreadId();

   return ThreadManager::compare( ThreadManager::getCurrentThreadId(), threadId );
}

} // namespace SFXInternal

#endif // !_SFXINTERNAL_H_
