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
#include "gfx/D3D11/gfxD3D11Target.h"
#include "gfx/D3D11/gfxD3D11Cubemap.h"
#include "gfx/D3D11/gfxD3D11EnumTranslate.h"
#include "gfx/gfxDebugEvent.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "windowManager/win32/win32Window.h"

GFXD3D11TextureTarget::GFXD3D11TextureTarget() 
   :  mTargetSize( Point2I::Zero ),
      mTargetFormat( GFXFormatR8G8B8A8 )
{
   for(S32 i=0; i<MaxRenderSlotId; i++)
   {
      mTargets[i] = NULL;
      mResolveTargets[i] = NULL;
      mTargetViews[i] = NULL;
      mTargetSRViews[i] = NULL;
   }
}

GFXD3D11TextureTarget::~GFXD3D11TextureTarget()
{
   // Release anything we might be holding.
   for(S32 i=0; i<MaxRenderSlotId; i++)
   {
      mResolveTargets[i] = NULL;
      SAFE_RELEASE(mTargetViews[i]);
      SAFE_RELEASE(mTargets[i]);
      SAFE_RELEASE(mTargetSRViews[i]);      
   }

   zombify();
}

void GFXD3D11TextureTarget::attachTexture( RenderSlot slot, GFXTextureObject *tex, U32 mipLevel/*=0*/, U32 zOffset /*= 0*/ )
{
   GFXDEBUGEVENT_SCOPE( GFXPCD3D11TextureTarget_attachTexture, ColorI::RED );

   AssertFatal(slot < MaxRenderSlotId, "GFXD3D11TextureTarget::attachTexture - out of range slot.");

   // TODO:  The way this is implemented... you can attach a texture 
   // object multiple times and it will release and reset it.
   //
   // We should rework this to detect when no change has occured
   // and skip out early.

   // Mark state as dirty so device can know to update.
   invalidateState();

   // Release what we had, it's definitely going to change.
   SAFE_RELEASE(mTargetViews[slot]);
   SAFE_RELEASE(mTargets[slot]);
   SAFE_RELEASE(mTargetSRViews[slot]);
   
   mResolveTargets[slot] = NULL;

   if(slot == Color0)
   {
      mTargetSize = Point2I::Zero;
      mTargetFormat = GFXFormatR8G8B8A8;
   }

   // Are we clearing?
   if(!tex)
   {
      // Yup - just exit, it'll stay NULL.      
      return;
   }

   // TODO: Mip map generation currently only supported on dynamic cubemaps
   mTargetSRViews[slot] = NULL;

   // Take care of default targets
   if( tex == GFXTextureTarget::sDefaultDepthStencil )
   {
      mTargets[slot] = D3D11->mDeviceDepthStencil;
      mTargetViews[slot] = D3D11->mDeviceDepthStencilView;
      mTargets[slot]->AddRef();
      mTargetViews[slot]->AddRef();
   }
   else
   {
      // Cast the texture object to D3D...
      AssertFatal(static_cast<GFXD3D11TextureObject*>(tex), "GFXD3D11TextureTarget::attachTexture - invalid texture object.");

      GFXD3D11TextureObject *d3dto = static_cast<GFXD3D11TextureObject*>(tex);

      // Grab the surface level.
      if( slot == DepthStencil )
      {       
         mTargets[slot] = d3dto->getSurface();
         if ( mTargets[slot] )
            mTargets[slot]->AddRef();

         mTargetViews[slot] = d3dto->getDSView();
         if( mTargetViews[slot])
            mTargetViews[slot]->AddRef();         

      }
      else
      {         
         // getSurface will almost always return NULL. It will only return non-NULL
         // if the surface that it needs to render to is different than the mip level
         // in the actual texture. This will happen with MSAA.
         if( d3dto->getSurface() == NULL )
         {
            
            mTargets[slot] = d3dto->get2DTex();
            mTargets[slot]->AddRef();
            mTargetViews[slot] = d3dto->getRTView();
            mTargetViews[slot]->AddRef();         
         } 
         else 
         {
            mTargets[slot] = d3dto->getSurface();
            mTargets[slot]->AddRef();
            mTargetViews[slot]->AddRef();
            // Only assign resolve target if d3dto has a surface to give us.
            //
            // That usually means there is an MSAA target involved, which is why
            // the resolve is needed to get the data out of the target.
            mResolveTargets[slot] = d3dto;

            if ( tex && slot == Color0 )
            {
               mTargetSize.set( tex->getSize().x, tex->getSize().y );
               mTargetFormat = tex->getFormat();
            }
         }           
      }

      // Update surface size
      if(slot == Color0)
      {
         ID3D11Texture2D *surface = mTargets[Color0];
         if ( surface )
         {
            D3D11_TEXTURE2D_DESC sd;
            surface->GetDesc(&sd);
            mTargetSize = Point2I(sd.Width, sd.Height);

            S32 format = sd.Format;

            if (format == DXGI_FORMAT_R8G8B8A8_TYPELESS || format == DXGI_FORMAT_B8G8R8A8_TYPELESS)
            {
               mTargetFormat = GFXFormatR8G8B8A8;
               return;
            }

            GFXREVERSE_LOOKUP( GFXD3D11TextureFormat, GFXFormat, format );
            mTargetFormat = (GFXFormat)format;
         }
      }
   }

}


