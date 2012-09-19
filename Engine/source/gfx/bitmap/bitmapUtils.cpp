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

#include "gfx/bitmap/bitmapUtils.h"

#include "platform/platform.h"


void bitmapExtrude5551_c(const void *srcMip, void *mip, U32 srcHeight, U32 srcWidth)
{
   const U16 *src = (const U16 *) srcMip;
   U16 *dst = (U16 *) mip;
   U32 stride = srcHeight != 1 ? srcWidth : 0;

   U32 width  = srcWidth  >> 1;
   U32 height = srcHeight >> 1;
   if (width  == 0) width  = 1;
   if (height == 0) height = 1;

   if (srcWidth != 1)
   {
      for(U32 y = 0; y < height; y++)
      {
         for(U32 x = 0; x < width; x++)
         {
            U32 a = src[0];
            U32 b = src[1];
            U32 c = src[stride];
            U32 d = src[stride+1];
#if defined(TORQUE_BIG_ENDIAN)
            dst[x] = (((  (a >> 10) + (b >> 10) + (c >> 10) + (d >> 10)) >> 2) << 10) |
                     ((( ((a >> 5) & 0x1F) + ((b >> 5) & 0x1F) + ((c >> 5) & 0x1F) + ((d >> 5) & 0x1F)) >> 2) << 5) |
                     ((( ((a >> 0) & 0x1F) + ((b >> 0) & 0x1F) + ((c >> 0) & 0x1F) + ((d >> 0) & 0x1F)) >> 2) << 0);
#else
            dst[x] = (((  (a >> 11) + (b >> 11) + (c >> 11) + (d >> 11)) >> 2) << 11) |
                     ((( ((a >> 6) & 0x1F) + ((b >> 6) & 0x1F) + ((c >> 6) & 0x1F) + ((d >> 6) & 0x1F)) >> 2) << 6) |
                     ((( ((a >> 1) & 0x1F) + ((b >> 1) & 0x1F) + ((c >> 1) & 0x1F) + ((d >> 1) & 0x1F)) >> 2) << 1);
#endif
            src += 2;
         }
         src += stride;
         dst += width;
      }
   }
   else
   {
      for(U32 y = 0; y < height; y++)
      {
         U32 a = src[0];
         U32 c = src[stride];
#if defined(TORQUE_OS_MAC)
            dst[y] = ((( (a >> 10) + (c >> 10)) >> 1) << 10) |
                     ((( ((a >> 5) & 0x1F) + ((c >> 5) & 0x1f)) >> 1) << 5) |
                     ((( ((a >> 0) & 0x1F) + ((c >> 0) & 0x1f)) >> 1) << 0);
#else
            dst[y] = ((( (a >> 11) + (c >> 11)) >> 1) << 11) |
                     ((( ((a >> 6) & 0x1f) + ((c >> 6) & 0x1f)) >> 1) << 6) |
                     ((( ((a >> 1) & 0x1F) + ((c >> 1) & 0x1f)) >> 1) << 1);
#endif
         src += 1 + stride;
      }
   }
}


//--------------------------------------------------------------------------
void bitmapExtrudeRGB_c(const void *srcMip, void *mip, U32 srcHeight, U32 srcWidth)
{
   const U8 *src = (const U8 *) srcMip;
   U8 *dst = (U8 *) mip;
   U32 stride = srcHeight != 1 ? (srcWidth) * 3 : 0;

   U32 width  = srcWidth  >> 1;
   U32 height = srcHeight >> 1;
   if (width  == 0) width  = 1;
   if (height == 0) height = 1;

   if (srcWidth != 1)
   {
      for(U32 y = 0; y < height; y++)
      {
         for(U32 x = 0; x < width; x++)
         {
            *dst++ = (U32(*src) + U32(src[3]) + U32(src[stride]) + U32(src[stride+3]) + 2) >> 2;
            src++;
            *dst++ = (U32(*src) + U32(src[3]) + U32(src[stride]) + U32(src[stride+3]) + 2) >> 2;
            src++;
            *dst++ = (U32(*src) + U32(src[3]) + U32(src[stride]) + U32(src[stride+3]) + 2) >> 2;
            src += 4;
         }
         src += stride;   // skip
      }
   }
   else
   {
      for(U32 y = 0; y < height; y++)
      {
         *dst++ = (U32(*src) + U32(src[stride]) + 1) >> 1;
         src++;
         *dst++ = (U32(*src) + U32(src[stride]) + 1) >> 1;
         src++;
         *dst++ = (U32(*src) + U32(src[stride]) + 1) >> 1;
         src += 4;

         src += stride;   // skip
      }
   }
}

