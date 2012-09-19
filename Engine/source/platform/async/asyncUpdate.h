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

#ifndef _ASYNCUPDATE_H_
#define _ASYNCUPDATE_H_

#ifndef _PLATFORM_THREADS_THREAD_H_
#  include "platform/threads/thread.h"
#endif
#ifndef _THREADSAFEREFCOUNT_H_
#  include "platform/threads/threadSafeRefCount.h"
#endif
#ifndef _THREADSAFEDEQUE_H_
#  include "platform/threads/threadSafeDeque.h"
#endif


class IPolled;

//--------------------------------------------------------------------------
//    Async update list.
//--------------------------------------------------------------------------

/// This structure keeps track of the objects that need
/// updating.
class AsyncUpdateList : public ThreadSafeRefCount< AsyncUpdateList >
{
   protected:

      typedef ThreadSafeDeque< IPolled* > UpdateList;

      /// List of structures currently in the update loop.
      UpdateList mUpdateList;

   public:

      virtual ~AsyncUpdateList() {}

      /// Update the structures currently on the processing list.
      ///
      /// @param timeOut Soft limit in milliseconds on the time
      ///   spent on flushing the list.  Default of -1 means no
      ///   limit and function will only return, if update list
      ///   has been fully flushed.
      virtual void process( S32 timeOut = -1 );

      /// Add the structure to the update list.  It will stay
      /// on this list, until its update() method returns false.
      ///
      /// @note This can be called on different threads.
      virtual void add( IPolled* ptr )
      {
         mUpdateList.pushBack( ptr );
      }
};

//--------------------------------------------------------------------------
//    Async update thread.
//--------------------------------------------------------------------------

/// Abstract baseclass for async update threads.
class AsyncUpdateThread : public Thread, public ThreadSafeRefCount< AsyncUpdateThread >
{
   public:

      typedef Thread Parent;

   protected:

      /// Name of this thread.
      String mName;

      /// Platform-dependent event data.
      void* mUpdateEvent;

      /// The update list processed on this thread.
      ThreadSafeRef< AsyncUpdateList > mUpdateList;

      /// Wait for an update event being triggered and
      /// immediately reset the event.
      ///
      /// @note Note that this must be an atomic operation to avoid
      ///   a race condition.  Immediately resetting the event shields
      ///   us from event releases happening during us updating getting
      ///   ignored.
      virtual void _waitForEventAndReset();

   public:

      /// Create the update thread.
      /// The thread won't immediately start (we have virtual functions
      /// so construction needs to finish first) and will not auto-delete
      /// itself.
      AsyncUpdateThread( String name, AsyncUpdateList* updateList );

      virtual ~AsyncUpdateThread();

      virtual void run( void* );

      /// Trigger the update event to notify the thread about
      /// pending updates.
      virtual void triggerUpdate();

      ///
      const String& getName() const { return mName; }

      ///
      void* getUpdateEvent() const { return mUpdateEvent; }
};

/// Extension to update thread that also does automatic
/// periodic updates.
class AsyncPeriodicUpdateThread : public AsyncUpdateThread
{
      typedef AsyncUpdateThread Parent;

   protected:

      /// Platform-dependent timer event.
      void* mUpdateTimer;
      
      /// Time between periodic updates in milliseconds.
      U32 mIntervalMS;

      virtual void _waitForEventAndReset();

   public:

      enum
      {
         /// Default interval between periodic updates in milliseconds.
         DEFAULT_UPDATE_INTERVAL = 4000
      };

      ///
      AsyncPeriodicUpdateThread(  String name,
                                  AsyncUpdateList* updateList,
                                  U32 intervalMS = DEFAULT_UPDATE_INTERVAL );

      virtual ~AsyncPeriodicUpdateThread();
};

#endif // _TORQUE_CORE_ASYNC_ASYNCUPDATE_H_
