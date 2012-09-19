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

#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#ifndef _MESSAGE_H_
#include "util/messaging/message.h"
#endif

#ifndef _CONSOLE_H_
#include "console/console.h"
#endif

/// @addtogroup msgsys Message System
// @{

//-----------------------------------------------------------------------------
/// @brief Namespace for the message dispatcher functions
//-----------------------------------------------------------------------------
namespace Dispatcher
{

// [tom, 2/19/2007] This semi colon prevents VS from auto indenting the comments
// below, which is really annoying when you're trying to write docs.
;

/// @addtogroup msgsys Message System
// @{

//-----------------------------------------------------------------------------
// Interface for objects that receive messages
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// @brief Listener interface for objects that receive messages
///
/// @see ScriptMsgListener
//-----------------------------------------------------------------------------
class IMessageListener
{
protected:
   /// List of queues this listener is registered with.
   Vector<StringTableEntry> mQueues;

public:
   virtual ~IMessageListener();

   //-----------------------------------------------------------------------------
   /// @brief Callback for when messages are received
   /// 
   /// @param queue The name of the queue the message was dispatched to
   /// @param msg The type of message
   /// @param data The data for the message
   /// @return false to prevent other listeners receiving this message, true otherwise
   /// @see onMessageObjectReceived()
   //-----------------------------------------------------------------------------
   virtual bool onMessageReceived(StringTableEntry queue, const char *msg, const char *data) = 0;

   //-----------------------------------------------------------------------------
   /// @brief Callback for when message objects are received
   /// 
   /// @param queue The name of the queue the message was dispatched to
   /// @param msg The message object
   /// @return false to prevent other listeners receiving this message, true otherwise
   /// @see onMessageReceived()
   //-----------------------------------------------------------------------------
   virtual bool onMessageObjectReceived(StringTableEntry queue, Message *msg ) = 0;


   //-----------------------------------------------------------------------------
   /// @brief Callback for when the listener is added to a queue
   ///
   /// The default implementation of onAddToQueue() and onRemoveFromQueue()
   /// provide tracking of the queues this listener is added to through the
   /// #mQueues member. Overrides of onAddToQueue() or onRemoveFromQueue()
   /// should ensure they call the parent implementation in any overrides.
   /// 
   /// @param queue The name of the queue that the listener added to
   /// @see onRemoveFromQueue()
   //-----------------------------------------------------------------------------
   virtual void onAddToQueue(StringTableEntry queue);

   //-----------------------------------------------------------------------------
   /// @brief Callback for when the listener is removed from a queue
   /// 
   /// The default implementation of onAddToQueue() and onRemoveFromQueue()
   /// provide tracking of the queues this listener is added to through the
   /// #mQueues member. Overrides of onAddToQueue() or onRemoveFromQueue()
   /// should ensure they call the parent implementation in any overrides.
   /// 
   /// @param queue The name of the queue the listener was removed from
   /// @see onAddToQueue()
   //-----------------------------------------------------------------------------
   virtual void onRemoveFromQueue(StringTableEntry queue);
};

//-----------------------------------------------------------------------------
/// @brief Internal class for tracking message queues
//-----------------------------------------------------------------------------
struct MessageQueue
{
   StringTableEntry mQueueName;
   VectorPtr<IMessageListener *> mListeners;

   MessageQueue() : mQueueName("")
   {
   }

   bool isEmpty()    { return mListeners.size() == 0; }

   bool dispatchMessage(const char* event, const char* data)
   {
      for(VectorPtr<IMessageListener *>::iterator i = mListeners.begin();i != mListeners.end();i++)
      {
         if( !(*i)->onMessageReceived(mQueueName, event, data) )
            return false;
      }
      return true;
   }

