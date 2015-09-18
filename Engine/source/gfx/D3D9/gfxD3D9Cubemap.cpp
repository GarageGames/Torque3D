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

#include "gfx/D3D9/gfxD3D9Cubemap.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/D3D9/gfxD3D9EnumTranslate.h"

_D3DCUBEMAP_FACES GFXD3D9Cubemap::faceList[6] = 
{ 
   D3DCUBEMAP_FACE_POSITIVE_X, D3DCUBEMAP_FACE_NEGATIVE_X,
   D3DCUBEMAP_FACE_POSITIVE_Y, D3DCUBEMAP_FACE_NEGATIVE_Y,
   D3DCUBEMAP_FACE_POSITIVE_Z, D3DCUBEMAP_FACE_NEGATIVE_Z
};

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
GFXD3D9Cubemap::GFXD3D9Cubemap()
{
   mCubeTex = NULL;
   mDynamic = false;
   mFaceFormat = GFXFormatR8G8B8A8;
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
GFXD3D9Cubemap::~GFXD3D9Cubemap()
{
   releaseSurfaces();

   if ( mDynamic )
      GFXTextureManager::removeEventDelegate( this, &GFXD3D9Cubemap::_onTextureEvent );
}

//-----------------------------------------------------------------------------
// Release D3D surfaces
//-----------------------------------------------------------------------------
void GFXD3D9Cubemap::releaseSurfaces()
{
   if ( !mCubeTex )
      return;

   mCubeTex->Release();
   mCubeTex = NULL;
}

void GFXD3D9Cubemap::_onTextureEvent( GFXTexCallbackCode code )
{
   // Can this happen?
   if ( !mDynamic ) 
      return;
   
   if ( code == GFXZombify )
      releaseSurfaces();
   else if ( code == GFXResurrect )
      initDynamic( mTexSize );
}

//-----------------------------------------------------------------------------
// Init Static
//-----------------------------------------------------------------------------
void GFXD3D9Cubemap::initStatic( GFXTexHandle *faces )
{
   //if( mCubeTex )
   //   return;

   if( faces )
   {
      AssertFatal( faces[0], "empty texture passed to CubeMap::create" );
   
      GFXD3D9Device *dev = static_cast<GFXD3D9Device *>(GFX);

      D3DPOOL pool = D3DPOOL_MANAGED;

      if (dev->isD3D9Ex())
         pool = D3DPOOL_DEFAULT;

      LPDIRECT3DDEVICE9 D3D9Device = dev->getDevice();     
      
      // NOTE - check tex sizes on all faces - they MUST be all same size
      mTexSize = faces[0].getWidth();
      mFaceFormat = faces[0].getFormat();

      U32 levels = faces->getPointer()->getMipLevels();
      if (levels >1)
      { 
         D3D9Assert(D3D9Device->CreateCubeTexture(mTexSize, levels, 0, GFXD3D9TextureFormat[mFaceFormat],
            pool, &mCubeTex, NULL), NULL);
         fillCubeTextures(faces, D3D9Device);
      }
      else
      {
         D3D9Assert( D3D9Device->CreateCubeTexture( mTexSize, 0, D3DUSAGE_AUTOGENMIPMAP, GFXD3D9TextureFormat[mFaceFormat],
                 pool, &mCubeTex, NULL ), NULL );
         fillCubeTextures( faces, D3D9Device );
         mCubeTex->GenerateMipSubLevels();
      }
   }
}

void GFXD3D9Cubemap::initStatic( DDSFile *dds )
{
   AssertFatal( dds, "GFXD3D9Cubemap::initStatic - Got null DDS file!" );
   AssertFatal( dds->isCubemap(), "GFXD3D9Cubemap::initStatic - Got non-cubemap DDS file!" );
   AssertFatal( dds->mSurfaces.size() == 6, "GFXD3D9Cubemap::initStatic - DDS has less than 6 surfaces!" );

   GFXD3D9Device *dev = static_cast<GFXD3D9Device *>(GFX);

   D3DPOOL pool = D3DPOOL_MANAGED;

   if (dev->isD3D9Ex())
      pool = D3DPOOL_DEFAULT;

   LPDIRECT3DDEVICE9 D3D9Device = dev->getDevice();     
   
   // NOTE - check tex sizes on all faces - they MUST be all same size
   mTexSize = dds->getWidth();
   mFaceFormat = dds->getFormat();
   U32 levels = dds->getMipLevels();

   D3D9Assert( D3D9Device->CreateCubeTexture( mTexSize, levels, 0, GFXD3D9TextureFormat[mFaceFormat],
              pool, &mCubeTex, NULL ), NULL );

   for( U32 i=0; i < 6; i++ )
   {
      if ( !dds->mSurfaces[i] )
      {
         // TODO: The DDS can skip surfaces, but i'm unsure what i should
         // do here when creating the cubemap.  Ignore it for now.
         continue;
      }

      // Now loop thru the mip levels!
      for( U32 l = 0; l < levels; l++ )
      {
         IDirect3DSurface9 *cubeSurf = NULL;
         D3D9Assert( mCubeTex->GetCubeMapSurface( faceList[i], l, &cubeSurf ), 
            "GFXD3D9Cubemap::initStatic - Get cubemap surface failed!" );

         // Lock the dest surface.
         D3DLOCKED_RECT lockedRect;
         D3D9Assert( cubeSurf->LockRect( &lockedRect, NULL, 0 ), 
            "GFXD3D9Cubemap::initStatic - Failed to lock surface level for load!" );

         dMemcpy( lockedRect.pBits, dds->mSurfaces[i]->mMips[l], dds->getSurfaceSize(l) );

         cubeSurf->UnlockRect();
         cubeSurf->Release();
      }
   }
}

//-----------------------------------------------------------------------------
// Init Dynamic
//-----------------------------------------------------------------------------
void GFXD3D9Cubemap::initDynamic( U32 texSize, GFXFormat faceFormat )
{
   if ( mCubeTex )
      return;

   if ( !mDynamic )
      GFXTextureManager::addEventDelegate( this, &GFXD3D9Cubemap::_onTextureEvent );

   mDynamic = true;
   mTexSize = texSize;
   mFaceFormat = faceFormat;
   
   LPDIRECT3DDEVICE9 D3D9Device = reinterpret_cast<GFXD3D9Device *>(GFX)->getDevice();

   // might want to try this as a 16 bit texture...
   D3D9Assert( D3D9Device->CreateCubeTexture( texSize,
                                            1, 
#ifdef TORQUE_OS_XENON
                                            0,
#else
                                            D3DUSAGE_RENDERTARGET, 
#endif
                                            GFXD3D9TextureFormat[faceFormat],
                                            D3DPOOL_DEFAULT, 
                                            &mCubeTex, 
                                            NULL ), NULL );
}

//-----------------------------------------------------------------------------
// Fills in face textures of cube map from existing textures
//-----------------------------------------------------------------------------
void GFXD3D9Cubemap::fillCubeTextures( GFXTexHandle *faces, LPDIRECT3DDEVICE9 D3DDevice)
{

   U32 levels = faces->getPointer()->getMipLevels();
   for (U32 mip = 0; mip < levels; mip++)
   {
      for (U32 i = 0; i < 6; i++)
      {
         // get cube face surface
         IDirect3DSurface9 *cubeSurf = NULL;
         D3D9Assert(mCubeTex->GetCubeMapSurface(faceList[i], mip, &cubeSurf), NULL);

         // get incoming texture surface
         GFXD3D9TextureObject *texObj = dynamic_cast<GFXD3D9TextureObject*>((GFXTextureObject*)faces[i]);
         IDirect3DSurface9 *inSurf;
         D3D9Assert(texObj->get2DTex()->GetSurfaceLevel(mip, &inSurf), NULL);

         // copy incoming texture into cube face
         D3D9Assert(GFXD3DX.D3DXLoadSurfaceFromSurface(cubeSurf, NULL, NULL, inSurf, NULL,
            NULL, D3DX_FILTER_NONE, 0), NULL);
         cubeSurf->Release();
         inSurf->Release();
      }
   }
}

//-----------------------------------------------------------------------------
// Set the cubemap to the specified texture unit num
//-----------------------------------------------------------------------------
void GFXD3D9Cubemap::setToTexUnit( U32 tuNum )
{
   static_cast<GFXD3D9Device *>(GFX)->getDevice()->SetTexture( tuNum, mCubeTex );
}

void GFXD3D9Cubemap::zombify()
{
   // Static cubemaps are handled by D3D
   if( mDynamic )
      releaseSurfaces();
}

void GFXD3D9Cubemap::resurrect()
{
   // Static cubemaps are handled by D3D
   if( mDynamic )
      initDynamic( mTexSize, mFaceFormat );
}
