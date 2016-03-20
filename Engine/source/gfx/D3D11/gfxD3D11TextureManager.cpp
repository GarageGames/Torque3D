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

#include "gfx/D3D11/gfxD3D11Device.h"
#include "gfx/D3D11/gfxD3D11EnumTranslate.h"
#include "gfx/bitmap/bitmapUtils.h"
#include "gfx/gfxCardProfile.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "core/strings/unicode.h"
#include "core/util/swizzle.h"
#include "core/util/safeDelete.h"
#include "console/console.h"
#include "core/resourceManager.h"

GFXD3D11TextureManager::GFXD3D11TextureManager()
{
   ZeroMemory(mCurTexSet, sizeof(mCurTexSet));
}

GFXD3D11TextureManager::~GFXD3D11TextureManager()
{
   // Destroy texture table now so just in case some texture objects
   // are still left, we don't crash on a pure virtual method call.
   SAFE_DELETE_ARRAY( mHashTable );
}

void GFXD3D11TextureManager::_innerCreateTexture( GFXD3D11TextureObject *retTex, 
                                               U32 height, 
                                               U32 width, 
                                               U32 depth,
                                               GFXFormat format, 
                                               GFXTextureProfile *profile, 
                                               U32 numMipLevels,
                                               bool forceMips,
                                               S32 antialiasLevel)
{
   U32 usage = 0;
   U32 bindFlags = 0;
   U32 miscFlags = 0;
   
   if(!retTex->mProfile->isZTarget() && !retTex->mProfile->isSystemMemory())
      bindFlags =  D3D11_BIND_SHADER_RESOURCE;
   
   U32 cpuFlags = 0;

   retTex->mProfile = profile;
   retTex->isManaged = false;
   DXGI_FORMAT d3dTextureFormat = GFXD3D11TextureFormat[format];

   if( retTex->mProfile->isDynamic() )
   {
      usage = D3D11_USAGE_DYNAMIC;
      cpuFlags |= D3D11_CPU_ACCESS_WRITE;
      retTex->isManaged = false;      
   }
   else if ( retTex->mProfile->isSystemMemory() )
   {
      usage |= D3D11_USAGE_STAGING;
      cpuFlags |= D3D11_CPU_ACCESS_READ;
   }
   else
   {
      usage = D3D11_USAGE_DEFAULT;
      retTex->isManaged = true;
   }

   if( retTex->mProfile->isRenderTarget() )
   {
      bindFlags |= D3D11_BIND_RENDER_TARGET;
      //need to check to make sure this format supports render targets
      U32 supportFlag = 0;
      
      D3D11DEVICE->CheckFormatSupport(d3dTextureFormat, &supportFlag);
      //if it doesn't support render targets then default to R8G8B8A8
      if(!(supportFlag & D3D11_FORMAT_SUPPORT_RENDER_TARGET))
         d3dTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

      retTex->isManaged =false;
   }

   if( retTex->mProfile->isZTarget() )
   {
      bindFlags |= D3D11_BIND_DEPTH_STENCIL;
      retTex->isManaged = false;
   }

   if( !forceMips && !retTex->mProfile->isSystemMemory() &&
       numMipLevels == 0 &&
       !(depth > 0) )
   {
      miscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
      bindFlags |= D3D11_BIND_RENDER_TARGET; // in order to automatically generate mips. Resource needs to be a rendertarget and shader resource
   }

   if( depth > 0 )
   {
      D3D11_TEXTURE3D_DESC desc;
      ZeroMemory(&desc, sizeof(D3D11_TEXTURE3D_DESC));

		desc.BindFlags = bindFlags;
		desc.CPUAccessFlags = cpuFlags;
		desc.Depth = depth;
		desc.Width = width;
		desc.Height = height;
		desc.Format = d3dTextureFormat;
		desc.Usage = (D3D11_USAGE)usage;
		desc.MipLevels = numMipLevels;

		HRESULT hr = D3D11DEVICE->CreateTexture3D(&desc, NULL, retTex->get3DTexPtr());

      if(FAILED(hr)) 
      {
         AssertFatal(false, "GFXD3D11TextureManager::_createTexture - failed to create volume texture!");
      }

      retTex->mTextureSize.set(width, height, depth);
      retTex->get3DTex()->GetDesc(&desc);
      retTex->mMipLevels = numMipLevels;
      retTex->mFormat = format;
   }
   else
   {
		UINT numQualityLevels = 0;

		switch (antialiasLevel)
		{
			case 0:
			case AA_MATCH_BACKBUFFER:
				antialiasLevel = 1;
				break;

			default:
			{
				antialiasLevel = 0;
				UINT numQualityLevels;
				D3D11DEVICE->CheckMultisampleQualityLevels(d3dTextureFormat, antialiasLevel, &numQualityLevels);
				AssertFatal(numQualityLevels, "Invalid AA level!");
				break;
			}
		}

		if(retTex->mProfile->isZTarget())
		{
			D3D11_TEXTURE2D_DESC desc;
		  
			ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
			desc.ArraySize = 1;
			desc.BindFlags = bindFlags;
			desc.CPUAccessFlags = cpuFlags;
			//depth stencil must be a typeless format if it is bound on render target and shader resource simultaneously
			// we'll send the real format for the creation of the views
			desc.Format =  DXGI_FORMAT_R24G8_TYPELESS; 
			desc.MipLevels = numMipLevels;
			desc.SampleDesc.Count = antialiasLevel;
			desc.SampleDesc.Quality = numQualityLevels;
			desc.Height = height;
			desc.Width = width;
			desc.Usage = (D3D11_USAGE)usage;
			HRESULT hr = D3D11DEVICE->CreateTexture2D(&desc, NULL, retTex->getSurfacePtr());
		   
			if(FAILED(hr)) 
			{
				AssertFatal(false, "Failed to create Zbuffer texture");
			}

			retTex->mFormat = format; // Assigning format like this should be fine.
		}
		else
		{
			D3D11_TEXTURE2D_DESC desc;
		  
			ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
			desc.ArraySize = 1;
			desc.BindFlags = bindFlags;
			desc.CPUAccessFlags = cpuFlags;
			desc.Format = d3dTextureFormat;
			desc.MipLevels = numMipLevels;
			desc.SampleDesc.Count = antialiasLevel;
			desc.SampleDesc.Quality = numQualityLevels;
			desc.Height = height;
			desc.Width = width;
			desc.Usage = (D3D11_USAGE)usage;
			desc.MiscFlags = miscFlags;
			HRESULT hr = D3D11DEVICE->CreateTexture2D(&desc, NULL, retTex->get2DTexPtr());

			if(FAILED(hr)) 
			{
				AssertFatal(false, "GFXD3D11TextureManager::_createTexture - failed to create texture!");
			}

			retTex->get2DTex()->GetDesc(&desc);
			retTex->mMipLevels = desc.MipLevels;
		}

		// start creating the resource views...
		// don't bother creating views for system memory/staging textures 
		// they are just used for copying

		if (!retTex->mProfile->isSystemMemory())
		{
         createResourceView(height, width, depth, d3dTextureFormat, numMipLevels, bindFlags, retTex);
		}

		// Get the actual size of the texture...
		D3D11_TEXTURE2D_DESC probeDesc;
		ZeroMemory(&probeDesc, sizeof(D3D11_TEXTURE2D_DESC));
	  
		if( retTex->get2DTex() != NULL )
		{
			retTex->get2DTex()->GetDesc(&probeDesc);
		}
		else if( retTex->getSurface() != NULL )
		{
			retTex->getSurface()->GetDesc(&probeDesc);
		}

		retTex->mTextureSize.set(probeDesc.Width, probeDesc.Height, 0);
		S32 fmt = 0;

		if(!profile->isZTarget())
		   fmt = probeDesc.Format;
		else
		   fmt = DXGI_FORMAT_D24_UNORM_S8_UINT; // we need to assign this manually.

		GFXREVERSE_LOOKUP( GFXD3D11TextureFormat, GFXFormat, fmt );
		retTex->mFormat = (GFXFormat)fmt;
	}
}

