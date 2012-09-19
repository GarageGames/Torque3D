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
#include "scene/sceneObject.h"

#include "platform/profiler.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "console/simPersistID.h"
#include "sim/netConnection.h"
#include "core/stream/bitStream.h"
#include "scene/sceneManager.h"
#include "scene/sceneTracker.h"
#include "scene/sceneRenderState.h"
#include "scene/zones/sceneZoneSpace.h"
#include "collision/extrudedPolyList.h"
#include "collision/earlyOutPolyList.h"
#include "collision/optimizedPolyList.h"
#include "math/mPolyhedron.h"
#include "gfx/bitmap/gBitmap.h"
#include "math/util/frustum.h"
#include "math/mathIO.h"
#include "math/mTransform.h"
#include "T3D/gameBase/gameProcess.h"

IMPLEMENT_CONOBJECT(SceneObject);

ConsoleDocClass( SceneObject,
   "@brief A networkable object that exists in the 3D world.\n\n"

   "The SceneObject class provides the foundation for 3D objects in the Engine.  It "
   "exposes the functionality for:\n\n"

   "<ul><li>Position, rotation and scale within the world.</li>"
   "<li>Working with a scene graph (in the Zone and Portal sections), allowing efficient "
   "and robust rendering of the game scene.</li>"
   "<li>Various helper functions, including functions to get bounding information "
   "and momentum/velocity.</li>"
   "<li>Mounting one SceneObject to another.</li>"
   "<li>An interface for collision detection, as well as ray casting.</li>"
   "<li>Lighting. SceneObjects can register lights both at lightmap generation "
   "time, and dynamic lights at runtime (for special effects, such as from flame "
   "or a projectile, or from an explosion).</li></ul>\n\n"

   "You do not typically work with SceneObjects themselves.  The SceneObject provides a reference "
   "within the game world (the scene), but does not render to the client on its own.  The "
   "same is true of collision detection beyond that of the bounding box.  Instead you "
   "use one of the many classes that derrive from SceneObject, such as TSStatic.\n\n"

   "@section SceneObject_Hiding Difference Between setHidden() and isRenderEnabled\n\n"

   "When it comes time to decide if a SceneObject should render or not, there are two "
   "methods that can stop the SceneObject from rendering at all.  You need to be aware of "
   "the differences between these two methods as they impact how the SceneObject is networked "
   "from the server to the client.\n\n"

   "The first method of manually controlling if a SceneObject is rendered is through its "
   "SceneObject::isRenderEnabled property.  When set to false the SceneObject is considered invisible but "
   "still present within the scene.  This means it still takes part in collisions and continues "
   "to be networked.\n\n"

   "The second method is using the setHidden() method.  This will actually remove a SceneObject "
   "from the scene and it will no longer be networked from the server to the cleint.  Any client-side "
   "ghost of the object will be deleted as the server no longer considers the object to be in scope.\n\n"

   "@ingroup gameObjects\n"
);


Signal< void( SceneObject* ) > SceneObject::smSceneObjectAdd;
Signal< void( SceneObject* ) > SceneObject::smSceneObjectRemove;


//-----------------------------------------------------------------------------

SceneObject::SceneObject()
{
   mContainer = 0;
   mTypeMask = DefaultObjectType;
   mCollisionCount = 0;
   mGlobalBounds = false;

   mObjScale.set(1,1,1);
   mObjToWorld.identity();
   mWorldToObj.identity();

   mObjBox      = Box3F(Point3F(0, 0, 0), Point3F(0, 0, 0));
   mWorldBox    = Box3F(Point3F(0, 0, 0), Point3F(0, 0, 0));
   mWorldSphere = SphereF(Point3F(0, 0, 0), 0);

   mRenderObjToWorld.identity();
   mRenderWorldToObj.identity();
   mRenderWorldBox = Box3F(Point3F(0, 0, 0), Point3F(0, 0, 0));
   mRenderWorldSphere = SphereF(Point3F(0, 0, 0), 0);

   mContainerSeqKey = 0;

   mBinRefHead = NULL;

   mSceneManager = NULL;

   mNumCurrZones = 0;
   mZoneRefHead = NULL;
   mZoneRefDirty = false;

   mBinMinX = 0xFFFFFFFF;
   mBinMaxX = 0xFFFFFFFF;
   mBinMinY = 0xFFFFFFFF;
   mBinMaxY = 0xFFFFFFFF;
   mLightPlugin = NULL;

   mMount.object = NULL;
   mMount.link = NULL;
   mMount.list = NULL;
   mMount.node = -1;
   mMount.xfm = MatrixF::Identity;
   mMountPID = NULL;

   mSceneObjectLinks = NULL;

   mObjectFlags.set( RenderEnabledFlag | SelectionEnabledFlag );
   mIsScopeAlways = false;
}

//-----------------------------------------------------------------------------

