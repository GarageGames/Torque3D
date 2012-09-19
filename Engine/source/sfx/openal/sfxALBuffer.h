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

#ifndef _SFXALBUFFER_H_
#define _SFXALBUFFER_H_

#ifndef _LOADOAL_H
   #include "sfx/openal/LoadOAL.h"
#endif
#ifndef _SFXINTERNAL_H_
   #include "sfx/sfxInternal.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif


class SFXALVoice;


class SFXALBuffer : public SFXBuffer
{
   public:

      typedef SFXBuffer Parent;

      friend class SFXALDevice;
      friend class SFXALVoice;

   protected:
      
      /// AL buffer in case this is a static, non-streaming buffer.
      ALuint mALBuffer;
      
      /// Free buffers for use in queuing in case this is a streaming buffer.
      Vector< ALuint > mFreeBuffers;

      ///
      SFXALBuffer(   const OPENALFNTABLE &oalft, 
                     const ThreadSafeRef< SFXStream >& stream,
                     SFXDescription* description,
                     bool useHardware );

      ///
      bool mIs3d;

      ///
      bool mUseHardware;

      const OPENALFNTABLE &mOpenAL;

      ///
      ALenum _getALFormat() const
      {
         return _sfxFormatToALFormat( getFormat() );
      }

      ///
      static ALenum _sfxFormatToALFormat( const SFXFormat& format )
      {
         if( format.getChannels() == 2 )
         {
            const U32 bps = format.getBitsPerSample();
            if( bps == 16 )
               return AL_FORMAT_STEREO8;
            else if( bps == 32 )
               return AL_FORMAT_STEREO16;
         }
         else if( format.getChannels() == 1 )
         {
            const U32 bps = format.getBitsPerSample();
            if( bps == 8 )
               return AL_FORMAT_MONO8;
            else if( bps == 16 )
               return AL_FORMAT_MONO16;
         }
         return 0;
      }

      ///
      SFXALVoice* _getUniqueVoice() const
      {
         return ( SFXALVoice* ) mUniqueVoice.getPointer();
      }

      // SFXBuffer.
      virtual void write( SFXInternal::SFXStreamPacket* const* packets, U32 num );
      void _flush();

   public:

      static SFXALBuffer* create(   const OPENALFNTABLE &oalft, 
                                    const ThreadSafeRef< SFXStream >& stream,
                                    SFXDescription* description,
                                    bool useHardware );

      virtual ~SFXALBuffer();
};

#endif // _SFXALBUFFER_H_