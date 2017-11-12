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

#include "RandomSelector.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// Random selector node
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(RandomSelector);

Task *RandomSelector::createTask(SimObject &owner, BehaviorTreeRunner &runner)
{
   return new RandomSelectorTask(*this, owner, runner);
}

//------------------------------------------------------------------------------
// Random selector task
//------------------------------------------------------------------------------
RandomSelectorTask::RandomSelectorTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : Parent(node, owner, runner)
{
}

void RandomSelectorTask::onInitialize()
{
   Parent::onInitialize();

   // randomize the order of our child tasks
   VectorPtr<Task *> randomChildren;

   while(mChildren.size() > 0)
   {
      U32 index = mRandI(0, mChildren.size() - 1);
      Task* child = mChildren[index];
      randomChildren.push_back(child);
      mChildren.erase_fast(index);
   }

   mChildren = randomChildren;

   // normal init
   mCurrentChild = mChildren.begin();
   if(mCurrentChild != mChildren.end())
      (*mCurrentChild)->reset();
}
