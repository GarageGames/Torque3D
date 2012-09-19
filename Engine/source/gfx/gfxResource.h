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

#ifndef _GFXRESOURCE_H_
#define _GFXRESOURCE_H_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif

class GFXDevice;

/// Mixin for the purpose of tracking GFX resources owned by a GFXDevice.
///
/// There are many types of resource that are allocated from a GFXDevice that
/// must be participatory in device resets. For instance, all default pool
/// DirectX resources have to be involved when the device resets. Render
/// targets in all APIs need to unbind themselves when resets happen.
///
/// This system is also handy for accounting purposes. For instance, we may
/// want to traverse all registered VBs, IBs, Textures, or RTs in order to
/// determine what, if any, items are still allocated. This can be used in
/// leak reports, memory usage reports, etc.
class GFXResource
{
private:
   friend class GFXDevice;

   GFXResource *mPrevResource;
   GFXResource *mNextResource;
   GFXDevice   *mOwningDevice;

   /// Helper flag to check new resource allocations
   bool mFlagged;

public:
   GFXResource();
   virtual ~GFXResource();

   /// Registers this resource with the given device
   void registerResourceWithDevice(GFXDevice *device);

   /// When called the resource should destroy all device sensitive information (e.g. D3D resources in D3DPOOL_DEFAULT
   virtual void zombify()=0;

   /// When called the resource should restore all device sensitive information destroyed by zombify()
   virtual void resurrect()=0;

   /// The resource should put a description of itself (number of vertices, size/width of texture, etc.) in buffer
   virtual const String describeSelf() const = 0;

   inline GFXResource *getNextResource() const
   {
      return mNextResource;
   }

   inline GFXResource *getPrevResource() const
   {
      return mPrevResource;
   }

   inline GFXDevice *getOwningDevice() const
   {
      return mOwningDevice;
   }

   inline bool isFlagged()
   {
      return mFlagged;
   }

   inline void setFlag()
   {
      mFlagged = true;
   }

   inline void clearFlag()
   {
      mFlagged = false;
   }
};

#endif