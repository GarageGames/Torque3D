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

#include "sfx/xaudio/sfxXAudioBuffer.h"
#include "sfx/xaudio/sfxXAudioVoice.h"


//#define DEBUG_SPEW


SFXXAudioBuffer* SFXXAudioBuffer::create( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description )
{
   SFXXAudioBuffer *buffer = new SFXXAudioBuffer( stream, description );
   return buffer;
}

SFXXAudioBuffer::SFXXAudioBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description )
   : Parent( stream, description )
{
   VECTOR_SET_ASSOCIATION( mBufferQueue );
}

SFXXAudioBuffer::~SFXXAudioBuffer()
{
   _flush();
}

void SFXXAudioBuffer::write( SFXInternal::SFXStreamPacket* const* packets, U32 num )
{
   AssertFatal( SFXInternal::isSFXThread(), "SFXXAudioBuffer::write() - not on SFX thread" );

   using namespace SFXInternal;

   // Unqueue processed packets.

   if( isStreaming() )
   {
      EnterCriticalSection( &_getUniqueVoice()->mLock );

      XAUDIO2_VOICE_STATE state;
      _getUniqueVoice()->mXAudioVoice->GetState( &state );

      U32 numProcessed = mBufferQueue.size() - state.BuffersQueued;
      for( U32 i = 0; i < numProcessed; ++ i )
      {
         destructSingle< SFXStreamPacket* >( mBufferQueue.first().mPacket );
         mBufferQueue.pop_front();

         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[SFXXAudioBuffer] Unqueued packet" );
         #endif
      }

      LeaveCriticalSection( &_getUniqueVoice()->mLock );
   }

   // Queue new packets.

   for( U32 i = 0; i < num; ++ i )
   {
      SFXStreamPacket* packet = packets[ i ];
      Buffer buffer;

      if( packet->mIsLast )
         buffer.mData.Flags = XAUDIO2_END_OF_STREAM;

      buffer.mPacket = packet;
      buffer.mData.AudioBytes = packet->mSizeActual;
      buffer.mData.pAudioData = packet->data;

      mBufferQueue.push_back( buffer );

      #ifdef DEBUG_SPEW
      Platform::outputDebugString( "[SFXXAudioBuffer] Queued packet" );
      #endif

      // If this is a streaming buffer, submit the packet to the
      // voice queue right away.

      if( isStreaming() )
      {
         EnterCriticalSection( &_getUniqueVoice()->mLock );

         IXAudio2SourceVoice* voice = _getUniqueVoice()->mXAudioVoice;
         voice->SubmitSourceBuffer( &buffer.mData );

         LeaveCriticalSection( &_getUniqueVoice()->mLock );
      }
   }
}

void SFXXAudioBuffer::_flush()
{
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXXAudioBuffer] Flushing buffer" );
   #endif

   if( _getUniqueVoice() )
      _getUniqueVoice()->_stop();

   while( !mBufferQueue.empty() )
   {
      destructSingle( mBufferQueue.last().mPacket );
      mBufferQueue.pop_back();
   }
}
