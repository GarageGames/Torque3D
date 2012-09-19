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

#include <pthread.h>
#include "platform/threads/thread.h"
#include "platform/threads/semaphore.h"
#include "platform/threads/mutex.h"
#include <stdlib.h>

class PlatformThreadData
{
public:
   ThreadRunFunction       mRunFunc;
   void*                   mRunArg;
   Thread*                 mThread;
   Semaphore               mGateway; // default count is 1
   U32                     mThreadID;
   bool                    mDead;
};

ThreadManager::MainThreadId ThreadManager::smMainThreadId;

//-----------------------------------------------------------------------------
// Function:    ThreadRunHandler
// Summary:     Calls Thread::run() with the thread's specified run argument.
//               Neccesary because Thread::run() is provided as a non-threaded
//               way to execute the thread's run function. So we have to keep
//               track of the thread's lock here.
static void *ThreadRunHandler(void * arg)
{
   PlatformThreadData *mData = reinterpret_cast<PlatformThreadData*>(arg);
   Thread *thread = mData->mThread;

   // mThreadID is filled in twice, once here and once in pthread_create().
   // We fill in mThreadID here as well as in pthread_create() because addThread()
   // can execute before pthread_create() returns and sets mThreadID.
   // The value from pthread_create() and pthread_self() are guaranteed to be equivalent (but not identical)
   mData->mThreadID = ThreadManager::getCurrentThreadId();
   
   ThreadManager::addThread(thread);
   thread->run(mData->mRunArg);
   ThreadManager::removeThread(thread);

   bool autoDelete = thread->autoDelete;
   
   mData->mThreadID = 0;
   mData->mDead = true;
   mData->mGateway.release();
   
   if( autoDelete )
      delete thread;
      
   // return value for pthread lib's benefit
   return NULL;
   // the end of this function is where the created pthread will die.
}

//-----------------------------------------------------------------------------
Thread::Thread(ThreadRunFunction func, void* arg, bool start_thread, bool autodelete)
{
   AssertFatal( !start_thread, "Thread::Thread() - auto-starting threads from ctor has been disallowed since the run() method is virtual" );

   mData = new PlatformThreadData;
   mData->mRunFunc = func;
   mData->mRunArg = arg;
   mData->mThread = this;
   mData->mThreadID = 0;
   mData->mDead = false;
   autoDelete = autodelete;
}

Thread::~Thread()
{
   stop();
   if( isAlive() )
      join();

   delete mData;
}

void Thread::start( void* arg )
{
   // cause start to block out other pthreads from using this Thread, 
   // at least until ThreadRunHandler exits.
   mData->mGateway.acquire();

   // reset the shouldStop flag, so we'll know when someone asks us to stop.
   shouldStop = false;
   
   mData->mDead = false;
   
   if( !mData->mRunArg )
      mData->mRunArg = arg;

   pthread_create((pthread_t*)(&mData->mThreadID), NULL, ThreadRunHandler, mData);
}

bool Thread::join()
{
   // not using pthread_join here because pthread_join cannot deal
   // with multiple simultaneous calls.
   
   mData->mGateway.acquire();
   AssertFatal( !isAlive(), "Thread::join() - thread not dead after join()" );
   mData->mGateway.release();
   
   return true;
}

void Thread::run(void* arg)
{
   if(mData->mRunFunc)
      mData->mRunFunc(arg);
}

bool Thread::isAlive()
{
   return ( !mData->mDead );
}

U32 Thread::getId()
{
   return mData->mThreadID;
}

void Thread::_setName( const char* )
{
   // Not supported.  Wading through endless lists of Thread-1, Thread-2, Thread-3, ... trying to find
   // that one thread you are looking for is just so much fun.
}

U32 ThreadManager::getCurrentThreadId()
{
   return (U32)pthread_self();
}

bool ThreadManager::compare(U32 threadId_1, U32 threadId_2)
{
   return pthread_equal((_opaque_pthread_t*)threadId_1, (_opaque_pthread_t*)threadId_2);
}
