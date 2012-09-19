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
#include "T3D/zone.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "scene/mixin/sceneAmbientSoundObject.impl.h"
#include "scene/mixin/scenePolyhedralObject.impl.h"


IMPLEMENT_CO_NETOBJECT_V1( Zone );

ConsoleDocClass( Zone,
   "@brief An object that represents an interior space.\n\n"

   "A zone is an invisible volume that encloses an interior space.  All objects that have "
   "their world space axis-aligned bounding boxes (AABBs) intersect the zone's volume are "
   "assigned to the zone.  This assignment happens automatically as objects are placed "
   "and transformed.  Also, assignment is not exclusive meaning that an object can be assigned "
   "to many zones at the same time if it intersects all of them.\n\n"

   "In itself, the volume of a zone is fully sealed off from the outside.  This means that while "
   "viewing the scene from inside the volume, only objects assigned to the zone are rendered while "
   "when viewing the scene from outside the volume, objects <em>exclusively</em> only assigned the "
   "zone are not rendered.\n\n"

   "Usually, you will want to connect zones to each other by means of portals.  A portal overlapping "
   "with a zone \n\n"

   "@tsexample\n"
   "// Example declaration of a Zone.  This creates a box-shaped zone.\n"
   "new Zone( TestZone )\n"
   "{\n"
   "   position = \"3.61793 -1.01945 14.7442\";\n"
   "   rotation = \"1 0 0 0\";\n"
   "   scale = \"10 10 10\";\n"
   "};\n"
   "@endtsexample\n\n"

   "@section Zone_zoneGroups Zone Groups\n\n"

   "Normally, Zones will not connect to each other when they overlap.  This means that if viewing "
   "the scene from one zone, the contents of the other zone will not be visible except when there "
   "is a portal connecting the zones.  However, sometimes it is convenient to represent a single interior "
   "space through a combination of Zones so that when any of these zones is visible, all other zones "
   "that are part of the same interior space are visible.  This is possible by employing \"zone groups\".\n\n"

   "@see Portal\n"

   "@ingroup enviroMisc\n"
);


//-----------------------------------------------------------------------------

void Zone::consoleInit()
{
   // Disable rendering of zones by default.
   getStaticClassRep()->mIsRenderEnabled = false;
}

//=============================================================================
//    Console API.
//=============================================================================
// MARK: ---- Console API ----

//-----------------------------------------------------------------------------

DefineEngineMethod( Zone, getZoneId, S32, (),,
   "Get the unique numeric ID of the zone in its scene.\n\n"
   "@return The ID of the zone." )
{
   return object->getZoneRangeStart();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Zone, dumpZoneState, void, ( bool updateFirst ), ( true ),
   "Dump a list of all objects assigned to the zone to the console as well as a list "
   "of all connected zone spaces.\n\n"
   "@param updateFirst Whether to update the contents of the zone before dumping.  Since zoning states of "
      "objects are updated on demand, the zone contents can be outdated." )
{
   object->dumpZoneState( updateFirst );
}
