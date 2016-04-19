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

#ifndef MROTATION_H
#define MROTATION_H

#ifndef _MMATHFN_H_
#include "math/mMathFn.h"
#endif

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

#ifndef _MANGAXIS_H_
#include "math/mAngAxis.h"
#endif

//------------------------------------------------------------------------------
/// Rotation Interop Utility class
///
/// Useful for easily handling rotations/orientations in transforms while manipulating or converting between formats.
class RotationF
{
   //-------------------------------------- Public data
  public:
   F32 x;   ///< X co-ordinate.
   F32 y;   ///< Y co-ordinate.
   F32 z;   ///< Z co-ordinate.
   F32 w;   ///< W co-ordinate.

   enum RotationTypes
   {
      Euler = 0,
      AxisAngle
   };

   enum UnitFormat
   {
      Radians = 0,
      Degrees
   };
   
   RotationTypes mRotationType;

   UnitFormat mUnitsFormat;

  public:
   RotationF();               ///< Create an uninitialized point.
   RotationF(const RotationF&); ///< Copy constructor.

   /// Create point from coordinates.
   RotationF(F32 _x, F32 _y, F32 _z, F32 _w, UnitFormat format = Radians);

   /// Set point's coordinates.
   void set(F32 _x, F32 _y, F32 _z, F32 _w, UnitFormat format = Radians);

   //Euler setup
   RotationF(EulerF euler, UnitFormat format = Radians);
   RotationF(F32 _x, F32 _y, F32 _z, UnitFormat format = Radians);

   void set(F32 _x, F32 _y, F32 _z, UnitFormat format = Radians);

   //Matrix
   RotationF(MatrixF mat);
   void set(MatrixF _mat);

   RotationF& operator=(const MatrixF&);

   //Quat
   RotationF(QuatF quat);
   void set(QuatF _quat);

   RotationF& operator=(const QuatF&);

   /// Interpolate from _pt1 to _pt2, based on _factor.
   ///
   /// @param   _pt1    Starting point.
   /// @param   _pt2    Ending point.
   /// @param   _factor Interpolation factor (0.0 .. 1.0).
   void interpolate(const RotationF& _pt1, const RotationF& _pt2, F32 _factor);

   void zero();

   operator F32*() { return (&x); }
   operator const F32*() const { return &x; }
   
   F32 len() const;

   RotationF operator/(F32) const;

   RotationF operator*(F32) const;
   RotationF  operator+(const RotationF&) const;
   RotationF& operator+=(const RotationF&);
   RotationF  operator-(const RotationF&) const;      
   RotationF operator*(const RotationF&) const;
   RotationF& operator*=(const RotationF&);

   RotationF& operator=(const RotationF&);

   //Conversion stuffs
   RotationF& operator=(const AngAxisF&);

   RotationF& operator=(const Point3F&);

   EulerF asEulerF(UnitFormat format = Radians) const;

   AngAxisF asAxisAngle(UnitFormat format = Radians) const;
   
   MatrixF asMatrixF() const;

   QuatF asQuatF() const;

   //set functions
   void set(EulerF _eul, UnitFormat format = Radians);
   void set(AngAxisF _aa, UnitFormat format = Radians);

	//-------------------------------------- Public static constants
  public:
	const static RotationF One;
	const static RotationF Zero;
};

//------------------------------------------------------------------------------
//
inline RotationF::RotationF()
{
   mRotationType = AxisAngle;
}

inline RotationF::RotationF(const RotationF& _copy)
 : x(_copy.x), y(_copy.y), z(_copy.z), w(_copy.w), mRotationType(_copy.mRotationType)
{
}

inline RotationF::RotationF(F32 _x, F32 _y, F32 _z, F32 _w, UnitFormat format)
 : x(_x), y(_y), z(_z), w(_w)
{
   mRotationType = AxisAngle;
}

inline void RotationF::set(F32 _x, F32 _y, F32 _z, F32 _w, UnitFormat format)
{
   x = _x;
   y = _y;
   z = _z;

   if(format == Degrees)
      w = mDegToRad(_w);
   else
      w = _w;
}

//Euler setup
inline RotationF::RotationF(F32 _x, F32 _y, F32 _z, UnitFormat format)
 : x(_x), y(_y), z(_z)
{
   mRotationType = Euler;
}

inline void RotationF::set(F32 _x, F32 _y, F32 _z, UnitFormat format)
{
   if(format == Radians)
   {
      x = _x;
      y = _y;
      z = _z;
      w = 1;
   }
   else
   {
      x = mDegToRad(_x);
      y = mDegToRad(_y);
      z = mDegToRad(_z);
      w = 1;
   }

   mRotationType = Euler;
}

