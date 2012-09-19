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

#include "sfx/dsound/sfxDSBuffer.h"
#include "sfx/sfxStream.h"
#include "sfx/sfxDescription.h"
#include "sfx/sfxInternal.h"
#include "platform/async/asyncUpdate.h"
#include "core/util/safeRelease.h"
#include "core/util/safeCast.h"


SFXDSBuffer* SFXDSBuffer::create(   IDirectSound8 *dsound, 
                                    const ThreadSafeRef< SFXStream >& stream,
                                    SFXDescription* description,
                                    bool useHardware )
{
   AssertFatal( dsound, "SFXDSBuffer::create() - Got null dsound!" );
   AssertFatal( stream, "SFXDSBuffer::create() - Got a null stream!" );
   AssertFatal( description, "SFXDSBuffer::create() - Got a null description" );

   SFXDSBuffer* buffer = new SFXDSBuffer( dsound,
                                          stream,
                                          description,
                                          useHardware );


   if( !buffer->_createBuffer( &buffer->mBuffer ) )
      SAFE_DELETE( buffer );

   return buffer;
}

SFXDSBuffer::SFXDSBuffer(  IDirectSound8* dsound,
                           const ThreadSafeRef< SFXStream >& stream,
                           SFXDescription* description,
                           bool useHardware )
   :  Parent( stream, description ),
      mDSound( dsound ),
      mIs3d( description->mIs3D ),
      mUseHardware( useHardware ),
      mBuffer( NULL ),
      mDuplicate( false )
{
   AssertFatal( mDSound, "SFXDSBuffer::SFXDSBuffer() - Got null dsound!" );

   mDSound->AddRef();
}

SFXDSBuffer::~SFXDSBuffer()
{
   SAFE_RELEASE( mBuffer );
   SAFE_RELEASE( mDSound );
}

bool SFXDSBuffer::_createBuffer( IDirectSoundBuffer8 **buffer8 )
{
   AssertFatal( mAsyncState != NULL,
      "SFXDSBuffer::_createBuffer() - Can't create buffer when not connected to stream!" );

   const SFXFormat& format = getFormat();

   // Set up WAV format structure. 
   WAVEFORMATEX wfx;
   dMemset( &wfx, 0, sizeof( WAVEFORMATEX ) ); 
   wfx.wFormatTag = WAVE_FORMAT_PCM; 
   wfx.nChannels = format.getChannels();
   wfx.nSamplesPerSec = format.getSamplesPerSecond();
   wfx.wBitsPerSample = format.getBitsPerChannel();
   wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
   wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign; 

   // Set up DSBUFFERDESC structure. 
   DSBUFFERDESC dsbdesc; 
   dMemset( &dsbdesc, 0, sizeof( DSBUFFERDESC ) ); 
   dsbdesc.dwSize = sizeof( DSBUFFERDESC ); 
   dsbdesc.dwFlags = 
      ( mIs3d ? DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE : DSBCAPS_CTRLPAN  ) |
      ( isStreaming() ? DSBCAPS_CTRLPOSITIONNOTIFY : 0 ) |
      DSBCAPS_CTRLFREQUENCY |
      DSBCAPS_CTRLVOLUME |
      DSBCAPS_GETCURRENTPOSITION2 |
      DSBCAPS_GLOBALFOCUS |
      DSBCAPS_STATIC |
      ( mUseHardware ? DSBCAPS_LOCHARDWARE : DSBCAPS_LOCSOFTWARE );
   dsbdesc.dwBufferBytes = mBufferSize;
   if ( mIs3d )
      dsbdesc.guid3DAlgorithm = DS3DALG_HRTF_FULL;
   dsbdesc.lpwfxFormat = &wfx;

   // Create the buffer.
   IDirectSoundBuffer *buffer = NULL;
   HRESULT hr = mDSound->CreateSoundBuffer( &dsbdesc, &buffer, NULL );
   if ( FAILED( hr ) || !buffer )
      return false;

   // Grab the version 8 interface.
   IDirectSoundBuffer8* buffer8Ptr;
   hr = buffer->QueryInterface( IID_IDirectSoundBuffer8, ( LPVOID* ) &buffer8Ptr );

   // Release the original interface.
   buffer->Release();

   // If we failed to get the 8 interface... exit.
   if ( FAILED( hr ) || !buffer8Ptr )
      return false;

   // Set up notification positions, if this is a streaming buffer.

   if( isStreaming() )
   {
      using namespace SFXInternal;

      const U32 maxQueuedPackets = SFXAsyncQueue::DEFAULT_STREAM_QUEUE_LENGTH;
      const U32 packetSize = mAsyncState->mStream->getPacketSize();

      LPDIRECTSOUNDNOTIFY8 lpDsNotify;
      if( FAILED( hr = buffer8Ptr->QueryInterface( IID_IDirectSoundNotify8, ( LPVOID* ) &lpDsNotify ) ) )
      {
         SAFE_RELEASE( buffer8Ptr );
         return false;
      }

      const U32 numNotifies = maxQueuedPackets + 1; // Add one for end-of-stream notification pos.
      DSBPOSITIONNOTIFY* dsbNotifyPos =
         ( DSBPOSITIONNOTIFY* ) _alloca( numNotifies * sizeof( DSBPOSITIONNOTIFY ) );

      // Events seem to be triggered way too early by DS causing the playback queues to
      // reject updates, so we nudge the update markers "somewhat" to the right here.
      // This value here is based on experimentation.  No harm should result if we don't
      // hit it other than updates happening in sub-optimal timing.
      enum { OFFSET_DELTA = 5000 };

      U32 offset = ( packetSize + OFFSET_DELTA ) % mBufferSize;
      HANDLE updateEvent = ( HANDLE ) UPDATE_THREAD()->getUpdateEvent();
      for( U32 i = 0; i < maxQueuedPackets; ++ i, offset = ( offset + packetSize ) % mBufferSize )
      {
         dsbNotifyPos[ i ].dwOffset      = offset;
         dsbNotifyPos[ i ].hEventNotify  = updateEvent;
      }

      // A end-of-stream notification position.

      //FIXME: this position will start to be wrong when doing stream seeks

      dsbNotifyPos[ numNotifies - 1 ].dwOffset = ( format.getDataLength( getDuration() ) + OFFSET_DELTA ) % mBufferSize;
      dsbNotifyPos[ numNotifies - 1 ].hEventNotify = updateEvent;

      // Install the notifications.

      lpDsNotify->SetNotificationPositions( numNotifies, dsbNotifyPos );
      SAFE_RELEASE( lpDsNotify );

      // Don't need to notify on stop as when playback is stopped,
      // the packet buffers will just fill up and stop updating
      // when saturated.
   }

   *buffer8 = buffer8Ptr;
   return true;
}

