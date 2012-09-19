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

#include "core/strings/stringFunctions.h"
#include "console/console.h"

#include "gfx/gfxDevice.h"
#include "gfx/gfxPrimitiveBuffer.h"

//-----------------------------------------------------------------------------
#ifdef TORQUE_DEBUG
GFXPrimitiveBuffer *GFXPrimitiveBuffer::smHead = NULL;
U32 GFXPrimitiveBuffer::smActivePBCount = 0;

void GFXPrimitiveBuffer::dumpActivePBs()
{
   if(!smActivePBCount)
   {
      Con::printf("GFXPrimitiveBuffer::dumpActivePBs - no currently active PBs to dump. You are A-OK!");
      return;
   }

   Con::printf("GFXPrimitiveBuffer Usage Report - %d active PBs", smActivePBCount);
   Con::printf("---------------------------------------------------------------");
   Con::printf(" Addr  #idx #prims Profiler Path     RefCount");
   for(GFXPrimitiveBuffer *walk = smHead; walk; walk=walk->mDebugNext)
   {
#if defined(TORQUE_ENABLE_PROFILER)
      Con::printf(" %x  %6d %6d %s %d", walk, walk->mIndexCount, walk->mPrimitiveCount, walk->mDebugCreationPath.c_str(), walk->getRefCount());
#else      
      Con::printf(" %x  %6d %6d %s %d", walk, walk->mIndexCount, walk->mPrimitiveCount, "", walk->getRefCount());
#endif      
   }
   Con::printf("----- dump complete -------------------------------------------");
   AssertFatal(false, "There is a primitive buffer leak, check the log for more details.");
}

#endif

/// The resource should put a description of itself (number of vertices, size/width of texture, etc.) in buffer
const String GFXPrimitiveBuffer::describeSelf() const
{
#if defined(TORQUE_DEBUG) && defined(TORQUE_ENABLE_PROFILER)  
   return String::ToString("indexCount: %6d primCount: %6d refCount: %d path: %s", 
      mIndexCount, mPrimitiveCount, getRefCount(), mDebugCreationPath.c_str());
#else      
   return String::ToString("indexCount: %6d primCount: %6d refCount: %d path: %s", 
      mIndexCount, mPrimitiveCount, getRefCount(), "");
#endif   
}

//-----------------------------------------------------------------------------
// Set
//-----------------------------------------------------------------------------
void GFXPrimitiveBufferHandle::set(GFXDevice *theDevice, U32 indexCount, U32 primitiveCount, GFXBufferType bufferType, String desc)
{
   StrongRefPtr<GFXPrimitiveBuffer>::operator=( theDevice->allocPrimitiveBuffer(indexCount, primitiveCount, bufferType) );

#ifdef TORQUE_DEBUG
   if( desc.isNotEmpty() )
      getPointer()->mDebugCreationPath = desc;
#endif
}