//-----------------------------------------------------------------------------
// createTexture
//-----------------------------------------------------------------------------
GFXTextureObject *GFXD3D11TextureManager::_createTextureObject( U32 height, 
                                                               U32 width,
                                                               U32 depth,
                                                               GFXFormat format, 
                                                               GFXTextureProfile *profile, 
                                                               U32 numMipLevels,
                                                               bool forceMips, 
                                                               S32 antialiasLevel,
                                                               GFXTextureObject *inTex )
{
   GFXD3D11TextureObject *retTex;
   if ( inTex )
   {
      AssertFatal(static_cast<GFXD3D11TextureObject*>( inTex ), "GFXD3D11TextureManager::_createTexture() - Bad inTex type!");
      retTex = static_cast<GFXD3D11TextureObject*>( inTex );
      retTex->release();
   }      
   else
   {
      retTex = new GFXD3D11TextureObject(GFX, profile);
      retTex->registerResourceWithDevice(GFX);
   }

   _innerCreateTexture(retTex, height, width, depth, format, profile, numMipLevels, forceMips, antialiasLevel);

   return retTex;
}

bool GFXD3D11TextureManager::_loadTexture(GFXTextureObject *aTexture, GBitmap *pDL)
{
   PROFILE_SCOPE(GFXD3D11TextureManager_loadTexture);

   GFXD3D11TextureObject *texture = static_cast<GFXD3D11TextureObject*>(aTexture);

   // Check with profiler to see if we can do automatic mipmap generation.
   const bool supportsAutoMips = GFX->getCardProfiler()->queryProfile("autoMipMapLevel", true);

   // Helper bool
   const bool isCompressedTexFmt = aTexture->mFormat >= GFXFormatDXT1 && aTexture->mFormat <= GFXFormatDXT5;

   // Settings for mipmap generation
   U32 maxDownloadMip = pDL->getNumMipLevels();
   U32 nbMipMapLevel  = pDL->getNumMipLevels();

   if( supportsAutoMips && !isCompressedTexFmt )
   {
      maxDownloadMip = 1;
      nbMipMapLevel  = aTexture->mMipLevels;
   }
   GFXD3D11Device* dev = D3D11;

   bool isDynamic = texture->mProfile->isDynamic();
   // Fill the texture...
   for( U32 i = 0; i < maxDownloadMip; i++ )
   {
	   U32 subResource = D3D11CalcSubresource(i, 0, aTexture->mMipLevels);

	   if(!isDynamic)
	   {
		   U8* copyBuffer = NULL;

		   switch(texture->mFormat)
			{
				case GFXFormatR8G8B8:
				{
					PROFILE_SCOPE(Swizzle24_Upload);
					AssertFatal(pDL->getFormat() == GFXFormatR8G8B8, "Assumption failed");

					U8* Bits = new U8[pDL->getWidth(i) * pDL->getHeight(i) * 4];
					dMemcpy(Bits, pDL->getBits(i), pDL->getWidth(i) * pDL->getHeight(i) * 3);
					bitmapConvertRGB_to_RGBX(&Bits, pDL->getWidth(i) * pDL->getHeight(i));
					copyBuffer = new U8[pDL->getWidth(i) * pDL->getHeight(i) * 4];
					
					dev->getDeviceSwizzle32()->ToBuffer(copyBuffer, Bits, pDL->getWidth(i) * pDL->getHeight(i) * 4);
					dev->getDeviceContext()->UpdateSubresource(texture->get2DTex(), subResource, NULL, copyBuffer, pDL->getWidth() * 4, pDL->getHeight() *4);
               SAFE_DELETE_ARRAY(Bits);
					break;
				}

				case GFXFormatR8G8B8A8:
				case GFXFormatR8G8B8X8:
				{
               PROFILE_SCOPE(Swizzle32_Upload);
               copyBuffer = new U8[pDL->getWidth(i) * pDL->getHeight(i) * pDL->getBytesPerPixel()];
               dev->getDeviceSwizzle32()->ToBuffer(copyBuffer, pDL->getBits(i), pDL->getWidth(i) * pDL->getHeight(i) * pDL->getBytesPerPixel());
               dev->getDeviceContext()->UpdateSubresource(texture->get2DTex(), subResource, NULL, copyBuffer, pDL->getWidth() * pDL->getBytesPerPixel(), pDL->getHeight() *pDL->getBytesPerPixel());
					break;
				}

				default:
				{
               // Just copy the bits in no swizzle or padding
               PROFILE_SCOPE(SwizzleNull_Upload);
               AssertFatal( pDL->getFormat() == texture->mFormat, "Format mismatch");
               dev->getDeviceContext()->UpdateSubresource(texture->get2DTex(), subResource, NULL, pDL->getBits(i), pDL->getWidth() *pDL->getBytesPerPixel(), pDL->getHeight() *pDL->getBytesPerPixel());
				}
			}

         SAFE_DELETE_ARRAY(copyBuffer);
	    }
	  
	   else
	   {
			D3D11_MAPPED_SUBRESOURCE mapping;
			HRESULT res =  dev->getDeviceContext()->Map(texture->get2DTex(), subResource, D3D11_MAP_WRITE, 0, &mapping);

			AssertFatal(res, "tex2d map call failure");

			switch( texture->mFormat )
			{
				case GFXFormatR8G8B8:
				{
					PROFILE_SCOPE(Swizzle24_Upload);
					AssertFatal(pDL->getFormat() == GFXFormatR8G8B8, "Assumption failed");

					U8* Bits = new U8[pDL->getWidth(i) * pDL->getHeight(i) * 4];
					dMemcpy(Bits, pDL->getBits(i), pDL->getWidth(i) * pDL->getHeight(i) * 3);
					bitmapConvertRGB_to_RGBX(&Bits, pDL->getWidth(i) * pDL->getHeight(i));					

					dev->getDeviceSwizzle32()->ToBuffer(mapping.pData, Bits, pDL->getWidth(i) * pDL->getHeight(i) * 4);
               SAFE_DELETE_ARRAY(Bits);
				}
				break;

            case GFXFormatR8G8B8A8:
            case GFXFormatR8G8B8X8:
            {
               PROFILE_SCOPE(Swizzle32_Upload);
               dev->getDeviceSwizzle32()->ToBuffer(mapping.pData, pDL->getBits(i), pDL->getWidth(i) * pDL->getHeight(i) * pDL->getBytesPerPixel());
            }
				break;

				default:
				{
               // Just copy the bits in no swizzle or padding
               PROFILE_SCOPE(SwizzleNull_Upload);
               AssertFatal( pDL->getFormat() == texture->mFormat, "Format mismatch");
               dMemcpy(mapping.pData, pDL->getBits(i), pDL->getWidth(i) * pDL->getHeight(i) * pDL->getBytesPerPixel());
				}
			}

			dev->getDeviceContext()->Unmap(texture->get2DTex(), subResource);
	   }
   }

   D3D11_TEXTURE2D_DESC desc;
   // if the texture asked for mip generation. lets generate it.
   texture->get2DTex()->GetDesc(&desc);
   if (desc.MiscFlags &D3D11_RESOURCE_MISC_GENERATE_MIPS)
   {
      dev->getDeviceContext()->GenerateMips(texture->getSRView());
      //texture->mMipLevels = desc.MipLevels;
   }

   return true;          
}

