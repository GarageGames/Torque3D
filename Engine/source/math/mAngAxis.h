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

#ifndef _MANGAXIS_H_
#define _MANGAXIS_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

class MatrixF;
class QuatF;

//----------------------------------------------------------------------------
// rotation about an arbitrary axis through the origin:

class AngAxisF
{
  public:
   Point3F axis;
   F32  angle;

   AngAxisF();
   AngAxisF( const Point3F & _axis, F32 _angle );
   explicit AngAxisF( const MatrixF &m );
   explicit AngAxisF( const QuatF &q );

   AngAxisF& set( const Point3F & _axis, F32 _angle );
   AngAxisF& set( const MatrixF & m );
   AngAxisF& set( const QuatF & q );

   bool operator ==( const AngAxisF & c ) const;
   bool operator !=( const AngAxisF & c ) const;

   MatrixF * setMatrix( MatrixF * mat ) const;

   static void RotateX(F32 angle, MatrixF * mat);
   static void RotateY(F32 angle, MatrixF * mat);
   static void RotateZ(F32 angle, MatrixF * mat);

   static void RotateX(F32 angle, const Point3F & from, Point3F * to);
   static void RotateY(F32 angle, const Point3F & from, Point3F * to);
   static void RotateZ(F32 angle, const Point3F & from, Point3F * to);
};

//----------------------------------------------------------------------------
// AngAxisF implementation:

inline AngAxisF::AngAxisF()
{
}

inline AngAxisF::AngAxisF( const Point3F & _axis, F32 _angle )
{
   set(_axis,_angle);
}

inline AngAxisF::AngAxisF( const MatrixF & mat )
{
   set(mat);
}

inline AngAxisF::AngAxisF( const QuatF & quat )
{
   set(quat);
}

inline AngAxisF& AngAxisF::set( const Point3F & _axis, F32 _angle )
{
   axis = _axis;
   angle = _angle;
   return *this;
}

inline bool AngAxisF::operator ==( const AngAxisF & c ) const
{
   return mFabs(angle-c.angle) < 0.0001f && (axis == c.axis);
}

inline bool AngAxisF::operator !=( const AngAxisF & c ) const
{
   return !(*this == c);
}

#endif // _MANGAXIS_H_
