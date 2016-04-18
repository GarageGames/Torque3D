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

#ifndef _GFXD3DTEXTUREMANAGER_H_
#define _GFXD3DTEXTUREMANAGER_H_

#include "gfx/D3D11/gfxD3D11TextureObject.h"
#include "core/util/safeRelease.h"

class GFXD3D11TextureManager : public GFXTextureManager 
{
   friend class GFXD3D11TextureObject;

public:
   GFXD3D11TextureManager();
   virtual ~GFXD3D11TextureManager();
   void createResourceView(U32 height, U32 width, U32 depth, DXGI_FORMAT format, U32 numMipLevels,U32 usageFlags, GFXTextureObject *inTex);
protected:

   // GFXTextureManager
   GFXTextureObject *_createTextureObject(   U32 height, 
                                             U32 width,
                                             U32 depth,
                                             GFXFormat format,
                                             GFXTextureProfile *profile,
                                             U32 numMipLevels,
                                             bool forceMips = false,
                                             S32 antialiasLevel = 0,
                                             GFXTextureObject *inTex = NULL );
   
   bool _loadTexture(GFXTextureObject *texture, DDSFile *dds);
   bool _loadTexture(GFXTextureObject *texture, GBitmap *bmp);
   bool _loadTexture(GFXTextureObject *texture, void *raw);
   bool _refreshTexture(GFXTextureObject *texture);
   bool _freeTexture(GFXTextureObject *texture, bool zombify = false);
   
private:
   U32 mCurTexSet[TEXTURE_STAGE_COUNT];

   void _innerCreateTexture(GFXD3D11TextureObject *obj, U32 height, U32 width, U32 depth, GFXFormat format, GFXTextureProfile *profile, U32 numMipLevels, bool forceMips = false, S32 antialiasLevel = 0);
};

#endif
