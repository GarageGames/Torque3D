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

#include "platform/platform.h"
#include "T3D/physics/physicsForce.h"

#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsWorld.h"
#include "T3D/physics/physicsBody.h"
#include "console/engineAPI.h"


IMPLEMENT_CONOBJECT(PhysicsForce);


ConsoleDocClass( PhysicsForce,
   "@brief Helper object for gameplay physical forces.\n\n"
   "%PhysicsForces can be created and \"attached\" to other @link PhysicsBody PhysicsBodies@endlink "
   "to attract them to the position of the PhysicsForce."
   "@ingroup Physics\n"
);


PhysicsForce::PhysicsForce()
   :
      mWorld( NULL ),
      mPhysicsTick( false ),
      mBody( NULL )
{
}

PhysicsForce::~PhysicsForce()
{
}

void PhysicsForce::initPersistFields()
{   
   Parent::initPersistFields();   
}


bool PhysicsForce::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   // Find the physics world we're in.
   if ( PHYSICSMGR )
      mWorld = PHYSICSMGR->getWorld( isServerObject() ? "server" : "client" );

   setProcessTick( true );
   getProcessList()->preTickSignal().notify( this, &PhysicsForce::_preTick );

   return true;
}

void PhysicsForce::onRemove()
{
   mWorld = NULL;
   mBody = NULL;
   getProcessList()->preTickSignal().remove( this, &PhysicsForce::_preTick );
   setProcessTick( false );

   Parent::onRemove();
}

void PhysicsForce::attach( const Point3F &start, const Point3F &direction, F32 maxDist )
{
   detach();

   // If there is no physics world then we cannot apply any forces.
   if ( !mWorld )
      return;

   PhysicsBody *body = mWorld->castRay( start, start + ( direction * maxDist ), PhysicsWorld::BT_Dynamic );
   if ( !body )
      return;

   mBody = body;
}

void PhysicsForce::detach( const Point3F &force )
{
   if ( mBody && !force.isZero() )
   {
      Point3F cMass = mBody->getCMassPosition();      
      F32 mass = mBody->getMass();

      Point3F impulse = ( mass * force ) / TickSec;
      mBody->applyImpulse( cMass, impulse );
   }

   mBody = NULL;
}

void PhysicsForce::onMount( SceneObject *obj, S32 node )
{      
   Parent::onMount( obj, node );

   processAfter( obj );

   MatrixF mat( true );
   mMount.object->getMountTransform( mMount.node, mMount.xfm, &mat );
   setTransform( mat );   
}

void PhysicsForce::onUnmount( SceneObject *obj, S32 node )
{
   clearProcessAfter();
   Parent::onUnmount( obj, node );
}

void PhysicsForce::_preTick()
{
   // How Torque works we sometimes get double or more
   // ticks within a single simulation step.  This occurs
   // for various reasons including odd timesteps and low
   // framerate.
   //
   // In order to keep performance from plumeting we do
   // not tick physics multiple times... instead it just
   // looses time.
   //
   // We set this variable below to let processTick know
   // that physics hasn't stepped yet and to skip processing.
   //
   // This doesn't completely solve the issue, but it does
   // make things much better.
   //
   mPhysicsTick = true;
}

void PhysicsForce::processTick( const Move * )
{
   if ( isMounted() )
   {
      MatrixF test( true );
      test.setPosition( Point3F( 0, 4, 0 ) );
      AssertFatal( test != mMount.xfm, "Error!" );

      MatrixF mat( true );
      mMount.object->getMountTransform( mMount.node, mMount.xfm, &mat );
      setTransform( mat );
   }

   // Nothing to do without a body or if physics hasn't ticked.
   if ( !mBody || !mPhysicsTick )
      return;

   mPhysicsTick = false;

   // If we lost the body then release it.
   if ( !mBody->isDynamic() || !mBody->isSimulationEnabled() )
   {
      detach();
      return;
   }

   // Get our distance to the body.
   Point3F cMass = mBody->getCMassPosition();
   Point3F vector = getPosition() - cMass;

   // Apply the force!
   F32 mass = mBody->getMass();
   Point3F impulse = ( mass * vector ) / TickSec;

   // Counter balance the linear impulse.
   Point3F linVel = mBody->getLinVelocity();
   Point3F currentForce = linVel * mass;

   // Apply it.
   mBody->applyImpulse( cMass, impulse - currentForce );
}

DefineEngineMethod( PhysicsForce, attach, void, ( Point3F start, Point3F direction, F32 maxDist ),,
   "@brief Attempts to associate the PhysicsForce with a PhysicsBody.\n\n"
   "Performs a physics ray cast of the provided length and direction. The %PhysicsForce " 
   "will attach itself to the first dynamic PhysicsBody the ray collides with. "
   "On every tick, the attached body will be attracted towards the position of the %PhysicsForce.\n\n"
   "A %PhysicsForce can only be attached to one body at a time.\n\n"
   "@note To determine if an %attach was successful, check isAttached() immediately after "
   "calling this function.n\n")
{
   object->attach( start, direction, maxDist );
}

DefineEngineMethod( PhysicsForce, detach, void, ( Point3F force ), ( Point3F::Zero ),
   "@brief Disassociates the PhysicsForce from any attached PhysicsBody.\n\n"
   "@param force Optional force to apply to the attached PhysicsBody "
   "before detaching.\n\n"
   "@note Has no effect if the %PhysicsForce is not attached to anything.\n\n")
{
   object->detach( force );
}

DefineEngineMethod( PhysicsForce, isAttached, bool, (),,
   "@brief Returns true if the %PhysicsForce is currently attached to an object.\n\n"
   "@see PhysicsForce::attach()")
{
   return object->isAttached();
}

