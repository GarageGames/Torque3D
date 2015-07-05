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

#include "platform/threads/threadPool.h"
#include "platform/threads/thread.h"
#include "platform/platformCPUCount.h"
#include "core/strings/stringFunctions.h"
#include "core/util/tSingleton.h"


//#define DEBUG_SPEW


//=============================================================================
//    ThreadPool::Context.
//=============================================================================

ThreadPool::Context ThreadPool::Context::smRootContext( "ROOT", NULL, 1.0 );

//--------------------------------------------------------------------------

ThreadPool::Context::Context( const char* name, ThreadPool::Context* parent, F32 priorityBias )
   : mName( name ),
     mParent( parent ),
     mSibling( 0 ),
     mChildren( 0 ),
     mPriorityBias( priorityBias ),
     mAccumulatedPriorityBias( 0.0 )
{
   if( parent )
   {
      mSibling = mParent->mChildren;
      mParent->mChildren = this;
   }
}

//--------------------------------------------------------------------------

ThreadPool::Context::~Context()
{
   if( mParent )
      for( Context* context = mParent->mChildren, *prev = 0; context != 0; prev = context, context = context->mSibling )
         if( context == this )
         {
            if( !prev )
               mParent->mChildren = this->mSibling;
            else
               prev->mSibling = this->mSibling;
         }
}

//--------------------------------------------------------------------------

ThreadPool::Context* ThreadPool::Context::getChild( const char* name )
{
   for( Context* child = getChildren(); child != 0; child = child->getSibling() )
      if( dStricmp( child->getName(), name ) == 0 )
         return child;
   return 0;
}

//--------------------------------------------------------------------------

F32 ThreadPool::Context::getAccumulatedPriorityBias()
{
   if( !mAccumulatedPriorityBias )
      updateAccumulatedPriorityBiases();
   return mAccumulatedPriorityBias;
}

//--------------------------------------------------------------------------

void ThreadPool::Context::setPriorityBias( F32 value )
{
   mPriorityBias = value;
   mAccumulatedPriorityBias = 0.0;
}

//--------------------------------------------------------------------------

void ThreadPool::Context::updateAccumulatedPriorityBiases()
{
   // Update our own priority bias.

   mAccumulatedPriorityBias = mPriorityBias;
   for( Context* context = getParent(); context != 0; context = context->getParent() )
      mAccumulatedPriorityBias *= context->getPriorityBias();
   
   // Update our children.

   for( Context* child = getChildren(); child != 0; child = child->getSibling() )
      child->updateAccumulatedPriorityBiases();
}

//=============================================================================
//    ThreadPool::WorkItem.
//=============================================================================

//--------------------------------------------------------------------------

void ThreadPool::WorkItem::process()
{
   execute();
   mExecuted = true;
}

//--------------------------------------------------------------------------

bool ThreadPool::WorkItem::isCancellationRequested()
{
   return false;
}

//--------------------------------------------------------------------------

bool ThreadPool::WorkItem::cancellationPoint()
{
   if( isCancellationRequested() )
   {
      onCancelled();
      return true;
   }
   else
      return false;
}

//--------------------------------------------------------------------------

F32 ThreadPool::WorkItem::getPriority()
{
   return 1.0;
}

//=============================================================================
//    ThreadPool::WorkItemWrapper.
//=============================================================================

/// Value wrapper for work items while placed on priority queue.
/// Conforms to interface dictated by ThreadSafePriorityQueueWithUpdate.
///
/// @see ThreadSafePriorityQueueWithUpdate
/// @see ThreadPool::WorkItem
///
struct ThreadPool::WorkItemWrapper : public ThreadSafeRef< WorkItem >
{
   typedef ThreadSafeRef< WorkItem > Parent;

   WorkItemWrapper() {}
   WorkItemWrapper( WorkItem* item )
      : Parent( item ) {}

   bool           isAlive();
   F32            getPriority();
};

inline bool ThreadPool::WorkItemWrapper::isAlive()
{
   WorkItem* item = ptr();
   if( !item )
      return false;
   else if( item->isCancellationRequested() )
   {
      ( *this ) = 0;
      return false;
   }
   else
      return true;
}

inline F32 ThreadPool::WorkItemWrapper::getPriority()
{
   WorkItem* item = ptr();
   AssertFatal( item != 0, "ThreadPool::WorkItemWrapper::getPriority - called on dead item" );

   // Compute a scaled priority value based on the item's context.
   return ( item->getContext()->getAccumulatedPriorityBias() * item->getPriority() );
}

