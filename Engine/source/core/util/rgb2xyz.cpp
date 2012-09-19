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
#include "core/util/rgb2xyz.h"

#include "math/mMatrix.h"


namespace ConvertRGB
{

// http://www.w3.org/Graphics/Color/sRGB
static const F32 scRGB2XYZ[] = 
{
   0.4124f,  0.3576f,  0.1805f,  0.0f,
   0.2126f,  0.7152f,  0.0722f,  0.0f,
   0.0193f,  0.1192f,  0.9505f,  0.0f,
   0.0f,     0.0f,     0.0f,     1.0f,
};

static const F32 scXYZ2RGB[] =
{
   3.2410f,  -1.5374f,  -0.4986f, 0.0f,
   -0.9692f, 1.8760f,   0.0416f,  0.0f,
   0.0556f,  -0.2040f,  1.0570f,  0.0f, 
   0.0f,     0.0f,      0.0f,     1.0f,
};

ColorF toXYZ( const ColorF &rgbColor )
{
   const MatrixF &rgb2xyz = *((MatrixF *)scRGB2XYZ);

   ColorF retColor = rgbColor;
   rgb2xyz.mul( *(Point4F *)&retColor );
   return retColor;
}

ColorF fromXYZ( const ColorF &xyzColor )
{
   const MatrixF &xyz2rgb = *((MatrixF *)scXYZ2RGB);

   ColorF retColor = xyzColor;
   xyz2rgb.mul( *(Point4F *)&retColor );
   return retColor;
}

}
