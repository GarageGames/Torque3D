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

#ifndef _TSTRANSFORM_H_
#define _TSTRANSFORM_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif

class Stream;

/// compressed quaternion class
struct Quat16
{
   enum { MAX_VAL = 0x7fff };

   S16 x, y, z, w;

   void read(Stream *);
   void write(Stream *);

   void identity();
   QuatF getQuatF() const;
   QuatF &getQuatF( QuatF * q ) const;
   void set( const QuatF & q );
   S32 operator==( const Quat16 & q ) const;
   S32 operator!=( const Quat16 & q ) const { return !(*this == q); }
};

/// class to handle general scaling case
///
/// transform = rot * scale * inv(rot)
struct TSScale
{
   QuatF            mRotate;
   Point3F          mScale;

   void identity() { mRotate.identity(); mScale.set( 1.0f,1.0f,1.0f ); }
   S32 operator==( const TSScale & other ) const { return mRotate==other.mRotate && mScale==other.mScale; }
};

/// struct for encapsulating ts transform related static functions
struct TSTransform
{
   static Point3F & interpolate(const Point3F & p1, const Point3F & p2, F32 t, Point3F *);
   static F32       interpolate(F32             p1, F32             p2, F32 t);
   static QuatF   & interpolate(const QuatF   & q1, const QuatF   & q2, F32 t, QuatF   *);
   static TSScale & interpolate(const TSScale & s1, const TSScale & s2, F32 t, TSScale *);

   static void      setMatrix(const QuatF     &  q, MatrixF *);
   static void      setMatrix(const QuatF     &  q, const Point3F & p, MatrixF *);

   static void      applyScale(F32 scale, MatrixF *);
   static void      applyScale(const Point3F & scale, MatrixF *);
   static void      applyScale(const TSScale & scale, MatrixF *);
};

inline Point3F & TSTransform::interpolate(const Point3F & p1, const Point3F & p2, F32 t, Point3F * p)
{
   p->x = p1.x + t * (p2.x-p1.x);
   p->y = p1.y + t * (p2.y-p1.y);
   p->z = p1.z + t * (p2.z-p1.z);
   return *p;
}

inline F32 TSTransform::interpolate(F32 p1, F32 p2, F32 t)
{
   return p1 + t*(p2-p1);
}

inline TSScale & TSTransform::interpolate(const TSScale & s1, const TSScale & s2, F32 t, TSScale * s)
{
   TSTransform::interpolate(s1.mRotate,s2.mRotate,t,&s->mRotate);
   TSTransform::interpolate(s1.mScale,s2.mScale,t,&s->mScale);
   return *s;
}

inline void TSTransform::setMatrix( const QuatF & q, const Point3F & p, MatrixF * pDest )
{
   q.setMatrix(pDest);
   pDest->setColumn(3,p);
}

inline void TSTransform::setMatrix( const QuatF & q, MatrixF * pDest )
{
   q.setMatrix(pDest);
}

#endif
