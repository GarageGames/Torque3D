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

#include "T3D/components/physics/physicsBehavior.h"
#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "core/util/safeDelete.h"
#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "ts/tsShapeInstance.h"
#include "core/stream/bitStream.h"
#include "gfx/gfxTransformSaver.h"
#include "console/engineAPI.h"
#include "lighting/lightQuery.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/containerQuery.h"
#include "math/mathIO.h"

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////
PhysicsComponent::PhysicsComponent() : Component()
{
   addComponentField("isStatic", "If enabled, object will not simulate physics", "bool", "0", "");
   addComponentField("gravity", "The direction of gravity affecting this object, as a vector", "vector", "0 0 -9", "");
   addComponentField("drag", "The drag coefficient that constantly affects the object", "float", "0.7", "");
   addComponentField("mass", "The mass of the object", "float", "1", "");

   mStatic = false;
   mAtRest = false;
   mAtRestCounter = 0;

   mGravity = VectorF(0, 0, 0);
   mVelocity = VectorF(0, 0, 0);
   mDrag = 0.7f;
   mMass = 1.f;

   mGravityMod = 1.f;

   csmAtRestTimer = 64;
   sAtRestVelocity = 0.15f;

   mDelta.pos = Point3F(0, 0, 0);
   mDelta.posVec = Point3F(0, 0, 0);
   mDelta.warpTicks = mDelta.warpCount = 0;
   mDelta.dt = 1;
   mDelta.move = NullMove;
   mPredictionCount = 0;
}

PhysicsComponent::~PhysicsComponent()
{
   for(S32 i = 0;i < mFields.size();++i)
   {
      ComponentField &field = mFields[i];
      SAFE_DELETE_ARRAY(field.mFieldDescription);
   }

   SAFE_DELETE_ARRAY(mDescription);
}

IMPLEMENT_CO_NETOBJECT_V1(PhysicsComponent);

void PhysicsComponent::onComponentAdd()
{
   Parent::onComponentAdd();

   // Initialize interpolation vars.      
   mDelta.rot[1] = mDelta.rot[0] = QuatF(mOwner->getTransform());
   mDelta.pos = mOwner->getPosition();
   mDelta.posVec = Point3F(0,0,0);
}

void PhysicsComponent::initPersistFields()
{
   Parent::initPersistFields();

   addField("gravity", TypePoint3F, Offset(mGravity, PhysicsComponent));
   addField("velocity", TypePoint3F, Offset(mVelocity, PhysicsComponent));
   addField("isStatic", TypeBool, Offset(mStatic, PhysicsComponent));
}

U32 PhysicsComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if(stream->writeFlag(mask & VelocityMask))
      mathWrite( *stream, mVelocity );

   if(stream->writeFlag(mask & UpdateMask))
   {
      stream->writeFlag(mStatic);
      stream->writeFlag(mAtRest);
      stream->writeInt(mAtRestCounter,8);

      mathWrite( *stream, mGravity );

      stream->writeFloat(mDrag, 12);
      //stream->writeFloat(mMass, 12);

      stream->writeFloat(mGravityMod, 12);
   }
   return retMask;
}

void PhysicsComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   if(stream->readFlag())
      mathRead( *stream, &mVelocity );

   if(stream->readFlag())
   {
      mStatic = stream->readFlag();
      mAtRest = stream->readFlag();
      mAtRestCounter = stream->readInt(8);

      mathRead( *stream, &mGravity );

      mDrag = stream->readFloat(12);
      //mMass = stream->readFloat(12);

      mGravityMod = stream->readFloat(12);
   }
}

//
void PhysicsComponent::interpolateTick(F32 dt)
{
   Point3F pos = mDelta.pos + mDelta.posVec * dt;
   //Point3F rot = mDelta.rot + mDelta.rotVec * dt;

   setRenderPosition(pos,dt);
}

