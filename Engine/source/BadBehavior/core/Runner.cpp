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

#include "Runner.h"


using namespace BadBehavior;

IMPLEMENT_CONOBJECT(BehaviorTreeRunner);

BehaviorTreeRunner::BehaviorTreeRunner()
   : mOwner(NULL), 
     mRootNode(NULL), 
     mRootTask(NULL),
     mIsRunning(0),
     mTickEvent(0),
     mTickFrequency(100)
{}


BehaviorTreeRunner::~BehaviorTreeRunner()
{
   delete mRootTask;

   if(Sim::isEventPending(mTickEvent))
   {
      Sim::cancelEvent(mTickEvent);
      mTickEvent = 0;
   }
}


void BehaviorTreeRunner::initPersistFields()
{
   addGroup( "Behavior" );

   addProtectedField("rootNode", TYPEID< SimObject >(), Offset(mRootNode, BehaviorTreeRunner),
      &_setRootNode, &defaultProtectedGetFn, "@brief The root node of the tree to be run.");

   addProtectedField("ownerObject", TYPEID< SimObject >(), Offset(mOwner, BehaviorTreeRunner),
      &_setOwner, &defaultProtectedGetFn, "@brief The object that owns the tree to be run.");

   addField("frequency", TypeS32, Offset(mTickFrequency, BehaviorTreeRunner),
      "@brief The frequency in ms that the tree is ticked at.");

   endGroup( "Behavior" );

   Parent::initPersistFields();
}

void BehaviorTreeRunner::onDeleteNotify(SimObject *object)
{
   // delete ourselves nicely
   // - this takes care of any events registered to this runner
   if(object == mOwner.getObject())
      safeDeleteObject();
}


// processTick is where the magic happens :)
void BehaviorTreeRunner::onTick()
{
   PROFILE_SCOPE(BehaviorTreeRunner_processTick);

   // check that we are setup to run
   if(mOwner.isNull() || mRootNode.isNull())
      return;

   if(gInBtEditor)
   {
      if(mRootTask)
         reset();
   }
   else
   {
      if(!mRootTask)
      {
         if((mRootTask = mRootNode->createTask(*mOwner, *this)) == NULL)
         {
            Con::errorf("BehaviorTreeTicker::processTick, no task for root node");
            return;
         }
      }

      // dispatch any signals
      mSignalHandler.dispatchSignals();

      // Evaluate the tree
      mRootTask->setup();
      mRootTask->tick();
      //Con::warnf("Tree returned %s", EngineMarshallData(mRootTask->getStatus()));
      mRootTask->finish();
   }

   // schedule the next tick
   if(Sim::isEventPending(mTickEvent))
      Sim::cancelEvent(mTickEvent);

   mTickEvent = Sim::postEvent(this, new BehaviorTreeTickEvent(), Sim::getCurrentTime() + mTickFrequency);
   
   mIsRunning = true;
}

void BehaviorTreeRunner::onReactivateEvent(Task *task)
{
   if(task)
      task->onResume();
}

bool BehaviorTreeRunner::_setRootNode( void *object, const char *index, const char *data )
{
   BehaviorTreeRunner *runner = static_cast<BehaviorTreeRunner *>( object );   
   Node* root = NULL;
   Sim::findObject( data, root );
   if(root)
      runner->setRootNode(root);
   return false;
}

bool BehaviorTreeRunner::_setOwner( void *object, const char *index, const char *data )
{
   BehaviorTreeRunner *runner = static_cast<BehaviorTreeRunner *>( object );   
   SimObject* owner = NULL;
   Sim::findObject( data, owner );
   if(owner)
      runner->setOwner(owner);
   return false;
}

void BehaviorTreeRunner::setOwner(SimObject *owner) 
{ 
   reset();
   mOwner = owner; 
   deleteNotify(mOwner);
   start();
}


void BehaviorTreeRunner::setRootNode(Node *root) 
{ 
   reset();
   mRootNode = root;
   start();
}


void BehaviorTreeRunner::stop()
{
   if(Sim::isEventPending(mTickEvent))
   {
      Sim::cancelEvent(mTickEvent);
      mTickEvent = 0;
   }
   mIsRunning = false;
}


void BehaviorTreeRunner::start()
{
   if(Sim::isEventPending(mTickEvent))
   {
      Sim::cancelEvent(mTickEvent);
      mTickEvent = 0;
   }
   
   mIsRunning = true;
   if(mRootTask)
      mRootTask->reset();

   mTickEvent = Sim::postEvent(this, new BehaviorTreeTickEvent(), -1);
}


void BehaviorTreeRunner::reset()
{
   //stop();
   if(mRootTask)
   {
      delete mRootTask;
      mRootTask = 0;
   }
   mSignalHandler.reset();
}


void BehaviorTreeRunner::clear()
{
   reset();
   mRootNode = 0;
}


bool BehaviorTreeRunner::isRunning()
{
   return mIsRunning;
}


void BehaviorTreeRunner::subscribeToSignal(const char *signal, SignalSubscriber *subscriber)
{
   mSignalHandler.registerSubscriber(signal, subscriber);
}


void BehaviorTreeRunner::unsubscribeFromSignal(const char *signal, SignalSubscriber *subscriber)
{
   mSignalHandler.unregisterSubscriber(signal, subscriber);
}


void BehaviorTreeRunner::postSignal(const char *signal)
{
   mSignalHandler.postSignal(signal);
}


DefineEngineMethod( BehaviorTreeRunner, stop, void, (), ,
                   "Halt the execution of the behavior tree.\n\n"
                   "@note The internal task status is retained, allowing execution to be resumed.")
{
   object->stop();
}


DefineEngineMethod( BehaviorTreeRunner, start, void, (), ,
                  "Resume execution of the (stopped) behavior tree.")
{
   object->start();
}


DefineEngineMethod( BehaviorTreeRunner, reset, void, (), ,
                    "Reset the behavior tree. Any internal state is lost.")
{
   object->reset();
}


DefineEngineMethod( BehaviorTreeRunner, clear, void, (), ,
                    "Clear the behavior tree.")
{
   object->clear();
}


DefineEngineMethod( BehaviorTreeRunner, isRunning, bool, (), ,
                    "Is the behavior tree running")
{
   return object->isRunning();
}


DefineEngineMethod(BehaviorTreeRunner, postSignal, void, (const char *signal),, 
   "@brief Posts a signal to the behavior tree.\n\n")
{
   object->postSignal( signal );
}