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

#include "T3D/components/physics/rigidBodyComponent.h"
#include "core/util/safeDelete.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "core/stream/bitStream.h"
#include "console/engineAPI.h"
#include "sim/netConnection.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsWorld.h"
#include "T3D/physics/physicsCollision.h"
#include "T3D/components/collision/collisionComponent.h"

bool RigidBodyComponent::smNoCorrections = false;
bool RigidBodyComponent::smNoSmoothing = false;

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////
RigidBodyComponent::RigidBodyComponent() : Component()   
{
   mMass = 20;
   mDynamicFriction = 1;
   mStaticFriction = 0.1f;
   mRestitution = 10;
   mLinearDamping = 0;
   mAngularDamping = 0;
   mLinearSleepThreshold = 1;
   mAngularSleepThreshold = 1;
   mWaterDampingScale = 0.1f;
   mBuoyancyDensity = 1;

   mSimType = SimType_ServerOnly;

   mPhysicsRep = NULL;
   mResetPos = MatrixF::Identity;

   mOwnerColComponent = NULL;

   mFriendlyName = "RigidBody(Component)";
}

RigidBodyComponent::~RigidBodyComponent()
{
}

IMPLEMENT_CO_NETOBJECT_V1(RigidBodyComponent);

bool RigidBodyComponent::onAdd()
{
   if(! Parent::onAdd())
      return false;

   return true;
}

void RigidBodyComponent::onRemove()
{
   Parent::onRemove();
}
void RigidBodyComponent::initPersistFields()
{
   Parent::initPersistFields();
}

//This is mostly a catch for situations where the behavior is re-added to the object and the like and we may need to force an update to the behavior
void RigidBodyComponent::onComponentAdd()
{
   Parent::onComponentAdd();

   if (isServerObject())
   {
      storeRestorePos();
      PhysicsPlugin::getPhysicsResetSignal().notify(this, &RigidBodyComponent::_onPhysicsReset);
   }

   CollisionComponent *colComp = mOwner->getComponent<CollisionComponent>();
   if (colComp)
   {
      colComp->onCollisionChanged.notify(this, &RigidBodyComponent::updatePhysics);
      updatePhysics(colComp->getCollisionData());
   }
   else
      updatePhysics();
}

void RigidBodyComponent::onComponentRemove()
{
   Parent::onComponentRemove();

   if (isServerObject())
   {
      PhysicsPlugin::getPhysicsResetSignal().remove(this, &RigidBodyComponent::_onPhysicsReset);
   }

   CollisionComponent *colComp = mOwner->getComponent<CollisionComponent>();
   if (colComp)
   {
      colComp->onCollisionChanged.remove(this, &RigidBodyComponent::updatePhysics);
   }

   SAFE_DELETE(mPhysicsRep);
}

void RigidBodyComponent::componentAddedToOwner(Component *comp)
{
   CollisionComponent *colComp = dynamic_cast<CollisionComponent*>(comp);
   if (colComp)
   {
      colComp->onCollisionChanged.notify(this, &RigidBodyComponent::updatePhysics);
      updatePhysics(colComp->getCollisionData());
   }
}

void RigidBodyComponent::componentRemovedFromOwner(Component *comp)
{
   //test if this is a shape component!
   CollisionComponent *colComp = dynamic_cast<CollisionComponent*>(comp);
   if (colComp)
   {
      colComp->onCollisionChanged.remove(this, &RigidBodyComponent::updatePhysics);
      updatePhysics();
   }
}

void RigidBodyComponent::ownerTransformSet(MatrixF *mat)
{
   if (mPhysicsRep)
      mPhysicsRep->setTransform(mOwner->getTransform());
}

void RigidBodyComponent::updatePhysics(PhysicsCollision* collision)
{
   SAFE_DELETE(mPhysicsRep);

   if (!PHYSICSMGR)
      return;

   mWorld = PHYSICSMGR->getWorld(isServerObject() ? "server" : "client");

   if (!collision)
      return;

   mPhysicsRep = PHYSICSMGR->createBody();

   mPhysicsRep->init(collision, mMass, 0, mOwner, mWorld);

   mPhysicsRep->setMaterial(mRestitution, mDynamicFriction, mStaticFriction);

   mPhysicsRep->setDamping(mLinearDamping, mAngularDamping);
   mPhysicsRep->setSleepThreshold(mLinearSleepThreshold, mAngularSleepThreshold);

   mPhysicsRep->setTransform(mOwner->getTransform());

   // The reset position is the transform on the server
   // at creation time... its not used on the client.
   if (isServerObject())
   {
      storeRestorePos();
      PhysicsPlugin::getPhysicsResetSignal().notify(this, &RigidBodyComponent::_onPhysicsReset);
   }
}

