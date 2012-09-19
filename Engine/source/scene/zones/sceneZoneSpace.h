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

#ifndef _SCENEZONESPACE_H_
#define _SCENEZONESPACE_H_

#ifndef _SCENESPACE_H_
#include "scene/sceneSpace.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif


class SceneZoneSpaceManager;
class SceneCullingState;


/// Abstract base class for an object that manages zones in a scene.
///
/// This class adds the ability to SceneSpace to define and manage zones within the object's
/// space.  Zones are used to determine visibility in a scene.
///
/// Each zone space manages one or more zones in a scene.  All the zones must be within the
/// AABB of the zone space but within that AABB, the zone space is free to distribute and
/// manage zones in arbitrary fashion.
///
/// For scene traversal, zone spaces are interconnected.  By default, zone spaces get a chance
/// to connect to each other when being moved into each other's zones.  An exception to this
/// is the root zone since it is both immobile and without position and (limited) extents.  If
/// a zone space wants to connect to the root zone, it must do so manually (same goes for
/// disconnecting).
class SceneZoneSpace : public SceneSpace
{
   public:

      typedef SceneSpace Parent;

      friend class SceneZoneSpaceManager;
      friend class SceneRootZone; // mZoneFlags, connectZoneSpace, disconnectZoneSpace

      enum
      {
         InvalidZoneGroup = 0
      };

      enum ZoneFlags
      {
         /// Whether this zoning space is "closed off" or not.  When connections between
         /// zoning spaces are established, by default only spaces that are closed off are
         /// connected to those that are *not* and vice versa.  This defines a natural progression
         /// where spaces that explicitly want to propagate traversals are connected to those
         /// that want to contain it.
         ///
         /// This flag is set by default.
         ZoneFlag_IsClosedOffSpace = BIT( 0 ),
      };

   protected:

      enum
      {
         ZoneGroupMask = Parent::NextFreeMask << 0,
         NextFreeMask = Parent::NextFreeMask << 1
      };

      /// The manager to which this zone space is registered.
      SceneZoneSpaceManager* mManager;

      /// ID of first zone defined by object.
      U32 mZoneRangeStart;

      /// Number of zones managed by #obj.  IDs are consecutive
      /// starting with #mZoneRangeStart.
      U32 mNumZones;

      /// Group which the zone is assigned to.  Zone spaces that would normally not connect
      /// do connect if they are assigned the same zone group.  O to disable (default).
      U32 mZoneGroup;

      ///
      BitSet32 mZoneFlags;
      
      /// @name Occluders
      ///
      /// Zone spaces keep track of the occluders that get added to them so that during
      /// traversal, they can be taken on board as early as possible.  This allows traversals
      /// themselves to take occlusion into account.
      ///
      /// @{

      /// Occluders in this space.  Every object that is assigned to this space
      /// and has the OccluderObjectType flag set, is added to this list.
      Vector< SceneObject* > mOccluders;

      /// Add the given object to the list of occluders in this zone space.
      void _addOccluder( SceneObject* object );

      /// Remove the given object from the list of occluders in this zone space.
      void _removeOccluder( SceneObject* object );

      /// Register the occluders in this zone with the given culling state.
      void _addOccludersToCullingState( SceneCullingState* state ) const;

      /// @}

      /// @name Zone Space Connectivity
      /// @{

      //TODO: we should have both automatic and manual connections; only automatic connections
      // should get reset when a zone space moves

      /// Link to another zone space.
      struct ZoneSpaceRef
      {
         // We could be storing zone IDs here to connect individual zones rather than just
         // the managers but since no one requires this at the moment, the code doesn't do it.
         // However, it's trivial to add.

         SceneZoneSpace* mZoneSpace;
         ZoneSpaceRef* mNext;
      };

      /// List of zone spaces that this space is connected to.  This is used
      /// for traversals.
      ZoneSpaceRef* mConnectedZoneSpaces;

      /// Allocator for ZoneSpaceRefs.
      static ClassChunker< ZoneSpaceRef > smZoneSpaceRefChunker;

      /// Disconnect all zone spaces currently connected to this space.
      virtual void _disconnectAllZoneSpaces();

      /// Hand the traversal over to connected zone spaces.
      virtual void _traverseConnectedZoneSpaces( SceneTraversalState* state );

      /// Return true if the given zone space, which has crossed into this space, should
      /// be automatically connected.  Note that event
      virtual bool _automaticallyConnectZoneSpace( SceneZoneSpace* zoneSpace ) const;

      /// @}

      /// @name SceneManager Notifications
      ///
      /// These methods are called when SceneManager assigns or removes objects to/from our zones.
      ///
      /// @{

      /// Called by the SceneManager when an object is added to one or more zones that
      /// are managed by this object.
      virtual void _onZoneAddObject( SceneObject* object, const U32* zoneIDs, U32 numZones );