SceneObject::~SceneObject()
{
   AssertFatal( mZoneRefHead == NULL && mBinRefHead == NULL,
      "SceneObject::~SceneObject - Object still linked in reference lists!");
   AssertFatal( !mSceneObjectLinks,
      "SceneObject::~SceneObject() - object is still linked to SceneTrackers" );

   unlink();
}

//-----------------------------------------------------------------------------

bool SceneObject::castRayRendered(const Point3F &start, const Point3F &end, RayInfo *info)
{
   // By default, all ray checking against the rendered mesh will be passed
   // on to the collision mesh.  This saves having to define both methods
   // for simple objects.
   return castRay( start, end, info );
}

//-----------------------------------------------------------------------------

bool SceneObject::containsPoint( const Point3F& point )
{
   // If it's not in the AABB, then it can't be in the OBB either,
   // so early out.

   if( !mWorldBox.isContained( point ) )
      return false;

   // Transform point into object space and test it against
   // our object space bounding box.

   Point3F objPoint( 0, 0, 0 );
   getWorldTransform().mulP( point, &objPoint );
   objPoint.convolveInverse( getScale() );

   return ( mObjBox.isContained( objPoint ) );
}

//-----------------------------------------------------------------------------

bool SceneObject::collideBox(const Point3F &start, const Point3F &end, RayInfo *info)
{
   const F32 * pStart = (const F32*)start;
   const F32 * pEnd = (const F32*)end;
   const F32 * pMin = (const F32*)mObjBox.minExtents;
   const F32 * pMax = (const F32*)mObjBox.maxExtents;

   F32 maxStartTime = -1;
   F32 minEndTime = 1;
   F32 startTime;
   F32 endTime;

   // used for getting normal
   U32 hitIndex = 0xFFFFFFFF;
   U32 side;

   // walk the axis
   for(U32 i = 0; i < 3; i++)
   {
      //
      if(pStart[i] < pEnd[i])
      {
         if(pEnd[i] < pMin[i] || pStart[i] > pMax[i])
            return(false);

         F32 dist = pEnd[i] - pStart[i];

         startTime = (pStart[i] < pMin[i]) ? (pMin[i] - pStart[i]) / dist : -1;
         endTime = (pEnd[i] > pMax[i]) ? (pMax[i] - pStart[i]) / dist : 1;
         side = 1;
      }
      else
      {
         if(pStart[i] < pMin[i] || pEnd[i] > pMax[i])
            return(false);

         F32 dist = pStart[i] - pEnd[i];
         startTime = (pStart[i] > pMax[i]) ? (pStart[i] - pMax[i]) / dist : -1;
         endTime = (pEnd[i] < pMin[i]) ? (pStart[i] - pMin[i]) / dist : 1;
         side = 0;
      }

      //
      if(startTime > maxStartTime)
      {
         maxStartTime = startTime;
         hitIndex = i * 2 + side;
      }
      if(endTime < minEndTime)
         minEndTime = endTime;
      if(minEndTime < maxStartTime)
         return(false);
   }

   // fail if inside
   if(maxStartTime < 0.f)
      return(false);

   //
   static Point3F boxNormals[] = {
      Point3F( 1, 0, 0),
      Point3F(-1, 0, 0),
      Point3F( 0, 1, 0),
      Point3F( 0,-1, 0),
      Point3F( 0, 0, 1),
      Point3F( 0, 0,-1),
   };

   //
   AssertFatal(hitIndex != 0xFFFFFFFF, "SceneObject::collideBox");
   info->t = maxStartTime;
   info->object = this;
   mObjToWorld.mulV(boxNormals[hitIndex], &info->normal);
   info->material = 0;
   return(true);
}

//-----------------------------------------------------------------------------

void SceneObject::disableCollision()
{
   mCollisionCount++;
   AssertFatal(mCollisionCount < 50, "SceneObject::disableCollision called 50 times on the same object. Is this inside a circular loop?" );
}

//-----------------------------------------------------------------------------

void SceneObject::enableCollision()
{
   if (mCollisionCount)
      --mCollisionCount;
}

//-----------------------------------------------------------------------------

bool SceneObject::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   mIsScopeAlways = mNetFlags.test( ScopeAlways );

   mWorldToObj = mObjToWorld;
   mWorldToObj.affineInverse();
   resetWorldBox();

   setRenderTransform(mObjToWorld);

   resolveMountPID();

   smSceneObjectAdd.trigger(this);

   return true;
}

//-----------------------------------------------------------------------------

