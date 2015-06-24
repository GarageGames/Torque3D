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

#include "gfx/D3D9/gfxD3D9Device.h"
#include "gfx/D3D9/gfxD3D9TextureObject.h"
#include "platform/profiler.h"

#ifdef TORQUE_OS_XENON
#include "gfx/D3D9/360/gfx360Device.h"
#include "gfx/D3D9/360/gfx360Target.h"
#include "gfx/D3D9/gfxD3D9EnumTranslate.h"
#endif

U32 GFXD3D9TextureObject::mTexCount = 0;

//*****************************************************************************
// GFX D3D Texture Object
//*****************************************************************************
GFXD3D9TextureObject::GFXD3D9TextureObject( GFXDevice * d, GFXTextureProfile *profile)
                                        : GFXTextureObject( d, profile )
{
#ifdef D3D_TEXTURE_SPEW
   mTexCount++;
   Con::printf("+ texMake %d %x", mTexCount, this);
#endif

   isManaged = false;
   mD3DTexture = NULL;
   mLocked = false;

   mD3DSurface = NULL;
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
GFXD3D9TextureObject::~GFXD3D9TextureObject()
{
   kill();
#ifdef D3D_TEXTURE_SPEW
   mTexCount--;
   Con::printf("+ texkill %d %x", mTexCount, this);
#endif
}

//-----------------------------------------------------------------------------
// lock
//-----------------------------------------------------------------------------
GFXLockedRect *GFXD3D9TextureObject::lock(U32 mipLevel /*= 0*/, RectI *inRect /*= NULL*/)
{
   AssertFatal( !mLocked, "GFXD3D9TextureObject::lock - The texture is already locked!" );

   if( mProfile->isRenderTarget() )
   {
      if( !mLockTex || 
          mLockTex->getWidth() != getWidth() ||
          mLockTex->getHeight() != getHeight() )
      {
         mLockTex.set( getWidth(), getHeight(), mFormat, &GFXSystemMemProfile, avar("%s() - mLockTex (line %d)", __FUNCTION__, __LINE__) );
      }

      PROFILE_START(GFXD3D9TextureObject_lockRT);

      IDirect3DSurface9 *source;
      D3D9Assert( get2DTex()->GetSurfaceLevel( 0, &source ), "GFXD3D9TextureObject::lock - failed to get our own texture's surface." );
      
      IDirect3DSurface9 *dest;
      GFXD3D9TextureObject *to = (GFXD3D9TextureObject *) &(*mLockTex);
      D3D9Assert( to->get2DTex()->GetSurfaceLevel( 0, &dest ), "GFXD3D9TextureObject::lock - failed to get dest texture's surface." );

#ifndef TORQUE_OS_XENON
      LPDIRECT3DDEVICE9 D3DDevice = dynamic_cast<GFXD3D9Device *>(GFX)->getDevice();
      HRESULT rtLockRes = D3DDevice->GetRenderTargetData( source, dest );
#else
      AssertFatal(false, "Use different functionality on the Xbox 360 to perform this task.");
      HRESULT rtLockRes = E_FAIL;
#endif
      source->Release();

      if(!SUCCEEDED(rtLockRes))
      {
         // This case generally occurs if the device is lost. The lock failed
         // so clean up and return NULL.
         dest->Release();
         PROFILE_END();
         return NULL;
      }

      D3D9Assert( dest->LockRect( &mLockRect, NULL, D3DLOCK_READONLY ), NULL );
      dest->Release();
      mLocked = true;

      PROFILE_END();
   }
   else
   {
      RECT r;

      if(inRect)
      {
         r.top  = inRect->point.y;
         r.left = inRect->point.x;
         r.bottom = inRect->point.y + inRect->extent.y;
         r.right  = inRect->point.x + inRect->extent.x;
      }

      D3D9Assert( get2DTex()->LockRect(mipLevel, &mLockRect, inRect ? &r : NULL, 0), 
         "GFXD3D9TextureObject::lock - could not lock non-RT texture!" );
      mLocked = true;

   }

   // GFXLockedRect is set up to correspond to D3DLOCKED_RECT, so this is ok.
   return (GFXLockedRect*)&mLockRect; 
}
   
//-----------------------------------------------------------------------------
// unLock
//-----------------------------------------------------------------------------
void GFXD3D9TextureObject::unlock(U32 mipLevel)
{
   AssertFatal( mLocked, "GFXD3D9TextureObject::unlock - Attempting to unlock a surface that has not been locked" );

#ifndef TORQUE_OS_XENON
   if( mProfile->isRenderTarget() )
   {
      IDirect3DSurface9 *dest;
      GFXD3D9TextureObject *to = (GFXD3D9TextureObject *) &(*mLockTex);
      D3D9Assert( to->get2DTex()->GetSurfaceLevel( 0, &dest ), NULL );

      dest->UnlockRect();
      dest->Release();

      mLocked = false;
   }
   else
#endif
   {
      D3D9Assert( get2DTex()->UnlockRect(mipLevel), 
         "GFXD3D9TextureObject::unlock - could not unlock non-RT texture." );

      mLocked = false;
   }
}

//------------------------------------------------------------------------------

void GFXD3D9TextureObject::release()
{
   static_cast<GFXD3D9Device *>( GFX )->destroyD3DResource( mD3DTexture );
   static_cast<GFXD3D9Device *>( GFX )->destroyD3DResource( mD3DSurface );
   mD3DTexture = NULL;
   mD3DSurface = NULL;
}

void GFXD3D9TextureObject::zombify()
{
   // Managed textures are managed by D3D
   AssertFatal(!mLocked, "GFXD3D9TextureObject::zombify - Cannot zombify a locked texture!");
   if(isManaged)
      return;

   release();
}

void GFXD3D9TextureObject::resurrect()
{
   // Managed textures are managed by D3D
   if(isManaged)
      return;

   static_cast<GFXD3D9TextureManager*>(TEXMGR)->refreshTexture(this);
}

//------------------------------------------------------------------------------

bool GFXD3D9TextureObject::copyToBmp(GBitmap* bmp)
{
#ifdef TORQUE_OS_XENON
   // TODO: Implement Xenon version -patw
   return false;
#else
   if (!bmp)
      return false;

   // check format limitations
   // at the moment we only support RGBA for the source (other 4 byte formats should
   // be easy to add though)
   AssertFatal(mFormat == GFXFormatR8G8B8A8 || mFormat == GFXFormatR8G8B8, "copyToBmp: invalid format");
   if (mFormat != GFXFormatR8G8B8A8 && mFormat != GFXFormatR8G8B8)
      return false;

   PROFILE_START(GFXD3D9TextureObject_copyToBmp);

   AssertFatal(bmp->getWidth() == getWidth(), "doh");
   AssertFatal(bmp->getHeight() == getHeight(), "doh");
   U32 width = getWidth();
   U32 height = getHeight();

   bmp->setHasTransparency(mHasTransparency);

   // set some constants
   const U32 sourceBytesPerPixel = 4;
   U32 destBytesPerPixel = 0;
   if (bmp->getFormat() == GFXFormatR8G8B8A8)
      destBytesPerPixel = 4;
   else if (bmp->getFormat() == GFXFormatR8G8B8)
      destBytesPerPixel = 3;
   else
      // unsupported
      AssertFatal(false, "unsupported bitmap format");

   // lock the texture
   D3DLOCKED_RECT* lockRect = (D3DLOCKED_RECT*) lock();

   // set pointers
   U8* srcPtr = (U8*)lockRect->pBits;
   U8* destPtr = bmp->getWritableBits();

   // we will want to skip over any D3D cache data in the source texture
   const S32 sourceCacheSize = lockRect->Pitch - width * sourceBytesPerPixel;
   AssertFatal(sourceCacheSize >= 0, "copyToBmp: cache size is less than zero?");

   PROFILE_START(GFXD3D9TextureObject_copyToBmp_pixCopy);
   // copy data into bitmap
   for (S32 row = 0; row < height; ++row)
   {
      for (S32 col = 0; col < width; ++col)
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
   PROFILE_END();

   // assert if we stomped or underran memory
   AssertFatal(U32(destPtr - bmp->getWritableBits()) == width * height * destBytesPerPixel, "copyToBmp: doh, memory error");
   AssertFatal(U32(srcPtr - (U8*)lockRect->pBits) == height * lockRect->Pitch, "copyToBmp: doh, memory error");

   // unlock
   unlock();

   PROFILE_END();

   return true;
#endif
}
