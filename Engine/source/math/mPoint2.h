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

#ifndef _MPOINT2_H_
#define _MPOINT2_H_

#ifndef _MMATHFN_H_
#include "math/mMathFn.h"
#endif

//------------------------------------------------------------------------------
/// 2D integer point
///
/// Uses S32 internally.
class Point2I
{
   //-------------------------------------- Public data
  public:
   S32 x;   ///< X position
   S32 y;   ///< Y position

   //-------------------------------------- Public interface
  public:
   Point2I();                               ///< Create an uninitialized point.
   Point2I(const Point2I&);                 ///< Copy constructor
   Point2I(S32 in_x, S32 in_y);             ///< Create point from two co-ordinates.

   //-------------------------------------- Non-math mutators and misc functions
   void set(S32 in_x, S32 in_y);            ///< Set (x,y) position
   void setMin(const Point2I&);             ///< Store lesser co-ordinates from parameter in this point.
   void setMax(const Point2I&);             ///< Store greater co-ordinates from parameter in this point.
   void zero();                             ///< Zero all values

   //-------------------------------------- Math mutators
   void neg();                              ///< Invert sign of point's co-ordinates.
   void convolve(const Point2I&);           ///< Convolve this point by parameter.

   //-------------------------------------- Queries
   bool isZero() const;                     ///< Is this point at the origin? (No epsilon used)
   F32  len() const;                        ///< Get the length of the point
   S32  lenSquared() const;                 ///< Get the length-squared of the point

   //-------------------------------------- Overloaded operators
  public:
   operator S32*() { return &x; }
   operator const S32*() const { return &x; }

   // Comparison operators
   bool operator==(const Point2I&) const;
   bool operator!=(const Point2I&) const;

   // Arithmetic w/ other points
   Point2I  operator+(const Point2I&) const;
   Point2I  operator-(const Point2I&) const;
   Point2I& operator+=(const Point2I&);
   Point2I& operator-=(const Point2I&);

   // Arithmetic w/ scalars
   Point2I  operator*(S32) const;
   Point2I& operator*=(S32);
   Point2I  operator/(S32) const;
   Point2I& operator/=(S32);

   Point2I  operator*(const Point2I&) const;
   Point2I& operator*=(const Point2I&);
   Point2I  operator/(const Point2I&) const;
   Point2I& operator/=(const Point2I&);

   // Unary operators
   Point2I operator-() const;

	//-------------------------------------- Public static constants
  public:
	const static Point2I One;
	const static Point2I Zero;
	const static Point2I Min;
	const static Point2I Max;
};

//------------------------------------------------------------------------------
/// 2D floating-point point.
class Point2F
{
   //-------------------------------------- Public data
  public:
   F32 x;
   F32 y;

  public:
   Point2F();                           ///< Create uninitialized point.
   Point2F(const Point2F&);             ///< Copy constructor
   Point2F(F32 _x, F32 _y);             ///< Create point from co-ordinates.

   //-------------------------------------- Non-math mutators and misc functions
  public:
   void set(F32 _x, F32 _y);            ///< Set point's co-ordinates.

   void setMin(const Point2F&);         ///< Store lesser co-ordinates.
   void setMax(const Point2F&);         ///< Store greater co-ordinates.

   /// Interpolate from a to b, based on c.
   ///
   /// @param   a   Starting point.
   /// @param   b   Ending point.
   /// @param   c   Interpolation factor (0.0 .. 1.0).
   void interpolate(const Point2F& a, const Point2F& b, const F32 c);

   void zero();                         ///< Zero all values

   operator F32*() { return &x; }
   operator const F32*() const { return &x; }

   //-------------------------------------- Queries
  public:
   bool  isZero() const;        ///< Check for zero coordinates. (No epsilon.)
   F32 len()    const;          ///< Get length.
   F32 lenSquared() const;      ///< Get squared length (one sqrt less than len()).
   bool equal( const Point2F &compare ) const;  ///< Is compare within POINT_EPSILON of all of the component of this point
   F32 magnitudeSafe() const;

