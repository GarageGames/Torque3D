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

#include "platform/platform.h"
#include "core/util/rgb2luv.h"

#include "core/util/rgb2xyz.h"
#include "math/mPoint3.h"
#include "math/mPoint2.h"


namespace ConvertRGB
{

ColorF toLUV( const ColorF &rgbColor )
{
   static const Point3F scXYZLUVDot( 1.0f, 15.0f, 3.0f );
   static const Point2F sc49( 4.0f, 9.0f );

   ColorF xyzColor = ConvertRGB::toXYZ( rgbColor );

   const Point2F &xyz_xy = *((Point2F *)&xyzColor);

   Point2F uvColor = sc49;
   uvColor.convolve( xyz_xy );
   uvColor /= mDot( *(Point3F *)&xyzColor, scXYZLUVDot );

   return ColorF( uvColor.x, uvColor.y, xyzColor.green, rgbColor.alpha );
}

ColorF toLUVScaled( const ColorF &rgbColor )
{
   ColorF luvColor = toLUV( rgbColor );
   luvColor.red /= 0.62f;
   luvColor.green /= 0.62f;
   return luvColor;
}

}