void SceneObject::onRemove()
{
   smSceneObjectRemove.trigger(this);

   unmount();
   plUnlink();

   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void SceneObject::addToScene()
{
   if( mSceneManager )
      return;

   if( isClientObject() )
      gClientSceneGraph->addObjectToScene( this );
   else
      gServerSceneGraph->addObjectToScene( this );
}

//-----------------------------------------------------------------------------

void SceneObject::removeFromScene()
{
   if( !mSceneManager )
      return;

   mSceneManager->removeObjectFromScene( this );
}

//-----------------------------------------------------------------------------

void SceneObject::onDeleteNotify( SimObject *obj )
{      
   // We are comparing memory addresses so even if obj really is not a 
   // ProcessObject this cast shouldn't break anything.
   if ( obj == mAfterObject )
      mAfterObject = NULL;

   if ( obj == mMount.object )
      unmount();

   Parent::onDeleteNotify( obj );   
}

//-----------------------------------------------------------------------------

void SceneObject::inspectPostApply()
{
   if( isServerObject() )
      setMaskBits( MountedMask );

   Parent::inspectPostApply();
}

//-----------------------------------------------------------------------------

void SceneObject::setGlobalBounds()
{ 
   mGlobalBounds = true;
   mObjBox.minExtents.set( -1e10, -1e10, -1e10 );
   mObjBox.maxExtents.set(  1e10,  1e10,  1e10 );

   if( mSceneManager )
      mSceneManager->notifyObjectDirty( this );
}

//-----------------------------------------------------------------------------

void SceneObject::setTransform( const MatrixF& mat )
{
   // This test is a bit expensive so turn it off in release.   
#ifdef TORQUE_DEBUG
   //AssertFatal( mat.isAffine(), "SceneObject::setTransform() - Bad transform (non affine)!" );
#endif

   PROFILE_SCOPE( SceneObject_setTransform );

   // Update the transforms.

   mObjToWorld = mWorldToObj = mat;
   mWorldToObj.affineInverse();

   // Update the world-space AABB.

   resetWorldBox();

   // If we're in a SceneManager, sync our scene state.

   if( mSceneManager != NULL )
      mSceneManager->notifyObjectDirty( this );

   setRenderTransform( mat );
}

//-----------------------------------------------------------------------------

void SceneObject::setScale( const VectorF &scale )
{
	AssertFatal( !mIsNaN( scale ), "SceneObject::setScale() - The scale is NaN!" );

   // Avoid unnecessary scaling operations.
   if ( mObjScale.equal( scale ) )
      return;

   mObjScale = scale;
   setTransform(MatrixF(mObjToWorld));

   // Make sure that any subclasses of me get a chance to react to the
   // scale being changed.
   onScaleChanged();

   setMaskBits( ScaleMask );
}

//-----------------------------------------------------------------------------

void SceneObject::resetWorldBox()
{
   AssertFatal(mObjBox.isValidBox(), "SceneObject::resetWorldBox - Bad object box!");

   mWorldBox = mObjBox;
   mWorldBox.minExtents.convolve(mObjScale);
   mWorldBox.maxExtents.convolve(mObjScale);
   mObjToWorld.mul(mWorldBox);

   AssertFatal(mWorldBox.isValidBox(), "SceneObject::resetWorldBox - Bad world box!");

   // Create mWorldSphere from mWorldBox
   mWorldBox.getCenter(&mWorldSphere.center);
   mWorldSphere.radius = (mWorldBox.maxExtents - mWorldSphere.center).len();

   // Update tracker links.
   
   for( SceneObjectLink* link = mSceneObjectLinks; link != NULL; 
        link = link->getNextLink() )
      link->update();
}

//-----------------------------------------------------------------------------

void SceneObject::resetObjectBox()
{
   AssertFatal( mWorldBox.isValidBox(), "SceneObject::resetObjectBox - Bad world box!" );

   mObjBox = mWorldBox;
   mWorldToObj.mul( mObjBox );

   Point3F objScale( mObjScale );
   objScale.setMax( Point3F( (F32)POINT_EPSILON, (F32)POINT_EPSILON, (F32)POINT_EPSILON ) );
   mObjBox.minExtents.convolveInverse( objScale );
   mObjBox.maxExtents.convolveInverse( objScale );

   AssertFatal( mObjBox.isValidBox(), "SceneObject::resetObjectBox - Bad object box!" );

   // Update the mWorldSphere from mWorldBox
   mWorldBox.getCenter( &mWorldSphere.center );
   mWorldSphere.radius = ( mWorldBox.maxExtents - mWorldSphere.center ).len();

   // Update scene managers.
   
   for( SceneObjectLink* link = mSceneObjectLinks; link != NULL; 
        link = link->getNextLink() )
      link->update();
}

//-----------------------------------------------------------------------------

void SceneObject::setRenderTransform(const MatrixF& mat)
{
   PROFILE_START(SceneObj_setRenderTransform);
   mRenderObjToWorld = mRenderWorldToObj = mat;
   mRenderWorldToObj.affineInverse();

   AssertFatal(mObjBox.isValidBox(), "Bad object box!");
   resetRenderWorldBox();
   PROFILE_END();
}

//-----------------------------------------------------------------------------

void SceneObject::resetRenderWorldBox()
{
   AssertFatal( mObjBox.isValidBox(), "Bad object box!" );

   mRenderWorldBox = mObjBox;
   mRenderWorldBox.minExtents.convolve( mObjScale );
   mRenderWorldBox.maxExtents.convolve( mObjScale );
   mRenderObjToWorld.mul( mRenderWorldBox );

   AssertFatal( mRenderWorldBox.isValidBox(), "Bad world box!" );

   // Create mRenderWorldSphere from mRenderWorldBox.

   mRenderWorldBox.getCenter( &mRenderWorldSphere.center );
   mRenderWorldSphere.radius = ( mRenderWorldBox.maxExtents - mRenderWorldSphere.center ).len();
}

//-----------------------------------------------------------------------------

void SceneObject::setHidden( bool hidden )
{
   if( hidden != isHidden() )
   {
      // Add/remove the object from the scene.  Removing it
      // will also cause the NetObject to go out of scope since
      // the container query will not find it anymore.  However,
      // ScopeAlways objects need to be treated separately as we
      // do next.

      if( !hidden )
         addToScene();
      else
         removeFromScene();

      // ScopeAlways objects stay in scope no matter what, i.e. even
      // if they aren't in the scene query anymore.  So, to force ghosts
      // to go away, we need to clear ScopeAlways while we are hidden.

      if( hidden && mIsScopeAlways )
         clearScopeAlways();
      else if( !hidden && mIsScopeAlways )
         setScopeAlways();

      Parent::setHidden( hidden );
   }
}

//-----------------------------------------------------------------------------

void SceneObject::initPersistFields()
{
   addGroup( "Transform" );

      addProtectedField( "position", TypeMatrixPosition, Offset( mObjToWorld, SceneObject ),
         &_setFieldPosition, &defaultProtectedGetFn,
         "Object world position." );
      addProtectedField( "rotation", TypeMatrixRotation, Offset( mObjToWorld, SceneObject ),
         &_setFieldRotation, &defaultProtectedGetFn,
         "Object world orientation." );
      addProtectedField( "scale", TypePoint3F, Offset( mObjScale, SceneObject ),
         &_setFieldScale, &defaultProtectedGetFn,
         "Object world scale." );

   endGroup( "Transform" );

   addGroup( "Editing" );

      addProtectedField( "isRenderEnabled", TypeBool, Offset( mObjectFlags, SceneObject ),
         &_setRenderEnabled, &_getRenderEnabled,
         "Controls client-side rendering of the object.\n"
         "@see isRenderable()\n" );

      addProtectedField( "isSelectionEnabled", TypeBool, Offset( mObjectFlags, SceneObject ),
         &_setSelectionEnabled, &_getSelectionEnabled,
         "Determines if the object may be selected from wihin the Tools.\n"
         "@see isSelectable()\n" );

   endGroup( "Editing" );

   addGroup( "Mounting" );

      addProtectedField( "mountPID", TypePID, Offset( mMountPID, SceneObject ), &_setMountPID, &defaultProtectedGetFn,
         "@brief PersistentID of object we are mounted to.\n\n"
         "Unlike the SimObjectID that is determined at run time, the PersistentID of an object is saved with the level/mission and "
         "may be used to form a link between objects." );
      addField( "mountNode", TypeS32, Offset( mMount.node, SceneObject ), "Node we are mounted to." );
      addField( "mountPos", TypeMatrixPosition, Offset( mMount.xfm, SceneObject ), "Position we are mounted at ( object space of our mount object )." );
      addField( "mountRot", TypeMatrixRotation, Offset( mMount.xfm, SceneObject ), "Rotation we are mounted at ( object space of our mount object )." );

   endGroup( "Mounting" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SceneObject::_setFieldPosition( void *object, const char *index, const char *data )
{
   SceneObject* so = static_cast<SceneObject*>( object );
   if ( so )
   {
      MatrixF txfm( so->getTransform() );
      Con::setData( TypeMatrixPosition, &txfm, 0, 1, &data );
      so->setTransform( txfm );
   }
   return false;
}

//-----------------------------------------------------------------------------

bool SceneObject::_setFieldRotation( void *object, const char *index, const char *data )
{
   SceneObject* so = static_cast<SceneObject*>( object );
   if ( so )
   {
      MatrixF txfm( so->getTransform() );
      Con::setData( TypeMatrixRotation, &txfm, 0, 1, &data );
      so->setTransform( txfm );
   }
   return false;
}

//-----------------------------------------------------------------------------

bool SceneObject::_setFieldScale( void *object, const char *index, const char *data )
{
   SceneObject* so = static_cast<SceneObject*>( object );
   if ( so )
   {
      Point3F scale;
      Con::setData( TypePoint3F, &scale, 0, 1, &data );
      so->setScale( scale );
   }
   return false;
}

//-----------------------------------------------------------------------------

bool SceneObject::writeField( StringTableEntry fieldName, const char* value )
{
   if( !Parent::writeField( fieldName, value ) )
      return false;
      
   static StringTableEntry sIsRenderEnabled = StringTable->insert( "isRenderEnabled" );
   static StringTableEntry sIsSelectionEnabled = StringTable->insert( "isSelectionEnabled" );
   static StringTableEntry sMountNode = StringTable->insert( "mountNode" );
   static StringTableEntry sMountPos = StringTable->insert( "mountPos" );
   static StringTableEntry sMountRot = StringTable->insert( "mountRot" );
   
   // Don't write flag fields if they are at their default values.
   
   if( fieldName == sIsRenderEnabled && dAtob( value ) )
      return false;
   else if( fieldName == sIsSelectionEnabled && dAtob( value ) )
      return false;
   else if ( mMountPID == NULL && ( fieldName == sMountNode || 
                                    fieldName == sMountPos || 
                                    fieldName == sMountRot ) )
   {
      return false;
   }

      
   return true;
}

//-----------------------------------------------------------------------------

static void scopeCallback( SceneObject* obj, void* conPtr )
{
   NetConnection* ptr = reinterpret_cast< NetConnection* >( conPtr );
   if( obj->isScopeable() )
      ptr->objectInScope(obj);
}

void SceneObject::onCameraScopeQuery( NetConnection* connection, CameraScopeQuery* query )
{
   // Object itself is in scope.

   if( this->isScopeable() )
      connection->objectInScope( this );

   // If we're mounted to something, that object is in scope too.

   if( isMounted() )
      connection->objectInScope( mMount.object );

   // If we're added to a scene graph, let the graph do the scene scoping.
   // Otherwise just put everything in the server container in scope.

   if( getSceneManager() )
      getSceneManager()->scopeScene( query, connection );
   else
      gServerContainer.findObjects( 0xFFFFFFFF, scopeCallback, connection );
}

//-----------------------------------------------------------------------------

bool SceneObject::isRenderEnabled() const
{
   AbstractClassRep *classRep = getClassRep();
   return ( mObjectFlags.test( RenderEnabledFlag ) && classRep->isRenderEnabled() );
}

//-----------------------------------------------------------------------------

void SceneObject::setRenderEnabled( bool value )
{
   if( value )
      mObjectFlags.set( RenderEnabledFlag );
   else
      mObjectFlags.clear( RenderEnabledFlag );
      
   setMaskBits( FlagMask );
}

//-----------------------------------------------------------------------------

const char* SceneObject::_getRenderEnabled( void* object, const char* data )
{
   SceneObject* obj = reinterpret_cast< SceneObject* >( object );
   if( obj->mObjectFlags.test( RenderEnabledFlag ) )
      return "true";
   else
      return "false";
}

//-----------------------------------------------------------------------------

bool SceneObject::_setRenderEnabled( void *object, const char *index, const char *data )
{
   SceneObject* obj = reinterpret_cast< SceneObject* >( object );
   obj->setRenderEnabled( dAtob( data ) );
   return false;
}

//-----------------------------------------------------------------------------

bool SceneObject::isSelectionEnabled() const
{
   AbstractClassRep *classRep = getClassRep();
   return ( mObjectFlags.test( SelectionEnabledFlag ) && classRep->isSelectionEnabled() );
}

//-----------------------------------------------------------------------------

void SceneObject::setSelectionEnabled( bool value )
{
   if( value )
      mObjectFlags.set( SelectionEnabledFlag );
   else
      mObjectFlags.clear( SelectionEnabledFlag );
      
   // Not synchronized on network so don't set dirty bit.
}

//-----------------------------------------------------------------------------

const char* SceneObject::_getSelectionEnabled( void* object, const char* data )
{
   SceneObject* obj = reinterpret_cast< SceneObject* >( object );
   if( obj->mObjectFlags.test( SelectionEnabledFlag ) )
      return "true";
   else
      return "false";
}

//-----------------------------------------------------------------------------

bool SceneObject::_setSelectionEnabled( void *object, const char *index, const char *data )
{
   SceneObject* obj = reinterpret_cast< SceneObject* >( object );
   obj->setSelectionEnabled( dAtob( data ) );
   return false;
}

//--------------------------------------------------------------------------

U32 SceneObject::packUpdate( NetConnection* conn, U32 mask, BitStream* stream )
{
   U32 retMask = Parent::packUpdate( conn, mask, stream );

   if ( stream->writeFlag( mask & FlagMask ) )
      stream->writeRangedU32( (U32)mObjectFlags, 0, getObjectFlagMax() );

   if ( mask & MountedMask ) 
   {                  
      if ( mMount.object ) 
      {
         S32 gIndex = conn->getGhostIndex( mMount.object );

         if ( stream->writeFlag( gIndex != -1 ) ) 
         {
            stream->writeFlag( true );
            stream->writeInt( gIndex, NetConnection::GhostIdBitSize );
            if ( stream->writeFlag( mMount.node != -1 ) )
               stream->writeInt( mMount.node, NumMountPointBits );
            mathWrite( *stream, mMount.xfm );
         }
         else
            // Will have to try again later
            retMask |= MountedMask;
      }
      else
         // Unmount if this isn't the initial packet
         if ( stream->writeFlag( !(mask & InitialUpdateMask) ) )
            stream->writeFlag( false );
   }
   else
      stream->writeFlag( false );
   
   return retMask;
}

//-----------------------------------------------------------------------------

void SceneObject::unpackUpdate( NetConnection* conn, BitStream* stream )
{
   Parent::unpackUpdate( conn, stream );
   
   // FlagMask
   if ( stream->readFlag() )      
      mObjectFlags = stream->readRangedU32( 0, getObjectFlagMax() );

   // MountedMask
   if ( stream->readFlag() ) 
   {
      if ( stream->readFlag() ) 
      {
         S32 gIndex = stream->readInt( NetConnection::GhostIdBitSize );
         SceneObject* obj = dynamic_cast<SceneObject*>( conn->resolveGhost( gIndex ) );
         S32 node = -1;
         if ( stream->readFlag() ) // node != -1
            node = stream->readInt( NumMountPointBits );
         MatrixF xfm;
         mathRead( *stream, &xfm );
         if ( !obj )
         {
            conn->setLastError( "Invalid packet from server." );
            return;
         }
         obj->mountObject( this, node, xfm );
      }
      else
         unmount();
   }
}

//-----------------------------------------------------------------------------

void SceneObject::_updateZoningState() const
{
   if( mZoneRefDirty )
   {
      SceneZoneSpaceManager* manager = getSceneManager()->getZoneManager();
      if( manager )
         manager->updateObject( const_cast< SceneObject* >( this ) );
      else
         mZoneRefDirty = false;
   }
}

//-----------------------------------------------------------------------------

U32 SceneObject::_getCurrZone( const U32 index ) const
{
   _updateZoningState();

   // Not the most efficient way to do this, walking the list,
   //  but it's an uncommon call...
   ZoneRef* walk = mZoneRefHead;
   for( U32 i = 0; i < index; ++ i )
   {
      walk = walk->nextInObj;
      AssertFatal( walk != NULL, "SceneObject::_getCurrZone - Too few object refs!" );
   }
   AssertFatal( walk != NULL, "SceneObject::_getCurrZone - Too few object refs!" );

   return walk->zone;
}

//-----------------------------------------------------------------------------

Point3F SceneObject::getPosition() const
{
   Point3F pos;
   mObjToWorld.getColumn(3, &pos);
   return pos;
}

//-----------------------------------------------------------------------------

Point3F SceneObject::getRenderPosition() const
{
   Point3F pos;
   mRenderObjToWorld.getColumn(3, &pos);
   return pos;
}

//-----------------------------------------------------------------------------

void SceneObject::setPosition(const Point3F &pos)
{
	AssertFatal( !mIsNaN( pos ), "SceneObject::setPosition() - The position is NaN!" );

   MatrixF xform = mObjToWorld;
   xform.setColumn(3, pos);
   setTransform(xform);
}

//-----------------------------------------------------------------------------

F32 SceneObject::distanceTo(const Point3F &pnt) const
{
   return mWorldBox.getDistanceToPoint( pnt );   
}

//-----------------------------------------------------------------------------

void SceneObject::processAfter( ProcessObject *obj )
{
   AssertFatal( dynamic_cast<SceneObject*>( obj ), "SceneObject::processAfter - Got non-SceneObject!" );

   mAfterObject = (SceneObject*)obj;
   if ( mAfterObject->mAfterObject == this )
      mAfterObject->mAfterObject = NULL;

   getProcessList()->markDirty();
}

//-----------------------------------------------------------------------------

void SceneObject::clearProcessAfter()
{
   mAfterObject = NULL;
}

//-----------------------------------------------------------------------------

void SceneObject::setProcessTick( bool t )
{
   if ( t == mProcessTick )
      return;

   if ( mProcessTick )
   {
      plUnlink();
      mProcessTick = false;
   }
   else
   {
      // Just to be sure...
      plUnlink();

      getProcessList()->addObject( this );

      mProcessTick = true;  
   }   
}

//-----------------------------------------------------------------------------

ProcessList* SceneObject::getProcessList() const
{
   if ( isClientObject() )      
      return ClientProcessList::get();
   else
      return ServerProcessList::get();
}

//-------------------------------------------------------------------------

bool SceneObject::isMounted()
{
   resolveMountPID();

   return mMount.object != NULL;
}

//-----------------------------------------------------------------------------

S32 SceneObject::getMountedObjectCount()
{
   S32 count = 0;
   for (SceneObject* itr = mMount.list; itr; itr = itr->mMount.link)
      count++;
   return count;
}

//-----------------------------------------------------------------------------

SceneObject* SceneObject::getMountedObject(S32 idx)
{
   if (idx >= 0) {
      S32 count = 0;
      for (SceneObject* itr = mMount.list; itr; itr = itr->mMount.link)
         if (count++ == idx)
            return itr;
   }
   return NULL;
}

//-----------------------------------------------------------------------------

S32 SceneObject::getMountedObjectNode(S32 idx)
{
   if (idx >= 0) {
      S32 count = 0;
      for (SceneObject* itr = mMount.list; itr; itr = itr->mMount.link)
         if (count++ == idx)
            return itr->mMount.node;
   }
   return -1;
}

//-----------------------------------------------------------------------------

SceneObject* SceneObject::getMountNodeObject(S32 node)
{
   for (SceneObject* itr = mMount.list; itr; itr = itr->mMount.link)
      if (itr->mMount.node == node)
         return itr;
   return NULL;
}

//-----------------------------------------------------------------------------

bool SceneObject::_setMountPID( void* object, const char* index, const char* data )
{
   SceneObject* so = static_cast<SceneObject*>( object );
   if ( so )
   {
      // Unmount old object (PID reference is released even if it had been resolved yet)
      if ( so->mMountPID )
      {
         so->mMountPID->decRefCount();
         so->mMountPID = NULL;
      }
      so->unmount();

      // Get the new PID (new object will be mounted on demand)
      Con::setData( TypePID, &so->mMountPID, 0, 1, &data );
      if ( so->mMountPID )
         so->mMountPID->incRefCount();    // Prevent PID from being deleted out from under us!
   }
   return false;
}

void SceneObject::resolveMountPID()
{
   if ( mMountPID && !mMount.object )
   {
      SceneObject *obj = dynamic_cast< SceneObject* >( mMountPID->getObject() );
      if ( obj )
         obj->mountObject( this, mMount.node, mMount.xfm );
   }
}

//-----------------------------------------------------------------------------

void SceneObject::mountObject( SceneObject *obj, S32 node, const MatrixF &xfm )
{
   if ( obj->mMount.object == this )
   {
      // Already mounted to this
      // So update our node and xfm which may have changed.
      obj->mMount.node = node;
      obj->mMount.xfm = xfm;
   }
   else
   {
      if ( obj->mMount.object )
         obj->unmount();

      obj->mMount.object = this;
      obj->mMount.node = node;
      obj->mMount.link = mMount.list;
      obj->mMount.xfm = xfm;
      mMount.list = obj;

      // Assign PIDs to both objects
      if ( isServerObject() )
      {
         obj->getOrCreatePersistentId();
         if ( !obj->mMountPID )
         {
            obj->mMountPID = getOrCreatePersistentId();
            obj->mMountPID->incRefCount();
         }
      }

      obj->onMount( this, node );
   }
}

//-----------------------------------------------------------------------------

void SceneObject::unmountObject( SceneObject *obj )
{
   if ( obj->mMount.object == this ) 
   {
      // Find and unlink the object
      for ( SceneObject **ptr = &mMount.list; *ptr; ptr = &(*ptr)->mMount.link )
      {
         if ( *ptr == obj )
         {
            *ptr = obj->mMount.link;
            break;
         }
      }

      obj->mMount.object = NULL;
      obj->mMount.link = NULL;

      if( obj->mMountPID != NULL ) // Only on server.
      {
         obj->mMountPID->decRefCount();
         obj->mMountPID = NULL;
      }

      obj->onUnmount( this, obj->mMount.node );
   }
}

//-----------------------------------------------------------------------------

void SceneObject::unmount()
{
   if (mMount.object)
      mMount.object->unmountObject(this);
}

//-----------------------------------------------------------------------------

void SceneObject::onMount( SceneObject *obj, S32 node )
{   
   deleteNotify( obj );

   if ( !isGhost() ) 
   {      
      setMaskBits( MountedMask );      
      //onMount_callback( node );
   }
}

//-----------------------------------------------------------------------------

void SceneObject::onUnmount( SceneObject *obj, S32 node )
{
   clearNotify(obj);

   if ( !isGhost() ) 
   {           
      setMaskBits( MountedMask );      
      //onUnmount_callback( node );
   }
}

//-----------------------------------------------------------------------------

void SceneObject::getMountTransform( S32 index, const MatrixF &xfm, MatrixF *outMat )
{
   MatrixF mountTransform( xfm );
   const Point3F &scale = getScale();
   Point3F position = mountTransform.getPosition();
   position.convolve( scale );
   mountTransform.setPosition( position );

   outMat->mul( mObjToWorld, mountTransform );
}

//-----------------------------------------------------------------------------

void SceneObject::getRenderMountTransform( F32 delta, S32 index, const MatrixF &xfm, MatrixF *outMat )
{
   MatrixF mountTransform( xfm );
   const Point3F &scale = getScale();
   Point3F position = mountTransform.getPosition();
   position.convolve( scale );
   mountTransform.setPosition( position );

   outMat->mul( mRenderObjToWorld, mountTransform );
}

//=============================================================================
//    Console API.
//=============================================================================
// MARK: ---- Console API ----

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getType, S32, (),,
   "Return the type mask for this object.\n"
   "@return The numeric type mask for the object." )
{
   return object->getTypeMask();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, mountObject, bool,
   ( SceneObject* objB, S32 slot, TransformF txfm ), ( MatrixF::Identity ),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)" )
{
   if ( objB )
   {
      object->mountObject( objB, slot, txfm.getMatrix() );
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, unmountObject, bool, ( SceneObject* target ),,
   "@brief Unmount an object from ourselves.\n\n"

   "@param target object to unmount\n"
   "@return true if successful, false if failed\n" )
{
   if ( target )
   {
      object->unmountObject(target);
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, unmount, void, (),,
   "Unmount us from the currently mounted object if any.\n" )
{
   object->unmount();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, isMounted, bool, (),,
   "@brief Check if we are mounted to another object.\n\n"
   "@return true if mounted to another object, false if not mounted." )
{
   return object->isMounted();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getObjectMount, S32, (),,
   "@brief Get the object we are mounted to.\n\n"
   "@return the SimObjectID of the object we're mounted to, or 0 if not mounted." )
{
   return object->isMounted()? object->getObjectMount()->getId(): 0;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getMountedObjectCount, S32, (),,
   "Get the number of objects mounted to us.\n"
   "@return the number of mounted objects." )
{
   return object->getMountedObjectCount();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getMountedObject, S32, ( S32 slot ),,
   "Get the object mounted at a particular slot.\n"
   "@param slot mount slot index to query\n"
   "@return ID of the object mounted in the slot, or 0 if no object." )
{
   SceneObject* mobj = object->getMountedObject( slot );
   return mobj? mobj->getId(): 0;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getMountedObjectNode, S32, ( S32 slot ),,
   "@brief Get the mount node index of the object mounted at our given slot.\n\n"
   "@param slot mount slot index to query\n"
   "@return index of the mount node used by the object mounted in this slot." )
{
   return object->getMountedObjectNode( slot );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getMountNodeObject, S32, ( S32 node ),,
   "@brief Get the object mounted at our given node index.\n\n"
   "@param node mount node index to query\n"
   "@return ID of the first object mounted at the node, or 0 if none found." )
{
   SceneObject* mobj = object->getMountNodeObject( node );
   return mobj? mobj->getId(): 0;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getTransform, TransformF, (),,
   "Get the object's transform.\n"
   "@return the current transform of the object\n" )
{
   return object->getTransform();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getInverseTransform, TransformF, (),,
   "Get the object's inverse transform.\n"
   "@return the inverse transform of the object\n" )
{
   return object->getWorldTransform();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getPosition, Point3F, (),,
   "Get the object's world position.\n"
   "@return the current world position of the object\n" )
{
   return object->getTransform().getPosition();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getEulerRotation, Point3F, (),,
   "Get Euler rotation of this object.\n"
   "@return the orientation of the object in the form of rotations around the "
   "X, Y and Z axes in degrees.\n" )
{
   Point3F euler = object->getTransform().toEuler();
   
   // Convert to degrees.
   euler.x = mRadToDeg( euler.x );
   euler.y = mRadToDeg( euler.y );
   euler.z = mRadToDeg( euler.z );
   
   return euler;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getForwardVector, VectorF, (),,
   "Get the direction this object is facing.\n"
   "@return a vector indicating the direction this object is facing.\n"
   "@note This is the object's y axis." )
{
   return object->getTransform().getForwardVector();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getRightVector, VectorF, (),,
   "Get the right vector of the object.\n"
   "@return a vector indicating the right direction of this object."
   "@note This is the object's x axis." )
{
   return object->getTransform().getRightVector();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getUpVector, VectorF, (),,
   "Get the up vector of the object.\n"
   "@return a vector indicating the up direction of this object."
   "@note This is the object's z axis." )
{
   return object->getTransform().getUpVector();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, setTransform, void, ( TransformF txfm ),,
   "Set the object's transform (orientation and position)."
   "@param txfm object transform to set" )
{
   if ( !txfm.hasRotation() )
      object->setPosition( txfm.getPosition() );
   else
      object->setTransform( txfm.getMatrix() );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getScale, Point3F, (),,
   "Get the object's scale.\n"
   "@return object scale as a Point3F" )
{
   return object->getScale();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, setScale, void, ( Point3F scale ),,
   "Set the object's scale.\n"
   "@param scale object scale to set\n" )
{
   object->setScale( scale );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getWorldBox, Box3F, (),,
   "Get the object's world bounding box.\n"
   "@return six fields, two Point3Fs, containing the min and max points of the "
   "worldbox." )
{
   return object->getWorldBox();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getWorldBoxCenter, Point3F, (),,
   "Get the center of the object's world bounding box.\n"
   "@return the center of the world bounding box for this object." )
{
   Point3F center;
   object->getWorldBox().getCenter( &center );
   return center;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, getObjectBox, Box3F, (),,
   "Get the object's bounding box (relative to the object's origin).\n"
   "@return six fields, two Point3Fs, containing the min and max points of the "
   "objectbox." )
{
   return object->getObjBox();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SceneObject, isGlobalBounds, bool, (),,
   "Check if this object has a global bounds set.\n"
   "If global bounds are set to be true, then the object is assumed to have an "
   "infinitely large bounding box for collision and rendering purposes.\n"
   "@return true if the object has a global bounds." )
{
   return object->isGlobalBounds();
}
