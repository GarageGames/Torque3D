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
#include "scene/zones/sceneRootZone.h"

#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"



//RD: for the SceneRootZone, it may be worthwhile to put an optimized path in place that
//    does some spatially aware testing of portals rather than just blindly going through
//    the list of connected zone managers


//-----------------------------------------------------------------------------

SceneRootZone::SceneRootZone()
{
   setGlobalBounds();
   resetWorldBox();

   // Clear netflags we have inherited.  We don't
   // network scene roots.
   mNetFlags.clear( Ghostable | ScopeAlways );

   // Clear type flags we have inherited.
   mTypeMask &= ~StaticObjectType;
}

//-----------------------------------------------------------------------------

String SceneRootZone::describeSelf() const
{
   String str = Parent::describeSelf();

   str += "|SceneRootZone";
   
   return str;
}

//-----------------------------------------------------------------------------

bool SceneRootZone::onSceneAdd()
{
   if( !Parent::onSceneAdd() )
      return false;

   AssertFatal( getZoneRangeStart() == SceneZoneSpaceManager::RootZoneId, "SceneRootZone::onSceneAdd - SceneRootZone must be first scene object zone manager!" );

   return true;
}

//-----------------------------------------------------------------------------

void SceneRootZone::onSceneRemove()
{
   AssertFatal( getZoneRangeStart() == SceneZoneSpaceManager::RootZoneId, "SceneRootZone::onSceneRemove - SceneRootZone must be first scene object zone manager!");
   Parent::onSceneRemove();
}

//-----------------------------------------------------------------------------

bool SceneRootZone::onAdd()
{
   AssertFatal( false, "SceneRootZone::onAdd - Must not register SceneRootZone!" );
   return false;
}

//-----------------------------------------------------------------------------

void SceneRootZone::onRemove()
{
   AssertFatal( false, "SceneRootZone::onRemove - Must not be called!" );
}

//-----------------------------------------------------------------------------

U32 SceneRootZone::getPointZone( const Point3F &p ) const
{
   return 0;
}

//-----------------------------------------------------------------------------

bool SceneRootZone::getOverlappingZones( const Box3F& aabb, U32* outZones, U32& outNumZones )
{
   // Everything overlaps the outside zone.

   outZones[ 0 ]  = SceneZoneSpaceManager::RootZoneId;
   outNumZones = 1;

   return false;
}

//-----------------------------------------------------------------------------

bool SceneRootZone::containsPoint( const Point3F &point ) const
{
   return true;
}
