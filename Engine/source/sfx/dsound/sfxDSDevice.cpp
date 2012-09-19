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

#include "sfx/dsound/sfxDSDevice.h"
#include "sfx/dsound/sfxDSBuffer.h"
#include "sfx/dsound/sfxDSVoice.h"
#include "platformWin32/platformWin32.h"
#include "core/util/safeRelease.h"
#include "platform/async/asyncUpdate.h"
#include "console/console.h"


SFXDSDevice::SFXDSDevice(  SFXProvider* provider, 
                           DSoundFNTable *dsFnTbl,
                           GUID* guid, 
                           String name,
                           bool useHardware,
                           S32 maxBuffers )
   :  SFXDevice( name, provider, useHardware, maxBuffers ),
      mDSound( NULL ),
      mPrimaryBuffer( NULL ),
      mListener( NULL ),
      mDSoundTbl( dsFnTbl ),
      mGUID( guid )
{
}

bool SFXDSDevice::_init()
{
   HRESULT hr = mDSoundTbl->DirectSoundCreate8( mGUID, &mDSound, NULL );   
   if ( FAILED( hr ) || !mDSound )
   {
      Con::errorf( "SFXDSDevice::SFXDSDevice() - DirectSoundCreate8 failed" );
      return false;
   }

   hr = mDSound->SetCooperativeLevel( getWin32WindowHandle(), DSSCL_PRIORITY );
   if ( FAILED( hr ) )
   {
      Con::errorf( "SFXDSDevice::SFXDSDevice() - SetCooperativeLevel failed" );
      return false;
   }

   // Get the primary buffer.
   DSBUFFERDESC dsbd;   
   dMemset( &dsbd, 0, sizeof( DSBUFFERDESC ) ); 
   dsbd.dwSize = sizeof( DSBUFFERDESC );
   dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
   hr = mDSound->CreateSoundBuffer( &dsbd, &mPrimaryBuffer, NULL );
   if ( FAILED( hr ) )
   {
      Con::errorf( "SFXDSDevice::SFXDSDevice - Creating primary sound buffer failed" );
      return false;
   }

   // Set the format and bitrate on the primary buffer.
   S32 frequency = Con::getIntVariable( "$pref::SFX::frequency", 44100 );
   S32 bitrate = Con::getIntVariable( "$pref::SFX::bitrate", 32 );

   WAVEFORMATEX wfx;
   dMemset( &wfx, 0, sizeof( WAVEFORMATEX ) ); 
   wfx.wFormatTag = WAVE_FORMAT_PCM; 
   wfx.nChannels = 2; 
   wfx.nSamplesPerSec = frequency;
   wfx.wBitsPerSample = bitrate; 
   wfx.nBlockAlign = ( wfx.nChannels * wfx.wBitsPerSample ) / 8;
   wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign; 
   hr = mPrimaryBuffer->SetFormat( &wfx );
   if( FAILED( hr ) )
   {
      Con::errorf( "SFXDSDevice::SFXDSDevice() - Setting format of primary buffer failed" );
      return false;
   }

   // Grab the 3d listener.
   hr = mPrimaryBuffer->QueryInterface( IID_IDirectSound3DListener8, (LPVOID*)&mListener );
   if ( FAILED( hr ) )
   {
      Con::errorf( "SFXDSDevice::SFXDevice() - Querying the listener interface failed!" );
      mListener = NULL;
   }

   mCaps.dwSize = sizeof( DSCAPS );
   mDSound->GetCaps( &mCaps );

   // If the device reports no hardware buffers then
   // we have no choice but to disable hardware.
   if ( mCaps.dwMaxHw3DAllBuffers == 0 )
      mUseHardware = false;

   // If mMaxBuffers is negative then use the caps
   // to decide on a good maximum value... or set 8.
   if ( mMaxBuffers < 0 )
      mMaxBuffers = getMax( mCaps.dwMaxHw3DAllBuffers, 8 );

   // Start the stream thread.

   if( !Con::getBoolVariable( "$_forceAllMainThread" ) )
   {
      SFXInternal::gUpdateThread =
         new AsyncUpdateThread( "DirectSound Update Thread", SFXInternal::gBufferUpdateList );
      SFXInternal::gUpdateThread->start();
   }

   return true;
}

SFXDSDevice::~SFXDSDevice()
{
   // And release our resources.
   SAFE_RELEASE( mListener );
   SAFE_RELEASE( mPrimaryBuffer );
   SAFE_RELEASE( mDSound );
}

SFXBuffer* SFXDSDevice::createBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description )
{
   AssertFatal( stream, "SFXDSDevice::createBuffer() - Got null stream!" );
   AssertFatal( description, "SFXDSDevice::createBuffer() - Got null description!" );

   SFXDSBuffer* buffer = SFXDSBuffer::create(   mDSound, 
                                                stream,
                                                description, 
                                                mUseHardware );

   if( buffer )
      _addBuffer( buffer );

   return buffer;
}

SFXVoice* SFXDSDevice::createVoice( bool is3D, SFXBuffer *buffer )
{
   // Don't bother going any further if we've 
   // exceeded the maximum voices.
   if ( mVoices.size() >= mMaxBuffers )
      return NULL;

   AssertFatal( buffer, "SFXDSDevice::createVoice() - Got null buffer!" );

   SFXDSBuffer* dsBuffer = dynamic_cast<SFXDSBuffer*>( buffer );
   AssertFatal( dsBuffer, "SFXDSDevice::createVoice() - Got bad buffer!" );

   SFXDSVoice* voice = SFXDSVoice::create( this, dsBuffer );
   if ( !voice )
      return NULL;

   _addVoice( voice );
	return voice;
}

void SFXDSDevice::_commitDeferred()
{
   if( mListener )
      mListener->CommitDeferredSettings();
}

void SFXDSDevice::update()
{
   Parent::update();

   // Apply the deferred settings that changed between updates.
   mListener->CommitDeferredSettings();
}

void SFXDSDevice::setListener( U32 index, const SFXListenerProperties& listener )
{
   // Get the transform from the listener.
   const MatrixF& transform = listener.getTransform();
   Point3F pos, dir, up;
   transform.getColumn( 3, &pos );
   transform.getColumn( 1, &dir );
   transform.getColumn( 2, &up );

   // And the velocity...
   const VectorF& velocity = listener.getVelocity();

   // Finally, set it all to DSound!
   mListener->SetPosition( pos.x, pos.z, pos.y, DS3D_DEFERRED );
   mListener->SetOrientation( dir.x, dir.z, dir.y, up.x, up.z, up.y, DS3D_DEFERRED );
   mListener->SetVelocity( velocity.x, velocity.z, velocity.y, DS3D_DEFERRED );
}

void SFXDSDevice::setDistanceModel( SFXDistanceModel model )
{
   switch( model )
   {
   case SFXDistanceModelLinear:
      Con::errorf( "SFXDSDevice::setDistanceModel - 'linear' distance attenuation not supported by DirectSound" );
      break;

   case SFXDistanceModelLogarithmic:
      break; // Nothing to do.

   default:
      AssertWarn( false, "SFXDSDevice::setDistanceModel() - model not implemented" );
   }
}

void SFXDSDevice::setDopplerFactor( F32 factor )
{
   if( mListener )
      mListener->SetDopplerFactor( factor, DS3D_DEFERRED ); // Committed in update.
}

void SFXDSDevice::setRolloffFactor( F32 factor )
{
   if( mListener )
      mListener->SetRolloffFactor( factor, DS3D_DEFERRED ); // Committed in update.
}
