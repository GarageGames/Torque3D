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

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#ifndef _THREADSAFEREFCOUNT_H_
   #include "platform/threads/threadSafeRefCount.h"
#endif
#ifndef _THREADSAFEPRIORITYQUEUE_H_
   #include "platform/threads/threadSafePriorityQueue.h"
#endif
#ifndef _PLATFORM_THREAD_SEMAPHORE_H_
   #include "platform/threads/semaphore.h"
#endif
#ifndef _TSINGLETON_H_
   #include "core/util/tSingleton.h"
#endif


/// @file
/// Interface for an asynchronous work manager.


/// Asynchronous work manager.
///
/// Thread pooling allows to submit work items for background execution.
/// Each work item will be placed on a queue and, based on a total priority
/// ordering, executed when it has the highest priority and a worker thread
/// becomes available.
///
/// @note The global pool maintains the invariant that only the main thread
///   may submit items in order to be able to flush the item queue reliably
///   from the main thread itself.  If other threads were issuing items to
///   the queue, the queue may never empty out and the main thread will
///   deadlock.
///
///   Flushing is the simplest method to guarantee that no asynchronous
///   operation is pending in a specific case (deletion of the target object
///   being the most common case).  However, when possible, avoid this
///   situation and design your work items to operate independently,
///   e.g. by having only a single point of access to data that may have
///   disappeared in the meantime and putting a check around that single
///   access so that the item will silently die when its target object has
///   disappeared.
///
///   The cleanest safe solution to this is to create a separate concurrently
///   reference-counted structure that holds all interfacing state and
///   functionality shared between a work item and its issueing code.  This way
///   the host object can safely disappear with the interfacing structure
///   automatically being released once the last concurrent work item has been
///   processed or discarded.
///
class ThreadPool
{
   public:
   
      /// A ThreadPool context defines a logical context in which WorkItems are
      /// being executed.  Their primary use is for biasing priorities of
      /// WorkItems.
      ///
      /// Contexts are arranged in a tree hierarchy.  Each parent node's priority
      /// bias scales all the priority biases underneath it.
      ///
      /// Note that instances of this class are meant to be instantiated
      /// globally only.
      ///
      class Context
      {
         protected:
         
            /// Superordinate context; scales this context's priority bias.
            Context* mParent;
            
            /// First child.
            Context* mChildren;
            
            /// Next sibling in child chain.
            Context* mSibling;
            
            /// Name of this context.  Should be unique in parent namespace.
            const char* mName;
            
            /// Priority scale factor of this context.
            F32 mPriorityBias;
            
            /// Accumulated scale factor.
            F32 mAccumulatedPriorityBias;

            /// The root context; does not modify priorities.  All contexts should be direct or indirect children of this one.
            static Context smRootContext;

            /// Recursively update cached accumulated priority biases.
            void updateAccumulatedPriorityBiases();

         public:
         
            Context( const char* name, Context* parent, F32 priorityBias );
            ~Context();

            /// Return the name of the worker threading context.
            const char* getName() const
            {
               return mName;
            }

            /// Return the context's own work item priority bias.
            F32 getPriorityBias() const
            {
               return mPriorityBias;
            }

            /// Return the superordinate node to the current context.
            Context* getParent() const
            {
               return mParent;
            }

            /// Return the next sibling to the current context.
            Context* getSibling() const
            {
               return mSibling;
            }

            /// Return the first child context.
            Context* getChildren() const
            {
               return mChildren;
            }

            /// Return the root context.
            static Context* ROOT_CONTEXT()
            {
               return &smRootContext;
            }
            
            ///
            F32 getAccumulatedPriorityBias();
            
            ///
            Context* getChild( const char* name );

            ///
            void setPriorityBias( F32 value );
      };

      /// An action to execute on a worker thread from the pool.
      ///
      /// Work items are concurrently reference-counted and will be
      /// automatically released once the last reference disappears.
      ///
      class WorkItem : public ThreadSafeRefCount< WorkItem >
      {
         public:

            typedef ThreadSafeRefCount< WorkItem > Parent;

         protected:
         
            /// The work context of this item.
            Context* mContext;

            /// Mark a point in a work item's execution where the item can
            /// be safely cancelled.
            ///
            /// This method should be called by subclasses' execute() methods
            /// whenever an item can be safely cancelled.  When it returns true,
            /// the work item should exit from its execute() method.
            bool cancellationPoint();
            
            /// Called when the item has been cancelled.
            virtual void onCancelled() {}

            /// Execute the actions associated with this work item.
            /// This is the primary function to implement by subclasses.
            virtual void execute() = 0;

            /// This flag is set after the execute() method has completed.
            bool mExecuted;

         public:
         
            /// Construct a new work item.
            ///
            /// @param context The work context in which the item should be placed.
            ///    If NULL, the root context will be used.
            WorkItem( Context* context = 0 )
               : mContext( context ? context : Context::ROOT_CONTEXT() ),
                 mExecuted( false )
            {
            }
            
            virtual ~WorkItem() {}

            /// Return the work context associated with the work item.
            inline Context* getContext() const
            {
               return mContext;
            }

            /// Process the work item.
            void process();
            
            /// Return true if the work item should be cancelled.
            ///
            /// This method can be overridden by subclasses.  It's value will be
            /// checked each time cancellationPoint() is called.  When it returns
            /// true, the item's process() method will exit automatically.
            ///
            /// @return true, if item should be cancelled; default is false.
            /// @see ThreadPool::WorkItem::cancellationPoint
            virtual bool isCancellationRequested();
            
            /// Return the item's base priority value.
            /// @return item priority; defaults to 1.0.
            virtual F32 getPriority();

            /// Has this work item been executed already?
            bool hasExecuted() const
            {
               return mExecuted;
            }
      };

      typedef ThreadSafeRef< WorkItem > WorkItemPtr;
      struct GlobalThreadPool;
      
   protected:
   
      struct WorkItemWrapper;
      struct WorkerThread;

      friend struct WorkerThread; // mSemaphore, mNumThreadsAwake, mThreads

      typedef ThreadSafePriorityQueueWithUpdate< WorkItemWrapper, F32 > QueueType;

      /// Name of this pool.  Mainly for debugging.  Used to name worker threads.
      String mName;
      
      /// Number of worker threads spawned by the pool.
      U32 mNumThreads;
      
      /// Number of worker threads in non-sleeping state.
      U32 mNumThreadsAwake;
      
      /// Number of worker threads guaranteed to be non-blocking.
      U32 mNumThreadsReady;

      /// Number of work items that have not yet completed execution.
      U32 mNumPendingItems;
      
      /// Semaphore used to wake up threads, if necessary.
      Semaphore mSemaphore;
      
      /// Threaded priority queue for concurrent access by worker threads.
      QueueType mWorkItemQueue;
      
      /// List of worker threads.
      WorkerThread* mThreads;

      /// Force all work items to execute on main thread;
      /// turns this into a single-threaded system.
      /// Primarily useful to find whether malfunctions are caused
      /// by parallel execution or not.
      static bool smForceAllMainThread;
      
      ///
      static U32 smMainThreadTimeMS;
            
      /// Work queue for main thread; can be used to ping back work items to
      /// main thread that need processing that can only happen on main thread.
      static QueueType smMainThreadQueue;

   public:

      /// Create a new thread pool with the given number of worker threads.
      ///
      /// If numThreads is zero (the default), the number of threads created
      /// will be based on the number of CPU cores available.
      ///
      /// @param numThreads Number of threads to create or zero for default.
      ThreadPool( const char* name, U32 numThreads = 0 );
      
      ~ThreadPool();

      /// Manually shutdown threads outside of static destructors.
      void shutdown();

      ///
      void queueWorkItem( WorkItem* item );
      
