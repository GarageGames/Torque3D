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

#include "Loop.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// Loop decorator node
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(Loop);

ImplementEnumType( LoopTerminationPolicy,
   "@brief The policy to use when determining if the loop should terminate early.\n\n"
   "@ingroup AI\n\n")
   { Loop::ON_FAILURE,  "ON_FAILURE",  "Will terminate and return FAILURE if child fails.\n" },
   { Loop::ON_SUCCESS,  "ON_SUCCESS",  "Will terminate and return SUCCESS if child succeeds.\n" },
EndImplementEnumType;

Loop::Loop()
   : mNumLoops(0), 
     mTerminationPolicy(ON_FAILURE) 
{
}

void Loop::initPersistFields()
{
   addGroup( "Behavior" );
   
   addProtectedField( "numLoops", TypeS32, Offset(mNumLoops, Loop), &_setNumLoops, &defaultProtectedGetFn,
      "The number of times to repeat the child behavior. 0 = infinite." );

   addField( "terminationPolicy", TYPEID< Loop::TerminationPolicy >(), Offset(mTerminationPolicy, Loop),
      "@brief The policy to use when deciding if the loop should terminate before completion.");
   
   endGroup( "Behavior" );

   Parent::initPersistFields();
}

bool Loop::_setNumLoops(void *object, const char *index, const char *data)
{
   Loop *node = static_cast<Loop *>( object );
   node->mNumLoops = getMax(0, dAtoi( data ));
   return false;
}

Task *Loop::createTask(SimObject &owner, BehaviorTreeRunner &runner)
{
   return new LoopTask(*this, owner, runner);
}

//------------------------------------------------------------------------------
// Loop decorator task
//------------------------------------------------------------------------------
LoopTask::LoopTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : Parent(node, owner, runner), 
     mCurrentLoop(0)
{
}

void LoopTask::onInitialize()
{
   Parent::onInitialize();
   mCurrentLoop = 0;
}

Task* LoopTask::update()
{
   if(Parent::update())
      return mChild;

   if(mStatus == RUNNING || mStatus == SUSPENDED)
      mIsComplete = false;

   // child has terminated with SUCCESS or FAILURE
   if( mIsComplete )
   {
      // check if we should continue looping or reset
      Loop *nodeRep = static_cast<Loop *>(mNodeRep);
      Loop::TerminationPolicy policy = nodeRep->getTerminationPolicy();
      S32 numLoops = nodeRep->getNumLoops();
      
      // termination policy not met?
      if( ((policy == Loop::ON_FAILURE) && (mStatus != FAILURE)) ||
          ((policy == Loop::ON_SUCCESS) && (mStatus != SUCCESS)) )
      {
         // more looping to be done
         if ( (++mCurrentLoop < numLoops) || (numLoops == 0) )
         {
            mIsComplete = false;
            mStatus = RUNNING;
         }
      }
   }
   
   // no children to return
   return NULL;
}