void GFXD3D11TextureTarget::attachTexture( RenderSlot slot, GFXCubemap *tex, U32 face, U32 mipLevel/*=0*/ )
{
   GFXDEBUGEVENT_SCOPE( GFXPCD3D11TextureTarget_attachTexture_Cubemap, ColorI::RED );

   AssertFatal(slot < MaxRenderSlotId, "GFXD3D11TextureTarget::attachTexture - out of range slot.");

   // Mark state as dirty so device can know to update.
   invalidateState();

   // Release what we had, it's definitely going to change.
   SAFE_RELEASE(mTargetViews[slot]);
   SAFE_RELEASE(mTargets[slot]);
   SAFE_RELEASE(mTargetSRViews[slot]);

   mResolveTargets[slot] = NULL;

   // Cast the texture object to D3D...
   AssertFatal(!tex || static_cast<GFXD3D11Cubemap*>(tex), "GFXD3DTextureTarget::attachTexture - invalid cubemap object.");

   if(slot == Color0)
   {
      mTargetSize = Point2I::Zero;
      mTargetFormat = GFXFormatR8G8B8A8;
   }

   // Are we clearing?
   if(!tex)
   {
      // Yup - just exit, it'll stay NULL.      
      return;
   }

   GFXD3D11Cubemap *cube = static_cast<GFXD3D11Cubemap*>(tex);

   mTargets[slot] = cube->get2DTex();
   mTargets[slot]->AddRef();
   mTargetViews[slot] = cube->getRTView(face);
   mTargetViews[slot]->AddRef();
   mTargetSRViews[slot] = cube->getSRView();
   mTargetSRViews[slot]->AddRef();
   
   // Update surface size
   if(slot == Color0)
   {
      ID3D11Texture2D *surface = mTargets[Color0];
      if ( surface )
      {
         D3D11_TEXTURE2D_DESC sd;
         surface->GetDesc(&sd);
         mTargetSize = Point2I(sd.Width, sd.Height);

         S32 format = sd.Format;
         GFXREVERSE_LOOKUP( GFXD3D11TextureFormat, GFXFormat, format );
         mTargetFormat = (GFXFormat)format;
      }
   }

}

void GFXD3D11TextureTarget::activate()
{
   GFXDEBUGEVENT_SCOPE( GFXPCD3D11TextureTarget_activate, ColorI::RED );

   AssertFatal( mTargets[GFXTextureTarget::Color0], "GFXD3D11TextureTarget::activate() - You can never have a NULL primary render target!" );
  
   // Clear the state indicator.
   stateApplied();
   
   // Now set all the new surfaces into the appropriate slots.
   ID3D11RenderTargetView* rtViews[MaxRenderSlotId] = { NULL, NULL, NULL, NULL, NULL, NULL};

   ID3D11DepthStencilView* dsView = (ID3D11DepthStencilView*)(mTargetViews[GFXTextureTarget::DepthStencil]);
   for (U32 i = 0; i < 4; i++)
   {
      rtViews[i] = (ID3D11RenderTargetView*)mTargetViews[GFXTextureTarget::Color0 + i];
   }

   D3D11DEVICECONTEXT->OMSetRenderTargets(MaxRenderSlotId, rtViews, dsView);

}

