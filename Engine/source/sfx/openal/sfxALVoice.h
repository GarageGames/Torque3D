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

#ifndef _SFXALVOICE_H_
#define _SFXALVOICE_H_

#ifndef _SFXVOICE_H_
   #include "sfx/sfxVoice.h"
#endif
#ifndef _OPENALFNTABLE
   #include "sfx/openal/LoadOAL.h"
#endif
#ifndef _PLATFORM_THREADS_MUTEX_H_
   #include "platform/threads/mutex.h"
#endif


class SFXALBuffer;
class SFXALDevice;

class SFXALVoice : public SFXVoice
{
   public:

      typedef SFXVoice Parent;
      friend class SFXALDevice;
      friend class SFXALBuffer;

   protected:

      SFXALVoice( const OPENALFNTABLE &oalft,
                  SFXALBuffer *buffer, 
                  ALuint sourceName );

      ALuint mSourceName;

      /// Buggy OAL jumps around when pausing.  Save playback cursor here.
      F32 mResumeAtSampleOffset;
      
      /// Amount by which OAL's reported sample position is offset.
      ///
      /// OAL's sample position is relative to the current queue state,
      /// so we manually need to keep track of how far into the total
      /// queue we are.
      U32 mSampleOffset;

      Mutex mMutex;

      const OPENALFNTABLE &mOpenAL;

      ///
      SFXALBuffer* _getBuffer() const
      {
         return ( SFXALBuffer* ) mBuffer.getPointer();
      }
      
      /// For non-streaming buffers, late-bind the audio buffer
      /// to the source as OAL will not accept writes to buffers
      /// already bound.
      void _lateBindStaticBufferIfNecessary();

      // SFXVoice.
      virtual SFXStatus _status() const;
      virtual void _play();
      virtual void _pause();
      virtual void _stop();
      virtual void _seek( U32 sample );
      virtual U32 _tell() const;

   public:

      static SFXALVoice* create( SFXALDevice* device,
                                 SFXALBuffer *buffer );

      virtual ~SFXALVoice();

      /// SFXVoice
      void setMinMaxDistance( F32 min, F32 max );
      void play( bool looping );
      void setVelocity( const VectorF& velocity );
      void setTransform( const MatrixF& transform );
      void setVolume( F32 volume );
      void setPitch( F32 pitch );
      void setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume );
      void setRolloffFactor( F32 factor );
};

#endif // _SFXALVOICE_H_