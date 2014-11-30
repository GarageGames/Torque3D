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

#include "app/net/httpObject.h"

#include "platform/platform.h"
#include "core/stream/fileStream.h"
#include "console/simBase.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(HTTPObject);

ConsoleDocClass( HTTPObject,
   "@brief Allows communications between the game and a server using HTTP.\n\n"
   
   "HTTPObject is derrived from TCPObject and makes use of the same callbacks for dealing with "
   "connections and received data.  However, the way in which you use HTTPObject to connect "
   "with a server is different than TCPObject.  Rather than opening a connection, sending data, "
   "waiting to receive data, and then closing the connection, you issue a get() or post() and "
   "handle the response.  The connection is automatically created and destroyed for you.\n\n"
   
   "@tsexample\n"
      "// In this example we'll retrieve the weather in Las Vegas using\n"
      "// Google's API.  The response is in XML which could be processed\n"
      "// and used by the game using SimXMLDocument, but we'll just output\n"
      "// the results to the console in this example.\n\n"

      "// Define callbacks for our specific HTTPObject using our instance's\n"
      "// name (WeatherFeed) as the namespace.\n\n"

      "// Handle an issue with resolving the server's name\n"
      "function WeatherFeed::onDNSFailed(%this)\n"
      "{\n"
      "   // Store this state\n"
      "   %this.lastState = \"DNSFailed\";\n\n"

      "   // Handle DNS failure\n"
      "}\n\n"

      "function WeatherFeed::onConnectFailed(%this)\n"
      "{\n"
      "   // Store this state\n"
      "   %this.lastState = \"ConnectFailed\";\n\n"
      "   // Handle connection failure\n"
      "}\n\n"

      "function WeatherFeed::onDNSResolved(%this)\n"
      "{\n"
      "   // Store this state\n"
      "   %this.lastState = \"DNSResolved\";\n\n"
      "}\n\n"

      "function WeatherFeed::onConnected(%this)\n"
      "{\n"
      "   // Store this state\n"
      "   %this.lastState = \"Connected\";\n\n"

      "   // Clear our buffer\n"
      "   %this.buffer = \"\";\n"
      "}\n\n"

      "function WeatherFeed::onDisconnect(%this)\n"
      "{\n"
      "   // Store this state\n"
      "   %this.lastState = \"Disconnected\";\n\n"

      "   // Output the buffer to the console\n"
      "   echo(\"Google Weather Results:\");\n"
      "   echo(%this.buffer);\n"
      "}\n\n"

      "// Handle a line from the server\n"
      "function WeatherFeed::onLine(%this, %line)\n"
      "{\n"
      "   // Store this line in out buffer\n"
      "   %this.buffer = %this.buffer @ %line;\n"
      "}\n\n"

      "// Create the HTTPObject\n"
      "%feed = new HTTPObject(WeatherFeed);\n\n"

      "// Define a dynamic field to store the last connection state\n"
      "%feed.lastState = \"None\";\n\n"

      "// Send the GET command\n"
      "%feed.get(\"www.google.com:80\", \"/ig/api\", \"weather=Las-Vegas,US\");\n"
	"@endtsexample\n\n" 
   
   "@see TCPObject\n"

   "@ingroup Networking\n"
);

//--------------------------------------

HTTPObject::HTTPObject()
{
   mHostName = 0;
   mPath = 0;
   mQuery = 0;
   mPost = 0;
   mBufferSave = 0;
}

HTTPObject::~HTTPObject()
{
   dFree(mHostName);
   dFree(mPath);
   dFree(mQuery);
   dFree(mPost);

   mHostName = 0;
   mPath = 0;
   mQuery = 0;
   mPost = 0;
   dFree(mBufferSave);
}

//--------------------------------------
//--------------------------------------
void HTTPObject::get(const char *host, const char *path, const char *query)
{
   if(mHostName)
      dFree(mHostName);
   if(mPath)
      dFree(mPath);
   if(mQuery)
      dFree(mQuery);
   if(mPost)
      dFree(mPost);
   if(mBufferSave)
      dFree(mBufferSave);

   mBufferSave = 0;
   mHostName = dStrdup(host);
   mPath = dStrdup(path);
   if(query)
      mQuery = dStrdup(query);
   else
      mQuery = NULL;
   mPost = NULL;

   connect(host);
}

