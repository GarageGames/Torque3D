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
#include "torqueConfig.h"
#ifdef TORQUE_ENABLE_ASSET_FILE_CLIENT_REPLICATION
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#include "tcpObject.h"
#include "core/fileObject.h"
#include "platform/platformNet.h"
#include "console/simObject.h"
#include "core/util/str.h"
#include "T3D/gameBase/gameConnection.h"
#include <stack>


class netFileClient : public TCPObject
{
protected:
   typedef TCPObject Parent;
public:
   DECLARE_CALLBACK(void, onProgress, ( F32 percent ));
   DECLARE_CALLBACK(void, onFileTransferMessage, ( const char* msg ));
   DECLARE_CALLBACK(void, onFileTransferComplete, ( ));
   DECLARE_CALLBACK(void, onFileTransferError, (const char* msg ));
   

   netFileClient();
   //Base Class Function Overrides
   virtual U32 onReceive(U8 *buffer, U32 bufferLen);
   virtual void onDownloadComplete();
   virtual bool processLine(UTF8 *line);
   virtual void send(const U8 *buffer, U32 bufferLen);
   virtual void onDisconnect();
   virtual void onDNSFailed();
   virtual void onConnected();
   void RequestSubmitFile(String file);                             //Trys to upload the request file to the server.

private:
   FileObject*           xferFile;                                   //Scratch pad for the sockets file io
   U32                   expectedDataSize;                               //current size of the file
   U32                   totalDataSize;                              //Expected Size of the file.
   bool                  dropRest;                                   //Flag to indicate to throw away rest of transmission.
   String                fileToSend;
   std::stack<String>    filesToDownload;                            //List of all the files this client needs to download.
   void toFile(U8 *buffer, U32 bufferLen);                           //Dump the buffer into the dumpFile
   bool prepareClientWrite(const char* filename,U32 size);           //prepare a file on the server to be written to by this socket.
   void SubmitFile(String file);                                     //Request to upload a file to the server
   
public:
   DECLARE_CONOBJECT(netFileClient);
};


#endif