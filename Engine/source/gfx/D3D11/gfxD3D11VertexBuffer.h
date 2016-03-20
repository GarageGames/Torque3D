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

#ifndef _GFXD3D_VERTEXBUFFER_H_
#define _GFXD3D_VERTEXBUFFER_H_

#include "gfx/D3D11/gfxD3D11Device.h"
#include "core/util/safeDelete.h"

class GFXD3D11VertexBuffer : public GFXVertexBuffer
{
public:
   ID3D11Buffer *vb;
   StrongRefPtr<GFXD3D11VertexBuffer> mVolatileBuffer;
   void *mLockedBuffer;
#ifdef TORQUE_DEBUG
   #define _VBGuardString "GFX_VERTEX_BUFFER_GUARD_STRING"
   U8 *mDebugGuardBuffer;
   
#endif TORQUE_DEBUG

   bool mIsFirstLock;
   bool mClearAtFrameEnd;

   GFXD3D11VertexBuffer();
   GFXD3D11VertexBuffer( GFXDevice *device, 
                        U32 numVerts, 
                        const GFXVertexFormat *vertexFormat,
                        U32 vertexSize, 
                        GFXBufferType bufferType );
   virtual ~GFXD3D11VertexBuffer();

   void lock(U32 vertexStart, U32 vertexEnd, void **vertexPtr);
   void unlock();
   void prepare() {}

   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
};

//-----------------------------------------------------------------------------
// This is for debugging vertex buffers and trying to track down which vbs
// aren't getting free'd

inline GFXD3D11VertexBuffer::GFXD3D11VertexBuffer() : GFXVertexBuffer(0,0,0,0,(GFXBufferType)0)
{
   vb = NULL;
   mIsFirstLock = true;
   lockedVertexEnd = lockedVertexStart = 0;
   mClearAtFrameEnd = false;

#ifdef TORQUE_DEBUG
   mDebugGuardBuffer = NULL;
   mLockedBuffer = NULL;
#endif
}

inline GFXD3D11VertexBuffer::GFXD3D11VertexBuffer(   GFXDevice *device, 
                                                   U32 numVerts, 
                                                   const GFXVertexFormat *vertexFormat, 
                                                   U32 vertexSize, 
                                                   GFXBufferType bufferType )
   : GFXVertexBuffer( device, numVerts, vertexFormat, vertexSize, bufferType )
{
   vb = NULL;
   mIsFirstLock = true;
   mClearAtFrameEnd = false;
   lockedVertexEnd = lockedVertexStart = 0;
   mLockedBuffer = NULL;
#ifdef TORQUE_DEBUG
   mDebugGuardBuffer = NULL;   
#endif
}

#endif // _GFXD3D_VERTEXBUFFER_H_