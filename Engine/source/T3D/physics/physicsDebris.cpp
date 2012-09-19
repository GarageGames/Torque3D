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
#include "T3D/physics/physicsDebris.h"

#include "core/stream/bitStream.h"
#include "math/mathUtils.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "sim/netConnection.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "ts/tsShapeInstance.h"
#include "T3D/gameBase/gameProcess.h"
#include "core/resourceManager.h"
#include "gfx/gfxTransformSaver.h"
#include "lighting/lightQuery.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsCollision.h"
#include "T3D/physics/physicsWorld.h"
#include "collision/concretePolyList.h"
#include "T3D/physics/physicsPlugin.h"
#include "math/mathUtils.h"
#include "gui/worldEditor/worldEditor.h"
#include "T3D/containerQuery.h"


F32 PhysicsDebris::smLifetimeScale = 1.0f;


IMPLEMENT_CO_DATABLOCK_V1( PhysicsDebrisData );

ConsoleDocClass( PhysicsDebrisData,
   
   "@brief Defines the properties of a PhysicsDebris object.\n\n"
   "@see PhysicsDebris.\n"   
   "@ingroup Physics"
);

PhysicsDebrisData::PhysicsDebrisData()
: mass( 1.0f ),
  dynamicFriction( 0.0f ),
  staticFriction( 0.0f ),
  restitution( 0.0f ),
  linearDamping( 0.0f ),
  angularDamping( 0.0f ),
  linearSleepThreshold( 1.0f ),
  angularSleepThreshold( 1.0f ),
  waterDampingScale( 1.0f ),
  buoyancyDensity( 0.0f ),
  castShadows( true )
{
   lifetime = 5.0f;
   lifetimeVariance = 0.0f;
   shapeName = NULL;
}

bool PhysicsDebrisData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   return true;
}

bool PhysicsDebrisData::preload( bool server, String &errorStr )
{
   if ( Parent::preload( server, errorStr ) == false )
      return false;

   if ( server ) return true;

   if ( shapeName && shapeName[0] != '\0' && !bool(shape) )
   {
      shape = ResourceManager::get().load( shapeName );
      if ( bool(shape) == false )
      {
         errorStr = String::ToString( "PhysicsDebrisData::load: Couldn't load shape \"%s\"", shapeName );
         return false;
      }
      else
      {
         // Create a dummy shape to force the generation of shaders and materials
         // during the level load and not during gameplay.
         TSShapeInstance *pDummy = new TSShapeInstance( shape, !server );
         delete pDummy;
      }
   }

   return true;
}

