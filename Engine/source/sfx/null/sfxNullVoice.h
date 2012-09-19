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

#ifndef _SFXNULLVOICE_H_
#define _SFXNULLVOICE_H_

#ifndef _SFXVOICE_H_
   #include "sfx/sfxVoice.h"
#endif
#ifndef _TIMESOURCE_H_
   #include "core/util/timeSource.h"
#endif


class SFXNullBuffer;


class SFXNullVoice : public SFXVoice
{
   public:

      typedef SFXVoice Parent;
      friend class SFXNullDevice;

   protected:
   
      typedef GenericTimeSource< VirtualMSTimer > TimeSource;

      SFXNullVoice( SFXNullBuffer* buffer );
      
      /// The virtual play timer.
      mutable TimeSource mPlayTimer;

      ///
      bool mIsLooping;

      // SFXVoice.
      virtual SFXStatus _status() const;
      virtual void _play();
      virtual void _pause();
      virtual void _stop();
      virtual void _seek( U32 sample );
      virtual U32 _tell() const;

      ///
      U32 _getPlayTime() const
      {
         return mPlayTimer.getPosition();
      }

   public:

      virtual ~SFXNullVoice();

      /// SFXVoice
      SFXStatus getStatus() const;
      void setPosition( U32 sample );
      void play( bool looping );
      void setMinMaxDistance( F32 min, F32 max );
      void setVelocity( const VectorF& velocity );
      void setTransform( const MatrixF& transform );
      void setVolume( F32 volume );
      void setPitch( F32 pitch );
      void setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume );
};

#endif // _SFXNULLVOICE_H_