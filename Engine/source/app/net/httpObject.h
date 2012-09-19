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

#ifndef _HTTPOBJECT_H_
#define _HTTPOBJECT_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _TCPOBJECT_H_
#include "app/net/tcpObject.h"
#endif

class HTTPObject : public TCPObject
{
private:
   typedef TCPObject Parent;
protected:
   enum ParseState {
      ParsingStatusLine,
      ParsingHeader,
      ParsingChunkHeader,
      ProcessingBody,
      ProcessingDone,
   };
   ParseState mParseState;
   U32 mTotalBytes;
   U32 mBytesRemaining;
 public:
   U32 mStatus;
   F32 mVersion;
   U32 mContentLength;
   bool mChunkedEncoding;
   U32 mChunkSize;
   const char *mContentType;
   char *mHostName;
   char *mPath;
   char *mQuery;
   char *mPost;
   U8 *mBufferSave;
   U32 mBufferSaveSize;
public:
   static void expandPath(char *dest, const char *path, U32 destSize);
   void get(const char *hostName, const char *urlName, const char *query);
   void post(const char *host, const char *path, const char *query, const char *post);
   HTTPObject();
   ~HTTPObject();

   //static HTTPObject *find(U32 tag);

   virtual U32 onDataReceive(U8 *buffer, U32 bufferLen);
   virtual U32 onReceive(U8 *buffer, U32 bufferLen); // process a buffer of raw packet data
   virtual void onConnected();
   virtual void onConnectFailed();
   virtual void onDisconnect();
   bool processLine(U8 *line);

   DECLARE_CONOBJECT(HTTPObject);
};


#endif  // _H_HTTPOBJECT_