void PhysicsDebrisData::initPersistFields()
{
   addGroup( "Display" );

      addField( "shapeFile", TypeShapeFilename, Offset( shapeName, PhysicsDebrisData ),
         "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
         "Compatable with Live-Asset Reloading.");

      addField( "castShadows", TypeBool, Offset( castShadows, PhysicsDebrisData ), 
        "@brief Determines if the shape's shadow should be cast onto the environment.\n\n" );

   endGroup( "Display" );

   addGroup( "Physical Properties" );

      addField("lifetime", TypeF32, Offset( lifetime, PhysicsDebrisData ),
         "@brief Base time, in seconds, that debris persists after time of creation.\n\n"
         "@note A %PhysicsDebris' lifetime multiplied by it's $pref::PhysicsDebris::lifetimeScale "
         "must be equal to or greater than 1.0.\n\n");

      addField("lifetimeVariance", TypeF32, Offset( lifetimeVariance, PhysicsDebrisData ),
         "@brief Range of variation randomly applied to lifetime when debris is created.\n\n"
         "Represents the maximum amount of seconds that will be added or subtracted to a shape's base lifetime. "
         "A value of 0 will apply the same lifetime to each shape created.\n\n");

      addField( "mass", TypeF32, Offset( mass, PhysicsDebrisData ),
         "@brief Value representing the mass of the shape.\n\n"
         "A shape's mass influences the magnitude of any force applied to it. "
         "@note All PhysicsDebris objects are dynamic.");

      addField( "friction", TypeF32, Offset( dynamicFriction, PhysicsDebrisData ),
         "@brief Coefficient of kinetic %friction to be applied to the shape.\n\n" 
         "Kinetic %friction reduces the velocity of a moving object while it is in contact with a surface. "
         "A larger coefficient will result in a larger reduction in velocity. "
         "A shape's friction should be smaller than it's staticFriction, but greater than 0.\n\n"
         "@note This value is only applied while an object is in motion. For an object starting at rest, see PhysicsDebrisData::staticFriction");

      addField( "staticFriction", TypeF32, Offset( staticFriction, PhysicsDebrisData ),
         "@brief Coefficient of static %friction to be applied to the shape.\n\n" 
         "Static %friction determines the force needed to start moving an at-rest object in contact with a surface. "
         "If the force applied onto shape cannot overcome the force of static %friction, the shape will remain at rest. "
         "A higher coefficient will require a larger force to start motion. "
         "This value should be both greater than 0 and the PhysicsDebrisData::friction.\n\n"
         "@note This value is only applied while an object is at rest. For an object in motion, see PhysicsDebrisData::friction");

      addField( "restitution", TypeF32, Offset( restitution, PhysicsDebrisData ),
         "@brief Bounce coeffecient applied to the shape in response to a collision.\n\n"
         "Restitution is a ratio of a shape's velocity before and after a collision. "
         "A value of 0 will zero out a shape's post-collision velocity, making it stop on contact. "
         "Larger values will remove less velocity after a collision, making it \'bounce\' with greater force. "
         "Normal %restitution values range between 0 and 1.0."
         "@note Values near or equaling 1.0 are likely to cause undesirable results in the physics simulation."
         " Because of this, it is reccomended to avoid values close to 1.0");

      addField( "linearDamping", TypeF32, Offset( linearDamping, PhysicsDebrisData ),
         "@brief Value that reduces an object's linear velocity over time.\n\n"
         "Larger values will cause velocity to decay quicker.\n\n" );

      addField( "angularDamping", TypeF32, Offset( angularDamping, PhysicsDebrisData ),
         "@brief Value that reduces an object's rotational velocity over time.\n\n"
         "Larger values will cause velocity to decay quicker.\n\n" );

      addField( "linearSleepThreshold", TypeF32, Offset( linearSleepThreshold, PhysicsDebrisData ),
         "@brief Minimum linear velocity before the shape can be put to sleep.\n\n"
         "This should be a positive value. Shapes put to sleep will not be simulated in order to save system resources.\n\n"
         "@note The shape must be dynamic.");

      addField( "angularSleepThreshold", TypeF32, Offset( angularSleepThreshold, PhysicsDebrisData ),
         "@brief Minimum rotational velocity before the shape can be put to sleep.\n\n"
         "This should be a positive value. Shapes put to sleep will not be simulated in order to save system resources.\n\n"
         "@note The shape must be dynamic.");

      addField( "waterDampingScale", TypeF32, Offset( waterDampingScale, PhysicsDebrisData ),
         "@brief Scale to apply to linear and angular dampening while underwater.\n\n "
         "@see angularDamping linearDamping" );

      addField( "buoyancyDensity", TypeF32, Offset( buoyancyDensity, PhysicsDebrisData ),
         "@brief The density of this shape for purposes of calculating buoyant forces.\n\n"
         "The result of the calculated buoyancy is relative to the density of the WaterObject the PhysicsDebris is within."
         "@see WaterObject::density");

   endGroup( "Physical Properties" );

   Parent::initPersistFields();
}

void PhysicsDebrisData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->writeFlag( castShadows );
   stream->write( lifetime );
   stream->write( lifetimeVariance );
   stream->write( mass );
   stream->write( dynamicFriction );
   stream->write( staticFriction );
   stream->write( restitution );
   stream->write( linearDamping );
   stream->write( angularDamping );
   stream->write( linearSleepThreshold );
   stream->write( angularSleepThreshold );
   stream->write( waterDampingScale );
   stream->write( buoyancyDensity );
   stream->writeString( shapeName );
}

