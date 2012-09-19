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

#include "core/ogg/oggVorbisDecoder.h"
#include "console/console.h"


//#define DEBUG_SPEW


//-----------------------------------------------------------------------------

OggVorbisDecoder::OggVorbisDecoder( const ThreadSafeRef< OggInputStream >& stream )
   : Parent( stream )
#ifdef TORQUE_DEBUG
     , mLock( 0 )
#endif
{
   // Initialize.
   
   vorbis_info_init( &mVorbisInfo );
   vorbis_comment_init( &mVorbisComment );
   dMemset( &mVorbisBlock, 0, sizeof( mVorbisBlock ) );
   dMemset( &mVorbisDspState, 0, sizeof( mVorbisDspState ) );
}

//-----------------------------------------------------------------------------

OggVorbisDecoder::~OggVorbisDecoder()
{
   vorbis_block_clear( &mVorbisBlock );
   vorbis_dsp_clear( &mVorbisDspState );
   vorbis_info_clear( &mVorbisInfo );
   vorbis_comment_clear( &mVorbisComment );
}

//-----------------------------------------------------------------------------

bool OggVorbisDecoder::_detect( ogg_page* startPage )
{
   _setStartPage( startPage );

   // Read initial header packet.
   
   ogg_packet nextPacket;
   if( !_readNextPacket( &nextPacket )
       || vorbis_synthesis_headerin( &mVorbisInfo, &mVorbisComment, &nextPacket ) < 0 )
   {
      vorbis_info_clear( &mVorbisInfo );
      vorbis_comment_clear( &mVorbisComment );
      return false;
   }
   
   return true;
}

//-----------------------------------------------------------------------------

bool OggVorbisDecoder::_init()
{   
   // Read header packets.
   
   bool haveVorbisHeader = true;
   for( U32 i = 0; i < 2; ++ i )
   {
      ogg_packet nextPacket;
      if( !_readNextPacket( &nextPacket ) )
      {
         haveVorbisHeader = false;
         break;
      }
          
      int result = vorbis_synthesis_headerin( &mVorbisInfo, &mVorbisComment, &nextPacket );
      if( result != 0 )
      {
         haveVorbisHeader = false;
         break;
      }
   }
   
   // Fail if we don't have a complete and valid Vorbis header.
   
   if( !haveVorbisHeader )
   {
      vorbis_info_clear( &mVorbisInfo );
      vorbis_comment_clear( &mVorbisComment );
      
      Con::errorf( "OggVorbisDecoder::_init() - Incorrect or corrupt Vorbis headers" );
      
      return false;
   }
   
   // Init synthesis.
   
   vorbis_synthesis_init( &mVorbisDspState, &mVorbisInfo );
   vorbis_block_init( &mVorbisDspState, &mVorbisBlock );
   
   return true;
}

//-----------------------------------------------------------------------------

bool OggVorbisDecoder::_packetin( ogg_packet* packet )
{
   return ( vorbis_synthesis( &mVorbisBlock, packet ) == 0 );
}

//-----------------------------------------------------------------------------

U32 OggVorbisDecoder::read( RawData** buffer, U32 num )
{
   #ifdef TORQUE_DEBUG
   AssertFatal( dCompareAndSwap( mLock, 0, 1 ), "OggVorbisDecoder::read() - simultaneous reads not thread-safe" );
   #endif
   
   U32 numRead = 0;
   
   for( U32 i = 0; i < num; ++ i )
   {
      float** pcmData;
      U32 numSamples;
      
      // Read sample data.
      
      while( 1 )
      {
         numSamples = vorbis_synthesis_pcmout( &mVorbisDspState, &pcmData );
         if( numSamples )
            break;
         else
         {
            if( !_nextPacket() )
               return numRead; // End of stream.
               
            vorbis_synthesis_blockin( &mVorbisDspState, &mVorbisBlock );
         }
      }
      vorbis_synthesis_read( &mVorbisDspState, numSamples );
      
      #ifdef DEBUG_SPEW
      Platform::outputDebugString( "[OggVorbisDecoder] read %i samples", numSamples );
      #endif
      
      // Allocate a packet.
      
      const U32 numChannels = getNumChannels();
      RawData* packet = constructSingle< RawData* >( numSamples * 2 * numChannels ); // Two bytes per channel.
      
      // Convert and copy the samples.
      
      S16* samplePtr = ( S16* ) packet->data;
      for( U32 n = 0; n < numSamples; ++ n )
         for( U32 c = 0; c < numChannels; ++ c )
         {
            S32 val = S32( pcmData[ c ][ n ] * 32767.f );
            if( val > 32767 )
               val = 32767;
            else if( val < -34768 )
               val = -32768;
               
            *samplePtr = val;
            ++ samplePtr;
         }         
      
      // Success.
      
      buffer[ i ] = packet;
      numRead ++;
   }
   
   #ifdef TORQUE_DEBUG
   AssertFatal( dCompareAndSwap( mLock, 1, 0 ), "" );
   #endif
   
   return numRead;
}
