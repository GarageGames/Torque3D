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

#include "T3D/sfx/sfx3DWorld.h"
#include "T3D/portal.h"
#include "T3D/shapeBase.h"
#include "ts/tsShapeInstance.h"
#include "sfx/sfxSystem.h"
#include "math/mBox.h"
#include "core/module.h"
#include "T3D/gameBase/gameConnection.h"


//#define DEBUG_SPEW


MODULE_BEGIN( SFX3D )

   MODULE_INIT_AFTER( Scene )
   MODULE_SHUTDOWN_BEFORE( Scene )
   
   MODULE_INIT
   {
      if( !Con::getBoolVariable( "$SFX::noSFXWorld", false ) )
         gSFX3DWorld = new SFX3DWorld;
   }
   
   MODULE_SHUTDOWN
   {
      if( gSFX3DWorld )
         SAFE_DELETE( gSFX3DWorld );
   }

MODULE_END;


SFX3DWorld* gSFX3DWorld;


//=============================================================================
//    SFX3DObject.
//=============================================================================

//-----------------------------------------------------------------------------

SFX3DObject::SFX3DObject( SFX3DWorld* world, SceneObject* object )
   : Parent( world, object )
{
   if( mObject->isOccludingSound() )
      setFlags( SFXObjectOccluder );
   if( dynamic_cast< Portal* >( mObject ) )
      setFlags( SFXObjectPortal );
   if( object->getSoundAmbience() )
      setFlags( SFXObjectZone );
}

//-----------------------------------------------------------------------------

void SFX3DObject::getEarTransform( MatrixF& transform ) const
{
   // If it's not a ShapeBase, just use the object transform.   
   ShapeBase* shape = dynamic_cast< ShapeBase* >( mObject );
   if ( !shape )
   {
      transform = mObject->getTransform();
      return;
   }

   // It it's ShapeBase, use the earNode transform if one was defined.
   // Otherwise, use the camera transform.
   TSShapeInstance* shapeInstance = shape->getShapeInstance();
   if ( !shapeInstance )
   {
      // Just in case.
      GameConnection* connection = dynamic_cast<GameConnection *>(NetConnection::getConnectionToServer());
      if ( !connection || !connection->getControlCameraTransform( 0.0f, &transform ) )
         transform = mObject->getTransform();
      return;
   }

   ShapeBaseData* datablock = dynamic_cast< ShapeBaseData* >( shape->getDataBlock() );
   AssertFatal( datablock, "SFX3DObject::getEarTransform() - shape without ShapeBaseData datablock!" );
   
   // Get the transform for the ear node.
       
   const S32 earNode = datablock->earNode;

   if ( earNode != -1 && earNode != datablock->eyeNode )
   {
      transform = shape->getTransform();
      transform *= shapeInstance->mNodeTransforms[ earNode ];
   }
   else
   {
      GameConnection* connection = dynamic_cast<GameConnection *>(NetConnection::getConnectionToServer());
      if ( !connection || !connection->getControlCameraTransform( 0.0f, &transform ) )
         transform = mObject->getTransform();
   }
}

//-----------------------------------------------------------------------------

void SFX3DObject::getReferenceCenter( F32 position[ 3 ] ) const
{
   MatrixF transform;
   getEarTransform( transform );
   Point3F pos = transform.getPosition();
   
   AssertFatal( TypeTraits< F32 >::MIN <= pos.x && pos.x <= TypeTraits< F32 >::MAX,
      "SFX3DObject::getReferenceCenter - invalid float in reference center X position" );
   AssertFatal( TypeTraits< F32 >::MIN <= pos.y && pos.y <= TypeTraits< F32 >::MAX,
      "SFX3DObject::getReferenceCenter - invalid float in reference center Y position" );
   AssertFatal( TypeTraits< F32 >::MIN <= pos.z && pos.z <= TypeTraits< F32 >::MAX,
      "SFX3DObject::getReferenceCenter - invalid float in reference center Z position" );
   
   dMemcpy( position, &pos.x, sizeof( F32 ) * 3 );
}

//-----------------------------------------------------------------------------

void SFX3DObject::getBounds( F32 minBounds[ 3 ], F32 maxBounds[ 3 ] ) const
{
   getRealBounds( minBounds, maxBounds );
}

//-----------------------------------------------------------------------------

void SFX3DObject::getRealBounds( F32 minBounds[ 3 ], F32 maxBounds[ 3 ] ) const
{
   const Box3F& worldBox = mObject->getWorldBox();
   dMemcpy( minBounds, &worldBox.minExtents.x, sizeof( F32 ) * 3 );
   dMemcpy( maxBounds, &worldBox.maxExtents.x, sizeof( F32 ) * 3 );
}

//-----------------------------------------------------------------------------

SFXAmbience* SFX3DObject::getAmbience() const
{
   return mObject->getSoundAmbience();
}

//-----------------------------------------------------------------------------

bool SFX3DObject::containsPoint( const F32 point[ 3 ] ) const
{
   return mObject->containsPoint( *( reinterpret_cast< const Point3F* >( &point[ 0 ] ) ) );
}

//-----------------------------------------------------------------------------

