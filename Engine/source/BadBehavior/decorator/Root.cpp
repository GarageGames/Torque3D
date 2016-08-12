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

#include "Root.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// Root decorator node
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(Root);

Task *Root::createTask(SimObject &owner, BehaviorTreeRunner &runner)
{
   return new RootTask(*this, owner, runner);
}

//------------------------------------------------------------------------------
// Root decorator task
//------------------------------------------------------------------------------
RootTask::RootTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : Parent(node, owner, runner),
     mBranch(NULL)
{
}

RootTask::~RootTask()
{
   if(mBranch)
      delete mBranch;
}

void RootTask::onInitialize()
{
   Parent::onInitialize();
   if(!mBranch)
      mBranch = new BehaviorTreeBranch(mChild);
   else
      mBranch->reset();
}

Task *RootTask::update()
{
   mStatus = mBranch->update();

   mIsComplete = mStatus != RUNNING && mStatus != SUSPENDED;
   return NULL;
}

Status RootTask::getStatus()
{
   if(mStatus == SUSPENDED)
      return mBranch->getStatus();

   return mStatus;
}