U32 RigidBodyComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & StateMask))
   {
      // This will encode the position relative to the control
      // object position.  
      //
      // This will compress the position to as little as 6.25
      // bytes if the position is within about 30 meters of the
      // control object.
      //
      // Worst case its a full 12 bytes + 2 bits if the position
      // is more than 500 meters from the control object.
      //
      stream->writeCompressedPoint(mState.position);

      // Use only 3.5 bytes to send the orientation.
      stream->writeQuat(mState.orientation, 9);

      // If the server object has been set to sleep then
      // we don't need to send any velocity.
      if (!stream->writeFlag(mState.sleeping))
      {
         // This gives me ~0.015f resolution in velocity magnitude
         // while only costing me 1 bit of the velocity is zero length,
         // <5 bytes in normal cases, and <8 bytes if the velocity is
         // greater than 1000.
         AssertWarn(mState.linVelocity.len() < 1000.0f,
            "PhysicsShape::packUpdate - The linVelocity is out of range!");
         stream->writeVector(mState.linVelocity, 1000.0f, 16, 9);

         // For angular velocity we get < 0.01f resolution in magnitude
         // with the most common case being under 4 bytes.
         AssertWarn(mState.angVelocity.len() < 10.0f,
            "PhysicsShape::packUpdate - The angVelocity is out of range!");
         stream->writeVector(mState.angVelocity, 10.0f, 10, 9);
      }
   }

   return retMask;
}

void RigidBodyComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag()) // StateMask
   {
      PhysicsState state;

      // Read the encoded and compressed position... commonly only 6.25 bytes.
      stream->readCompressedPoint(&state.position);

      // Read the compressed quaternion... 3.5 bytes.
      stream->readQuat(&state.orientation, 9);

      state.sleeping = stream->readFlag();
      if (!state.sleeping)
      {
         stream->readVector(&state.linVelocity, 1000.0f, 16, 9);
         stream->readVector(&state.angVelocity, 10.0f, 10, 9);
      }

      if (!smNoCorrections && mPhysicsRep && mPhysicsRep->isDynamic())
      {
         // Set the new state on the physics object immediately.
         mPhysicsRep->applyCorrection(state.getTransform());

         mPhysicsRep->setSleeping(state.sleeping);
         if (!state.sleeping)
         {
            mPhysicsRep->setLinVelocity(state.linVelocity);
            mPhysicsRep->setAngVelocity(state.angVelocity);
         }

         mPhysicsRep->getState(&mState);
      }

      // If there is no physics object then just set the
      // new state... the tick will take care of the 
      // interpolation and extrapolation.
      if (!mPhysicsRep || !mPhysicsRep->isDynamic())
         mState = state;
   }
}

void RigidBodyComponent::processTick()
{
   Parent::processTick();

   if (!mPhysicsRep || !PHYSICSMGR)
      return;

   // Note that unlike TSStatic, the serverside PhysicsShape does not
   // need to play the ambient animation because even if the animation were
   // to move collision shapes it would not affect the physx representation.

   PROFILE_START(RigidBodyComponent_ProcessTick);

   if (!mPhysicsRep->isDynamic())
      return;

   // SINGLE PLAYER HACK!!!!
   if (PHYSICSMGR->isSinglePlayer() && isClientObject() && getServerObject())
   {
      RigidBodyComponent *servObj = (RigidBodyComponent*)getServerObject();
      mOwner->setTransform(servObj->mState.getTransform());
      mRenderState[0] = servObj->mRenderState[0];
      mRenderState[1] = servObj->mRenderState[1];

      return;
   }

   // Store the last render state.
   mRenderState[0] = mRenderState[1];

   // If the last render state doesn't match the last simulation 
   // state then we got a correction and need to 
   Point3F errorDelta = mRenderState[1].position - mState.position;
   const bool doSmoothing = !errorDelta.isZero() && !smNoSmoothing;

   const bool wasSleeping = mState.sleeping;

   // Get the new physics state.
   mPhysicsRep->getState(&mState);
   updateContainerForces();

   // Smooth the correction back into the render state.
   mRenderState[1] = mState;
   if (doSmoothing)
   {
      F32 correction = mClampF(errorDelta.len() / 20.0f, 0.1f, 0.9f);
      mRenderState[1].position.interpolate(mState.position, mRenderState[0].position, correction);
      mRenderState[1].orientation.interpolate(mState.orientation, mRenderState[0].orientation, correction);
   }

   //Check if any collisions occured
   findContact();

   // If we haven't been sleeping then update our transform
   // and set ourselves as dirty for the next client update.
   if (!wasSleeping || !mState.sleeping)
   {
      // Set the transform on the parent so that
      // the physics object isn't moved.
      mOwner->setTransform(mState.getTransform());

      // If we're doing server simulation then we need
      // to send the client a state update.
      if (isServerObject() && mPhysicsRep && !smNoCorrections &&
         !PHYSICSMGR->isSinglePlayer() // SINGLE PLAYER HACK!!!!
         )
         setMaskBits(StateMask);
   }

   PROFILE_END();
}

