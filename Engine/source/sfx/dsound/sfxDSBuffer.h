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

#ifndef _SFXDSBUFFER_H_
#define _SFXDSBUFFER_H_

#include <dsound.h>

#ifndef _SFXINTERNAL_H_
#  include "sfx/sfxInternal.h"
#endif


/// DirectSound SFXBuffer implementation.
///
/// Note that the actual sound buffer held by the buffer may
/// get duplicated around for individual voices.  This is kind
/// of ugly as the resulting buffers aren't tied to a SFXDSBuffer
/// anymore.
class SFXDSBuffer : public SFXInternal::SFXWrapAroundBuffer
{
      typedef SFXInternal::SFXWrapAroundBuffer Parent;

      friend class SFXDSDevice;
      friend class SFXDSVoice;

   protected:

      ///
      bool mIs3d;

      ///
      bool mUseHardware;

      IDirectSound8 *mDSound;

      /// The buffer used when duplication is allowed.
      IDirectSoundBuffer8 *mBuffer;
    
      /// We set this to true when the original buffer has
      /// been handed out and duplicates need to be made.
      bool mDuplicate;

      ///
      SFXDSBuffer(   IDirectSound8 *dsound,
                     const ThreadSafeRef< SFXStream >& stream,
                     SFXDescription* description,
                     bool useHardware );

      virtual ~SFXDSBuffer();

      /// Set up a DirectSound buffer.
      /// @note This method will not fill the buffer with data.
      /// @note If this is a streaming buffer, the resulting buffer
      ///    will have its notification positions set up and already
      ///    be registered with SFXDSStreamThread.
      bool _createBuffer( IDirectSoundBuffer8 **buffer8 );

      ///
      bool _duplicateBuffer( IDirectSoundBuffer8 **buffer8 );

      // SFXWrapAroundBuffer.
      virtual bool _copyData( U32 offset, const U8* data, U32 length );
      virtual void _flush();

public:

      ///
      static SFXDSBuffer* create(   IDirectSound8* dsound, 
                                    const ThreadSafeRef< SFXStream >& stream,
                                    SFXDescription* description,
                                    bool useHardware );

      //
      bool createVoice( IDirectSoundBuffer8 **buffer );

      //
      void releaseVoice( IDirectSoundBuffer8 **buffer );

};

#endif // _SFXDSBUFFER_H_