void PhysicsDebrisData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   castShadows = stream->readFlag();
   stream->read( &lifetime );
   stream->read( &lifetimeVariance );
   stream->read( &mass );
   stream->read( &dynamicFriction );
   stream->read( &staticFriction );
   stream->read( &restitution );
   stream->read( &linearDamping );
   stream->read( &angularDamping );
   stream->read( &linearSleepThreshold );
   stream->read( &angularSleepThreshold );
   stream->read( &waterDampingScale );
   stream->read( &buoyancyDensity );

   shapeName   = stream->readSTString();
}

ConsoleMethod( PhysicsDebrisData, preload, void, 2, 2, 
   "@brief Loads some information to have readily available at simulation time.\n\n"
   "Forces generation of shaders, materials, and other data used by the %PhysicsDebris object. "
   "This function should be used while a level is loading in order to shorten "
   "the amount of time to create a PhysicsDebris in game.\n\n")
{
   String errorStr;

   object->shape = NULL;
   if( !object->preload( false, errorStr ) )
      Con::errorf( "PhsysicsDebrisData::preload - error: %s", errorStr.c_str() );
}


IMPLEMENT_CO_NETOBJECT_V1( PhysicsDebris );

ConsoleDocClass( PhysicsDebris,
   
   "@brief Represents one or more rigid bodies defined in a single mesh file with "
   "a limited lifetime.\n\n"

   "A PhysicsDebris object can be viewed as a single system capable of generating multiple "
   "@link PhysicsBody PhysicsBodies@endlink as debris when triggered. Vaguely similar to how "
   "a ParticleEmitter is capable of creating Particles, but isn't a particle in itself. "

   "After it's lifetime has elapsed, the object will be deleted.\n\n"

   "%PhysicsDebris loads a standard .DAE or .DTS file and creates a rigid body for "
   "each defined collision node.\n\n"

   "For collision nodes to work correctly, they must be setup as follows:\n"
   " - Visible mesh nodes are siblings of the collision node under a common parent dummy node.\n"
   " -  Collision node is a child of its visible mesh node.\n\n"

   "Colmesh type nodes are NOT supported; physx and most standard rigid "
   "body simulations do not support arbitrary triangle meshs for dynamics "
   "do to the computational expense.\n\n"
   "Therefore, collision nodes must be one of the following:\n"
   " - Colbox\n"
   " - Colsphere\n"
   " - Colcapsule\n"
   " - Col (convex).\n\n"

   "%PhysicsDebris should NOT be created on the server.\n\n"
   
   "@ingroup Physics"
);

PhysicsDebris* PhysicsDebris::create(  PhysicsDebrisData *datablock,
                                       const MatrixF &transform,
                                       const VectorF &linVel )
{
   // Skip out if we don't have a datablock or the
   // global lifetime scale has it living less than
   // a second.
   if (  !datablock || 
         (  datablock->lifetime > 0.0f && 
            datablock->lifetime * smLifetimeScale < 1.0f ) )
      return NULL;

   PhysicsDebris *debris = new PhysicsDebris();
   debris->setDataBlock( datablock );
   debris->setTransform( transform );
   debris->mInitialLinVel = linVel;
   if ( !debris->registerObject() )
   {
      delete debris;
      return NULL;
   }

   return debris; 
}

PhysicsDebris::PhysicsDebris()
   :  mLifetime( 0.0f ),
      mShapeInstance( NULL ),
      mWorld( NULL ),
      mInitialLinVel( Point3F::Zero )
{
   mTypeMask |= DebrisObjectType | DynamicShapeObjectType;

   // Only allocated client side.
   mNetFlags.set( IsGhost );   
}

PhysicsDebris::~PhysicsDebris()
{
}

void PhysicsDebris::initPersistFields()
{
   Con::addVariable( "$pref::PhysicsDebris::lifetimeScale", TypeF32, &smLifetimeScale,
      "@brief Scales how long %PhysicsDebris will live before being removed.\n"
      "@note A value of 0 will disable PhysicsDebris entirely.");

   Parent::initPersistFields();
}

