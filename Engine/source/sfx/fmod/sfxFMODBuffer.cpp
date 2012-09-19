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

#include "sfx/fmod/sfxFMODBuffer.h"
#include "sfx/fmod/sfxFMODDevice.h"
#include "sfx/sfxDescription.h"
#include "core/util/safeDelete.h"
#include "core/volume.h"


//-----------------------------------------------------------------------------

static const char* sExtensions[] =
{
   "",      // First try without doing anything with the given path.
   "",      // Then try it without an extension but by expanding it through Torque::FS.
   "aiff",
   "asf",
   "asx",
   "dls",
   "flac",
   "fsb",
   "it",
   "m3u",
   "mid",
   "mod",
   "mp2",
   "mp3",
   "ogg",
   "pls",
   "s3m",
   "vag",
   "wav",
   "wax",
   "wma",
   "xm",
   
#ifdef TORQUE_OS_XENON
   ".xma",
#endif

   NULL
};

//-----------------------------------------------------------------------------

SFXFMODBuffer* SFXFMODBuffer::create( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description )
{
   SFXFMODBuffer *buffer = new SFXFMODBuffer( stream, description );
   if( !buffer->mSound )
      SAFE_DELETE( buffer );

   return buffer;
}

//-----------------------------------------------------------------------------

SFXFMODBuffer* SFXFMODBuffer::create( const String& filename, SFXDescription* description )
{
   if( Con::getBoolVariable( "$pref::SFX::FMOD::noCustomFileLoading", false ) )
      return NULL;
      
   SFXFMODBuffer *buffer = new SFXFMODBuffer( filename, description );
   if( !buffer->mSound )
      SAFE_DELETE( buffer );

   return buffer;
}

//-----------------------------------------------------------------------------

SFXFMODBuffer::SFXFMODBuffer( const String& filename, SFXDescription* description )
   : Parent( description ),
     mSound( NULL )
{
   FMOD_MODE fMode = ( description->mUseHardware ? FMOD_HARDWARE : FMOD_SOFTWARE )
      | ( description->mIs3D ? FMOD_3D : FMOD_2D );

   if( description->mIsStreaming )
   {
      fMode |= FMOD_CREATESTREAM;
      mIsUnique = true;
   }
   
   // Go through the extensions and try each with the given path.  The
   // first two are special.  First we try without touching the filename at all
   // so FMOD gets a chance to handle URLs and whatever, and then second we
   // try by expanding the path but without adding an extension.
   
   Torque::Path path = filename;
   for( U32 i = 0; sExtensions[ i ]; ++ i )
   {
      path.setExtension( sExtensions[ i ] );

      if( !i || Torque::FS::IsFile( path ) )
      {
         // Translate to full path.
         //TODO: Remove this when hooking up the file system functions in sfxFMODDevice.cpp

         String fullPath;
         if( !i )
            fullPath = filename;
         else
         {
            Torque::Path realPath;
            if( !Torque::FS::GetFSPath( path, realPath ) )
               continue;
            
            fullPath = realPath.getFullPath().c_str();
         }
      
         mSound = NULL;
         FMOD_RESULT result = SFXFMODDevice::smFunc->FMOD_System_CreateSound(
               SFXFMODDevice::smSystem, 
               fullPath.c_str(), 
               fMode, 
               ( FMOD_CREATESOUNDEXINFO* ) NULL, 
               &mSound );
               
         if( result == FMOD_OK )
         {
            SFXFMODDevice::smFunc->FMOD_Sound_GetMode( mSound, &mMode );
            
            // Read out format.
            
            int numChannels;
            int bitsPerSample;
            unsigned int length;
            float frequency;
            
            SFXFMODDevice::smFunc->FMOD_Sound_GetFormat( mSound, ( FMOD_SOUND_TYPE* ) NULL, ( FMOD_SOUND_FORMAT* ) NULL, &numChannels, &bitsPerSample );
            SFXFMODDevice::smFunc->FMOD_Sound_GetLength( mSound, &length, FMOD_TIMEUNIT_MS );
            SFXFMODDevice::smFunc->FMOD_Sound_GetDefaults( mSound, &frequency, ( float* ) NULL, ( float* ) NULL, ( int* ) NULL );

            mDuration = length;
            mFormat = SFXFormat( numChannels, numChannels * bitsPerSample, frequency );
            
            break;
         }
      }
   }

   if( !mSound )
      Con::errorf( "SFXFMODBuffer::SFXFMODBuffer - failed to load '%s' through FMOD", filename.c_str() );
}

//-----------------------------------------------------------------------------

