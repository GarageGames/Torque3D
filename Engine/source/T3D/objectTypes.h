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

#ifndef _OBJECTTYPES_H_
#define _OBJECTTYPES_H_

#include "platform/types.h"

/// Types used for SceneObject type masks (SceneObject::mTypeMask)
///
/// @note If a new object type is added, don't forget to add it to
///      RegisterGameFunctions().
enum SceneObjectTypes
{
   /// @name Types used by the SceneObject class
   /// @{

   /// Default value for type masks.
   DefaultObjectType = 0,

   /// @}

   /// @name Basic Engine Types
   /// @{

   /// Any kind of SceneObject that is not supposed to change transforms
   /// except during editing (or not at all).
   StaticObjectType = BIT( 0 ),

   /// Environment objects such as clouds, skies, forests, etc.
   EnvironmentObjectType = BIT( 1 ),

   /// A terrain object.
   /// @see TerrainBlock
   TerrainObjectType = BIT( 2 ),

   /// An object defining a water volume.
   /// @see WaterObject
   WaterObjectType = BIT( 3 ),

   /// An object defining an invisible trigger volume.
   /// @see Trigger
   TriggerObjectType = BIT( 4 ),

   /// An object defining an invisible marker.
   /// @see MissionMarker
   MarkerObjectType = BIT( 5 ),

   /// A light emitter.
   /// @see LightBase
   LightObjectType = BIT( 6 ),

   /// An object that manages zones.  This is automatically set by
   /// SceneZoneSpaceManager when a SceneZoneSpace registers zones.  Should
   /// not be manually set.
   ///
   /// @see SceneZoneSpace
   /// @see SceneZoneSpaceManager
   ZoneObjectType = BIT( 7 ),

   /// Any object that defines one or more solid, renderable static geometries that
   /// should be included in collision and raycasts.
   ///
   /// Use this mask to find objects that are part of the static level geometry.
   ///
   /// @note If you set this, you will also want to set StaticObjectType.
   StaticShapeObjectType = BIT( 8 ),

   /// Any object that defines one or more solid, renderable dynamic geometries that
   /// should be included in collision and raycasts.
   ///
   /// Use this mask to find objects that are part of the dynamic game geometry.
   DynamicShapeObjectType = BIT( 9 ),

   /// @}

   /// @name Game Types
   /// @{

   /// Any GameBase-derived object.
   /// @see GameBase
   GameBaseObjectType = BIT( 10 ),

   /// An object that uses hifi networking.
   GameBaseHiFiObjectType = BIT( 11 ),

   /// Any ShapeBase-derived object.
   /// @see ShapeBase
   ShapeBaseObjectType = BIT( 12 ),

   /// A camera object.
   /// @see Camera
   CameraObjectType = BIT( 13 ),

   /// A human or AI player object.
   /// @see Player
   PlayerObjectType = BIT( 14 ),

   /// An item pickup.
   /// @see Item
   ItemObjectType = BIT( 15 ),

   /// A vehicle.
   /// @see Vehicle
   VehicleObjectType = BIT( 16 ),

   /// An object that blocks vehicles.
   /// @see VehicleBlocker
   VehicleBlockerObjectType = BIT( 17 ),

   /// A weapon projectile.
   /// @see Projectile
   ProjectileObjectType = BIT( 18 ),

   /// An explosion object.
   /// @see Explosion
   ExplosionObjectType = BIT( 19 ),

   /// A dead player.  This is dynamically set and unset.
   /// @see Player
   CorpseObjectType = BIT( 20 ),

   /// A debris object.
   /// @see Debris
   DebrisObjectType = BIT( 21 ),

   /// A volume that asserts forces on player objects.
   /// @see PhysicalZone
   PhysicalZoneObjectType = BIT( 22 ),

   /// @}
};

enum SceneObjectTypeMasks
{
   STATIC_COLLISION_TYPEMASK = StaticShapeObjectType,

   DAMAGEABLE_TYPEMASK = (   PlayerObjectType        |
                             VehicleObjectType ),

   /// Typemask for objects that should be rendered into shadow passes.
   /// These should be all objects that are either meant to receive or cast
   /// shadows or both.
   SHADOW_TYPEMASK = (  StaticShapeObjectType |
                        DynamicShapeObjectType ),

   /// Typemask for objects that should be subjected to more fine-grained
   /// culling tests.  Anything that is trivial rendering stuff or doesn't
   /// render except when in the editor should be excluded here.
   ///
   /// Also, objects that do their own culling internally (terrains, forests, etc.)
   /// should be excluded.
   CULLING_INCLUDE_TYPEMASK = (  GameBaseObjectType | // Includes most other renderable types; but broader than we ideally want.
                                 StaticShapeObjectType |
                                 DynamicShapeObjectType |
                                 ZoneObjectType ), // This improves the result of zone traversals.

   /// Mask for objects that should be specifically excluded from zone culling.
   CULLING_EXCLUDE_TYPEMASK = (  TerrainObjectType |
                                 EnvironmentObjectType ),

   /// Default object type mask to use for render queries.
   DEFAULT_RENDER_TYPEMASK = (   EnvironmentObjectType |
                                 TerrainObjectType |
                                 WaterObjectType |
                                 StaticShapeObjectType |
                                 DynamicShapeObjectType |
                                 LightObjectType | // Flares.
                                 GameBaseObjectType ),

   /// Typemask to use for rendering when inside the editor.
   EDITOR_RENDER_TYPEMASK = U32( -1 ),

   /// All objects that fit this type mask will be exclusively assigned to the outdoor (root)
   /// zone and not be assigned to individual interior objects.
   ///
   /// @note Terrains have their own means for rendering inside interior zones.
   OUTDOOR_OBJECT_TYPEMASK = (   TerrainObjectType |
                                 EnvironmentObjectType )
};

#endif
