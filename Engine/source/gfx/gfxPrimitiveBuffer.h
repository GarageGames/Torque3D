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

#ifndef _GFXPRIMITIVEBUFFER_H_
#define _GFXPRIMITIVEBUFFER_H_

#ifndef _GFXSTRUCTS_H_
#include "gfx/gfxStructs.h"
#endif

#ifdef TORQUE_ENABLE_PROFILER
#include "platform/profiler.h"
#endif


class GFXPrimitiveBuffer : public StrongRefBase, public GFXResource
{
   friend class GFXPrimitiveBufferHandle;
   friend class GFXDevice;

public: //protected:

   U32 mIndexCount;
   U32 mPrimitiveCount;
   GFXBufferType mBufferType;
   GFXPrimitive *mPrimitiveArray;
   GFXDevice *mDevice;  

#ifdef TORQUE_DEBUG
   // In debug builds we provide a TOC leak tracking system.
   static U32 smActivePBCount;
   static GFXPrimitiveBuffer *smHead;
   static void dumpActivePBs();

   String               mDebugCreationPath;
   GFXPrimitiveBuffer   *mDebugNext;
   GFXPrimitiveBuffer   *mDebugPrev;
#endif

   GFXPrimitiveBuffer(  GFXDevice *device, 
                        U32 indexCount, 
                        U32 primitiveCount, 
                        GFXBufferType bufferType )
   {
      mDevice = device;
      mIndexCount = indexCount;
      mPrimitiveCount = primitiveCount;
      mBufferType = bufferType;
      if(primitiveCount)
      {
         mPrimitiveArray = new GFXPrimitive[primitiveCount];
         dMemset((void *) mPrimitiveArray, 0, primitiveCount * sizeof(GFXPrimitive));
      }
      else
         mPrimitiveArray = NULL;

#if defined(TORQUE_DEBUG)
      // Active copy tracking.
      smActivePBCount++;
#if defined(TORQUE_ENABLE_PROFILE_PATH)
      mDebugCreationPath = gProfiler->getProfilePath();
#endif      
      mDebugNext = smHead;
      mDebugPrev = NULL;

      if(smHead)
      {
         AssertFatal(smHead->mDebugPrev == NULL, "GFXPrimitiveBuffer::GFXPrimitiveBuffer - found unexpected previous in current head!");
         smHead->mDebugPrev = this;
      }

      smHead = this;
#endif
   }

   virtual ~GFXPrimitiveBuffer()
   {
      if( mPrimitiveArray != NULL )
      {
         delete [] mPrimitiveArray;
         mPrimitiveArray = NULL;
      }

#ifdef TORQUE_DEBUG
      if(smHead == this)
         smHead = this->mDebugNext;

      if(mDebugNext)
         mDebugNext->mDebugPrev = mDebugPrev;

      if(mDebugPrev)
         mDebugPrev->mDebugNext = mDebugNext;

      mDebugPrev = mDebugNext = NULL;

      smActivePBCount--;
#endif
   }

   virtual void lock(U32 indexStart, U32 indexEnd, void **indexPtr)=0; ///< locks this primitive buffer for writing into
   virtual void unlock()=0; ///< unlocks this primitive buffer.
   virtual void prepare()=0;  ///< prepares this primitive buffer for use on the device it was allocated on

   // GFXResource interface
   /// The resource should put a description of itself (number of vertices, size/width of texture, etc.) in buffer
   virtual const String describeSelf() const;   
};

class GFXPrimitiveBufferHandle : public StrongRefPtr<GFXPrimitiveBuffer>
{
   typedef StrongRefPtr<GFXPrimitiveBuffer> Parent;
public:
   enum Constants {
      MaxIndexCount = 65535,
   };
   GFXPrimitiveBufferHandle() {};

   GFXPrimitiveBufferHandle(GFXDevice *theDevice, U32 indexCount, U32 primitiveCount, GFXBufferType bufferType, String desc = String::EmptyString )
   {
      set(theDevice, indexCount, primitiveCount, bufferType, desc);
   }

   void set(GFXDevice *theDevice, U32 indexCount, U32 primitiveCount, GFXBufferType bufferType, String desc = String::EmptyString );
   
   void immutable(GFXDevice *theDevice, U32 indexCount, U32 primitiveCount, void* data, String desc = String::EmptyString );

   void lock(U16 **indexBuffer, GFXPrimitive **primitiveBuffer = NULL, U32 indexStart = 0, U32 indexEnd = 0)
   {
      if(indexEnd == 0)
         indexEnd = getPointer()->mIndexCount;
      AssertFatal(indexStart < indexEnd && indexEnd <= getPointer()->mIndexCount, "Out of range index lock!");
      getPointer()->lock(indexStart, indexEnd, (void**)indexBuffer);
      if(primitiveBuffer)
         *primitiveBuffer = getPointer()->mPrimitiveArray;
   }

   void unlock()
   {
      getPointer()->unlock();
   }

   void prepare()
   {
      getPointer()->prepare();
   }

   bool operator==(const GFXPrimitiveBufferHandle &buffer) const {
      return getPointer() == buffer.getPointer();
   }

   GFXPrimitiveBufferHandle& operator=(GFXPrimitiveBuffer *ptr)
   {
      StrongObjectRef::set(ptr);
      return *this;
   }
};

#endif // _GFXPRIMITIVEBUFFER_H_
