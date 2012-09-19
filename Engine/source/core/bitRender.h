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

#ifndef _BITRENDER_H_
#define _BITRENDER_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif

/// Functions for rendering to 1bpp bitmaps.
///
/// Used primarily for fast shadow rendering.
struct BitRender
{
   /// Render a triangle to a bitmap of 1-bit per pixel and size dim X dim.
   static void render(const Point2I *, const Point2I *, const Point2I *, S32 dim, U32 * bits);

   /// Render a number of triangle strips to 1-bit per pixel bmp of size dim by dim.
   static void render_strips(const U8 * draw, S32 numDraw, S32 szDraw, const U16 * indices, const Point2I * points, S32 dim, U32 * bits);

   /// Render a number of triangles to 1-bit per pixel bmp of size dim by dim.
   static void render_tris(const U8 * draw, S32 numDraw, S32 szDraw, const U16 * indices, const Point2I * points, S32 dim, U32 * bits);

   /// @name Render Bits
   /// These are used to convert a 1bpp bitmap to an 8bpp bitmap.
   ///
   /// @see Shadow::endRenderToBitmap
   /// @{

   /// Render bits to the bitmap.
   static void bitTo8Bit(U32 * bits, U32 * eightBits, S32 dim);

   /// Render bits to the bitmap, with gaussian pass.
   static void bitTo8Bit_3(U32 * bits, U32 * eightBits, S32 dim);
   /// @}
};

#endif // _BIT_RENDER_H_

