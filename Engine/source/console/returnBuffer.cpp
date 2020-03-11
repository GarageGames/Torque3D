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

#include "returnBuffer.h"

ReturnBuffer::ReturnBuffer()
{
   mBuffer = nullptr;
   mBufferSize = 0;
   mStart = 0;

   //Decent starting alloc of ~1 page = 4kb
   ensureSize(4 * 1024);
}

ReturnBuffer::~ReturnBuffer()
{
   if (mBuffer)
   {
      dFree(mBuffer);
   }
}

void ReturnBuffer::ensureSize(U32 newSize)
{
   //Round up to nearest multiple of 16 bytes
   if (newSize & 0xF)
   {
      newSize = (newSize & ~0xF) + 0x10;
   }
   if (mBuffer == NULL)
   {
      //First alloc
      mBuffer = (char *)dMalloc(newSize * sizeof(char));
      mBufferSize = newSize;
   }
   else if (mBufferSize < newSize)
   {
      //Just use the expected size
      mBuffer = (char *)dRealloc(mBuffer, newSize * sizeof(char));
      mBufferSize = newSize;
   }
}

char *ReturnBuffer::getBuffer(U32 size, U32 alignment)
{
   ensureSize(size);

   //Align the start if necessary
   if (mStart % alignment != 0)
   {
      mStart += alignment - (mStart % alignment);
   }

   if (size + mStart > mBufferSize)
   {
      //Restart
      mStart = 0;
   }
   char *buffer = mBuffer + mStart;
   mStart += size;

   return buffer;
}
