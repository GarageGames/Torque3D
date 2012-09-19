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

#include "sfx/sfxInternal.h"
#include "sfx/sfxDescription.h"
#include "core/util/safeDelete.h"
#include "platform/threads/threadPoolAsyncIO.h"


/// @file
/// Implementation of async sound I/O.


//#define DEBUG_SPEW


namespace SFXInternal {


ThreadSafeRef< SFXUpdateThread >       gUpdateThread;
ThreadSafeRef< SFXBufferProcessList >  gBufferUpdateList = new SFXBufferProcessList;
ThreadSafeDeque< SFXBuffer* >          gDeadBufferList;


//==========================================================================
//    SFXAsyncStream implementation.
//==========================================================================

//--------------------------------------------------------------------------

SFXAsyncStream::SFXAsyncStream(  const SFXStreamRef& stream,
                                 bool isIncremental,
                                 U32 streamPacketLength,
                                 U32 numReadAhead,
                                 bool isLooping )
   : Parent(   stream,
               isIncremental
               ? streamPacketLength
                  * stream->getFormat().getSamplesPerSecond()
                  * stream->getFormat().getBytesPerSample()    // Streamed buffer; read in incremental packets.
               : stream->getDataLength(),                      // Non-streamed buffer; read entire stream in one packet.
               stream->getDataLength()                         // Read all remaining data in stream.
               - ( dynamic_cast< IPositionable< U32 >* >( stream.ptr() )
                   ? dynamic_cast< IPositionable< U32 >* >( stream.ptr() )->getPosition()
                   : 0 ),
               numReadAhead,
               isLooping,
               &THREAD_POOL() ),
     mReadSilenceAtEnd( false )
{
}

//--------------------------------------------------------------------------

void SFXAsyncStream::_onArrival( SFXStreamPacket* const& packet )
{
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXAsyncStream] Packet arrived" );
   #endif

   Parent::_onArrival( packet );

   // Some buffer may be waiting for this data so trigger
   // an update.
   
   if( !mIsStopped )
      TriggerUpdate();
}

//--------------------------------------------------------------------------

void SFXAsyncStream::_requestNext()
{
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXAsyncStream] Next packet requested" );
   #endif

   if( !mNumRemainingSourceElements && mReadSilenceAtEnd )
   {
      // Push an artificial packet of silence.
      
      SFXStreamPacket* packet = _newPacket( mPacketSize );
      packet->mIndex = mNextPacketIndex;
      mNextPacketIndex ++;
      mReadSilenceAtEnd = false;
      dMemset( packet->data, 0, packet->size );
      packet->mIsLast = true;
      
      _onArrival( packet );
   }
   else
      Parent::_requestNext();
}

//==========================================================================
//    SFXWrapAroundBuffer implementation.
//==========================================================================

//--------------------------------------------------------------------------

SFXWrapAroundBuffer::SFXWrapAroundBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description )
   : Parent( stream, description ),
     mWriteOffset( 0 )
{
   // Determine the device buffer metrics.
   
   const U32 maxQueuedPackets = isStreaming() ? SFXAsyncQueue::DEFAULT_STREAM_QUEUE_LENGTH : 1;
   const U32 packetSize = mAsyncState->mStream->getPacketSize();

   mBufferSize = maxQueuedPackets * packetSize;
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXWrapAroundBuffer] size=%i, packets=%i",
      mBufferSize, maxQueuedPackets );
   #endif
   
   // For streaming buffers that are not looping, add a packet of silence to the
   // source stream.

   if( isStreaming() && !description->mIsLooping )
      mAsyncState->mStream->setReadSilenceAtEnd( true );
}

//--------------------------------------------------------------------------

void SFXWrapAroundBuffer::write( SFXStreamPacket* const* packets, U32 num )
{
   AssertFatal( SFXInternal::isSFXThread(), "SFXWrapAroundBuffer::write() - not on SFX thread" );

   for( U32 i = 0; i < num; ++ i )
   {
      const SFXStreamPacket* packet = packets[ i ];
      
      // Determine where in the buffer to copy the data to.  In case we are crossing over
      // the wrap-around point, we need to copy in two slices.

      U32 offset1    = 0;
      U32 offset2    = 0;
      U32 numBytes1  = 0;
      U32 numBytes2  = 0;

      offset1        = mWriteOffset % mBufferSize;
      numBytes1      = packet->size;

      if( offset1 + numBytes1 > mBufferSize )
      {
         // Crossing wrap-around point.
         
         numBytes1   = mBufferSize - offset1;
         numBytes2   = packet->size - numBytes1;
      }

      offset2        = offset1 + numBytes1;
      
      #ifdef DEBUG_SPEW
      Platform::outputDebugString( "[SFXWrapAroundBuffer] writing %i bytes from packet #%i at %i (stream offset: %i)",
         numBytes1, packet->mIndex, offset1, mWriteOffset );
      #endif
      
      // Copy the packet data.
      
      _copyData( offset1, packet->data, numBytes1 );
      if( numBytes2 > 0 )
      {
         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[SFXWrapAroundBuffer] writing %i more bytes at %i",
            numBytes2, offset2 );
         #endif
         
         _copyData( offset2, &packet->data[ numBytes1 ], numBytes2 );
      }
   
      dFetchAndAdd( mWriteOffset, packet->size );

      // Free the packet.

      destructSingle( packet );
   }
}

} // namespace SFXInternal
