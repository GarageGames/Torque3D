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

#ifndef _MCONSTANTS_H_
#define _MCONSTANTS_H_

#undef M_PI
#undef M_SQRT2

#define M_PI         3.14159265358979323846
#define M_SQRT2      1.41421356237309504880

#define M_2PI        (3.1415926535897932384626433 * 2.0)
#define M_SQRTHALF   0.7071067811865475244008443

#define M_HALFPI     1.57079632679489661923

#define M_PI_F         3.14159265358979323846f
#define M_SQRT2_F      1.41421356237309504880f

#define M_2PI_F        (3.1415926535897932384626433f * 2.0f)
#define M_SQRTHALF_F   0.7071067811865475244008443f

#define M_HALFPI_F     1.57079632679489661923f

#define M_CONST_E_F    2.7182818284590452353602874f

#define POINT_EPSILON (1e-4) ///< Epsilon for point types.


/// Result of an overlap test.
enum OverlapTestResult
{
   GeometryInside = 1,          ///< Completely inside test volume/space.
   GeometryIntersecting = 0,    ///< Partly inside and partly outside test volume/space.
   GeometryOutside = -1         ///< No overlap with test volume/space.
};

#endif
