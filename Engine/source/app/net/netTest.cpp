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
#include "console/simBase.h"
#include "sim/netConnection.h"
#include "core/stream/bitStream.h"
#include "sim/netObject.h"
#include "console/engineAPI.h"

class SimpleMessageEvent : public NetEvent
{
   char *msg;
public:
   typedef NetEvent Parent;
   SimpleMessageEvent(const char *message = NULL)
      {
         if(message)
            msg = dStrdup(message);
         else
            msg = NULL;
      }
   ~SimpleMessageEvent()
      { dFree(msg); }

   virtual void pack(NetConnection* /*ps*/, BitStream *bstream)
      { bstream->writeString(msg); }
   virtual void write(NetConnection*, BitStream *bstream)
      { bstream->writeString(msg); }
   virtual void unpack(NetConnection* /*ps*/, BitStream *bstream)
      { char buf[256]; bstream->readString(buf); msg = dStrdup(buf); }
   virtual void process(NetConnection *)
      { Con::printf("RMSG %d  %s", mSourceId, msg); }

   DECLARE_CONOBJECT(SimpleMessageEvent);
};

IMPLEMENT_CO_NETEVENT_V1(SimpleMessageEvent);

ConsoleDocClass( SimpleMessageEvent,
	"@brief A very simple example of a network event.\n\n"

	"This object exists purely for instructional purposes. It is primarily "
	"geared toward developers that wish to understand the inner-working of "
	"Torque 3D's networking system. This is not intended for actual game "
	"development.\n\n "

	"@see NetEvent for the inner workings of network events\n\n"

	"@ingroup Networking\n");

DefineEngineStaticMethod( SimpleMessageEvent, msg, void, (NetConnection* con, const char* message),,
   "@brief Send a SimpleMessageEvent message to the specified connection.\n\n"

   "The far end that receives the message will print the message out to the console.\n"

   "@param con The unique ID of the connection to transmit to\n"
   "@param message The string containing the message to transmit\n\n"
   
   "@tsexample\n"
      "// Send a message to the other end of the given NetConnection\n"
      "SimpleMessageEvent::msg( %conn, \"A message from me!\");\n\n"

      "// The far end will see the following in the console\n"
      "// (Note: The number may be something other than 1796 as it is the SimObjectID\n"
      "// of the received event)\n"
      "// \n"
      "// RMSG 1796  A message from me!\n"
   "@endtsexample\n\n"
   )
{
	//NetConnection *con = (NetConnection *) Sim::findObject(argv[1]);

	if(con)
		con->postNetEvent(new SimpleMessageEvent(message));
}

//ConsoleFunction( msg, void, 3, 3, "(NetConnection id, string message)"
//                "Send a SimpleNetObject message to the specified connection.")
//{
//   NetConnection *con = (NetConnection *) Sim::findObject(argv[1]);
//   if(con)
//      con->postNetEvent(new SimpleMessageEvent(argv[2]));
//}

class SimpleNetObject : public NetObject
{
   typedef NetObject Parent;
public:
   char message[256];
   SimpleNetObject()
   {
      mNetFlags.set(ScopeAlways | Ghostable);
      dStrcpy(message, "Hello World!");
   }
   U32 packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
   {
      stream->writeString(message);
      return 0;
   }
   void unpackUpdate(NetConnection *conn, BitStream *stream)
   {
      stream->readString(message);
      Con::printf("Got message: %s", message);
   }
   void setMessage(const char *msg)
   {
      setMaskBits(1);
      dStrcpy(message, msg);
   }

   DECLARE_CONOBJECT(SimpleNetObject);
};

IMPLEMENT_CO_NETOBJECT_V1(SimpleNetObject);

ConsoleDocClass( SimpleNetObject,
	"@brief A very simple example of a class derived from NetObject.\n\n"

	"This object exists purely for instructional purposes. It is primarily "
	"geared toward developers that wish to understand the inner-working of "
	"Torque 3D's networking system. This is not intended for actual game "
	"development.\n\n "
   
   "@tsexample\n"
      "// On the server, create a new SimpleNetObject.  This is a ghost always\n"
      "// object so it will be immediately ghosted to all connected clients.\n"
      "$s = new SimpleNetObject();\n\n"

      "// All connected clients will see the following in their console:\n"
      "// \n"
      "// Got message: Hello World!\n"
   "@endtsexample\n\n"

	"@see NetObject for a full breakdown of this example object\n"

	"@ingroup Networking\n");

DefineEngineMethod( SimpleNetObject, setMessage, void, (const char* msg),,
   "@brief Sets the internal message variable.\n\n"

   "SimpleNetObject is set up to automatically transmit this new message to "
   "all connected clients.  It will appear in the clients' console.\n"

   "@param msg The new message to send\n\n"
   
   "@tsexample\n"
      "// On the server, create a new SimpleNetObject.  This is a ghost always\n"
      "// object so it will be immediately ghosted to all connected clients.\n"
      "$s = new SimpleNetObject();\n\n"

      "// All connected clients will see the following in their console:\n"
      "// \n"
      "// Got message: Hello World!\n\n"

      "// Now again on the server, change the message.  This will cause it to\n"
      "// be sent to all connected clients.\n"
      "$s.setMessage(\"A new message from me!\");\n\n"

      "// All connected clients will now see in their console:\n"
      "// \n"
      "// Go message: A new message from me!\n"
   "@endtsexample\n\n"
   )
{
	object->setMessage(msg);
}

//ConsoleMethod( SimpleNetObject, setMessage, void, 3, 3, "(string msg)")
//{
//   object->setMessage(argv[2]);
//}