bool GFXD3D11TextureManager::_loadTexture(GFXTextureObject *inTex, void *raw)
{
   PROFILE_SCOPE(GFXD3D11TextureManager_loadTextureRaw);

   GFXD3D11TextureObject *texture = (GFXD3D11TextureObject *) inTex;
   GFXD3D11Device* dev = static_cast<GFXD3D11Device *>(GFX);
   // currently only for volume textures...
   if(texture->getDepth() < 1) return false;

   U8* Bits = NULL;
  
   if(texture->mFormat == GFXFormatR8G8B8)
   {
	   // convert 24 bit to 32 bit
	   Bits = new U8[texture->getWidth() * texture->getHeight() * texture->getDepth() * 4];
	   dMemcpy(Bits, raw, texture->getWidth() * texture->getHeight() * texture->getDepth() * 3);
	   bitmapConvertRGB_to_RGBX(&Bits, texture->getWidth() * texture->getHeight() * texture->getDepth());      
   }

   U32 bytesPerPix = 1;

   switch(texture->mFormat)
   {
      case GFXFormatR8G8B8:
      case GFXFormatR8G8B8A8:
      case GFXFormatR8G8B8X8:
         bytesPerPix = 4;
         break;
   }

   D3D11_BOX box;
   box.left    = 0;
   box.right   = texture->getWidth();
   box.front   = 0;
   box.back    = texture->getDepth();
   box.top     = 0;
   box.bottom  = texture->getHeight();

   if(texture->mFormat == GFXFormatR8G8B8) // converted format also for volume textures
		dev->getDeviceContext()->UpdateSubresource(texture->get3DTex(), 0, &box, Bits, texture->getWidth() * bytesPerPix, texture->getHeight() * bytesPerPix);
   else
		dev->getDeviceContext()->UpdateSubresource(texture->get3DTex(), 0, &box, raw, texture->getWidth() * bytesPerPix, texture->getHeight() * bytesPerPix);

   SAFE_DELETE_ARRAY(Bits);

   return true;
}

