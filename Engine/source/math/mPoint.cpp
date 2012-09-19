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
#include "math/mPoint2.h"
#include "math/mPoint3.h"
#include "math/mPoint4.h"


const Point2I Point2I::One(1, 1);
const Point2I Point2I::Zero(0, 0);
const Point2I Point2I::Min(S32_MIN, S32_MIN);
const Point2I Point2I::Max(S32_MAX, S32_MAX);

const Point2F Point2F::One(1.0f, 1.0f);
const Point2F Point2F::Zero(0.0f, 0.0f);
const Point2F Point2F::Min(F32_MIN, F32_MIN);
const Point2F Point2F::Max(F32_MAX, F32_MAX);

const Point2D Point2D::One(1.0, 1.0);
const Point2D Point2D::Zero(0.0, 0.0);

const Point3I Point3I::One(1, 1, 1);
const Point3I Point3I::Zero(0, 0, 0);

const Point3F Point3F::One(1.0f, 1.0f, 1.0f);
const Point3F Point3F::Zero(0.0f, 0.0f, 0.0f);
const Point3F Point3F::Min(F32_MIN, F32_MIN, F32_MIN);
const Point3F Point3F::Max(F32_MAX, F32_MAX, F32_MAX);
const Point3F Point3F::UnitX(1.0f, 0.0f, 0.0f);
const Point3F Point3F::UnitY(0.0f, 1.0f, 0.0f);
const Point3F Point3F::UnitZ(0.0f, 0.0f, 1.0f);

const Point3D Point3D::One(1.0, 1.0, 1.0);
const Point3D Point3D::Zero(0.0, 0.0, 0.0);

const Point4I Point4I::One(1, 1, 1, 1);
const Point4I Point4I::Zero(0, 0, 0, 0);

const Point4F Point4F::One(1.0f, 1.0f, 1.0f, 1.0f);
const Point4F Point4F::Zero(0.0f, 0.0f, 0.0f, 0.0f);


Point3F mPerp( const Point3F &inVec )
{   
   AssertFatal( inVec.len() > 0.0f, "mPerp() - zero length vector has no perp!" );
   AssertFatal( inVec.isUnitLength(), "mPerp() - passed vector must be normalized!" );

   U32 idx = inVec.getLeastComponentIndex();

   Point3F vec( 0.0f, 0.0f, 0.0f );
   vec[idx] = 1.0f;

   Point3F outVec = mCross( inVec, vec );
   outVec.normalize();

   //AssertFatal( mIsZero( mDot( inVec, outVec ) ), "mPerp, failed to generate perpendicular" );

   return outVec;   
}

