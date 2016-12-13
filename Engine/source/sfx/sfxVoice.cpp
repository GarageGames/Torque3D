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

#include "sfx/sfxVoice.h"
#include "sfx/sfxBuffer.h"
#include "sfx/sfxInternal.h"
#include "console/console.h"


// [rene, 07-May-11] The interplay between SFXBuffer and SFXVoice here isn't good.
//    Too complex, and while it works reliably in most cases, when doing seeks
//    on streaming sources, it is prone to subtle timing dependencies.


//#define DEBUG_SPEW


Signal< void( SFXVoice* voice ) > SFXVoice::smVoiceCreatedSignal;
Signal< void( SFXVoice* voice ) > SFXVoice::smVoiceDestroyedSignal;


//-----------------------------------------------------------------------------

SFXVoice::SFXVoice( SFXBuffer* buffer )
   : mStatus( SFXStatusNull ),
     mBuffer( buffer ),
     mOffset( 0 )
{
}

//-----------------------------------------------------------------------------

SFXVoice::~SFXVoice()
{
   smVoiceDestroyedSignal.trigger( this );
   
   if( mBuffer )
      mBuffer->mOnStatusChange.remove( this, &SFXVoice::_onBufferStatusChange );
}

//-----------------------------------------------------------------------------

void SFXVoice::_attachToBuffer()
{
   using namespace SFXInternal;

   // If the buffer is unique, attach us as its unique voice.

   if( mBuffer->isUnique() )
   {
      AssertFatal( !mBuffer->mUniqueVoice,
         "SFXVoice::SFXVoice - streaming buffer already is assigned a voice" );

      mBuffer->mUniqueVoice = this;

      // The buffer can start its queuing now so give it a chance
      // to run an update.
      SFXInternal::TriggerUpdate();
   }

   mBuffer->mOnStatusChange.notify( this, &SFXVoice::_onBufferStatusChange );
   
   smVoiceCreatedSignal.trigger( this );
}

//-----------------------------------------------------------------------------

void SFXVoice::_onBufferStatusChange( SFXBuffer* buffer, SFXBuffer::Status newStatus )
{
   AssertFatal( buffer == mBuffer, "SFXVoice::_onBufferStatusChange() - got an invalid buffer" );

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXVoice] Buffer changes status to: %s",
      newStatus == SFXBuffer::STATUS_Null ? "STATUS_Null" :
      newStatus == SFXBuffer::STATUS_Loading ? "STATUS_Loading" :
      newStatus == SFXBuffer::STATUS_Ready ? "STATUS_Ready" :
      newStatus == SFXBuffer::STATUS_Blocked ? "STATUS_Blocked" :
      newStatus == SFXBuffer::STATUS_AtEnd ? "STATUS_AtEnd" : "unknown" );
   #endif

   // This is called concurrently!

   switch( newStatus )
   {
      case SFXBuffer::STATUS_Loading:
         // Can ignore this.  Buffer simply lets us know it has started
         // its initial stream load.
         break;

      case SFXBuffer::STATUS_AtEnd:
         
         // Streaming voice has played to end of stream.

         if( dCompareAndSwap( ( U32& ) mStatus, SFXStatusPlaying, SFXStatusTransition ) )
         {
            _stop();
            mOffset = 0;

            dCompareAndSwap( ( U32& ) mStatus, SFXStatusTransition, SFXStatusStopped );
         }
         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[SFXVoice] Voice stopped as end of stream reached" );
         #endif
         break;

      case SFXBuffer::STATUS_Blocked:

         // Streaming has fallen behind.

         if( dCompareAndSwap( ( U32& ) mStatus, SFXStatusPlaying, SFXStatusTransition ) )
         {
            _pause();
            dCompareAndSwap( ( U32& ) mStatus, SFXStatusTransition, SFXStatusBlocked );
         }
         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[SFXVoice] Voice waiting for buffer to catch up" );
         #endif
         break;

      case SFXBuffer::STATUS_Ready:
         if( dCompareAndSwap( ( U32& ) mStatus, SFXStatusBlocked, SFXStatusTransition ) )
         {
            // Get the playback going again.

            _play();
            dCompareAndSwap( ( U32& ) mStatus, SFXStatusTransition, SFXStatusPlaying );

            #ifdef DEBUG_SPEW
            Platform::outputDebugString( "[SFXVoice] Buffer caught up with voice" );
            #endif
         }
         break;
      
      case SFXBuffer::STATUS_Null:
         AssertFatal( false, "SFXVoice::_onBufferStatusChange - Buffer changed to invalid NULL status" );
         break;
   }
}

