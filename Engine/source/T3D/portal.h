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

#ifndef _PORTAL_H_
#define _PORTAL_H_

#ifndef _ZONE_H_
#include "T3D/zone.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif


class SceneCullingState;
class SceneCullingVolume;


/// A transitioning zone that connects other zones.
///
/// Basically a portal is two things:
///
/// 1) A zone that overlaps multiple other zones and thus connects them.
/// 2) A polygon standing upright in the middle of the portal zone's world box.
///
/// When traversing from zone to zone, portals serve as both zones in their own
/// right (i.e. objects may be located in a portal zone) as well as a peek hole
/// that determines what area of a target zone is visible through a portal.
///
/// Torque's portals are special in that they are two-way by default.  This greatly
/// simplifies zone setups but it also complicates handling in the engine somewhat.
/// Also, these portals here are nothing but peek holes--they do not define transform
/// portals that could be looking at a different location in space altogether.
///
/// Portals can be marked explicitly as being one-sided by flagging either of the portal's
/// sides as impassable.  This flagging can also be used dynamically to, for example, block
/// a portal while a door is still down and then unblock the portal when the door is
/// opened.
///
/// Portals are classified as either interior or exterior portals.  An exterior portal is
/// a portal that has only non-SceneRootZone zones on side of the portal plane and only the
/// SceneRootZone on the other side of it.  An interior portal is a portal that has only
/// non-SceneRootZone zones on both sides of the portal plane.  A mixture of the two is not
/// allowed &ndash; when adding SceneRootZone to a portal, it must exist alone on its portal
/// side.
class Portal : public Zone
{
   public:

      typedef Zone Parent;

      /// Identifies the subspaces defined by the portal plane.
      enum Side
      {
         FrontSide,           ///< Subspace on front side of portal plane.
         BackSide             ///< Subspace on back side of portal plane.
      };

      /// Identifies the type of portal.
      enum Classification
      {
         InvalidPortal,       ///< Portal does not connect anything.
         InteriorPortal,      ///< Portal between interior zones.
         ExteriorPortal       ///< Portal between interior zones on one and side and SceneRootZone on the other.
      };

   protected:
      
      enum
      {
         PassableMask = Parent::NextFreeMask << 0,    ///< #mPassableSides has changed.
         NextFreeMask = Parent::NextFreeMask << 1,
      };

      /// Flags that allow preventing traversal through specific
      /// sides of the portal.  By default, both sides are passable.
      bool mPassableSides[ 2 ];

      /// @name Derived Portal Data
      /// @{

      /// Classification of this portal as interior or exterior portal.
      Classification mClassification;

      /// For exterior portals, this is the side of the portal on which
      /// the connected interior zones lie.
      Side mInteriorSide;

      /// Whether the portal plane and polygon need to be updated.
      bool mIsGeometryDirty;

      /// Portal polygon in world space.
      Vector< Point3F > mPortalPolygonWS;

      /// The plane defined by the portal's rectangle.
      PlaneF mPortalPlane;

      /// Update derived data, if necessary.
      void _update();

      /// Update the world space portal geometry.
      void _updateGeometry();

      /// Detect whether this is an exterior, interior, or invalid portal.
      void _updateConnectivity();

      /// @}

      /// Compute a clipped culling volume from the portal geometry and current
      /// traversal state.  If successful, store the resulting culling volume in
      /// @a outVolume and return true.
      bool _generateCullingVolume( SceneTraversalState* state, SceneCullingVolume& outVolume ) const;

      // SceneSpace.
      virtual void _renderObject( ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* overrideMat );
      virtual ColorI _getDefaultEditorSolidColor() const { return ColorI( 0, 255, 0, 45 ); }
      virtual ColorI _getDefaultEditorWireframeColor() const
      {
         switch( mClassification )
         {
            case ExteriorPortal: return ColorI( 0, 128, 128, 255 ); break;
            case InteriorPortal: return ColorI( 128, 128, 0, 255 );
            default:  return ColorI( 255, 255, 255, 255 ); break;
         }
      }

      // SceneObject.
      virtual void onSceneRemove();

      // SceneZoneSpace.
      virtual void _traverseConnectedZoneSpaces( SceneTraversalState* state );
      virtual void _disconnectAllZoneSpaces();

   public:

      Portal();

      /// Return what kind of portal this is (interior or exterior).
      Classification getClassification() const { return mClassification; }

      /// Return the plane that is defined by the portal's rectangle.
      const PlaneF& getPortalPlane() const { return mPortalPlane; }

      /// Return the side that the given point is in relative to the portal plane.
      Side getSideRelativeToPortalPlane( const Point3F& point ) const;

      /// Test whether the given side of the portal is open for traversal.
      bool isSidePassable( Side side ) const { return mPassableSides[ side ]; }

      /// Set whether the given portal side is passable.
      void setSidePassable( Side side, bool value );

      /// Return true if the portal leads to the outdoor zone.
      bool isExteriorPortal() const { return ( getClassification() == ExteriorPortal ); }

      /// Return true if the portal connects interior zones only.
      bool isInteriorPortal() const { return ( getClassification() == InteriorPortal ); }

      /// For exterior portals, get the side on which the interior zones of the portal lie.
      Side getInteriorSideOfExteriorPortal() const
      {
         AssertFatal( isExteriorPortal(), "Portal::getInteriorSideOfExteriorPortal - Not an exterior portal!" );
         return mInteriorSide;
      }

      // SimObject.
      DECLARE_CONOBJECT( Portal );
      
      static void initPersistFields();
      static void consoleInit();

      virtual bool writeField( StringTableEntry fieldName, const char* value );
      virtual String describeSelf() const;

      // NetObject.
      virtual U32 packUpdate( NetConnection* conn, U32 mask, BitStream* stream );
      virtual void unpackUpdate( NetConnection* conn, BitStream* stream );

      // SceneObject.
      virtual void setTransform( const MatrixF &mat );

      // SceneZoneSpace.
      virtual void traverseZones( SceneTraversalState* state, U32 startZoneId );
      virtual void connectZoneSpace( SceneZoneSpace* zoneSpace );
      virtual void disconnectZoneSpace( SceneZoneSpace* zoneSpace );

   private:

      static bool _setFrontSidePassable( void* object, const char* index, const char* data );
      static bool _setBackSidePassable( void* object, const char* index, const char* data );
};

#endif // _PORTAL_H_
