//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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

#ifndef _SIMEVENTS_H_
#define _SIMEVENTS_H_

#ifndef _SIM_H_
#include "console/sim.h"
#endif

#ifndef _UTIL_DELEGATE_H_
#include "core/util/delegate.h"
#endif

// Forward Refs
class SimObject;
class Semaphore;

/// Represents a queued event in the sim.
///
/// Sim provides an event queue for your convenience, which
/// can be used to schedule events. A few things which use
/// this event queue:
///
///     - The scene lighting system. In order to keep the game
///       responsive while scene lighting occurs, the lighting
///       process is divided into little chunks. In implementation
///       terms, there is a subclass of SimEvent called
///       SceneLightingProcessEvent. The process method of this
///       subclass calls into the lighting code, telling it to
///       perform the next chunk of lighting calculations.
///     - The schedule() console function uses a subclass of
///       SimEvent called SimConsoleEvent to keep track of
///       scheduled events.
class SimEvent
{
public:
   SimEvent *nextEvent;     ///< Linked list details - pointer to next item in the list.
   SimTime startTime;       ///< When the event was posted.
   SimTime time;            ///< When the event is scheduled to occur.
   U32 sequenceCount;       ///< Unique ID. These are assigned sequentially based on order
   ///  of addition to the list.
   SimObject *destObject;   ///< Object on which this event will be applied.

   SimEvent() { destObject = NULL; }
   virtual ~SimEvent() {}   ///< Destructor
   ///
   /// A dummy virtual destructor is required
   /// so that subclasses can be deleted properly

   /// Function called when event occurs.
   ///
   /// This is where the meat of your event's implementation goes.
   ///
   /// See any of the subclasses for ideas of what goes in here.
   ///
   /// The event is deleted immediately after processing. If the
   /// object referenced in destObject is deleted, then the event
   /// is not called. The even will be executed unconditionally if
   /// the object referenced is NULL.
   ///
   /// @param   object  Object stored in destObject.
   virtual void process(SimObject *object)=0;
};

/// Implementation of schedule() function.
///
/// This allows you to set a console function to be
/// called at some point in the future.
class SimConsoleEvent : public SimEvent
{
protected:
   S32 mArgc;
   char **mArgv;
   bool mOnObject;
public:

   /// Constructor
   ///
   /// Pass the arguments of a function call, optionally on an object.
   ///
   /// The object for the call to be executed on is specified by setting
   /// onObject and storing a reference to the object in destObject. If
   /// onObject is false, you don't need to store anything into destObject.
   ///
   /// The parameters here are passed unmodified to Con::execute() at the
   /// time of the event.
   ///
   /// @see Con::execute(S32 argc, const char *argv[])
   /// @see Con::execute(SimObject *object, S32 argc, const char *argv[])
   SimConsoleEvent(S32 argc, const char **argv, bool onObject);

   ~SimConsoleEvent();
   virtual void process(SimObject *object);
};

/// Used by Con::threadSafeExecute()
struct SimConsoleThreadExecCallback
{
   Semaphore   *sem;
   const char  *retVal;

   SimConsoleThreadExecCallback();
   ~SimConsoleThreadExecCallback();

   void handleCallback(const char *ret);
   const char *waitForResult();
};

class SimConsoleThreadExecEvent : public SimConsoleEvent
{
   SimConsoleThreadExecCallback *cb;

public:
   SimConsoleThreadExecEvent(S32 argc, const char **argv, bool onObject, SimConsoleThreadExecCallback *callback);

   virtual void process(SimObject *object);
};

/// General purpose SimEvent which calls a Delegate<void()> callback.
class SimDelegateEvent : public SimEvent
{
public:

   U32 *mEventId;
   Delegate<void()> mCallback;

   void process( SimObject* )
   {
      // Clear the event id and call the delegate.
      *mEventId = InvalidEventId;
      mCallback();
   }
};

#endif // _SIMEVENTS_H_
