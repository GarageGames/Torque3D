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

#ifndef _BITMAPUTILS_H_
#define _BITMAPUTILS_H_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

extern void (*bitmapExtrude5551)(const void *srcMip, void *mip, U32 height, U32 width);
extern void (*bitmapExtrudeRGB)(const void *srcMip, void *mip, U32 height, U32 width);
extern void (*bitmapExtrudeRGBA)(const void *srcMip, void *mip, U32 height, U32 width);
extern void (*bitmapConvertRGB_to_5551)(U8 *src, U32 pixels);
extern void (*bitmapConvertRGB_to_1555)(U8 *src, U32 pixels);
extern void (*bitmapConvertRGB_to_RGBX)( U8 **src, U32 pixels );
extern void (*bitmapConvertRGBX_to_RGB)( U8 **src, U32 pixels );
extern void (*bitmapConvertA8_to_RGBA)( U8 **src, U32 pixels );

void bitmapExtrudeRGB_c(const void *srcMip, void *mip, U32 height, U32 width);

#endif //_BITMAPUTILS_H_
