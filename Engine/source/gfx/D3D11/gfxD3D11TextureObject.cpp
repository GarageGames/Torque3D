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
#include "gfx/D3D11/gfxD3D11TextureObject.h"
#include "platform/profiler.h"
#include "console/console.h"

#ifdef TORQUE_DEBUG
U32 GFXD3D11TextureObject::mTexCount = 0;
#endif


//	GFXFormatR8G8B8 has now the same behaviour as GFXFormatR8G8B8X8. 
//	This is because 24 bit format are now deprecated by microsoft, for data alignment reason there's no changes beetween 24 and 32 bit formats.
//	DirectX 10-11 both have 24 bit format no longer.


GFXD3D11TextureObject::GFXD3D11TextureObject( GFXDevice * d, GFXTextureProfile *profile) : GFXTextureObject( d, profile )
{
#ifdef D3D11_DEBUG_SPEW
   mTexCount++;
   Con::printf("+ texMake %d %x", mTexCount, this);
#endif

   mD3DTexture = NULL;
   mLocked = false;

   mD3DSurface = NULL;
   mLockedSubresource = 0;
   mDSView = NULL;
   mRTView = NULL;
   mSRView = NULL;
}

GFXD3D11TextureObject::~GFXD3D11TextureObject()
{
   kill();
#ifdef D3D11_DEBUG_SPEW
   mTexCount--;
   Con::printf("+ texkill %d %x", mTexCount, this);
#endif
}

GFXLockedRect *GFXD3D11TextureObject::lock(U32 mipLevel /*= 0*/, RectI *inRect /*= NULL*/)
{
   AssertFatal( !mLocked, "GFXD3D11TextureObject::lock - The texture is already locked!" );

   if( !mStagingTex ||
      mStagingTex->getWidth() != getWidth() ||
      mStagingTex->getHeight() != getHeight() )
   {
      mStagingTex.set( getWidth(), getHeight(), mFormat, &GFXSystemMemTextureProfile, avar("%s() - mLockTex (line %d)", __FUNCTION__, __LINE__) );
   }

   ID3D11DeviceContext* pContext = D3D11DEVICECONTEXT;
   D3D11_MAPPED_SUBRESOURCE mapInfo;
   U32 offset = 0;
   mLockedSubresource = D3D11CalcSubresource(mipLevel, 0, getMipLevels());
   GFXD3D11TextureObject* pD3DStagingTex = (GFXD3D11TextureObject*)&(*mStagingTex);

   //map staging texture
   HRESULT hr = pContext->Map(pD3DStagingTex->get2DTex(), mLockedSubresource, D3D11_MAP_READ, 0, &mapInfo);

   if (FAILED(hr))
      AssertFatal(false, "GFXD3D11TextureObject:lock - failed to map render target resource!");

   const U32 width = mTextureSize.x >> mipLevel;
   const U32 height = mTextureSize.y >> mipLevel;

   //calculate locked box region and offset
   if (inRect)
   {
      if ((inRect->point.x + inRect->extent.x > width) || (inRect->point.y + inRect->extent.y > height))
         AssertFatal(false, "GFXD3D11TextureObject::lock - Rectangle too big!");

      mLockBox.top = inRect->point.y;
      mLockBox.left = inRect->point.x;
      mLockBox.bottom = inRect->point.y + inRect->extent.y;
      mLockBox.right = inRect->point.x + inRect->extent.x;
      mLockBox.back = 1;
      mLockBox.front = 0;

      //calculate offset
      offset = inRect->point.x * getFormatByteSize() + inRect->point.y * mapInfo.RowPitch;
   }
   else
   {
      mLockBox.top = 0;
      mLockBox.left = 0;
      mLockBox.bottom = height;
      mLockBox.right = width;
      mLockBox.back = 1;
      mLockBox.front = 0;
   }

   mLocked = true;
   mLockRect.pBits = static_cast<U8*>(mapInfo.pData) + offset;
   mLockRect.Pitch = mapInfo.RowPitch;

   return (GFXLockedRect*)&mLockRect;
}

void GFXD3D11TextureObject::unlock(U32 mipLevel)
{
   AssertFatal( mLocked, "GFXD3D11TextureObject::unlock - Attempting to unlock a surface that has not been locked" );

   //profile in the unlock function because all the heavy lifting is done here
   PROFILE_START(GFXD3D11TextureObject_lockRT);

   ID3D11DeviceContext* pContext = D3D11DEVICECONTEXT;
   GFXD3D11TextureObject* pD3DStagingTex = (GFXD3D11TextureObject*)&(*mStagingTex);
   ID3D11Texture2D *pStagingTex = pD3DStagingTex->get2DTex();

   //unmap staging texture
   pContext->Unmap(pStagingTex, mLockedSubresource);
   //copy lock box region from the staging texture to our regular texture
   pContext->CopySubresourceRegion(mD3DTexture, mLockedSubresource, mLockBox.left, mLockBox.top, 0, pStagingTex, mLockedSubresource, &mLockBox);

   PROFILE_END();

   mLockedSubresource = 0;
   mLocked = false;
}

void GFXD3D11TextureObject::release()
{
   SAFE_RELEASE(mSRView);
   SAFE_RELEASE(mRTView);
   SAFE_RELEASE(mDSView);
   SAFE_RELEASE(mD3DTexture);
   SAFE_RELEASE(mD3DSurface);
}

void GFXD3D11TextureObject::zombify()
{
   // Managed textures are managed by D3D
   AssertFatal(!mLocked, "GFXD3D11TextureObject::zombify - Cannot zombify a locked texture!");
   if(isManaged)
      return;
   release();
}

