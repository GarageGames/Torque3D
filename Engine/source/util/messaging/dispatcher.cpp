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

#include "platform/platform.h"
#include "util/messaging/dispatcher.h"

#include "platform/threads/mutex.h"
#include "core/tSimpleHashTable.h"
#include "core/util/safeDelete.h"

namespace Dispatcher
{

//-----------------------------------------------------------------------------
// IMessageListener Methods
//-----------------------------------------------------------------------------

IMessageListener::~IMessageListener()
{
   for(S32 i = 0;i < mQueues.size();i++)
   {
      unregisterMessageListener(mQueues[i], this);
   }
}

void IMessageListener::onAddToQueue(StringTableEntry queue)
{
   // [tom, 8/20/2006] The dispatcher won't let us get added twice, so no need
   // to worry about it here.

   mQueues.push_back(queue);
}

void IMessageListener::onRemoveFromQueue(StringTableEntry queue)
{
   for(S32 i = 0;i < mQueues.size();i++)
   {
      if(mQueues[i] == queue)
      {
         mQueues.erase(i);
         return;
      }
   }
}

//-----------------------------------------------------------------------------
// Global State
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// @brief Internal class used by the dispatcher
//-----------------------------------------------------------------------------
typedef struct _DispatchData
{
   void *mMutex;
   SimpleHashTable<MessageQueue> mQueues;
   U32 mLastAnonQueueID;

   _DispatchData()
   {
      mMutex = Mutex::createMutex();
      mLastAnonQueueID = 0;
   }

   ~_DispatchData()
   {
      if(Mutex::lockMutex( mMutex ) )
      {
         mQueues.clearTables();

         Mutex::unlockMutex( mMutex );
      }

      Mutex::destroyMutex( mMutex );
      //SAFE_DELETE(mMutex);
      mMutex = NULL;
   }

   const char *makeAnonQueueName()
   {
      char buf[512];
      dSprintf(buf, sizeof(buf), "AnonQueue.%lu", mLastAnonQueueID++);
      return StringTable->insert(buf);
   }
} _DispatchData;

static _DispatchData& _dispatcherGetGDispatchData()
{
   static _DispatchData dispatchData;
   return dispatchData;
}

#define gDispatchData _dispatcherGetGDispatchData()


//-----------------------------------------------------------------------------
// Queue Registration
//-----------------------------------------------------------------------------

bool isQueueRegistered(const char *name)
{
   MutexHandle mh;
   if(mh.lock(gDispatchData.mMutex, true))
   {
      return gDispatchData.mQueues.retreive(name) != NULL;
   }

   return false;
}

void registerMessageQueue(const char *name)
{
   if(isQueueRegistered(name))
      return;

   if(Mutex::lockMutex( gDispatchData.mMutex, true ))
   {
      MessageQueue *queue = new MessageQueue;
      queue->mQueueName = StringTable->insert(name);
      gDispatchData.mQueues.insert(queue, name);

      Mutex::unlockMutex( gDispatchData.mMutex );
   }
}

extern const char * registerAnonMessageQueue()
{
   const char *name = NULL;
   if(Mutex::lockMutex( gDispatchData.mMutex, true ))
   {
      name = gDispatchData.makeAnonQueueName();
      Mutex::unlockMutex( gDispatchData.mMutex );
   }

   if(name)
      registerMessageQueue(name);

   return name;
}

void unregisterMessageQueue(const char *name)
{
   MutexHandle mh;
   if(mh.lock(gDispatchData.mMutex, true))
   {
      MessageQueue *queue = gDispatchData.mQueues.remove(name);
      if(queue == NULL)
         return;

      // Tell the listeners about it
      for(S32 i = 0;i < queue->mListeners.size();i++)
      {
         queue->mListeners[i]->onRemoveFromQueue(name);
      }

      delete queue;
   }
}

//-----------------------------------------------------------------------------
// Message Listener Registration
//-----------------------------------------------------------------------------

bool registerMessageListener(const char *queue, IMessageListener *listener)
{
   if(! isQueueRegistered(queue))
      registerMessageQueue(queue);

   MutexHandle mh;

   if(! mh.lock(gDispatchData.mMutex, true))
      return false;

   MessageQueue *q = gDispatchData.mQueues.retreive(queue);
   if(q == NULL)
   {
      Con::errorf("Dispatcher::registerMessageListener - Queue '%s' not found?! It should have been added automatically!", queue);
      return false;
   }

   for(VectorPtr<IMessageListener *>::iterator i = q->mListeners.begin();i != q->mListeners.end();i++)
   {
      if(*i == listener)
         return false;
   }

   q->mListeners.push_front(listener);
   listener->onAddToQueue(StringTable->insert(queue));
   return true;
}

void unregisterMessageListener(const char *queue, IMessageListener *listener)
{
   if(! isQueueRegistered(queue))
      return;

   MutexHandle mh;

   if(! mh.lock(gDispatchData.mMutex, true))
      return;

   MessageQueue *q = gDispatchData.mQueues.retreive(queue);
   if(q == NULL)
      return;

   for(VectorPtr<IMessageListener *>::iterator i = q->mListeners.begin();i != q->mListeners.end();i++)
   {
      if(*i == listener)
      {
         listener->onRemoveFromQueue(StringTable->insert(queue));
         q->mListeners.erase(i);
         return;
      }
   }
}

//-----------------------------------------------------------------------------
// Dispatcher
//-----------------------------------------------------------------------------

bool dispatchMessage( const char* queue, const char* msg, const char* data)
{
   AssertFatal( queue != NULL, "Dispatcher::dispatchMessage - Got a NULL queue name" );
   AssertFatal( msg != NULL, "Dispatcher::dispatchMessage - Got a NULL message" );

   MutexHandle mh;

   if(! mh.lock(gDispatchData.mMutex, true))
      return true;

   MessageQueue *q = gDispatchData.mQueues.retreive(queue);
   if(q == NULL)
   {
      Con::errorf("Dispatcher::dispatchMessage - Attempting to dispatch to unknown queue '%s'", queue);
      return true;
   }

   return q->dispatchMessage(msg, data);
}


bool dispatchMessageObject(const char *queue, Message *msg)
{
   MutexHandle mh;

   if(msg == NULL)
      return true;

   msg->addReference();

   if(! mh.lock(gDispatchData.mMutex, true))
   {
      msg->freeReference();
      return true;
   }

   MessageQueue *q = gDispatchData.mQueues.retreive(queue);
   if(q == NULL)
   {
      Con::errorf("Dispatcher::dispatchMessage - Attempting to dispatch to unknown queue '%s'", queue);
      msg->freeReference();
      return true;
   }

   // [tom, 8/19/2006] Make sure that the message is registered with the sim, since
   // when it's ref count is zero it'll be deleted with deleteObject()
   if(! msg->isProperlyAdded())
   {
      SimObjectId id = Message::getNextMessageID();
      if(id != 0xffffffff)
         msg->registerObject(id);
      else
      {
         Con::errorf("dispatchMessageObject: Message was not registered and no more object IDs are available for messages");

         msg->freeReference();
         return false;
      }
   }

   bool bResult = q->dispatchMessageObject(msg);
   msg->freeReference();

   return bResult;
}

//-----------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------

MessageQueue * getMessageQueue(const char *name)
{
   return gDispatchData.mQueues.retreive(name);
}

extern bool lockDispatcherMutex()
{
   return Mutex::lockMutex(gDispatchData.mMutex);
}

extern void unlockDispatcherMutex()
{
   Mutex::unlockMutex(gDispatchData.mMutex);
}

} // end namespace Dispatcher

