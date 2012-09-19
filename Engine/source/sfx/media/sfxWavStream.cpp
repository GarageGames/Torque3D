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

#include "sfx/media/sfxWavStream.h"
#include "core/stream/stream.h"
#include "core/strings/stringFunctions.h"


/// WAV File-header
struct WAVFileHdr
{
   U8  id[4];
   U32 size;
   U8  type[4];
};

//// WAV Fmt-header
struct WAVFmtHdr
{
   U16 format;
   U16 channels;
   U32 samplesPerSec;
   U32 bytesPerSec;
   U16 blockAlign;
   U16 bitsPerSample;
};

/// WAV FmtEx-header
struct WAVFmtExHdr
{
   U16 size;
   U16 samplesPerBlock;
};

/// WAV Smpl-header
struct WAVSmplHdr
{
   U32 manufacturer;
   U32 product;
   U32 samplePeriod;
   U32 note;
   U32 fineTune;
   U32 SMPTEFormat;
   U32 SMPTEOffest;
   U32 loops;
   U32 samplerData;

   struct
   {
      U32 identifier;
      U32 type;
      U32 start;
      U32 end;
      U32 fraction;
      U32 count;
   } loop[1];
};

/// WAV Chunk-header
struct WAVChunkHdr
{
   U8  id[4];
   U32 size;
};


SFXWavStream* SFXWavStream::create( Stream *stream )
{
   SFXWavStream *sfxStream = new SFXWavStream();
   if ( sfxStream->open( stream, true ) )
      return sfxStream;

   delete sfxStream;
   return NULL;
}

SFXWavStream::SFXWavStream()
{
}

SFXWavStream::SFXWavStream( const SFXWavStream& cloneFrom )
   : Parent( cloneFrom ),
     mDataStart( cloneFrom.mDataStart )
{
}

SFXWavStream::~SFXWavStream()
{
   // We must call close from our own destructor
   // and not the base class... as it causes a
   // pure virtual runtime assertion.
   close();
}

void SFXWavStream::_close()
{
   mDataStart = -1;
}

bool SFXWavStream::_readHeader()
{
   // We read the wav chunks to gather than header info
   // and find the start and end position of the data chunk. 
   mDataStart = -1;

   WAVFileHdr fileHdr;
   mStream->read( 4, &fileHdr.id[0] );
   mStream->read( &fileHdr.size );
   mStream->read( 4, &fileHdr.type[0] );

   fileHdr.size=((fileHdr.size+1)&~1)-4;

   WAVChunkHdr chunkHdr;
   mStream->read( 4, &chunkHdr.id[0] );
   mStream->read( &chunkHdr.size );

   // Unread chunk data rounded up to nearest WORD.
   S32 chunkRemaining = chunkHdr.size + ( chunkHdr.size & 1 );

   WAVFmtHdr   fmtHdr;
   WAVFmtExHdr fmtExHdr;
   WAVSmplHdr  smplHdr;

   dMemset(&fmtHdr, 0, sizeof(fmtHdr));

   while ((fileHdr.size!=0) && (mStream->getStatus() != Stream::EOS))
   {
      // WAV format header chunk.
      if ( !dStrncmp( (const char*)chunkHdr.id, "fmt ", 4 ) )
      {
         mStream->read(&fmtHdr.format);
         mStream->read(&fmtHdr.channels);
         mStream->read(&fmtHdr.samplesPerSec);
         mStream->read(&fmtHdr.bytesPerSec);
         mStream->read(&fmtHdr.blockAlign);
         mStream->read(&fmtHdr.bitsPerSample);

         if ( fmtHdr.format == 0x0001 )
         {
            mFormat.set( fmtHdr.channels, fmtHdr.bitsPerSample * fmtHdr.channels, fmtHdr.samplesPerSec );
            chunkRemaining -= sizeof( WAVFmtHdr );
         }
         else
         {
            mStream->read(sizeof(WAVFmtExHdr), &fmtExHdr);
            chunkRemaining -= sizeof(WAVFmtExHdr);
         }
      }

      // WAV data chunk
      else if (!dStrncmp((const char*)chunkHdr.id,"data",4))
      {
         // TODO: Handle these other formats in a more graceful manner!

         if (fmtHdr.format==0x0001)
         {
            mDataStart = mStream->getPosition();
            mStream->setPosition( mDataStart + chunkHdr.size );
            chunkRemaining -= chunkHdr.size;
            mSamples = chunkHdr.size / mFormat.getBytesPerSample();
         }
         else if (fmtHdr.format==0x0011)
         {
            //IMA ADPCM
         }
         else if (fmtHdr.format==0x0055)
         {
            //MP3 WAVE
         }
      }

      // WAV sample header
      else if (!dStrncmp((const char*)chunkHdr.id,"smpl",4))
      {
         // this struct read is NOT endian safe but it is ok because
         // we are only testing the loops field against ZERO
         mStream->read(sizeof(WAVSmplHdr), &smplHdr);

         // This has never been hooked up and its usefulness is
         // dubious.  Do we really want the audio file overriding
         // the SFXDescription setting?    
         //mLooping = ( smplHdr.loops ? true : false );

         chunkRemaining -= sizeof(WAVSmplHdr);
      }

      // either we have unread chunk data or we found an unknown chunk type
      // loop and read up to 1K bytes at a time until we have
      // read to the end of this chunk
      AssertFatal(chunkRemaining >= 0, "AudioBuffer::readWAV: remaining chunk data should never be less than zero.");
      if ( chunkRemaining > 0 )
      {
         U32 pos = mStream->getPosition();
         mStream->setPosition( pos + chunkRemaining );
         chunkRemaining = 0;
      }

      fileHdr.size-=(((chunkHdr.size+1)&~1)+8);

      // read next chunk header...
      mStream->read(4, &chunkHdr.id[0]);
      mStream->read(&chunkHdr.size);
      // unread chunk data rounded up to nearest WORD
      chunkRemaining = chunkHdr.size + (chunkHdr.size&1);
   }

   return ( mDataStart != -1 );
}

