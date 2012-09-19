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

#ifndef _RESIZESTREAM_H_
#define _RESIZESTREAM_H_

//Includes
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _FILTERSTREAM_H_
#include "core/filterStream.h"
#endif

class ResizeFilterStream : public FilterStream, public IStreamByteCount
{
   typedef FilterStream Parent;

   Stream* m_pStream;
   U32     m_startOffset;
   U32     m_streamLen;
   U32     m_currOffset;
   U32 m_lastBytesRead;

  public:
   ResizeFilterStream();
   ~ResizeFilterStream();

   bool    attachStream(Stream* io_pSlaveStream);
   void    detachStream();
   Stream* getStream();

   bool setStreamOffset(const U32 in_startOffset,
                        const U32 in_streamLen);

   // Mandatory overrides.
  protected:
   bool _read(const U32 in_numBytes,  void* out_pBuffer);
  public:
   U32  getPosition() const;
   bool setPosition(const U32 in_newPosition);

   U32  getStreamSize();

   // IStreamByteCount
   U32 getLastBytesRead() { return m_lastBytesRead; }
   U32 getLastBytesWritten() { return 0; }
};

#endif //_RESIZESTREAM_H_