String SFX3DObject::describeSelf() const
{
   return mObject->describeSelf();
}

//=============================================================================
//    SFX3DWorld.
//=============================================================================

//-----------------------------------------------------------------------------

SFX3DWorld::SFX3DWorld()
   : Parent( true, TYPEMASK )
{
}

//-----------------------------------------------------------------------------

bool SFX3DWorld::_isTrackableObject( SceneObject* object ) const
{
   if( !Parent::_isTrackableObject( object ) )
      return false;
      
   // We are only interested in occluders, zones, and portals.
      
   if( object->isOccludingSound() )
      return true;
   else if( object->getSoundAmbience() )
      return true;
   else if( dynamic_cast< Portal* >( object ) )
      return true;
      
   return false;
}

//-----------------------------------------------------------------------------

void SFX3DWorld::update()
{
   mSFXWorld.update();
}

//-----------------------------------------------------------------------------

void SFX3DWorld::registerObject( SceneObject* object )
{
   if( !_isTrackableObject( object ) )
      return;
      
   // Construct a new scene object link.
      
   SFX3DObject* sfxObject = mChunker.alloc();
   constructInPlace( sfxObject, this, object );
   
   // Register it with the SFX world.
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFX3DWorld] Registering %i:%s as 0x%x",
      object->getId(), object->getClassName(), sfxObject );
   #endif
   
   mSFXWorld.registerObject( sfxObject );
}

//-----------------------------------------------------------------------------

void SFX3DWorld::unregisterObject( SceneObject* object )
{
   SFX3DObject* sfxObject = dynamic_cast< SFX3DObject* >( SFX3DObject::getLinkForTracker( this, object ) );
   if( !sfxObject )
      return;
      
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFX3DWorld] Unregistering %i:%s (was 0x%x)",
      object->getId(), object->getClassName(), sfxObject );
   #endif

   // Remove the object from the SFX world.
      
   if( sfxObject->isListener() )
      mSFXWorld.setReferenceObject( NULL );
   else
      mSFXWorld.unregisterObject( sfxObject );
   
   // Destroy the scene object link.
   
   destructInPlace( sfxObject );
   mChunker.free( sfxObject );
}

//-----------------------------------------------------------------------------

void SFX3DWorld::updateObject( SceneObjectLink* object )
{
   SFX3DObject* sfxObject = dynamic_cast< SFX3DObject* >( object );
   AssertFatal( sfxObject, "SFX3DWorld::updateObject - invalid object type" );
   
   mSFXWorld.updateObject( sfxObject );
   
   // If this is the listener object, update its
   // properties on the SFX system.
   
   if( sfxObject->isListener() )
   {
      SFXListenerProperties listener;
      sfxObject->getEarTransform( listener.getTransform() );
      listener.getVelocity() = sfxObject->getObject()->getVelocity();
      
      SFX->setListener( 0, listener );
   }
}

//-----------------------------------------------------------------------------

SceneObject* SFX3DWorld::getListener() const
{
   SFX3DObject* object = mSFXWorld.getReferenceObject();
   if( !object )
      return NULL;
      
   return object->getObject();
}

//-----------------------------------------------------------------------------

void SFX3DWorld::setListener( SceneObject* object )
{
   SFX3DObject* oldListener = mSFXWorld.getReferenceObject();

   // If it's the same object as our current listener,
   // return.

   if( oldListener && oldListener->getObject() == object )
      return;

   // Create a SFX3DObject for the given SceneObject.

   SFX3DObject* sfxObject = NULL;
   if( object )
   {
      AssertFatal( !dynamic_cast< SFX3DObject* >( SFX3DObject::getLinkForTracker( this, object ) ),
         "SFX3DWorld::setListener - listener objects must not be registered for tracking" );

      sfxObject = mChunker.alloc();
      constructInPlace( sfxObject, this, object );
      sfxObject->setFlags( SFXObjectListener );
   }

#ifdef DEBUG_SPEW
   if( object )
      Platform::outputDebugString( "[SFX3DWorld] Listener is now %i:%s (%s)",
      object->getId(), object->getClassName(), object->getName() );
   else
      Platform::outputDebugString( "[SFX3DWorld] Unsetting listener" );
#endif

   // Make this object the center of our SFX world.

   mSFXWorld.setReferenceObject( sfxObject );

   // Remove the tracking links from the old listener so we
   // don't see further updates on it.

   if( oldListener )
   {
      destructInPlace( oldListener );
      mChunker.free( oldListener );
   }
}

//-----------------------------------------------------------------------------

void SFX3DWorld::notifyChanged( SceneObject* object )
{
   SFX3DObject* sfxObject = dynamic_cast< SFX3DObject* >( SFX3DObject::getLinkForTracker( this, object ) );

   if( !sfxObject && _isTrackableObject( object ) )
      registerObject( object );
   else if( sfxObject && !_isTrackableObject( object ) )
      unregisterObject( object );
   else if( sfxObject )
      mSFXWorld.notifyChanged( sfxObject );
}

//-----------------------------------------------------------------------------

void SFX3DWorld::debugDump()
{
   mSFXWorld.debugDump();
}
