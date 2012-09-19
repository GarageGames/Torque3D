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

#ifndef _SFXDSVOICE_H_
#define _SFXDSVOICE_H_

#ifndef _SFXVOICE_H_
   #include "sfx/sfxVoice.h"
#endif
#ifndef _SFXDSBUFFER_H_
   #include "sfx/dsound/sfxDSBuffer.h"
#endif

#include <dsound.h>

class SFXDSDevice;


class SFXDSVoice : public SFXVoice
{
      typedef SFXVoice Parent;

   protected:

      SFXDSVoice( SFXDSDevice *device,
                  SFXDSBuffer *buffer,
                  IDirectSoundBuffer8 *dsBuffer, 
                  IDirectSound3DBuffer8 *dsBuffer3d );

      /// The device used to commit deferred settings. 
      SFXDSDevice *mDevice;

      IDirectSoundBuffer8 *mDSBuffer;

      IDirectSound3DBuffer8 *mDSBuffer3D;

      bool mIsLooping;

      SFXDSBuffer* _getBuffer() const { return ( SFXDSBuffer* ) mBuffer.getPointer(); }

      /// Helper for converting floating point linear volume
      /// to a logrithmic integer volume for dsound.
      static LONG _linearToLogVolume( F32 linVolume );

      // SFXVoice
      virtual SFXStatus _status() const;
      virtual void _play();
      virtual void _pause();
      virtual void _stop();
      virtual void _seek( U32 sample );
      virtual U32 _tell() const;

   public:

      ///
      static SFXDSVoice* create( SFXDSDevice *device,
                                 SFXDSBuffer *buffer );

      ///
      virtual ~SFXDSVoice();

      // SFXVoice
      void setMinMaxDistance( F32 min, F32 max );
      void play( bool looping );
      void setVelocity( const VectorF& velocity );
      void setTransform( const MatrixF& transform );
      void setVolume( F32 volume );
      void setPitch( F32 pitch );
      void setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume );

};

#endif // _SFXDSBUFFER_H_