//-----------------------------------------------------------------------------

SFXStatus SFXVoice::getStatus() const
{
   // Detect when the device has finished playback.  Only for
   // non-streaming voices.  For streaming voices, we rely on
   // the buffer to send us a STATUS_AtEnd signal when it is
   // done playing.

   if( mStatus == SFXStatusPlaying &&
       !mBuffer->isStreaming() &&
       _status() == SFXStatusStopped )
      mStatus = SFXStatusStopped;

   return mStatus;
}

//-----------------------------------------------------------------------------

void SFXVoice::play( bool looping )
{
   AssertFatal( mBuffer != NULL, "SFXVoice::play() - no buffer" );
   using namespace SFXInternal;

   // For streaming, check whether we have played previously.
   // If so, reset the buffer's stream.

   if( mBuffer->isStreaming() &&
       mStatus == SFXStatusStopped )
      _resetStream( 0 );

   // Now switch state.

   while( mStatus != SFXStatusPlaying &&
          mStatus != SFXStatusBlocked )
   {
      if( !mBuffer->isReady() &&
          ( dCompareAndSwap( ( U32& ) mStatus, SFXStatusNull, SFXStatusBlocked ) ||
            dCompareAndSwap( ( U32& ) mStatus, SFXStatusStopped, SFXStatusBlocked ) ||
            dCompareAndSwap( ( U32& ) mStatus, SFXStatusPaused, SFXStatusBlocked ) ) )
      {
         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[SFXVoice] Wanted to start playback but buffer isn't ready" );
         #endif

         break;
      }

      else if( dCompareAndSwap( ( U32& ) mStatus, SFXStatusNull, SFXStatusTransition ) ||
               dCompareAndSwap( ( U32& ) mStatus, SFXStatusStopped, SFXStatusTransition ) ||
               dCompareAndSwap( ( U32& ) mStatus, SFXStatusPaused, SFXStatusTransition ) )
      {
         _play();
         dCompareAndSwap( ( U32& ) mStatus, SFXStatusTransition, SFXStatusPlaying );

         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[SFXVoice] Started playback" );
         #endif

         break;
      }
   }
}

//-----------------------------------------------------------------------------

void SFXVoice::pause()
{
   while( mStatus != SFXStatusPaused &&
          mStatus != SFXStatusNull &&
          mStatus != SFXStatusStopped )
   {
      if( dCompareAndSwap( ( U32& ) mStatus, SFXStatusPlaying, SFXStatusTransition ) ||
          dCompareAndSwap( ( U32& ) mStatus, SFXStatusBlocked, SFXStatusTransition ) )
      {
         _pause();
         dCompareAndSwap( ( U32& ) mStatus, SFXStatusTransition, SFXStatusPaused );

         break;
      }
   }
}

//-----------------------------------------------------------------------------

void SFXVoice::stop()
{
   while( mStatus != SFXStatusStopped &&
          mStatus != SFXStatusNull )
   {
      if( dCompareAndSwap( ( U32& ) mStatus, ( U32 ) SFXStatusPlaying, ( U32 ) SFXStatusTransition ) ||
          dCompareAndSwap( ( U32& ) mStatus, SFXStatusPaused, SFXStatusTransition ) ||
          dCompareAndSwap( ( U32& ) mStatus, SFXStatusBlocked, SFXStatusTransition ) )
      {
         _stop();
         dCompareAndSwap( ( U32& ) mStatus, SFXStatusTransition, SFXStatusStopped );

         break;
      }
   }
}

//-----------------------------------------------------------------------------

U32 SFXVoice::getPosition() const
{
   // When stopped, always return 0.

   if( getStatus() == SFXStatusStopped )
      return 0;

   // It depends on the device if and when it will return a count of the total samples
   // played so far.  With streaming buffers, all devices will do that.  With non-streaming
   // buffers, some may do for looping voices thus returning a number that exceeds the actual
   // source stream size.  So, clamp things into range here and also take care of any offsetting
   // resulting from a setPosition() call.

   U32 pos = _tell() + mOffset;
   const U32 numStreamSamples = mBuffer->getFormat().getSampleCount( mBuffer->getDuration() );

   if( mBuffer->mIsLooping )
      pos %= numStreamSamples;
   else if( pos > numStreamSamples )
   {
      // Ensure we never report out-of-range positions even if the device does.

      pos = numStreamSamples;
   }

   return pos;
}