void GFXD3D11TextureObject::resurrect()
{
	// Managed textures are managed by D3D
   if(isManaged)
      return;

   static_cast<GFXD3D11TextureManager*>(TEXMGR)->refreshTexture(this);
}

bool GFXD3D11TextureObject::copyToBmp(GBitmap* bmp)
{
   if (!bmp)
      return false;

   // check format limitations
   // at the moment we only support RGBA for the source (other 4 byte formats should
   // be easy to add though)
   AssertFatal(mFormat == GFXFormatR8G8B8A8 || mFormat == GFXFormatR8G8B8A8_LINEAR_FORCE || mFormat == GFXFormatR8G8B8A8_SRGB, "copyToBmp: invalid format");
   if (mFormat != GFXFormatR8G8B8A8 && mFormat != GFXFormatR8G8B8A8_LINEAR_FORCE && mFormat != GFXFormatR8G8B8A8_SRGB)
      return false;

   PROFILE_START(GFXD3D11TextureObject_copyToBmp);

   AssertFatal(bmp->getWidth() == getWidth(), "GFXD3D11TextureObject::copyToBmp - source/dest width does not match");
   AssertFatal(bmp->getHeight() == getHeight(), "GFXD3D11TextureObject::copyToBmp - source/dest height does not match");
   const U32 width = getWidth();
   const U32 height = getHeight();

   bmp->setHasTransparency(mHasTransparency);

   // set some constants
   const U32 sourceBytesPerPixel = 4;
   U32 destBytesPerPixel = 0;

   const GFXFormat fmt = bmp->getFormat();
   if (fmt == GFXFormatR8G8B8A8 || fmt == GFXFormatR8G8B8A8_LINEAR_FORCE || fmt == GFXFormatR8G8B8A8_SRGB)
      destBytesPerPixel = 4;
   else if(bmp->getFormat() == GFXFormatR8G8B8)
      destBytesPerPixel = 3;
   else
      // unsupported
      AssertFatal(false, "GFXD3D11TextureObject::copyToBmp - unsupported bitmap format");
   
   //create temp staging texture
   D3D11_TEXTURE2D_DESC desc;
   static_cast<ID3D11Texture2D*>(mD3DTexture)->GetDesc(&desc);
   desc.BindFlags = 0;
   desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
   desc.Usage = D3D11_USAGE_STAGING;

   ID3D11Texture2D* pStagingTexture = NULL;
   HRESULT hr = D3D11DEVICE->CreateTexture2D(&desc, NULL, &pStagingTexture);
   if (FAILED(hr))
   {
      Con::errorf("GFXD3D11TextureObject::copyToBmp - Failed to create staging texture"); 
      return false;
   }

   //copy the classes texture to the staging texture
   D3D11DEVICECONTEXT->CopyResource(pStagingTexture, mD3DTexture);

   //map the staging resource
   D3D11_MAPPED_SUBRESOURCE mappedRes;
   hr = D3D11DEVICECONTEXT->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedRes);
   if (FAILED(hr))
   {
      //cleanup
      SAFE_RELEASE(pStagingTexture);
      Con::errorf("GFXD3D11TextureObject::copyToBmp - Failed to map staging texture");
      return false;
   }

   // set pointers
   const U8* srcPtr = (U8*)mappedRes.pData;
   U8* destPtr = bmp->getWritableBits();

   // we will want to skip over any D3D cache data in the source texture
   const S32 sourceCacheSize = mappedRes.RowPitch - width * sourceBytesPerPixel;
   AssertFatal(sourceCacheSize >= 0, "GFXD3D11TextureObject::copyToBmp - cache size is less than zero?");

   // copy data into bitmap
   for (U32 row = 0; row < height; ++row)
   {
      for (U32 col = 0; col < width; ++col)
      {
         destPtr[0] = srcPtr[2]; // red
         destPtr[1] = srcPtr[1]; // green
         destPtr[2] = srcPtr[0]; // blue 
         if (destBytesPerPixel == 4)
            destPtr[3] = srcPtr[3]; // alpha

         // go to next pixel in src
         srcPtr += sourceBytesPerPixel;

         // go to next pixel in dest
         destPtr += destBytesPerPixel;
      }
      // skip past the cache data for this row (if any)
      srcPtr += sourceCacheSize;
   }

   // assert if we stomped or underran memory
   AssertFatal(U32(destPtr - bmp->getWritableBits()) == width * height * destBytesPerPixel, "GFXD3D11TextureObject::copyToBmp - memory error");
   AssertFatal(U32(srcPtr - (U8*)mappedRes.pData) == height * mappedRes.RowPitch, "GFXD3D11TextureObject::copyToBmp - memory error");

   D3D11DEVICECONTEXT->Unmap(pStagingTexture, 0);

   SAFE_RELEASE(pStagingTexture);
   PROFILE_END();

   return true;
}

ID3D11ShaderResourceView* GFXD3D11TextureObject::getSRView()
{
	return mSRView;
}
ID3D11RenderTargetView* GFXD3D11TextureObject::getRTView()
{
	return mRTView;
}
ID3D11DepthStencilView* GFXD3D11TextureObject::getDSView()
{
	return mDSView;
}

ID3D11ShaderResourceView** GFXD3D11TextureObject::getSRViewPtr()
{
	return &mSRView;
}
ID3D11RenderTargetView** GFXD3D11TextureObject::getRTViewPtr()
{
	return &mRTView;
}

ID3D11DepthStencilView** GFXD3D11TextureObject::getDSViewPtr()
{
	return &mDSView;
}