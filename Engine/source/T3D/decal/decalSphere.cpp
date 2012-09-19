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
#include "T3D/decal/decalSphere.h"

#include "scene/zones/sceneZoneSpaceManager.h"
#include "T3D/decal/decalInstance.h"


F32 DecalSphere::smDistanceTolerance = 30.0f;
F32 DecalSphere::smRadiusTolerance = 40.0f;


//-----------------------------------------------------------------------------

bool DecalSphere::tryAddItem( DecalInstance* inst )
{
   // If we are further away from the center than our tolerance
   // allows, don't use this sphere.
   //
   // Note that we take the distance from the nearest point on the
   // bounding sphere rather than the distance to the center of the
   // decal.  This takes decal sizes into account and generally works
   // better.

   const F32 distCenterToCenter = ( mWorldSphere.center - inst->mPosition ).len();
   const F32 distBoundsToCenter = distCenterToCenter - inst->mSize / 2.f;

   if( distBoundsToCenter > smDistanceTolerance )
      return false;

   // Also, if adding the current decal to the sphere would make
   // it larger than our radius tolerance, don't use it.

   const F32 newRadius = distCenterToCenter + inst->mSize / 2.f + 0.5f;
   if( newRadius > mWorldSphere.radius && newRadius > smRadiusTolerance )
      return false;

   // Otherwise, go with this sphere and add the item to it.

   mItems.push_back( inst );

   // Update the sphere bounds, if necessary.

   if( newRadius > mWorldSphere.radius )
      updateWorldSphere();

   return true;
}

//-----------------------------------------------------------------------------

void DecalSphere::updateWorldSphere()
{
   AssertFatal( mItems.size() >= 1, "DecalSphere::updateWorldSphere - Sphere is empty!" );

   Box3F aabb( mItems[ 0 ]->getWorldBox() );

   const U32 numItems = mItems.size();
   for( U32 i = 1; i < numItems; ++ i )
      aabb.intersect( mItems[ i ]->getWorldBox() );

   mWorldSphere = aabb.getBoundingSphere();
      
   // Clear the zoning data so that it gets recomputed.
   mZones.clear();
}

//-----------------------------------------------------------------------------

void DecalSphere::updateZoning( SceneZoneSpaceManager* zoneManager )
{
   mZones.clear();

   // If there's only a single zone in the world, it must be the
   // outdoor zone, so there's no point maintaining zoning information
   // on all the decalspheres.

   if( zoneManager->getNumZones() == 1 )
      return;

   // Otherwise query the scene graph for all zones that intersect with the
   // AABB around our world sphere.

   Box3F aabb( mWorldSphere.radius );
   aabb.setCenter( mWorldSphere.center );

   zoneManager->findZones( aabb, mZones );
}
