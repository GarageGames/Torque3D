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

#include "T3D/rigid.h"
#include "console/console.h"


//----------------------------------------------------------------------------

Rigid::Rigid()
{
   force.set(0.0f,0.0f,0.0f);
   torque.set(0.0f,0.0f,0.0f);
   linVelocity.set(0.0f,0.0f,0.0f);
   linPosition.set(0.0f,0.0f,0.0f);
   linMomentum.set(0.0f,0.0f,0.0f);
   angVelocity.set(0.0f,0.0f,0.0f);
   angMomentum.set(0.0f,0.0f,0.0f);
   
   angPosition.identity();
   invWorldInertia.identity();

   centerOfMass.set(0.0f,0.0f,0.0f);
   worldCenterOfMass = linPosition;
   mass = oneOverMass = 1.0f;
   invObjectInertia.identity();
   restitution = 0.3f;
   friction = 0.5f;
   atRest = false;
}

void Rigid::clearForces()
{
   force.set(0.0f,0.0f,0.0f);
   torque.set(0.0f,0.0f,0.0f);
}

//-----------------------------------------------------------------------------
void Rigid::integrate(F32 delta)
{
   // Update Angular position
   F32 angle = angVelocity.len();
   if (angle != 0.0f) {
      QuatF dq;
      F32 sinHalfAngle;
      mSinCos(angle * delta * -0.5f, sinHalfAngle, dq.w);
      sinHalfAngle *= 1.0f / angle;
      dq.x = angVelocity.x * sinHalfAngle;
      dq.y = angVelocity.y * sinHalfAngle;
      dq.z = angVelocity.z * sinHalfAngle;
      QuatF tmp = angPosition;
      angPosition.mul(tmp, dq);
      angPosition.normalize();

      // Rotate the position around the center of mass
      Point3F lp = linPosition - worldCenterOfMass;
      dq.mulP(lp,&linPosition);
      linPosition += worldCenterOfMass;
   }

   // Update angular momentum
   angMomentum = angMomentum + torque * delta;

   // Update linear position, momentum
   linPosition = linPosition + linVelocity * delta;
   linMomentum = linMomentum + force * delta;
   linVelocity = linMomentum * oneOverMass;

   // Update dependent state variables
   updateInertialTensor();
   updateVelocity();
   updateCenterOfMass();
}

void Rigid::updateVelocity()
{
   linVelocity.x = linMomentum.x * oneOverMass;
   linVelocity.y = linMomentum.y * oneOverMass;
   linVelocity.z = linMomentum.z * oneOverMass;
   invWorldInertia.mulV(angMomentum,&angVelocity);
}

void Rigid::updateInertialTensor()
{
   MatrixF iv,qmat;
   angPosition.setMatrix(&qmat);
   iv.mul(qmat,invObjectInertia);
   qmat.transpose();
   invWorldInertia.mul(iv,qmat);
}

void Rigid::updateCenterOfMass()
{
   // Move the center of mass into world space
   angPosition.mulP(centerOfMass,&worldCenterOfMass);
   worldCenterOfMass += linPosition;
}

void Rigid::applyImpulse(const Point3F &r, const Point3F &impulse)
{
   atRest = false;

   // Linear momentum and velocity
   linMomentum  += impulse;
   linVelocity.x = linMomentum.x * oneOverMass;
   linVelocity.y = linMomentum.y * oneOverMass;
   linVelocity.z = linMomentum.z * oneOverMass;

   // Rotational momentum and velocity
   Point3F tv;
   mCross(r,impulse,&tv);
   angMomentum += tv;
   invWorldInertia.mulV(angMomentum, &angVelocity);
}

//-----------------------------------------------------------------------------
/** Resolve collision with another rigid body
   Computes & applies the collision impulses needed to keep the bodies
   from interpenetrating.

   tg: This function was commented out... I uncommented it, but haven't
   double checked the math.
*/
bool Rigid::resolveCollision(const Point3F& p, const Point3F &normal, Rigid* rigid)
{
   atRest = false;
   Point3F v1,v2,r1,r2;
   getOriginVector(p,&r1);
   getVelocity(r1,&v1);
   rigid->getOriginVector(p,&r2);
   rigid->getVelocity(r2,&v2);

   // Make sure they are converging
   F32 nv = mDot(v1,normal);
   nv -= mDot(v2,normal);
   if (nv > 0.0f)
      return false;

   // Compute impulse
   F32 d, n = -nv * (2.0f + restitution * rigid->restitution);
   Point3F a1,b1,c1;
   mCross(r1,normal,&a1);
   invWorldInertia.mulV(a1,&b1);
   mCross(b1,r1,&c1);

   Point3F a2,b2,c2;
   mCross(r2,normal,&a2);
   rigid->invWorldInertia.mulV(a2,&b2);
   mCross(b2,r2,&c2);

   Point3F c3 = c1 + c2;
   d = oneOverMass + rigid->oneOverMass + mDot(c3,normal);
   Point3F impulse = normal * (n / d);

   applyImpulse(r1,impulse);
   impulse.neg();
   rigid->applyImpulse(r2, impulse);
   return true;
}

