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

#ifndef _BB_PARALLEL_H_
#define _BB_PARALLEL_H_

#ifndef _BB_CORE_H_
#include "BadBehavior/core/Composite.h"
#endif

#ifndef _BB_BRANCH_H_
#include "BadBehavior/core/Branch.h"
#endif

namespace BadBehavior
{
   //---------------------------------------------------------------------------
   // Parallel sequence node
   // Runs all of its children irrespective of their return status
   // The final return status depends on the chosen policy
   // (not a true parallel, as branches are actually evaluated sequentially)
   //---------------------------------------------------------------------------
   class Parallel: public CompositeNode
   {
      typedef CompositeNode Parent;

   public:
      // parallel return policies
      enum ParallelPolicy
      {
         REQUIRE_NONE,
         REQUIRE_ONE,
         REQUIRE_ALL,
      };
      
   protected:
      ParallelPolicy mReturnPolicy;

   public:
      Parallel();

      virtual Task *createTask(SimObject &owner, BehaviorTreeRunner &runner);

      static void initPersistFields();
      
      ParallelPolicy getReturnPolicy() const { return mReturnPolicy; }

      DECLARE_CONOBJECT(Parallel);
   };

   //---------------------------------------------------------------------------
   // Parallel Task
   //---------------------------------------------------------------------------
   class ParallelTask: public CompositeTask
   {
      typedef CompositeTask Parent;

   protected:
      Vector<BehaviorTreeBranch> mBranches;

      bool mHasSuccess, mHasFailure;
      
      virtual void onInitialize();
      virtual Task* update();

   public:
      ParallelTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner);

      virtual Status getStatus();
   };

} // namespace BadBehavior

// make the return policy enum accessible from script
typedef BadBehavior::Parallel::ParallelPolicy ParallelReturnPolicy;
DefineEnumType( ParallelReturnPolicy );

#endif