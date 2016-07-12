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

#include "app/net/tcpObject.h"

#include "platform/platform.h"
#include "console/simBase.h"
#include "console/consoleInternal.h"
#include "core/strings/stringUnit.h"
#include "console/engineAPI.h"
#include "core/stream/fileStream.h"

TCPObject *TCPObject::table[TCPObject::TableSize] = {0, };

IMPLEMENT_CONOBJECT(TCPObject);

ConsoleDocClass( TCPObject,
   "@brief Allows communications between the game and a server using TCP/IP protocols.\n\n"

   "To use TCPObject you set up a connection to a server, send data to the server, and handle "
   "each line of the server's response using a callback.  Once you are done communicating with "
   "the server, you disconnect.\n\n"

   "TCPObject is intended to be used with text based protocols which means you'll need to "
   "delineate the server's response with an end-of-line character. i.e. the newline "
   "character @\\n.  You may optionally include the carriage return character @\\r prior to the newline "
   "and TCPObject will strip it out before sending the line to the callback.  If a newline "
   "character is not included in the server's output, the received data will not be "
   "processed until you disconnect from the server (which flushes the internal buffer).\n\n"

   "TCPObject may also be set up to listen to a specific port, making Torque into a TCP server.  "
   "When used in this manner, a callback is received when a client connection is made.  Following "
   "the outside connection, text may be sent and lines are processed in the usual manner.\n\n"

   "If you want to work with HTTP you may wish to use HTTPObject instead as it handles all of the "
   "HTTP header setup and parsing.\n\n"
   
   "@tsexample\n"
      "// In this example we'll retrieve the new forum threads RSS\n"
      "// feed from garagegames.com.  As we're using TCPObject, the\n"
      "// raw text response will be received from the server, including\n"
      "// the HTTP header.\n\n"

      "// Define callbacks for our specific TCPObject using our instance's\n"
      "// name (RSSFeed) as the namespace.\n\n"

      "// Handle an issue with resolving the server's name\n"
      "function RSSFeed::onDNSFailed(%this)\n"
      "{\n"
      "   // Store this state\n"
      "   %this.lastState = \"DNSFailed\";\n\n"

      "   // Handle DNS failure\n"
      "}\n\n"

      "function RSSFeed::onConnectFailed(%this)\n"
      "{\n"
      "   // Store this state\n"
      "   %this.lastState = \"ConnectFailed\";\n\n"
      "   // Handle connection failure\n"
      "}\n\n"

      "function RSSFeed::onDNSResolved(%this)\n"
      "{\n"
      "   // Store this state\n"
      "   %this.lastState = \"DNSResolved\";\n\n"
      "}\n\n"

      "function RSSFeed::onConnected(%this)\n"
      "{\n"
      "   // Store this state\n"
      "   %this.lastState = \"Connected\";\n\n"
      "}\n\n"

      "function RSSFeed::onDisconnect(%this)\n"
      "{\n"
      "   // Store this state\n"
      "   %this.lastState = \"Disconnected\";\n\n"
      "}\n\n"

      "// Handle a line from the server\n"
      "function RSSFeed::onLine(%this, %line)\n"
      "{\n"
      "   // Print the line to the console\n"
      "   echo( %line );\n"
      "}\n\n"

      "// Create the TCPObject\n"
      "%rss = new TCPObject(RSSFeed);\n\n"

      "// Define a dynamic field to store the last connection state\n"
      "%rss.lastState = \"None\";\n\n"

      "// Connect to the server\n"
      "%rss.connect(\"www.garagegames.com:80\");\n\n"

      "// Send the RSS feed request to the server.  Response will be\n"
      "// handled in onLine() callback above\n"
      "%rss.send(\"GET /feeds/rss/threads HTTP/1.1\\r\\nHost: www.garagegames.com\\r\\n\\r\\n\");\n"
	"@endtsexample\n\n" 
   
   "@see HTTPObject\n"

   "@ingroup Networking\n"
);


IMPLEMENT_CALLBACK(TCPObject, onConnectionRequest, void, (const char* address, const char* ID), (address, ID),
   "@brief Called whenever a connection request is made.\n\n"
   "This callback is used when the TCPObject is listening to a port and a client is attempting to connect.\n"
   "@param address Server address connecting from.\n"
   "@param ID Connection ID\n"
   "@see listen()\n"
   );

