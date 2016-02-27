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

#include "sfx/sfxDevice.h"
#include "sfx/sfxBuffer.h"
#include "sfx/sfxVoice.h"
#include "sfx/sfxInternal.h"
#include "core/tAlgorithm.h"
#include "console/console.h"
#include "console/consoleTypes.h"


//-----------------------------------------------------------------------------

SFXDevice::SFXDevice( const String& name, SFXProvider* provider, bool useHardware, S32 maxBuffers )
   :  mName( name ),
      mProvider( provider ),
      mUseHardware( useHardware ),
      mMaxBuffers( maxBuffers ),
      mCaps( 0 ),
      mStatNumBuffers( 0 ),
      mStatNumVoices( 0 ),
      mStatNumBufferBytes( 0 )
{
   AssertFatal( provider, "We must have a provider pointer on device creation!" );

   VECTOR_SET_ASSOCIATION( mBuffers );
   VECTOR_SET_ASSOCIATION( mVoices );

   SFXBuffer::smBufferDestroyedSignal.notify( this, &SFXDevice::_removeBuffer );
   SFXVoice::smVoiceDestroyedSignal.notify( this, &SFXDevice::_removeVoice );

   Con::addVariable( "SFX::Device::numBuffers", TypeS32, &mStatNumBuffers );
   Con::addVariable( "SFX::Device::numVoices", TypeS32, &mStatNumVoices );
   Con::addVariable( "SFX::Device::numBufferBytes", TypeS32, &mStatNumBufferBytes );
}

//-----------------------------------------------------------------------------

SFXDevice::~SFXDevice()
{
   Con::removeVariable( "SFX::Device::numBuffers" );
   Con::removeVariable( "SFX::Device::numVoices" );
   Con::removeVariable( "SFX::Device::numBufferBytes" );

   _releaseAllResources();
}

//-----------------------------------------------------------------------------

void SFXDevice::_releaseAllResources()
{
   using namespace SFXInternal;

   // Kill the update thread, if there is one.
   // Do this first so that further buffer processing
   // can be done synchronously by us.

   ThreadSafeRef< SFXUpdateThread > sfxThread = UPDATE_THREAD();
   if( sfxThread != NULL )
   {
      gUpdateThread = NULL; // Kill the global reference.

      sfxThread->stop();
      sfxThread->triggerUpdate();
      sfxThread->join();

      sfxThread = NULL;
   }

   // Clean up voices.  Do this before cleaning up buffers so that
   // resources held by voices that are tied to resources held by buffers
   // get released properly.

   SFXVoice::smVoiceDestroyedSignal.remove( this, &SFXDevice::_removeVoice );
   for( VoiceIterator voice = mVoices.begin();
        voice != mVoices.end(); voice++ )
      ( *voice )->destroySelf();
   mVoices.clear();

   // Clean up buffers.

   SFXBuffer::smBufferDestroyedSignal.remove( this, &SFXDevice::_removeBuffer );
   for( BufferIterator buffer = mBuffers.begin();
        buffer != mBuffers.end(); ++ buffer )
      if( !( *buffer )->isDead() )
         ( *buffer )->destroySelf();
   mBuffers.clear();

   // Flush all asynchronous requests.

   THREAD_POOL().flushWorkItems();

   // Clean out the buffer update list and kill
   // all buffers that surfaced on the dead list.
   // Now the sound buffers are really gone.

   UPDATE_LIST().process();
   PurgeDeadBuffers();

   // Clean out stats.

   mStatNumBuffers = 0;
   mStatNumVoices = 0;
   mStatNumBufferBytes = 0;
}

//-----------------------------------------------------------------------------

void SFXDevice::update()
{
   using namespace SFXInternal;

   // If we don't have an update thread, do the
   // updates now on the main thread.

   if( !UPDATE_THREAD() )
      UPDATE_LIST().process( MAIN_THREAD_PROCESS_TIMEOUT );
      
   // Clean out buffers that have surfaced on the dead
   // buffer list.

   PurgeDeadBuffers();
}

//-----------------------------------------------------------------------------

void SFXDevice::_addBuffer( SFXBuffer* buffer )
{
   AssertFatal( buffer, "SFXDevice::_addBuffer() - Got a null buffer!" );
   
   // Register the buffer.
   
   mBuffers.push_back( buffer );
   mStatNumBuffers ++;
   mStatNumBufferBytes += buffer->getMemoryUsed();
   
   // Start loading the buffer.

   buffer->load();
}

//-----------------------------------------------------------------------------

void SFXDevice::_removeBuffer( SFXBuffer* buffer )
{
   AssertFatal( buffer, "SFXDevice::_removeBuffer() - Got a null buffer!" );

   BufferIterator iter = find( mBuffers.begin(), mBuffers.end(), buffer );
   if( iter != mBuffers.end() )
   {
      SFXBuffer* buffer = *iter;

      mStatNumBufferBytes -= buffer->getMemoryUsed();
      mStatNumBuffers --;
      
      mBuffers.erase( iter );
   }
}

//-----------------------------------------------------------------------------

void SFXDevice::_addVoice( SFXVoice* voice )
{
   AssertFatal( voice, "SFXDevice::_addVoice() - Got a null voice!" );
   using namespace SFXInternal;

   // Bind the voice to its buffer.  This is deferred up to here in order
   // to only bind voices that have been successfully constructed.

   voice->_attachToBuffer();
   
   // Register the voice.

   mVoices.push_back( voice );
   mStatNumVoices ++;
}

//-----------------------------------------------------------------------------

void SFXDevice::_removeVoice( SFXVoice* voice )
{
   AssertFatal( voice, "SFXDevice::_removeVoice() - Got null voice!" );

   VoiceIterator iter = find( mVoices.begin(), mVoices.end(), voice );
   if( iter != mVoices.end() )
   {
      mStatNumVoices --;
      mVoices.erase( iter );
   }
}