   //-------------------------------------- Mathematical mutators
  public:
   void neg();                              ///< Invert signs of co-ordinates.
   void normalize();                        ///< Normalize vector.
   void normalize(F32 val);                 ///< Normalize, scaling by val.
   void normalizeSafe();
   void convolve(const Point2F&);           ///< Convolve by parameter.
   void convolveInverse(const Point2F&);    ///< Inversely convolute by parameter. (ie, divide)
   void rotate( F32 radians );              ///< Rotate vector around origin.

   /// Return a perpendicular vector to this one.  The result is equivalent to rotating the
   /// vector 90 degrees clockwise around the origin.
   Point2F getPerpendicular() const { return Point2F( y, - x ); }

   //-------------------------------------- Overloaded operators
  public:
   // Comparison operators
   bool operator==(const Point2F&) const;
   bool operator!=(const Point2F&) const;

   // Arithmetic w/ other points
   Point2F  operator+(const Point2F&) const;
   Point2F  operator-(const Point2F&) const;
   Point2F& operator+=(const Point2F&);
   Point2F& operator-=(const Point2F&);

   // Arithmetic w/ scalars
   Point2F  operator*(F32) const;
   Point2F  operator/(F32) const;
   Point2F& operator*=(F32);
   Point2F& operator/=(F32);

   Point2F  operator*(const Point2F&) const;
   Point2F& operator*=(const Point2F&);
   Point2F  operator/(const Point2F&) const;
   Point2F& operator/=(const Point2F&);

   // Unary operators
   Point2F operator-() const;

	//-------------------------------------- Public static constants
  public:
	const static Point2F One;
	const static Point2F Zero;
	const static Point2F Min;
	const static Point2F Max;
};


//------------------------------------------------------------------------------
/// 2D high-precision point.
///
/// Uses F64 internally.
class Point2D
{
   //-------------------------------------- Public data
  public:
   F64 x;   ///< X co-ordinate.
   F64 y;   ///< Y co-ordinate.

  public:
   Point2D();                           ///< Create uninitialized point.
   Point2D(const Point2D&);             ///< Copy constructor
   Point2D(F64 _x, F64 _y);             ///< Create point from coordinates.

   //-------------------------------------- Non-math mutators and misc functions
  public:
   void set(F64 _x, F64 _y);            ///< Set point's coordinates.

   void setMin(const Point2D&);         ///< Store lesser co-ordinates.
   void setMax(const Point2D&);         ///< Store greater co-ordinates.

   /// Interpolate from a to b, based on c.
   ///
   /// @param   a   Starting point.
   /// @param   b   Ending point.
   /// @param   c   Interpolation factor (0.0 .. 1.0).
   void interpolate(const Point2D &a, const Point2D &b, const F64 c);

   void zero();                         ///< Zero all values

   operator F64*() { return &x; }
   operator const F64*() const { return &x; }

   //-------------------------------------- Queries
  public:
   bool  isZero() const;
   F64 len()    const;
   F64 lenSquared() const;

   //-------------------------------------- Mathematical mutators
  public:
   void neg();
   void normalize();
   void normalize(F64 val);
   void convolve(const Point2D&);
   void convolveInverse(const Point2D&);

   //-------------------------------------- Overloaded operators
  public:
   // Comparison operators
   bool operator==(const Point2D&) const;
   bool operator!=(const Point2D&) const;

   // Arithmetic w/ other points
   Point2D  operator+(const Point2D&) const;
   Point2D  operator-(const Point2D&) const;
   Point2D& operator+=(const Point2D&);
   Point2D& operator-=(const Point2D&);

   // Arithmetic w/ scalars
   Point2D  operator*(F64) const;
   Point2D  operator/(F64) const;
   Point2D& operator*=(F64);
   Point2D& operator/=(F64);

   // Unary operators
   Point2D operator-() const;

	//-------------------------------------- Public static constants
  public:
	const static Point2D One;
	const static Point2D Zero;
};

//------------------------------------------------------------------------------
//-------------------------------------- Point2I
//
inline Point2I::Point2I()
{
   //
}


inline Point2I::Point2I(const Point2I& _copy)
 : x(_copy.x), y(_copy.y)
{
   //
}


inline Point2I::Point2I(S32 _x, S32 _y)
 : x(_x), y(_y)
{
   //
}


inline void Point2I::set(S32 _x, S32 _y)
{
   x = _x;
   y = _y;
}


