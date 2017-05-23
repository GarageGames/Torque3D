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

#include "gfx/D3D11/gfxD3D11Cubemap.h"
#include "gfx/gfxCardProfile.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/D3D11/gfxD3D11EnumTranslate.h"

GFXD3D11Cubemap::GFXD3D11Cubemap() : mTexture(NULL), mSRView(NULL), mDSView(NULL)
{
	mDynamic = false;
   mAutoGenMips = false;
	mFaceFormat = GFXFormatR8G8B8A8;

   for (U32 i = 0; i < CubeFaces; i++)
	{
      mRTView[i] = NULL;
	}
}

GFXD3D11Cubemap::~GFXD3D11Cubemap()
{
	releaseSurfaces();
}

void GFXD3D11Cubemap::releaseSurfaces()
{
   if (mDynamic)
		GFXTextureManager::removeEventDelegate(this, &GFXD3D11Cubemap::_onTextureEvent);

   for (U32 i = 0; i < CubeFaces; i++)
	{
      SAFE_RELEASE(mRTView[i]);
	}

   SAFE_RELEASE(mDSView);
   SAFE_RELEASE(mSRView);
	SAFE_RELEASE(mTexture);
}

void GFXD3D11Cubemap::_onTextureEvent(GFXTexCallbackCode code)
{
   if (code == GFXZombify)
      releaseSurfaces();
   else if (code == GFXResurrect)
      initDynamic(mTexSize);
}

bool GFXD3D11Cubemap::isCompressed(GFXFormat format)
{
   if (format >= GFXFormatDXT1 && format <= GFXFormatDXT5)
      return true;

   return false;
}

void GFXD3D11Cubemap::initStatic(GFXTexHandle *faces)
{
   AssertFatal( faces, "GFXD3D11Cubemap::initStatic - Got null GFXTexHandle!" );
	AssertFatal( *faces, "empty texture passed to CubeMap::create" );
  
	// NOTE - check tex sizes on all faces - they MUST be all same size
	mTexSize = faces->getWidth();
	mFaceFormat = faces->getFormat();
   bool compressed = isCompressed(mFaceFormat);

   UINT bindFlags = D3D11_BIND_SHADER_RESOURCE;
   UINT miscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
   if (!compressed)
   {
      bindFlags |= D3D11_BIND_RENDER_TARGET;
      miscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
   }

   U32 mipLevels = faces->getPointer()->getMipLevels();
   if (mipLevels > 1 && !compressed)
      mAutoGenMips = true;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = mTexSize;
	desc.Height = mTexSize;
   desc.MipLevels = mAutoGenMips ? 0 : mipLevels;
	desc.ArraySize = 6;
	desc.Format = GFXD3D11TextureFormat[mFaceFormat];
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = bindFlags;
	desc.MiscFlags = miscFlags;
	desc.CPUAccessFlags = 0;

	HRESULT hr = D3D11DEVICE->CreateTexture2D(&desc, NULL, &mTexture);

	if (FAILED(hr))
	{
		AssertFatal(false, "GFXD3D11Cubemap:initStatic(GFXTexhandle *faces) - failed to create texcube texture");
	}
   
   for (U32 i = 0; i < CubeFaces; i++)
   {
      GFXD3D11TextureObject *texObj = static_cast<GFXD3D11TextureObject*>((GFXTextureObject*)faces[i]);
      for (U32 currentMip = 0; currentMip < mipLevels; currentMip++)
      {
         U32 subResource = D3D11CalcSubresource(currentMip, i, mipLevels);
         D3D11DEVICECONTEXT->CopySubresourceRegion(mTexture, subResource, 0, 0, 0, texObj->get2DTex(), currentMip, NULL);
      }
   }
   
	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = GFXD3D11TextureFormat[mFaceFormat];
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
   SMViewDesc.TextureCube.MipLevels = mAutoGenMips ? -1 : mipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	hr = D3D11DEVICE->CreateShaderResourceView(mTexture, &SMViewDesc, &mSRView);
	if (FAILED(hr))
	{
		AssertFatal(false, "GFXD3D11Cubemap::initStatic(GFXTexHandle *faces) - texcube shader resource view  creation failure");
	} 

   if (mAutoGenMips && !compressed)
      D3D11DEVICECONTEXT->GenerateMips(mSRView);
}

void GFXD3D11Cubemap::initStatic(DDSFile *dds)
{
   AssertFatal(dds, "GFXD3D11Cubemap::initStatic - Got null DDS file!");
   AssertFatal(dds->isCubemap(), "GFXD3D11Cubemap::initStatic - Got non-cubemap DDS file!");
   AssertFatal(dds->mSurfaces.size() == 6, "GFXD3D11Cubemap::initStatic - DDS has less than 6 surfaces!");  
   
   // NOTE - check tex sizes on all faces - they MUST be all same size
   mTexSize = dds->getWidth();
   mFaceFormat = dds->getFormat();
   U32 levels = dds->getMipLevels();

	D3D11_TEXTURE2D_DESC desc;

	desc.Width = mTexSize;
	desc.Height = mTexSize;
	desc.MipLevels = levels;
	desc.ArraySize = 6;
	desc.Format = GFXD3D11TextureFormat[mFaceFormat];
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;

	D3D11_SUBRESOURCE_DATA* pData = new D3D11_SUBRESOURCE_DATA[6 + levels];

   for (U32 i = 0; i<CubeFaces; i++)
	{
		if (!dds->mSurfaces[i])
			continue;

		for(U32 j = 0; j < levels; j++)
		{
			pData[i + j].pSysMem = dds->mSurfaces[i]->mMips[j];
			pData[i + j].SysMemPitch = dds->getSurfacePitch(j);
			pData[i + j].SysMemSlicePitch = dds->getSurfaceSize(j);
		}
	}

	HRESULT hr = D3D11DEVICE->CreateTexture2D(&desc, pData, &mTexture);

	delete [] pData;

	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = GFXD3D11TextureFormat[mFaceFormat];
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels =  levels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	hr = D3D11DEVICE->CreateShaderResourceView(mTexture, &SMViewDesc, &mSRView);

	if(FAILED(hr)) 
	{
		AssertFatal(false, "GFXD3D11Cubemap::initStatic(DDSFile *dds) - CreateTexture2D call failure");
	}
}

