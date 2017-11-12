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

#include "BadBehavior\core\Runner.h"
#include "Wait.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// Wait leaf node
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(Wait);

Wait::Wait() 
   : mWaitMs(1000)
{
}

void Wait::initPersistFields()
{
   addGroup( "Behavior" );
   
   addProtectedField( "waitMs", TypeS32, Offset(mWaitMs, Wait), &_setWait, &defaultProtectedGetFn,
      "The time in ms that the node should wait before completion." );

   endGroup( "Behavior" );

   Parent::initPersistFields();
}

bool Wait::_setWait(void *object, const char *index, const char *data)
{
   Wait *node = static_cast<Wait *>( object );
   node->mWaitMs = getMax(0, dAtoi( data ));
   return false;
}

Task *Wait::createTask(SimObject &owner, BehaviorTreeRunner &runner)
{
   return new WaitTask(*this, owner, runner);
}

//------------------------------------------------------------------------------
// Wait task
//------------------------------------------------------------------------------
WaitTask::WaitTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : Parent(node, owner, runner),
   mEventId(0)
{
}

WaitTask::~WaitTask()
{
   cancelEvent();
}

void WaitTask::cancelEvent()
{
   if(Sim::isEventPending(mEventId))
   {
      Sim::cancelEvent(mEventId);
      mEventId = 0;
   }
}

void WaitTask::onInitialize()
{
   Parent::onInitialize();
   cancelEvent();
}

void WaitTask::onTerminate()
{
   Parent::onTerminate();
   cancelEvent();
}

Task* WaitTask::update() 
{ 
   if(mStatus == RESUME)
   {
      mStatus = SUCCESS;
      mIsComplete = true;
   }
   else if(mStatus == INVALID)
   {
      mEventId = Sim::postEvent(mRunner, new TaskReactivateEvent(*this), Sim::getCurrentTime() + static_cast<Wait*>(mNodeRep)->getWaitMs());
      mStatus = SUSPENDED;
   }

   return NULL; 
}