inline RotationF::RotationF(EulerF euler, UnitFormat format)
{
   if (format == Radians)
   {
      x = euler.x;
      y = euler.y;
      z = euler.z;
      w = 1;
   }
   else
   {
      x = mDegToRad(euler.x);
      y = mDegToRad(euler.y);
      z = mDegToRad(euler.z);
      w = 1;
   }

   mRotationType = Euler;
}

inline RotationF::RotationF(MatrixF mat)
{
   set(mat);
}

inline RotationF::RotationF(QuatF quat)
{
   set(quat);
}

inline F32 RotationF::len() const
{
   if(mRotationType == AxisAngle)
      return mSqrt(x*x + y*y + z*z + w*w);
   else
      return mSqrt(x*x + y*y + z*z);
}

inline void RotationF::interpolate(const RotationF& _from, const RotationF& _to, F32 _factor)
{
   x = (_from.x * (1.0f - _factor)) + (_to.x * _factor);
   y = (_from.y * (1.0f - _factor)) + (_to.y * _factor);
   z = (_from.z * (1.0f - _factor)) + (_to.z * _factor);

   if(mRotationType == AxisAngle)
      w = (_from.w * (1.0f - _factor)) + (_to.w * _factor);
}

inline void RotationF::zero()
{
   x = y = z = w = 0.0f;
}

inline RotationF& RotationF::operator=(const RotationF &_vec)
{
   mRotationType = _vec.mRotationType;

   mUnitsFormat = _vec.mUnitsFormat;

   x = _vec.x;
   y = _vec.y;
   z = _vec.z;

   if(mRotationType == AxisAngle)
      w = _vec.w;
   else
      w = 1;

   if(mRotationType == Euler)
   {
      x = mFmod( x, M_2PI_F );
      y = mFmod( y, M_2PI_F );
      z = mFmod( z, M_2PI_F );
   }
   else
   {
      w = mFmod( w, M_2PI_F );
   }

   return *this;
}

inline RotationF RotationF::operator+(const RotationF& _add) const
{
   //return RotationF( x + _add.x, y + _add.y, z + _add.z, w + _add.w );
   if(mRotationType == Euler)
   {
      return RotationF( x + _add.x, y + _add.y, z + _add.z, w + _add.w );
   }
   else
   {
      MatrixF tempMat = asMatrixF();
      MatrixF tempMatAdd = _add.asMatrixF();

      tempMat.mul(tempMatAdd);

      RotationF out;
      out.set(tempMat);
      return out;
   }
}

inline RotationF& RotationF::operator+=(const RotationF& _add)
{
   if(mRotationType == Euler)
   {
      if(_add.mRotationType == Euler)
      {
         x += _add.x;
         y += _add.y;
         z += _add.z;
         w = 1;
      }
      else
      {
         EulerF temp = _add.asEulerF();
         x += temp.x;
         y += temp.y;
         z += temp.z;
         w = 1;
      }

      return *this;
   }
   else
   {
      MatrixF tempMat = asMatrixF();
      MatrixF tempMatAdd = _add.asMatrixF();

      tempMat.mul(tempMatAdd);

      set(tempMat);
      return *this;
   }
}

inline RotationF RotationF::operator-(const RotationF& _rSub) const
{
   return RotationF( x - _rSub.x, y - _rSub.y, z - _rSub.z, w - _rSub.w );
}

inline RotationF RotationF::operator*(const RotationF &_vec) const
{
   return RotationF(x * _vec.x, y * _vec.y, z * _vec.z, w * _vec.w);
}

inline RotationF RotationF::operator*(F32 _mul) const
{
   return RotationF(x * _mul, y * _mul, z * _mul, w * _mul);
}

inline RotationF RotationF::operator /(F32 t) const
{
   F32 f = 1.0f / t;
   return RotationF( x * f, y * f, z * f, w * f );
}

inline RotationF& RotationF::operator=(const AngAxisF& _aa) 
{
   mRotationType = AxisAngle;
   mUnitsFormat = Radians;

   set(_aa);
   return *this;
}

inline RotationF& RotationF::operator=(const MatrixF& _mat)
{
   //probably change this later
   mRotationType = Euler;
   mUnitsFormat = Radians;

   set(_mat);
   return *this;
}

inline RotationF& RotationF::operator=(const QuatF& _qat)
{
   mRotationType = AxisAngle;
   mUnitsFormat = Radians;

   set(_qat);
   return *this;
}

