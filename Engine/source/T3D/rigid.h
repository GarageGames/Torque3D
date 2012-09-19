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

#ifndef _RIGID_H_
#define _RIGID_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif
#ifndef _MQUAT_H_
#include "math/mQuat.h"
#endif

//----------------------------------------------------------------------------

class Rigid
{
public:
   MatrixF objectInertia;        ///< Moment of inertia
   MatrixF invObjectInertia;     ///< Inverse moment of inertia
   MatrixF invWorldInertia;      ///< Inverse moment of inertia in world space

   Point3F force;
   Point3F torque;

   Point3F linVelocity;          ///< Linear velocity
   Point3F linPosition;          ///< Current position
   Point3F linMomentum;          ///< Linear momentum
   Point3F angVelocity;          ///< Angular velocity
   QuatF   angPosition;          ///< Current rotation
   Point3F angMomentum;          ///< Angular momentum

   Point3F centerOfMass;         ///< Center of mass in object space
   Point3F worldCenterOfMass;    ///< CofM in world space
   F32 mass;                     ///< Rigid body mass
   F32 oneOverMass;              ///< 1 / mass
   F32 restitution;              ///< Collision restitution
   F32 friction;                 ///< Friction coefficient
   bool atRest;

private:
   void translateCenterOfMass(const Point3F &oldPos,const Point3F &newPos);

public:
   //
   Rigid();
   void clearForces();
   void integrate(F32 delta);

   void updateInertialTensor();
   void updateVelocity();
   void updateCenterOfMass();

   void applyImpulse(const Point3F &v,const Point3F &impulse);
   bool resolveCollision(const Point3F& p,const Point3F &normal,Rigid*);
   bool resolveCollision(const Point3F& p,const Point3F &normal);

   F32  getZeroImpulse(const Point3F& r,const Point3F& normal);
   F32  getKineticEnergy();
   void getOriginVector(const Point3F &r,Point3F* v);
   void setCenterOfMass(const Point3F &v);
   void getVelocity(const Point3F &p,Point3F* r);
   void getTransform(MatrixF* mat);
   void setTransform(const MatrixF& mat);

   void setObjectInertia(const Point3F& r);
   void setObjectInertia();
   void invertObjectInertia();

   bool checkRestCondition();
   void setAtRest();
};


#endif
