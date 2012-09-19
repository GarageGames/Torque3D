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


GFXGLVertexBuffer::GFXGLVertexBuffer(  GFXDevice *device, 
                                       U32 numVerts, 
                                       const GFXVertexFormat *vertexFormat, 
                                       U32 vertexSize, 
                                       GFXBufferType bufferType )
   :  GFXVertexBuffer( device, numVerts, vertexFormat, vertexSize, bufferType ), 
      mZombieCache(NULL)
{
   PRESERVE_VERTEX_BUFFER();
	// Generate a buffer and allocate the needed memory.
	glGenBuffers(1, &mBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	glBufferData(GL_ARRAY_BUFFER, numVerts * vertexSize, NULL, GFXGLBufferType[bufferType]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GFXGLVertexBuffer::~GFXGLVertexBuffer()
{
	// While heavy handed, this does delete the buffer and frees the associated memory.
   glDeleteBuffers(1, &mBuffer);
   
   if( mZombieCache )
      delete [] mZombieCache;
}

void GFXGLVertexBuffer::lock( U32 vertexStart, U32 vertexEnd, void **vertexPtr )
{
   PRESERVE_VERTEX_BUFFER();
	// Bind us, get a pointer into the buffer, then
	// offset it by vertexStart so we act like the D3D layer.
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
   glBufferData(GL_ARRAY_BUFFER, mNumVerts * mVertexSize, NULL, GFXGLBufferType[mBufferType]);
	*vertexPtr = (void*)((U8*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY) + (vertexStart * mVertexSize));
	lockedVertexStart = vertexStart;
	lockedVertexEnd   = vertexEnd;
}

void GFXGLVertexBuffer::unlock()
{
   PRESERVE_VERTEX_BUFFER();
	// Unmap the buffer and bind 0 to GL_ARRAY_BUFFER
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	bool res = glUnmapBuffer(GL_ARRAY_BUFFER);
   AssertFatal(res, "GFXGLVertexBuffer::unlock - shouldn't fail!");

   lockedVertexStart = 0;
	lockedVertexEnd   = 0;
}

void GFXGLVertexBuffer::prepare()
{
	// Bind the buffer...
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
   U8* buffer = (U8*)getBuffer();

   // Loop thru the vertex format elements adding the array state...
   U32 texCoordIndex = 0;
   for ( U32 i=0; i < mVertexFormat.getElementCount(); i++ )
   {
      const GFXVertexElement &element = mVertexFormat.getElement( i );
      
      if ( element.isSemantic( GFXSemantic::POSITION ) )
      {
         glEnableClientState( GL_VERTEX_ARRAY );
         glVertexPointer( element.getSizeInBytes() / 4, GL_FLOAT, mVertexSize, buffer );
         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::NORMAL ) )
      {
         glEnableClientState( GL_NORMAL_ARRAY );
         glNormalPointer( GL_FLOAT, mVertexSize, buffer );
         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::COLOR ) )
      {
         glEnableClientState( GL_COLOR_ARRAY );
         glColorPointer( element.getSizeInBytes(), GL_UNSIGNED_BYTE, mVertexSize, buffer );
         buffer += element.getSizeInBytes();
      }
      else // Everything else is a texture coordinate.
      {
         glClientActiveTexture( GL_TEXTURE0 + texCoordIndex );
         glEnableClientState( GL_TEXTURE_COORD_ARRAY );
         glTexCoordPointer( element.getSizeInBytes() / 4, GL_FLOAT, mVertexSize, buffer );
         buffer += element.getSizeInBytes();
         ++texCoordIndex;
      }
      
   }
}

void GFXGLVertexBuffer::finish()
{
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   
   U32 texCoordIndex = 0;
   for ( U32 i=0; i < mVertexFormat.getElementCount(); i++ )
   {
      const GFXVertexElement &element = mVertexFormat.getElement( i );

      if ( element.isSemantic( GFXSemantic::POSITION ) )
         glDisableClientState( GL_VERTEX_ARRAY );
      else if ( element.isSemantic( GFXSemantic::NORMAL ) )
         glDisableClientState( GL_NORMAL_ARRAY );
      else if ( element.isSemantic( GFXSemantic::COLOR ) )
         glDisableClientState( GL_COLOR_ARRAY );
      else
      {
         glClientActiveTexture( GL_TEXTURE0 + texCoordIndex );
         glDisableClientState(GL_TEXTURE_COORD_ARRAY);
         ++texCoordIndex;
      }
   }
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
