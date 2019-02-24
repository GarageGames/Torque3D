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

#include "Decorator.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// Base decorator node
// overrides for decorators to only allow 1 child
//------------------------------------------------------------------------------
void DecoratorNode::addObject(SimObject *obj)
{
   if(empty())
      Parent::addObject(obj);
}

bool DecoratorNode::acceptsAsChild( SimObject *object ) const 
{
   return (dynamic_cast<Node *>(object) && empty());
}

//------------------------------------------------------------------------------
// Base decorator task
//------------------------------------------------------------------------------
DecoratorTask::DecoratorTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : Parent(node, owner, runner),
   mChild(NULL)
{
}

DecoratorTask::~DecoratorTask()
{
   if(mChild)
   {
      delete mChild;
      mChild = NULL;
   }
}

void DecoratorTask::onInitialize()
{
   if(!mChild)
   {
      if(mNodeRep->size() > 0)
      {
         Node *childNode = static_cast<Node*>(*mNodeRep->begin());
         if(childNode)
         {
            mChild = childNode->createTask(*mOwner, *mRunner);
            if(mChild)
            {
               mChild->setParent(this);
               mChild->reset();
            }
         }
      }
   }
   
   mStatus = INVALID;
}

void DecoratorTask::onTerminate()
{
   mStatus = INVALID;
}

Task* DecoratorTask::update() 
{ 
   // first time through, return child
   if(!mIsComplete)
      return mStatus != SUSPENDED ? mChild : NULL;
   
   // child has completed, are we latent?
   if(mStatus == RUNNING || mStatus == SUSPENDED)
      mIsComplete = false;
   
   // no more children
   return NULL; 
}
      
void DecoratorTask::onChildComplete(Status s)
{
   // set our status to the child status and flag completed
   mStatus = s;
   mIsComplete = true;
}