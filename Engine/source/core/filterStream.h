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

#ifndef _FILTERSTREAM_H_
#define _FILTERSTREAM_H_

//Includes
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _STREAM_H_
#include "core/stream/stream.h"
#endif

class FilterStream : public Stream
{
  public:
   virtual ~FilterStream();

   virtual bool    attachStream(Stream* io_pSlaveStream) = 0;
   virtual void    detachStream()                        = 0;
   virtual Stream* getStream()                           = 0;

   // Mandatory overrides.  By default, these are simply passed to
   //  whatever is returned from getStream();
  protected:
   bool _read(const U32 in_numBytes,  void* out_pBuffer);
   bool _write(const U32 in_numBytes, const void* in_pBuffer);
  public:
   bool hasCapability(const Capability) const;

   U32  getPosition() const;
   bool setPosition(const U32 in_newPosition);
   U32  getStreamSize();
};

#endif //_FILTERSTREAM_H_
