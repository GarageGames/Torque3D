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

#ifndef _SFXWAVSTREAM_H_
#define _SFXWAVSTREAM_H_

#ifndef _SFXFILESTREAM_H_
   #include "sfx/sfxFileStream.h"
#endif
#include "core/util/safeDelete.h"


/// An SFXFileStream that loads sample data from a WAV file.
class SFXWavStream : public SFXFileStream,
                     public IPositionable< U32 >
{
   public:

      typedef SFXFileStream Parent;

   protected:

      /// The file position of the start of
      /// the PCM data for fast reset().
      U32 mDataStart;

      // SFXFileStream
      virtual bool _readHeader();
      virtual void _close();

   public:

      ///
      static SFXWavStream* create( Stream *stream );

      ///
	   SFXWavStream();

      ///
      SFXWavStream( const SFXWavStream& cloneFrom );

      /// Destructor.
      virtual ~SFXWavStream();

      // SFXStream
      virtual void reset();
      virtual U32 read( U8 *buffer, U32 length );
      virtual SFXStream* clone() const
      {
         SFXWavStream* stream = new SFXWavStream( *this );
         if( !stream->mStream )
            SAFE_DELETE( stream );
         return stream;
      }

      // IPositionable
      virtual U32 getPosition() const;
      virtual void setPosition( U32 offset );
};

#endif  // _SFXWAVSTREAM_H_