IMPLEMENT_CALLBACK(TCPObject, onLine, void, (const char* line), (line),
   "@brief Called whenever a line of data is sent to this TCPObject.\n\n"
   "This callback is called when the received data contains a newline @\\n character, or "
   "the connection has been disconnected and the TCPObject's buffer is flushed.\n"
   "@param line Data sent from the server.\n"
   );

IMPLEMENT_CALLBACK(TCPObject, onPacket, bool, (const char* data), (data),
   "@brief Called when we get a packet with no newlines or nulls (probably websocket).\n\n"
   "@param data Data sent from the server.\n"
   "@return true if script handled the packet.\n"
   );
IMPLEMENT_CALLBACK(TCPObject, onEndReceive, void, (), (),
   "@brief Called when we are done reading all lines.\n\n"
   );

IMPLEMENT_CALLBACK(TCPObject, onDNSResolved, void, (),(),
   "Called whenever the DNS has been resolved.\n"
   );

IMPLEMENT_CALLBACK(TCPObject, onDNSFailed, void, (),(),
   "Called whenever the DNS has failed to resolve.\n"
   );

IMPLEMENT_CALLBACK(TCPObject, onConnected, void, (),(),
   "Called whenever a connection is established with a server.\n"
   );

IMPLEMENT_CALLBACK(TCPObject, onConnectFailed, void, (),(),
   "Called whenever a connection has failed to be established with a server.\n"
   );

IMPLEMENT_CALLBACK(TCPObject, onDisconnect, void, (),(),
   "Called whenever the TCPObject disconnects from whatever it is currently connected to.\n"
   );

TCPObject *TCPObject::find(NetSocket tag)
{
   for(TCPObject *walk = table[U32(tag) & TableMask]; walk; walk = walk->mNext)
      if(walk->mTag == tag)
         return walk;
   return NULL;
}

void TCPObject::addToTable(NetSocket newTag)
{
   removeFromTable();
   mTag = newTag;
   mNext = table[U32(mTag) & TableMask];
   table[U32(mTag) & TableMask] = this;
}

void TCPObject::removeFromTable()
{
   for(TCPObject **walk = &table[U32(mTag) & TableMask]; *walk; walk = &((*walk)->mNext))
   {
      if(*walk == this)
      {
         *walk = mNext;
         return;
      }
   }
}

void processConnectedReceiveEvent(NetSocket sock, RawData incomingData);
void processConnectedAcceptEvent(NetSocket listeningPort, NetSocket newConnection, NetAddress originatingAddress);
void processConnectedNotifyEvent( NetSocket sock, U32 state );

S32 gTCPCount = 0;

TCPObject::TCPObject()
{
   mBuffer = NULL;
   mBufferSize = 0;
   mPort = 0;
   mTag = InvalidSocket;
   mNext = NULL;
   mState = Disconnected;

   gTCPCount++;

   if(gTCPCount == 1)
   {
      Net::smConnectionAccept.notify(processConnectedAcceptEvent);
      Net::smConnectionReceive.notify(processConnectedReceiveEvent);
      Net::smConnectionNotify.notify(processConnectedNotifyEvent);
   }
}

TCPObject::~TCPObject()
{
   disconnect();
   dFree(mBuffer);

   gTCPCount--;

   if(gTCPCount == 0)
   {
      Net::smConnectionAccept.remove(processConnectedAcceptEvent);
      Net::smConnectionReceive.remove(processConnectedReceiveEvent);
      Net::smConnectionNotify.remove(processConnectedNotifyEvent);
   }
}

bool TCPObject::processArguments(S32 argc, ConsoleValueRef *argv)
{
   if(argc == 0)
      return true;
   else if(argc == 1)
   {
      addToTable(U32(dAtoi(argv[0])));
      return true;
   }
   return false;
}

bool TCPObject::onAdd()
{
   if(!Parent::onAdd())
      return false;

   const char *name = getName();

   if(name && name[0] && getClassRep())
   {
      Namespace *parent = getClassRep()->getNameSpace();
      Con::linkNamespaces(parent->mName, name);
      mNameSpace = Con::lookupNamespace(name);

   }

   Sim::getTCPGroup()->addObject(this);

   return true;
}

U32 TCPObject::onReceive(U8 *buffer, U32 bufferLen)
{
   // we got a raw buffer event
   // default action is to split the buffer into lines of text
   // and call processLine on each
   // for any incomplete lines we have mBuffer
   U32 start = 0;
   parseLine(buffer, &start, bufferLen);
   return start;
}

