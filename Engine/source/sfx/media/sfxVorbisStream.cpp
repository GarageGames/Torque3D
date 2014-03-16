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

#ifdef TORQUE_OGGVORBIS


#include "sfx/media/sfxVorbisStream.h"
#include "core/stream/stream.h"
#include "console/console.h"


SFXVorbisStream* SFXVorbisStream::create( Stream *stream )
{
   SFXVorbisStream *sfxStream = new SFXVorbisStream();
   if ( sfxStream->open( stream, true ) )
      return sfxStream;

   delete sfxStream;
   return NULL;
}

SFXVorbisStream::SFXVorbisStream()
   : mVF( NULL ),
     mBytesRead( 0 )
{
}

SFXVorbisStream::SFXVorbisStream( const SFXVorbisStream& cloneFrom )
   : Parent( cloneFrom )
{
   if( !mStream->hasCapability( Stream::StreamPosition ) )
   {
      Con::errorf( "SFXVorbisStream::SFXVorbisStream() - Source stream does not allow seeking" );
      return;
   }
   
   mStream->setPosition( 0 );
   if( !_readHeader() )
   {
      Con::errorf( "SFXVorbisStream::SFXVorbisStream() - Opening Vorbis stream failed" );
      return;
   }

   ov_pcm_seek( mVF, ov_pcm_tell( cloneFrom.mVF ) );
   
   mBitstream = cloneFrom.mBitstream;
   mBytesRead = cloneFrom.mBytesRead;
}

SFXVorbisStream::~SFXVorbisStream()
{
   // We must call close from our own destructor
   // and not the base class... as it causes a
   // pure virtual runtime assertion.
   close();
}

size_t SFXVorbisStream::_read_func( void *ptr, size_t size, size_t nmemb, void *datasource )
{
   Stream *stream = reinterpret_cast<Stream*>( datasource );

   // Stream::read() returns true if any data was
   // read, so we must track the read bytes ourselves.
   U32 startByte = stream->getPosition();
   stream->read( size * nmemb, ptr );
   U32 endByte = stream->getPosition();

   // How many did we actually read?
   U32 readBytes = ( endByte - startByte );
   U32 readItems = readBytes / size;

   return readItems;
}

S32 SFXVorbisStream::_seek_func( void *datasource, ogg_int64_t offset, S32 whence )
{
   Stream *stream = reinterpret_cast<Stream*>( datasource );

   U32 newPos = 0; 
   if ( whence == SEEK_CUR )
      newPos = stream->getPosition() + (U32)offset;
   else if ( whence == SEEK_END )
      newPos = stream->getStreamSize() - (U32)offset;
   else
      newPos = (U32)offset;

   return stream->setPosition( newPos ) ? 0 : -1;
}

long SFXVorbisStream::_tell_func( void *datasource )
{
   Stream *stream = reinterpret_cast<Stream*>( datasource );
   return stream->getPosition();
}

bool SFXVorbisStream::_openVorbis()
{
#if defined(TORQUE_OS_XENON)
   // For some reason the datasource pointer passed to the callbacks is not the
   // same as it is when passed in to ov_open_callbacks
#pragma message("There is a strange bug in ov_open_callbacks as it compiles on the Xbox360. Use FMOD resource loading.")
   AssertWarn(false, "There is a strange bug in ov_open_callbacks as it compiles on the Xbox360. Use FMOD resource loading.");
   return false;
#endif
   mVF = new OggVorbis_File;
   dMemset( mVF, 0, sizeof( OggVorbis_File ) );

   const bool canSeek = mStream->hasCapability( Stream::StreamPosition );

   ov_callbacks cb;
   cb.read_func = _read_func;
   cb.seek_func = canSeek ? _seek_func : NULL;
   cb.close_func = NULL;
   cb.tell_func = canSeek ? _tell_func : NULL;

   // Open it.
   S32 ovResult = ov_open_callbacks( mStream, mVF, NULL, 0, cb );
   if( ovResult != 0 )
      return false;

   return true;
}

