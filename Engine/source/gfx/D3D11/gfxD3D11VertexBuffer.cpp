//-----------------------------------------------------------------------------
// Copyright (c) 2015 GarageGames, LLC
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
#include "gfx/D3D11/gfxD3D11VertexBuffer.h"
#include "console/console.h"

GFXD3D11VertexBuffer::~GFXD3D11VertexBuffer() 
{
   if(getOwningDevice() != NULL)
   {
      if(mBufferType != GFXBufferTypeVolatile)
      {
         SAFE_RELEASE(vb);
      }
   }
}

void GFXD3D11VertexBuffer::lock(U32 vertexStart, U32 vertexEnd, void **vertexPtr)
{
   PROFILE_SCOPE(GFXD3D11VertexBuffer_lock);

   AssertFatal(lockedVertexStart == 0 && lockedVertexEnd == 0, "Cannot lock a buffer more than once!");

   D3D11_MAP flags = D3D11_MAP_WRITE_DISCARD;

   switch(mBufferType)
   {
   case GFXBufferTypeStatic:
   case GFXBufferTypeDynamic:
      flags = D3D11_MAP_WRITE_DISCARD;
      break;

   case GFXBufferTypeVolatile:

      // Get or create the volatile buffer...
      mVolatileBuffer = D3D11->findVBPool( &mVertexFormat, vertexEnd );

      if( !mVolatileBuffer )
         mVolatileBuffer = D3D11->createVBPool( &mVertexFormat, mVertexSize );

      vb = mVolatileBuffer->vb;

      // Get our range now...
      AssertFatal(vertexStart == 0,              "Cannot get a subrange on a volatile buffer.");
      AssertFatal(vertexEnd <= MAX_DYNAMIC_VERTS, "Cannot get more than MAX_DYNAMIC_VERTS in a volatile buffer. Up the constant!");
      AssertFatal(mVolatileBuffer->lockedVertexStart == 0 && mVolatileBuffer->lockedVertexEnd == 0, "Got more than one lock on the volatile pool.");

      // We created the pool when we requested this volatile buffer, so assume it exists...
      if( mVolatileBuffer->mNumVerts + vertexEnd > MAX_DYNAMIC_VERTS ) 
      {
         flags = D3D11_MAP_WRITE_DISCARD;
         mVolatileStart = vertexStart  = 0;
         vertexEnd      = vertexEnd;
      }
      else 
      {
         flags = D3D11_MAP_WRITE_NO_OVERWRITE;
         mVolatileStart = vertexStart  = mVolatileBuffer->mNumVerts;
         vertexEnd                    += mVolatileBuffer->mNumVerts;
      }

      mVolatileBuffer->mNumVerts = vertexEnd+1;

      mVolatileBuffer->lockedVertexStart = vertexStart;
      mVolatileBuffer->lockedVertexEnd   = vertexEnd;
      break;
   }

   lockedVertexStart = vertexStart;
   lockedVertexEnd   = vertexEnd;

   // uncomment it for debugging purpose. called many times per frame... spammy! 
   //Con::printf("%x: Locking %s range (%d, %d)", this, (mBufferType == GFXBufferTypeVolatile ? "volatile" : "static"), lockedVertexStart, lockedVertexEnd);

   U32 sizeToLock = (vertexEnd - vertexStart) * mVertexSize;
   if(mBufferType == GFXBufferTypeStatic)
   {	   
	   *vertexPtr = new U8[sizeToLock];
	   mLockedBuffer = *vertexPtr;
   }
   else
   {
		D3D11_MAPPED_SUBRESOURCE pVertexData;
		ZeroMemory(&pVertexData, sizeof(D3D11_MAPPED_SUBRESOURCE));

		HRESULT hr = D3D11DEVICECONTEXT->Map(vb, 0, flags, 0, &pVertexData);

		if(FAILED(hr)) 
		{
			AssertFatal(false, "Unable to lock vertex buffer.");
		}

		*vertexPtr = (U8*)pVertexData.pData + (vertexStart * mVertexSize);
   }
   
  

   #ifdef TORQUE_DEBUG
   
      // Allocate a debug buffer large enough for the lock
      // plus space for over and under run guard strings.
      const U32 guardSize = sizeof( _VBGuardString );
      mDebugGuardBuffer = new U8[sizeToLock+(guardSize*2)];

      // Setup the guard strings.
      dMemcpy( mDebugGuardBuffer, _VBGuardString, guardSize ); 
      dMemcpy( mDebugGuardBuffer + sizeToLock + guardSize, _VBGuardString, guardSize ); 

      // Store the real lock pointer and return our debug pointer.
      mLockedBuffer = *vertexPtr;
      *vertexPtr = mDebugGuardBuffer + guardSize;

   #endif // TORQUE_DEBUG
}