void GFXD3D11Cubemap::initDynamic(U32 texSize, GFXFormat faceFormat)
{
	if(!mDynamic)
		GFXTextureManager::addEventDelegate(this, &GFXD3D11Cubemap::_onTextureEvent);

	mDynamic = true;
   mAutoGenMips = true;
	mTexSize = texSize;
	mFaceFormat = faceFormat;
   bool compressed = isCompressed(mFaceFormat);

   UINT bindFlags = D3D11_BIND_SHADER_RESOURCE;
   UINT miscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
   if (!compressed)
   {
      bindFlags |= D3D11_BIND_RENDER_TARGET;
      miscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
   }

	D3D11_TEXTURE2D_DESC desc;

	desc.Width = mTexSize;
	desc.Height = mTexSize;
	desc.MipLevels = 0;
	desc.ArraySize = 6;
	desc.Format = GFXD3D11TextureFormat[mFaceFormat];
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
   desc.BindFlags = bindFlags;
	desc.CPUAccessFlags = 0;
   desc.MiscFlags = miscFlags;


	HRESULT hr = D3D11DEVICE->CreateTexture2D(&desc, NULL, &mTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = GFXD3D11TextureFormat[mFaceFormat];
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
   SMViewDesc.TextureCube.MipLevels = -1;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	hr = D3D11DEVICE->CreateShaderResourceView(mTexture, &SMViewDesc, &mSRView);


	if(FAILED(hr)) 
	{
		AssertFatal(false, "GFXD3D11Cubemap::initDynamic - CreateTexture2D call failure");
	}

   D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
	viewDesc.Format = desc.Format;
	viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.ArraySize = 1;
	viewDesc.Texture2DArray.MipSlice = 0;

   for (U32 i = 0; i < CubeFaces; i++)
	{
		viewDesc.Texture2DArray.FirstArraySlice = i;
      hr = D3D11DEVICE->CreateRenderTargetView(mTexture, &viewDesc, &mRTView[i]);

		if(FAILED(hr)) 
		{
			AssertFatal(false, "GFXD3D11Cubemap::initDynamic - CreateRenderTargetView call failure");
		}
	}

   D3D11_TEXTURE2D_DESC depthTexDesc;
   depthTexDesc.Width = mTexSize;
   depthTexDesc.Height = mTexSize;
   depthTexDesc.MipLevels = 1;
   depthTexDesc.ArraySize = 1;
   depthTexDesc.SampleDesc.Count = 1;
   depthTexDesc.SampleDesc.Quality = 0;
   depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
   depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
   depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
   depthTexDesc.CPUAccessFlags = 0;
   depthTexDesc.MiscFlags = 0;

   ID3D11Texture2D* depthTex = 0;
   hr = D3D11DEVICE->CreateTexture2D(&depthTexDesc, 0, &depthTex);

	if(FAILED(hr)) 
	{
		AssertFatal(false, "GFXD3D11Cubemap::initDynamic - CreateTexture2D for depth stencil call failure");
	}

   // Create the depth stencil view for the entire cube
   D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
   dsvDesc.Format = depthTexDesc.Format; //The format must match the depth texture we created above
   dsvDesc.Flags  = 0;
   dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
   dsvDesc.Texture2D.MipSlice = 0;
   hr = D3D11DEVICE->CreateDepthStencilView(depthTex, &dsvDesc, &mDSView);

	if(FAILED(hr)) 
	{
      AssertFatal(false, "GFXD3D11Cubemap::initDynamic - CreateDepthStencilView call failure");
	}

   SAFE_RELEASE(depthTex);

}

//-----------------------------------------------------------------------------
// Set the cubemap to the specified texture unit num
//-----------------------------------------------------------------------------
void GFXD3D11Cubemap::setToTexUnit(U32 tuNum)
{
   D3D11DEVICECONTEXT->PSSetShaderResources(tuNum, 1, &mSRView);
}

void GFXD3D11Cubemap::zombify()
{
   // Static cubemaps are handled by D3D
   if( mDynamic )
      releaseSurfaces();
}

void GFXD3D11Cubemap::resurrect()
{
   // Static cubemaps are handled by D3D
   if( mDynamic )
      initDynamic( mTexSize, mFaceFormat );
}

ID3D11ShaderResourceView* GFXD3D11Cubemap::getSRView()
{
   return mSRView;
}

ID3D11RenderTargetView* GFXD3D11Cubemap::getRTView(U32 faceIdx)
{
   AssertFatal(faceIdx < CubeFaces, "GFXD3D11Cubemap::getRTView - face index out of bounds");

   return mRTView[faceIdx];
}

ID3D11RenderTargetView** GFXD3D11Cubemap::getRTViewArray()
{
   return mRTView;
}

ID3D11DepthStencilView* GFXD3D11Cubemap::getDSView()
{
   return mDSView;
}

ID3D11Texture2D* GFXD3D11Cubemap::get2DTex()
{
   return mTexture;
}