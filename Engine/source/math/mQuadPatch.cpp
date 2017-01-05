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

#include "math/mQuadPatch.h"


//******************************************************************************
// Quadratic spline patch
//******************************************************************************
QuadPatch::QuadPatch()
{
   setNumReqControlPoints(3);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void QuadPatch::calcABC( const Point3F *points )
{
   a = points[2] - points[1];
   b = points[1] - points[0];
   c = points[0];
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void QuadPatch::submitControlPoints( SplCtrlPts &points )
{
   Parent::submitControlPoints( points );
   calcABC( points.getPoint(0) );
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void QuadPatch::setControlPoint( Point3F &point, S32 index )
{
   ( (SplCtrlPts*) getControlPoints() )->setPoint( point, index );
   calcABC( getControlPoint(0) );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void QuadPatch::calc( F32 t, Point3F &result )
{
   F32 t2 = t*t;
   result = a*t2 + b*t + c;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void QuadPatch::calc( Point3F *points, F32 t, Point3F &result )
{
   calcABC( points );
   calc( t, result );
}
