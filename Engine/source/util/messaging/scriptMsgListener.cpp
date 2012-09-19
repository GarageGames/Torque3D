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
#include "util/messaging/scriptMsgListener.h"

#include "console/consoleTypes.h"

#include "console/engineAPI.h"

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

ScriptMsgListener::ScriptMsgListener()
{
}

IMPLEMENT_CONOBJECT(ScriptMsgListener);

ConsoleDoc("@class ScriptMsgListener\n"
	"@brief Script accessible version of Dispatcher::IMessageListener. Often used in conjunction with EventManager\n\n"

	"The main use of ScriptMsgListener is to allow script to listen for"
	"messages. You can subclass ScriptMsgListener in script to receive"
	"the Dispatcher::IMessageListener callbacks.\n\n"

	"Alternatively, you can derive from it in C++ instead of SimObject to"
	"get an object that implements Dispatcher::IMessageListener with script"
	"callbacks. If you need to derive from something other then SimObject,"
	"then you will need to implement the Dispatcher::IMessageListener"
	"interface yourself.\n\n"

	"@tsexample\n"
	"// Create the EventManager.\n"
	"$MyEventManager = new EventManager() { queue = \"MyEventManager\"; };\n\n"
	"// Create an event.\n"
	"$MyEventManager.registerEvent( \"SomeCoolEvent\" );\n\n"
	"// Create a listener and subscribe.\n"
	"$MyListener = new ScriptMsgListener() { class = MyListener; };\n"
	"$MyEventManager.subscribe( $MyListener, \"SomeCoolEvent\" );\n\n"
	"function MyListener::onSomeCoolEvent( %this, %data )\n"
	"{\n"
	"	echo( \"onSomeCoolEvent Triggered\" );\n"
	"}\n\n"
	"// Trigger the event.\n"
	"$MyEventManager.postEvent( \"SomeCoolEvent\", \"Data\" );\n"
	"@endtsexample\n\n"

   "@ingroup Messaging\n"
);

//-----------------------------------------------------------------------------
IMPLEMENT_CALLBACK(ScriptMsgListener, onAdd, void, (),(),
				   "Script callback when a listener is first created and registered.\n\n"
				   "@tsexample\n"
				   "function ScriptMsgListener::onAdd(%this)\n"
				   "{\n"
				   "	// Perform on add code here\n"
				   "}\n"
				   "@endtsexample\n\n"
				   );

bool ScriptMsgListener::onAdd()
{
   if(! Parent::onAdd())
      return false;

   linkNamespaces();
   onAdd_callback();
   //Con::executef(this, "onAdd");
   return true;
}

IMPLEMENT_CALLBACK(ScriptMsgListener, onRemove, void, (),(),
				   "Script callback when a listener is deleted.\n\n"
				   "@tsexample\n"
				   "function ScriptMsgListener::onRemove(%this)\n"
				   "{\n"
				   "	// Perform on remove code here\n"
				   "}\n"
				   "@endtsexample\n\n"
				   );

void ScriptMsgListener::onRemove()
{
   onRemove_callback();
   //Con::executef(this, "onRemove");
   unlinkNamespaces();
   
   Parent::onRemove();
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------
IMPLEMENT_CALLBACK( ScriptMsgListener, onMessageReceived, bool, ( const char* queue, const char* event, const char* data ), ( queue, event, data ),
   "Called when the listener has received a message.\n"
   "@param queue The name of the queue the message was dispatched to\n"
   "@param event The name of the event (function) that was triggered\n"
   "@param data The data (parameters) for the message\n\n"
   "@return false to prevent other listeners receiving this message, true otherwise\n" );

bool ScriptMsgListener::onMessageReceived(StringTableEntry queue, const char* event, const char* data)
{
	return onMessageReceived_callback(queue, event, data);
   //return dAtob(Con::executef(this, "onMessageReceived", queue, event, data));
}

IMPLEMENT_CALLBACK( ScriptMsgListener, onMessageObjectReceived, bool, ( const char* queue, Message *msg ), ( queue, msg ),
   "Called when a message object (not just the message data) is passed to a listener.\n"
   "@param queue The name of the queue the message was dispatched to\n"
   "@param msg The message object\n"
   "@return false to prevent other listeners receiving this message, true otherwise\n" 
   "@see Message\n"
   "@see onMessageReceived");

bool ScriptMsgListener::onMessageObjectReceived(StringTableEntry queue, Message *msg)
{
	return onMessageObjectReceived_callback(queue, msg);
   //return dAtob(Con::executef(this, "onMessageObjectReceived", queue, Con::getIntArg(msg->getId())));
}

//-----------------------------------------------------------------------------
IMPLEMENT_CALLBACK( ScriptMsgListener, onAddToQueue, void, ( const char* queue), ( queue),
	"@brief Callback for when the listener is added to a queue\n\n"
	"The default implementation of onAddToQueue() and onRemoveFromQueue() "
	"provide tracking of the queues this listener is added to through the "
	"mQueues member. Overrides of onAddToQueue() or onRemoveFromQueue() "
	"should ensure they call the parent implementation in any overrides.\n"
	"@param queue The name of the queue that the listener added to\n"
   	"@see onRemoveFromQueue()");

void ScriptMsgListener::onAddToQueue(StringTableEntry queue)
{
   //Con::executef(this, "onAddToQueue", queue);
   onAddToQueue_callback(queue);
   IMLParent::onAddToQueue(queue);
}

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
IMPLEMENT_CALLBACK( ScriptMsgListener, onRemoveFromQueue, void, ( const char* queue), ( queue),
	"@brief Callback for when the listener is removed from a queue\n\n"
	"The default implementation of onAddToQueue() and onRemoveFromQueue() "
	"provide tracking of the queues this listener is added to through the "
	"mQueues member. Overrides of onAddToQueue() or onRemoveFromQueue() "
	"should ensure they call the parent implementation in any overrides.\n"
	"@param queue The name of the queue that the listener was removed from\n"
   	"@see onAddToQueue()");

void ScriptMsgListener::onRemoveFromQueue(StringTableEntry queue)
{
   //Con::executef(this, "onRemoveFromQueue", queue);
   onRemoveFromQueue_callback(queue);
   IMLParent::onRemoveFromQueue(queue);
}