inline void Point2I::setMin(const Point2I& _test)
{
   x = (_test.x < x) ? _test.x : x;
   y = (_test.y < y) ? _test.y : y;
}


inline void Point2I::setMax(const Point2I& _test)
{
   x = (_test.x > x) ? _test.x : x;
   y = (_test.y > y) ? _test.y : y;
}


inline void Point2I::zero()
{
   x = y = 0;
}


inline void Point2I::neg()
{
   x = -x;
   y = -y;
}

inline void Point2I::convolve(const Point2I& c)
{
   x *= c.x;
   y *= c.y;
}

inline bool Point2I::isZero() const
{
   return ((x == 0) && (y == 0));
}


inline F32 Point2I::len() const
{
   return mSqrt(F32(x*x + y*y));
}

inline S32 Point2I::lenSquared() const
{
   return x*x + y*y;
}

inline bool Point2I::operator==(const Point2I& _test) const
{
   return ((x == _test.x) && (y == _test.y));
}


inline bool Point2I::operator!=(const Point2I& _test) const
{
   return (operator==(_test) == false);
}


inline Point2I Point2I::operator+(const Point2I& _add) const
{
   return Point2I(x + _add.x, y + _add.y);
}


inline Point2I Point2I::operator-(const Point2I& _rSub) const
{
   return Point2I(x - _rSub.x, y - _rSub.y);
}


inline Point2I& Point2I::operator+=(const Point2I& _add)
{
   x += _add.x;
   y += _add.y;

   return *this;
}


inline Point2I& Point2I::operator-=(const Point2I& _rSub)
{
   x -= _rSub.x;
   y -= _rSub.y;

   return *this;
}


inline Point2I Point2I::operator-() const
{
   return Point2I(-x, -y);
}


inline Point2I Point2I::operator*(S32 mul) const
{
   return Point2I(x * mul, y * mul);
}

inline Point2I Point2I::operator/(S32 div) const
{
   AssertFatal(div != 0, "Error, div by zero attempted");
   return Point2I(x/div, y/div);
}


inline Point2I& Point2I::operator*=(S32 mul)
{
   x *= mul;
   y *= mul;

   return *this;
}


inline Point2I& Point2I::operator/=(S32 div)
{
   AssertFatal(div != 0, "Error, div by zero attempted");

   x /= div;
   y /= div;

   return *this;
}

inline Point2I Point2I::operator*(const Point2I &_vec) const
{
   return Point2I(x * _vec.x, y * _vec.y);
}

inline Point2I& Point2I::operator*=(const Point2I &_vec)
{
   x *= _vec.x;
   y *= _vec.y;
   return *this;
}

inline Point2I Point2I::operator/(const Point2I &_vec) const
{
   return Point2I(x / _vec.x, y / _vec.y);
}

inline Point2I& Point2I::operator/=(const Point2I &_vec)
{
   x /= _vec.x;
   y /= _vec.y;
   return *this;
}

//------------------------------------------------------------------------------
//-------------------------------------- Point2F
//
inline Point2F::Point2F()
{
   //
}


inline Point2F::Point2F(const Point2F& _copy)
 : x(_copy.x), y(_copy.y)
{
   //
}


inline Point2F::Point2F(F32 _x, F32 _y)
 : x(_x), y(_y)
{
}


inline void Point2F::set(F32 _x, F32 _y)
{
   x = _x;
   y = _y;
}


inline void Point2F::setMin(const Point2F& _test)
{
   x = (_test.x < x) ? _test.x : x;
   y = (_test.y < y) ? _test.y : y;
}


inline void Point2F::setMax(const Point2F& _test)
{
   x = (_test.x > x) ? _test.x : x;
   y = (_test.y > y) ? _test.y : y;
}


inline void Point2F::interpolate(const Point2F& _rFrom, const Point2F& _to, const F32 _factor)
{
   AssertFatal(_factor >= 0.0f && _factor <= 1.0f, "Out of bound interpolation factor");
   x = (_rFrom.x * (1.0f - _factor)) + (_to.x * _factor);
   y = (_rFrom.y * (1.0f - _factor)) + (_to.y * _factor);
}


inline void Point2F::zero()
{
   x = y = 0.0f;
}


inline bool Point2F::isZero() const
{
   return (x == 0.0f) && (y == 0.0f);
}


