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

#include "math/mMathFn.h"

#include "BadBehavior/core/Runner.h"
#include "RandomWait.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// RandomWait leaf node
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(RandomWait);

RandomWait::RandomWait() 
   : mWaitMinMs(0), 
     mWaitMaxMs(99999) 
{
}

void RandomWait::initPersistFields()
{
   addGroup( "Behavior" );
   
   addProtectedField( "waitMinMs", TypeS32, Offset(mWaitMinMs, RandomWait), &_setWaitMin, &defaultProtectedGetFn,
      "The minimum time period in ms to wait before completion." );

   addProtectedField( "waitMaxMs", TypeS32, Offset(mWaitMaxMs, RandomWait), &_setWaitMax, &defaultProtectedGetFn,
      "The maximum time period in ms to wait before completion." );

   endGroup( "Behavior" );

   Parent::initPersistFields();
}

bool RandomWait::_setWaitMin(void *object, const char *index, const char *data)
{
   RandomWait *node = static_cast<RandomWait *>( object );
   node->mWaitMinMs = getMin(node->mWaitMaxMs, dAtoi( data ));
   return false;
}

bool RandomWait::_setWaitMax(void *object, const char *index, const char *data)
{
   RandomWait *node = static_cast<RandomWait *>( object );
   node->mWaitMaxMs = getMax(node->mWaitMinMs, dAtoi( data ));
   return false;
}

Task *RandomWait::createTask(SimObject &owner, BehaviorTreeRunner &runner)
{
   return new RandomWaitTask(*this, owner, runner);
}

//------------------------------------------------------------------------------
// RandomWait task
//------------------------------------------------------------------------------
RandomWaitTask::RandomWaitTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : Parent(node, owner, runner) 
{
}

RandomWaitTask::~RandomWaitTask()
{
   cancelEvent();
}

void RandomWaitTask::cancelEvent()
{
   if(Sim::isEventPending(mEventId))
   {
      Sim::cancelEvent(mEventId);
      mEventId = 0;
   }
}

void RandomWaitTask::onInitialize()
{
   Parent::onInitialize();
   cancelEvent();
}

void RandomWaitTask::onTerminate()
{
   Parent::onTerminate();
   cancelEvent();
}

Task* RandomWaitTask::update() 
{
   if(mStatus == RESUME)
   {
      mStatus = SUCCESS;
      mIsComplete = true;
   }
   else if(mStatus == INVALID)
   {
      RandomWait *node = static_cast<RandomWait*>(mNodeRep);
      mEventId = Sim::postEvent(mRunner, new TaskReactivateEvent(*this), Sim::getCurrentTime() + mRandI(node->getWaitMinMs(), node->getWaitMaxMs()));
      mStatus = SUSPENDED;
   }

   return NULL; 
}
