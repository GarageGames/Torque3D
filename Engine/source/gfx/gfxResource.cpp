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

#include "gfx/gfxResource.h"
#include "gfx/gfxDevice.h"

GFXResource::GFXResource()
{
   mPrevResource = mNextResource = NULL;
   mOwningDevice = NULL;
   mFlagged = false;
}

GFXResource::~GFXResource()
{
   // Make sure we're not the head of the list and referencd on the device.
   if(mOwningDevice && mOwningDevice->mResourceListHead == this)
   {
      AssertFatal(mPrevResource == NULL, 
         "GFXResource::~GFXResource - head of list but have a previous item!");
      mOwningDevice->mResourceListHead = mNextResource;
   }

   // Unlink ourselves from the list.
   if(mPrevResource)
      mPrevResource->mNextResource = mNextResource;
   if(mNextResource)
      mNextResource->mPrevResource = mPrevResource;

   mPrevResource = mNextResource = NULL;
}

void GFXResource::registerResourceWithDevice( GFXDevice *device )
{
   mOwningDevice = device;
   mNextResource = device->mResourceListHead;
   device->mResourceListHead = this;

   if(mNextResource)
      mNextResource->mPrevResource = this;
}