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

#ifndef _SFXXAUDIOBUFFER_H_
#define _SFXXAUDIOBUFFER_H_

#include <xaudio2.h>

#ifndef _SFXINTERNAL_H_
#include "sfx/sfxInternal.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif


/// Audio data buffer for the XAudio device layer.
class SFXXAudioBuffer : public SFXBuffer
{
   public:

      typedef SFXBuffer Parent;
      
      friend class SFXXAudioDevice;
      friend class SFXXAudioVoice;

   protected:

      struct Buffer
      {
         XAUDIO2_BUFFER mData;
         SFXInternal::SFXStreamPacket* mPacket;

         Buffer()
            : mPacket( 0 )
         {
            dMemset( &mData, 0, sizeof( mData ) );
         }
      };

      typedef Vector< Buffer > QueueType;

      QueueType mBufferQueue;

      /// If this is a streaming buffer, return the unique voice associated
      /// with the buffer.
      SFXXAudioVoice* _getUniqueVoice() { return ( SFXXAudioVoice* ) mUniqueVoice.getPointer(); }

      ///
      SFXXAudioBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description );
      virtual ~SFXXAudioBuffer();

      // SFXBuffer.
      virtual void write( SFXInternal::SFXStreamPacket* const* packets, U32 num );
      void _flush();

   public:

      ///
      static SFXXAudioBuffer* create( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description );
};

#endif // _SFXXAUDIOBUFFER_H_