void TCPObject::parseLine(U8 *buffer, U32 *start, U32 bufferLen)
{
   // find the first \n in buffer
   U32 i;
   U8 *line = buffer + *start;

   for(i = *start; i < bufferLen; i++)
      if(buffer[i] == '\n' || buffer[i] == 0)
         break;
   U32 len = i - *start;

   if(i == bufferLen || mBuffer)
   {
      // we've hit the end with no newline
      mBuffer = (U8 *) dRealloc(mBuffer, mBufferSize + len + 1);
      dMemcpy(mBuffer + mBufferSize, line, len);
      mBufferSize += len;
      *start = i;

      // process the line
      if(i != bufferLen)
      {
         mBuffer[mBufferSize] = 0;
         if(mBufferSize && mBuffer[mBufferSize-1] == '\r')
            mBuffer[mBufferSize - 1] = 0;
         U8 *temp = mBuffer;
         mBuffer = 0;
         mBufferSize = 0;

         processLine((UTF8*)temp);
         dFree(temp);
      }
   }
   else if(i != bufferLen)
   {
      line[len] = 0;
      if(len && line[len-1] == '\r')
         line[len-1] = 0;
      processLine((UTF8*)line);
   }
   if(i != bufferLen)
      *start = i + 1;
}

void TCPObject::onConnectionRequest(const NetAddress *addr, U32 connectId)
{
   char idBuf[16];
   char addrBuf[256];
   Net::addressToString(addr, addrBuf);
   dSprintf(idBuf, sizeof(idBuf), "%d", connectId);
   onConnectionRequest_callback(addrBuf,idBuf);
}

bool TCPObject::processLine(UTF8 *line)
{
   onLine_callback(line);
   return true;
}



void TCPObject::onDNSResolved()
{
   mState = DNSResolved;
   onDNSResolved_callback();
}

void TCPObject::onDNSFailed()
{
   mState = Disconnected;
   onDNSFailed_callback();
}


void TCPObject::onConnected()
{
   mState = Connected;
   onConnected_callback();
}

void TCPObject::onConnectFailed()
{
   mState = Disconnected;
   onConnectFailed_callback();
}

bool TCPObject::finishLastLine()
{
   if(mBufferSize)
   {
      mBuffer[mBufferSize] = 0;
      processLine((UTF8*)mBuffer);
      dFree(mBuffer);
      mBuffer = 0;
      mBufferSize = 0;

      return true;
   }

   return false;
}

bool TCPObject::isBufferEmpty()
{
   return (mBufferSize <= 0);
}

void TCPObject::emptyBuffer()
{
   if(mBufferSize)
   {
      dFree(mBuffer);
      mBuffer = 0;
      mBufferSize = 0;
   }
}

void TCPObject::onDisconnect()
{
   finishLastLine();
   mState = Disconnected;
   onDisconnect_callback();
}

void TCPObject::listen(U16 port)
{
   mState = Listening;
   U32 newTag = Net::openListenPort(port);
   addToTable(newTag);
}

void TCPObject::connect(const char *address)
{
   NetSocket newTag = Net::openConnectTo(address);
   addToTable(newTag);
}

void TCPObject::disconnect()
{
   if( mTag != InvalidSocket ) {
      Net::closeConnectTo(mTag);
   }
   removeFromTable();
}

void TCPObject::send(const U8 *buffer, U32 len)
{
   Net::sendtoSocket(mTag, buffer, S32(len));
}

bool TCPObject::sendFile(const char* fileName)
{
   //Open the file for reading
   FileStream readFile;
   if(!readFile.open(fileName, Torque::FS::File::Read))
   {
      return false;
   }

   //Read each byte into our buffer
   Vector<U8> buffer(readFile.getStreamSize());
   readFile.read(buffer.size(), &buffer);

   //Send the buffer
   send(buffer.address(), buffer.size());

   	return true;
}

DefineEngineMethod(TCPObject, send, void, (const char *data),, 
   "@brief Transmits the data string to the connected computer.\n\n"

   "This method is used to send text data to the connected computer regardless if we initiated the "
   "connection using connect(), or listening to a port using listen().\n"

   "@param data The data string to send.\n"

   "@tsexample\n"
      "// Set the command data\n"
      "%data = \"GET \" @ $RSSFeed::serverURL @ \" HTTP/1.0\\r\\n\";\n"
      "%data = %data @ \"Host: \" @ $RSSFeed::serverName @ \"\\r\\n\";\n"
      "%data = %data @ \"User-Agent: \" @ $RSSFeed::userAgent @ \"\\r\\n\\r\\n\"\n\n"

      "// Send the command to the connected server.\n"
      "%thisTCPObj.send(%data);\n"
   "@endtsexample\n")
{
   object->send( (const U8*)data, dStrlen(data) );
}

