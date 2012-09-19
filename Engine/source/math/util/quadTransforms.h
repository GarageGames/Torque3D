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

#ifndef _QUADTOQUADTRANSFORMS_H_
#define _QUADTOQUADTRANSFORMS_H_

#ifndef _MPOINT2_H_
#include "math/mPoint2.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif

// NOTE: The code in these classes originate from the Wild Magic Source Code
// library by David Eberly and is used with permission.


/// This class does bilinear mapping of quadrilateral to a square.
class BiQuadToSqr
{
public:

   /// Constructs the transform class from the quadrilateral
   /// points in counter clockwise order.
   BiQuadToSqr(   const Point2F &p00, 
                  const Point2F &p10,
                  const Point2F &p11, 
                  const Point2F &p01 );

   /// Transforms the point.
   Point2F transform( const Point2F &p ) const;

protected:

   static F32 deviation( const Point2F &sp );

   Point2F m_kP00, m_kB, m_kC, m_kD;

   F32 m_fBC, m_fBD, m_fCD;

};


class BiSqrToQuad3D
{
public:

   BiSqrToQuad3D( const Point3F &pnt00, 
                  const Point3F &pnt10,
                  const Point3F &pnt11, 
                  const Point3F &pnt01 );

   Point3F transform( const Point2F &pnt ) const;

protected:

   Point3F p00, p01, p10, p11;
};

#endif // _QUADTOQUADTRANSFORMS_H_
