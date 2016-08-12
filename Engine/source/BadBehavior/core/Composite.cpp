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

#include "Composite.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// Base composite node
// override addObject to only allow behavior tree nodes to be added
//------------------------------------------------------------------------------
void CompositeNode::addObject(SimObject *object)
{
   if(dynamic_cast<Node*>(object))
      Parent::addObject(object);
}

bool CompositeNode::acceptsAsChild( SimObject *object ) const 
{ 
   return (dynamic_cast<Node*>(object)); 
}

//------------------------------------------------------------------------------
// Base composite task
//------------------------------------------------------------------------------
CompositeTask::CompositeTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner) 
   : Parent(node, owner, runner) 
{
}

CompositeTask::~CompositeTask()
{
   while(mChildren.size())
   {
      Task *child = mChildren.back();
      mChildren.pop_back();
      if(child)
         delete child;
   }
}

void CompositeTask::onInitialize()
{
   if(mChildren.empty())
   {
      CompositeNode *node = static_cast<CompositeNode *>(mNodeRep);
      for(SimSet::iterator i = node->begin(); i != node->end(); ++i)
      {
         Task *task = static_cast<Node*>(*i)->createTask(*mOwner, *mRunner);
         if(task)
         {
            task->setParent(this);
            mChildren.push_back(task);
         }
      }
   }
   
   mStatus = INVALID;
   mCurrentChild = mChildren.begin();
   if(mCurrentChild != mChildren.end())
      (*mCurrentChild)->reset();
}

void CompositeTask::onTerminate()
{
   mStatus = INVALID;
}

Task* CompositeTask::update()
{
   // reached the end of child list, we are complete
   if (mCurrentChild == mChildren.end())
      mIsComplete = true;

   // task has finished
   if( mIsComplete )
   {
      // are we latent?
      if(mStatus == RUNNING || mStatus == SUSPENDED)
         mIsComplete = false;

      return NULL;
   }

   // reset the child ready for the next tick
   if(mStatus != RUNNING && mStatus != SUSPENDED)
      (*mCurrentChild)->reset();

   // return child
   return (*mCurrentChild);   
}