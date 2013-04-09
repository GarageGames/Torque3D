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

#ifndef _MPOINT4_H_
#define _MPOINT4_H_

#ifndef _MMATHFN_H_
#include "math/mMathFn.h"
#endif

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif


//------------------------------------------------------------------------------
/// 4D integer point
///
/// Uses S32 internally. Currently storage only.
class Point4I
{
  public:
   Point4I() {}
   Point4I(S32 _x, S32 _y, S32 _z, S32 _w);

   void zero();   ///< Zero all values

   S32 x;                                                   
   S32 y;                                                   
   S32 z;                                                   
   S32 w;       

	//-------------------------------------- Public static constants
  public:
	const static Point4I One;
	const static Point4I Zero;
};

//------------------------------------------------------------------------------
/// 4D floating-point point.
///
/// Uses F32 internally.
///
/// Useful for representing quaternions and other 4d beasties.
class Point4F
{
   //-------------------------------------- Public data
  public:
   F32 x;   ///< X co-ordinate.
   F32 y;   ///< Y co-ordinate.
   F32 z;   ///< Z co-ordinate.
   F32 w;   ///< W co-ordinate.

  public:
   Point4F();               ///< Create an uninitialized point.
   Point4F(const Point4F&); ///< Copy constructor.

   /// Create point from coordinates.
   Point4F(F32 _x, F32 _y, F32 _z, F32 _w);

   /// Set point's coordinates.
   void set(F32 _x, F32 _y, F32 _z, F32 _w);

   /// Interpolate from _pt1 to _pt2, based on _factor.
   ///
   /// @param   _pt1    Starting point.
   /// @param   _pt2    Ending point.
   /// @param   _factor Interpolation factor (0.0 .. 1.0).
   void interpolate(const Point4F& _pt1, const Point4F& _pt2, F32 _factor);

   void zero();

   operator F32*() { return (&x); }
   operator const F32*() const { return &x; }
   
   F32 len() const;

   Point4F operator/(F32) const;

   Point4F operator*(F32) const;
   Point4F  operator+(const Point4F&) const;
   Point4F& operator+=(const Point4F&);
   Point4F  operator-(const Point4F&) const;      
   Point4F operator*(const Point4F&) const;
   Point4F& operator*=(const Point4F&);
   Point4F& operator=(const Point3F&);
   Point4F& operator=(const Point4F&);
   
   Point3F asPoint3F() const { return Point3F(x,y,z); }

	//-------------------------------------- Public static constants
  public:
	const static Point4F One;
	const static Point4F Zero;
};

typedef Point4F Vector4F;   ///< Points can be vectors!

//------------------------------------------------------------------------------
//-------------------------------------- Point4I

inline void Point4I::zero()
{
   x = y = z = w = 0;
}

//------------------------------------------------------------------------------
//-------------------------------------- Point4F
//
inline Point4F::Point4F()
{
}

inline Point4F::Point4F(const Point4F& _copy)
 : x(_copy.x), y(_copy.y), z(_copy.z), w(_copy.w)
{
}

inline Point4F::Point4F(F32 _x, F32 _y, F32 _z, F32 _w)
 : x(_x), y(_y), z(_z), w(_w)
{
}

inline void Point4F::set(F32 _x, F32 _y, F32 _z, F32 _w)
{
   x = _x;
   y = _y;
   z = _z;
   w = _w;
}

inline F32 Point4F::len() const
{
   return mSqrt(x*x + y*y + z*z + w*w);
}

inline void Point4F::interpolate(const Point4F& _from, const Point4F& _to, F32 _factor)
{
   x = (_from.x * (1.0f - _factor)) + (_to.x * _factor);
   y = (_from.y * (1.0f - _factor)) + (_to.y * _factor);
   z = (_from.z * (1.0f - _factor)) + (_to.z * _factor);
   w = (_from.w * (1.0f - _factor)) + (_to.w * _factor);
}

inline void Point4F::zero()
{
   x = y = z = w = 0.0f;
}

inline Point4F& Point4F::operator=(const Point3F &_vec)
{
   x = _vec.x;
   y = _vec.y;
   z = _vec.z;
   w = 1.0f;
   return *this;
}

inline Point4F& Point4F::operator=(const Point4F &_vec)
{
   x = _vec.x;
   y = _vec.y;
   z = _vec.z;
   w = _vec.w;

   return *this;
}

inline Point4F Point4F::operator+(const Point4F& _add) const
{
   return Point4F( x + _add.x, y + _add.y, z + _add.z, w + _add.w );
}

inline Point4F& Point4F::operator+=(const Point4F& _add)
{
   x += _add.x;
   y += _add.y;
   z += _add.z;
   w += _add.w;

   return *this;
}

inline Point4F Point4F::operator-(const Point4F& _rSub) const
{
   return Point4F( x - _rSub.x, y - _rSub.y, z - _rSub.z, w - _rSub.w );
}

inline Point4F Point4F::operator*(const Point4F &_vec) const
{
   return Point4F(x * _vec.x, y * _vec.y, z * _vec.z, w * _vec.w);
}

inline Point4F Point4F::operator*(F32 _mul) const
{
   return Point4F(x * _mul, y * _mul, z * _mul, w * _mul);
}

inline Point4F Point4F::operator /(F32 t) const
{
   F32 f = 1.0f / t;
   return Point4F( x * f, y * f, z * f, w * f );
}

//------------------------------------------------------------------------------
//-------------------------------------- Point4F

inline Point4I::Point4I(S32 _x, S32 _y, S32 _z, S32 _w) : x(_x), y(_y), z(_z), w(_w) 
{
}

//-------------------------------------------------------------------
// Non-Member Operators
//-------------------------------------------------------------------

inline Point4F operator*(F32 mul, const Point4F& multiplicand)
{
   return multiplicand * mul;
}

inline bool mIsNaN( const Point4F &p )
{
   return mIsNaN_F( p.x ) || mIsNaN_F( p.y ) || mIsNaN_F( p.z ) || mIsNaN_F( p.w );
}

#endif // _MPOINT4_H_