void GFXD3D11VertexBuffer::unlock()
{
   PROFILE_SCOPE(GFXD3D11VertexBuffer_unlock);
   
   #ifdef TORQUE_DEBUG
   
      if ( mDebugGuardBuffer )
      {
         const U32 guardSize = sizeof( _VBGuardString );
		   const U32 sizeLocked = (lockedVertexEnd - lockedVertexStart) * mVertexSize;

		   // First check the guard areas for overwrites.
		   AssertFatal(dMemcmp( mDebugGuardBuffer, _VBGuardString, guardSize) == 0,
		   "GFXD3D11VertexBuffer::unlock - Caught lock memory underrun!" );
		   AssertFatal(dMemcmp( mDebugGuardBuffer + sizeLocked + guardSize, _VBGuardString, guardSize) == 0,
		   "GFXD3D11VertexBuffer::unlock - Caught lock memory overrun!" );
                        
		   // Copy the debug content down to the real VB.
		   dMemcpy(mLockedBuffer, mDebugGuardBuffer + guardSize, sizeLocked);

		   // Cleanup.
		   delete [] mDebugGuardBuffer;
		   mDebugGuardBuffer = NULL;
		   //mLockedBuffer = NULL;
      }

   #endif // TORQUE_DEBUG

   if(mBufferType == GFXBufferTypeStatic)
   {
		const U32 sizeLocked = (lockedVertexEnd - lockedVertexStart) * mVertexSize;
		//set up the update region of the buffer
		D3D11_BOX box;
		box.back  = 1;
		box.front = 0;
		box.top = 0;
		box.bottom = 1;
		box.left = lockedVertexStart * mVertexSize;
		box.right = lockedVertexEnd * mVertexSize;
		//update the real vb buffer
		D3D11DEVICECONTEXT->UpdateSubresource(vb, 0, &box,mLockedBuffer,sizeLocked, 0);
		//clean up the old buffer
		delete[] mLockedBuffer;
		mLockedBuffer = NULL;
   }
   else
   {	 
      D3D11DEVICECONTEXT->Unmap(vb,0);
   }
   

   mIsFirstLock = false;

   //uncomment it for debugging purpose. called many times per frame... spammy!
   //Con::printf("%x: Unlocking %s range (%d, %d)", this, (mBufferType == GFXBufferTypeVolatile ? "volatile" : "static"), lockedVertexStart, lockedVertexEnd);

   lockedVertexEnd = lockedVertexStart = 0;

   if(mVolatileBuffer.isValid())
   {
      mVolatileBuffer->lockedVertexStart = 0;
      mVolatileBuffer->lockedVertexEnd   = 0;
      mVolatileBuffer = NULL;
   }
}

void GFXD3D11VertexBuffer::zombify()
{
   AssertFatal(lockedVertexStart == 0 && lockedVertexEnd == 0, "GFXD3D11VertexBuffer::zombify - Cannot zombify a locked buffer!");
   // Static buffers are managed by D3D11 so we don't deal with them.
   if(mBufferType == GFXBufferTypeDynamic)
   {
      SAFE_RELEASE(vb);
   }
}

void GFXD3D11VertexBuffer::resurrect()
{
   // Static buffers are managed by D3D11 so we don't deal with them.
   if(mBufferType == GFXBufferTypeDynamic)
   {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = mVertexSize * mNumVerts;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		HRESULT hr = D3D11DEVICE->CreateBuffer(&desc, NULL, &vb);

      if(FAILED(hr)) 
      {
		   AssertFatal(false, "GFXD3D11VertexBuffer::resurrect - Failed to allocate VB");
      }
   }
}

