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

#ifndef _RETURNBUFFER_H_
#define _RETURNBUFFER_H_

#ifndef _PLATFORM_H_
   #include <platform/platform.h>
#endif

/// Simple circular buffer class for temporary return storage.
class ReturnBuffer
{
   char *mBuffer;
   U32 mBufferSize;
   U32 mStart;

   /// Possibly expand the buffer to be larger than newSize
   void ensureSize(U32 newSize);

public:
   ReturnBuffer();
   ~ReturnBuffer();

   /// Get a temporary buffer with a given size (and alignment)
   /// @note The buffer will be re-used so do not consider it permanent
   char *getBuffer(U32 size, U32 alignment = 16);
};

#endif //_RETURNBUFFER_H_
