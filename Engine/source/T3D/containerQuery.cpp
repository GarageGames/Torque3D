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
#include "T3D/containerQuery.h"

#include "scene/sceneObject.h"
#include "environment/waterObject.h"
#include "T3D/physicalZone.h"


void findRouter( SceneObject *obj, void *key )
{
   if (obj->getTypeMask() & WaterObjectType)
      waterFind(obj, key);
   else if (obj->getTypeMask() & PhysicalZoneObjectType)
      physicalZoneFind(obj, key);
   else {
      AssertFatal(false, "Error, must be either water or physical zone here!");
   }
}

void waterFind( SceneObject *obj, void *key )
{     
   PROFILE_SCOPE( waterFind );

   // This is called for each WaterObject the ShapeBase object is overlapping.

   ContainerQueryInfo *info = static_cast<ContainerQueryInfo*>(key);
   WaterObject *water = dynamic_cast<WaterObject*>(obj);      
   AssertFatal( water != NULL, "containerQuery - waterFind(), passed object was not of class WaterObject!");      

   // Get point at the bottom/center of the box.
   Point3F testPnt = info->box.getCenter();
   testPnt.z = info->box.minExtents.z;

   F32 coverage = water->getWaterCoverage(info->box);

   // Since a WaterObject can have global bounds we may get this call
   // even though we have zero coverage.  If so we want to early out and
   // not save the water properties.
   if ( coverage == 0.0f )
      return;

   // Add in flow force.  Would be appropriate to try scaling it by coverage
   // thought. Or perhaps have getFlow do that internally and take
   // the box parameter.
   info->appliedForce += water->getFlow( testPnt );

   // Only save the following properties for the WaterObject with the
   // greatest water coverage for this ShapeBase object.
   if ( coverage < info->waterCoverage )
      return;
   
   info->waterCoverage = coverage;
   info->liquidType = water->getLiquidType();
   info->waterViscosity = water->getViscosity();
   info->waterDensity = water->getDensity();
   info->waterHeight = water->getSurfaceHeight( Point2F(testPnt.x,testPnt.y) );   
   info->waterObject = water;
}

void physicalZoneFind(SceneObject* obj, void *key)
{    
   PROFILE_SCOPE( physicalZoneFind );

   ContainerQueryInfo *info = static_cast<ContainerQueryInfo*>(key);
   PhysicalZone* pz = dynamic_cast<PhysicalZone*>(obj);
   AssertFatal(pz != NULL, "Error, not a physical zone!");
   if (pz == NULL || pz->testBox(info->box) == false)
      return;

   if (pz->isActive()) {
      info->gravityScale *= pz->getGravityMod();
      info->appliedForce += pz->getForce();
   }
}

