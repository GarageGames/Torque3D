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

#ifdef _MSC_VER
#pragma warning(disable: 4996) 
#endif

#include "gfx/D3D9/gfxD3D9Device.h"
#include "gfx/D3D9/gfxD3D9EnumTranslate.h"
#include "gfx/bitmap/bitmapUtils.h"
#include "gfx/gfxCardProfile.h"
#include "core/strings/unicode.h"
#include "core/util/swizzle.h"
#include "core/util/safeDelete.h"
#include "console/console.h"
#include "core/resourceManager.h"

//-----------------------------------------------------------------------------
// Utility function, valid only in this file
#ifdef D3D_TEXTURE_SPEW
U32 GFXD3D9TextureObject::mTexCount = 0;
#endif

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
GFXD3D9TextureManager::GFXD3D9TextureManager( LPDIRECT3DDEVICE9 d3ddevice, U32 adapterIndex ) 
{
   mD3DDevice = d3ddevice;
   mAdapterIndex = adapterIndex;
   dMemset( mCurTexSet, 0, sizeof( mCurTexSet ) );   
   mD3DDevice->GetDeviceCaps(&mDeviceCaps);
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
GFXD3D9TextureManager::~GFXD3D9TextureManager()
{
   // Destroy texture table now so just in case some texture objects
   // are still left, we don't crash on a pure virtual method call.
   SAFE_DELETE_ARRAY( mHashTable );
}

//-----------------------------------------------------------------------------
// _innerCreateTexture
//-----------------------------------------------------------------------------
void GFXD3D9TextureManager::_innerCreateTexture( GFXD3D9TextureObject *retTex, 
                                               U32 height, 
                                               U32 width, 
                                               U32 depth,
                                               GFXFormat format, 
                                               GFXTextureProfile *profile, 
                                               U32 numMipLevels,
                                               bool forceMips,
                                               S32 antialiasLevel)
{
   GFXD3D9Device* d3d = static_cast<GFXD3D9Device*>(GFX);

   // Some relevant helper information...
   bool supportsAutoMips = GFX->getCardProfiler()->queryProfile("autoMipMapLevel", true);
   
   DWORD usage = 0;   // 0, D3DUSAGE_RENDERTARGET, or D3DUSAGE_DYNAMIC
   D3DPOOL pool = D3DPOOL_DEFAULT;

   retTex->mProfile = profile;

   D3DFORMAT d3dTextureFormat = GFXD3D9TextureFormat[format];

#ifndef TORQUE_OS_XENON
   if( retTex->mProfile->isDynamic() )
   {
      usage = D3DUSAGE_DYNAMIC;
   }
   else
   {
      usage = 0;
      pool = d3d->isD3D9Ex() ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
   }

   if( retTex->mProfile->isRenderTarget() )
   {
      pool = D3DPOOL_DEFAULT;
      usage |= D3DUSAGE_RENDERTARGET;
   }

   if(retTex->mProfile->isZTarget())
   {
      usage |= D3DUSAGE_DEPTHSTENCIL;
      pool = D3DPOOL_DEFAULT;
   }

   if( retTex->mProfile->isSystemMemory() )
   {
      pool = D3DPOOL_SYSTEMMEM;
   }

   if( supportsAutoMips && 
       !forceMips &&
       !retTex->mProfile->isSystemMemory() &&
       numMipLevels == 0 &&
       !(depth > 0) )
   {
      usage |= D3DUSAGE_AUTOGENMIPMAP;
   }
#else
   if(retTex->mProfile->isRenderTarget())
   {
      d3dTextureFormat = (D3DFORMAT)MAKELEFMT(d3dTextureFormat);
   }
#endif

   // Set the managed flag...
   retTex->isManaged = (pool == D3DPOOL_MANAGED) || d3d->isD3D9Ex();
   
   if( depth > 0 )
   {
#ifdef TORQUE_OS_XENON
      D3D9Assert( mD3DDevice->CreateVolumeTexture( width, height, depth, numMipLevels, 0 /* usage ignored on the 360 */, 
         d3dTextureFormat, pool, retTex->get3DTexPtr(), NULL), "Failed to create volume texture" );
#else
      D3D9Assert(
         GFXD3DX.D3DXCreateVolumeTexture(
            mD3DDevice,
            width,
            height,
            depth,
            numMipLevels,
            usage,
            d3dTextureFormat,
            pool,
            retTex->get3DTexPtr()
         ), "GFXD3D9TextureManager::_createTexture - failed to create volume texture!"
      );
#endif

      retTex->mTextureSize.set( width, height, depth );
      retTex->mMipLevels = retTex->get3DTex()->GetLevelCount();
      // required for 3D texture support - John Kabus
	  retTex->mFormat = format;
   }
   else
   {
#ifdef TORQUE_OS_XENON
      D3D9Assert( mD3DDevice->CreateTexture(width, height, numMipLevels, usage, d3dTextureFormat, pool, retTex->get2DTexPtr(), NULL), "Failed to create texture" );
      retTex->mMipLevels = retTex->get2DTex()->GetLevelCount();
#else
      // Figure out AA settings for depth and render targets
      D3DMULTISAMPLE_TYPE mstype;
      DWORD mslevel;

      switch (antialiasLevel)
      {
         case 0 :
            mstype = D3DMULTISAMPLE_NONE;
            mslevel = 0;
            break;
         case AA_MATCH_BACKBUFFER :
            mstype = d3d->getMultisampleType();
            mslevel = d3d->getMultisampleLevel();
            break;
         default :
            {
               mstype = D3DMULTISAMPLE_NONMASKABLE;
               mslevel = antialiasLevel;
#ifdef TORQUE_DEBUG
               DWORD MaxSampleQualities;      
               d3d->getD3D()->CheckDeviceMultiSampleType(mAdapterIndex, D3DDEVTYPE_HAL, d3dTextureFormat, FALSE, D3DMULTISAMPLE_NONMASKABLE, &MaxSampleQualities);
               AssertFatal(mslevel < MaxSampleQualities, "Invalid AA level!");
#endif
            }
            break;
      }
     
      bool fastCreate = true;
      // Check for power of 2 textures - this is a problem with FX 5xxx cards
      // with current drivers - 3/2/05
      if( !isPow2(width) || !isPow2(height) )
      {
         fastCreate = false;
      }

      if(retTex->mProfile->isZTarget())
      {
         D3D9Assert(mD3DDevice->CreateDepthStencilSurface(width, height, d3dTextureFormat,
            mstype, mslevel, retTex->mProfile->canDiscard(), retTex->getSurfacePtr(), NULL), "Failed to create Z surface" );

         retTex->mFormat = format; // Assigning format like this should be fine.
      }
      else
      {
         // Try to create the texture directly - should gain us a bit in high
         // performance cases where we know we're creating good stuff and we
         // don't want to bother with D3DX - slow function.
         HRESULT res = D3DERR_INVALIDCALL;
         if( fastCreate )
         {
            res = mD3DDevice->CreateTexture(width, height, numMipLevels, usage, d3dTextureFormat, pool, retTex->get2DTexPtr(), NULL);
         }

         if( !fastCreate || (res != D3D_OK) )
         {
            D3D9Assert(
               GFXD3DX.D3DXCreateTexture(
               mD3DDevice,
               width,
               height,
               numMipLevels,
               usage,
               d3dTextureFormat,
               pool,
               retTex->get2DTexPtr()
               ), "GFXD3D9TextureManager::_createTexture - failed to create texture!"
               );
         }

         // If this is a render target, and it wants AA or wants to match the backbuffer (for example, to share the z)
         // Check the caps though, if we can't stretchrect between textures, use the old RT method.  (Which hopefully means
         // that they can't force AA on us as well.)
         if (retTex->mProfile->isRenderTarget() && mslevel != 0 && (mDeviceCaps.Caps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES))
         {
            D3D9Assert(mD3DDevice->CreateRenderTarget(width, height, d3dTextureFormat, 
               mstype, mslevel, false, retTex->getSurfacePtr(), NULL),
               "GFXD3D9TextureManager::_createTexture - unable to create render target");
         }

         // All done!
         retTex->mMipLevels = retTex->get2DTex()->GetLevelCount();
      }
#endif

      // Get the actual size of the texture...
      D3DSURFACE_DESC probeDesc;
      ZeroMemory(&probeDesc, sizeof probeDesc);

      if( retTex->get2DTex() != NULL )
         D3D9Assert( retTex->get2DTex()->GetLevelDesc( 0, &probeDesc ), "Failed to get surface description");
      else if( retTex->getSurface() != NULL )
         D3D9Assert( retTex->getSurface()->GetDesc( &probeDesc ), "Failed to get surface description");

      retTex->mTextureSize.set(probeDesc.Width, probeDesc.Height, 0);
      
      S32 fmt = probeDesc.Format;

#if !defined(TORQUE_OS_XENON)
      GFXREVERSE_LOOKUP( GFXD3D9TextureFormat, GFXFormat, fmt );
      retTex->mFormat = (GFXFormat)fmt;
#else
      retTex->mFormat = format;
#endif
   }
}

//-----------------------------------------------------------------------------
// createTexture
//-----------------------------------------------------------------------------
GFXTextureObject *GFXD3D9TextureManager::_createTextureObject( U32 height, 
                                                               U32 width,
                                                               U32 depth,
                                                               GFXFormat format, 
                                                               GFXTextureProfile *profile, 
                                                               U32 numMipLevels,
                                                               bool forceMips, 
                                                               S32 antialiasLevel,
                                                               GFXTextureObject *inTex )
{
   GFXD3D9TextureObject *retTex;
   if ( inTex )
   {
      AssertFatal( dynamic_cast<GFXD3D9TextureObject*>( inTex ), "GFXD3D9TextureManager::_createTexture() - Bad inTex type!" );
      retTex = static_cast<GFXD3D9TextureObject*>( inTex );
      retTex->release();
   }      
   else
   {
      retTex = new GFXD3D9TextureObject( GFX, profile );
      retTex->registerResourceWithDevice( GFX );
   }

   _innerCreateTexture(retTex, height, width, depth, format, profile, numMipLevels, forceMips, antialiasLevel);

   return retTex;
}

//-----------------------------------------------------------------------------
// loadTexture - GBitmap
//-----------------------------------------------------------------------------
bool GFXD3D9TextureManager::_loadTexture(GFXTextureObject *aTexture, GBitmap *pDL)
{
   PROFILE_SCOPE(GFXD3D9TextureManager_loadTexture);

   GFXD3D9TextureObject *texture = static_cast<GFXD3D9TextureObject*>(aTexture);

#ifdef TORQUE_OS_XENON
   // If the texture is currently bound, it needs to be unbound before modifying it
   if( texture->getTex() && texture->getTex()->IsSet( mD3DDevice ) )
   {
      mD3DDevice->SetTexture( 0, NULL );
      mD3DDevice->SetTexture( 1, NULL );
      mD3DDevice->SetTexture( 2, NULL );
      mD3DDevice->SetTexture( 3, NULL );
      mD3DDevice->SetTexture( 4, NULL );
      mD3DDevice->SetTexture( 5, NULL );
      mD3DDevice->SetTexture( 6, NULL );
      mD3DDevice->SetTexture( 7, NULL );
   }
#endif

   // Check with profiler to see if we can do automatic mipmap generation.
   const bool supportsAutoMips = GFX->getCardProfiler()->queryProfile("autoMipMapLevel", true);

   // Helper bool
   const bool isCompressedTexFmt = aTexture->mFormat >= GFXFormatDXT1 && aTexture->mFormat <= GFXFormatDXT5;

   GFXD3D9Device* dev = static_cast<GFXD3D9Device *>(GFX);

   // Settings for mipmap generation
   U32 maxDownloadMip = pDL->getNumMipLevels();
   U32 nbMipMapLevel  = pDL->getNumMipLevels();

   if( supportsAutoMips && !isCompressedTexFmt )
   {
      maxDownloadMip = 1;
      nbMipMapLevel  = aTexture->mMipLevels;
   }

   // Fill the texture...
   for( int i = 0; i < maxDownloadMip; i++ )
   {
      LPDIRECT3DSURFACE9 surf = NULL;
      D3D9Assert(texture->get2DTex()->GetSurfaceLevel( i, &surf ), "Failed to get surface");

      D3DLOCKED_RECT lockedRect;

#ifdef TORQUE_OS_XENON
      // On the 360, doing a LockRect doesn't work like it does with untiled memory
      // so instead swizzle into some temporary memory, and then later use D3DX
      // to do the upload properly.
      FrameTemp<U8> swizzleMem(pDL->getWidth(i) * pDL->getHeight(i) * pDL->getBytesPerPixel());
      lockedRect.pBits = (void*)~swizzleMem;
#else
      U32 waterMark = 0;
      if (!dev->isD3D9Ex())
         surf->LockRect( &lockedRect, NULL, 0 );
      else
      {
         waterMark = FrameAllocator::getWaterMark();
         lockedRect.pBits = static_cast<void*>(FrameAllocator::alloc(pDL->getWidth(i) * pDL->getHeight(i) * pDL->getBytesPerPixel()));
      }
#endif
      
      switch( texture->mFormat )
      {
      case GFXFormatR8G8B8:
         {
            PROFILE_SCOPE(Swizzle24_Upload);
            AssertFatal( pDL->getFormat() == GFXFormatR8G8B8, "Assumption failed" );
            GFX->getDeviceSwizzle24()->ToBuffer( lockedRect.pBits, pDL->getBits(i), 
               pDL->getWidth(i) * pDL->getHeight(i) * pDL->getBytesPerPixel() );
         }
         break;

      case GFXFormatR8G8B8A8:
      case GFXFormatR8G8B8X8:
         {
            PROFILE_SCOPE(Swizzle32_Upload);
            GFX->getDeviceSwizzle32()->ToBuffer( lockedRect.pBits, pDL->getBits(i), 
               pDL->getWidth(i) * pDL->getHeight(i) * pDL->getBytesPerPixel() );
         }
         break;

      default:
         {
            // Just copy the bits in no swizzle or padding
            PROFILE_SCOPE(SwizzleNull_Upload);
            AssertFatal( pDL->getFormat() == texture->mFormat, "Format mismatch" );
            dMemcpy( lockedRect.pBits, pDL->getBits(i), 
               pDL->getWidth(i) * pDL->getHeight(i) * pDL->getBytesPerPixel() );
         }
      }

#ifdef TORQUE_OS_XENON
      RECT srcRect;
      srcRect.bottom = pDL->getHeight(i);
      srcRect.top = 0;
      srcRect.left = 0;
      srcRect.right = pDL->getWidth(i);

      D3DXLoadSurfaceFromMemory(surf, NULL, NULL, ~swizzleMem, (D3DFORMAT)MAKELINFMT(GFXD3D9TextureFormat[pDL->getFormat()]),
         pDL->getWidth(i) * pDL->getBytesPerPixel(), NULL, &srcRect, false, 0, 0, D3DX_FILTER_NONE, 0);
#else
      if (!dev->isD3D9Ex())
         surf->UnlockRect();
      else
      {
         RECT srcRect;
         srcRect.top = 0;
         srcRect.left = 0;
         srcRect.right = pDL->getWidth(i);
         srcRect.bottom = pDL->getHeight(i);
         D3DXLoadSurfaceFromMemory(surf, NULL, NULL, lockedRect.pBits, GFXD3D9TextureFormat[pDL->getFormat()], pDL->getBytesPerPixel() * pDL->getWidth(i), NULL, &srcRect, D3DX_DEFAULT, 0);
         FrameAllocator::setWaterMark(waterMark);
      }
#endif
      
      surf->Release();
   }

   return true;          
}

//-----------------------------------------------------------------------------
// loadTexture - raw
//-----------------------------------------------------------------------------
bool GFXD3D9TextureManager::_loadTexture( GFXTextureObject *inTex, void *raw )
{
   PROFILE_SCOPE(GFXD3D9TextureManager_loadTextureRaw);

   GFXD3D9TextureObject *texture = (GFXD3D9TextureObject *) inTex;

   // currently only for volume textures...
   if( texture->getDepth() < 1 ) return false;

   
   U32 bytesPerPix = 1;

   switch( texture->mFormat )
   {
      case GFXFormatR8G8B8:
         bytesPerPix = 3;
         break;
      case GFXFormatR8G8B8A8:
      case GFXFormatR8G8B8X8:
      case GFXFormatB8G8R8A8:
         bytesPerPix = 4;
         break;
   }

   U32 rowPitch = texture->getWidth() * bytesPerPix;
   U32 slicePitch = texture->getWidth() * texture->getHeight() * bytesPerPix;

   D3DBOX box;
   box.Left    = 0;
   box.Right   = texture->getWidth();
   box.Front   = 0;
   box.Back    = texture->getDepth();
   box.Top     = 0;
   box.Bottom  = texture->getHeight();


   LPDIRECT3DVOLUME9 volume = NULL;
   D3D9Assert( texture->get3DTex()->GetVolumeLevel( 0, &volume ), "Failed to load volume" );

#ifdef TORQUE_OS_XENON
   D3DLOCKED_BOX lockedBox;
   volume->LockBox( &lockedBox, &box, 0 );
   
   dMemcpy( lockedBox.pBits, raw, slicePitch * texture->getDepth() );

   volume->UnlockBox();
#else
   D3D9Assert(
      GFXD3DX.D3DXLoadVolumeFromMemory(
         volume,
         NULL,
         NULL,
         raw,
         GFXD3D9TextureFormat[texture->mFormat],
         rowPitch,
         slicePitch,
         NULL,
         &box,
#ifdef TORQUE_OS_XENON
         false, 0, 0, 0, // Unique to Xenon -pw
#endif
         D3DX_FILTER_NONE,
         0
      ),
      "Failed to load volume texture" 
   );
#endif

   volume->Release();


   return true;
}

//-----------------------------------------------------------------------------
// refreshTexture
//-----------------------------------------------------------------------------
bool GFXD3D9TextureManager::_refreshTexture(GFXTextureObject *texture)
{
   U32 usedStrategies = 0;
   GFXD3D9TextureObject *realTex = static_cast<GFXD3D9TextureObject *>( texture );

   if(texture->mProfile->doStoreBitmap())
   {
//      SAFE_RELEASE(realTex->mD3DTexture);
//      _innerCreateTexture(realTex, texture->mTextureSize.x, texture->mTextureSize.y, texture->mFormat, texture->mProfile, texture->mMipLevels);

      if(texture->mBitmap)
         _loadTexture(texture, texture->mBitmap);

      if(texture->mDDS)
         _loadTexture(texture, texture->mDDS);

      usedStrategies++;
   }

   if(texture->mProfile->isRenderTarget() || texture->mProfile->isDynamic() ||
	   texture->mProfile->isZTarget())
   {
      realTex->release();
      _innerCreateTexture(realTex, texture->getHeight(), texture->getWidth(), texture->getDepth(), texture->mFormat, 

         texture->mProfile, texture->mMipLevels, false, texture->mAntialiasLevel);
      usedStrategies++;
   }

   AssertFatal(usedStrategies < 2, "GFXD3D9TextureManager::_refreshTexture - Inconsistent profile flags!");

   return true;
}


//-----------------------------------------------------------------------------
// freeTexture
//-----------------------------------------------------------------------------
bool GFXD3D9TextureManager::_freeTexture(GFXTextureObject *texture, bool zombify)
{
   AssertFatal(dynamic_cast<GFXD3D9TextureObject *>(texture),"Not an actual d3d texture object!");
   GFXD3D9TextureObject *tex = static_cast<GFXD3D9TextureObject *>( texture );

   // If it's a managed texture and we're zombifying, don't blast it, D3D allows
   // us to keep it.
   if(zombify && tex->isManaged)
      return true;

   tex->release();

   return true;
}

/// Load a texture from a proper DDSFile instance.
bool GFXD3D9TextureManager::_loadTexture(GFXTextureObject *aTexture, DDSFile *dds)
{
   PROFILE_SCOPE(GFXD3D9TextureManager_loadTextureDDS);

   GFXD3D9TextureObject *texture = static_cast<GFXD3D9TextureObject*>(aTexture);

   // Fill the texture...
   for( S32 i = 0; i < aTexture->mMipLevels; i++ )
   {
      PROFILE_SCOPE(GFXD3DTexMan_loadSurface);

      LPDIRECT3DSURFACE9 surf = NULL;
      D3D9Assert(texture->get2DTex()->GetSurfaceLevel( i, &surf ), "Failed to get surface");

#if defined(TORQUE_OS_XENON)
      XGTEXTURE_DESC surfDesc;
      dMemset(&surfDesc, 0, sizeof(XGTEXTURE_DESC));
      XGGetSurfaceDesc(surf, &surfDesc);

      RECT srcRect;
      srcRect.top = srcRect.left = 0;
      srcRect.bottom = dds->getHeight(i);
      srcRect.right = dds->getWidth(i);

      D3DXLoadSurfaceFromMemory(surf, NULL, NULL, dds->mSurfaces[0]->mMips[i],
         (D3DFORMAT)MAKELINFMT(GFXD3D9TextureFormat[dds->mFormat]), dds->getSurfacePitch(i), 
         NULL, &srcRect, false, 0, 0, D3DX_FILTER_NONE, 0);
#else

      GFXD3D9Device* dev = static_cast<GFXD3D9Device *>(GFX);

      if (dev->isD3D9Ex())
      {
         RECT r;
         r.top = r.left = 0;
         r.bottom = dds->getHeight(i);
         r.right = dds->getWidth(i);
         D3DXLoadSurfaceFromMemory(surf, NULL, NULL, dds->mSurfaces[0]->mMips[i], GFXD3D9TextureFormat[dds->mFormat], dds->getSurfacePitch(i), NULL, &r, D3DX_DEFAULT, 0);
      }
      else
      {
         D3DLOCKED_RECT lockedRect;
         D3D9Assert( surf->LockRect( &lockedRect, NULL, 0 ), "Failed to lock surface level for load" );

         AssertFatal( dds->mSurfaces.size() > 0, "Assumption failed. DDSFile has no surfaces." );

         if ( dds->getSurfacePitch( i ) != lockedRect.Pitch )
         {
            // Do a row-by-row copy.
            U32 srcPitch = dds->getSurfacePitch( i );
            U32 srcHeight = dds->getHeight();
            U8* srcBytes = dds->mSurfaces[0]->mMips[i];
            U8* dstBytes = (U8*)lockedRect.pBits;
            for (U32 i = 0; i<srcHeight; i++)          
            {
               dMemcpy( dstBytes, srcBytes, srcPitch );
               dstBytes += lockedRect.Pitch;
               srcBytes += srcPitch;
            }           
            surf->UnlockRect();
            surf->Release();

            return true;
         }

         dMemcpy( lockedRect.pBits, dds->mSurfaces[0]->mMips[i], dds->getSurfaceSize(i) );

         surf->UnlockRect();
      }
#endif

      surf->Release();
   }

   return true;
}