void GFXD3D11TextureTarget::deactivate()
{
   //re-gen mip maps
   for (U32 i = 0; i < 4; i++)
   {
      ID3D11ShaderResourceView* pSRView = mTargetSRViews[GFXTextureTarget::Color0 + i];
      if (pSRView)
         D3D11DEVICECONTEXT->GenerateMips(pSRView);
   }
   
}

void GFXD3D11TextureTarget::resolve()
{
   GFXDEBUGEVENT_SCOPE( GFXPCD3D11TextureTarget_resolve, ColorI::RED );

   for (U32 i = 0; i < MaxRenderSlotId; i++)
   {
      // We use existance @ mResolveTargets as a flag that we need to copy
      // data from the rendertarget into the texture.
      if (mResolveTargets[i])
      {
         D3D11_TEXTURE2D_DESC desc;
         mTargets[i]->GetDesc(&desc);
         D3D11DEVICECONTEXT->CopySubresourceRegion(mResolveTargets[i]->get2DTex(), 0, 0, 0, 0, mTargets[i], 0, NULL);
      }
   }
}

void GFXD3D11TextureTarget::resolveTo( GFXTextureObject *tex )
{
   GFXDEBUGEVENT_SCOPE( GFXPCD3D11TextureTarget_resolveTo, ColorI::RED );

   if ( mTargets[Color0] == NULL )
      return;

   D3D11_TEXTURE2D_DESC desc;
   mTargets[Color0]->GetDesc(&desc);
   D3D11DEVICECONTEXT->CopySubresourceRegion(((GFXD3D11TextureObject*)(tex))->get2DTex(), 0, 0, 0, 0, mTargets[Color0], 0, NULL);
      
}

void GFXD3D11TextureTarget::zombify()
{
   for(U32 i = 0; i < MaxRenderSlotId; i++)
      attachTexture(RenderSlot(i), NULL);
}

void GFXD3D11TextureTarget::resurrect()
{
}

GFXD3D11WindowTarget::GFXD3D11WindowTarget()
{
   mWindow = NULL;
   mBackBuffer = NULL;
   mDepthStencilView = NULL;
   mDepthStencil = NULL;
   mBackBufferView = NULL;
   mSecondaryWindow = false;
}

GFXD3D11WindowTarget::~GFXD3D11WindowTarget()
{
   SAFE_RELEASE(mDepthStencilView)
   SAFE_RELEASE(mDepthStencil);
   SAFE_RELEASE(mBackBufferView);
   SAFE_RELEASE(mBackBuffer);
   SAFE_RELEASE(mSwapChain);
}

void GFXD3D11WindowTarget::initPresentationParams()
{
   // Get some video mode related info.
   const GFXVideoMode &vm = mWindow->getVideoMode();
   HWND hwnd = (HWND)mWindow->getSystemWindow(PlatformWindow::WindowSystem_Windows);

   // Do some validation...
   if (vm.fullScreen && mSecondaryWindow)
   {
      AssertFatal(false, "GFXD3D11WindowTarget::initPresentationParams - Cannot go fullscreen with secondary window!");
   }

   mPresentationParams = D3D11->setupPresentParams(vm, hwnd);
}

const Point2I GFXD3D11WindowTarget::getSize()
{
   return mWindow->getVideoMode().resolution; 
}

GFXFormat GFXD3D11WindowTarget::getFormat()
{ 
   S32 format = mPresentationParams.BufferDesc.Format;
   GFXREVERSE_LOOKUP( GFXD3D11TextureFormat, GFXFormat, format );
   return (GFXFormat)format;
}

bool GFXD3D11WindowTarget::present()
{
   return (mSwapChain->Present(!D3D11->smDisableVSync, 0) == S_OK);
}

void GFXD3D11WindowTarget::createSwapChain()
{
   //create dxgi factory & swapchain
   IDXGIFactory1* DXGIFactory;
   HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&DXGIFactory));
   if (FAILED(hr))
      AssertFatal(false, "GFXD3D11WindowTarget::createSwapChain - couldn't create dxgi factory.");

   hr = DXGIFactory->CreateSwapChain(D3D11DEVICE, &mPresentationParams, &mSwapChain);

   if (FAILED(hr))
      AssertFatal(false, "GFXD3D11WindowTarget::createSwapChain - couldn't create swap chain.");

   SAFE_RELEASE(DXGIFactory);   
}

