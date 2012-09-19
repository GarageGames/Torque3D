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
#include "scene/zones/sceneTraversalState.h"

#include "scene/sceneManager.h"
#include "scene/culling/sceneCullingState.h"
#include "scene/culling/sceneCullingVolume.h"
#include "scene/zones/sceneZoneSpaceManager.h"


//-----------------------------------------------------------------------------

SceneTraversalState::SceneTraversalState( SceneCullingState* cullingState )
   : mCullingState( cullingState ),
     mTraversedArea( 0.f )
{
   VECTOR_SET_ASSOCIATION( mZoneStack );
   VECTOR_SET_ASSOCIATION( mCullingVolumeStack );

   // Push the polyhedron of the root frustum onto the traversal
   // stack as the current culling volume.

   pushCullingVolume( cullingState->getRootVolume() );
}

//-----------------------------------------------------------------------------

SceneZoneSpace* SceneTraversalState::getZoneFromStack( U32 depth )
{
   SceneZoneSpaceManager* zoneManager = getCullingState()->getSceneManager()->getZoneManager();
   U32 zoneId = getZoneIdFromStack( depth );

   return zoneManager->getZoneOwner( zoneId );
}

//-----------------------------------------------------------------------------

void SceneTraversalState::pushCullingVolume( const SceneCullingVolume& volume )
{
   mCullingVolumeStack.push_back( volume );
}

//-----------------------------------------------------------------------------

void SceneTraversalState::popCullingVolume()
{
   AssertFatal( mCullingVolumeStack.size() > 1, "SceneTraversalState::popCullingVolume - Must not pop root volume" );
   mCullingVolumeStack.pop_back();
}