//=============================================================================
//    ThreadPool::WorkerThread.
//=============================================================================

///
///
struct ThreadPool::WorkerThread : public Thread
{
   WorkerThread( ThreadPool* pool, U32 index );

   WorkerThread*     getNext();
   virtual void      run( void* arg = 0 );

private:
   U32               mIndex;
   ThreadPool*       mPool;
   WorkerThread*     mNext;
};

ThreadPool::WorkerThread::WorkerThread( ThreadPool* pool, U32 index )
   : mPool( pool ),
     mIndex( index )
{
   // Link us to the pool's thread list.

   mNext = pool->mThreads;
   pool->mThreads = this;
}

inline ThreadPool::WorkerThread* ThreadPool::WorkerThread::getNext()
{
   return mNext;
}

void ThreadPool::WorkerThread::run( void* arg )
{
   #ifdef TORQUE_DEBUG
   {
      // Set the thread's name for debugging.
      char buffer[ 2048 ];
      dSprintf( buffer, sizeof( buffer ), "ThreadPool(%s) WorkerThread %i", mPool->mName.c_str(), mIndex );
      _setName( buffer );
   }
   #endif

#if defined(TORQUE_OS_XENON)
   // On Xbox 360 you must explicitly assign software threads to hardware threads.

   // This will distribute job threads across the secondary CPUs leaving both
   // primary CPU cores available to the "main" thread. This will help prevent
   // more L2 thrashing of the main thread/core.
   static U32 sCoreAssignment = 2;
   XSetThreadProcessor( GetCurrentThread(), sCoreAssignment );
   sCoreAssignment = sCoreAssignment < 6 ? sCoreAssignment + 1 : 2;
#endif
      
   while( 1 )
   {
      if( checkForStop() )
      {
#ifdef DEBUG_SPEW
         Platform::outputDebugString( "[ThreadPool::WorkerThread] thread '%i' exits", getId() );
#endif
         dFetchAndAdd( mPool->mNumThreads, ( U32 ) -1 );
         return;
      }

      // Mark us as potentially blocking.
      dFetchAndAdd( mPool->mNumThreadsReady, ( U32 ) -1 );

      bool waitForSignal = false;
      {
         // Try to take an item from the queue.  Do
         // this in a separate block, so we'll be
         // releasing the item after we have finished.

         WorkItemWrapper workItem;
         if( mPool->mWorkItemQueue.takeNext( workItem ) )
         {
            // Mark us as non-blocking as this loop definitely
            // won't wait on the semaphore.
            dFetchAndAdd( mPool->mNumThreadsReady, 1 );

#ifdef DEBUG_SPEW
            Platform::outputDebugString( "[ThreadPool::WorkerThread] thread '%i' takes item '0x%x'", getId(), *workItem );
#endif
            workItem->process();

            dFetchAndAdd( mPool->mNumPendingItems, ( U32 ) -1 );
         }
         else
            waitForSignal = true;
      }

      if( waitForSignal )
      {
         dFetchAndAdd( mPool->mNumThreadsAwake, ( U32 ) -1 );

#ifdef DEBUG_SPEW
         Platform::outputDebugString( "[ThreadPool::WorkerThread] thread '%i' going to sleep", getId() );
#endif
         mPool->mSemaphore.acquire();
#ifdef DEBUG_SPEW
         Platform::outputDebugString( "[ThreadPool::WorkerThread] thread '%i' waking up", getId() );
#endif

         dFetchAndAdd( mPool->mNumThreadsAwake, 1 );
         dFetchAndAdd( mPool->mNumThreadsReady, 1 );
      }
   }
}

//=============================================================================
//    ThreadPool.
//=============================================================================

bool                          ThreadPool::smForceAllMainThread;
U32                           ThreadPool::smMainThreadTimeMS;
ThreadPool::QueueType         ThreadPool::smMainThreadQueue;

//--------------------------------------------------------------------------

