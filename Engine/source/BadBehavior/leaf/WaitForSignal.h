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

#ifndef _BB_WAITFORSIGNAL_H_
#define _BB_WAITFORSIGNAL_H_

#ifndef _BB_CORE_H_
#include "BadBehavior/core/Core.h"
#endif
#ifndef _BB_SIGNAL_H_
#include "BadBehavior/core/Signal.h"
#endif

namespace BadBehavior
{
   //---------------------------------------------------------------------------
   // WaitForSignal leaf
   // Waits until it receives the requested signal.
   //---------------------------------------------------------------------------
   class WaitForSignal : public LeafNode
   {
      typedef LeafNode Parent;

   protected:
      String mSignalName;
      S32 mTimeoutMs;

      static bool _setTimeout(void *object, const char *index, const char *data);

   public:
      WaitForSignal();
   
      virtual Task *createTask(SimObject &owner, BehaviorTreeRunner &runner);
      
      static void initPersistFields();

      const String &getSignalName() const { return mSignalName; }
      S32 getTimeoutMs() const { return mTimeoutMs; }

      DECLARE_CONOBJECT(WaitForSignal);
   };

   //---------------------------------------------------------------------------
   // WaitForSignal leaf task
   //---------------------------------------------------------------------------
   class WaitForSignalTask : public Task, public virtual SignalSubscriber
   {
      typedef Task Parent;

   protected:
      U32 mEventId;

      virtual void onInitialize();
      virtual void onTerminate();
      virtual Task* update();

      void cancelEvent();

      // SignalSubscriber
      virtual void subscribe();
      virtual void unsubscribe();

  public:
      WaitForSignalTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner);
      virtual ~WaitForSignalTask();

      // SignalSubscriber
      virtual void onSignal();

      // timeout
      void onTimeout();
   };


   class WaitForSignalTimeoutEvent : public SimEvent
   {
      WaitForSignalTask *mTask;
   public:
      WaitForSignalTimeoutEvent(WaitForSignalTask &task) 
      { 
         mTask = &task; 
      }

      void process( SimObject *object )
      {
         mTask->onTimeout();
      }
   };

} // namespace BadBehavior

#endif