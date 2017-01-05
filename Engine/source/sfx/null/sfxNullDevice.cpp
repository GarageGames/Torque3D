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

#include "sfx/null/sfxNullDevice.h"
#include "sfx/null/sfxNullBuffer.h"
#include "sfx/sfxInternal.h"


SFXNullDevice::SFXNullDevice( SFXProvider* provider, 
                              String name, 
                              bool useHardware, 
                              S32 maxBuffers )

   :  SFXDevice( name, provider, useHardware, maxBuffers )
{
   mMaxBuffers = getMax( maxBuffers, 8 );
}

SFXNullDevice::~SFXNullDevice()
{
}

SFXBuffer* SFXNullDevice::createBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description )
{
   SFXNullBuffer* buffer = new SFXNullBuffer( stream, description );
   _addBuffer( buffer );

   return buffer;
}

SFXVoice* SFXNullDevice::createVoice( bool is3D, SFXBuffer *buffer )
{
   // Don't bother going any further if we've 
   // exceeded the maximum voices.
   if ( mVoices.size() >= mMaxBuffers )
      return NULL;

   AssertFatal( buffer, "SFXNullDevice::createVoice() - Got null buffer!" );

   SFXNullBuffer* nullBuffer = dynamic_cast<SFXNullBuffer*>( buffer );
   AssertFatal( nullBuffer, "SFXNullDevice::createVoice() - Got bad buffer!" );

   SFXNullVoice* voice = new SFXNullVoice( nullBuffer );

   _addVoice( voice );
	return voice;
}

void SFXNullDevice::update()
{
   // Do nothing.  Prevent SFXDevice from running
   // its thing.
}
