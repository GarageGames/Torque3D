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

#ifndef _GFXD3D9PRIMITIVEBUFFER_H_
#define _GFXD3D9PRIMITIVEBUFFER_H_

#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif


struct IDirect3DIndexBuffer9;

class GFXD3D9PrimitiveBuffer : public GFXPrimitiveBuffer
{
   public:
      IDirect3DIndexBuffer9 *ib;
      StrongRefPtr<GFXD3D9PrimitiveBuffer> mVolatileBuffer;
      U32 mVolatileStart;

#ifdef TORQUE_DEBUG
   #define _PBGuardString "GFX_PRIMTIVE_BUFFER_GUARD_STRING"
   U8 *mDebugGuardBuffer;
   void *mLockedBuffer;
   U32 mLockedSize;
#endif TORQUE_DEBUG

      bool mLocked;
      bool                  mIsFirstLock;

      GFXD3D9PrimitiveBuffer( GFXDevice *device, 
                              U32 indexCount, 
                              U32 primitiveCount, 
                              GFXBufferType bufferType );

      virtual ~GFXD3D9PrimitiveBuffer();

      virtual void lock(U32 indexStart, U32 indexEnd, void **indexPtr);
      virtual void unlock();

      virtual void prepare();      

#ifdef TORQUE_DEBUG
   //GFXD3D9PrimitiveBuffer *next;
#endif

      // GFXResource interface
      virtual void zombify();
      virtual void resurrect();
};

inline GFXD3D9PrimitiveBuffer::GFXD3D9PrimitiveBuffer(   GFXDevice *device, 
                                                         U32 indexCount, 
                                                         U32 primitiveCount, 
                                                         GFXBufferType bufferType ) 
   : GFXPrimitiveBuffer( device, indexCount, primitiveCount, bufferType )
{
   mVolatileStart = 0;
   ib             = NULL;
   mIsFirstLock   = true;
   mLocked = false;
#ifdef TORQUE_DEBUG
   mDebugGuardBuffer = NULL;
   mLockedBuffer = NULL;
   mLockedSize = 0;
#endif
}

#endif