void SFXWavStream::reset()
{
   AssertFatal( mStream, "SFXWavStream::reset() - Stream is null!" );
   AssertFatal( mDataStart != -1, "SFXWavStream::seek() - Data start offset is invalid!" );
   mStream->setPosition( mDataStart );
}

U32 SFXWavStream::getPosition() const
{
   AssertFatal( mStream, "SFXWavStream::getPosition() - Stream is null!" );
   return ( mStream->getPosition() - mDataStart );
}

void SFXWavStream::setPosition( U32 offset )
{
   AssertFatal( mStream, "SFXWavStream::setPosition() - Stream is null!" );

   offset -= offset % mFormat.getBytesPerSample();
   const U32 dataLength = mSamples * mFormat.getBytesPerSample();
   if( offset > dataLength )
      offset = dataLength;

   AssertFatal( mDataStart != -1, "SFXWavStream::getPosition() - Data start offset is invalid!" );

   U32 byte = mDataStart + offset;

   mStream->setPosition( byte );
}

U32 SFXWavStream::read( U8 *buffer, U32 bytes )
{
   AssertFatal( mStream, "SFXWavStream::seek() - Stream is null!" );

   // Read in even sample chunks.
   
   bytes -= bytes % mFormat.getBytesPerSample();

   // Read the data and determine how much we've read.
   // FileStreams apparently report positions past
   // the actual stream length, so manually cap the
   // numbers here.
   
   const U32 oldPosition = mStream->getPosition();
   mStream->read( bytes, buffer );
   U32 newPosition = mStream->getPosition();
   const U32 maxPosition = getDataLength() + mDataStart;
   if( newPosition > maxPosition )
      newPosition = maxPosition;
      
   const U32 numBytesRead = newPosition - oldPosition;

   // TODO: Is it *just* 16 bit samples that needs to 
   // be flipped?  What about 32 bit samples?
   #ifdef TORQUE_BIG_ENDIAN

      // We need to endian-flip 16-bit data.
      if ( getFormat().getBytesPerChannel() == 2 ) 
      {
         U16 *ds = (U16*)buffer;
         U16 *de = (U16*)(buffer+bytes);
         while (ds<de)
         {
            *ds = convertLEndianToHost(*ds);
            ds++;
         }
      }

   #endif

   return numBytesRead;
}
