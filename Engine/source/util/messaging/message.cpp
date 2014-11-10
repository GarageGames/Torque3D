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
#include "util/messaging/message.h"

#include "console/consoleTypes.h"
#include "core/util/safeDelete.h"
#include "core/stream/bitStream.h"
#include "console/engineAPI.h"

//-----------------------------------------------------------------------------

namespace Sim
{
extern SimIdDictionary *gIdDictionary;
}

//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------

Message::Message()
{
}


IMPLEMENT_CONOBJECT(Message);

ConsoleDocClass( Message,
	"@brief Base class for messages\n\n"
	
	"Message is the base class for C++ defined messages, and may also be used "
	"in script for script defined messages if no C++ subclass is appropriate.\n\n"

	"Messages are reference counted and will be automatically deleted when "
	"their reference count reaches zero. When you dispatch a message, a "
	"reference will be added before the dispatch and freed after the dispatch. "
	"This allows for temporary messages with no additional code. If you want "
	"to keep the message around, for example to dispatch it to multiple "
	"queues, call addReference() before dispatching it and freeReference() "
	"when you are done with it. Never delete a Message object directly "
	"unless addReference() has not been called or the message has not been "
	"dispatched.\n\n"

	"Message IDs are pooled similarly to datablocks, with the exception that "
	"IDs are reused. If you keep a message for longer than a single dispatch, "
	"then you should ensure that you clear any script variables that refer "
	"to it after the last freeReference(). If you don't, then it is probable "
	"that the object ID will become valid again in the future and could cause "
	"hard to track down bugs.\n\n"

	"Messages have a unique type to simplify message handling code. For object "
	"messages, the type is defined as either the script defined class name "
	"or the C++ class name if no script class was defined. The message type "
	"may be obtained through the getType() method.\n\n"

	"By convention, any data for the message is held in script accessible "
	"fields. Messages that need to be handled in C++ as well as script "
	"provide the relevant data through persistent fields in a subclass of "
	"Message to provide best performance on the C++ side. Script defined "
	"messages usually their through dynamic fields, and may be accessed in " 
	"C++ using the SimObject::getDataField() method.\n\n"

   "@ingroup Messaging\n"
);

//-----------------------------------------------------------------------------
IMPLEMENT_CALLBACK(Message, onAdd, void, (),(),
				   "Script callback when a message is first created and registered.\n\n"
				   "@tsexample\n"
				   "function Message::onAdd(%this)\n"
				   "{\n"
				   "	// Perform on add code here\n"
				   "}\n"
				   "@endtsexample\n\n"
				   );

bool Message::onAdd()
{
   if(! Parent::onAdd())
      return false;

   linkNamespaces();
   onAdd_callback();
   //Con::executef(this, "onAdd");
   return true;
}

IMPLEMENT_CALLBACK(Message, onRemove, void, (),(),
				   "Script callback when a message is deleted.\n\n"
				   "@tsexample\n"
				   "function Message::onRemove(%this)\n"
				   "{\n"
				   "	// Perform on remove code here\n"
				   "}\n"
				   "@endtsexample\n\n"
				   );

void Message::onRemove()
{   
   onRemove_callback();
   //Con::executef(this, "onRemove");
   unlinkNamespaces();
   
   Parent::onRemove();
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

SimObjectId Message::getNextMessageID()
{
   for(S32 i = MessageObjectIdFirst;i < MessageObjectIdLast;i++)
   {
      if(Sim::gIdDictionary->find(i) == NULL)
         return i;
   }

   // Oh no ...
   return 0xffffffff;
}

//-----------------------------------------------------------------------------

const char *Message::getType()
{
   if(mClassName && mClassName[0] != 0)
      return mClassName;

   return getClassName();
}

//-----------------------------------------------------------------------------
// Console Methods
//-----------------------------------------------------------------------------

DefineConsoleMethod(Message, getType, const char *, (), , "() Get message type (script class name or C++ class name if no script defined class)")
{
   return object->getType();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod(Message, addReference, void, (), , "() Increment the reference count for this message")
{
   object->addReference();
}

DefineConsoleMethod(Message, freeReference, void, (), , "() Decrement the reference count for this message")
{
   object->freeReference();
}
