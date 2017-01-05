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

#ifndef _GFX_D3D_GFXD3D11TARGET_H_
#define _GFX_D3D_GFXD3D11TARGET_H_

#include "gfx/D3D11/gfxD3D11Device.h"
#include "gfx/D3D11/gfxD3D11TextureObject.h"
#include "gfx/gfxTarget.h"
#include "math/mPoint3.h"
#include "math/mPoint2.h"

class GFXD3D11TextureTarget : public GFXTextureTarget
{
   friend class GFXD3D11Device;

   // Array of target surfaces, this is given to us by attachTexture
   ID3D11Texture2D* mTargets[MaxRenderSlotId];

   // Array of shader resource views
   ID3D11ShaderResourceView* mTargetSRViews[MaxRenderSlotId];
   
   //ID3D11DepthStencilView* mDepthTargetView;
   ID3D11View* mTargetViews[MaxRenderSlotId];
   // Array of texture objects which correspond to the target surfaces above,
   // needed for copy from RenderTarget to texture situations.  Current only valid in those situations
   GFXD3D11TextureObject* mResolveTargets[MaxRenderSlotId];

   Point2I mTargetSize;

   GFXFormat mTargetFormat;

public:

   GFXD3D11TextureTarget();
   ~GFXD3D11TextureTarget();

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

class GFXD3D11WindowTarget : public GFXWindowTarget
{
   friend class GFXD3D11Device;

   /// Our backbuffer
   ID3D11Texture2D *mBackBuffer;
   ID3D11Texture2D *mDepthStencil;
   ID3D11RenderTargetView* mBackBufferView;
   ID3D11DepthStencilView* mDepthStencilView;
   IDXGISwapChain *mSwapChain;

   /// Maximum size we can render to.
   Point2I mSize;
   /// D3D presentation info.
   DXGI_SWAP_CHAIN_DESC mPresentationParams;
   /// Internal interface that notifies us we need to reset our video mode.
   void resetMode();

   /// Is this a secondary window
   bool mSecondaryWindow;

public:

   GFXD3D11WindowTarget();
   ~GFXD3D11WindowTarget();
 
   virtual const Point2I getSize();
   virtual GFXFormat getFormat();
   virtual bool present();

   void initPresentationParams();
   void createSwapChain();
   void createBuffersAndViews();
   void setBackBuffer();

   virtual void activate();   

   void zombify();
   void resurrect();

   virtual void resolveTo( GFXTextureObject *tex );

   // These are all reference counted and must be released by whomever uses the get* function
   IDXGISwapChain *getSwapChain();
   ID3D11Texture2D *getBackBuffer();
   ID3D11Texture2D *getDepthStencil();
   ID3D11RenderTargetView* getBackBufferView();
   ID3D11DepthStencilView* getDepthStencilView();
};

#endif