bool PhysicsDebris::onAdd()
{
   AssertFatal( isClientObject(), "PhysicsDebris::onAdd - This shouldn't be added on the server!" );

   if ( !Parent::onAdd() )  
      return false;  

   // If it has a fixed lifetime then calculate it.
   if ( mDataBlock->lifetime > 0.0f )
   {
      F32 lifeVar = (mDataBlock->lifetimeVariance * 2.0f * gRandGen.randF(-1.0,1.0)) - mDataBlock->lifetimeVariance;
      mLifetime = mDataBlock->lifetime + lifeVar;
   }

   // Setup our bounding box
   mObjBox = mDataBlock->shape->bounds;   
   resetWorldBox();

   // Add it to the client scene.
   addToScene();

   // We add the debris to the net connection so that
   // it is cleaned up when the client disconnects.
   NetConnection *conn = NetConnection::getConnectionToServer();
   AssertFatal( conn != NULL, "PhysicsDebris::onAdd - Got null net connection!");
   conn->addObject(this);

   PhysicsPlugin::getPhysicsResetSignal().notify( this, &PhysicsDebris::_onPhysicsReset );
   _createFragments();

   return true;
}

bool PhysicsDebris::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   if ( !dptr )
      return false;

   mDataBlock = dynamic_cast< PhysicsDebrisData* >( dptr );
   if ( !mDataBlock )
   {
      Con::errorf( ConsoleLogEntry::General, "PhysicsDebris::onNewDataBlock - datablock ( %i ) is not of type PhysicsDebrisData.", dptr->getId() );
      return false;
   }

   return true;
}

void PhysicsDebris::onRemove()
{   
   PhysicsPlugin::getPhysicsResetSignal().remove( this, &PhysicsDebris::_onPhysicsReset );

   _deleteFragments();

   removeFromScene();

   Parent::onRemove();
}

void PhysicsDebris::processTick( const Move* )
{
   PROFILE_SCOPE( PhysicsDebris_processTick );

   // Delete the debris if our lifetime has expired.
   if (  mDataBlock->lifetime > 0.0f && 
         mIsZero( mLifetime ) )
   {
      deleteObject();
      return;
   }

   MatrixF mat;
   mWorldBox = Box3F::Invalid;
   Box3F bounds;

   FragmentVector::iterator fragment = mFragments.begin();
   for ( ; fragment != mFragments.end(); fragment++ )
   {      
      // Store the last position.
      fragment->lastPos = fragment->pos;
      fragment->lastRot = fragment->rot;

      // Get the new position.
      fragment->body->getTransform( &mat );

      // Calculate the delta between the current
      // global pose and the last global pose.
      fragment->pos = mat.getPosition();
      fragment->rot.set( mat );

      // Update the bounds.
      bounds = fragment->body->getWorldBounds();
      mWorldBox.intersect( bounds );

      // Apply forces for the next tick.
      _updateForces( fragment->body, bounds );
   }

   // Finish up the bounds update.
   mWorldSphere.radius = (mWorldBox.maxExtents - mWorldSphere.center).len();
   mObjBox = mWorldBox;
   mWorldToObj.mul(mObjBox);
   mRenderWorldBox = mWorldBox;
   mRenderWorldSphere = mWorldSphere;
}

void PhysicsDebris::_updateForces( PhysicsBody *body, const Box3F &bounds )
{
   PROFILE_SCOPE( PhysicsDebris_updateForces );

   // If we're not simulating don't update forces.
   if ( !mWorld->isEnabled() )
      return;

   ContainerQueryInfo info;
   info.box = bounds;
   info.mass = mDataBlock->mass;

   // Find and retreive physics info from intersecting WaterObject(s)
   getContainer()->findObjects( bounds, WaterObjectType|PhysicalZoneObjectType, findRouter, &info );

   // Calculate buoyancy and drag
   F32 angDrag = mDataBlock->angularDamping;
   F32 linDrag = mDataBlock->linearDamping;
   F32 buoyancy = 0.0f;
   Point3F cmass = body->getCMassPosition();

   F32 density = mDataBlock->buoyancyDensity;
   if ( density > 0.0f )
   {
      if ( info.waterCoverage > 0.0f )
      {
         F32 waterDragScale = info.waterViscosity * mDataBlock->waterDampingScale;
         F32 powCoverage = mPow( info.waterCoverage, 0.25f );

         angDrag = mLerp( angDrag, angDrag * waterDragScale, powCoverage );
         linDrag = mLerp( linDrag, linDrag * waterDragScale, powCoverage );
      }

      // A little hackery to prevent oscillation
      // Based on this blog post:
      // (http://reinot.blogspot.com/2005/11/oh-yes-they-float-georgie-they-all.html)

      buoyancy = ( info.waterDensity / density ) * mPow( info.waterCoverage, 2.0f );            
      
      Point3F buoyancyForce = buoyancy * -mWorld->getGravity() * TickSec * mDataBlock->mass;
      body->applyImpulse( cmass, buoyancyForce );      
   }

   // Update the dampening as the container might have changed.
   body->setDamping( linDrag, angDrag );

   // Apply physical zone forces.
   if ( !info.appliedForce.isZero() )
      body->applyImpulse( cmass, info.appliedForce );
}

