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

#ifndef _SGUTIL_H_
#define _SGUTIL_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

class Frustum;
class RectI;
class MatrixF;
class PlaneF;

struct SGWinding
{
   Point3F points[32];
   U32     numPoints;
};

bool sgComputeNewFrustum(const Frustum    &oldFrustum,
                         const F64        nearPlane,
                         const F64        farPlane,
                         const RectI&     oldViewport,
                         const SGWinding* windings,
                         const U32        numWindings,
                         const MatrixF&   modelview,
                         F64              *newFrustum,
                         RectI&           newViewport,
                         const bool       flippedMatrix);

/// Compute frustrum planes.
///
/// Frustum parameters are:
///  - [0] = left
///  - [1] = right
///  - [2] = top
///  - [3] = bottom
///  - [4] = near
///  - [5] = far
void sgComputeOSFrustumPlanes(const F64      frustumParameters[6],
                              const MatrixF& worldSpaceToObjectSpace,
                              const Point3F& wsCamPoint,
                              PlaneF&        outFarPlane,
                              PlaneF&        outXMinPlane,
                              PlaneF&        outXMaxPlane,
                              PlaneF&        outYMinPlane,
                              PlaneF&        outYMaxPlane);

void sgOrientClipPlanes(PlaneF * planes, const Point3F & camPos, const Point3F & leftUp, const Point3F & leftDown, const Point3F & rightUp, const Point3F & rightDown);

#endif  // _H_SGUTIL_
