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

#ifndef _BYTEBUFFER_H_
#define _BYTEBUFFER_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

namespace Torque
{

class PrivateBBData;

class ByteBuffer
{
public:
   ByteBuffer();

   /// Create a ByteBuffer from a chunk of memory.
   ByteBuffer(U8 *dataPtr, U32 bufferSize);

   /// Create a ByteBuffer of the specified size.
   ByteBuffer(U32 bufferSize);

   /// Copy constructor
   ByteBuffer(const ByteBuffer &theBuffer);
   
   ByteBuffer  &operator=(const ByteBuffer &theBuffer);

   ~ByteBuffer();

   /// Set the ByteBuffer to point to a new chunk of memory.
   void setBuffer(U8 *dataPtr, U32 bufferSize, bool copyData);

   /// Resize the buffer.
   void resize(U32 newBufferSize);

   /// Appends the specified buffer to the end of the byte buffer.
   void appendBuffer(const U8 *dataBuffer, U32 bufferSize);

   /// Appends the specified ByteBuffer to the end of this byte buffer.
   void appendBuffer(const ByteBuffer &theBuffer)
   {
      appendBuffer(theBuffer.getBuffer(), theBuffer.getBufferSize());
   }

   U32 getBufferSize() const;

   U8 *getBuffer();
   const U8 *getBuffer() const;

   /// Copy the data in the buffer.
   ByteBuffer  getCopy() const;

   /// Clear the buffer.
   void clear();

private:
   PrivateBBData  *_data;
};

}

#endif
