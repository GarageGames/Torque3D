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

#ifndef _BB_RUNNER_H_
#define _BB_RUNNER_H_

#ifndef _BB_CORE_H_
#include "Core.h"
#endif
#ifndef _BB_SIGNAL_H_
#include "Signal.h"
#endif
#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _TVECTOR_H_
#include "util/tVector.h"
#endif

namespace BadBehavior
{

   //---------------------------------------------------------------------------
   // BehaviorTreeRunner - handles the evaluation of the tree
   //---------------------------------------------------------------------------
   class BehaviorTreeRunner : public SimObject
   {
      typedef SimObject Parent;

   private:
      // is this tree running?
      bool mIsRunning;

      // event ID of the tick event
      U32 mTickEvent;

      // frequency of ticks in ms
      U32 mTickFrequency;

      // the root node of the tree
      SimObjectPtr<Node> mRootNode;
      
      // the task associated with the root node
      Task *mRootTask;

      // the game object that is using this tree
      SimObjectPtr<SimObject> mOwner;

      // signal handler for throwing signals around
      SignalHandler mSignalHandler;

      // setters for the script interface
      static bool _setRootNode( void *object, const char *index, const char *data );
      static bool _setOwner( void *object, const char *index, const char *data );
 
   public:
      /*Ctor*/ BehaviorTreeRunner();
      /*Dtor*/ ~BehaviorTreeRunner();

      // public setters for the script interface
      void setOwner(SimObject *owner);
      void setRootNode(Node *root);

      // notification if our owner is deleted
      virtual void onDeleteNotify(SimObject *object);

      // for script control
      void stop();
      void start();
      void reset();
      void clear();
      bool isRunning();

      // tick
      void onTick();

      // signal handling
      void subscribeToSignal(const char *signal, SignalSubscriber *subscriber);
      void unsubscribeFromSignal(const char *signal, SignalSubscriber *subscriber);
      void postSignal(const char *signal);
      
      // task reactivation
      void onReactivateEvent(Task *task);

      // script interface
      static void initPersistFields();

      DECLARE_CONOBJECT(BehaviorTreeRunner);
   };

   
   class BehaviorTreeTickEvent : public SimEvent
   {
   public:
      void process( SimObject *object )
      {
         ((BehaviorTreeRunner*)object)->onTick();
      }
   };

   class TaskReactivateEvent : public SimEvent
   {
      Task *mTask;
   public:
      TaskReactivateEvent(Task &task) 
      { 
         mTask = &task; 
      }

      void process( SimObject *object )
      {
         ((BehaviorTreeRunner*)object)->onReactivateEvent(mTask);
      }
   };

} // namespace BadBehavior

#endif