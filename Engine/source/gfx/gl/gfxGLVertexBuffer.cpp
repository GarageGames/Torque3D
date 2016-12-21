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

#include "platform/platform.h"
#include "gfx/gl/gfxGLVertexBuffer.h"

#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLEnumTranslate.h"
#include "gfx/gl/gfxGLUtils.h"
#include "gfx/gl/gfxGLVertexAttribLocation.h"

#include "gfx/gl/gfxGLCircularVolatileBuffer.h"

GLCircularVolatileBuffer* getCircularVolatileVertexBuffer()
{
   static GLCircularVolatileBuffer sCircularVolatileVertexBuffer(GL_ARRAY_BUFFER);
   return &sCircularVolatileVertexBuffer;
}

GFXGLVertexBuffer::GFXGLVertexBuffer(  GFXDevice *device, 
                                       U32 numVerts, 
                                       const GFXVertexFormat *vertexFormat, 
                                       U32 vertexSize, 
                                       GFXBufferType bufferType )
   :  GFXVertexBuffer( device, numVerts, vertexFormat, vertexSize, bufferType ),
      mBufferOffset(0),
      mBufferVertexOffset(0),
      mZombieCache(NULL)

{
   if( mBufferType == GFXBufferTypeVolatile )
   {
      mBuffer = getCircularVolatileVertexBuffer()->getHandle();
      return;
   }

   // Generate a buffer
   glGenBuffers(1, &mBuffer);

   //and allocate the needed memory
   PRESERVE_VERTEX_BUFFER();
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
   glBufferData(GL_ARRAY_BUFFER, numVerts * vertexSize, NULL, GFXGLBufferType[bufferType]);
}

GFXGLVertexBuffer::~GFXGLVertexBuffer()
{
	// While heavy handed, this does delete the buffer and frees the associated memory.
   if( mBufferType != GFXBufferTypeVolatile )
      glDeleteBuffers(1, &mBuffer);

   if( mZombieCache )
      delete [] mZombieCache;
}

void GFXGLVertexBuffer::lock( U32 vertexStart, U32 vertexEnd, void **vertexPtr )
{
   PROFILE_SCOPE(GFXGLVertexBuffer_lock);

   if( mBufferType == GFXBufferTypeVolatile )
   {
      AssertFatal(vertexStart == 0, "");
      if( GFXGL->mCapabilities.vertexAttributeBinding )
      {
         getCircularVolatileVertexBuffer()->lock( mNumVerts * mVertexSize, 0, mBufferOffset, *vertexPtr );
      }
      else
      {
         getCircularVolatileVertexBuffer()->lock( mNumVerts * mVertexSize, mVertexSize, mBufferOffset, *vertexPtr );
         mBufferVertexOffset = mBufferOffset / mVertexSize;
      }
   }
   else
   {
      mFrameAllocator.lock( mNumVerts * mVertexSize );

      lockedVertexPtr = (void*)(mFrameAllocator.getlockedPtr() + (vertexStart * mVertexSize));
      *vertexPtr = lockedVertexPtr;
   }

	lockedVertexStart = vertexStart;
	lockedVertexEnd   = vertexEnd;
}

void GFXGLVertexBuffer::unlock()
{
   PROFILE_SCOPE(GFXGLVertexBuffer_unlock);

   if( mBufferType == GFXBufferTypeVolatile )
   {
      getCircularVolatileVertexBuffer()->unlock();
   }
   else
   {
      U32 offset = lockedVertexStart * mVertexSize;
      U32 length = (lockedVertexEnd - lockedVertexStart) * mVertexSize;
   
      PRESERVE_VERTEX_BUFFER();
      glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
   
      if( !lockedVertexStart && lockedVertexEnd == mNumVerts)
         glBufferData(GL_ARRAY_BUFFER, mNumVerts * mVertexSize, NULL, GFXGLBufferType[mBufferType]); // orphan the buffer

      glBufferSubData(GL_ARRAY_BUFFER, offset, length, mFrameAllocator.getlockedPtr() + offset );

      mFrameAllocator.unlock();
   }

   lockedVertexStart = 0;
	lockedVertexEnd   = 0;
   lockedVertexPtr = NULL;
}

void GFXGLVertexBuffer::prepare()
{
   AssertFatal(0, "GFXGLVertexBuffer::prepare - use GFXGLVertexBuffer::prepare(U32 stream, U32 divisor)");
}

void GFXGLVertexBuffer::prepare(U32 stream, U32 divisor)
{
   if( GFXGL->mCapabilities.vertexAttributeBinding )
   {      
      glBindVertexBuffer( stream, mBuffer, mBufferOffset, mVertexSize );
      glVertexBindingDivisor( stream, divisor );
      return;
   }
}

void GFXGLVertexBuffer::finish()
{
   
}

GLvoid* GFXGLVertexBuffer::getBuffer()
{
	// NULL specifies no offset into the hardware buffer
	return (GLvoid*)NULL;
}

void GFXGLVertexBuffer::zombify()
{
   if(mZombieCache || !mBuffer)
      return;
      
   mZombieCache = new U8[mNumVerts * mVertexSize];
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
   glGetBufferSubData(GL_ARRAY_BUFFER, 0, mNumVerts * mVertexSize, mZombieCache);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glDeleteBuffers(1, &mBuffer);
   mBuffer = 0;
}

void GFXGLVertexBuffer::resurrect()
{
   if(!mZombieCache)
      return;
   
   glGenBuffers(1, &mBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
   glBufferData(GL_ARRAY_BUFFER, mNumVerts * mVertexSize, mZombieCache, GFXGLBufferType[mBufferType]);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   
   delete[] mZombieCache;
   mZombieCache = NULL;
}

namespace
{   
   bool onGFXDeviceSignal( GFXDevice::GFXDeviceEventType type )
   {
      if( GFX->getAdapterType() == OpenGL && GFXDevice::deEndOfFrame == type )
         getCircularVolatileVertexBuffer()->protectUsedRange();

      return true;
   }
}

MODULE_BEGIN( GFX_GL_VertexBuffer )
   MODULE_INIT_AFTER( gfx )
   MODULE_SHUTDOWN_BEFORE( gfx )

   MODULE_INIT
   {
      GFXDevice::getDeviceEventSignal().notify( &onGFXDeviceSignal );
   }

   MODULE_SHUTDOWN
   {
      GFXDevice::getDeviceEventSignal( ).remove( &onGFXDeviceSignal );
   }
MODULE_END