inline F32 Point2F::lenSquared() const
{
   return (x * x) + (y * y);
}


inline bool Point2F::equal( const Point2F &compare ) const
{
   return( ( mFabs( x - compare.x ) < POINT_EPSILON ) &&
           ( mFabs( y - compare.y ) < POINT_EPSILON ) );
}


inline void Point2F::neg()
{
   x = -x;
   y = -y;
}

inline void Point2F::convolve(const Point2F& c)
{
   x *= c.x;
   y *= c.y;
}


inline void Point2F::convolveInverse(const Point2F& c)
{
   x /= c.x;
   y /= c.y;
}

inline void Point2F::rotate( F32 radians )
{
   F32 sinTheta, cosTheta;
   mSinCos( radians, sinTheta, cosTheta );

   set(  cosTheta * x - sinTheta * y,
         sinTheta * x + cosTheta * y );
}

inline bool Point2F::operator==(const Point2F& _test) const
{
   return (x == _test.x) && (y == _test.y);
}


inline bool Point2F::operator!=(const Point2F& _test) const
{
   return operator==(_test) == false;
}


inline Point2F Point2F::operator+(const Point2F& _add) const
{
   return Point2F(x + _add.x, y + _add.y);
}


inline Point2F Point2F::operator-(const Point2F& _rSub) const
{
   return Point2F(x - _rSub.x, y - _rSub.y);
}


inline Point2F& Point2F::operator+=(const Point2F& _add)
{
   x += _add.x;
   y += _add.y;

   return *this;
}


inline Point2F& Point2F::operator-=(const Point2F& _rSub)
{
   x -= _rSub.x;
   y -= _rSub.y;

   return *this;
}


inline Point2F Point2F::operator*(F32 _mul) const
{
   return Point2F(x * _mul, y * _mul);
}


inline Point2F Point2F::operator/(F32 _div) const
{
   AssertFatal(_div != 0.0f, "Error, div by zero attempted");

   F32 inv = 1.0f / _div;

   return Point2F(x * inv, y * inv);
}


inline Point2F& Point2F::operator*=(F32 _mul)
{
   x *= _mul;
   y *= _mul;

   return *this;
}


inline Point2F& Point2F::operator/=(F32 _div)
{
   AssertFatal(_div != 0.0f, "Error, div by zero attempted");

   F32 inv = 1.0f / _div;

   x *= inv;
   y *= inv;

   return *this;
}

inline Point2F Point2F::operator*(const Point2F &_vec) const
{
   return Point2F(x * _vec.x, y * _vec.y);
}

inline Point2F& Point2F::operator*=(const Point2F &_vec)
{
   x *= _vec.x;
   y *= _vec.y;
   return *this;
}

inline Point2F Point2F::operator/(const Point2F &_vec) const
{
   return Point2F(x / _vec.x, y / _vec.y);
}

inline Point2F& Point2F::operator/=(const Point2F &_vec)
{
   x /= _vec.x;
   y /= _vec.y;
   return *this;
}

inline Point2F Point2F::operator-() const
{
   return Point2F(-x, -y);
}

inline F32 Point2F::len() const
{
   return mSqrt(x*x + y*y);
}

inline void Point2F::normalize()
{
   m_point2F_normalize(*this);
}

inline void Point2F::normalize(F32 val)
{
   m_point2F_normalize_f(*this, val);
}

inline F32 Point2F::magnitudeSafe() const
{
   if( isZero() )
      return 0.0f;
   else
      return len();
}

inline void Point2F::normalizeSafe()
{
   F32 vmag = magnitudeSafe();

   if( vmag > POINT_EPSILON )
      *this *= F32(1.0 / vmag);
}

//------------------------------------------------------------------------------
//-------------------------------------- Point2D
//
inline Point2D::Point2D()
{
   //
}


inline Point2D::Point2D(const Point2D& _copy)
 : x(_copy.x), y(_copy.y)
{
   //
}


inline Point2D::Point2D(F64 _x, F64 _y)
 : x(_x), y(_y)
{
}


inline void Point2D::set(F64 _x, F64 _y)
{
   x = _x;
   y = _y;
}