//-----------------------------------------------------------------------------

void SFXVoice::setPosition( U32 inSample )
{
   // Clamp to sample range.
   const U32 sample = inSample % ( mBuffer->getFormat().getSampleCount( mBuffer->getDuration() ) - 1 );
   
   // Don't perform a seek when we already are at the
   // given position.  Especially avoids a costly stream
   // clone when seeking on a streamed voice.
   
   if( getPosition() == sample )
      return;

   if( !mBuffer->isStreaming() )
   {
      // Non-streaming sound.  Just seek in the device buffer.

      _seek( sample );
   }
   else
   {
      // Streaming sound.  Reset the stream and playback.
      //
      // Unfortunately, the logic here is still prone to subtle timing dependencies
      // in relation to the buffer updates.  In retrospect, I feel that solving all issues
      // of asynchronous operation on a per-voice/buffer level has greatly complicated
      // the system.  It seems now that it would have been a lot simpler to have a single
      // asynchronous buffer/voice manager that manages the updates of all voices and buffers
      // currently in the system in one spot.  Packet reads could still be pushed out to
      // the thread pool but queue updates would all be handled centrally in one spot.  This
      // would do away with problems like those (mostly) solved by the multi-step procedure
      // here.

      // Go into transition.

      SFXStatus oldStatus;
      while( true )
      {
         oldStatus = mStatus;
         if( oldStatus != SFXStatusTransition &&
             dCompareAndSwap( ( U32& ) mStatus, oldStatus, SFXStatusTransition ) )
            break;
      }

      // Switch the stream.

      _resetStream( sample, false );

      // Come out of transition.

      SFXStatus newStatus = oldStatus;
      if( oldStatus == SFXStatusPlaying )
         newStatus = SFXStatusBlocked;

      dCompareAndSwap( ( U32& ) mStatus, SFXStatusTransition, newStatus );

      // Trigger an update.

      SFXInternal::TriggerUpdate();
   }
}

//-----------------------------------------------------------------------------

void SFXVoice::_resetStream( U32 sampleStartPos, bool triggerUpdate )
{
   AssertFatal( mBuffer->isStreaming(), "SFXVoice::_resetStream - Not a streaming voice!" );

   ThreadSafeRef< SFXBuffer::AsyncState > oldState = mBuffer->mAsyncState;
   AssertFatal( oldState != NULL,
      "SFXVoice::_resetStream() - streaming buffer must have valid async state" );

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXVoice] Resetting stream to %i", sampleStartPos );
   #endif

   // Rather than messing up the async code by adding repositioning (which
   // further complicates synchronizing the various parts), just construct
   // a complete new AsyncState and discard the old one.  The only problem
   // here is the stateful sound streams.  We can't issue a new packet as long
   // as we aren't sure there's no request pending, so we just clone the stream
   // and leave the old one to the old AsyncState.

   ThreadSafeRef< SFXStream > sfxStream = oldState->mStream->getSourceStream()->clone();
   if( sfxStream == NULL )
   {
      Con::errorf( "SFXVoice::_resetStream - could not clone SFXStream" );
      return;
   }

   IPositionable< U32 >* sfxPositionable = dynamic_cast< IPositionable< U32 >* >( sfxStream.ptr() );
   if( !sfxPositionable )
   {
      Con::errorf( "SFXVoice::_resetStream - could not seek in SFXStream" );
      return;
   }

   sfxPositionable->setPosition( sampleStartPos * sfxStream->getFormat().getBytesPerSample() );
   
   ThreadSafeRef< SFXInternal::SFXAsyncStream > newStream =
      new SFXInternal::SFXAsyncStream
         ( sfxStream,
           true,
           oldState->mStream->getPacketDuration() / 1000,
           oldState->mStream->getReadAhead(),
           oldState->mStream->isLooping() );
   newStream->setReadSilenceAtEnd( oldState->mStream->getReadSilenceAtEnd() );

   AssertFatal( newStream->getPacketSize() == oldState->mStream->getPacketSize(),
      "SFXVoice::setPosition() - packet size mismatch with new stream" );

   ThreadSafeRef< SFXBuffer::AsyncState > newState =
      new SFXBuffer::AsyncState( newStream );
   newStream->start();

   // Switch the states.

   mOffset = sampleStartPos;
   mBuffer->mAsyncState = newState;

   // Stop the old state from reading more data.

   oldState->mStream->stop();

   // Trigger update.

   if( triggerUpdate )
      SFXInternal::TriggerUpdate();
}
