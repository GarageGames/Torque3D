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

#ifndef _MQUAT_H_
#define _MQUAT_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

class MatrixF;
class AngAxisF;

//----------------------------------------------------------------------------
// unit quaternion class:

class QuatF
{
   //-------------------------------------- Public static constants
public:
   const static QuatF Identity;

  public:
   F32  x,y,z,w;

   QuatF() {} // no init constructor
   QuatF( F32 _x, F32 _y, F32 _z, F32 w );
   QuatF( const Point3F &axis, F32 angle );
   QuatF( const MatrixF & m );
   QuatF( const AngAxisF & a );
   QuatF( const EulerF & e );

   QuatF& set( F32 _x, F32 _y, F32 _z, F32 _w );
   QuatF& set( const Point3F &axis, F32 angle );
   QuatF& set( const MatrixF & m );
   QuatF& set( const AngAxisF & a );
   QuatF& set( const EulerF & e );

   int operator ==( const QuatF & c ) const;
   int operator !=( const QuatF & c ) const;
   QuatF& operator *=( const QuatF & c );
   QuatF& operator /=( const QuatF & c );
   QuatF& operator +=( const QuatF & c );
   QuatF& operator -=( const QuatF & c );
   QuatF& operator *=( F32 a );
   QuatF& operator /=( F32 a );

   QuatF operator-( const QuatF &c ) const;
   QuatF operator*( F32 a ) const;

   QuatF& square();
   QuatF& neg();
   F32  dot( const QuatF &q ) const;

   MatrixF* setMatrix( MatrixF * mat ) const;
   QuatF& normalize();
   QuatF& inverse();
   QuatF& identity();
   int    isIdentity() const;
   QuatF& slerp( const QuatF & q, F32 t );
   QuatF& extrapolate( const QuatF & q1, const QuatF & q2, F32 t );
   QuatF& interpolate( const QuatF & q1, const QuatF & q2, F32 t );
   F32  angleBetween( const QuatF & q );

   Point3F& mulP(const Point3F& a, Point3F* b);   // r = p * this
   QuatF& mul(const QuatF& a, const QuatF& b);    // This = a * b

   // Vectors passed in must be normalized
   QuatF& shortestArc( const VectorF &normalizedA, const VectorF &normalizedB );
};

// a couple simple utility methods
inline F32 QuatIsEqual(F32 a,F32 b,F32 epsilon = 0.0001f) { return mFabs(a-b) < epsilon; }
inline F32 QuatIsZero(F32 a,F32 epsilon = 0.0001f) { return mFabs(a) < epsilon; }

//----------------------------------------------------------------------------
// quaternion implementation:

inline QuatF::QuatF( F32 _x, F32 _y, F32 _z, F32 _w )
{
   set( _x, _y, _z, _w );
}

inline QuatF::QuatF( const Point3F &axis, F32 angle )
{
   set( axis, angle );
}

inline QuatF::QuatF( const AngAxisF & a )
{
   set( a );
}

inline QuatF::QuatF( const EulerF & e )
{
   set( e );
}

inline QuatF::QuatF( const MatrixF & m )
{
   set( m );
}

inline QuatF& QuatF::set( F32 _x, F32 _y, F32 _z, F32 _w )
{
   x = _x;
   y = _y;
   z = _z;
   w = _w;
   return *this;
}

inline int QuatF::operator ==( const QuatF & c ) const
{
   QuatF a = *this;
   QuatF b = c;
   a.normalize();
   b.normalize();
   b.inverse();
   a *= b;
   return a.isIdentity();
}

inline int QuatF::isIdentity() const
{
   return QuatIsZero( x ) && QuatIsZero( y ) && QuatIsZero( z );
}

inline QuatF& QuatF::identity()
{
   x = 0.0f;
   y = 0.0f;
   z = 0.0f;
   w = 1.0f;
   return *this;
}

inline int QuatF::operator !=( const QuatF & c ) const
{
   return !operator==( c );
}

inline QuatF& QuatF::operator +=( const QuatF & c )
{
   x += c.x;
   y += c.y;
   z += c.z;
   w += c.w;
   return *this;
}

inline QuatF& QuatF::operator -=( const QuatF & c )
{
   x -= c.x;
   y -= c.y;
   z -= c.z;
   w -= c.w;
   return *this;
}

inline QuatF& QuatF::operator *=( F32 a )
{
   x *= a;
   y *= a;
   z *= a;
   w *= a;
   return *this;
}

inline QuatF& QuatF::operator /=( F32 a )
{
   x /= a;
   y /= a;
   z /= a;
   w /= a;
   return *this;
}

inline QuatF QuatF::operator -( const QuatF &c ) const
{
   return QuatF( x - c.x,
                 y - c.y,
                 z - c.z,
                 w - c.w );
}

inline QuatF QuatF::operator *( F32 a ) const
{
   return QuatF( x * a,
                 y * a,
                 z * a,
                 w * a );
}

inline QuatF& QuatF::neg()
{
   x = -x;
   y = -y;
   z = -z;
   w = -w;
   return *this;
}

inline F32 QuatF::dot( const QuatF &q ) const
{
   return (w*q.w + x*q.x + y*q.y + z*q.z);
}

inline F32 QuatF::angleBetween( const QuatF & q )
{
   // angle between to quaternions
   return mAcos(x * q.x + y * q.y + z * q.z + w * q.w);
}

#endif // _MQUAT_H_