void GFXD3D11WindowTarget::createBuffersAndViews()
{
   //release old if they exist
   SAFE_RELEASE(mDepthStencilView);
   SAFE_RELEASE(mDepthStencil);
   SAFE_RELEASE(mBackBufferView);
   SAFE_RELEASE(mBackBuffer);

   //grab video mode
   const GFXVideoMode &vm = mWindow->getVideoMode();
   //create depth/stencil
   D3D11_TEXTURE2D_DESC desc;
   desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
   desc.CPUAccessFlags = 0;
   desc.Format = GFXD3D11TextureFormat[GFXFormatD24S8];
   desc.MipLevels = 1;
   desc.ArraySize = 1;
   desc.Usage = D3D11_USAGE_DEFAULT;
   desc.Width = vm.resolution.x;
   desc.Height = vm.resolution.y;
   desc.SampleDesc.Count = 1;
   desc.SampleDesc.Quality = 0;
   desc.MiscFlags = 0;

   HRESULT hr = D3D11DEVICE->CreateTexture2D(&desc, NULL, &mDepthStencil);
   if (FAILED(hr))
      AssertFatal(false, "GFXD3D11WindowTarget::createBuffersAndViews - couldn't create device's depth-stencil surface.");

   D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
   depthDesc.Format = GFXD3D11TextureFormat[GFXFormatD24S8];
   depthDesc.Flags = 0;
   depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
   depthDesc.Texture2D.MipSlice = 0;

   hr = D3D11DEVICE->CreateDepthStencilView(mDepthStencil, &depthDesc, &mDepthStencilView);
   if (FAILED(hr))
      AssertFatal(false, "GFXD3D11WindowTarget::createBuffersAndViews - couldn't create depth stencil view");

   setBackBuffer();

   //create back buffer view
   D3D11_RENDER_TARGET_VIEW_DESC RTDesc;
   RTDesc.Format = GFXD3D11TextureFormat[GFXFormatR8G8B8A8];
   RTDesc.Texture2D.MipSlice = 0;
   RTDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

   hr = D3D11DEVICE->CreateRenderTargetView(mBackBuffer, &RTDesc, &mBackBufferView);

   if (FAILED(hr))
      AssertFatal(false, "GFXD3D11WindowTarget::createBuffersAndViews - couldn't create back buffer target view");

   //debug names
#ifdef TORQUE_DEBUG
   if (!mSecondaryWindow)
   {
      String backBufferName = "MainBackBuffer";
      String depthSteniclName = "MainDepthStencil";
      String backBuffViewName = "MainBackBuffView";
      String depthStencViewName = "MainDepthView";
      mBackBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, backBufferName.size(), backBufferName.c_str());
      mDepthStencil->SetPrivateData(WKPDID_D3DDebugObjectName, depthSteniclName.size(), depthSteniclName.c_str());
      mDepthStencilView->SetPrivateData(WKPDID_D3DDebugObjectName, depthStencViewName.size(), depthStencViewName.c_str());
      mBackBufferView->SetPrivateData(WKPDID_D3DDebugObjectName, backBuffViewName.size(), backBuffViewName.c_str());
   }
#endif
}

