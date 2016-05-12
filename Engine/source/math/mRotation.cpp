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
#include "math/mRotation.h"

//====================================================================
//Eulers setup
//====================================================================
RotationF::RotationF(EulerF _euler, UnitFormat format)
{
   set(_euler.x, _euler.y, _euler.z, format);
}

RotationF::RotationF(F32 _x, F32 _y, F32 _z, UnitFormat format)
{
   set(_x, _y, _z, format);
}

void RotationF::set(EulerF _euler, UnitFormat format)
{
   set(_euler.x, _euler.y, _euler.z, format);
}

void RotationF::set(F32 _x, F32 _y, F32 _z, UnitFormat format)
{
   EulerF tempEul;
   if (format == Radians)
   {
      tempEul.set(mRadToDeg(_x), mRadToDeg(_y), mRadToDeg(_z));
   }
   else
   {
      tempEul.set(_x, _y, _z);
   }

   mRotation.set(tempEul);
}

//====================================================================
//AxisAngle setup
//====================================================================
RotationF::RotationF(AngAxisF _aa, UnitFormat format)
{
   set(_aa, format);
}

void RotationF::set(AngAxisF _aa, UnitFormat format)
{
   if (format == Radians)
   {
      _aa.angle = mRadToDeg(_aa.angle);
   }

   mRotation.set(_aa);
}

//====================================================================
//QuatF setup
//====================================================================
RotationF::RotationF(QuatF _quat)
{
   set(_quat);
}

void RotationF::set(QuatF _quat)
{
   mRotation = _quat;
}

//====================================================================
//MatrixF setup
//====================================================================
RotationF::RotationF(MatrixF _mat)
{
   set(_mat);
}

void RotationF::set(MatrixF _mat)
{
   mRotation.set(_mat);
}

//
inline F32 RotationF::len() const
{
   return asEulerF().len();
}

inline void RotationF::interpolate(const RotationF& _from, const RotationF& _to, F32 _factor)
{
   mRotation.interpolate(_from.asQuatF(), _to.asQuatF(), _factor);
}

//========================================================
EulerF RotationF::asEulerF(UnitFormat format) const
{
   EulerF returnEuler = asMatrixF().toEuler();

   if (format == Radians)
   {
      returnEuler.x = mDegToRad(returnEuler.x);
      returnEuler.y = mDegToRad(returnEuler.y);
      returnEuler.z = mDegToRad(returnEuler.z);
   }

   return returnEuler;
}

AngAxisF RotationF::asAxisAngle(UnitFormat format) const
{
   AngAxisF returnAA;
   returnAA.set(mRotation);

   if (format == Radians)
   {
      returnAA.angle = mDegToRad(returnAA.angle);
   }

   return returnAA;
}

MatrixF RotationF::asMatrixF() const
{
   MatrixF returnMat;
   mRotation.setMatrix(&returnMat);

   return returnMat;
}

QuatF RotationF::asQuatF() const
{
   return mRotation;
}

void RotationF::normalize()
{
   mRotation.normalize();
}