//-----------------------------------------------------------------------------
/** Resolve collision with an immovable object
   Computes & applies the collision impulse needed to keep the body
   from penetrating the given surface.
*/
bool Rigid::resolveCollision(const Point3F& p, const Point3F &normal)
{
   atRest = false;
   Point3F v,r;
   getOriginVector(p,&r);
   getVelocity(r,&v);
   F32 n = -mDot(v,normal);
   if (n >= 0.0f) {

      // Collision impulse, straight forward force stuff.
      F32 d = getZeroImpulse(r,normal);
      F32 j = n * (1.0f + restitution) * d;
      Point3F impulse = normal * j;

      // Friction impulse, calculated as a function of the
      // amount of force it would take to stop the motion
      // perpendicular to the normal.
      Point3F uv = v + (normal * n);
      F32 ul = uv.len();
      if (ul) {
         uv /= -ul;
         F32 u = ul * getZeroImpulse(r,uv);
         j *= friction;
         if (u > j)
            u = j;
         impulse += uv * u;
      }

      //
      applyImpulse(r,impulse);
   }
   return true;
}

//-----------------------------------------------------------------------------
/** Calculate the inertia along the given vector
   This function can be used to calculate the amount of force needed to
   affect a change in velocity along the specified normal applied at
   the given point.
*/
F32 Rigid::getZeroImpulse(const Point3F& r,const Point3F& normal)
{
   Point3F a,b,c;
   mCross(r,normal,&a);
   invWorldInertia.mulV(a,&b);
   mCross(b,r,&c);
   return 1 / (oneOverMass + mDot(c,normal));
}

F32 Rigid::getKineticEnergy()
{
   Point3F w;
   QuatF qmat = angPosition;
   qmat.inverse();
   qmat.mulP(angVelocity,&w);
   const F32* f = invObjectInertia;
   return 0.5f * ((mass * mDot(linVelocity,linVelocity)) +
      w.x * w.x / f[0] +
      w.y * w.y / f[5] +
      w.z * w.z / f[10]);
}

void Rigid::getOriginVector(const Point3F &p,Point3F* r)
{
   *r = p - worldCenterOfMass;
}

void Rigid::setCenterOfMass(const Point3F &newCenter)
{
   // Sets the center of mass relative to the origin.
   centerOfMass = newCenter;

   // Update world center of mass
   angPosition.mulP(centerOfMass,&worldCenterOfMass);
   worldCenterOfMass += linPosition;
}

void Rigid::translateCenterOfMass(const Point3F &oldPos,const Point3F &newPos)
{
   // I + mass * (crossmatrix(centerOfMass)^2 - crossmatrix(newCenter)^2)
   MatrixF oldx,newx;
   oldx.setCrossProduct(oldPos);
   newx.setCrossProduct(newPos);
   for (S32 row = 0; row < 3; row++)
      for (S32 col = 0; col < 3; col++) {
         F32 n = newx(row,col), o = oldx(row,col);
         objectInertia(row,col) += mass * ((o * o) - (n * n));
      }

   // Make sure the matrix is symetrical
   objectInertia(1,0) = objectInertia(0,1);
   objectInertia(2,0) = objectInertia(0,2);
   objectInertia(2,1) = objectInertia(1,2);
}

void Rigid::getVelocity(const Point3F& r, Point3F* v)
{
   mCross(angVelocity, r, v);
   *v += linVelocity;
}

void Rigid::getTransform(MatrixF* mat)
{
   angPosition.setMatrix(mat);
   mat->setColumn(3,linPosition);
}

void Rigid::setTransform(const MatrixF& mat)
{
   angPosition.set(mat);
   mat.getColumn(3,&linPosition);

   // Update center of mass
   angPosition.mulP(centerOfMass,&worldCenterOfMass);
   worldCenterOfMass += linPosition;
}


//----------------------------------------------------------------------------
/** Set the rigid body moment of inertia
   The moment is calculated as a box with the given dimensions.
*/
void Rigid::setObjectInertia(const Point3F& r)
{
   // Rotational moment of inertia of a box
   F32 ot = mass / 12.0f;
   F32 a = r.x * r.x;
   F32 b = r.y * r.y;
   F32 c = r.z * r.z;

   objectInertia.identity();
   F32* f = objectInertia;
   f[0]  = ot * (b + c);
   f[5]  = ot * (c + a);
   f[10] = ot * (a + b);

   invertObjectInertia();
   updateInertialTensor();
}


//----------------------------------------------------------------------------
/** Set the rigid body moment of inertia
   The moment is calculated as a unit sphere.
*/
void Rigid::setObjectInertia()
{
   objectInertia.identity();
   F32 radius = 1.0f;
   F32* f = objectInertia;
   f[0] = f[5] = f[10] = (0.4f * mass * radius * radius);
   invertObjectInertia();
   updateInertialTensor();
}

void Rigid::invertObjectInertia()
{
  invObjectInertia = objectInertia;
  invObjectInertia.fullInverse();
}


//----------------------------------------------------------------------------

bool Rigid::checkRestCondition()
{
//   F32 k = getKineticEnergy(mWorldToObj);
//   F32 G = -force.z * oneOverMass * 0.032;
//   F32 Kg = 0.5 * mRigid.mass * G * G;
//   if (k < Kg * restTol)
//      mRigid.setAtRest();
   return atRest;
}

void Rigid::setAtRest()
{
   atRest = true;
   linVelocity.set(0.0f,0.0f,0.0f);
   linMomentum.set(0.0f,0.0f,0.0f);
   angVelocity.set(0.0f,0.0f,0.0f);
   angMomentum.set(0.0f,0.0f,0.0f);
}
