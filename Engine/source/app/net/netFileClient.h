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
   netFileClient();

   //Base Class Function Overrides
   virtual U32 onReceive(U8 *buffer, U32 bufferLen);
   virtual void onDownloadComplete();
   virtual bool processLine(UTF8 *line);
   virtual void send(const U8 *buffer, U32 bufferLen);
   virtual void onDisconnect();
   virtual void onDNSFailed();
   virtual void onConnected();
   virtual void onConnectFailed();
   static void ConnectAndDownload(const char* remoteAddress,GameConnection* gc);//Main Entry Point for Class.
   void RequestSubmitFile(String file);                             //Trys to upload the request file to the server.
   GameConnection* getGameConnection(){return mGameConnection;}     //Returns the GameConnection to establish after download.
   const char* getRemoteAddress(){return remoteAddress;}            //Returns the RemoteAddress.

private:
   FileObject*           xferFile;                                   //Scratch pad for the sockets file io
   U32                   dataSize ;                                  //current size of the file
   U32                   totalDataSize;                              //Expected Size of the file.
   FileObject*           fs;                                         //Socket's FileObject
   bool                  dropRest;                                   //Flag to indicate to throw away rest of transmission.
   bool                  isDownloadDialogPushed;                     //Flag indicating whether or not the Download Display Gui is showing.
   String                fileToSend;
   SimObject*            gcDownloadDisplayDlg;                       //Reference to the Download Display Gui Control.
   SimObject*            gtcDisplayMessage;                          //Reference to the GuiTextControl which shows the user messages about download.
   SimObject*            canvas;                                     //Reference to the main canvas.
   SimObject*            pbDownloadProgress;                         //Reference to a ProgressBar that shows download completion.
   std::stack<String>    filesToDownload;                            //List of all the files this client needs to download.
   GameConnection*       mGameConnection;                            //The calling GameConnection that needs to be connected to the server after download.
   const char*           remoteAddress;                              //The address the calling GameConnection wants to connect to.
   void toFile(U8 *buffer, U32 bufferLen);                           //Dump the buffer into the dumpFile
   void setXferFile(FileObject *pFile, U32 nDataSize);               //Assign the dumpFile to a real File
   bool prepareWrite(const char* filename,U32 size);                 //prepare a file on the server to be written to by this socket.
   void SubmitFile(String file);                                     //Request to upload a file to the server
   void pushDialog();                                                //Close the Downloading Gui
   void popdialog();                                                 //Show the Downloading Gui
   static void _CreateGui();                                         //Create the Downloading gui
   
public:
   DECLARE_CONOBJECT(netFileClient);
};


#endif