//-----------------------------------------------------------------------------
// Console Methods
//-----------------------------------------------------------------------------

using namespace Dispatcher;

ConsoleFunction(isQueueRegistered, bool, 2, 2, "(string queueName)"
				"@brief Determines if a dispatcher queue exists\n\n"
				"@param queueName String containing the name of queue\n"
				"@ingroup Messaging")
{
   return isQueueRegistered(argv[1]);
}

ConsoleFunction(registerMessageQueue, void, 2, 2, "(string queueName)"
				"@brief Registeres a dispatcher queue\n\n"
				"@param queueName String containing the name of queue\n"
				"@ingroup Messaging")
{
   return registerMessageQueue(argv[1]);
}

ConsoleFunction(unregisterMessageQueue, void, 2, 2, "(string queueName)"
				"@brief Unregisters a dispatcher queue\n\n"
				"@param queueName String containing the name of queue\n"
				"@ingroup Messaging")
{
   return unregisterMessageQueue(argv[1]);
}

//-----------------------------------------------------------------------------

ConsoleFunction(registerMessageListener, bool, 3, 3, "(string queueName, string listener)"
				"@brief Registers an event message\n\n"
				"@param queueName String containing the name of queue to attach listener to\n"
				"@param listener Name of event messenger\n"
				"@ingroup Messaging")
{
   IMessageListener *listener = dynamic_cast<IMessageListener *>(Sim::findObject(argv[2]));
   if(listener == NULL)
   {
      Con::errorf("registerMessageListener - Unable to find listener object, not an IMessageListener ?!");
      return false;
   }

   return registerMessageListener(argv[1], listener);
}

ConsoleFunction(unregisterMessageListener, void, 3, 3, "(string queueName, string listener)"
				"@brief Unregisters an event message\n\n"
				"@param queueName String containing the name of queue\n"
				"@param listener Name of event messenger\n"
				"@ingroup Messaging")
{
   IMessageListener *listener = dynamic_cast<IMessageListener *>(Sim::findObject(argv[2]));
   if(listener == NULL)
   {
      Con::errorf("unregisterMessageListener - Unable to find listener object, not an IMessageListener ?!");
      return;
   }

   unregisterMessageListener(argv[1], listener);
}

//-----------------------------------------------------------------------------

ConsoleFunction(dispatchMessage, bool, 3, 4, "(string queueName, string message, string data)"
				"@brief Dispatch a message to a queue\n\n"
				"@param queueName Queue to dispatch the message to\n"
				"@param message Message to dispatch\n"
				"@param data Data for message\n"
				"@return True for success, false for failure\n"
				"@see dispatchMessageObject\n"
				"@ingroup Messaging")
{
   return dispatchMessage(argv[1], argv[2], argc > 3 ? argv[3] : "" );
}

ConsoleFunction(dispatchMessageObject, bool, 3, 3, "(string queueName, string message)"
				"@brief Dispatch a message object to a queue\n\n"
				"@param queueName Queue to dispatch the message to\n"
				"@param message Message to dispatch\n"
				"@return true for success, false for failure\n"
				"@see dispatchMessage\n"
				"@ingroup Messaging")
{
   Message *msg = dynamic_cast<Message *>(Sim::findObject(argv[2]));
   if(msg == NULL)
   {
      Con::errorf("dispatchMessageObject - Unable to find message object");
      return false;
   }

   return dispatchMessageObject(argv[1], msg);
}
