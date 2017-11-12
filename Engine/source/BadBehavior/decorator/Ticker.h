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

#ifndef _BB_TICKER_H_
#define _BB_TICKER_H_

#ifndef _BB_DECORATOR_H_
#include "BadBehavior/core/Decorator.h"
#endif
#ifndef _BB_BRANCH_H_
#include "BadBehavior\core\Branch.h"
#endif

namespace BadBehavior
{
   //---------------------------------------------------------------------------
   // Ticker decorator
   // Limits the frequency at which it's children are updated.
   //---------------------------------------------------------------------------
   class Ticker : public DecoratorNode
   {
      typedef DecoratorNode Parent;

   protected:
      static bool _setFrequency(void *object, const char *index, const char *data);
      
      // time between ticks (in ms)
      S32 mFrequencyMs;

   public:
      Ticker();

      virtual Task *createTask(SimObject &owner, BehaviorTreeRunner &runner);
      
      static void initPersistFields();

      S32 getFrequencyMs() const { return mFrequencyMs; }

      DECLARE_CONOBJECT(Ticker);
   };

   //---------------------------------------------------------------------------
   // Ticker decorator task
   //---------------------------------------------------------------------------
   class TickerTask : public DecoratorTask
   {
      typedef DecoratorTask Parent;

   protected:
      U32 mNextTimeMs;
      U32 mEventId;

      BehaviorTreeBranch *mBranch;

      virtual void onInitialize();
      virtual void onTerminate();
      virtual Task* update();
      
      void cancelEvent();
      
   public:
      TickerTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner);
      virtual ~TickerTask();

      virtual Status getStatus();
   };

} // namespace BadBehavior

#endif