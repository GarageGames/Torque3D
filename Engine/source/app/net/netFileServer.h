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

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#include "tcpObject.h"
#include "core/fileObject.h"
#include "platform/platformNet.h"
#include "console/simObject.h"
#include "core/util/str.h"


class netFileServer : public TCPObject
{
protected:
   typedef TCPObject Parent;
public:
   DECLARE_CONOBJECT(netFileServer);
   netFileServer();
   //Base Class Function Overrides
   virtual U32 onReceive(U8 *buffer, U32 bufferLen);
   virtual void onDownloadComplete();
   virtual void onConnectionRequest(const NetAddress *addr, NetSocket connectId);
   virtual bool processLine(UTF8 *line);
   virtual void send(const U8 *buffer, U32 bufferLen);
   virtual void onDisconnect();
    
   void SendChatToClient(const char* msg);                           //Sends a message to a client
   inline String FilesAt(S32 i){return files[i];};                   //Retrieves the file at the index

   void LoadPathPattern(const char* pattern, bool recursive, bool multiMatch, bool verbose);
   bool LoadFile(const char* file);

   bool start (S32 port = 0);
   void stop ();
   inline void FilesClear(){files.clear();};                         //Clears the file list
private:

   
   inline void FilesPush(String file){files.push_back_unique(file);};//Pushes a file onto the stack
   inline S32 FilesSize(){return files.size();};

   static Vector<String> files;                                      //Shared list of all files that will be sent down to all clients
   Vector<String>        currentlyUploadingFiles;                    //Files being uploaded to the server
   FileObject*           xferFile;                                   //Scratch pad for the sockets file io
   U32                   expectedDataSize;                           //expected size of the file
   bool                  dropRest;                                   //Flag to indicate to throw away rest of transmission.

   void toFile(U8 *buffer, U32 bufferLen);                           //Dump the buffer into the dumpFile
   void SendFileToClient(String file);                               //Get the real file and send it to the client
   void SendFileListToClient();                                      //Get all the files plus there crc and send it to the client
   bool prepareWrite(const char* filename,U32 size);                 //prepare a file on the server to be written to by this socket.
   void VerifyClientSubmit(String fileName,  String crc);            //Checks to see if the socket is allowed to write to this file on the server.




};