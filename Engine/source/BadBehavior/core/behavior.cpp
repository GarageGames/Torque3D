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

#include "console/engineAPI.h"
#include "platform/profiler.h"

#include "behavior.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// script enum for precondition mode
//------------------------------------------------------------------------------
ImplementEnumType( BehaviorPreconditionType,
   "@brief When should the precondition function be evaluated.\n\n"
   "@ingroup AI\n\n")
   { ONCE, "ONCE", "The first time the node is executed.\n" },
   { TICK, "TICK", "Each time the node is executed.\n" },
EndImplementEnumType;

//------------------------------------------------------------------------------
// Behavior node
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(Behavior);

Behavior::Behavior()
   : mPreconditionMode(ONCE)
{
}

void Behavior::initPersistFields()
{
   addGroup( "Behavior" );

   addField( "preconditionMode", TYPEID< BadBehavior::PreconditionMode >(), Offset(mPreconditionMode, Behavior),
      "@brief When to evaluate the precondition function.");

   endGroup( "Behavior" );

   Parent::initPersistFields();
}

Task *Behavior::createTask(SimObject &owner, BehaviorTreeRunner &runner)
{
   return new BehaviorTask(*this, owner, runner);
}

//------------------------------------------------------------------------------
// ScriptedBehavior task
//------------------------------------------------------------------------------
BehaviorTask::BehaviorTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : Parent(node, owner, runner)
{
}

Task* BehaviorTask::update()
{  
   PROFILE_SCOPE(BehaviorTask_update);

   Behavior *node = static_cast<Behavior*>(mNodeRep);
   
   // first check preconditions are valid
   bool precondition = true;
   if( (node->getPreconditionMode() == ONCE && mStatus == INVALID) || (node->getPreconditionMode() == TICK) )
      precondition = node->precondition( mOwner );
   
   if(precondition)
   {
      // run onEnter if this is the first time the node is ticked
      if(mStatus == INVALID)
         node->onEnter(mOwner);
      
      // execute the main behavior and get its return value
      mStatus = node->behavior(mOwner);
   }
   else
   {
      mStatus = FAILURE;
   }

   mIsComplete = mStatus != RUNNING && mStatus != SUSPENDED;

   if(mIsComplete)
      node->onExit(mOwner);

   return NULL; // leaves don't have children
}
