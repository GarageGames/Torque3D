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

#include "SubTree.h"

using namespace BadBehavior;


//------------------------------------------------------------------------------
// SubTree node - links to another behavior tree
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(SubTree);

SubTree::SubTree()
   : mSubTreeName(0)
{
}

void SubTree::initPersistFields()
{
   addGroup( "Behavior" );

   addField( "subTreeName", TypeString, Offset(mSubTreeName, SubTree),
      "@brief The name of the behavior tree that this node links to.  Max 255 characters." );

   endGroup( "Behavior" );

   Parent::initPersistFields();
}

Task *SubTree::createTask(SimObject &owner, BehaviorTreeRunner &runner)
{
   if(!mSubTreeName || mSubTreeName[0] == 0)
   {
      Con::errorf("SubTree::onInitialize: no sub tree specified");
      return NULL;
   }

   SimObject *subTreeNode;

   if(!Sim::findObject(mSubTreeName, subTreeNode))
   {
      Con::errorf("SubTree:onInitialize: the specified sub tree does not exist");
      return NULL;
   }

   Node *node = dynamic_cast<Node*>(subTreeNode);
   if(!node)
   {
      Con::errorf("SubTree::onInitialize: the specified sub tree is not a behavior tree node");
      return NULL;
   }

   return node->createTask(owner, runner);
}