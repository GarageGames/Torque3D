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
#include "console/console.h"
#include "console/engineAPI.h"
#include "math/mathUtils.h"

#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#endif

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
   x = format == Degrees ? mDegToRad(_euler.x) : _euler.x;
   y = format == Degrees ? mDegToRad(_euler.y) : _euler.y;
   z = format == Degrees ? mDegToRad(_euler.z) : _euler.z;

   mRotationType = Euler;
}

void RotationF::set(F32 _x, F32 _y, F32 _z, UnitFormat format)
{
   EulerF tempEul;
   if (format == Degrees)
   {
      tempEul.set(mDegToRad(_x), mDegToRad(_y), mDegToRad(_z));
   }
   else
   {
      tempEul.set(_x, _y, _z);
   }

   set(tempEul);
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
   x = _aa.axis.x;
   y = _aa.axis.y;
   z = _aa.axis.z;

   w = format == Degrees ? mDegToRad(_aa.angle) : _aa.angle;

   mRotationType = AxisAngle;
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
   AngAxisF tmpAA;
   tmpAA.set(_quat);

   set(tmpAA);
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
   set(_mat.toEuler());
}

//
inline F32 RotationF::len() const
{
   return asEulerF().len();
}

inline void RotationF::interpolate(const RotationF& _from, const RotationF& _to, F32 _factor)
{
   QuatF tmpQuat;

   tmpQuat.interpolate(_from.asQuatF(), _to.asQuatF(), _factor);

   set(tmpQuat);
}

void RotationF::lookAt(const Point3F& _origin, const Point3F& _target, const Point3F& _up)
{
   MatrixF mat;

   VectorF newForward = _target - _origin;
   newForward.normalize();

   VectorF up(0.0f, 0.0f, 1.0f);
   VectorF axisX;
   VectorF axisY = newForward;
   VectorF axisZ;

   if (_up != VectorF::Zero)
      up = _up;

   // Validate and normalize input:  
   F32 lenSq;
   lenSq = axisY.lenSquared();
   if (lenSq < 0.000001f)
   {
      //degenerate forward vector
      axisY.set(0.0f, 1.0f, 0.0f);
   }
   else
   {
      axisY /= mSqrt(lenSq);
   }


   lenSq = up.lenSquared();
   if (lenSq < 0.000001f)
   {
      //degenerate up vector - too small
      up.set(0.0f, 0.0f, 1.0f);
   }
   else
   {
      up /= mSqrt(lenSq);
   }

   if (fabsf(mDot(up, axisY)) > 0.9999f)
   {
      //degenerate up vector - same as forward
      F32 tmp = up.x;
      up.x = -up.y;
      up.y = up.z;
      up.z = tmp;
   }

   // construct the remaining axes:  
   mCross(axisY, up, &axisX);
   mCross(axisX, axisY, &axisZ);

   mat.setColumn(0, axisX);
   mat.setColumn(1, axisY);
   mat.setColumn(2, axisZ);

   set(mat);
}

VectorF RotationF::getDirection()
{
   VectorF dir;
   EulerF angles = asEulerF();
   MathUtils::getVectorFromAngles(dir, angles.z, angles.x);

   return dir;
}

//========================================================
EulerF RotationF::asEulerF(UnitFormat _format) const
{
   if (mRotationType == Euler)
   {
      if (_format == Degrees)
      {
         return EulerF(mRadToDeg(x), mRadToDeg(y), mRadToDeg(z));
      }
      else
      {
         return EulerF(x, y, z);
      }
   }
   else
   {
      EulerF returnEuler = asMatrixF().toEuler();

      if (_format == Degrees)
      {
         returnEuler.x = mRadToDeg(returnEuler.x);
         returnEuler.y = mRadToDeg(returnEuler.y);
         returnEuler.z = mRadToDeg(returnEuler.z);
      }

      return returnEuler;
   }
}

AngAxisF RotationF::asAxisAngle(UnitFormat format) const
{
   AngAxisF returnAA;

   if (mRotationType == Euler)
   {
      returnAA.set(EulerF(x, y, z));
   }
   else
   {
      returnAA.set(Point3F(x, y, z), w);
   }

   if (format == Radians)
   {
      returnAA.angle = mDegToRad(returnAA.angle);
   }

   return returnAA;
}

MatrixF RotationF::asMatrixF() const
{
   MatrixF returnMat;
   if (mRotationType == Euler)
   {
      returnMat.set(EulerF(x, y, z));
   }
   else
   {
      AngAxisF aa;
      aa.set(Point3F(x, y, z), w);

      aa.setMatrix(&returnMat);
   }

   return returnMat;
}

QuatF RotationF::asQuatF() const
{
   QuatF returnQuat;
   if (mRotationType == Euler)
   {
      returnQuat.set(EulerF(x, y, z));
   }
   else
   {
      AngAxisF aa;
      aa.set(Point3F(x, y, z), w);

      returnQuat.set(aa);
   }

   return returnQuat;
}

void RotationF::normalize()
{
   if (mRotationType == Euler)
   {
      EulerF eul = EulerF(x, y, z);
      eul.normalize();
      set(eul);
   }
   else
   {
      QuatF quat;
      quat.set(Point3F(x, y, z), w);

      quat.normalize();

      set(quat);
   }
}

//Testing
#ifdef TORQUE_TESTS_ENABLED
TEST(Maths, RotationF_Calculations)
{
   //TODO: implement unit test
};
#endif

DefineConsoleStaticMethod(rotation, Add, RotationF, (RotationF a, RotationF b), ,
   "Adds two rotations together.\n"
   "@param a Rotation one."
   "@param b Rotation two."
   "@returns v sum of both rotations."
   "@ingroup Math")
{
   return a + b;
}
 
DefineConsoleStaticMethod(rotation, Subtract, RotationF, (RotationF a, RotationF b), ,
   "Subtracts two rotations.\n"
   "@param a Rotation one."
   "@param b Rotation two."
   "@returns v difference of both rotations."
   "@ingroup Math")
{
   return a - b;
}
 
DefineConsoleStaticMethod(rotation, Interpolate, RotationF, (RotationF a, RotationF b, F32 factor), ,
   "Interpolates between two rotations.\n"
   "@param a Rotation one."
   "@param b Rotation two."
   "@param factor The amount to interpolate between the two."
   "@returns v, interpolated result."
   "@ingroup Math")
{
   RotationF result;
   result.interpolate(a, b, factor);
   return result;
}
 
DefineConsoleStaticMethod(rotation, LookAt, RotationF, (Point3F origin, Point3F target, Point3F up),
   (Point3F(0, 0, 0), Point3F(0, 0, 0), Point3F(0, 0, 1)),
   "Provides a rotation orientation to look at a target from a given position.\n"
   "@param origin Position of the object doing the looking."
   "@param target Position to be looked at."
   "@param up The up angle to orient the rotation."
   "@returns v orientation result."
   "@ingroup Math")
{
   RotationF result;
   result.lookAt(origin, target, up);
   return result;
}

DefineConsoleStaticMethod(rotation, getDirection, Point3F, (RotationF rot),,
"Takes the angles of the provided rotation and returns a direction vector.\n"
"@param rot Our rotation."
"@returns v Direction vector result."
"@ingroup Math")
{
   return rot.getDirection();
}
