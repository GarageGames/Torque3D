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
#include "T3D/portal.h"

#include "core/stream/bitStream.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "scene/zones/sceneRootZone.h"
#include "scene/culling/sceneCullingState.h"
#include "scene/zones/sceneTraversalState.h"
#include "math/mPlaneSet.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"

#include "scene/mixin/sceneAmbientSoundObject.impl.h"
#include "scene/mixin/scenePolyhedralObject.impl.h"
#include "math/mPolyhedron.impl.h"


IMPLEMENT_CO_NETOBJECT_V1( Portal );

ConsoleDocClass( Portal,
   "@brief An object that provides a \"window\" into a zone, allowing a viewer "
      "to see what's rendered in the zone.\n\n"

   "A portal is an object that connects zones such that the content of one zone becomes "
   "visible in the other when looking through the portal.\n\n"

   "Each portal is a full zone which is divided into two sides by the portal plane that "
   "intersects it.  This intersection polygon is shown in red in the editor.  Either of the "
   "sides of a portal can be connected to one or more zones.\n\n"

   "A connection from a specific portal side to a zone is made in either of two ways:\n\n"

   "<ol>\n"
   "<li>By moving a Zone object to intersect with the portal at the respective side.  While usually it makes "
   "sense for this overlap to be small, the connection is established correctly as long as the center of the Zone "
   "object that should connect is on the correct side of the portal plane.</li>\n"
   "<li>By the respective side of the portal free of Zone objects that would connect to it.  In this case, given "
   "that the other side is connected to one or more Zones, the portal will automatically connect itself to the "
   "outdoor \"zone\" which implicitly is present in any level.</li>\n"
   "</ol>\n\n"

   "From this, it follows that there are two types of portals:\n\n"

   "<dl>\n"
   "<dt>Exterior Portals</dt>"
   "<dd>An exterior portal is one that is connected to one or more Zone objects on one side and to the outdoor "
   "zone at the other side.  This kind of portal is most useful for covering transitions from outdoor spaces to "
   "indoor spaces.</dd>"
   "<dt>Interior Portals</dt>"
   "<dd>An interior portal is one that is connected to one or more Zone objects on both sides.  This kind of portal "
   "is most useful for covering transitions between indoor spaces./dd>"
   "</dl>\n\n"

   "Strictly speaking, there is a third type of portal called an \"invalid portal\".  This is a portal that is not "
   "connected to a Zone object on either side in which case the portal serves no use.\n\n"

   "Portals in Torque are bidirectional meaning that they connect zones both ways and "
   "you can look through the portal's front side as well as through its back-side.\n\n"

   "Like Zones, Portals can either be box-shaped or use custom convex polyhedral shapes.\n\n"

   "Portals will usually be created in the editor but can, of course, also be created "
   "in script code as such:\n\n"

   "@tsexample\n"
      "// Example declaration of a Portal.  This will create a box-shaped portal.\n"
      "new Portal( PortalToTestZone )\n"
      "{\n"
      "   position = \"12.8467 -4.02246 14.8017\";\n"
      "	 rotation = \"0 0 -1 97.5085\";\n"
      "	 scale = \"1 0.25 1\";\n"
      "	 canSave = \"1\";\n"
      "	 canSaveDynamicFields = \"1\";\n"
      "};\n"
   "@endtsexample\n\n"

   "@note Keep in mind that zones and portals are more or less strictly a scene optimization mechanism meant to "
      "improve render times.\n\n"

   "@see Zone\n"
   "@ingroup enviroMisc\n"
);


// Notes:
// - This class implements portal spaces as single zones.  A different, interesting take
//   on this is to turn portal spaces into two zones with the portal plane acting as
//   the separator.
// - One downside to our treatment of portals as full zones in their own right is that
//   in certain cases we end up including space in the traversal that is clearly not visible.
//   Take a situation where you are in the outside zone and you are looking straight into
//   the wall of a house.  On the other side of that house is a portal leading into the house.
//   While the traversal will not step through that portal into the house since that would
//   be leading towards the camera rather than away from it, it will still add the frustum
//   to the visible space of the portal zone and thus make everything in the portal zone
//   visible.  It has to do this since, while it can easily tell whether to step through the
//   portal or not, it cannot easily tell whether the whole portal zone is visible or not as
//   that depends on the occlusion situation in the outdoor zone.