      /// Called by the SceneManager when an object that was previously added to one or more
      /// zones managed by this object is now removed.
      virtual void _onZoneRemoveObject( SceneObject* object );

      /// @}

      // SceneObject.
      virtual void onSceneRemove();

   public:

      SceneZoneSpace();
      virtual ~SceneZoneSpace();

      /// Return true if this is the outdoor zone.
      bool isRootZone() const { return ( getZoneRangeStart() == 0 ); }

      /// Gets the index of the first zone this object manages in the collection of zones or 0xFFFFFFFF if the
      /// object is not managing zones.
      U32 getZoneRangeStart() const { return mZoneRangeStart; }

      /// Return the number of zones that are managed by this object.
      U32 getZoneRange() const { return mNumZones; }

      /// Return the zone group that this zone space belongs to.  0 by default which means the
      /// zone space is not allocated to a specific zone group.
      U32 getZoneGroup() const { return mZoneGroup; }

      /// Set the zone group of this zone space.  Zone spaces in the same group will connect even if
      /// not connecting by default.  Set to 0 to deactivate.
      void setZoneGroup( U32 group );

      /// Dump a listing of all objects assigned to this zone space to the console as well
      /// as a list of all connected spaces.
      ///
      /// @param update Whether to update the zone contents before dumping.  Since the zoning states of
      ///   SceneObjects are updated lazily, the contents of a zone can be outdated.
      void dumpZoneState( bool update = true );

      /// Get the ambient light color of the given zone in this space or return false if the
      /// given zone does not have an ambient light color assigned to it.
      virtual bool getZoneAmbientLightColor( U32 zone, ColorF& outColor ) const { return false; }

      /// @name Containment Tests
      /// @{

      ///
      virtual bool getOverlappingZones( const Box3F& aabb, U32* zones, U32& numZones ) = 0;

      /// Find the zones in this object that @a obj is part of.
      ///
      /// @param obj Object in question.
      /// @param outZones Indices of zones containing the object.  Must have at least as many entries
      ///   as there as zones in this object or SceneObject::MaxObjectZones, whichever is smaller.
      ///   Note that implementations should never write more than SceneObject::MaxObjectZones entries.
      /// @param outNumZones Number of elements in the returned array.
      ///
      /// @return Return true if the world box of @a obj is fully contained within the zones of this object or
      ///   false if it is at least partially outside of them.
      virtual bool getOverlappingZones( SceneObject* obj, U32* outZones, U32& outNumZones );

      /// Returns the ID of the zone that are managed by this object that contains @a p.
      /// @param p Point to test.
      /// @return ID of the zone containing @a p or InvalidZoneId if none of the zones defined by this
      ///   object contain the point.
      virtual U32 getPointZone( const Point3F& p ) = 0;

      /// @}

      /// @name Connectivity
      /// @{

      /// Connect this zone space to the given zone space.
      ///
      /// @param zoneSpace A zone space.
      ///
      /// @note Connectivity is reset when a zone space is moved!
      virtual void connectZoneSpace( SceneZoneSpace* zoneSpace );

      /// If the object is a zone space, then this method is called to instruct the object
      /// to remove any zone connectivity to @a zoneSpace.
      ///
      /// @param zoneSpace A zone space which had previously been passed to connectZoneSpace().
      virtual void disconnectZoneSpace( SceneZoneSpace* zoneSpace );

      /// @}

      /// @name Traversals
      /// @{

      /// Traverse into the zones of this space.  Set the render states of those zones and add the frustums
      /// that lead into them.
      ///
      /// The traversal stack is expected to not be empty and the topmost entry on the stack should be the
      /// zone of another manager from which traversal is handed over to this manager.
      ///
      /// @param state Scene traversal state.
      ///
      /// @note If the zone's of a zone space are reached via different traversal paths, this method
      ///   will be called multiple times on the same space.
      virtual void traverseZones( SceneTraversalState* state ) = 0;

      /// Traverse the zones in this space starting with the given zone.  Where appropriate, traversal
      /// should be handed off to connected spaces.
      ///
      /// @param state State which should be updated by the traversal.
      /// @param startZoneId ID of zone in this manager in which to start traversal.
      virtual void traverseZones( SceneTraversalState* state, U32 startZoneId ) = 0;

      /// @}

      // SimObject.
      virtual bool writeField( StringTableEntry fieldName, const char* value );

      static void initPersistFields();

      // NetObject.
      virtual U32 packUpdate( NetConnection* connection, U32 mask, BitStream* stream );
      virtual void unpackUpdate( NetConnection* connection, BitStream* stream );

   private:

      static bool _setZoneGroup( void* object, const char* index, const char* data );
};

#endif // !_SCENEZONESPACE_H_
