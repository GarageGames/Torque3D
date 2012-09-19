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

#include "math/mMath.h"
#include "gfx/bitmap/bitmapUtils.h"

//--------------------------------------------------------------------------
void bitmapExtrude5551_asm(const void *srcMip, void *mip, U32 height, U32 width)
{
   const U16 *src = (const U16 *) srcMip;
   U16 *dst = (U16 *) mip;
   U32 stride = width << 1;

   for(U32 y = 0; y < height; y++)
   {
      for(U32 x = 0; x < width; x++)
      {
         U32 a = src[0];
         U32 b = src[1];
         U32 c = src[stride];
         U32 d = src[stride+1];
         dst[x] = ((((a >> 11) + (b >> 11) + (c >> 11) + (d >> 11)) >> 2) << 11) |
                  (((  ((a >> 6) & 0x1f) + ((b >> 6) & 0x1f) + ((c >> 6) & 0x1f) + ((d >> 6) & 0x1F) ) >> 2) << 6) |
                  ((( ((a >> 1) & 0x1F) + ((b >> 1) & 0x1F) + ((c >> 1) & 0x1f) + ((d >> 1) & 0x1f)) >> 2) << 1);
         src += 2;
      }
      src += stride;
      dst += width;
   }
}

//--------------------------------------------------------------------------
void PlatformBlitInit()
{
   bitmapExtrude5551 = bitmapExtrude5551_asm;
   bitmapExtrudeRGB  = bitmapExtrudeRGB_c;

   if (Platform::SystemInfo.processor.properties & CPU_PROP_MMX)
   {
      // JMQ: haven't bothered porting mmx bitmap funcs because they don't
      // seem to offer a big performance boost right now.
   }
}

