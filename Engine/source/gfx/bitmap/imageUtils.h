//-----------------------------------------------------------------------------
// Copyright (c) 2016 GarageGames, LLC
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

#ifndef _IMAGE_UTILS_H_
#define _IMAGE_UTILS_H_

#ifndef _SWIZZLE_H_
#include "core/util/swizzle.h"
#endif
#ifndef _GFXENUMS_H_
#include "gfx/gfxEnums.h"
#endif

struct DDSFile;

namespace ImageUtil
{
   enum CompressQuality
   {
      LowQuality,
      MediumQuality,
      HighQuality
   };

   // compress raw pixel data, expects rgba format
   bool rawCompress(const U8 *srcRGBA, U8 *dst, const S32 width, const S32 height, const GFXFormat compressFormat, const CompressQuality compressQuality = LowQuality);
   // compress DDSFile
   bool ddsCompress(DDSFile *srcDDS, const GFXFormat compressFormat, const CompressQuality compressQuality = LowQuality);
   // decompress compressed pixel data, dest data should be rgba format
   bool decompress(const U8 *src, U8 *dstRGBA, const S32 width, const S32 height, const GFXFormat srcFormat);
   //swizzle dds file
   void swizzleDDS(DDSFile *srcDDS, const Swizzle<U8, 4> &swizzle);
   //check if a GFXFormat is compressed
   bool isCompressedFormat(const GFXFormat format);
   bool isSRGBFormat(const GFXFormat format);
   //check if a GFXFormat has an alpha channel
   bool isAlphaFormat(const GFXFormat format);

   //convert to sRGB format
   GFXFormat toSRGBFormat(const GFXFormat format);
};

#endif