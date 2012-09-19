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
#include "scene/zones/sceneZoneSpace.h"

#include "scene/zones/sceneTraversalState.h"
#include "scene/zones/sceneZoneSpaceManager.h"
#include "scene/sceneRenderState.h"
#include "sim/netConnection.h"
#include "core/stream/bitStream.h"
#include "console/engineAPI.h"


//#define DEBUG_SPEW


ClassChunker< SceneZoneSpace::ZoneSpaceRef > SceneZoneSpace::smZoneSpaceRefChunker;


//-----------------------------------------------------------------------------

SceneZoneSpace::SceneZoneSpace()
   : mManager( NULL ),
     mZoneGroup( InvalidZoneGroup ),
     mZoneRangeStart( SceneZoneSpaceManager::InvalidZoneId ),
     mZoneFlags( ZoneFlag_IsClosedOffSpace ),
     mNumZones( 0 ),
     mConnectedZoneSpaces( NULL )
{
   VECTOR_SET_ASSOCIATION( mOccluders );
}

//-----------------------------------------------------------------------------

SceneZoneSpace::~SceneZoneSpace()
{
   AssertFatal( mConnectedZoneSpaces == NULL, "SceneZoneSpace::~SceneZoneSpace - Still have connected zone spaces!" );
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::onSceneRemove()
{
   _disconnectAllZoneSpaces();
   Parent::onSceneRemove();
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::initPersistFields()
{
   addGroup( "Zoning" );

      addProtectedField( "zoneGroup", TypeS32, Offset( mZoneGroup, SceneZoneSpace ),
         &_setZoneGroup, &defaultProtectedGetFn,
         "ID of group the zone is part of." );

   endGroup( "Zoning" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SceneZoneSpace::writeField( StringTableEntry fieldName, const char* value )
{
   // Don't write zoneGroup field if at default.
   static StringTableEntry sZoneGroup = StringTable->insert( "zoneGroup" );
   if( fieldName == sZoneGroup && getZoneGroup() == InvalidZoneGroup )
      return false;

   return Parent::writeField( fieldName, value );
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::setZoneGroup( U32 group )
{
   if( mZoneGroup == group )
      return;

   mZoneGroup = group;
   setMaskBits( ZoneGroupMask );

   // Rezone to establish new connectivity.

   if( mManager )
      mManager->notifyObjectChanged( this );
}

//-----------------------------------------------------------------------------

U32 SceneZoneSpace::packUpdate( NetConnection* connection, U32 mask, BitStream* stream )
{
   U32 retMask = Parent::packUpdate( connection, mask, stream );

   if( stream->writeFlag( mask & ZoneGroupMask ) )
      stream->write( mZoneGroup );

   return retMask;
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::unpackUpdate( NetConnection* connection, BitStream* stream )
{
   Parent::unpackUpdate( connection, stream );

   if( stream->readFlag() ) // ZoneGroupMask
   {
      U32 zoneGroup;
      stream->read( &zoneGroup );
      setZoneGroup( zoneGroup );
   }
}

//-----------------------------------------------------------------------------

bool SceneZoneSpace::getOverlappingZones( SceneObject* obj, U32* outZones, U32& outNumZones )
{
   return getOverlappingZones( obj->getWorldBox(), outZones, outNumZones );
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::_onZoneAddObject( SceneObject* object, const U32* zoneIDs, U32 numZones )
{
   if( object->isVisualOccluder() )
      _addOccluder( object );

   // If this isn't the root zone and the object is zone space,
   // see if we should automatically connect the two.
   
   if( !isRootZone() && object->getTypeMask() & ZoneObjectType )
   {
      SceneZoneSpace* zoneSpace = dynamic_cast< SceneZoneSpace* >( object );

      // Don't connect a zone space that has the same closed off status
      // that we have except it is assigned to the same zone group.

      if( zoneSpace &&
          ( zoneSpace->mZoneFlags.test( ZoneFlag_IsClosedOffSpace ) != mZoneFlags.test( ZoneFlag_IsClosedOffSpace ) ||
            ( zoneSpace->getZoneGroup() == getZoneGroup() &&
              zoneSpace->getZoneGroup() != InvalidZoneGroup ) ) &&
          _automaticallyConnectZoneSpace( zoneSpace ) )
      {
         connectZoneSpace( zoneSpace );
      }
   }
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::_onZoneRemoveObject( SceneObject* object )
{
   if( object->isVisualOccluder() )
      _removeOccluder( object );

   if( !isRootZone() && object->getTypeMask() & ZoneObjectType )
   {
      SceneZoneSpace* zoneSpace = dynamic_cast< SceneZoneSpace* >( object );
      if( zoneSpace )
         disconnectZoneSpace( zoneSpace );
   }
}

//-----------------------------------------------------------------------------

bool SceneZoneSpace::_automaticallyConnectZoneSpace( SceneZoneSpace* zoneSpace ) const
{
   //TODO: This is suboptimal.  While it prevents the most blatantly wrong automatic connections,
   //  we need a true polyhedron/polyhedron intersection to accurately determine zone intersection
   //  when it comes to automatic connections.

   U32 numZones = 0;
   U32 zones[ SceneObject::MaxObjectZones ];

   zoneSpace->getOverlappingZones( getWorldBox(), zones, numZones );
   
   return ( numZones > 0 );
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::connectZoneSpace( SceneZoneSpace* zoneSpace )
{
   // If the zone space is already in the list, do nothing.

   for( ZoneSpaceRef* ref = mConnectedZoneSpaces; ref != NULL; ref = ref->mNext )
      if( ref->mZoneSpace == zoneSpace )
         return;

   // Link the zone space to the zone space refs.

   ZoneSpaceRef* ref = smZoneSpaceRefChunker.alloc();

   ref->mZoneSpace = zoneSpace;
   ref->mNext = mConnectedZoneSpaces;

   mConnectedZoneSpaces = ref;

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SceneZoneSpace] Connecting %i-%i to %i-%i",
      getZoneRangeStart(), getZoneRangeStart() + getZoneRange(),
      zoneSpace->getZoneRangeStart(), zoneSpace->getZoneRangeStart() + zoneSpace->getZoneRange()
   );
   #endif
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::disconnectZoneSpace( SceneZoneSpace* zoneSpace )
{
   ZoneSpaceRef* prev = NULL;
   for( ZoneSpaceRef* ref = mConnectedZoneSpaces; ref != NULL; prev = ref, ref = ref->mNext )
      if( ref->mZoneSpace == zoneSpace )
      {
         if( prev )
            prev->mNext = ref->mNext;
         else
            mConnectedZoneSpaces = ref->mNext;

         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[SceneZoneSpace] Disconnecting %i-%i from %i-%i",
            getZoneRangeStart(), getZoneRangeStart() + getZoneRange(),
            zoneSpace->getZoneRangeStart(), zoneSpace->getZoneRangeStart() + zoneSpace->getZoneRange()
         );
         #endif

         smZoneSpaceRefChunker.free( ref );
         break;
      }
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::_disconnectAllZoneSpaces()
{
   #ifdef DEBUG_SPEW
   if( mConnectedZoneSpaces != NULL )
      Platform::outputDebugString( "[SceneZoneSpace] Disconnecting all from %i-%i",
         getZoneRangeStart(), getZoneRangeStart() + getZoneRange()
      );
   #endif
   
   for( ZoneSpaceRef* ref = mConnectedZoneSpaces; ref != NULL; )
   {
      ZoneSpaceRef* next = ref->mNext;
      smZoneSpaceRefChunker.free( ref );
      ref = next;
   }
   mConnectedZoneSpaces = NULL;
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::_addOccluder( SceneObject* object )
{
   AssertFatal( !mOccluders.contains( object ), "SceneZoneSpace::_addOccluder - Occluder already added to this zone space!" );
   mOccluders.push_back( object );
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::_removeOccluder( SceneObject* object )
{
   const U32 numOccluders = mOccluders.size();
   for( U32 i = 0; i < numOccluders; ++ i )
      if( mOccluders[ i ] == object )
      {
         mOccluders.erase_fast( i );
         break;
      }

   AssertFatal( !mOccluders.contains( object ), "SceneZoneSpace::_removeOccluder - Occluder still added to this zone space!" );
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::_addOccludersToCullingState( SceneCullingState* state ) const
{
   const U32 numOccluders = mOccluders.size();
   for( U32 i = 0; i < numOccluders; ++ i )
      state->addOccluder( mOccluders[ i ] );
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::_traverseConnectedZoneSpaces( SceneTraversalState* state )
{
   // Hand the traversal over to all connected zone spaces.

   for( ZoneSpaceRef* ref = mConnectedZoneSpaces; ref != NULL; ref = ref->mNext )
   {
      SceneZoneSpace* zoneSpace = ref->mZoneSpace;
      zoneSpace->traverseZones( state );
   }
}

//-----------------------------------------------------------------------------

void SceneZoneSpace::dumpZoneState( bool update )
{
   // Nothing to dump if not registered.

   if( !mManager )
      return;

   // If we should update, trigger rezoning for the space
   // we occupy.

   if( update )
      mManager->_rezoneObjects( getWorldBox() );

   Con::printf( "====== Zones in: %s =====", describeSelf().c_str() );

   // Dump connections.

   for( ZoneSpaceRef* ref = mConnectedZoneSpaces; ref != NULL; ref = ref->mNext )
      Con::printf( "Connected to: %s", ref->mZoneSpace->describeSelf().c_str() );

   // Dump objects.

   for( U32 i = 0; i < getZoneRange(); ++ i )
   {
      U32 zoneId = getZoneRangeStart() + i;

      Con::printf( "--- Zone %i", zoneId );

      for( SceneZoneSpaceManager::ZoneContentIterator iter( mManager, zoneId, false ); iter.isValid(); ++ iter )
         Con::printf( iter->describeSelf() );
   }
}

//-----------------------------------------------------------------------------

bool SceneZoneSpace::_setZoneGroup( void* object, const char* index, const char* data )
{
   SceneZoneSpace* zone = reinterpret_cast< SceneZoneSpace* >( object );
   zone->setZoneGroup( EngineUnmarshallData< S32 >()( data ) );
   return false;
}
