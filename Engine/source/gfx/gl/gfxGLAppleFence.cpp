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

#include "gfx/gl/gfxGLAppleFence.h"

GFXGLAppleFence::GFXGLAppleFence(GFXDevice* device) : GFXFence(device), mIssued(false)
{
   glGenFencesAPPLE(1, &mHandle);
}

GFXGLAppleFence::~GFXGLAppleFence()
{
   glDeleteFencesAPPLE(1, &mHandle);
}

void GFXGLAppleFence::issue()
{
   glSetFenceAPPLE(mHandle);
   mIssued = true;
}

GFXFence::FenceStatus GFXGLAppleFence::getStatus() const
{
   if(!mIssued)
      return GFXFence::Unset;
      
   GLboolean res = glTestFenceAPPLE(mHandle);
   return res ? GFXFence::Processed : GFXFence::Pending;
}

void GFXGLAppleFence::block()
{
   if(!mIssued)
      return;
      
   glFinishFenceAPPLE(mHandle);
}

void GFXGLAppleFence::zombify()
{
   glDeleteFencesAPPLE(1, &mHandle);
}

void GFXGLAppleFence::resurrect()
{
   glGenFencesAPPLE(1, &mHandle);
}

const String GFXGLAppleFence::describeSelf() const
{
   return String::ToString("   GL Handle: %i", mHandle);
}
