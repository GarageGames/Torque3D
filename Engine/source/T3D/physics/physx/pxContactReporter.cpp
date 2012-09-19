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
#include "T3D/physics/physX/pxContactReporter.h"

#include "T3D/physics/physX/pxCasts.h"
#include "T3D/physics/physicsUserData.h"
#include "T3D/physics/physX/pxMultiActor.h"
#include "platform/profiler.h"


PxContactReporter::PxContactReporter()
{
}

PxContactReporter::~PxContactReporter()
{
}

void PxContactReporter::onContactNotify( NxContactPair &pair, NxU32 events )
{
   PROFILE_SCOPE( PxContactReporter_OnContactNotify );

   // For now we only care about start touch events.
   if ( !( events & NX_NOTIFY_ON_START_TOUCH ) )
      return;

   // Skip if either actor is deleted.
   if ( pair.isDeletedActor[0] || pair.isDeletedActor[1] )
      return;

   NxActor *actor0 = pair.actors[0];
   NxActor *actor1 = pair.actors[1];

   PhysicsUserData *userData0 = PhysicsUserData::cast( actor0->userData );
   PhysicsUserData *userData1 = PhysicsUserData::cast( actor1->userData );

   // Early out if we don't have user data or signals to notify.
   if (  ( !userData0 || userData0->getContactSignal().isEmpty() ) &&
         ( !userData1 || userData1->getContactSignal().isEmpty() ) )
      return;

   // Get an average contact point.
   U32 points = 0;
   NxVec3 hitPoint( 0.0f );
   NxContactStreamIterator iter( pair.stream );
   while( iter.goNextPair() )
   {
      while( iter.goNextPatch() )
      {
         while( iter.goNextPoint() )
         {
            hitPoint += iter.getPoint();
            ++points;
         }
      }
   }
   hitPoint /= (F32)points;

   if ( userData0 )
      userData0->getContactSignal().trigger( userData0, 
                                             userData1, 
                                             pxCast<Point3F>( hitPoint ),
                                             pxCast<Point3F>( pair.sumNormalForce ) );

   if ( userData1 )
      userData1->getContactSignal().trigger( userData1, 
                                             userData0, 
                                             pxCast<Point3F>( hitPoint ),
                                             pxCast<Point3F>( -pair.sumNormalForce ) );
}

bool PxUserNotify::onJointBreak( NxReal breakingForce, NxJoint &brokenJoint )
{
   PROFILE_SCOPE( PxUserNotify_OnJointBreak );

   PxUserData *userData = PxUserData::getData( brokenJoint );   

   if ( userData )
      userData->getOnJointBreakSignal().trigger( breakingForce, brokenJoint );

   // NOTE: Returning true here will tell the
   // PhysX SDK to delete the joint, which will
   // cause MANY problems if any of the user app's
   // objects still hold references to it.

   return false;
}