void PhysicsDebris::advanceTime( F32 dt )
{
   // Decrement the lifetime.
   if ( smLifetimeScale > 0.0f )
      mLifetime = getMax( 0.0f, mLifetime - ( dt / smLifetimeScale ) );
   else
      mLifetime = 0.0f;
}

void PhysicsDebris::interpolateTick( F32 dt )
{
   PROFILE_SCOPE( PhysicsDebris_interpolateTick );

   mShapeInstance->animate();
   if ( mShapeInstance->getCurrentDetail() < 0 )
      return;

   const MatrixF &objectXfm = getRenderWorldTransform();
   Vector<MatrixF> &nodeXfms = mShapeInstance->mNodeTransforms;

   MatrixF globalXfm;
   MatrixF tempXfm;
   QuatF newRot;
   Point3F newPos;

   FragmentVector::iterator fragment = mFragments.begin();
   for ( ; fragment != mFragments.end(); fragment++ )
   {      
      // Do the interpolation.
      newRot.interpolate( fragment->rot, fragment->lastRot, dt );
      newRot.setMatrix( &globalXfm );
      newPos.interpolate( fragment->pos, fragment->lastPos, dt );
      globalXfm.setPosition( newPos );

      tempXfm = objectXfm * globalXfm;

      for ( S32 i = 0; i < fragment->nodeIds.size(); i++ )
      {
         S32 n = fragment->nodeIds[i];
         nodeXfms[n] = tempXfm;
      }
   }
}

void PhysicsDebris::prepRenderImage( SceneRenderState *state )
{
   if( !mShapeInstance )
      return;

   // Skip shadow rendering if this debris doesn't support it.
   if (  state->isShadowPass() && 
         !mDataBlock->castShadows )
      return;

   // If the debris is completed LOD'd out then skip it.
   if ( mShapeInstance->setDetailFromPosAndScale( state, getRenderPosition(), getScale() ) < 0 )
      return;

   // Fade out the debris over the last second of its lifetime.
   F32 alpha = 1.0;
   if ( mDataBlock->lifetime > 0.0f )
      alpha = getMin( mLifetime * smLifetimeScale, 1.0f );

   // Set up our TS render state.
   TSRenderState rdata;
   rdata.setSceneState( state );
   rdata.setFadeOverride( alpha );

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init( getWorldSphere() );
   rdata.setLightQuery( &query );

   GFXTransformSaver saver;   

   MatrixF mat = getRenderTransform();
   mat.scale( getScale() );
   GFX->setWorldMatrix( mat );       

   mShapeInstance->animate();
   mShapeInstance->render( rdata );
}

void PhysicsDebris::applyImpulse( const Point3F &pos, const VectorF &vec )
{
   FragmentVector::iterator fragment = mFragments.begin();
   for ( ; fragment != mFragments.end(); fragment++ )
      fragment->body->applyImpulse( pos, vec );
}

void PhysicsDebris::applyRadialImpulse( const Point3F &origin, F32 radius, F32 magnitude )
{
   FragmentVector::iterator fragment = mFragments.begin();
   for ( ; fragment != mFragments.end(); fragment++ )
   {
      PhysicsBody &body = *fragment->body;

      Box3F bounds = body.getWorldBounds();

      VectorF force = bounds.getCenter() - origin;
      F32 dist = force.magnitudeSafe();
      force.normalize();

      if ( dist == 0.0f )
         force *= magnitude;
      else
         force *= mClampF( radius / dist, 0.0f, 1.0f ) * magnitude;

      body.applyImpulse( origin, force );
   }
}