bool SFXVorbisStream::_readHeader()
{
   if( !_openVorbis() )
      return false;

   // Fill in the format info.
   const vorbis_info *vi = getInfo();
   mFormat.set( vi->channels, vi->channels * 16, vi->rate );

   // Set the sample count.
   mSamples = getPcmTotal();

   // Reset the bitstream.
   mBitstream = 0;

   return true;
}

void SFXVorbisStream::_close()
{
   if ( !mVF )
      return;

   ov_clear( mVF );
   delete mVF;
   mVF = NULL;
   mBitstream = -1;
}

const vorbis_info* SFXVorbisStream::getInfo( S32 link )
{
   AssertFatal( mVF, "SFXVorbisStream::getInfo() - Stream is closed!" );   
   return ov_info( mVF, link );
}

const vorbis_comment* SFXVorbisStream::getComment( S32 link )
{
   AssertFatal( mVF, "SFXVorbisStream::getComment() - Stream is closed!" );   
   return ov_comment( mVF, link );
}

U64 SFXVorbisStream::getPcmTotal( S32 link )
{
   AssertFatal( mVF, "SFXVorbisStream::getInfo() - Stream is closed!" );   
   return ov_pcm_total( mVF, link );
}

S32 SFXVorbisStream::read( U8 *buffer, 
                           U32 length, 
                           S32 *bitstream )
{
   AssertFatal( mVF, "SFXVorbisStream::read() - Stream is closed!" );   

   mBitstream = *bitstream;

   #ifdef TORQUE_BIG_ENDIAN
      static const S32 isBigEndian = 1;
   #else
      static const S32 isBigEndian = 0;
   #endif

   // Vorbis doesn't seem to like reading 
   // requests longer than this.
   const U32 MAXREAD = 4096;

   U32 bytesRead = 0;
   U32 offset = 0;
   U32 bytesToRead = 0;

   // Since it only returns the result of one packet 
   // per call, you generally have to loop to read it all.
   while( offset < length )
   {
      if ( ( length - offset ) < MAXREAD )
         bytesToRead = length - offset;
      else
         bytesToRead = MAXREAD;

      bytesRead = ov_read( mVF, (char*)buffer, bytesToRead, isBigEndian, 2, 1, bitstream );
      if( bytesRead == 0 ) // EOF
         return offset;
      else if( bytesRead < 0 )
      {
         // We got an error... return the result.
         return bytesRead;
      }

      offset += bytesRead;
      buffer += bytesRead;
      mBytesRead += bytesRead;
   }

   // Return the total data read.
   return offset;
}

bool SFXVorbisStream::isEOS() const
{
   return ( Parent::isEOS() || ( mStream && ov_pcm_tell( mVF ) == mSamples ) );
}

void SFXVorbisStream::reset()
{
   AssertFatal( mVF, "SFXVorbisStream::reset() - Stream is closed!" );

   // There's a bug in libvorbis 1.2.0 that will cause the
   // ov_pcm_seek* functions to go into an infinite loop on
   // some files (apparently if they contain Theora streams).
   // Avoiding to seek when not necessary seems to do the trick
   // but if it deadlocks here, that's why.

   if( mBytesRead > 0 )
   {
      ov_pcm_seek_page( mVF, 0 );
      mBitstream = 0;
      mBytesRead = 0;
   }
}

U32 SFXVorbisStream::getPosition() const
{
   AssertFatal( mVF, "SFXVorbisStream::getPosition() - Stream is closed!" );
   return U32( ov_pcm_tell( mVF ) ) * mFormat.getBytesPerSample();
}

void SFXVorbisStream::setPosition( U32 offset )
{
   AssertFatal( mVF, "SFXVorbisStream::setPosition() - Stream is closed!" );
   ov_pcm_seek( mVF, offset / mFormat.getBytesPerSample() );
}

U32 SFXVorbisStream::read( U8 *buffer, U32 length )
{
   S32 result = read( buffer, length, &mBitstream );
   if ( result < 0 )
      return 0;

   return result;
}

#endif // TORQUE_OGGVORBIS
