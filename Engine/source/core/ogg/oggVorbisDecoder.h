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

#ifndef _OGGVORBISDECODER_H_
#define _OGGVORBISDECODER_H_

#ifndef _OGGINPUTSTREAM_H_
   #include "core/ogg/oggInputStream.h"
#endif
#ifndef _TSTREAM_H_
   #include "core/stream/tStream.h"
#endif
#ifndef _RAWDATA_H_
   #include "core/util/rawData.h"
#endif
#include "vorbis/codec.h"


/// Decodes a Vorbis substream into sample packets.
///
/// Vorbis samples are always 16bits.
class OggVorbisDecoder : public OggDecoder,
                         public IInputStream< RawData* >
{
   public:
   
      typedef OggDecoder Parent;
      
   protected:
      
      ///
      vorbis_info mVorbisInfo;
      
      ///
      vorbis_comment mVorbisComment;
      
      ///
      vorbis_dsp_state mVorbisDspState;
      
      ///
      vorbis_block mVorbisBlock;
      
      #ifdef TORQUE_DEBUG
      U32 mLock;
      #endif

      // OggDecoder.
      virtual bool _detect( ogg_page* startPage );
      virtual bool _init();
      virtual bool _packetin( ogg_packet* packet );
   
   public:
   
      ///
      OggVorbisDecoder( const ThreadSafeRef< OggInputStream >& stream );
      
      ~OggVorbisDecoder();
      
      ///
      U32 getNumChannels() const { return mVorbisInfo.channels; }
      
      ///
      U32 getSamplesPerSecond() const { return mVorbisInfo.rate; }
      
      // OggDecoder.
      virtual const char* getName() const { return "Vorbis"; }
      
      // IInputStream.
      virtual U32 read( RawData** buffer, U32 num );
};

#endif // !_OGGVORBISDECODER_H_
