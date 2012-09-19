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

#ifndef _PLATFORM_THREADS_THREAD_H_
#define _PLATFORM_THREADS_THREAD_H_

#ifndef _TORQUE_TYPES_H_
   #include "platform/types.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif
#ifndef _PLATFORM_THREADS_MUTEX_H_
   #include "platform/threads/mutex.h"
#endif
#ifndef _TSINGLETON_H_
   #include "core/util/tSingleton.h"
#endif


// Forward ref used by platform code
class PlatformThreadData;


// Typedefs
typedef void (*ThreadRunFunction)(void *data);

class Thread
{
public:
   typedef void Parent;

protected:
   PlatformThreadData*  mData;

   /// Used to signal threads need to stop. 
   /// Threads set this flag to false in start()
   U32 shouldStop;

   /// Set the name of this thread for identification in debuggers.
   /// Maybe a NOP on platforms that do not support this.  Always a NOP
   /// in non-debug builds.
   void _setName( const char* name );

public:
   /// If set, the thread will delete itself once it has finished running.
   bool autoDelete;

   /// Create a thread.
   /// @param func The starting function for the thread.
   /// @param arg Data to be passed to func, when the thread starts.
   /// @param start_thread Supported for compatibility.  Must be false.  Starting threads from
   ///   within the constructor is not allowed anymore as the run() method is virtual.
   Thread(ThreadRunFunction func = 0, void *arg = 0, bool start_thread = false, bool autodelete = false);
   
   /// Destroy a thread.
   /// The thread MUST be allowed to exit before it is destroyed.
   virtual ~Thread();

   /// Start a thread. 
   /// Sets shouldStop to false and calls run() in a new thread of execution.
   void start( void* arg = 0 );

   /// Ask a thread to stop running.
   void stop()
   {
      shouldStop = true;
   }

   /// Block until the thread stops running.
   /// @note Don't use this in combination with auto-deletion as otherwise the thread will kill
   ///   itself while still executing the join() method on the waiting thread.
   bool join();

   /// Threads may call checkForStop() periodically to check if they've been 
   /// asked to stop. As soon as checkForStop() returns true, the thread should
   /// clean up and return.
   bool checkForStop()
   {
      return shouldStop;
   }

   /// Run the Thread's entry point function.
   /// Override this method in a subclass of Thread to create threaded code in
   /// an object oriented way, and without passing a function ptr to Thread().
   /// Also, you can call this method directly to execute the thread's
   /// code in a non-threaded way.
   virtual void run(void *arg = 0);

   /// Returns true if the thread is running.
   bool isAlive();

   /// Returns the platform specific thread id for this thread.
   U32 getId();
};


///
class ThreadManager 
{
   Vector<Thread*> threadPool;
   Mutex poolLock;

   struct MainThreadId
   {
      U32 mId;
      MainThreadId()
      {
         mId = ThreadManager::getCurrentThreadId();
      }
      U32 get()
      {
         // Okay, this is a bit soso.  The main thread ID may get queried during
         // global ctor phase before MainThreadId's ctor ran.  Since global
         // ctors will/should all run on the main thread, we can sort of safely
         // assume here that we can just query the current thread's ID.

         if( !mId )
            mId = ThreadManager::getCurrentThreadId();
         return mId;
      }
   };

   static MainThreadId smMainThreadId;
   
public:
   ThreadManager()
   {
      VECTOR_SET_ASSOCIATION( threadPool );
   }

   /// Return true if the caller is running on the main thread.
   static bool isMainThread();

   /// Returns true if threadId is the same as the calling thread's id.
   static bool isCurrentThread(U32 threadId);

   /// Returns true if the 2 thread ids represent the same thread. Some thread
   /// APIs return an opaque object as a thread id, so the == operator cannot
   /// reliably compare thread ids.
   // this comparator is needed by pthreads and ThreadManager.
   static bool compare(U32 threadId_1, U32 threadId_2);
      
   /// Returns the platform specific thread id of the calling thread. Some 
   /// platforms do not guarantee that this ID stays the same over the life of 
   /// the thread, so use ThreadManager::compare() to compare thread ids.
   static U32 getCurrentThreadId();

   /// Returns the platform specific thread id ot the main thread.
   static U32 getMainThreadId() { return smMainThreadId.get(); }
   
   /// Each thread should add itself to the thread pool the first time it runs.
   static void addThread(Thread* thread)
   {
      ThreadManager &manager = *ManagedSingleton< ThreadManager >::instance();
      manager.poolLock.lock();
      Thread *alreadyAdded = getThreadById(thread->getId());
      if(!alreadyAdded)
         manager.threadPool.push_back(thread);
      manager.poolLock.unlock();
   }

   static void removeThread(Thread* thread)
   {
      ThreadManager &manager = *ManagedSingleton< ThreadManager >::instance();
      manager.poolLock.lock();
      
      U32 threadID = thread->getId();
      for(U32 i = 0;i < manager.threadPool.size();++i)
      {
         if( compare( manager.threadPool[i]->getId(), threadID ) )
         {
            manager.threadPool.erase(i);
            break;
         }
      }
      
      manager.poolLock.unlock();
   }
   
   /// Searches the pool of known threads for a thread whose id is equivalent to
   /// the given threadid. Compares thread ids with ThreadManager::compare().
   static Thread* getThreadById(U32 threadid)
   {
      AssertFatal(threadid != 0, "ThreadManager::getThreadById() Searching for a bad thread id.");
      Thread* ret = NULL;
      
      ThreadManager &manager = *ManagedSingleton< ThreadManager >::instance();
      manager.poolLock.lock();
      Vector<Thread*> &pool = manager.threadPool;
      for( S32 i = pool.size() - 1; i >= 0; i--)
      {
         Thread* p = pool[i];
         if(compare(p->getId(), threadid))
         {
            ret = p;
            break;
         }
      }
      manager.poolLock.unlock();
      return ret;
   }

   static Thread* getCurrentThread()
   {
      return getThreadById(ThreadManager::getCurrentThreadId());
   }
   
   static const char* getSingletonName()
   {
      return "ThreadManager";
   }
};

inline bool ThreadManager::isMainThread()
{
   return compare( ThreadManager::getCurrentThreadId(), smMainThreadId.get() );
}

inline bool ThreadManager::isCurrentThread(U32 threadId)
{
   U32 current = getCurrentThreadId();
   return compare(current, threadId);
}

#endif // _PLATFORM_THREADS_THREAD_H_