//
void PhysicsComponent::updateContainer()
{
   PROFILE_SCOPE( PhysicsBehaviorInstance_updateContainer );

   // Update container drag and buoyancy properties

   // Set default values.
   //mDrag = mDataBlock->drag;
   //mBuoyancy = 0.0f;      
   //mGravityMod = 1.0;
   //mAppliedForce.set(0,0,0);

   ContainerQueryInfo info;
   info.box = mOwner->getWorldBox();
   info.mass = mMass;

   mOwner->getContainer()->findObjects(info.box, WaterObjectType|PhysicalZoneObjectType,findRouter,&info);

   //mWaterCoverage = info.waterCoverage;
   //mLiquidType    = info.liquidType;
   //mLiquidHeight  = info.waterHeight;   
   //setCurrentWaterObject( info.waterObject );

   // This value might be useful as a datablock value,
   // This is what allows the player to stand in shallow water (below this coverage)
   // without jiggling from buoyancy
   if (info.waterCoverage >= 0.25f) 
   {      
      // water viscosity is used as drag for in water.
      // ShapeBaseData drag is used for drag outside of water.
      // Combine these two components to calculate this ShapeBase object's 
      // current drag.
      mDrag = ( info.waterCoverage * info.waterViscosity ) + 
         ( 1.0f - info.waterCoverage ) * mDrag;
      //mBuoyancy = (info.waterDensity / mDataBlock->density) * info.waterCoverage;
   }

   //mAppliedForce = info.appliedForce;
   mGravityMod = info.gravityScale;
}
//
void PhysicsComponent::_updatePhysics()
{
   /*SAFE_DELETE( mOwner->mPhysicsRep );

   if ( !PHYSICSMGR )
   return;

   if (mDataBlock->simpleServerCollision)
   {
   // We only need the trigger on the server.
   if ( isServerObject() )
   {
   PhysicsCollision *colShape = PHYSICSMGR->createCollision();
   colShape->addBox( mObjBox.getExtents() * 0.5f, MatrixF::Identity );

   PhysicsWorld *world = PHYSICSMGR->getWorld( isServerObject() ? "server" : "client" );
   mPhysicsRep = PHYSICSMGR->createBody();
   mPhysicsRep->init( colShape, 0, PhysicsBody::BF_TRIGGER | PhysicsBody::BF_KINEMATIC, this, world );
   mPhysicsRep->setTransform( getTransform() );
   }
   }
   else
   {
   if ( !mShapeInstance )
   return;

   PhysicsCollision* colShape = mShapeInstance->getShape()->buildColShape( false, getScale() );

   if ( colShape )
   {
   PhysicsWorld *world = PHYSICSMGR->getWorld( isServerObject() ? "server" : "client" );
   mPhysicsRep = PHYSICSMGR->createBody();
   mPhysicsRep->init( colShape, 0, PhysicsBody::BF_KINEMATIC, this, world );
   mPhysicsRep->setTransform( getTransform() );
   }
   }*/
   return;
}

PhysicsBody *PhysicsComponent::getPhysicsRep()
{
   /*if(mOwner)
   {
      Entity* ac = dynamic_cast<Entity*>(mOwner);
      if(ac)
         return ac->mPhysicsRep;
   }*/
   return NULL;
}
//
void PhysicsComponent::setTransform(const MatrixF& mat)
{
   mOwner->setTransform(mat);

   if (!mStatic)
   {
      mAtRest = false;
      mAtRestCounter = 0;
   }

   if ( getPhysicsRep() )
      getPhysicsRep()->setTransform( mOwner->getTransform() );

   setMaskBits(UpdateMask);
}

void PhysicsComponent::setPosition(const Point3F& pos)
{
   MatrixF mat = mOwner->getTransform();
   if (mOwner->isMounted()) {
      // Use transform from mounted object
      //mOwner->getObjectMount()->getMountTransform( mOwner->getMountNode(), mMount.xfm, &mat );
      return;
   }
   else {
      mat.setColumn(3,pos);
   }

   mOwner->setTransform(mat);

   if ( getPhysicsRep() )
      getPhysicsRep()->setTransform( mat );
}


void PhysicsComponent::setRenderPosition(const Point3F& pos, F32 dt)
{
   MatrixF mat = mOwner->getRenderTransform();
   if (mOwner->isMounted()) {
      // Use transform from mounted object
      //mOwner->getObjectMount()->getMountRenderTransform( dt, mOwner->getMountNode(), mMount.xfm, &mat );
      return;
   }
   else {
      mat.setColumn(3,pos);
   }

   mOwner->setRenderTransform(mat);
}

void PhysicsComponent::updateVelocity(const F32 dt)
{
}

void PhysicsComponent::setVelocity(const VectorF& vel)
{
   mVelocity = vel;

   mAtRest = false;
   mAtRestCounter = 0;
   setMaskBits(VelocityMask);
}

void PhysicsComponent::getVelocity(const Point3F& r, Point3F* v)
{
   *v = mVelocity;
}

void PhysicsComponent::getOriginVector(const Point3F &p,Point3F* r)
{
   *r = p - mOwner->getObjBox().getCenter();
}

F32 PhysicsComponent::getZeroImpulse(const Point3F& r,const Point3F& normal)
{
   Point3F a,b,c;

   //set up our inverse matrix
   MatrixF iv,qmat;
   MatrixF inverse = MatrixF::Identity;
   qmat = mOwner->getTransform();
   iv.mul(qmat,inverse);
   qmat.transpose();
   inverse.mul(iv,qmat);

   mCross(r, normal, &a);
   inverse.mulV(a, &b);
   mCross(b, r, &c);

   return 1 / ((1/mMass) + mDot(c, normal));
}

void PhysicsComponent::accumulateForce(F32 dt, Point3F force)
{
   mVelocity += force * dt;
}

void PhysicsComponent::applyImpulse(const Point3F&,const VectorF& vec)
{
   // Items ignore angular velocity
   VectorF vel;
   vel.x = vec.x / mMass;
   vel.y = vec.y / mMass;
   vel.z = vec.z / mMass;
   setVelocity(mVelocity + vel);
}

DefineEngineMethod( PhysicsComponent, applyImpulse, bool, ( Point3F pos, VectorF vel ),,
                   "@brief Apply an impulse to this object as defined by a world position and velocity vector.\n\n"

                   "@param pos impulse world position\n"
                   "@param vel impulse velocity (impulse force F = m * v)\n"
                   "@return Always true\n"

                   "@note Not all objects that derrive from GameBase have this defined.\n")
{
   object->applyImpulse(pos,vel);
   return true;
}