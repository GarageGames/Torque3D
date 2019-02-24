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

#include "core/stringTable.h"

#include "Signal.h"

using namespace BadBehavior;

// clean up
SignalHandler::~SignalHandler()
{
   clearSignalQueue();
   clearSubscribers();
}

void SignalHandler::clearSignalQueue()
{
   while(!mSignalQueue.empty())
   {
      Signal *sig = mSignalQueue.back();
      mSignalQueue.pop_back();
      delete sig;
      sig = NULL;
   }
}

void SignalHandler::clearSubscribers()
{
   for( Vector<StringTableEntry>::const_iterator it = mSignals.begin(); it != mSignals.end(); ++it )
   {
      // Delete the subscriber list.
      VectorPtr<SignalSubscriber*>* subscribers = mSubscribers.remove( *it );
      if(subscribers)
         delete subscribers;
   }
}

void SignalHandler::reset()
{
   // empty out the signal queue
   clearSignalQueue();

   // clear out the subscribers table
   clearSubscribers();
   
   // reset the vector of registered signals
   mSignals.clear();
}

// check if the specified signal is registered
bool SignalHandler::isSignalRegistered(const char *signal)
{
   StringTableEntry signalName = StringTable->insert( signal );
   return mSignals.contains(signalName);
}

// register a signal
void SignalHandler::registerSignal(const char *signal)
{
   // check signal has a name
   if(!signal || !signal[0])
      return;

   // Make sure the signal has not been registered yet.
   if(!isSignalRegistered( signal ) )
   {
      // Add to the signal list.
      mSignals.push_back( StringTable->insert( signal ) );

      // Create a list of subscribers for this event.
      mSubscribers.insert( new VectorPtr<SignalSubscriber*>, signal );
   }
}

// register a subscriber to a signal
void SignalHandler::registerSubscriber(const char *signal, SignalSubscriber *subscriber)
{
   // must have a subscriber
   if(!subscriber)
      return;

   // and an signal name
   if(!signal || !signal[0])
      return;

   // register a new event if this one hasn't already been registered
   if(!isSignalRegistered(signal))
      registerSignal(signal);

   // add the subscriber if it's not already registered for this signal
   VectorPtr<SignalSubscriber*>* subscribers = mSubscribers.retreive( signal );

   if(!subscribers->contains(subscriber))
      subscribers->push_back(subscriber);
}

// unregister a subscriber
void SignalHandler::unregisterSubscriber(const char *signal, SignalSubscriber *subscriber)
{
   // check if the subscriber exists and that the signal is actually registered
   if(!subscriber || !isSignalRegistered(signal))
      return;

   // find the subscriber and remove it
   VectorPtr<SignalSubscriber*>* subscribers = mSubscribers.retreive( signal );

   for( VectorPtr<SignalSubscriber *>::iterator iter = subscribers->begin(); iter != subscribers->end(); ++iter )
   {
      if( *iter == subscriber )
      {
         subscribers->erase_fast( iter );
         break;
      }
   }
}

// post a signal
void SignalHandler::postSignal(const char *signal)
{
   // signal must be registered
   if(!isSignalRegistered(signal))
      return;

   // dispatch a signal to each of the subscribers
   VectorPtr<SignalSubscriber*>* subscribers = mSubscribers.retreive( signal );

   for( VectorPtr<SignalSubscriber *>::iterator iter = subscribers->begin(); iter != subscribers->end(); ++iter )
   {
      mSignalQueue.push_back(new Signal((*iter)));
   }
}


// dispatch queued signals
void SignalHandler::dispatchSignals()
{
   if(mSignalQueue.empty())
      return;

   while(!mSignalQueue.empty())
   {
      Signal *sig = mSignalQueue.back();
      mSignalQueue.pop_back();
      sig->send();
      delete sig;
      sig = NULL;
   }
}