      ///
      /// <em>For the global pool, it is very important to only ever call
      /// this function on the main thread and to let work items only ever
      /// come from the main thread.  Otherwise this function has the potential
      /// of dead-locking as new work items may constantly be fed to the queue
      /// without it ever getting empty.</em>
      ///
      /// @param timeOut Soft limit on the number of milliseconds to wait for
      ///   the queue to flush out.  -1 = infinite.
      void flushWorkItems( S32 timeOut = -1 );

      /// If you're using a non-global thread pool to parallelise some work, you
      /// may want to block until all the parallel work is complete. As with
      /// flushWorkItems, this method may block indefinitely if new items keep
      /// getting added to the pool before old ones finish.
      ///
      /// <em>This method will not wait for items queued on the main thread using
      /// queueWorkItemOnMainThread!</em>
      ///
      /// @param timeOut Soft limit on the number of milliseconds to wait for
      ///   all items to complete.  -1 = infinite.
      void waitForAllItems( S32 timeOut = -1 );

      /// Add a work item to the main thread's work queue.
      ///
      /// The main thread's work queue will be processed each frame using
      /// a set timeout to limit the work being done.  Nonetheless, work
      /// items will not be suspended in-midst of processing, so make sure
      /// that whatever work you issue to the main thread is light work
      /// or you may see short hangs in gameplay.
      ///
      /// To reiterate this: any code executed through this interface directly
      /// adds to frame processing time on the main thread.
      ///
      /// This method *may* (and is meant to) be called from threads
      /// other than the main thread.
      static void queueWorkItemOnMainThread( WorkItem* item );
      
      /// Process work items waiting on the main thread's work queue.
      ///
      /// There is a soft limit imposed on the time this method is allowed
      /// to run so as to balance frame-to-frame load.  However, work
      /// items, once their processing is initiated, will not be suspended
      /// and will run for as long as they take to complete, so make sure
      /// individual items perform as little work as necessary.
      ///
      /// @see ThreadPool::getMainThreadThesholdTimeMS
      static void processMainThreadWorkItems();

      /// Return the interval in which item priorities are updated on the queue.
      /// @return update interval in milliseconds.
      U32 getQueueUpdateInterval() const
      {
         return mWorkItemQueue.getUpdateInterval();
      }

      /// Return the priority increment applied to work items on each passing of the update interval.
      F32 getQueueTimeBasedPriorityBoost() const
      {
         return mWorkItemQueue.getTimeBasedPriorityBoost();
      }

      /// Set the update interval of the work item queue to the given value.
      /// @param milliSeconds Time between updates in milliseconds.
      void setQueueUpdateInterval( U32 milliSeconds )
      {
         mWorkItemQueue.setUpdateInterval( milliSeconds );
      }

      /// Set the priority increment applied to work items on each update interval.
      /// @param value Priority increment.  Set to zero to deactivate.
      void setQueueTimeBasedPriorityBoost( F32 value )
      {
         mWorkItemQueue.setTimeBasedPriorityBoost( value );
      }

      ///
      static U32& getMainThreadThresholdTimeMS()
      {
         return smMainThreadTimeMS;
      }

      ///
      static bool& getForceAllMainThread()
      {
         return smForceAllMainThread;
      }

      /// Return the global thread pool singleton.
      static ThreadPool& GLOBAL();
};

typedef ThreadPool::Context ThreadContext;
typedef ThreadPool::WorkItem ThreadWorkItem;


struct ThreadPool::GlobalThreadPool : public ThreadPool, public ManagedSingleton< GlobalThreadPool >
{
   typedef ThreadPool Parent;
   
   GlobalThreadPool()
      : Parent( "GLOBAL" ) {}
      
   // For ManagedSingleton.
   static const char* getSingletonName() { return "GlobalThreadPool"; }
};

inline ThreadPool& ThreadPool::GLOBAL()
{
   return *( GlobalThreadPool::instance() );
}

#endif // !_THREADPOOL_H_