void HTTPObject::post(const char *host, const char *path, const char *query, const char *post)
{
   if(mHostName)
      dFree(mHostName);
   if(mPath)
      dFree(mPath);
   if(mQuery)
      dFree(mQuery);
   if(mPost)
      dFree(mPost);
   if(mBufferSave)
      dFree(mBufferSave);

   mBufferSave = 0;
   mHostName = dStrdup(host);
   mPath = dStrdup(path);
   if(query && query[0])
      mQuery = dStrdup(query);
   else
      mQuery = NULL;
   mPost = dStrdup(post);
   connect(host);
}

static char getHex(char c)
{
   if(c <= 9)
      return c + '0';
   return c - 10 + 'A';
}

static S32 getHexVal(char c)
{
   if(c >= '0' && c <= '9')
      return c - '0';
   else if(c >= 'A' && c <= 'Z')
      return c - 'A' + 10;
   else if(c >= 'a' && c <= 'z')
      return c - 'a' + 10;
   return -1;
}

void HTTPObject::expandPath(char *dest, const char *path, U32 destSize)
{
   static bool asciiEscapeTableBuilt = false;
   static bool asciiEscapeTable[256];
   if(!asciiEscapeTableBuilt)
   {
      asciiEscapeTableBuilt = true;
      U32 i;
      for(i = 0; i <= ' '; i++)
         asciiEscapeTable[i] = true;
      for(;i <= 0x7F; i++)
         asciiEscapeTable[i] = false;
      for(;i <= 0xFF; i++)
         asciiEscapeTable[i] = true;
      asciiEscapeTable[static_cast<U32>('\"')] = true;
      asciiEscapeTable[static_cast<U32>('_')] = true;
      asciiEscapeTable[static_cast<U32>('\'')] = true;
      asciiEscapeTable[static_cast<U32>('#')] = true;
      asciiEscapeTable[static_cast<U32>('$')] = true;
      asciiEscapeTable[static_cast<U32>('%')] = true;
      asciiEscapeTable[static_cast<U32>('&')] = false;
      asciiEscapeTable[static_cast<U32>('+')] = true;
      asciiEscapeTable[static_cast<U32>('-')] = true;
      asciiEscapeTable[static_cast<U32>('~')] = true;
   }

   U32 destIndex = 0;
   U32 srcIndex = 0;
   while(path[srcIndex] && destIndex < destSize - 3)
   {
      char c = path[srcIndex++];
      if(asciiEscapeTable[static_cast<U32>(c)])
      {
         dest[destIndex++] = '%';
         dest[destIndex++] = getHex((c >> 4) & 0xF);
         dest[destIndex++] = getHex(c & 0xF);
      }
      else
         dest[destIndex++] = c;
   }
   dest[destIndex] = 0;
}

//--------------------------------------
void HTTPObject::onConnected()
{
   Parent::onConnected();
   char expPath[8192];
   char buffer[8192];

   if(mQuery)
   {
      dSprintf(buffer, sizeof(buffer), "%s?%s", mPath, mQuery);
      expandPath(expPath, buffer, sizeof(expPath));
   }
   else
      expandPath(expPath, mPath, sizeof(expPath));

   char *pt = dStrchr(mHostName, ':');
   if(pt)
      *pt = 0;

   //If we want to do a get request
   if(mPost == NULL)
   {
      dSprintf(buffer, sizeof(buffer), "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", expPath, mHostName);
   }
   //Else we want to do a post request
   else
   {
      dSprintf(buffer, sizeof(buffer), "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %i\r\n\r\n%s\r\n\r\n",
         expPath, mHostName, dStrlen(mPost), mPost);
   }

   if(pt)
      *pt = ':';

   send((U8*)buffer, dStrlen(buffer));
   mParseState = ParsingStatusLine;
   mChunkedEncoding = false;
}

void HTTPObject::onConnectFailed()
{
   dFree(mHostName);
   dFree(mPath);
   dFree(mQuery);
   mHostName = 0;
   mPath = 0;
   mQuery = 0;
   Parent::onConnectFailed();
}


void HTTPObject::onDisconnect()
{
   dFree(mHostName);
   dFree(mPath);
   dFree(mQuery);
   mHostName = 0;
   mPath = 0;
   mQuery = 0;
   Parent::onDisconnect();
}

