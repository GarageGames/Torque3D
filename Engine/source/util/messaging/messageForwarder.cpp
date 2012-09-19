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
#include "util/messaging/messageForwarder.h"

#include "console/consoleTypes.h"

//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------

MessageForwarder::MessageForwarder()
{
   mToQueue = "";
}

MessageForwarder::~MessageForwarder()
{
}

IMPLEMENT_CONOBJECT(MessageForwarder);

ConsoleDocClass( MessageForwarder,
	"@brief Forward messages from one queue to another\n\n"

	"MessageForwarder is a script class that can be used to forward messages "
	"from one queue to another.\n\n"
	
	"@tsexample\n"
	"%fwd = new MessageForwarder()\n"
	"{\n"
	"	toQueue = \"QueueToSendTo\";\n"
	"};\n\n"
	"registerMessageListener(\"FromQueue\", %fwd);\n"
	"@endtsexample\n\n"

	"Where \"QueueToSendTo\" is the queue you want to forward to, and "
	"\"FromQueue\" is the queue you want to forward from.\n\n"

   "@ingroup Messaging\n"
);

//-----------------------------------------------------------------------------

void MessageForwarder::initPersistFields()
{
   addField("toQueue", TypeCaseString, Offset(mToQueue, MessageForwarder), "Name of queue to forward to");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

bool MessageForwarder::onMessageReceived(StringTableEntry queue, const char *event, const char *data)
{
   if(*mToQueue)
      Dispatcher::dispatchMessage(queue, event, data);
   return Parent::onMessageReceived(queue, event, data);
}

bool MessageForwarder::onMessageObjectReceived(StringTableEntry queue, Message *msg)
{
   if(*mToQueue)
      Dispatcher::dispatchMessageObject(mToQueue, msg);
   return Parent::onMessageObjectReceived(queue, msg);
}