bool SFXDSBuffer::_copyData(  U32 offset, 
                              const U8 *data,
                              U32 length )
{
   AssertFatal( mBuffer, "SFXDSBuffer::_copyData() - no buffer" );

   // Fill the buffer with the resource data.      
   VOID* lpvWrite;
   DWORD  dwLength;
   VOID* lpvWrite2;
   DWORD  dwLength2;
   HRESULT hr = mBuffer->Lock(
         offset,           // Offset at which to start lock.
         length,           // Size of lock.
         &lpvWrite,        // Gets address of first part of lock.
         &dwLength,        // Gets size of first part of lock.
         &lpvWrite2,       // Address of wraparound not needed. 
         &dwLength2,       // Size of wraparound not needed.
         0 );
   if ( FAILED( hr ) )
      return false;

   // Copy the first part.
   dMemcpy( lpvWrite, data, dwLength );

   // Do we have a wrap?
   if ( lpvWrite2 )
      dMemcpy( lpvWrite2, data + dwLength, dwLength2 );

   // And finally, unlock.
   hr = mBuffer->Unlock(
         lpvWrite,      // Address of lock start.
         dwLength,      // Size of lock.
         lpvWrite2,     // No wraparound portion.
         dwLength2 );   // No wraparound size.

   // Return success code.
   return SUCCEEDED(hr);
}

void SFXDSBuffer::_flush()
{
   AssertFatal( isStreaming(), "SFXDSBuffer::_flush() - not a streaming buffer" );
   AssertFatal( SFXInternal::isSFXThread(), "SFXDSBuffer::_flush() - not on SFX thread" );

   Parent::_flush();
   mBuffer->SetCurrentPosition( 0 );
}

bool SFXDSBuffer::_duplicateBuffer( IDirectSoundBuffer8 **buffer8 )
{
   AssertFatal( mBuffer, "SFXDSBuffer::_duplicateBuffer() - Duplicate buffer is null!" );

   // If this is the first duplicate then
   // give the caller our original buffer.
   if ( !mDuplicate )
   {
      mDuplicate = true;

      *buffer8 = mBuffer;
      (*buffer8)->AddRef();

      return true;
   }

   IDirectSoundBuffer *buffer1 = NULL;
   HRESULT hr = mDSound->DuplicateSoundBuffer( mBuffer, &buffer1 );
   if ( FAILED( hr ) || !buffer1 )
      return false;

   // Grab the version 8 interface.
   hr = buffer1->QueryInterface( IID_IDirectSoundBuffer8, (LPVOID*)buffer8 );

   // Release the original interface.
   buffer1->Release();

   return SUCCEEDED( hr ) && (*buffer8);
}

bool SFXDSBuffer::createVoice( IDirectSoundBuffer8 **buffer8 )
{
   return ( mBuffer && _duplicateBuffer( buffer8 ) && *buffer8 );
}

void SFXDSBuffer::releaseVoice( IDirectSoundBuffer8 **buffer )
{
   AssertFatal( *buffer, "SFXDSBuffer::releaseVoice() - Got null buffer!" );

   if ( *buffer == mBuffer )
   {
      mDuplicate = false;
      (*buffer)->Stop();
   }

   SAFE_RELEASE( (*buffer) );
}