DefineEngineMethod(TCPObject, sendFile, bool, (const char *fileName),, 
   "@brief Transmits the file in binary to the connected computer.\n\n"

   "@param fileName The filename of the file to transfer.\n")
{
   return object->sendFile(fileName);
}

DefineEngineMethod(TCPObject, finishLastLine, void, (),, 
   "@brief Eat the rest of the lines.\n")
{
   object->finishLastLine();
}

DefineEngineMethod(TCPObject, listen, void, (U32 port),, 
   "@brief Start listening on the specified port for connections.\n\n"

   "This method starts a listener which looks for incoming TCP connections to a port.  "
   "You must overload the onConnectionRequest callback to create a new TCPObject to "
   "read, write, or reject the new connection.\n\n"

   "@param port Port for this TCPObject to start listening for connections on.\n"

   "@tsexample\n"

    "// Create a listener on port 8080.\n"
    "new TCPObject( TCPListener );\n"
    "TCPListener.listen( 8080 );\n\n"

    "function TCPListener::onConnectionRequest( %this, %address, %id )\n"
    "{\n"
    "   // Create a new object to manage the connection.\n"
    "   new TCPObject( TCPClient, %id );\n"
    "}\n\n"

    "function TCPClient::onLine( %this, %line )\n"
    "{\n"
    "   // Print the line of text from client.\n"
    "   echo( %line );\n"
    "}\n"

   "@endtsexample\n")
{
   object->listen(U32(port));
}

DefineEngineMethod(TCPObject, connect, void, (const char* address),, 
   "@brief Connect to the given address.\n\n"

   "@param address Server address (including port) to connect to.\n"

   "@tsexample\n"
      "// Set the address.\n"
      "%address = \"www.garagegames.com:80\";\n\n"

      "// Inform this TCPObject to connect to the specified address.\n"
      "%thisTCPObj.connect(%address);\n"
   "@endtsexample\n")
{
   object->connect(address);
}

DefineEngineMethod(TCPObject, disconnect, void, (),, 
   "@brief Disconnect from whatever this TCPObject is currently connected to, if anything.\n\n"

   "@tsexample\n"
      "// Inform this TCPObject to disconnect from anything it is currently connected to.\n"
      "%thisTCPObj.disconnect();\n"
   "@endtsexample\n")
{
   object->disconnect();
}

void processConnectedReceiveEvent(NetSocket sock, RawData incomingData)
{
   TCPObject *tcpo = TCPObject::find(sock);
   if(!tcpo)
   {
      Con::printf("Got bad connected receive event.");
      return;
   }

   U32 size = incomingData.size;
   U8 *buffer = (U8*)incomingData.data;

   while(size)
   {
      U32 ret = tcpo->onReceive(buffer, size);
      AssertFatal(ret <= size, "Invalid return size");
      size -= ret;
      buffer += ret;
   }

   //If our buffer now has something in it then it's probably a web socket packet and lets handle it
   if(!tcpo->isBufferEmpty())
   {
      //Copy all the data into a string (may be a quicker way of doing this)
      U8 *data = (U8*)incomingData.data;
      String temp;
      for(S32 i = 0; i < incomingData.size; i++)
      {
         temp += data[i];
      }

      //Send the packet to script
      bool handled = tcpo->onPacket_callback(temp);

      //If the script did something with it, clear the buffer
      if(handled)
      {
         tcpo->emptyBuffer();
      }
   }

   tcpo->onEndReceive_callback();
}

void processConnectedAcceptEvent(NetSocket listeningPort, NetSocket newConnection, NetAddress originatingAddress)
{
   TCPObject *tcpo = TCPObject::find(listeningPort);
   if(!tcpo)
      return;

   tcpo->onConnectionRequest(&originatingAddress, newConnection);
}

void processConnectedNotifyEvent( NetSocket sock, U32 state )
{
   TCPObject *tcpo = TCPObject::find(sock);
   if(!tcpo)
      return;

   switch(state)
   {
      case Net::DNSResolved:
         tcpo->onDNSResolved();
         break;
      case Net::DNSFailed:
         tcpo->onDNSFailed();
         break;
      case Net::Connected:
         tcpo->onConnected();
         break;
      case Net::ConnectFailed:
         tcpo->onConnectFailed();
         break;
      case Net::Disconnected:
         tcpo->onDisconnect();
         break;
   }
}
