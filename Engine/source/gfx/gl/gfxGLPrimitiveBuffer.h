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

#ifndef _GFXGLPRIMITIVEBUFFER_H_
#define _GFXGLPRIMITIVEBUFFER_H_

#include "gfx/gfxPrimitiveBuffer.h"

/// This is a primitive buffer (index buffer to GL users) which uses VBOs.
class GFXGLPrimitiveBuffer : public GFXPrimitiveBuffer
{
public:
	GFXGLPrimitiveBuffer(GFXDevice *device, U32 indexCount, U32 primitiveCount, GFXBufferType bufferType);
	~GFXGLPrimitiveBuffer();

	virtual void lock(U32 indexStart, U32 indexEnd, void **indexPtr); ///< calls glMapBuffer, offets pointer by indexStart
	virtual void unlock(); ///< calls glUnmapBuffer, unbinds the buffer
	virtual void prepare();  ///< binds the buffer
   virtual void finish(); ///< We're done with this buffer

	virtual void* getBuffer(); ///< returns NULL

   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
   
private:
	/// Handle to our GL buffer object
	GLuint mBuffer;
   
   U8* mZombieCache;
};

#endif