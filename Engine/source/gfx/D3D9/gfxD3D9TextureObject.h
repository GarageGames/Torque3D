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

#ifndef _GFXD3D9TEXTUREOBJECT_H_
#define _GFXD3D9TEXTUREOBJECT_H_

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif

#ifndef _GFXTEXTUREMANAGER_H_
#include "gfx/gfxTextureManager.h"
#endif


class GFXD3D9TextureObject : public GFXTextureObject
{
protected:
   static U32 mTexCount;
   GFXTexHandle   mLockTex;
   D3DLOCKED_RECT mLockRect;
   bool           mLocked;

   IDirect3DBaseTexture9 *mD3DTexture;

   // used for z buffers...
   IDirect3DSurface9 *mD3DSurface;

public:

   GFXD3D9TextureObject( GFXDevice * d, GFXTextureProfile *profile);
   ~GFXD3D9TextureObject();

   IDirect3DBaseTexture9 *    getTex(){ return mD3DTexture; }
   IDirect3DTexture9 *        get2DTex(){ return (LPDIRECT3DTEXTURE9) mD3DTexture; }
   IDirect3DTexture9 **       get2DTexPtr(){ return (LPDIRECT3DTEXTURE9*) &mD3DTexture; }
   IDirect3DVolumeTexture9 *  get3DTex(){ return (LPDIRECT3DVOLUMETEXTURE9) mD3DTexture; }
   IDirect3DVolumeTexture9 ** get3DTexPtr(){ return (LPDIRECT3DVOLUMETEXTURE9*) &mD3DTexture; }

   void release();

   bool isManaged;

   virtual GFXLockedRect * lock(U32 mipLevel = 0, RectI *inRect = NULL);
   virtual void unlock(U32 mipLevel = 0 );

   virtual bool copyToBmp(GBitmap* bmp);
   IDirect3DSurface9 *getSurface() {return mD3DSurface;}
   IDirect3DSurface9 **getSurfacePtr() {return &mD3DSurface;}

   // GFXResource
   void zombify();
   void resurrect();

#ifdef TORQUE_DEBUG
   virtual void pureVirtualCrash() {};
#endif
};


#endif