   bool dispatchMessageObject(Message *msg)
   {
      for(VectorPtr<IMessageListener *>::iterator i = mListeners.begin();i != mListeners.end();i++)
      {
         if( !(*i)->onMessageObjectReceived(mQueueName, msg) )
            return false;
      }
      return true;
   }
};

//-----------------------------------------------------------------------------
// Message Dispatcher Functions
//-----------------------------------------------------------------------------

/// @name Message Queue Management
// @{

//-----------------------------------------------------------------------------
/// @brief Check if a message queue is registered
/// 
/// @param name The name of the message queue
/// @return true if the queue is registered, false otherwise
/// @see registerMessageQueue(), unregisterMessageQueue()
//-----------------------------------------------------------------------------
extern bool isQueueRegistered(const char *name);

//-----------------------------------------------------------------------------
/// @brief Register a message queue
/// 
/// @param name The name of the message queue to register
/// @see isQueueRegistered(), unregisterMessageQueue()
//-----------------------------------------------------------------------------
extern void registerMessageQueue(const char *name);

//-----------------------------------------------------------------------------
/// @brief Register an anonymous message queue
/// 
/// @return name of anonymous message queue for passing to other functions
/// @see isQueueRegistered(), unregisterMessageQueue()
//-----------------------------------------------------------------------------
extern const char *registerAnonMessageQueue();

//-----------------------------------------------------------------------------
/// @brief Unregister a message queue
/// 
/// @param name The name of the message queue
/// @see registerMessageQueue(), isQueueRegistered()
//-----------------------------------------------------------------------------
extern void unregisterMessageQueue(const char *name);

//-----------------------------------------------------------------------------
/// @brief Register a listener with a queue to receive messages
/// 
/// @param queue The name of the queue to register the listener with
/// @param listener The listener interface that receives messages
/// @return true for success, false otherwise
/// @see unregisterMessageListener()
//-----------------------------------------------------------------------------
extern bool registerMessageListener(const char *queue, IMessageListener *listener);

//-----------------------------------------------------------------------------
/// @brief Unregister a listener with a queue
/// 
/// @param queue The name of the queue to unregister the listener
/// @param listener The listener interface that was passed to registerMessageListener()
/// @see registerMessageListener()
//-----------------------------------------------------------------------------
extern void unregisterMessageListener(const char *queue, IMessageListener *listener);

// @}

/// @name Message Dispatcher
// @{

//-----------------------------------------------------------------------------
/// @brief Dispatch a message to a queue
/// 
/// @param queue Queue to dispatch the message to
/// @param msg Message to dispatch
/// @param data Data for message
/// @return true for success, false for failure
/// @see dispatchMessageObject()
//-----------------------------------------------------------------------------
extern bool dispatchMessage(const char *queue, const char *msg, const char *data);

//-----------------------------------------------------------------------------
/// @brief Dispatch a message object to a queue
/// 
/// @param queue Queue to dispatch the message to
/// @param msg Message to dispatch
/// @return true for success, false for failure
/// @see dispatchMessage()
//-----------------------------------------------------------------------------
extern bool dispatchMessageObject(const char *queue, Message *msg);

// @}

//-----------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------

/// @name Internal Functions
// @{

//-----------------------------------------------------------------------------
/// @brief Internal function: Lock the dispatcher mutex.
/// @return true for success, false for failure
/// @see unlockDispatcherMutex()
//-----------------------------------------------------------------------------
extern bool lockDispatcherMutex();

//-----------------------------------------------------------------------------
/// @brief Internal function: Unlock the dispatcher mutex.
/// @see lockDispatcherMutex()
//-----------------------------------------------------------------------------
extern void unlockDispatcherMutex();

//-----------------------------------------------------------------------------
/// @brief Internal function: obtain message queue. Dispatcher mutex must be locked.
/// 
/// @param name Name of the queue
/// @return Message queue
/// @see lockDispatcherMutex(), unlockDispatcherMutex()
//-----------------------------------------------------------------------------
extern MessageQueue *getMessageQueue(const char *name);

// @}

// @}

} // end namespace Dispatcher

// @}

#endif // _DISPATCHER_H_
