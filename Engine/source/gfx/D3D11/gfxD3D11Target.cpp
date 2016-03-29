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
   mWindow       = NULL;
   mBackbuffer   = NULL;
}

GFXD3D11WindowTarget::~GFXD3D11WindowTarget()
{
   SAFE_RELEASE(mBackbuffer);
}

void GFXD3D11WindowTarget::initPresentationParams()
{
   // Get some video mode related info.
   GFXVideoMode vm = mWindow->getVideoMode();
   Win32Window* win = static_cast<Win32Window*>(mWindow);
   HWND hwnd = win->getHWND();

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
   return (D3D11->getSwapChain()->Present(!D3D11->smDisableVSync, 0) == S_OK);
}

void GFXD3D11WindowTarget::setImplicitSwapChain()
{
   if (!mBackbuffer)      
      D3D11->mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&mBackbuffer);
}

void GFXD3D11WindowTarget::resetMode()
{
   mWindow->setSuppressReset(true);

   // Setup our presentation params.
   initPresentationParams();

   // Otherwise, we have to reset the device, if we're the implicit swapchain.
   D3D11->reset(mPresentationParams);

   // Update our size, too.
   mSize = Point2I(mPresentationParams.BufferDesc.Width, mPresentationParams.BufferDesc.Height);

   mWindow->setSuppressReset(false);
   GFX->beginReset();
}

void GFXD3D11WindowTarget::zombify()
{
   SAFE_RELEASE(mBackbuffer);
}

void GFXD3D11WindowTarget::resurrect()
{
   setImplicitSwapChain();
}

void GFXD3D11WindowTarget::activate()
{
   GFXDEBUGEVENT_SCOPE(GFXPCD3D11WindowTarget_activate, ColorI::RED);

   //clear ther rendertargets first
   ID3D11RenderTargetView* rtViews[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

   D3D11DEVICECONTEXT->OMSetRenderTargets(8, rtViews, NULL);
   D3D11DEVICECONTEXT->OMSetRenderTargets(1, &D3D11->mDeviceBackBufferView, D3D11->mDeviceDepthStencilView);

   DXGI_SWAP_CHAIN_DESC pp;
   D3D11->mSwapChain->GetDesc(&pp);

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
	D3D11DEVICECONTEXT->ResolveSubresource(surf, 0, D3D11->mDeviceBackbuffer, 0, desc.Format);
}