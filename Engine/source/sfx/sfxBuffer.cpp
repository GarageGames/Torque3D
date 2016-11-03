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

#include "sfx/sfxBuffer.h"
#include "sfx/sfxVoice.h"
#include "sfx/sfxDescription.h"
#include "sfx/sfxInternal.h"


//#define DEBUG_SPEW


Signal< void( SFXBuffer* ) > SFXBuffer::smBufferDestroyedSignal;


//-----------------------------------------------------------------------------

SFXBuffer::SFXBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description, bool createAsyncState )
   : mStatus( STATUS_Null ),
     mFormat( stream->getFormat() ),
     mDuration( stream->getDuration() ),
     mIsStreaming( description->mIsStreaming ),
     mIsLooping( description->mIsLooping ),
     mIsUnique( description->mIsStreaming ),
     mIsDead( false ),
     mUniqueVoice( NULL )
{
   using namespace SFXInternal;
   
   if( createAsyncState )
   {
      U32 packetLength = description->mStreamPacketSize;
      U32 readAhead = description->mStreamReadAhead;

      ThreadSafeRef< SFXStream > streamRef( stream );
      mAsyncState = new AsyncState(
         new SFXAsyncStream
         ( streamRef, mIsStreaming, packetLength, readAhead,
           mIsStreaming ? description->mIsLooping : false ) );
   }
}

//-----------------------------------------------------------------------------

SFXBuffer::SFXBuffer( SFXDescription* description )
   : mStatus( STATUS_Ready ),
     mDuration( 0 ), // Must be set by subclass.
     mIsStreaming( false ), // Not streaming through our system.
     mIsLooping( description->mIsLooping ),
     mIsUnique( false ), // Must be set by subclass.
     mIsDead( false ),
     mUniqueVoice( NULL )
{
}

//-----------------------------------------------------------------------------

SFXBuffer::~SFXBuffer()
{
   smBufferDestroyedSignal.trigger( this );
}

//-----------------------------------------------------------------------------

void SFXBuffer::load()
{
   if( getStatus() == STATUS_Null )
   {
      AssertFatal( mAsyncState != NULL, "SFXBuffer::load() - no async state!" );

      _setStatus( STATUS_Loading );
      SFXInternal::UPDATE_LIST().add( this );
      mAsyncState->mStream->start();
   }
}

//-----------------------------------------------------------------------------

bool SFXBuffer::update()
{
   using namespace SFXInternal;

   if( isDead() )
   {
      // Buffer requested to finish its async operations.
      // Kill our async state and put us on the dead buffer list.

      mAsyncState->mStream->stop();
      mAsyncState = NULL;
      gDeadBufferList.pushFront( this );
      return false;
   }
   else if( isAtEnd() && isStreaming() )
   {
      // Streaming buffers remain in the update loop even if they
      // have played in full to allow for stream seeks.
      return true;
   }

   AssertFatal( mAsyncState != NULL, "SFXBuffer::update() - async state has already been released" );

   bool needFurtherUpdates = true;
   if( !isStreaming() )
   {
      // Not a streaming buffer.  If the async stream has its data
      // ready, we load it and finish up on our async work.

      SFXStreamPacket* packet;
      while( mAsyncState->mStream->read( &packet, 1 ) )
      {
         bool isLast = packet->mIsLast;

         // Write packet data into buffer.
         write( &packet, 1 );
         packet = NULL;
         
         if( isLast )
         {
            // Release async state.
            mAsyncState = NULL;

            // Once loaded, non-streaming buffers disappear from the SFX
            // update thread.
            needFurtherUpdates = false;

            // Signal that we are ready.
            _setStatus( STATUS_Ready );

            #ifdef DEBUG_SPEW
            Platform::outputDebugString( "[SFXBuffer] Buffer ready" );
            #endif

            break;
         }
      }
   }
   else
   {
      // A streaming buffer.
      //
      // If we don't have a queue, construct one now.  Note that when doing
      // a stream seek on us, SFXVoice will drop our async stream and queue.
      // Work on local copies of the pointers to allow having the async state
      // be switched in parallel.
      //
      // Note that despite us being a streamed buffer, our unique voice may
      // not yet have been assigned to us.

      AsyncStatePtr state = mAsyncState;
      if( !state->mQueue && !mUniqueVoice.isNull() )
      {
         // Make sure we have no data currently submitted to the device.
         // This will stop and discard an outdated feed if we've been
         // switching streams.
         
         _setStatus( STATUS_Loading );
         _flush();

         // Create a new queue.

         state->mQueue = new SFXAsyncQueue( mUniqueVoice, this, mIsLooping );
      }
      
      // Check the queue.

      if( state->mQueue != NULL )
      {
         // Feed the queue, if necessary and possible.

         while( state->mQueue->needPacket() )
         {
            // Try to read a packet.

            SFXStreamPacket* packet;
            if( !state->mStream->read( &packet, 1 ) )
               break;

            // Submit it.

            state->mQueue->submitPacket( packet, packet->getSampleCount() );

            #ifdef DEBUG_SPEW
            Platform::outputDebugString( "[SFXBuffer] Stream packet queued" );
            #endif

            // Signal that the buffer has data ready.

            _setStatus( STATUS_Ready );
         }

         // Detect buffer underrun and end-of-stream.

         if( !isReady() && state->mQueue->isEmpty() )
         {
            #ifdef DEBUG_SPEW
            Platform::outputDebugString( "[SFXBuffer] Stream blocked" );
            #endif

            _setStatus( STATUS_Blocked );
         }
         else if( state->mQueue->isAtEnd() )
         {
            #ifdef DEBUG_SPEW
            Platform::outputDebugString( "[SFXBuffer] Stream at end" );
            #endif

            _setStatus( STATUS_AtEnd );
            _flush();
         }
      }
   }

   return needFurtherUpdates;
}

//-----------------------------------------------------------------------------

void SFXBuffer::destroySelf()
{
   AssertFatal( !isDead(), "SFXBuffer::destroySelf() - buffer already dead" );

   mIsDead = true;
   if( !mAsyncState )
   {
      // Easy way.  This buffer has finished all its async
      // processing, so we can just kill it.

      delete this;
   }
   else
   {
      // Hard way.  We will have to make the buffer finish
      // all its concurrent stuff, so we mark it dead, make sure
      // to see an update, and then wait for the buffer to surface
      // on the dead buffer list.

      SFXInternal::TriggerUpdate();
   }
}

//-----------------------------------------------------------------------------

void SFXBuffer::_setStatus( Status status )
{
   if( mStatus != status )
   {
      mOnStatusChange.trigger( this, status );
      mStatus = status;
   }
}

//-----------------------------------------------------------------------------

SFXBuffer::AsyncState::AsyncState()
   : mQueue( NULL )
{
}

//-----------------------------------------------------------------------------

SFXBuffer::AsyncState::AsyncState( SFXInternal::SFXAsyncStream* stream )
   : mStream( stream ), mQueue( NULL )
{
}

//-----------------------------------------------------------------------------

SFXBuffer::AsyncState::~AsyncState()
{
   if( mQueue )
      SAFE_DELETE( mQueue );
}
