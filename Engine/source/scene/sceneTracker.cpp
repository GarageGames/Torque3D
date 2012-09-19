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

#include "scene/sceneTracker.h"


//=============================================================================
//    SceneObjectLink.
//=============================================================================

//-----------------------------------------------------------------------------

SceneObjectLink::SceneObjectLink( SceneTracker* tracker, SceneObject* object )
   : mTracker( tracker ),
     mObject( object ),
     mNextLink( NULL ),
     mPrevLink( NULL )
{
   if( object )
   {
      mNextLink = object->mSceneObjectLinks;
      if( mNextLink )
         mNextLink->mPrevLink = this;
      object->mSceneObjectLinks = this;
   }
}

//-----------------------------------------------------------------------------

SceneObjectLink::~SceneObjectLink()
{
   if( mObject )
   {
      // Unlink from SceneObject's tracker list.

      if( mNextLink )
         mNextLink->mPrevLink = mPrevLink;
      if( mPrevLink )
         mPrevLink->mNextLink = mNextLink;
      else
         mObject->mSceneObjectLinks = mNextLink;
   }
}

//-----------------------------------------------------------------------------

void SceneObjectLink::update()
{
   getTracker()->updateObject( this );
}

//-----------------------------------------------------------------------------

SceneObjectLink* SceneObjectLink::getLinkForTracker( SceneTracker* tracker, SceneObject* fromObject )
{
   for( SceneObjectLink* link = fromObject->mSceneObjectLinks; link != NULL; link = link->getNextLink() )
      if( link->getTracker() == tracker )
         return link;

   return NULL;
}

//=============================================================================
//    SceneObjectLink.
//=============================================================================

//-----------------------------------------------------------------------------

SceneTracker::SceneTracker( bool isClientTracker, U32 typeMask )
   : mIsClientTracker( isClientTracker ),
     mObjectTypeMask( typeMask )
{
   // Hook up to SceneObject add/remove notifications.

   SceneObject::smSceneObjectAdd.notify( this, &SceneTracker::registerObject );
   SceneObject::smSceneObjectRemove.notify( this, &SceneTracker::unregisterObject );
}

//-----------------------------------------------------------------------------

SceneTracker::~SceneTracker()
{
   SceneObject::smSceneObjectAdd.remove( this, &SceneTracker::registerObject );
   SceneObject::smSceneObjectRemove.remove( this, &SceneTracker::unregisterObject );
}

//-----------------------------------------------------------------------------

void SceneTracker::init()
{
   // Register existing scene graph objects.

   SceneContainer* container;
   if( isClientTracker() )
      container = &gClientContainer;
   else
      container = &gServerContainer;

   container->findObjects( getObjectTypeMask(),
                           ( SceneContainer::FindCallback ) &_containerFindCallback,
                           this );
}

//-----------------------------------------------------------------------------

void SceneTracker::_containerFindCallback( SceneObject* object, SceneTracker* tracker )
{
   tracker->registerObject( object );
}