void GFXD3D11WindowTarget::resetMode()
{
   HRESULT hr;
   if (mSwapChain)
   {
      // The current video settings.
      DXGI_SWAP_CHAIN_DESC desc;
      hr = mSwapChain->GetDesc(&desc);
      if (FAILED(hr))
         AssertFatal(false, "GFXD3D11WindowTarget::resetMode - failed to get swap chain description!");

      bool fullscreen = !desc.Windowed;
      Point2I backbufferSize(desc.BufferDesc.Width, desc.BufferDesc.Height);

      // The settings we are now applying.
      const GFXVideoMode &vm = mWindow->getVideoMode();

      // Early out if none of the settings which require a device reset
      // have changed.      
      if (backbufferSize == vm.resolution &&
         fullscreen == vm.fullScreen)
         return;
   }

   //release old buffers and views
   SAFE_RELEASE(mDepthStencilView)
   SAFE_RELEASE(mDepthStencil);
   SAFE_RELEASE(mBackBufferView);
   SAFE_RELEASE(mBackBuffer);

   if(!mSecondaryWindow)
      D3D11->beginReset();

   mWindow->setSuppressReset(true);

   // Setup our presentation params.
   initPresentationParams();

   if (!mPresentationParams.Windowed)
   {
      mPresentationParams.BufferDesc.RefreshRate.Numerator = 0;
      mPresentationParams.BufferDesc.RefreshRate.Denominator = 0;
      hr = mSwapChain->ResizeTarget(&mPresentationParams.BufferDesc);

      if (FAILED(hr))
         AssertFatal(false, "GFXD3D11WindowTarget::resetMode - failed to resize target!");

   }

   hr = mSwapChain->ResizeBuffers(mPresentationParams.BufferCount, mPresentationParams.BufferDesc.Width, mPresentationParams.BufferDesc.Height, 
      mPresentationParams.BufferDesc.Format, mPresentationParams.Windowed ? 0 : DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

   if (FAILED(hr))
      AssertFatal(false, "GFXD3D11WindowTarget::resetMode - failed to resize back buffer!");

   hr = mSwapChain->SetFullscreenState(!mPresentationParams.Windowed, NULL);

   if (FAILED(hr))
      AssertFatal(false, "GFXD3D11WindowTarget::resetMode - failed to change screen states!");

   // Update our size, too.
   mSize = Point2I(mPresentationParams.BufferDesc.Width, mPresentationParams.BufferDesc.Height);

   mWindow->setSuppressReset(false);

   //re-create buffers and views
   createBuffersAndViews();

   if (!mSecondaryWindow)
      D3D11->endReset(this);
}

void GFXD3D11WindowTarget::zombify()
{
   SAFE_RELEASE(mBackBuffer);
}

void GFXD3D11WindowTarget::resurrect()
{
   setBackBuffer();
}

void GFXD3D11WindowTarget::setBackBuffer()
{
   if (!mBackBuffer)
      mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&mBackBuffer);
}

void GFXD3D11WindowTarget::activate()
{
   GFXDEBUGEVENT_SCOPE(GFXPCD3D11WindowTarget_activate, ColorI::RED);

   //clear ther rendertargets first
   ID3D11RenderTargetView* rtViews[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

   D3D11DEVICECONTEXT->OMSetRenderTargets(8, rtViews, NULL);
   D3D11DEVICECONTEXT->OMSetRenderTargets(1, &mBackBufferView, mDepthStencilView);

   DXGI_SWAP_CHAIN_DESC pp;
   mSwapChain->GetDesc(&pp);

   // Update our video mode here, too.
   GFXVideoMode vm;
   vm = mWindow->getVideoMode();
   vm.resolution.x = pp.BufferDesc.Width;
   vm.resolution.y = pp.BufferDesc.Height;
   vm.fullScreen = !pp.Windowed;
   mSize = vm.resolution;
}

void GFXD3D11WindowTarget::resolveTo(GFXTextureObject *tex)
{
   GFXDEBUGEVENT_SCOPE(GFXPCD3D11WindowTarget_resolveTo, ColorI::RED);

   D3D11_TEXTURE2D_DESC desc;
   ID3D11Texture2D* surf = ((GFXD3D11TextureObject*)(tex))->get2DTex();
   surf->GetDesc(&desc);
   D3D11DEVICECONTEXT->ResolveSubresource(surf, 0, mBackBuffer, 0, desc.Format);
}

IDXGISwapChain *GFXD3D11WindowTarget::getSwapChain()
{
   mSwapChain->AddRef();
   return mSwapChain;
}

ID3D11Texture2D *GFXD3D11WindowTarget::getBackBuffer()
{
   mBackBuffer->AddRef();
   return mBackBuffer;
}

ID3D11Texture2D *GFXD3D11WindowTarget::getDepthStencil()
{
   mDepthStencil->AddRef();
   return mDepthStencil;
}

ID3D11RenderTargetView* GFXD3D11WindowTarget::getBackBufferView()
{
   mBackBufferView->AddRef();
   return mBackBufferView;
}

ID3D11DepthStencilView* GFXD3D11WindowTarget::getDepthStencilView()
{
   mDepthStencilView->AddRef();
   return mDepthStencilView;
}