inline void Point2D::setMin(const Point2D& _test)
{
   x = (_test.x < x) ? _test.x : x;
   y = (_test.y < y) ? _test.y : y;
}


inline void Point2D::setMax(const Point2D& _test)
{
   x = (_test.x > x) ? _test.x : x;
   y = (_test.y > y) ? _test.y : y;
}


inline void Point2D::interpolate(const Point2D& _rFrom, const Point2D& _to, const F64 _factor)
{
   AssertFatal(_factor >= 0.0f && _factor <= 1.0f, "Out of bound interpolation factor");
   x = (_rFrom.x * (1.0f - _factor)) + (_to.x * _factor);
   y = (_rFrom.y * (1.0f - _factor)) + (_to.y * _factor);
}


inline void Point2D::zero()
{
   x = y = 0.0;
}


inline bool Point2D::isZero() const
{
   return (x == 0.0f) && (y == 0.0f);
}


inline F64 Point2D::lenSquared() const
{
   return (x * x) + (y * y);
}


inline void Point2D::neg()
{
   x = -x;
   y = -y;
}

inline void Point2D::convolve(const Point2D& c)
{
   x *= c.x;
   y *= c.y;
}

inline void Point2D::convolveInverse(const Point2D& c)
{
   x /= c.x;
   y /= c.y;
}

inline bool Point2D::operator==(const Point2D& _test) const
{
   return (x == _test.x) && (y == _test.y);
}


inline bool Point2D::operator!=(const Point2D& _test) const
{
   return operator==(_test) == false;
}


inline Point2D Point2D::operator+(const Point2D& _add) const
{
   return Point2D(x + _add.x, y + _add.y);
}


inline Point2D Point2D::operator-(const Point2D& _rSub) const
{
   return Point2D(x - _rSub.x, y - _rSub.y);
}


inline Point2D& Point2D::operator+=(const Point2D& _add)
{
   x += _add.x;
   y += _add.y;

   return *this;
}


inline Point2D& Point2D::operator-=(const Point2D& _rSub)
{
   x -= _rSub.x;
   y -= _rSub.y;

   return *this;
}


inline Point2D Point2D::operator*(F64 _mul) const
{
   return Point2D(x * _mul, y * _mul);
}


inline Point2D Point2D::operator/(F64 _div) const
{
   AssertFatal(_div != 0.0f, "Error, div by zero attempted");

   F64 inv = 1.0f / _div;

   return Point2D(x * inv, y * inv);
}


inline Point2D& Point2D::operator*=(F64 _mul)
{
   x *= _mul;
   y *= _mul;

   return *this;
}


inline Point2D& Point2D::operator/=(F64 _div)
{
   AssertFatal(_div != 0.0f, "Error, div by zero attempted");

   F64 inv = 1.0f / _div;

   x *= inv;
   y *= inv;

   return *this;
}


inline Point2D Point2D::operator-() const
{
   return Point2D(-x, -y);
}

inline F64 Point2D::len() const
{
   return mSqrtD(x*x + y*y);
}

inline void Point2D::normalize()
{
   m_point2D_normalize(*this);
}

inline void Point2D::normalize(F64 val)
{
   m_point2D_normalize_f(*this, val);
}


//-------------------------------------------------------------------
// Non-Member Operators
//-------------------------------------------------------------------

inline Point2I operator*(S32 mul, const Point2I& multiplicand)
{
   return multiplicand * mul;
}

inline Point2F operator*(F32 mul, const Point2F& multiplicand)
{
   return multiplicand * mul;
}

inline Point2D operator*(F64 mul, const Point2D& multiplicand)
{
   return multiplicand * mul;
}

inline F32 mDot(const Point2F &p1, const Point2F &p2)
{
   return (p1.x*p2.x + p1.y*p2.y);
}

inline F32 mDotPerp(const Point2F &p1, const Point2F &p2)
{
   return p1.x*p2.y - p2.x*p1.y;
}

inline bool mIsNaN( const Point2F &p )
{
   return mIsNaN_F( p.x ) || mIsNaN_F( p.y );
}


namespace DictHash
{
   /// Generates a 32bit hash from a Point2I.
   /// @see DictHash
   inline U32 hash( const Point2I &key )
   {
      return (key.x * 2230148873u) ^ key.y;
   }
}

#endif // _MPOINT2_H_
