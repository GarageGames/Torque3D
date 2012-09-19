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

#include "gfx/gfxCubemap.h"
#include "gfx/gfxDevice.h"
#include "gfx/bitmap/gBitmap.h"
#include "gfx/gfxTextureManager.h"


GFXCubemap::~GFXCubemap()
{
   // If we're not dynamic and we were loaded from a
   // file then give the texture manager a chance to
   // remove us from the cache.
   if ( mPath.isNotEmpty() )
      TEXMGR->releaseCubemap( this );
}

void GFXCubemap::initNormalize( U32 size )
{
   Point3F axis[6] =
         {Point3F(1.0, 0.0, 0.0), Point3F(-1.0,  0.0,  0.0),
         Point3F(0.0, 1.0, 0.0), Point3F( 0.0, -1.0,  0.0),
         Point3F(0.0, 0.0, 1.0), Point3F( 0.0,  0.0, -1.0),};
   Point3F s[6] =
         {Point3F(0.0, 0.0, -1.0), Point3F( 0.0, 0.0, 1.0),
         Point3F(1.0, 0.0,  0.0), Point3F( 1.0, 0.0, 0.0),
         Point3F(1.0, 0.0,  0.0), Point3F(-1.0, 0.0, 0.0),};
   Point3F t[6] =
         {Point3F(0.0, -1.0, 0.0), Point3F(0.0, -1.0,  0.0),
         Point3F(0.0,  0.0, 1.0), Point3F(0.0,  0.0, -1.0),
         Point3F(0.0, -1.0, 0.0), Point3F(0.0, -1.0,  0.0),};

   F32 span = 2.0;
   F32 start = -1.0;

   F32 stride = span / F32(size - 1);
   GFXTexHandle faces[6];

   for(U32 i=0; i<6; i++)
   {
      GFXTexHandle &tex = faces[i];
      GBitmap *bitmap = new GBitmap(size, size);

      // fill in...
      for(U32 v=0; v<size; v++)
      {
         for(U32 u=0; u<size; u++)
         {
            Point3F vector;
            vector = axis[i] +
               ((F32(u) * stride) + start) * s[i] +
               ((F32(v) * stride) + start) * t[i];
            vector.normalizeSafe();
            vector = ((vector * 0.5) + Point3F(0.5, 0.5, 0.5)) * 255.0;
            vector.x = mClampF(vector.x, 0.0f, 255.0f);
            vector.y = mClampF(vector.y, 0.0f, 255.0f);
            vector.z = mClampF(vector.z, 0.0f, 255.0f);
            // easy way to avoid knowledge of the format (RGB, RGBA, RGBX, ...)...
            U8 *bits = bitmap->getAddress(u, v);
            bits[0] = U8(vector.x);
            bits[1] = U8(vector.y);
            bits[2] = U8(vector.z);
         }
      }

      tex.set(bitmap, &GFXDefaultStaticDiffuseProfile, true, "Cubemap");
   }

   initStatic(faces);
}

const String GFXCubemap::describeSelf() const
{
   // We've got nothing
   return String();
}


bool GFXCubemapHandle::set( const String &cubemapDDS )
{
   /// Free the previous handle to give us
   /// back any texture memory when it can.
   free();

   // Let the texture manager find this for us.
   StrongRefPtr<GFXCubemap>::set( TEXMGR->createCubemap( cubemapDDS ) );

   return isValid();
}