//-----------------------------------------------------------------------------

Portal::Portal()
   : mClassification( InvalidPortal ),
     mIsGeometryDirty( true )
{
   VECTOR_SET_ASSOCIATION( mPortalPolygonWS );

   mNetFlags.set( Ghostable | ScopeAlways );
   mTypeMask |= StaticObjectType;

   mPassableSides[ FrontSide ] = true;
   mPassableSides[ BackSide ] = true;

   mObjScale.set( 1.0f, 0.25f, 1.0f );

   // We're not closed off.
   mZoneFlags.clear( ZoneFlag_IsClosedOffSpace );
}

//-----------------------------------------------------------------------------

void Portal::initPersistFields()
{
   addGroup( "Zoning" );

      addProtectedField( "frontSidePassable", TypeBool, Offset( mPassableSides[ FrontSide ], Portal ),
         &_setFrontSidePassable, &defaultProtectedGetFn,
         "Whether one can view through the front-side of the portal." );
      addProtectedField( "backSidePassable", TypeBool, Offset( mPassableSides[ BackSide ], Portal ),
         &_setBackSidePassable, &defaultProtectedGetFn,
         "Whether one can view through the back-side of the portal." );

   endGroup( "Zoning" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void Portal::consoleInit()
{
   // Disable rendering of portals by default.
   getStaticClassRep()->mIsRenderEnabled = false;
}

//-----------------------------------------------------------------------------

String Portal::describeSelf() const
{
   String str = Parent::describeSelf();
   
   switch( getClassification() )
   {
      case InvalidPortal:     str += "|InvalidPortal"; break;
      case ExteriorPortal:    str += "|ExteriorPortal"; break;
      case InteriorPortal:    str += "|InteriorPortal"; break;
   }

   return str;
}

//-----------------------------------------------------------------------------

bool Portal::writeField( StringTableEntry fieldName, const char* value )
{
   static StringTableEntry sFrontSidePassable = StringTable->insert( "frontSidePassable" );
   static StringTableEntry sBackSidePassable = StringTable->insert( "backSidePassable" );

   // Don't write passable flags if at default.
   if( ( fieldName == sFrontSidePassable || fieldName == sBackSidePassable ) &&
       dAtob( value ) )
      return false;

   return Parent::writeField( fieldName, value );
}

//-----------------------------------------------------------------------------

void Portal::onSceneRemove()
{
   // Disconnect from root zone, if it's an exterior portal.

   if( mClassification == ExteriorPortal )
   {
      AssertFatal( getSceneManager()->getZoneManager(), "Portal::onSceneRemove - Portal classified as exterior without having a zone manager!" );
      getSceneManager()->getZoneManager()->getRootZone()->disconnectZoneSpace( this );
   }

   Parent::onSceneRemove();
}

//-----------------------------------------------------------------------------

void Portal::setTransform( const MatrixF& mat )
{
   // Portal geometry needs updating.  Set this before calling
   // parent because the transform change will cause an immediate
   // update of the portal's zoning state.
   mIsGeometryDirty = true;

   Parent::setTransform( mat );
}

//-----------------------------------------------------------------------------

void Portal::setSidePassable( Side side, bool value )
{
   if( mPassableSides[ side ] == value )
      return;

   mPassableSides[ side ] = value;

   if( isServerObject() )
      setMaskBits( PassableMask );
}

//-----------------------------------------------------------------------------

U32 Portal::packUpdate( NetConnection *con, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( con, mask, stream );

   stream->writeFlag( mPassableSides[ FrontSide ] );
   stream->writeFlag( mPassableSides[ BackSide ] );

   return retMask;
}

//-----------------------------------------------------------------------------

void Portal::unpackUpdate( NetConnection *con, BitStream *stream )
{
   Parent::unpackUpdate( con, stream );

   mPassableSides[ FrontSide ] = stream->readFlag();
   mPassableSides[ BackSide ] = stream->readFlag();
}

//-----------------------------------------------------------------------------

void Portal::_renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* overrideMat )
{
   if( overrideMat )
      return;

   // Update geometry if necessary.

   if( mIsGeometryDirty )
      _updateGeometry();

   // Render portal polygon.

   GFXStateBlockDesc desc;
   desc.setBlend( true );
   desc.setZReadWrite( true, false );
   desc.setCullMode( GFXCullNone );

   PlaneF::Side viewSide = mPortalPlane.whichSide( state->getCameraPosition() );

   ColorI color;
   switch( mClassification )
   {
      case InvalidPortal:  color = ColorI( 255, 255, 255, 45 ); break;
      case ExteriorPortal: color = viewSide == PlaneF::Front ? ColorI( 0, 128, 128, 45 ) : ColorI( 0, 255, 255, 45 ); break;
      case InteriorPortal: color = viewSide == PlaneF::Front ? ColorI( 128, 128, 0, 45 ) : ColorI( 255, 255, 0, 45 ); break;
   }

   GFX->getDrawUtil()->drawPolygon( desc, mPortalPolygonWS.address(), mPortalPolygonWS.size(), color );

   desc.setFillModeWireframe();
   GFX->getDrawUtil()->drawPolygon( desc, mPortalPolygonWS.address(), mPortalPolygonWS.size(), ColorF::RED );

   // Render rest.

   Parent::_renderObject( ri, state, overrideMat );
}

//-----------------------------------------------------------------------------

void Portal::traverseZones( SceneTraversalState* state, U32 startZoneId )
{
   // Check whether the portal is occluded.

   if( state->getTraversalDepth() > 0 &&
       state->getCullingState()->isOccluded( this ) )
      return;

   Parent::traverseZones( state, startZoneId );
}

//-----------------------------------------------------------------------------

void Portal::_traverseConnectedZoneSpaces( SceneTraversalState* state )
{
   PROFILE_SCOPE( Portal_traverseConnectedZoneSpaces );

   // Don't traverse out from the portal if it is invalid.

   if( mClassification == InvalidPortal )
      return;

   AssertFatal( !mIsGeometryDirty, "Portal::_traverseConnectedZoneSpaces - Geometry not up-to-date!" );

   // When starting traversal within a portal zone, we cannot really use the portal
   // plane itself to direct our visibility queries.  For example, the camera might
   // actually be located in front of the portal plane and thus cannot actually look
   // through the portal, though it will still see what lies on front of where the
   // portal leads.
   //
   // So if we're the start of the traversal chain, i.e. the traversal has started
   // out in the portal zone, then just put the traversal through to SceneZoneSpace
   // so it can hand it over to all connected zone managers.
   //
   // Otherwise, just do a normal traversal by stepping through the portal.

   if( state->getTraversalDepth() == 1 )
   {
      Parent::_traverseConnectedZoneSpaces( state );
      return;
   }
   
   SceneCullingState* cullingState = state->getCullingState();
   const SceneCameraState& cameraState = cullingState->getCameraState();

   // Get the data of the zone we're coming from.  Note that at this point
   // the portal zone itself is already on top of the traversal stack, so
   // we skip over the bottom-most entry.

   const U32 sourceZoneId = state->getZoneIdFromStack( 1 );
   const SceneZoneSpace* sourceZoneSpace = state->getZoneFromStack( 1 );

   // Find out which side of the portal we are on given the
   // source zone.

   const Portal::Side currentZoneSide =
      sourceZoneId == SceneZoneSpaceManager::RootZoneId
      ? ( getInteriorSideOfExteriorPortal() == FrontSide ? BackSide : FrontSide )
      : getSideRelativeToPortalPlane( sourceZoneSpace->getPosition() );

   // Don't step through portal if the side we're interested in isn't passable.

   if( !isSidePassable( currentZoneSide ) )
      return;

   // If the viewpoint isn't on the same side of the portal as the source zone,
   // then stepping through the portal would mean we are stepping back towards
   // the viewpoint which doesn't make sense; so, skip the portal.

   const Point3F& viewPos = cameraState.getViewPosition();
   const F32 viewPosDistToPortal = mFabs( getPortalPlane().distToPlane( viewPos ) );
   if( !mIsZero( viewPosDistToPortal ) && getSideRelativeToPortalPlane( viewPos ) != currentZoneSide )
      return;

   // Before we go ahead and do the real work, try to find out whether
   // the portal is at a perpendicular or near-perpendicular angle to the view
   // direction.  If so, there's no point in going further since we can't really
   // see much through the portal anyway.  It also prevents us from stepping
   // over front/back side ambiguities.

   Point3F viewDirection = cameraState.getViewDirection();
   const F32 dotProduct = mDot( viewDirection, getPortalPlane() );
   if( mIsZero( dotProduct ) )
      return;

   // Finally, if we have come through a portal to the current zone, check if the target
   // portal we are trying to step through now completely lies on the "backside"--i.e.
   // the side of portal on which our current zone lies--of the source portal.  If so,
   // we can be sure this portal leads us in the wrong direction.  This prevents the
   // outdoor zone from having just arrived through a window on one side of a house just
   // to go round the house and re-enter it from the other side.

   Portal* sourcePortal = state->getTraversalDepth() > 2 ? dynamic_cast< Portal* >( state->getZoneFromStack( 2 ) ) : NULL;
   if( sourcePortal != NULL )
   {
      const Side sourcePortalFrontSide =
         sourceZoneId == SceneZoneSpaceManager::RootZoneId
         ? sourcePortal->getInteriorSideOfExteriorPortal()
         : sourcePortal->getSideRelativeToPortalPlane( sourceZoneSpace->getPosition() );
      const PlaneF::Side sourcePortalPlaneFrontSide =
         sourcePortalFrontSide == FrontSide ? PlaneF::Front : PlaneF::Back;

      bool allPortalVerticesOnBackside = true;
      const U32 numVertices = mPortalPolygonWS.size();
      for( U32 i = 0; i < numVertices; ++ i )
      {
         // Not using getSideRelativeToPortalPlane here since we want PlaneF::On to be
         // counted as backside here.
         if( sourcePortal->mPortalPlane.whichSide( mPortalPolygonWS[ i ] ) == sourcePortalPlaneFrontSide )
         {
            allPortalVerticesOnBackside = false;
            break;
         }
      }

      if( allPortalVerticesOnBackside )
         return;
   }

   // If we come from the outdoor zone, then we don't want to step through any portal
   // where the interior zones are actually on the same side as our camera since that
   // would mean we are stepping into an interior through the backside of a portal.

   if( sourceZoneId == SceneZoneSpaceManager::RootZoneId )
   {
      const Portal::Side cameraSide = getSideRelativeToPortalPlane( viewPos );
      if( cameraSide == getInteriorSideOfExteriorPortal() )
         return;
   }

   // Clip the current culling volume against the portal's polygon.  If the polygon
   // lies completely outside the volume or for some other reason there's no good resulting
   // volume, _generateCullingVolume() will return false and we terminate this portal sequence
   // here.
   //
   // However, don't attempt to clip the portal if we are standing really close to or
   // even below the near distance away from the portal plane.  In that case, trying to
   // clip the portal will only result in trouble so we stick to the original culling volume
   // in that case.

   bool haveClipped = false;
   if( viewPosDistToPortal > ( cameraState.getFrustum().getNearDist() + 0.1f ) )
   {
      SceneCullingVolume volume;
      if( !_generateCullingVolume( state, volume ) )
         return;

      state->pushCullingVolume( volume );
      haveClipped = true;
   }

   // Short-circuit things if we are stepping from an interior zone outside.  In this
   // case we know that the only zone we care about is the outdoor zone so head straight
   // into it.

   if( isExteriorPortal() && sourceZoneId != SceneZoneSpaceManager::RootZoneId )
      getSceneManager()->getZoneManager()->getRootZone()->traverseZones( state );
   else
   {
      // Go through the zones that the portal connects to and
      // traverse into them.

      for( ZoneSpaceRef* ref = mConnectedZoneSpaces; ref != NULL; ref = ref->mNext )
      {
         SceneZoneSpace* targetSpace = ref->mZoneSpace;
         if( targetSpace == sourceZoneSpace )
            continue; // Skip space we originated from.

         // We have handled the case of stepping into the outdoor zone above and
         // by skipping the zone we originated from, we have implicitly handled the
         // case of stepping out of the outdoor zone.  Thus, we should not see the
         // outdoor zone here.  Important as getPosition() is meaningless for it.
         AssertFatal( targetSpace->getZoneRangeStart() != SceneZoneSpaceManager::RootZoneId,
            "Portal::_traverseConnectedZoneSpaces - Outdoor zone must have been handled already" );

         // Skip zones that lie on the same side as the zone
         // we originated from.

         if( getSideRelativeToPortalPlane( targetSpace->getPosition() ) == currentZoneSide )
            continue;

         // Traverse into the space.

         targetSpace->traverseZones( state );
      }
   }

   // If we have pushed our own clipping volume,
   // remove that from the stack now.

   if( haveClipped )
      state->popCullingVolume();
}

//-----------------------------------------------------------------------------

bool Portal::_generateCullingVolume( SceneTraversalState* state, SceneCullingVolume& outVolume ) const
{
   PROFILE_SCOPE( Portal_generateCullingVolume );

   SceneCullingState* cullingState = state->getCullingState();
   const SceneCullingVolume& currentVolume = state->getCurrentCullingVolume();

   // Clip the portal polygon against the current culling volume.

   Point3F vertices[ 64 ];
   U32 numVertices = 0;

   numVertices = currentVolume.getPlanes().clipPolygon(
      mPortalPolygonWS.address(),
      mPortalPolygonWS.size(),
      vertices,
      sizeof( vertices ) /sizeof( vertices[ 0 ] )
   );

   AssertFatal( numVertices == 0 || numVertices >= 3,
      "Portal::_generateCullingVolume - Clipping produced degenerate polygon" );

   if( !numVertices )
      return false;

   // Create a culling volume.

   return cullingState->createCullingVolume(
      vertices, numVertices,
      SceneCullingVolume::Includer,
      outVolume
   );
}

//-----------------------------------------------------------------------------

void Portal::connectZoneSpace( SceneZoneSpace* zoneSpace )
{
   Parent::connectZoneSpace( zoneSpace );

   // Update portal state.  Unfortunately, we can't do that on demand
   // easily since everything must be in place before a traversal starts.

   _update();
}

//-----------------------------------------------------------------------------

void Portal::disconnectZoneSpace( SceneZoneSpace* zoneSpace )
{
   Parent::disconnectZoneSpace( zoneSpace );

   // Update portal state.

   _update();
}

//-----------------------------------------------------------------------------

void Portal::_disconnectAllZoneSpaces()
{
   Parent::_disconnectAllZoneSpaces();

   // Update portal state.

   _update();
}

//-----------------------------------------------------------------------------

void Portal::_update()
{
   if( mIsGeometryDirty )
      _updateGeometry();

   _updateConnectivity();
}

//-----------------------------------------------------------------------------

void Portal::_updateGeometry()
{
   const F32 boxHalfWidth = getScale().x * 0.5f;
   const F32 boxHalfHeight = getScale().z * 0.5f;

   const Point3F center = getTransform().getPosition();
   const Point3F up = getTransform().getUpVector() * boxHalfHeight;
   const Point3F right = getTransform().getRightVector() * boxHalfWidth;

   // Update the portal polygon and plane.

   if( mIsBox )
   {
      // It's a box so the portal polygon is a rectangle.
      // Simply compute the corner points by stepping from the
      // center to the corners using the up and right vector.

      mPortalPolygonWS.setSize( 4 );

      mPortalPolygonWS[ 0 ] = center + right - up; // bottom right
      mPortalPolygonWS[ 1 ] = center - right - up; // bottom left
      mPortalPolygonWS[ 2 ] = center - right + up; // top left
      mPortalPolygonWS[ 3 ] = center + right + up; // top right

      // Update the plane by going through three of the points.

      mPortalPlane = PlaneF(
         mPortalPolygonWS[ 0 ],
         mPortalPolygonWS[ 1 ],
         mPortalPolygonWS[ 2 ]
      );
   }
   else
   {
      // It's not necessarily a box so we must use the general
      // routine.

      // Update the portal plane by building a plane that cuts the
      // OBB in half vertically along its Y axis.

      mPortalPlane = PlaneF(
         center + right - up,
         center - right - up,
         center - right + up
      );

      // Slice the polyhedron along the same plane in object space.

      const PlaneF slicePlane = PlaneF( Point3F::Zero, Point3F( 0.f, 1.f, 0.f ) );

      mPortalPolygonWS.setSize( mPolyhedron.getNumEdges() );
      U32 numPoints = mPolyhedron.constructIntersection( slicePlane, mPortalPolygonWS.address(), mPortalPolygonWS.size() );
      mPortalPolygonWS.setSize( numPoints );

      // Transform the polygon to world space.

      for( U32 i = 0; i < numPoints; ++ i )
      {
         mPortalPolygonWS[ i ].convolve( getScale() );
         mObjToWorld.mulP( mPortalPolygonWS[ i ] );
      }
   }

   mIsGeometryDirty = false;
}

//-----------------------------------------------------------------------------

void Portal::_updateConnectivity()
{
   SceneZoneSpaceManager* zoneManager = getSceneManager()->getZoneManager();
   if( !zoneManager )
      return;

   // Find out where our connected zones are in respect to the portal
   // plane.

   bool haveInteriorZonesOnFrontSide = false;
   bool haveInteriorZonesOnBackSide = false;
   bool isConnectedToRootZone = ( mClassification == ExteriorPortal );

   for(  ZoneSpaceRef* ref = mConnectedZoneSpaces;
         ref != NULL; ref = ref->mNext )
   {
      SceneZoneSpace* zone = dynamic_cast< SceneZoneSpace* >( ref->mZoneSpace );
      if( !zone || zone->isRootZone() )
         continue;

      if( getSideRelativeToPortalPlane( zone->getPosition() ) == FrontSide )
         haveInteriorZonesOnFrontSide = true;
      else
         haveInteriorZonesOnBackSide = true;
   }

   // If we have zones connected to us on only one side, we are an exterior
   // portal.  Otherwise, we're an interior portal.

   SceneRootZone* rootZone = zoneManager->getRootZone();
   if( haveInteriorZonesOnFrontSide && haveInteriorZonesOnBackSide )
   {
      mClassification = InteriorPortal;
   }
   else if( haveInteriorZonesOnFrontSide || haveInteriorZonesOnBackSide )
   {
      mClassification = ExteriorPortal;

      // Remember where our interior zones are.

      if( haveInteriorZonesOnBackSide )
         mInteriorSide = BackSide;
      else
         mInteriorSide = FrontSide;

      // If we aren't currently connected to the root zone,
      // establish the connection now.

      if( !isConnectedToRootZone )
      {
         Parent::connectZoneSpace( rootZone );
         rootZone->connectZoneSpace( this );
      }
   }
   else
      mClassification = InvalidPortal;

   // If we have been connected to the outdoor zone already but the
   // portal got classified as invalid or interior now, break the
   // connection to the outdoor zone.

   if( isConnectedToRootZone &&
       ( mClassification == InvalidPortal || mClassification == InteriorPortal ) )
   {
      Parent::disconnectZoneSpace( rootZone );
      rootZone->disconnectZoneSpace( this );
   }
}

//-----------------------------------------------------------------------------

Portal::Side Portal::getSideRelativeToPortalPlane( const Point3F& point ) const
{
   // For our purposes, we consider PlaneF::Front and PlaneF::On
   // placement as FrontSide.

   PlaneF::Side planeSide = getPortalPlane().whichSide( point );
   if( planeSide == PlaneF::Front || planeSide == PlaneF::On )
      return FrontSide;
   else
      return BackSide;
}

//-----------------------------------------------------------------------------

bool Portal::_setFrontSidePassable( void* object, const char* index, const char* data )
{
   Portal* portal = reinterpret_cast< Portal* >( object );
   portal->setSidePassable( Portal::FrontSide, EngineUnmarshallData< bool >()( data ) );
   return false;
}

//-----------------------------------------------------------------------------

bool Portal::_setBackSidePassable( void* object, const char* index, const char* data )
{
   Portal* portal = reinterpret_cast< Portal* >( object );
   portal->setSidePassable( Portal::BackSide, EngineUnmarshallData< bool >()( data ) );
   return false;
}

//=============================================================================
//    Console API.
//=============================================================================
// MARK: ---- Console API ----

//-----------------------------------------------------------------------------

DefineEngineMethod( Portal, isInteriorPortal, bool, (),,
   "Test whether the portal connects interior zones only.\n\n"
   "@return True if the portal is an interior portal." )
{
   return object->isInteriorPortal();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Portal, isExteriorPortal, bool, (),,
   "Test whether the portal connects interior zones to the outdoor zone.\n\n"
   "@return True if the portal is an exterior portal." )
{
   return object->isExteriorPortal();
}
