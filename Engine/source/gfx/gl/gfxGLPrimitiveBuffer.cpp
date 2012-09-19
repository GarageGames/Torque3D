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

#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLPrimitiveBuffer.h"
#include "gfx/gl/gfxGLEnumTranslate.h"

#include "gfx/gl/ggl/ggl.h"
#include "gfx/gl/gfxGLUtils.h"

GFXGLPrimitiveBuffer::GFXGLPrimitiveBuffer(GFXDevice *device, U32 indexCount, U32 primitiveCount, GFXBufferType bufferType) :
GFXPrimitiveBuffer(device, indexCount, primitiveCount, bufferType), mZombieCache(NULL) 
{
   PRESERVE_INDEX_BUFFER();
	// Generate a buffer and allocate the needed memory
	glGenBuffers(1, &mBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(U16), NULL, GFXGLBufferType[bufferType]);
}

GFXGLPrimitiveBuffer::~GFXGLPrimitiveBuffer()
{
	// This is heavy handed, but it frees the buffer memory
	glDeleteBuffersARB(1, &mBuffer);
   
   if( mZombieCache )
      delete [] mZombieCache;
}

void GFXGLPrimitiveBuffer::lock(U32 indexStart, U32 indexEnd, void **indexPtr)
{
	// Preserve previous binding
   PRESERVE_INDEX_BUFFER();
   
   // Bind ourselves and map
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexCount * sizeof(U16), NULL, GFXGLBufferType[mBufferType]);
   
   // Offset the buffer to indexStart
	*indexPtr = (void*)((U8*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY) + (indexStart * sizeof(U16)));
}

void GFXGLPrimitiveBuffer::unlock()
{
	// Preserve previous binding
   PRESERVE_INDEX_BUFFER();
   
   // Bind ourselves and unmap
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
	bool res = glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
   AssertFatal(res, "GFXGLPrimitiveBuffer::unlock - shouldn't fail!");
}

void GFXGLPrimitiveBuffer::prepare()
{
	// Bind
	static_cast<GFXGLDevice*>(mDevice)->setPB(this);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
}

void GFXGLPrimitiveBuffer::finish()
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

GLvoid* GFXGLPrimitiveBuffer::getBuffer()
{
	// NULL specifies no offset into the hardware buffer
	return (GLvoid*)NULL;
}

void GFXGLPrimitiveBuffer::zombify()
{
   if(mZombieCache)
      return;
      
   mZombieCache = new U8[mIndexCount * sizeof(U16)];
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
   glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mIndexCount * sizeof(U16), mZombieCache);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glDeleteBuffers(1, &mBuffer);
   mBuffer = 0;
}

void GFXGLPrimitiveBuffer::resurrect()
{
   if(!mZombieCache)
      return;
   
   glGenBuffers(1, &mBuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexCount * sizeof(U16), mZombieCache, GFXGLBufferType[mBufferType]);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   
   delete[] mZombieCache;
   mZombieCache = NULL;
}