ThreadPool::ThreadPool( const char* name, U32 numThreads )
   : mName( name ),
     mNumThreads( numThreads ),
     mNumThreadsAwake( 0 ),
     mNumPendingItems( 0 ),
     mThreads( 0 ),
     mSemaphore( 0 )
{
   // Number of worker threads to create.

   if( !mNumThreads )
   {
      // Use platformCPUInfo directly as in the case of the global pool,
      // Platform::SystemInfo will not yet have been initialized.
      
      U32 numLogical = 0;
      U32 numPhysical = 0;
      U32 numCores = 0;

      CPUInfo::CPUCount( numLogical, numCores, numPhysical );
      
      const U32 baseCount = getMax( numLogical, numCores );
      mNumThreads = (baseCount > 0) ? baseCount : 2;
   }
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[ThreadPool] spawning %i threads", mNumThreads );
   #endif

   // Create the threads.

   mNumThreadsAwake = mNumThreads;
   mNumThreadsReady = mNumThreads;
   for( U32 i = 0; i < mNumThreads; i ++ )
   {
      WorkerThread* thread = new WorkerThread( this, i );
      thread->start();
   }
}

//--------------------------------------------------------------------------

ThreadPool::~ThreadPool()
{
	shutdown();
}

//--------------------------------------------------------------------------

void ThreadPool::shutdown()
{
   const U32 numThreads = mNumThreads;
   
	// Tell our worker threads to stop.

	for( WorkerThread* thread = mThreads; thread != 0; thread = thread->getNext() )
		thread->stop();

	// Release the semaphore as many times as there are threads.
	// Doing this separately guarantees we're not waking a thread
	// that hasn't been set its stop flag yet.

	for( U32 n = 0; n < numThreads; ++ n )
		mSemaphore.release();

	// Delete each worker thread.  Wait until death as we're prone to
	// running into issues with decomposing work item lists otherwise.

	for( WorkerThread* thread = mThreads; thread != 0; )
	{
		WorkerThread* next = thread->getNext();
		thread->join();
		delete thread;
		thread = next;
	}

	mThreads = NULL;
	mNumThreads = 0;
}

//--------------------------------------------------------------------------

void ThreadPool::queueWorkItem( WorkItem* item )
{
   bool executeRightAway = ( getForceAllMainThread() );
#ifdef DEBUG_SPEW
   Platform::outputDebugString( "[ThreadPool] %s work item '0x%x'",
                                ( executeRightAway ? "executing" : "queuing" ),
                                item );
#endif

   if( executeRightAway )
      item->process();
   else
   {
      // Put the item in the queue.
      dFetchAndAdd( mNumPendingItems, 1 );
      mWorkItemQueue.insert( item->getPriority(), item );

      mSemaphore.release();
   }
}

//--------------------------------------------------------------------------

void ThreadPool::flushWorkItems( S32 timeOut )
{
   AssertFatal( mNumThreads, "ThreadPool::flushWorkItems() - no worker threads in pool" );
   
   U32 endTime = 0;
   if( timeOut != -1 )
      endTime = Platform::getRealMilliseconds() + timeOut;

   // Spinlock until the queue is empty.

   while( !mWorkItemQueue.isEmpty() )
   {
      Platform::sleep( 25 );

      // Stop if we have exceeded our processing time budget.

      if( timeOut != -1
          && Platform::getRealMilliseconds() >= endTime )
          break;
   }
}

void ThreadPool::waitForAllItems( S32 timeOut )
{
   U32 endTime = 0;
   if( timeOut != -1 )
      endTime = Platform::getRealMilliseconds() + timeOut;

   // Spinlock until there are no items that have not been processed.

   while( dAtomicRead( mNumPendingItems ) )
   {
      Platform::sleep( 25 );

      // Stop if we have exceeded our processing time budget.

      if( timeOut != -1
          && Platform::getRealMilliseconds() >= endTime )
          break;
   }
}

//--------------------------------------------------------------------------

void ThreadPool::queueWorkItemOnMainThread( WorkItem* item )
{
   smMainThreadQueue.insert( item->getPriority(), item );
}

//--------------------------------------------------------------------------

void ThreadPool::processMainThreadWorkItems()
{
   AssertFatal( ThreadManager::isMainThread(),
      "ThreadPool::processMainThreadWorkItems - this function must only be called on the main thread" );

   U32 timeLimit = ( Platform::getRealMilliseconds() + getMainThreadThresholdTimeMS() );

   do
   {
      WorkItemWrapper item;
      if( !smMainThreadQueue.takeNext( item ) )
         break;
      else
         item->process();
   }
   while( Platform::getRealMilliseconds() < timeLimit );
}
