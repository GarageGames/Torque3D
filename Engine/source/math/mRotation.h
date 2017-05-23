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

#ifndef _MQUAT_H_
#include "math/mQuat.h"
#endif

#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
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
   RotationTypes mRotationType;

   enum UnitFormat
   {
      Radians = 0,
      Degrees
   };

   RotationF();               ///< Create an uninitialized point.
   RotationF(const RotationF&); ///< Copy constructor.

   //
   //Eulers
   RotationF(EulerF euler, UnitFormat format = Radians);
   RotationF(F32 _x, F32 _y, F32 _z, UnitFormat format = Radians);

   void set(EulerF euler, UnitFormat format = Radians);
   void set(F32 _x, F32 _y, F32 _z, UnitFormat format = Radians);

   //As with AxisAngles, we make the assumption here that if not told otherwise, inbound rotations are in Degrees.
   RotationF operator=(const EulerF&);
   RotationF operator-(const EulerF&) const;
   RotationF operator+(const EulerF&) const;
   RotationF& operator-=(const EulerF&);
   RotationF& operator+=(const EulerF&);
   S32 operator==(const EulerF&) const;
   S32 operator!=(const EulerF&) const;

   //
   //AxisAngle
   RotationF(AngAxisF aa, UnitFormat format = Radians);
   void set(AngAxisF aa, UnitFormat format = Radians);

   //As with Eulers, we make the assumption here that if not told otherwise, inbound rotations are in Degrees.
   RotationF operator=(const AngAxisF&);
   RotationF operator-(const AngAxisF&) const;
   RotationF operator+(const AngAxisF&) const;
   RotationF& operator-=(const AngAxisF&);
   RotationF& operator+=(const AngAxisF&);
   S32 operator==(const AngAxisF&) const;
   S32 operator!=(const AngAxisF&) const;

   //
   //Quat
   RotationF(QuatF quat);
   void set(QuatF _quat);

   RotationF operator=(const QuatF&);
   RotationF operator-(const QuatF&) const;
   RotationF operator+(const QuatF&) const;
   RotationF& operator-=(const QuatF&);
   RotationF& operator+=(const QuatF&);
   S32 operator==(const QuatF&) const;
   S32 operator!=(const QuatF&) const;

   //
   //Matrix
   RotationF(MatrixF mat);
   void set(MatrixF _mat);

   RotationF operator=(const MatrixF&);
   RotationF operator-(const MatrixF&) const;
   RotationF operator+(const MatrixF&) const;
   RotationF& operator-=(const MatrixF&);
   RotationF& operator+=(const MatrixF&);
   S32 operator==(const MatrixF&) const;
   S32 operator!=(const MatrixF&) const;

   //
   void interpolate(const RotationF& _pt1, const RotationF& _pt2, F32 _factor);
   void lookAt(const Point3F& _origin, const Point3F& _target, const Point3F& _up = Point3F(0, 0, 1));
   VectorF getDirection();

   F32 len() const;

   void normalize();

   //Non-converting operators
   S32 operator ==(const RotationF &) const;
   S32 operator !=(const RotationF &) const;

   RotationF  operator+(const RotationF&) const;
   RotationF& operator+=(const RotationF&);
   RotationF  operator-(const RotationF&) const;
   RotationF&  operator-=(const RotationF&);

   RotationF& operator=(const RotationF&);

   //Conversion stuffs
   EulerF asEulerF(UnitFormat format = Radians) const;
   AngAxisF asAxisAngle(UnitFormat format = Radians) const;
   MatrixF asMatrixF() const;
   QuatF asQuatF() const;
};

inline RotationF::RotationF()
{
   x = 0;
   y = 0;
   z = 0;
   w = 0;

   mRotationType = AxisAngle;
}

inline RotationF::RotationF(const RotationF& _copy)
   : x(_copy.x), y(_copy.y), z(_copy.z), w(_copy.w), mRotationType(_copy.mRotationType)
{}

inline int RotationF::operator ==(const RotationF& _rotation) const
{
   return (x == _rotation.x && y == _rotation.y && z == _rotation.z && w == _rotation.w);
}

inline int RotationF::operator !=(const RotationF& _rotation) const
{
   return (x != _rotation.x || y != _rotation.y || z != _rotation.z || w != _rotation.w);
}

//When it comes to actually trying to add rotations, we, in fact, actually multiply their data together.
//Since we're specifically operating on usability for RotationF, we'll operate on this, rather than the literal addition of the values
inline RotationF& RotationF::operator +=(const RotationF& _rotation)
{
   if (mRotationType == Euler)
   {
      x += _rotation.x;
      y += _rotation.y;
      z += _rotation.z;
   }
   else
   {
      MatrixF tempMat = asMatrixF();
      MatrixF tempMatAdd = _rotation.asMatrixF();

      tempMat.mul(tempMatAdd);

      this->set(tempMat);
   }

   return *this;
}

inline RotationF RotationF::operator +(const RotationF& _rotation) const
{
   RotationF result = *this;

   if (mRotationType == Euler)
   {
      result.x += _rotation.x;
      result.y += _rotation.y;
      result.z += _rotation.z;
   }
   else
   {
      MatrixF tempMat = asMatrixF();
      MatrixF tempMatAdd = _rotation.asMatrixF();

      tempMat.mul(tempMatAdd);

      result.set(tempMat);
   }

   return result;
}

