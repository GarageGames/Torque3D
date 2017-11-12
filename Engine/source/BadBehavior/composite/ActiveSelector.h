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

#ifndef _BB_ACTIVESELECTOR_H_
#define _BB_ACTIVESELECTOR_H_

#ifndef _BB_CORE_H_
#include "BadBehavior/core/Composite.h"
#endif
#ifndef _BB_BRANCH_H_
#include "BadBehavior/core/Branch.h"
#endif

//==============================================================================
// Active Selector
// Re-evaluates its children from the beginning each tick. Lower priority
// children which previously returned RUNNING are resumed if re-selected
//
// ***** TODO - This runs OK, but may need a bit more work
// -- abort previously running branches?
//
//==============================================================================

namespace BadBehavior
{
   //---------------------------------------------------------------------------
   // Active selector Node
   //---------------------------------------------------------------------------
   class ActiveSelector : public CompositeNode
   {
      typedef CompositeNode Parent;

   protected:
      U32 mRecheckFrequency;
         
   public:
      ActiveSelector();

      virtual Task *createTask(SimObject &owner, BehaviorTreeRunner &runner);

      static void initPersistFields();

      U32 getRecheckFrequency() const { return mRecheckFrequency; }

      DECLARE_CONOBJECT(ActiveSelector);
   };

   //---------------------------------------------------------------------------
   // Active selector Task
   //---------------------------------------------------------------------------
   class ActiveSelectorTask : public CompositeTask
   {
      typedef CompositeTask Parent;

   protected:
      Vector<BehaviorTreeBranch>::iterator mRunningBranch;
      Vector<BehaviorTreeBranch>::iterator mCurrentBranch;
      Vector<BehaviorTreeBranch> mBranches;

      U32 mRecheckTime;
      
      virtual void onInitialize();
      virtual Task* update();
   
   public:
      ActiveSelectorTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner);

      Status getStatus();
};

} // namespace BadBehavior

#endif