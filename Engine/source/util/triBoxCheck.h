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
// AABB-triangle overlap test code originally by Tomas Akenine-Möller
//               Assisted by Pierre Terdiman and David Hunt
// http://www.cs.lth.se/home/Tomas_Akenine_Moller/code/
// Ported to TSE by BJG, 2005-4-14
// Modified to avoid a lot of copying by ASM, 2007-9-28
//-----------------------------------------------------------------------------

#ifndef _TRIBOXCHECK_H_
#define _TRIBOXCHECK_H_

#include "math/mPoint3.h"
#include "math/mBox.h"

bool triBoxOverlap(const Point3F &boxcenter, const Point3F &boxhalfsize, const Point3F triverts[3]);

/// Massage stuff into right format for triBoxOverlap test. This is really
/// just a helper function - use the other version if you want to be fast!
inline bool triBoxOverlap(Box3F box, Point3F a, Point3F b, Point3F c)
{
   Point3F halfSize(box.len_x() / 2.f, box.len_y() / 2.f, box.len_z() / 2.f);

   Point3F center;
   box.getCenter(&center);

   Point3F verts[3] = {a,b,c};

   return triBoxOverlap(center, halfSize, verts);
}

#endif