void RigidBodyComponent::findContact()
{
   SceneObject *contactObject = NULL;

   VectorF *contactNormal = new VectorF(0, 0, 0);

   Vector<SceneObject*> overlapObjects;

   mPhysicsRep->findContact(&contactObject, contactNormal, &overlapObjects);

   if (!overlapObjects.empty())
   {
      //fire our signal that the physics sim said collisions happened
      onPhysicsCollision.trigger(*contactNormal, overlapObjects);
   }
}

void RigidBodyComponent::_onPhysicsReset(PhysicsResetEvent reset)
{
   if (reset == PhysicsResetEvent_Store)
      mResetPos = mOwner->getTransform();

   else if (reset == PhysicsResetEvent_Restore)
   {
      mOwner->setTransform(mResetPos);
   }
}

void RigidBodyComponent::storeRestorePos()
{
   mResetPos = mOwner->getTransform();
}

void RigidBodyComponent::applyImpulse(const Point3F &pos, const VectorF &vec)
{
   if (mPhysicsRep && mPhysicsRep->isDynamic())
      mPhysicsRep->applyImpulse(pos, vec);
}

void RigidBodyComponent::applyRadialImpulse(const Point3F &origin, F32 radius, F32 magnitude)
{
   if (!mPhysicsRep || !mPhysicsRep->isDynamic())
      return;

   // TODO: Find a better approximation of the
   // force vector using the object box.

   VectorF force = mOwner->getWorldBox().getCenter() - origin;
   F32 dist = force.magnitudeSafe();
   force.normalize();

   if (dist == 0.0f)
      force *= magnitude;
   else
      force *= mClampF(radius / dist, 0.0f, 1.0f) * magnitude;

   mPhysicsRep->applyImpulse(origin, force);

   // TODO: There is no simple way to really sync this sort of an 
   // event with the client.
   //
   // The best is to send the current physics snapshot, calculate the
   // time difference from when this event occured and the time when the
   // client recieves it, and then extrapolate where it should be.
   //
   // Even then its impossible to be absolutely sure its synced.
   //
   // Bottom line... you shouldn't use physics over the network like this.
   //
}

void RigidBodyComponent::updateContainerForces()
{
   PROFILE_SCOPE(RigidBodyComponent_updateContainerForces);

   // If we're not simulating don't update forces.
   PhysicsWorld *world = PHYSICSMGR->getWorld(isServerObject() ? "server" : "client");
   if (!world || !world->isEnabled())
      return;

   ContainerQueryInfo info;
   info.box = mOwner->getWorldBox();
   info.mass = mMass;

   // Find and retreive physics info from intersecting WaterObject(s)
   mOwner->getContainer()->findObjects(mOwner->getWorldBox(), WaterObjectType | PhysicalZoneObjectType, findRouter, &info);

   // Calculate buoyancy and drag
   F32 angDrag = mAngularDamping;
   F32 linDrag = mLinearDamping;
   F32 buoyancy = 0.0f;
   Point3F cmass = mPhysicsRep->getCMassPosition();

   F32 density = mBuoyancyDensity;
   if (density > 0.0f)
   {
      if (info.waterCoverage > 0.0f)
      {
         F32 waterDragScale = info.waterViscosity * mWaterDampingScale;
         F32 powCoverage = mPow(info.waterCoverage, 0.25f);

         angDrag = mLerp(angDrag, angDrag * waterDragScale, powCoverage);
         linDrag = mLerp(linDrag, linDrag * waterDragScale, powCoverage);
      }

      buoyancy = (info.waterDensity / density) * mPow(info.waterCoverage, 2.0f);

      // A little hackery to prevent oscillation
      // Based on this blog post:
      // (http://reinot.blogspot.com/2005/11/oh-yes-they-float-georgie-they-all.html)
      // JCF: disabled!
      Point3F buoyancyForce = buoyancy * -world->getGravity() * TickSec * mMass;
      mPhysicsRep->applyImpulse(cmass, buoyancyForce);
   }

   // Update the dampening as the container might have changed.
   mPhysicsRep->setDamping(linDrag, angDrag);

   // Apply physical zone forces.
   if (!info.appliedForce.isZero())
      mPhysicsRep->applyImpulse(cmass, info.appliedForce);
}