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
#include "Core.h"
#include "Runner.h"

bool gInBtEditor = false;

using namespace BadBehavior;

//------------------------------------------------------------------------------
// script enum for return type
//------------------------------------------------------------------------------
ImplementEnumType( BehaviorReturnType,
   "@brief The return status for a behavior.\n\n"
   "@ingroup AI\n\n")
   // not needed script side 
   //{ BadBehavior::INVALID, "INVALID", "The behavior could not be evaluated.\n" },
   { BadBehavior::SUCCESS, "SUCCESS", "The behavior succeeded.\n" },
   { BadBehavior::FAILURE, "FAILURE", "The behavior failed.\n" },
   { BadBehavior::RUNNING, "RUNNING", "The behavior is still running.\n" },
   // not needed script side 
   //{ BadBehavior::SUSPENDED, "SUSPENDED", "The behavior has been suspended.\n" },
   //{ BadBehavior::RESUME, "RESUME", "The behavior is resuming from suspended.\n" }
EndImplementEnumType;


//================================LeafNode======================================

//------------------------------------------------------------------------------
// don't allow objects to be added
//------------------------------------------------------------------------------
void LeafNode::addObject(SimObject *object)
{
}

bool LeafNode::acceptsAsChild( SimObject *object ) const 
{ 
   return false; 
}


//==================================Task========================================

Task::Task(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : mStatus(INVALID), 
     mIsComplete(false), 
     mNodeRep(&node),
     mOwner(&owner),
     mRunner(&runner),
     mParent(NULL)
{
}

Task::~Task()
{
}

void Task::onInitialize()
{
}

void Task::onTerminate()
{
}

Task* Task::tick()
{
   PROFILE_SCOPE(Task_Tick);
   
   return update();
}

void Task::setup()
{
   PROFILE_SCOPE(Task_setup);
   
   if(mStatus != RUNNING && mStatus != SUSPENDED)
      onInitialize();
   
   mIsComplete = false;
}

void Task::finish()
{
   if(mIsComplete)
      onTerminate();
}

void Task::reset()
{
   mStatus = INVALID;
}

Status Task::getStatus() 
{ 
   return mStatus; 
}

void Task::setStatus(Status newStatus) 
{ 
   mStatus = newStatus; 
}

void Task::setParent(Task *parent)
{
   mParent = parent;
}

Task *Task::getParent()
{
   return mParent;
}

void Task::onChildComplete(Status)
{
}

void Task::onResume()
{ 
   if(mStatus == SUSPENDED)
      mStatus = RESUME;
   
   //Con::warnf("onResume %s", 
   //            mNodeRep->getIdString());
}

DefineEngineFunction(onBehaviorTreeEditorStart, void, (),,
   "@brief Notify the engine that the behavior tree editor is active")
{
   gInBtEditor = true;
}


DefineEngineFunction(onBehaviorTreeEditorStop, void, (),,
   "@brief Notify the engine that the behavior tree editor has finished")
{
   gInBtEditor = false;
}