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

#include "ActiveSelector.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// Active selector node
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(ActiveSelector);

ActiveSelector::ActiveSelector()
   :  mRecheckFrequency(0)
{
}

Task *ActiveSelector::createTask(SimObject &owner, BehaviorTreeRunner &runner)
{
   return new ActiveSelectorTask(*this, owner, runner);
}

void ActiveSelector::initPersistFields()
{
   addGroup( "Behavior" );

   addField( "recheckFrequency", TypeS32, Offset(mRecheckFrequency, ActiveSelector),
      "@brief The minimum time period in milliseconds to wait between re-evaluations of higher priority branches.");

   endGroup( "Behavior" );

   Parent::initPersistFields();
}

//------------------------------------------------------------------------------
// Active selector task
//------------------------------------------------------------------------------
ActiveSelectorTask::ActiveSelectorTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : Parent(node, owner, runner),
   mRecheckTime(0)
{
}

void ActiveSelectorTask::onInitialize()
{
   Parent::onInitialize();

   if(mBranches.empty())
   {
      for (VectorPtr<Task*>::iterator i = mChildren.begin(); i != mChildren.end(); ++i)
      {
         mBranches.push_back(BehaviorTreeBranch(*i));
      }
   }

   mCurrentBranch = mBranches.begin();
   mRunningBranch = mBranches.end();
   mRecheckTime = 0;
}


Task* ActiveSelectorTask::update()
{
   // empty node, bail
   if(mBranches.empty())
   {
      mStatus = INVALID;
      return NULL;
   }

   // is it time to re-check higher priority branches?
   if(Sim::getCurrentTime() >= mRecheckTime)
   {
      // pick highest priority branch
      mCurrentBranch = mBranches.begin();
      
      // determine the next recheck time
      mRecheckTime = Sim::getCurrentTime() + static_cast<ActiveSelector *>(mNodeRep)->getRecheckFrequency();
   }

   // run a branch, if it fails move on to the next
   for(mCurrentBranch; mCurrentBranch != mBranches.end(); ++mCurrentBranch)
   {
      // reset the branch if it's not the current running branch
      if(mCurrentBranch != mRunningBranch)
         mCurrentBranch->reset();

      mStatus = mCurrentBranch->update();

      if(mStatus == FAILURE) // move on to next
         continue;

      if(mStatus == RUNNING || mStatus == SUSPENDED) // track the current running branch
         mRunningBranch = mCurrentBranch;
      
      break;
   }

   if( (mStatus != RUNNING && mStatus != SUSPENDED) || mCurrentBranch == mBranches.end() )
      mIsComplete = true;

   return NULL;
}

Status ActiveSelectorTask::getStatus()
{
   if(mStatus == SUSPENDED && mCurrentBranch != mBranches.end())
      return mCurrentBranch->getStatus(); // suspended branch may have resumed

   return mStatus;
}