SFXFMODBuffer::SFXFMODBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description )
   : Parent( stream, description ),
     mSound( NULL )
{
   FMOD_MODE fMode = ( description->mUseHardware ? FMOD_HARDWARE : FMOD_SOFTWARE )
      | ( description->mIs3D ? FMOD_3D : FMOD_2D );

   FMOD_CREATESOUNDEXINFO* pCreatesoundexinfo = NULL;
   FMOD_CREATESOUNDEXINFO  createsoundexinfo;

   fMode |= FMOD_OPENUSER; // this tells fmod we are supplying the data directly
   if( isStreaming() )
      fMode |= FMOD_LOOP_NORMAL | FMOD_UNIQUE;

   const SFXFormat& format = getFormat();
   U32 channels = format.getChannels();
   U32 frequency = format.getSamplesPerSecond();
   U32 bitsPerChannel = format.getBitsPerSample() / channels;
   U32 dataSize = mBufferSize;

   FMOD_SOUND_FORMAT sfxFmt = FMOD_SOUND_FORMAT_NONE;
   switch(bitsPerChannel)
   {
      case 8:
         sfxFmt = FMOD_SOUND_FORMAT_PCM8;
         break;
      case 16:
         sfxFmt = FMOD_SOUND_FORMAT_PCM16;
         break;
      case 24:
         sfxFmt = FMOD_SOUND_FORMAT_PCM24;
         break;
      case 32:
         sfxFmt = FMOD_SOUND_FORMAT_PCM32;
         break;
      default:
         AssertISV(false, "SFXFMODBuffer::SFXFMODBuffer() - unsupported bits-per-sample (what format is it in, 15bit PCM?)");
         break;
   }

   dMemset(&createsoundexinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
   createsoundexinfo.cbsize            = sizeof(FMOD_CREATESOUNDEXINFO); /* required. */
   createsoundexinfo.decodebuffersize  = frequency; /* Chunk size of stream update in samples.  This will be the amount of data passed to the user callback. */
   createsoundexinfo.length            = dataSize; /* Length of PCM data in bytes of whole sound (for Sound::getLength) */
   createsoundexinfo.numchannels       = channels; /* Number of channels in the sound. */
   createsoundexinfo.defaultfrequency  = frequency; /* Default playback rate of sound. */
   createsoundexinfo.format            = sfxFmt;  /* Data format of sound. */
   createsoundexinfo.pcmreadcallback   = NULL; /* User callback for reading. */
   createsoundexinfo.pcmsetposcallback = NULL; /* User callback for seeking. */
   pCreatesoundexinfo = &createsoundexinfo;

   FMOD_RESULT result = SFXFMODDevice::smFunc->FMOD_System_CreateSound(
         SFXFMODDevice::smSystem, 
         ( const char* ) NULL, 
         fMode, 
         pCreatesoundexinfo, 
         &mSound );
         
   if( result != FMOD_OK )
   {
      mSound = NULL;
      Con::errorf( "SFXFMODBuffer::SFXFMODBuffer - failed to create buffer (%i)", result );
   }
   else
      SFXFMODDevice::smFunc->FMOD_Sound_GetMode( mSound, &mMode );
}

//-----------------------------------------------------------------------------

SFXFMODBuffer::~SFXFMODBuffer()
{
   if( mSound )
      FModAssert( SFXFMODDevice::smFunc->FMOD_Sound_Release( mSound ), 
         "SFXFMODBuffer::~SFXFMODBuffer - Failed to release a sound!" );

   mSound = NULL;
}

//-----------------------------------------------------------------------------

void SFXFMODBuffer::_flush()
{
   AssertFatal( isStreaming(), "SFXFMODBuffer::_flush() - not a streaming buffer" );
   AssertFatal( SFXInternal::isSFXThread(), "SFXFMODBuffer::_flush() - not on SFX thread" );

   Parent::_flush();
   SFXFMODDevice::smFunc->FMOD_Channel_SetPosition
      ( ( ( SFXFMODVoice* ) mUniqueVoice.getPointer() )->mChannel, 0, FMOD_TIMEUNIT_PCM );
}

//-----------------------------------------------------------------------------

bool SFXFMODBuffer::_copyData( U32 offset, const U8* data, U32 length )
{
   AssertFatal( data != NULL && length > 0, "Must have data!" );

   // Fill the buffer with the resource data.      
   void* lpvWrite;
   U32  dwLength;
   void* lpvWrite2;
   U32  dwLength2;
   int res = SFXFMODDevice::smFunc->FMOD_Sound_Lock(
      mSound,
      offset,           // Offset at which to start lock.
      length,           // Size of lock.
      &lpvWrite,        // Gets address of first part of lock.
      &lpvWrite2,       // Address of wraparound not needed. 
      &dwLength,        // Gets size of first part of lock.
      &dwLength2       // Size of wraparound not needed.
      );

   if ( res != FMOD_OK )
   {
      // You can remove this if it gets spammy. However since we can
      // safely fail in this case it doesn't seem right to assert...
      // at the same time it can be very annoying not to know why 
      // an upload fails!
      Con::errorf("SFXFMODBuffer::_copyData - failed to lock a sound buffer! (%d)", this);
      return false;
   }

   // Copy the first part.
   dMemcpy( lpvWrite, data, dwLength );

   // Do we have a wrap?
   if ( lpvWrite2 )
      dMemcpy( lpvWrite2, data + dwLength, dwLength2 );

   // And finally, unlock.
   FModAssert( SFXFMODDevice::smFunc->FMOD_Sound_Unlock(
      mSound,
      lpvWrite,      // Address of lock start.
      lpvWrite2,     // No wraparound portion.
      dwLength,      // Size of lock.
      dwLength2 ),   // No wraparound size.
      "Failed to unlock sound buffer!" );

   return true;
}

//-----------------------------------------------------------------------------

U32 SFXFMODBuffer::getMemoryUsed() const
{
   unsigned int memoryUsed;
   
   SFXFMODDevice::smFunc->FMOD_Sound_GetMemoryInfo(
      mSound,
      FMOD_MEMBITS_ALL,
      FMOD_EVENT_MEMBITS_ALL,
      &memoryUsed,
      ( unsigned int* ) NULL );
      
   return memoryUsed;
}