bool GFXD3D11TextureManager::_refreshTexture(GFXTextureObject *texture)
{
   U32 usedStrategies = 0;
   GFXD3D11TextureObject *realTex = static_cast<GFXD3D11TextureObject *>(texture);

   if(texture->mProfile->doStoreBitmap())
   {
      if(texture->mBitmap)
         _loadTexture(texture, texture->mBitmap);

      if(texture->mDDS)
         _loadTexture(texture, texture->mDDS);

      usedStrategies++;
   }

   if(texture->mProfile->isRenderTarget() || texture->mProfile->isDynamic() || texture->mProfile->isZTarget())
   {
      realTex->release();
      _innerCreateTexture(realTex, texture->getHeight(), texture->getWidth(), texture->getDepth(), texture->mFormat, texture->mProfile, texture->mMipLevels, false, texture->mAntialiasLevel);
      usedStrategies++;
   }

   AssertFatal(usedStrategies < 2, "GFXD3D11TextureManager::_refreshTexture - Inconsistent profile flags!");

   return true;
}

bool GFXD3D11TextureManager::_freeTexture(GFXTextureObject *texture, bool zombify)
{
   AssertFatal(dynamic_cast<GFXD3D11TextureObject *>(texture),"Not an actual d3d texture object!");
   GFXD3D11TextureObject *tex = static_cast<GFXD3D11TextureObject *>( texture );

   // If it's a managed texture and we're zombifying, don't blast it, D3D allows
   // us to keep it.
   if(zombify && tex->isManaged)
     return true;

   tex->release();

   return true;
}

