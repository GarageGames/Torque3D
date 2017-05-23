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

//-----------------------------------------------------------------------------
// Ray to triangle intersection test code originally by Tomas Akenine-Möller
// and Ben Trumbore.
// http://www.cs.lth.se/home/Tomas_Akenine_Moller/code/
// Ported to TGE by DAW, 2005-7-15
//-----------------------------------------------------------------------------

#ifndef _TRIRAYCHECK_H_
#define _TRIRAYCHECK_H_

#include "math/mPoint2.h"
#include "math/mPoint3.h"

bool intersect_triangle(Point3F orig, Point3F dir,
                   Point3F vert0, Point3F vert1, Point3F vert2,
                   F32& t, F32& u, F32& v);

//*** Taken from TSE, but based on the above
bool castRayTriangle(const Point3F& orig, const Point3F& dir, const Point3F& vert0, const Point3F& vert1, const Point3F& vert2, F32 &t, Point2F &bary);
bool castRayTriangle(const Point3D &orig, const Point3D &dir, const Point3D &vert0, const Point3D &vert1, const Point3D &vert2);

#endif // _TRIRAYCHECK_H_
