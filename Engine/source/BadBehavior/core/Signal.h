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

#ifndef _BB_SIGNAL_H_
#define _BB_SIGNAL_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _TSIMPLEHASHTABLE_H
#include "core/tSimpleHashTable.h"
#endif

namespace BadBehavior
{
   //---------------------------------------------------------------------------
   // Signal subscriber interface
   //---------------------------------------------------------------------------
   class SignalSubscriber
   {
   protected:
      virtual void subscribe() = 0;
      virtual void unsubscribe() = 0;

   public:
      virtual void onSignal() = 0;
   };

   //---------------------------------------------------------------------------
   // Signal class
   //---------------------------------------------------------------------------
   class Signal
   {
   private:
      SignalSubscriber *mSubscriber;
   public:
      Signal(SignalSubscriber *subscriber) : mSubscriber(subscriber) {}
      void send() { if(mSubscriber) mSubscriber->onSignal(); }
   };

   //---------------------------------------------------------------------------
   // SignalHandler
   // Simple handler to pass signals to tree task listeners
   //---------------------------------------------------------------------------
   class SignalHandler
   {
   private:
      SimpleHashTable<VectorPtr<SignalSubscriber*>> mSubscribers;
      Vector<StringTableEntry> mSignals;
      
      VectorPtr<Signal*> mSignalQueue;
      
      void registerSignal(const char *signal);
      bool isSignalRegistered(const char *signal);
      void clearSignalQueue();
      void clearSubscribers();

   public:
      SignalHandler() {}
      ~SignalHandler();

      // reset
      void reset();

      // register a scubscriber
      void registerSubscriber(const char *signal, SignalSubscriber *subscriber);

      // unregister a subscriber
      void unregisterSubscriber(const char *signal, SignalSubscriber *subscriber);

      // post an signal
      void postSignal(const char *signal);

      // dispatch queued signals
      void dispatchSignals();
   };
} // namespace BadBehavior
#endif