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
#ifndef _GFX_D3D_GFXD3D9TARGET_H_
#define _GFX_D3D_GFXD3D9TARGET_H_

#ifndef _GFXTARGET_H_
#include "gfx/gfxTarget.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MPOINT2_H_
#include "math/mPoint2.h"
#endif
#include <d3d9.h>

struct IDirect3DSurface9;
struct IDirect3DSwapChain9;
class GFXD3D9TextureObject;


class GFXPCD3D9TextureTarget : public GFXTextureTarget
{
   friend class GFXPCD3D9Device;

   // Array of target surfaces, this is given to us by attachTexture
   IDirect3DSurface9 * mTargets[MaxRenderSlotId];

   // Array of texture objects which correspond to the target surfaces above,
   // needed for copy from RenderTarget to texture situations.  Current only valid in those situations
   GFXD3D9TextureObject* mResolveTargets[MaxRenderSlotId];

   /// Owning d3d device.
   GFXD3D9Device *mDevice;

   Point2I mTargetSize;

   GFXFormat mTargetFormat;

public:

   GFXPCD3D9TextureTarget();
   ~GFXPCD3D9TextureTarget();

   // Public interface.
   virtual const Point2I getSize() { return mTargetSize; }
   virtual GFXFormat getFormat() { return mTargetFormat; }
   virtual void attachTexture(RenderSlot slot, GFXTextureObject *tex, U32 mipLevel=0, U32 zOffset = 0);
   virtual void attachTexture(RenderSlot slot, GFXCubemap *tex, U32 face, U32 mipLevel=0);
   virtual void resolve();

   /// Note we always copy the Color0 RenderSlot.
   virtual void resolveTo( GFXTextureObject *tex );

   virtual void activate();
   virtual void deactivate();

   void zombify();
   void resurrect();
};

class GFXPCD3D9WindowTarget : public GFXWindowTarget
{
   friend class GFXPCD3D9Device;

   /// Our depth stencil buffer, if any.
   IDirect3DSurface9 *mDepthStencil;

   /// Our backbuffer
   IDirect3DSurface9 *mBackbuffer;

   /// Maximum size we can render to.
   Point2I mSize;

   /// Our swap chain, potentially the implicit device swap chain.
   IDirect3DSwapChain9 *mSwapChain;

   /// D3D presentation info.
   D3DPRESENT_PARAMETERS mPresentationParams;

   /// Owning d3d device.
   GFXD3D9Device  *mDevice;

   /// Is this the implicit swap chain?
   bool mImplicit;

   /// Internal interface that notifies us we need to reset our video mode.
   void resetMode();

public:

   GFXPCD3D9WindowTarget();
   ~GFXPCD3D9WindowTarget();
 
   virtual const Point2I getSize();
   virtual GFXFormat getFormat();
   virtual bool present();

   void initPresentationParams();
   void setImplicitSwapChain();
   void createAdditionalSwapChain();

   virtual void activate();   

   void zombify();
   void resurrect();

   virtual void resolveTo( GFXTextureObject *tex );
};

#endif // _GFX_D3D_GFXD3D9TARGET_H_