//--------------------------------------------------------------------------
void bitmapExtrudeRGBA_c(const void *srcMip, void *mip, U32 srcHeight, U32 srcWidth)
{
   const U8 *src = (const U8 *) srcMip;
   U8 *dst = (U8 *) mip;
   U32 stride = srcHeight != 1 ? (srcWidth) * 4 : 0;

   U32 width  = srcWidth  >> 1;
   U32 height = srcHeight >> 1;
   if (width  == 0) width  = 1;
   if (height == 0) height = 1;

   if (srcWidth != 1)
   {
      for(U32 y = 0; y < height; y++)
      {
         for(U32 x = 0; x < width; x++)
         {
            *dst++ = (U32(*src) + U32(src[4]) + U32(src[stride]) + U32(src[stride+4]) + 2) >> 2;
            src++;
            *dst++ = (U32(*src) + U32(src[4]) + U32(src[stride]) + U32(src[stride+4]) + 2) >> 2;
            src++;
            *dst++ = (U32(*src) + U32(src[4]) + U32(src[stride]) + U32(src[stride+4]) + 2) >> 2;
            src++;
            *dst++ = (U32(*src) + U32(src[4]) + U32(src[stride]) + U32(src[stride+4]) + 2) >> 2;
            src += 5;
         }
         src += stride;   // skip
      }
   }
   else
   {
      for(U32 y = 0; y < height; y++)
      {
         *dst++ = (U32(*src) + U32(src[stride]) + 1) >> 1;
         src++;
         *dst++ = (U32(*src) + U32(src[stride]) + 1) >> 1;
         src++;
         *dst++ = (U32(*src) + U32(src[stride]) + 1) >> 1;
         src++;
         *dst++ = (U32(*src) + U32(src[stride]) + 1) >> 1;
         src += 5;

         src += stride;   // skip
      }
   }
}

void (*bitmapExtrude5551)(const void *srcMip, void *mip, U32 height, U32 width) = bitmapExtrude5551_c;
void (*bitmapExtrudeRGB)(const void *srcMip, void *mip, U32 srcHeight, U32 srcWidth) = bitmapExtrudeRGB_c;
void (*bitmapExtrudeRGBA)(const void *srcMip, void *mip, U32 srcHeight, U32 srcWidth) = bitmapExtrudeRGBA_c;


//--------------------------------------------------------------------------

void bitmapConvertRGB_to_1555_c(U8 *src, U32 pixels)
{
   U16 *dst = (U16 *)src;
   for(U32 j = 0; j < pixels; j++)
   {
      U32 r = src[0] >> 3;
      U32 g = src[1] >> 3;
      U32 b = src[2] >> 3;

#if defined(TORQUE_OS_MAC)
      *dst++ = 0x8000 | (b << 10) | (g << 5) | (r << 0);
#else
      *dst++ = b | (g << 5) | (r << 10) | 0x8000;
#endif
      src += 3;
   }
}

void (*bitmapConvertRGB_to_1555)(U8 *src, U32 pixels) = bitmapConvertRGB_to_1555_c;

//------------------------------------------------------------------------------

void bitmapConvertRGB_to_5551_c(U8 *src, U32 pixels)
{
   U16 *dst = (U16 *)src;
   for(U32 j = 0; j < pixels; j++)
   {
      U32 r = src[0] >> 3;
      U32 g = src[1] >> 3;
      U32 b = src[2] >> 3;

#if defined(TORQUE_OS_MAC)
      *dst++ = (1 << 15) | (b << 10) | (g << 5) | (r << 0);
#else
      *dst++ = (b << 1) | (g << 6) | (r << 11) | 1;
#endif
      src += 3;
   }
}



void (*bitmapConvertRGB_to_5551)(U8 *src, U32 pixels) = bitmapConvertRGB_to_5551_c;

//------------------------------------------------------------------------------

void bitmapConvertRGB_to_RGBX_c( U8 **src, U32 pixels )
{
   const U8 *oldBits = *src;
   U8 *newBits = new U8[pixels * 4];
   dMemset( newBits, 0xFF, pixels * 4 ); // This is done to set alpha values -patw

   // Copy the bits over to the new memory
   for( U32 i = 0; i < pixels; i++ )
      dMemcpy( &newBits[i * 4], &oldBits[i * 3], sizeof(U8) * 3 );

   // Now hose the old bits
   delete [] *src;
   *src = newBits;
}

void (*bitmapConvertRGB_to_RGBX)( U8 **src, U32 pixels ) = bitmapConvertRGB_to_RGBX_c;

//------------------------------------------------------------------------------

void bitmapConvertRGBX_to_RGB_c( U8 **src, U32 pixels )
{
   const U8 *oldBits = *src;
   U8 *newBits = new U8[pixels * 3];

   // Copy the bits over to the new memory
   for( U32 i = 0; i < pixels; i++ )
      dMemcpy( &newBits[i * 3], &oldBits[i * 4], sizeof(U8) * 3 );

   // Now hose the old bits
   delete [] *src;
   *src = newBits;
}

void (*bitmapConvertRGBX_to_RGB)( U8 **src, U32 pixels ) = bitmapConvertRGBX_to_RGB_c;

//------------------------------------------------------------------------------

void bitmapConvertA8_to_RGBA_c( U8 **src, U32 pixels )
{
   const U8 *oldBits = *src;
   U8 *newBits = new U8[pixels * 4];

   // Zero new bits
   dMemset( newBits, 0, pixels * 4 );

   // Copy Alpha values
   for( U32 i = 0; i < pixels; i++ )      
      newBits[i * 4 + 3] = oldBits[i];

   // Now hose the old bits
   delete [] *src;
   *src = newBits;
}

void (*bitmapConvertA8_to_RGBA)( U8 **src, U32 pixels ) = bitmapConvertA8_to_RGBA_c;
