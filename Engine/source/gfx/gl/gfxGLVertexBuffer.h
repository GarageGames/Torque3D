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

#ifndef _GFXGLVERTEXBUFFER_H_
#define _GFXGLVERTEXBUFFER_H_

#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#include "gfx/gl/tGL/tGL.h"
#include "gfx/gl/util/glFrameAllocatorLockableHelper.h"

/// This is a vertex buffer which uses GL_ARB_vertex_buffer_object.
class GFXGLVertexBuffer : public GFXVertexBuffer 
{
public:

	GFXGLVertexBuffer(   GFXDevice *device, 
                        U32 numVerts, 
                        const GFXVertexFormat *vertexFormat, 
                        U32 vertexSize, 
                        GFXBufferType bufferType );

	~GFXGLVertexBuffer();

	virtual void lock(U32 vertexStart, U32 vertexEnd, void **vertexPtr); ///< Only write lock are supported. 
	virtual void unlock(); ///<
	virtual void prepare(); ///< Do nothing. Use void prepare(U32 stream, U32 divisor).
   virtual void finish(); ///< Do nothing.

   void prepare(U32 stream, U32 divisor);

	GLvoid* getBuffer(); ///< returns NULL

   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();

private:
   friend class GFXGLDevice;
	/// GL buffer handle
	GLuint mBuffer;

    /// bytes offset in buffer 
	U32 mBufferOffset;

	/// start vertex offset in buffer
	U32 mBufferVertexOffset;
   U8* mZombieCache;

   FrameAllocatorLockableHelper mFrameAllocator;
};

#endif
