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
#include "scene/zones/sceneSimpleZone.h"

#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "scene/zones/sceneTraversalState.h"
#include "scene/culling/sceneCullingVolume.h"
#include "core/stream/bitStream.h"
#include "console/engineAPI.h"
#include "platform/profiler.h"


extern bool gEditingMission;


//-----------------------------------------------------------------------------

SceneSimpleZone::SceneSimpleZone()
   : mUseAmbientLightColor( false ),
     mAmbientLightColor( 0.1f, 0.1f, 0.1f, 1.f ),
     mIsRotated( false )
{
   // Box zones are unit cubes that are scaled to fit.

   mObjScale.set( 10, 10, 10 );
   mObjBox.set(
      Point3F( -0.5f, -0.5f, -0.5f ),
      Point3F( 0.5f, 0.5f, 0.5f )
   );
}

//-----------------------------------------------------------------------------

void SceneSimpleZone::initPersistFields()
{
   addGroup( "Lighting" );

      addProtectedField( "useAmbientLightColor", TypeBool, Offset( mUseAmbientLightColor, SceneSimpleZone ),
         &_setUseAmbientLightColor, &defaultProtectedGetFn,
         "Whether to use #ambientLightColor for ambient lighting in this zone or the global ambient color." );
      addProtectedField( "ambientLightColor", TypeColorF, Offset( mAmbientLightColor, SceneSimpleZone ),
         &_setAmbientLightColor, &defaultProtectedGetFn,
         "Color of ambient lighting in this zone.\n\n"
         "Only used if #useAmbientLightColor is true." );

   endGroup( "Lighting" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

String SceneSimpleZone::describeSelf() const
{
   String str = Parent::describeSelf();
   str += String::ToString( "|zoneid: %i", getZoneRangeStart() );
   return str;
}

//-----------------------------------------------------------------------------

bool SceneSimpleZone::onSceneAdd()
{
   if( !Parent::onSceneAdd() )
      return false;

   // Register our zone.

   SceneZoneSpaceManager* manager = getSceneManager()->getZoneManager();
   if( manager )
      manager->registerZones( this, 1 );

   return true;
}

//-----------------------------------------------------------------------------

U32 SceneSimpleZone::packUpdate( NetConnection* connection, U32 mask, BitStream* stream )
{
   U32 retMask = Parent::packUpdate( connection, mask, stream );

   if( stream->writeFlag( mask & AmbientMask ) )
   {
      stream->writeFlag( mUseAmbientLightColor );
      stream->writeFloat( mAmbientLightColor.red, 7 );
      stream->writeFloat( mAmbientLightColor.green, 7 );
      stream->writeFloat( mAmbientLightColor.blue, 7 );
      stream->writeFloat( mAmbientLightColor.alpha, 7 );
   }

   return retMask;
}

//-----------------------------------------------------------------------------

void SceneSimpleZone::unpackUpdate( NetConnection* connection, BitStream* stream )
{
   Parent::unpackUpdate( connection, stream );

   if( stream->readFlag() ) // AmbientMask
   {
      mUseAmbientLightColor = stream->readFlag();
      mAmbientLightColor.red = stream->readFloat( 7 );
      mAmbientLightColor.green = stream->readFloat( 7 );
      mAmbientLightColor.blue = stream->readFloat( 7 );
      mAmbientLightColor.alpha = stream->readFloat( 7 );
   }
}

//-----------------------------------------------------------------------------

void SceneSimpleZone::setUseAmbientLightColor( bool value )
{
   if( mUseAmbientLightColor == value )
      return;

   mUseAmbientLightColor = value;
   if( isServerObject() )
      setMaskBits( AmbientMask );
}

//-----------------------------------------------------------------------------

void SceneSimpleZone::setAmbientLightColor( const ColorF& color )
{
   mAmbientLightColor = color;
   if( isServerObject() )
      setMaskBits( AmbientMask );
}

//-----------------------------------------------------------------------------

bool SceneSimpleZone::getZoneAmbientLightColor( U32 zone, ColorF& outColor ) const
{
   AssertFatal( zone == getZoneRangeStart(), "SceneSimpleZone::getZoneAmbientLightColor - Invalid zone ID!" );

   if( !mUseAmbientLightColor )
      return false;

   outColor = mAmbientLightColor;
   return true;
}

//-----------------------------------------------------------------------------

U32 SceneSimpleZone::getPointZone( const Point3F& p )
{
   if( !containsPoint( p ) )
      return SceneZoneSpaceManager::InvalidZoneId;

   return getZoneRangeStart();
}

//-----------------------------------------------------------------------------

bool SceneSimpleZone::getOverlappingZones( const Box3F& aabb, U32* outZones, U32& outNumZones )
{
   PROFILE_SCOPE( SceneBoxZone_getOverlappingZones );

   bool isOverlapped = false;
   bool isContained = false;

   // If the zone has not been rotated, we can simply use straightforward
   // AABB/AABB intersection based on the world boxes of the zone and the
   // object.
   //
   // If, however, the zone has been rotated, then we must use the zone's
   // OBB and test that against the object's AABB.

   if( !mIsRotated )
   {
      isOverlapped = mWorldBox.isOverlapped( aabb );
      isContained = isOverlapped && mWorldBox.isContained( aabb );
   }
   else
   {
      // Check if the zone's OBB intersects the object's AABB.

      isOverlapped = aabb.collideOrientedBox(
         getScale() / 2.f,
         getTransform()
      );

      // If so, check whether the object's AABB is fully contained
      // inside the zone's OBB.

      if( isOverlapped )
      {
         isContained = true;

         for( U32 i = 0; i < Box3F::NUM_POINTS; ++ i )
         {
            Point3F cornerPoint = aabb.computeVertex( i );
            if( !mOrientedWorldBox.isContained( cornerPoint ) )
            {
               isContained = false;
               break;
            }
         }
      }
   }

   if( isOverlapped )
   {
      outNumZones = 1;
      outZones[ 0 ] = getZoneRangeStart();
   }
   else
      outNumZones = 0;

   return !isContained;
}

//-----------------------------------------------------------------------------

void SceneSimpleZone::setTransform( const MatrixF& mat )
{
   Parent::setTransform( mat );

   // Find out whether the zone has been rotated.

   EulerF rotation = getTransform().toEuler();
   mIsRotated =   !mIsZero( rotation.x ) ||
                  !mIsZero( rotation.y ) ||
                  !mIsZero( rotation.z );

   // Update the OBB.

   _updateOrientedWorldBox();
}

//-----------------------------------------------------------------------------

void SceneSimpleZone::prepRenderImage( SceneRenderState* state )
{
   if( isRootZone() )
      return;

   Parent::prepRenderImage( state );
}

//-----------------------------------------------------------------------------

void SceneSimpleZone::traverseZones( SceneTraversalState* state )
{
   traverseZones( state, getZoneRangeStart() );
}

//-----------------------------------------------------------------------------

void SceneSimpleZone::traverseZones( SceneTraversalState* state, U32 startZoneId )
{
   PROFILE_SCOPE( SceneSimpleZone_traverseZones );
   AssertFatal( startZoneId == getZoneRangeStart(), "SceneSimpleZone::traverseZones - Invalid start zone ID!" );

   // If we aren't the root of the traversal, do a number of checks
   // to see if we can early out of the traversal here.  The primary reason
   // we don't do the checks if we are the root is because of the frustum
   // near plane.  The start zone of a traversal is selected based on the
   // current viewpoint.  However, that point still has some space in between
   // it and the near plane so if we end up with a case where that's all the
   // space that is needed to cull away our starting zone, we won't see any
   // traversal at all and get a blank scene back even if the starting zone
   // would actually hand the traversal over to other zones and eventually
   // discover visible space.

   if( state->getTraversalDepth() > 0 )
   {
      // If we have already visited this zone in this traversal chain,
      // exit out.  This can happen when zones are grouped together.
      // Note that this can also happen with the outdoor zone since it isn't
      // convex.  However, in that case, this acts as a nice side optimization
      // we get since if the outdoor zone is already on the stack, our culling
      // culling volume can only be smaller than the one we started with and thus
      // by earlying out here, we prevent adding a pointless culling volume
      // to the zone.

      //TODO: would be nice to catch this via "visibility changed?" checks but
      //    that's non-trivial

      if( state->haveVisitedZone( getZoneRangeStart() ) )
         return;

      // First check whether we even intersect the given frustum at all.

      if( mIsRotated )
      {
         // Space has been rotated, so do a frustum/OBB check.
         if( !state->getCurrentCullingVolume().test( _getOrientedWorldBox() ) )
            return;
      }
      else
      {
         // Space has not been rotated, so we can do a faster frustum/ABB check.
         if( !state->getCurrentCullingVolume().test( getWorldBox() ) )
            return;
      }
   }

   // Add the current culling volume to the culling state for this zone.
   // If that doesn't result in new space becoming visible, we can terminate the traversal
   // here.

   if( !state->getCullingState()->addCullingVolumeToZone( startZoneId, state->getCurrentCullingVolume() ) )
      return;

   // Add our occluders to the rendering state.

   _addOccludersToCullingState( state->getCullingState() );

   // Merge the zone into the traversal area.

   state->addToTraversedArea( getWorldBox() );

   // Push our zone ID on the traversal stack and traverse into our
   // connected zone managers.

   state->pushZone( startZoneId );
   _traverseConnectedZoneSpaces( state );
   state->popZone();
}

//-----------------------------------------------------------------------------

bool SceneSimpleZone::_setUseAmbientLightColor( void* object, const char* index, const char* data )
{
   SceneSimpleZone* zone = reinterpret_cast< SceneSimpleZone* >( object );
   zone->setUseAmbientLightColor( EngineUnmarshallData< bool >()( data ) );
   return false;
}

//-----------------------------------------------------------------------------

bool SceneSimpleZone::_setAmbientLightColor( void* object, const char* index, const char* data )
{
   SceneSimpleZone* zone = reinterpret_cast< SceneSimpleZone* >( object );
   zone->setAmbientLightColor( EngineUnmarshallData< ColorF >()( data ) );
   return false;
}