bool HTTPObject::processLine(UTF8 *line)
{
   if(mParseState == ParsingStatusLine)
   {
      mParseState = ParsingHeader;
   }
   else if(mParseState == ParsingHeader)
   {
      if(!dStricmp((char *) line, "transfer-encoding: chunked"))
         mChunkedEncoding = true;
      if(line[0] == 0)
      {
         if(mChunkedEncoding)
            mParseState = ParsingChunkHeader;
         else
            mParseState = ProcessingBody;
         return true;
      }
   }
   else if(mParseState == ParsingChunkHeader)
   {
      if(line[0]) // strip off the crlf if necessary
      {
         mChunkSize = 0;
         S32 hexVal;
         while((hexVal = getHexVal(*line++)) != -1)
         {
            mChunkSize *= 16;
            mChunkSize += hexVal;
         }
         if(mBufferSave)
         {
            mBuffer = mBufferSave;
            mBufferSize = mBufferSaveSize;
            mBufferSave = 0;
         }
         if(mChunkSize)
            mParseState = ProcessingBody;
         else
         {
            mParseState = ProcessingDone;
            finishLastLine();
         }
      }
   }
   else
   {
      return Parent::processLine((UTF8*)line);
   }
   return true;
}

U32 HTTPObject::onDataReceive(U8 *buffer, U32 bufferLen)
{
   U32 start = 0;
   parseLine(buffer, &start, bufferLen);
   return start;
}

//--------------------------------------
U32 HTTPObject::onReceive(U8 *buffer, U32 bufferLen)
{
   if(mParseState == ProcessingBody)
   {
      if(mChunkedEncoding && bufferLen >= mChunkSize)
      {
         U32 ret = onDataReceive(buffer, mChunkSize);
         mChunkSize -= ret;
         if(mChunkSize == 0)
         {
            if(mBuffer)
            {
               mBufferSaveSize = mBufferSize;
               mBufferSave = mBuffer;
               mBuffer = 0;
               mBufferSize = 0;
            }
            mParseState = ParsingChunkHeader;
         }
         return ret;
      }
      else
      {
         U32 ret = onDataReceive(buffer, bufferLen);
         mChunkSize -= ret;
         return ret;
      }
   }
   else if(mParseState != ProcessingDone)
   {
      U32 start = 0;
      parseLine(buffer, &start, bufferLen);
      return start;
   }
   return bufferLen;
}

//--------------------------------------
DefineEngineMethod( HTTPObject, get, void, ( const char* Address, const char* requirstURI, const char* query ), ( "" ),
   "@brief Send a GET command to a server to send or retrieve data.\n\n"

   "@param Address HTTP web address to send this get call to. Be sure to include the port at the end (IE: \"www.garagegames.com:80\").\n"
   "@param requirstURI Specific location on the server to access (IE: \"index.php\".)\n"
   "@param query Optional. Actual data to transmit to the server. Can be anything required providing it sticks with limitations of the HTTP protocol. "
   "If you were building the URL manually, this is the text that follows the question mark.  For example: http://www.google.com/ig/api?<b>weather=Las-Vegas,US</b>\n"
   
   "@tsexample\n"
	   "// Create an HTTP object for communications\n"
	   "%httpObj = new HTTPObject();\n\n"
	   "// Specify a URL to transmit to\n"
      "%url = \"www.garagegames.com:80\";\n\n"
	   "// Specify a URI to communicate with\n"
	   "%URI = \"/index.php\";\n\n"
	   "// Specify a query to send.\n"
	   "%query = \"\";\n\n"
	   "// Send the GET command to the server\n"
	   "%httpObj.get(%url,%URI,%query);\n"
   "@endtsexample\n\n"
   )
{
   if( !query || !query[ 0 ] )
		object->get(Address, requirstURI, NULL);
   else
		object->get(Address, requirstURI, query);
}

DefineEngineMethod( HTTPObject, post, void, ( const char* Address, const char* requirstURI, const char* query, const char* post ),,
   "@brief Send POST command to a server to send or retrieve data.\n\n"

   "@param Address HTTP web address to send this get call to. Be sure to include the port at the end (IE: \"www.garagegames.com:80\").\n"
   "@param requirstURI Specific location on the server to access (IE: \"index.php\".)\n"
   "@param query Actual data to transmit to the server. Can be anything required providing it sticks with limitations of the HTTP protocol. \n"
   "@param post Submission data to be processed.\n"

   "@tsexample\n"
	   "// Create an HTTP object for communications\n"
	   "%httpObj = new HTTPObject();\n\n"
	   "// Specify a URL to transmit to\n"
      "%url = \"www.garagegames.com:80\";\n\n"
	   "// Specify a URI to communicate with\n"
	   "%URI = \"/index.php\";\n\n"
	   "// Specify a query to send.\n"
	   "%query = \"\";\n\n"
	   "// Specify the submission data.\n"
	   "%post = \"\";\n\n"
	   "// Send the POST command to the server\n"
	   "%httpObj.POST(%url,%URI,%query,%post);\n"
   "@endtsexample\n\n"
   )
{
   object->post(Address, requirstURI, query, post);
}
