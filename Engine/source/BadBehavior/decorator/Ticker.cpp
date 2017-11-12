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

#include "BadBehavior/core/Runner.h"
#include "Ticker.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// Ticker decorator node
// **** EXPERIMENTAL ****
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(Ticker);

Ticker::Ticker()
   : mFrequencyMs(100)
{
}

void Ticker::initPersistFields()
{
   addGroup( "Behavior" );
   
   addProtectedField( "frequencyMs", TypeS32, Offset(mFrequencyMs, Ticker), &_setFrequency, &defaultProtectedGetFn,
      "The time to wait between evaluations of this nodes child." );

   endGroup( "Behavior" );

   Parent::initPersistFields();
}

bool Ticker::_setFrequency(void *object, const char *index, const char *data)
{
   Ticker *node = static_cast<Ticker *>( object );
   node->mFrequencyMs = getMax(0, dAtoi( data ));
   return false;
}

Task *Ticker::createTask(SimObject &owner, BehaviorTreeRunner &runner)
{
   return new TickerTask(*this, owner, runner);
}

//------------------------------------------------------------------------------
// Ticker decorator task
//------------------------------------------------------------------------------
TickerTask::TickerTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : Parent(node, owner, runner), 
     mNextTimeMs(0),
     mEventId(0),
     mBranch(NULL)
{
}

TickerTask::~TickerTask()
{
   cancelEvent();

   if(mBranch)
      delete mBranch;
}

void TickerTask::cancelEvent()
{
   if(Sim::isEventPending(mEventId))
   {
      Sim::cancelEvent(mEventId);
      mEventId = 0;
   }
}

void TickerTask::onInitialize()
{
   Parent::onInitialize();
   cancelEvent();
   if(!mBranch)
      mBranch = new BehaviorTreeBranch(mChild);
   else
      mBranch->reset();
}

void TickerTask::onTerminate()
{
   Parent::onTerminate();
   cancelEvent();
}

Task* TickerTask::update() 
{
   if(Sim::getCurrentTime() < mNextTimeMs)
   {
      if(!Sim::isEventPending(mEventId))
         mEventId = Sim::postEvent(mRunner, new TaskReactivateEvent(*this), mNextTimeMs);
      
      mStatus = SUSPENDED;
   }
   else if(mStatus != SUSPENDED)
   {
      mNextTimeMs = Sim::getCurrentTime() + static_cast<Ticker *>(mNodeRep)->getFrequencyMs();
      Status s = mBranch->update();
      mStatus = s != SUSPENDED ? s : RUNNING;
   }

   mIsComplete = mStatus != SUSPENDED && mStatus != RUNNING;

   return NULL;
}

Status TickerTask::getStatus()
{
   if(mStatus != SUSPENDED && mStatus != RESUME)
      return mBranch->getStatus();

   return mStatus;
}