/// Load a texture from a proper DDSFile instance.
bool GFXD3D11TextureManager::_loadTexture(GFXTextureObject *aTexture, DDSFile *dds)
{
   PROFILE_SCOPE(GFXD3D11TextureManager_loadTextureDDS);

   GFXD3D11TextureObject *texture = static_cast<GFXD3D11TextureObject*>(aTexture);
   GFXD3D11Device* dev = static_cast<GFXD3D11Device *>(GFX);
   // Fill the texture...
   for( U32 i = 0; i < aTexture->mMipLevels; i++ )
   {
      PROFILE_SCOPE(GFXD3DTexMan_loadSurface);

		AssertFatal( dds->mSurfaces.size() > 0, "Assumption failed. DDSFile has no surfaces." );

		U32 subresource = D3D11CalcSubresource(i, 0, aTexture->mMipLevels);
		dev->getDeviceContext()->UpdateSubresource(texture->get2DTex(), subresource, 0, dds->mSurfaces[0]->mMips[i], dds->getSurfacePitch(i), 0);
   }

   D3D11_TEXTURE2D_DESC desc;
   // if the texture asked for mip generation. lets generate it.
   texture->get2DTex()->GetDesc(&desc);
   if (desc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
      dev->getDeviceContext()->GenerateMips(texture->getSRView());

   return true;
}

void GFXD3D11TextureManager::createResourceView(U32 height, U32 width, U32 depth, DXGI_FORMAT format, U32 numMipLevels,U32 usageFlags, GFXTextureObject *inTex)
{
	GFXD3D11TextureObject *tex = static_cast<GFXD3D11TextureObject*>(inTex);
	ID3D11Resource* resource = NULL;
	
	if(tex->get2DTex())
		resource = tex->get2DTex();
	else if(tex->getSurface())
		resource = tex->getSurface();
	else
		resource = tex->get3DTex();

	HRESULT hr;
	//TODO: add MSAA support later.
	if(usageFlags & D3D11_BIND_SHADER_RESOURCE)
	{
      D3D11_SHADER_RESOURCE_VIEW_DESC desc;

      if(usageFlags & D3D11_BIND_DEPTH_STENCIL)
         desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // reads the depth
      else
         desc.Format = format;
	
		if(depth > 0)
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MipLevels = -1;
			desc.Texture3D.MostDetailedMip = 0;
		}
		else
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipLevels = -1;
			desc.Texture2D.MostDetailedMip = 0;
		}
		
		hr = D3D11DEVICE->CreateShaderResourceView(resource,&desc, tex->getSRViewPtr());
		AssertFatal(SUCCEEDED(hr), "CreateShaderResourceView:: failed to create view!");
	}

	if(usageFlags & D3D11_BIND_RENDER_TARGET)
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		hr = D3D11DEVICE->CreateRenderTargetView(resource, &desc, tex->getRTViewPtr());
		AssertFatal(SUCCEEDED(hr), "CreateRenderTargetView:: failed to create view!");
	}

	if(usageFlags & D3D11_BIND_DEPTH_STENCIL)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		desc.Flags = 0;
		hr = D3D11DEVICE->CreateDepthStencilView(resource,&desc, tex->getDSViewPtr());
		AssertFatal(SUCCEEDED(hr), "CreateDepthStencilView:: failed to create view!");
	}
}