//Much like addition, when subtracting, we're not literally subtracting the values, but infact multiplying the inverse.
//This subtracts the rotation angles to get the difference
inline RotationF& RotationF::operator -=(const RotationF& _rotation)
{
   if (mRotationType == Euler)
   {
      x -= _rotation.x;
      y -= _rotation.y;
      z -= _rotation.z;
   }
   else
   {
      MatrixF tempMat = asMatrixF();
      MatrixF tempMatAdd = _rotation.asMatrixF();

      tempMatAdd.inverse();

      tempMat.mul(tempMatAdd);

      this->set(tempMat);
   }

   return *this;
}

inline RotationF RotationF::operator -(const RotationF& _rotation) const
{
   RotationF result = *this;

   if (mRotationType == Euler)
   {
      result.x += _rotation.x;
      result.y += _rotation.y;
      result.z += _rotation.z;
   }
   else
   {
      MatrixF tempMat = asMatrixF();
      MatrixF tempMatAdd = _rotation.asMatrixF();
      tempMatAdd.inverse();

      tempMat.mul(tempMatAdd);

      result.set(tempMat);
   }

   return result;
}

inline RotationF& RotationF::operator =(const RotationF& _rotation)
{
   x = _rotation.x;
   y = _rotation.y;
   z = _rotation.z;
   w = _rotation.w;

   mRotationType = _rotation.mRotationType;

   return *this;
}

//====================================================================
// Euler operators
//====================================================================
inline RotationF RotationF::operator=(const EulerF& _euler)
{
   return RotationF(_euler, Radians);
}

inline RotationF RotationF::operator-(const EulerF& _euler) const
{
   RotationF temp = *this;
   temp -= RotationF(_euler, Radians);
   return temp;
}

inline RotationF RotationF::operator+(const EulerF& _euler) const
{
   RotationF temp = *this;
   temp += RotationF(_euler, Radians);
   return temp;
}

inline RotationF& RotationF::operator-=(const EulerF& _euler)
{
   *this -= RotationF(_euler, Radians);
   return *this;
}

inline RotationF& RotationF::operator+=(const EulerF& _euler)
{
   *this += RotationF(_euler, Radians);
   return *this;
}

inline S32 RotationF::operator==(const EulerF& _euler) const
{
   return *this == RotationF(_euler);
}

inline S32 RotationF::operator!=(const EulerF& _euler) const
{
   return *this != RotationF(_euler);
}

//====================================================================
// AxisAngle operators
//====================================================================
inline RotationF RotationF::operator=(const AngAxisF& _aa)
{
   return RotationF(_aa, Radians);
}

inline RotationF RotationF::operator-(const AngAxisF& _aa) const
{
   RotationF temp = *this;
   temp -= RotationF(_aa, Radians);
   return temp;
}

inline RotationF RotationF::operator+(const AngAxisF& _aa) const
{
   RotationF temp = *this;
   temp += RotationF(_aa, Radians);
   return temp;
}

inline RotationF& RotationF::operator-=(const AngAxisF& _aa)
{
   *this -= RotationF(_aa, Radians);
   return *this;
}

inline RotationF& RotationF::operator+=(const AngAxisF& _aa)
{
   *this += RotationF(_aa, Radians);
   return *this;
}

inline S32 RotationF::operator==(const AngAxisF& _aa) const
{
   return *this == RotationF(_aa);
}

inline S32 RotationF::operator!=(const AngAxisF& _aa) const
{
   return *this != RotationF(_aa);
}

//====================================================================
// QuatF operators
//====================================================================
inline RotationF RotationF::operator=(const QuatF& _quat)
{
   return RotationF(_quat);
}

inline RotationF RotationF::operator-(const QuatF& _quat) const
{
   RotationF temp = *this;
   temp -= RotationF(_quat);
   return temp;
}

inline RotationF RotationF::operator+(const QuatF& _quat) const
{
   RotationF temp = *this;
   temp += RotationF(_quat);
   return temp;
}

inline RotationF& RotationF::operator-=(const QuatF& _quat)
{
   *this -= RotationF(_quat);
   return *this;
}

inline RotationF& RotationF::operator+=(const QuatF& _quat)
{
   *this += RotationF(_quat);
   return *this;
}

inline S32 RotationF::operator==(const QuatF& _quat) const
{
   return *this == RotationF(_quat);
}

inline S32 RotationF::operator!=(const QuatF& _quat) const
{
   return *this != RotationF(_quat);
}

//====================================================================
// MatrixF operators
//====================================================================
inline RotationF RotationF::operator=(const MatrixF& _mat)
{
   return RotationF(_mat);
}

inline RotationF RotationF::operator-(const MatrixF& _mat) const
{
   RotationF temp = *this;
   temp -= RotationF(_mat);
   return temp;
}

inline RotationF RotationF::operator+(const MatrixF& _mat) const
{
   RotationF temp = *this;
   temp += RotationF(_mat);
   return temp;
}

inline RotationF& RotationF::operator-=(const MatrixF& _mat)
{
   *this -= RotationF(_mat);
   return *this;
}

inline RotationF& RotationF::operator+=(const MatrixF& _mat)
{
   *this += RotationF(_mat);
   return *this;
}

inline S32 RotationF::operator==(const MatrixF& _mat) const
{
   return *this == RotationF(_mat);
}

inline S32 RotationF::operator!=(const MatrixF& _mat) const
{
   return *this != RotationF(_mat);
}

#endif // MROTATION_H
