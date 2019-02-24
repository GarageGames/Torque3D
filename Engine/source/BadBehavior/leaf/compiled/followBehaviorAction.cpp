//-----------------------------------------------------------------------------
// Copyright (c) 2014 Guy Allard
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

#include "FollowBehaviorAction.h"

#include "T3D/aiPlayer.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// FollowBehavior node
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(FollowBehaviorAction);

FollowBehaviorAction::FollowBehaviorAction()
{
}

bool FollowBehaviorAction::precondition( SimObject *owner )
{
   PROFILE_SCOPE( FollowBehaviorAction_precondition);

   // check that our owner is an AIPlayer
   AIPlayer *aiPlayer = dynamic_cast<AIPlayer *>(owner);
   if(!aiPlayer)
      return false; 
   
   return true;
}

void FollowBehaviorAction::onEnter( SimObject *owner )
{
   //PROFILE_SCOPE( FollowBehaviorAction_onEnter );
}

void FollowBehaviorAction::onExit( SimObject *owner )
{
   //PROFILE_SCOPE( FollowBehaviorAction_onExit );
}

Status FollowBehaviorAction::behavior( SimObject *owner )
{
   PROFILE_SCOPE( FollowBehaviorAction_behavior );

   // get the owning AIPlayer object
   AIPlayer *aiPlayer = dynamic_cast<AIPlayer *>(owner);
   if(!aiPlayer)
      return FAILURE;

   // get the script-specified followObject
   const char *followFieldName = StringTable->insert("followObject");
   const char *followFieldValue = owner->getDataField(followFieldName, NULL);
   if(!followFieldValue || !followFieldValue[0])
      return FAILURE;

   SimObject *followSimObj = Sim::findObject(dAtoi(followFieldValue));
   if(!followSimObj)
      return FAILURE;

   // check that the follow object is a SceneObject
   SceneObject *followObject = dynamic_cast<SceneObject *>(followSimObj);
   if(!followObject)
      return FAILURE;

   // get the script-specified followDistance field
   const char *distanceFieldName = StringTable->insert("followDistance"); 
   const char *distanceFieldValue = owner->getDataField(distanceFieldName, NULL);
   float followDistance = 1.f;
   if(distanceFieldValue && distanceFieldValue[0])
      followDistance = dAtof(distanceFieldValue);

   // try and stay at followDistance from the followObject
   Point3F targetPos = followObject->getPosition();
   Point3F followVec = aiPlayer->getPosition() - targetPos;
   
   // get current distance (ignore z component)
   F32 curDist = Point3F(followVec.x, followVec.y, 0.f).len();

   if(mFabs(curDist - followDistance) > aiPlayer->getMoveTolerance())
   {
      followVec.normalize();
      followVec *= followDistance;
      Point3F destination = targetPos + followVec;
   
      aiPlayer->setMoveDestination(destination, true);
      return RUNNING;
   }

   return SUCCESS;
}