void PhysicsDebris::_createFragments()
{
   _deleteFragments();

   mWorld = PHYSICSMGR->getWorld( "client" );
   if ( !mWorld )
      return;

   TSShape *shape = mDataBlock->shape;

   mShapeInstance = new TSShapeInstance( shape, true );
   mShapeInstance->animate();
   
   Vector< CollisionShapeInfo > infoList;
   shape->buildColShapes( false, Point3F::One, &infoList );
      
   mFragments.setSize( infoList.size() );
   dMemset( mFragments.address(), 0, mFragments.memSize() );

   const Point3F damageDir( 0, 0, 1 );

   MatrixF bodyMat( true );
   bodyMat = getTransform();

   const U32 bodyFlags = PhysicsBody::BF_DEBRIS;
   mWorldBox = Box3F::Invalid;

   for ( S32 i = 0; i < infoList.size(); i++ )
   {      
      const CollisionShapeInfo &info = infoList[i];      

      Fragment &fragment = mFragments[i];

      if ( info.colNode == -1 )      
         Con::errorf( "PhysicsDebris::_createFragments, Missing or couldnt find a colNode." );                  
      else
         _findNodes( info.colNode, fragment.nodeIds );               

      PhysicsBody *body = PHYSICSMGR->createBody();
      body->init( info.colShape, mDataBlock->mass, bodyFlags, this, mWorld );
      body->setMaterial( mDataBlock->restitution, mDataBlock->dynamicFriction, mDataBlock->staticFriction );
      body->setDamping( mDataBlock->linearDamping, mDataBlock->angularDamping );
      body->setSleepThreshold( mDataBlock->linearSleepThreshold, mDataBlock->angularSleepThreshold );
      body->setTransform( bodyMat );
      body->setLinVelocity( mInitialLinVel );
      fragment.body = body;

      // Set the initial delta state.
      fragment.pos = bodyMat.getPosition();
      fragment.rot.set( bodyMat );
      fragment.lastPos = fragment.pos;
      fragment.lastRot = fragment.rot;

      // Update the bounds.
      mWorldBox.intersect( body->getWorldBounds() );
   }   

   // Finish up updating the bounds.
   mWorldSphere.radius = (mWorldBox.maxExtents - mWorldSphere.center).len();
   mObjBox = mWorldBox;
   mWorldToObj.mul(mObjBox);
   mRenderWorldBox = mWorldBox;
   mRenderWorldSphere = mWorldSphere;
}

void PhysicsDebris::_deleteFragments()
{
   FragmentVector::iterator fragment = mFragments.begin();
   for ( ; fragment != mFragments.end(); fragment++ )
      delete fragment->body;

   mFragments.clear();

   SAFE_DELETE( mShapeInstance );
}

void PhysicsDebris::_findNodes( U32 colNode, Vector<U32> &nodeIds )
{   
   // Two possible cases:
   // 1. Visible mesh nodes are siblings of the collision node under a common parent dummy node
   // 2. Collision node is a child of its visible mesh node

   TSShape *shape = mDataBlock->shape;
   S32 itr = shape->nodes[colNode].parentIndex;
   itr = shape->nodes[itr].firstChild;

   while ( itr != -1 )
   {
      if ( itr != colNode )
         nodeIds.push_back(itr);
      itr = shape->nodes[itr].nextSibling;
   }

   // If we didn't find any siblings of the collision node we assume
   // it is case #2 and the collision nodes direct parent is the visible mesh.
   if ( nodeIds.size() == 0 && shape->nodes[colNode].parentIndex != -1 )
      nodeIds.push_back( shape->nodes[colNode].parentIndex );
}

extern bool gEditingMission;

void PhysicsDebris::_onPhysicsReset( PhysicsResetEvent reset )
{
   if ( gEditingMission )
   {
      // Editing stuff, clean up the trash!
      safeDeleteObject();
   }
}