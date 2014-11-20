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

#ifndef _NetFTPServer_H_
#define _NetFTPServer_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#include "core/fileObject.h"

#include "platform/platformNet.h"
#include <unordered_map>  
#include <list>
#include "console/simObject.h"
#include "platform\platformNet.h"

#include "core/util/str.h"

#include <stack>

class NetFTPServer : public SimObject
   {
   private:
      typedef std::list<NetFTPServer*> ConnectionsList;
      enum State {Disconnected, DNSResolved, Connected, Listening };
      static ConnectionsList mConnections;
      NetSocket mTag;
      NetFTPServer *mNext;
      enum { TableSize = 256, TableMask = 0xFF };
      static NetFTPServer *table[TableSize];
      State mState;
      void _List();
      void _Get(String file);
      void _SendFile();
      void _SendChatToClient(String msg);
      static Vector<String> mFiles;
      //Reading files from client sender
      bool mDropRest;
      void pushDialog();
      void popdialog();
      bool mDialogPushed;
      SimObject* mMsg;
      Vector<String> mFilesToGrab;
      //File reading functions
      bool _ftpPrepareWrite(String param1,String param2);
      FileObject* fs;
      bool isWriteable(const char* fileName);
   protected:
      typedef SimObject Parent;
      U8 *mBuffer;
      U32 mBufferSize;
      U16 mPort;
      FileObject *pDumpFile ;
      U32 mDataSize ;
      U32 mOrigSize ;
   public:
      void FilesClear(){mFiles.clear();};
      void FilesPush(String file){Con::printf("Pushing File: '%s'",file.c_str());mFiles.push_back_unique(file);};
      String FilesAt(S32 i){return mFiles[i];};
      S32 FilesSize(){return mFiles.size();};
      NetFTPServer();
      virtual ~NetFTPServer();
      void setDumpFile(FileObject *pFile, U32 nDataSize) ;
      void dumpToFile(U8 *buffer, U32 bufferLen);
      void parseLine(U8 *buffer, U32 *start, U32 bufferLen);
      void finishLastLine();
      static NetFTPServer *find(NetSocket tag);
      virtual U32 onReceive(U8 *buffer, U32 bufferLen);
      virtual bool processLine(UTF8 *line);
      virtual void onDNSResolved();
      virtual void onDNSFailed();
      virtual void onConnected();
      virtual void onConnectFailed();
      virtual void onConnectionRequest(const NetAddress *addr, U32 connectId);
      virtual void onDisconnect();
      virtual void onDownloadComplete();
      void connect(const char *address);
      bool listen(U16 port);
      void disconnect();
      State getState() { return mState; }
      bool processArguments(S32 argc, const char **argv);
      void send(const U8 *buffer, U32 bufferLen);
      void addToTable(NetSocket newTag);
      void removeFromTable();
      void setPort(U16 port) { mPort = port; }
      bool onAdd();
      static void processConnectedReceiveEvent(NetSocket sock, RawData incomingData);
      static void processConnectedAcceptEvent(NetSocket listeningPort, NetSocket newConnection, NetAddress originatingAddress);
      static void processConnectedNotifyEvent( NetSocket sock, U32 state );
      DECLARE_CONOBJECT(NetFTPServer);
      //Client Submitting a file
      void VerifyClientSubmit(String fileName,  String crc);
};




#endif  // _H_NetFTPServer_
#endif