inline RotationF& RotationF::operator=(const Point3F& _eul)
{
   mRotationType = Euler;
   mUnitsFormat = Radians;

   set(_eul);
   return *this;
}
//------------------------------------------------------------------------------
//-------------------------------------- RotationF
inline EulerF RotationF::asEulerF(UnitFormat format) const 
{ 
   if(mRotationType == Euler)
   {
      if(format == Radians)
         return Point3F(x,y,z); 
      else
         return Point3F(mRadToDeg(x), mRadToDeg(y), mRadToDeg(z));
   }
   else
   {
      AngAxisF tempAA = AngAxisF(Point3F(x, y, z), w);
      MatrixF temp;
      tempAA.setMatrix(&temp);

      if(format == Radians)
      {
         return temp.toEuler();
      }
      else
      {
         EulerF tempEul = temp.toEuler();
         return EulerF(mRadToDeg(tempEul.x), mRadToDeg(tempEul.y), mRadToDeg(tempEul.z));
      }
   }
}

inline AngAxisF RotationF::asAxisAngle(UnitFormat format) const 
{ 
   if(mRotationType == AxisAngle)
   {
      if(format == Radians)
         return AngAxisF(Point3F(x,y,z),w); 
      else
         return AngAxisF(Point3F(x,y,z),mRadToDeg(w));
   }
   else
   {
      AngAxisF tempAA;
      QuatF q;
      q.set(Point3F(x,y,z));

      tempAA.set(q);

      if(format == Radians)
      {
         return tempAA;
      }
      else
      {
         tempAA.angle = mRadToDeg(tempAA.angle);
         return tempAA;
      }
   }
}
   
inline MatrixF RotationF::asMatrixF() const 
{ 
   if(mRotationType == AxisAngle)
   {
      AngAxisF tempAA;
      tempAA.axis.x = x;
      tempAA.axis.y = y;
      tempAA.axis.z = z;
      tempAA.angle = w;

      MatrixF tempMat;
      tempAA.setMatrix(&tempMat);
      return tempMat;
   }
   else
   {
      MatrixF temp, imat, xmat, ymat, zmat;
      xmat.set(EulerF(x,0,0));
      ymat.set(EulerF(0.0f, y, 0.0f));
      zmat.set(EulerF(0,0,z));

      imat.mul(zmat, xmat);
      temp.mul(imat, ymat);

      return temp;
   }
}

inline QuatF RotationF::asQuatF() const 
{ 
   if(mRotationType == Euler)
   {
      QuatF q = QuatF(Point3F(x,y,z));
      return q;
   }
   else
   {
      QuatF q = QuatF(Point3F(x,y,z), w);
      return q;
   }
}

//set functions
inline void RotationF::set(EulerF _eul, UnitFormat format)
{
   if(format == Radians)
   {
      x = _eul.x;
      y = _eul.y;
      z = _eul.z;
   }
   else
   {
      x = mDegToRad(_eul.x);
      y = mDegToRad(_eul.y);
      z = mDegToRad(_eul.z);
   }

   //first, wrap to +/- PI
   x = mFmod( x, M_2PI_F );
   y = mFmod( y, M_2PI_F );
   z = mFmod( z, M_2PI_F );

   mRotationType = Euler;
}

inline void RotationF::set(AngAxisF _aa, UnitFormat format)
{
   if(format == Radians)
   {
      x = _aa.axis.x;
      y = _aa.axis.y;
      z = _aa.axis.z;
      w = _aa.angle;
   }
   else
   {
      x = _aa.axis.x;
      y = _aa.axis.y;
      z = _aa.axis.z;
      w = mDegToRad(_aa.angle);
   }

   w = mFmod( w, M_2PI_F );

   mRotationType = AxisAngle;
}

inline void RotationF::set(QuatF _quat)
{
   //Taking in a quat assumes radians (check on this?)
   AngAxisF tempAA;
   tempAA.set(_quat);

   x = tempAA.axis.x;
   y = tempAA.axis.y;
   z = tempAA.axis.z;
   w = tempAA.angle;

   w = mFmod( w, M_2PI_F );

   mRotationType = AxisAngle;
}

inline void RotationF::set(MatrixF _mat)
{
   //Taking in a matrix assumes radians (check on this?)
   EulerF temp;
   temp = _mat.toEuler();
   x = temp.x;
   y = temp.y;
   z = temp.z;
   w = 1;

   //first, wrap to +/- PI
   x = mFmod( x, M_2PI_F );
   y = mFmod( y, M_2PI_F );
   z = mFmod( z, M_2PI_F );

   mRotationType = Euler;
}
//-------------------------------------------------------------------
// Non-Member Operators
//-------------------------------------------------------------------

inline RotationF operator*(F32 mul, const RotationF& multiplicand)
{
   return multiplicand * mul;
}

inline bool mIsNaN( const RotationF &p )
{
   return mIsNaN_F( p.x ) || mIsNaN_F( p.y ) || mIsNaN_F( p.z ) || mIsNaN_F( p.w );
}

#endif // MROTATION_H
