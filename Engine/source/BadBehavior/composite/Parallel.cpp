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

#include "Parallel.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// Parallel node
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(Parallel);

Parallel::Parallel() 
   : mReturnPolicy(REQUIRE_ALL) 
{
}

ImplementEnumType( ParallelReturnPolicy,
   "@brief The policy to use when determining the return status of a parallel node.\n\n"
   "@ingroup AI\n\n")
   { Parallel::REQUIRE_ALL,  "REQUIRE_ALL",  "Will return success if all children succeed.\n"
                                             "Will terminate and return failure if one child fails \n"},
   { Parallel::REQUIRE_NONE, "REQUIRE_NONE", "Will return success even if all children fail.\n" },
   { Parallel::REQUIRE_ONE,  "REQUIRE_ONE",  "Will terminate and return success when one child succeeds.\n" },
EndImplementEnumType;

void Parallel::initPersistFields()
{
   addGroup( "Behavior" );

   addField( "returnPolicy", TYPEID< Parallel::ParallelPolicy >(), Offset(mReturnPolicy, Parallel),
      "@brief The policy to use when deciding the return status for the parallel sequence.");

   endGroup( "Behavior" );

   Parent::initPersistFields();
}

Task *Parallel::createTask(SimObject &owner, BehaviorTreeRunner &runner)
{
   return new ParallelTask(*this, owner, runner);
}

//------------------------------------------------------------------------------
// Parallel Task
//------------------------------------------------------------------------------
ParallelTask::ParallelTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : Parent(node, owner, runner),
   mHasSuccess(false),
   mHasFailure(false)
{
}

void ParallelTask::onInitialize()
{
   Parent::onInitialize();

   mHasSuccess = mHasFailure = false;

   if(mBranches.empty())
   {
      for (VectorPtr<Task*>::iterator i = mChildren.begin(); i != mChildren.end(); ++i)
      {
         mBranches.push_back(BehaviorTreeBranch(*i));
      }
   }
   else
   {
      for (Vector<BehaviorTreeBranch>::iterator it = mBranches.begin(); it != mBranches.end(); ++it)
      {
         it->reset();
      }
   }
}

Task* ParallelTask::update()
{
   bool hasRunning = false, hasSuspended = false,  hasResume = false;
   for (Vector<BehaviorTreeBranch>::iterator it = mBranches.begin(); it != mBranches.end(); ++it)
   {
      Status s = it->getStatus();
      if(s == INVALID || s == RUNNING || s == RESUME)
      {
         s = it->update();

         switch(it->getStatus())
         {
         case SUCCESS:
            mHasSuccess = true;
            break;
         case FAILURE:
            mHasFailure = true;
            break;
         case RUNNING:
            hasRunning = true;
            break;
         case SUSPENDED:
            hasSuspended = true;
            break;
         case RESUME:
            hasResume = true;
            break;
         }
      }
   }

   switch(static_cast<Parallel *>(mNodeRep)->getReturnPolicy())
   {
   // REQUIRE_NONE
   // returns SUCCESS when all children have finished irrespective of their return status.
   case Parallel::REQUIRE_NONE:
      mStatus = hasResume ? RESUME : ( hasRunning ? RUNNING : ( hasSuspended ? SUSPENDED : SUCCESS ) );
      break;
      
   // REQUIRE_ONE
   // terminates and returns SUCCESS when any of its children succeed
   // returns FAILURE if no children succeed
   case Parallel::REQUIRE_ONE:
      mStatus = mHasSuccess ? SUCCESS : ( hasResume ? RESUME : ( hasRunning ? RUNNING : ( hasSuspended ? SUSPENDED : FAILURE ) ) );
      break;

   // REQUIRE_ALL
   // returns SUCCESS if all of its children succeed.
   // terminates and returns failure if any of its children fail
   case Parallel::REQUIRE_ALL:
      mStatus = mHasFailure ? FAILURE : ( hasResume ? RESUME : ( hasRunning ? RUNNING : ( hasSuspended ? SUSPENDED : SUCCESS ) ) );
      break;
   }

   mIsComplete = (mStatus != RUNNING && mStatus != SUSPENDED && mStatus != RESUME);

   return NULL;
}


Status ParallelTask::getStatus()
{
   if(mStatus == SUSPENDED)
   {
      // need to check if the parallel is still suspended.
      // A parallel will only report SUSPENDED when all of its children are suspended
      for(Vector<BehaviorTreeBranch>::iterator it = mBranches.begin(); it != mBranches.end(); ++it)
      {
         switch(it->getStatus())
         {
         case RUNNING:
            return RUNNING;
         case RESUME:
            return RESUME;
         }
      }
   }
   return mStatus;
}