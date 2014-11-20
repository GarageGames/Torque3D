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

#include "NetFTPServer.h"
#include "platform/platform.h"
#include "console/simBase.h"
#include "console/consoleInternal.h"
#include "core/strings/stringUnit.h"
#include "console/engineAPI.h"
#include "core/fileObject.h"
#include "T3D/gameBase/gameBase.h"
#include "platform/platformNet.h"
#include <unordered_map>  
#include "console/simObject.h"
#include "platform\platformNet.h"

NetFTPServer *NetFTPServer::table[NetFTPServer::TableSize] = {0, };

NetFTPServer::ConnectionsList NetFTPServer::mConnections;

IMPLEMENT_CONOBJECT(NetFTPServer);

ConsoleDocClass( NetFTPServer,"");
 
Vector<String> NetFTPServer::mFiles;

NetFTPServer *NetFTPServer::find(NetSocket tag)
   {
   for(NetFTPServer *walk = table[U32(tag) & TableMask]; walk; walk = walk->mNext)
      if(walk->mTag == tag)
         return walk;
   return NULL;
   }

void NetFTPServer::addToTable(NetSocket newTag)
   {
   removeFromTable();
   mTag = newTag;
   mNext = table[U32(mTag) & TableMask];
   table[U32(mTag) & TableMask] = this;
   }

void NetFTPServer::removeFromTable()
   {
   for(NetFTPServer **walk = &table[U32(mTag) & TableMask]; *walk; walk = &((*walk)->mNext))
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

S32 gNetFTPServerCount = 0;

NetFTPServer::NetFTPServer()
   {
   mBuffer = NULL;
   mBufferSize = 0;
   mPort = 0;
   mTag = InvalidSocket;
   mState = Disconnected;

   gNetFTPServerCount++;

   if(gNetFTPServerCount == 1)
      {
      Net::smConnectionAccept.notify(processConnectedAcceptEvent);
      Net::smConnectionReceive.notify(processConnectedReceiveEvent);
      Net::smConnectionNotify.notify(processConnectedNotifyEvent);
      }
   pDumpFile = NULL ;
   mDataSize = 0 ;
   mOrigSize = 0;
   }

NetFTPServer::~NetFTPServer()
   {
   disconnect();
   dFree(mBuffer);

   gNetFTPServerCount--;

   if(gNetFTPServerCount == 0)
      {
      Net::smConnectionAccept.remove(processConnectedAcceptEvent);
      Net::smConnectionReceive.remove(processConnectedReceiveEvent);
      Net::smConnectionNotify.remove(processConnectedNotifyEvent);
      }
   }

bool NetFTPServer::processArguments(S32 argc, const char **argv)
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

bool NetFTPServer::onAdd()
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

U32 NetFTPServer::onReceive(U8 *buffer, U32 bufferLen)
   {
   // we got a raw buffer event
   // default action is to split the buffer into lines of text
   // and call processLine on each
   // for any incomplete lines we have mBuffer
   U32 start = 0;
   U32 extractSize = 0 ;

   if(pDumpFile && mDataSize)
      {      
      // could have used start but it's a bit confusing because of the name
      if(bufferLen < mDataSize)
         extractSize = bufferLen ;
      else
         extractSize = mDataSize ;

      dumpToFile(buffer, extractSize) ;      

      start = extractSize ;
      }
   else
      parseLine(buffer, &start, bufferLen);
   return start;
   }

void NetFTPServer::dumpToFile(U8 *buffer, U32 bufferLen)
   {
   if(pDumpFile)
      pDumpFile->writeBinary(bufferLen, buffer) ;

   mDataSize -= bufferLen ;

   if(mDataSize == 0)
      onDownloadComplete();
   }

void NetFTPServer::setDumpFile(FileObject *pFile, U32 nDataSize)
   { 
   pDumpFile = pFile ; 
   mOrigSize = nDataSize ;
   mDataSize = nDataSize ;
   }

void NetFTPServer::parseLine(U8 *buffer, U32 *start, U32 bufferLen)
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

void NetFTPServer::onConnectionRequest(const NetAddress *addr, U32 connectId)
   {
   char idBuf[16];
   char addrBuf[256];
   Net::addressToString(addr, addrBuf);
   dSprintf(idBuf, sizeof(idBuf), "%d", connectId);

   const char * clientid = Con::evaluate((String("%client = new NetFTPServer(ftpClient") + String(idBuf) + String(", ") +  String(idBuf) + String (");return %client;")).c_str());

   NetFTPServer* obj = dynamic_cast<NetFTPServer*>(Sim::findObject(clientid));
   mConnections.push_back(obj);
	Con::printf("Client Connected!");
   }

bool NetFTPServer::processLine(UTF8 *line)
   {
   String sline = String(line);
   String cmd = String("");
   String param = String("");
   String param2 = String("");

   S32 p = 0;
   for (int i = 0;i<sline.length();i++)
      {
      if (sline.substr(i,1)==String(":"))
         p++; 
      else 
         {
         if (p==0)
            cmd +=sline[i];
         else if (p==1)
            param +=sline[i];
         else if (p==2)
            param2 += sline[i];
         }
      }
   if (cmd.equal("list",1))
      _List();
   else if (cmd.equal("get",1))
      _Get(param);
   else if (cmd.equal("finished",1))
      {
      if (mFilesToGrab.size()>0)
         {
         String str = String ("get:") +  mFilesToGrab.first() + String("\n");
         mFilesToGrab.erase(mFilesToGrab.begin());
         send((const U8*)str.c_str(), dStrlen(str.c_str()));
         }
      }
   else if (cmd.equal("writefile",1))
      {
      if (!_ftpPrepareWrite(param,param2))
         {
         mDropRest=true;
         String errmsg = String("Could not update local file '") + param + String("', It is ReadOnly.");
         disconnect();
         return false;
         }
      }
   else if (cmd.equal("requestsubmit"))
      VerifyClientSubmit(param, param2);

   return true;
   }

void NetFTPServer::_Get(String file)
   {
   FileObject* fs = new FileObject();
   if (!fs->readMemory(file.c_str()))
      return;
   char idBuf[16];
   dSprintf(idBuf, sizeof(idBuf), "%u", fs->getSize());
   String str = String("writefile:") + file + String(":")+ String(idBuf) + String("\n");
   send((const U8*)str.c_str(), dStrlen(str.c_str()));
   send((const U8 *) fs->getBuffer(), fs->getSize()) ;
   }


class FileList_SysEvent : public SimEvent
   {
   public:
      S32 mIdx;
      S32 mFileSize;
    FileList_SysEvent(S32 idx,S32 filesize )
      {
      mIdx = idx;
      mFileSize = filesize;
      }
   void process(SimObject* obj)
      {
        NetFTPServer* mobj = dynamic_cast<NetFTPServer*>(obj);
      if (mobj)
         {
         if (mIdx < mFileSize)
            {   
            String filename = mobj->FilesAt(mIdx);
            Torque::FS::FileNodeRef fileRef = Torque::FS::GetFileNode( filename );
            U32 crc = (fileRef->getChecksum());
            String str = String("requestsubmit:") + filename + String(":" ) +String( Con::getIntArg(crc)) + String("\n");
            mobj->send((const U8*)str.c_str(), dStrlen(str.c_str()));
            Sim::postEvent(mobj,new FileList_SysEvent(mIdx+1,mFileSize),Sim::getCurrentTime() + 25);
            }
         else
            {
            String str = String ("finished:\n");
            mobj->send((const U8*)str.c_str(), dStrlen(str.c_str()));
            }
         
         }
      }
   };


void NetFTPServer::_List()
   {
   Sim::postEvent(this,new FileList_SysEvent(0,FilesSize()),Sim::getCurrentTime() + 25);
   }

void NetFTPServer::_SendChatToClient(String msg)
   {
   String str = String ("send:") + msg + String("\n");
   send((const U8*)str.c_str(), dStrlen(str.c_str()));
   }

void NetFTPServer::onDNSResolved()
   {
   mState = DNSResolved;
   }

void NetFTPServer::onDNSFailed()
   {
   mState = Disconnected;
   }


void NetFTPServer::onConnected()
   {
   mState = Connected;
   }

void NetFTPServer::onConnectFailed()
   {
   mState = Disconnected;
   }

void NetFTPServer::finishLastLine()
   {
   if(!mBufferSize)
      return;
   mBuffer[mBufferSize] = 0;
   processLine((UTF8*)mBuffer);
   dFree(mBuffer);
   mBuffer = 0;
   mBufferSize = 0;
   }

void NetFTPServer::onDisconnect()
   {
   mConnections.remove(this);
   finishLastLine();
   mState = Disconnected;
   this->deleteObject();
   }

void NetFTPServer::onDownloadComplete()
   {
   if (fs)
      {
      fs->close();
      fs->deleteObject();
      }
   if (mFilesToGrab.size()>0)
      {
      String str = String ("get:") +  mFilesToGrab.first() + String("\n");
      mFilesToGrab.erase(mFilesToGrab.begin());
      send((const U8*)str.c_str(), dStrlen(str.c_str()));
      }
   }

bool NetFTPServer::listen(U16 port)
   {
   U32 newTag = Net::openListenPort(port);
   if (newTag ==-1)
      return false;
   mState = Listening;
   addToTable(newTag);
   return true;
   }

void NetFTPServer::connect(const char *address)
   {
   NetSocket newTag = Net::openConnectTo(address);
   addToTable(newTag);
   }

void NetFTPServer::disconnect()
   {
   if( mTag != InvalidSocket ) 
      Net::closeConnectTo(mTag);
   removeFromTable();
   }


void NetFTPServer::send(const U8 *buffer, U32 len)
   {
   if (mState==State::Connected)
      {
      
      Net::Error err = Net::sendtoSocket(mTag, buffer, S32(len));
      while (err == Net::Error::WouldBlock)
         err = Net::sendtoSocket(mTag, buffer, S32(len));
      }
   else
      disconnect();
   }

void NetFTPServer::processConnectedReceiveEvent(NetSocket sock, RawData incomingData)
   {
   NetFTPServer *tcpo = NetFTPServer::find(sock);
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
   }

void NetFTPServer::processConnectedAcceptEvent(NetSocket listeningPort, NetSocket newConnection, NetAddress originatingAddress)
   {
   NetFTPServer *tcpo = NetFTPServer::find(listeningPort);
   if(!tcpo)
      return;

   tcpo->onConnectionRequest(&originatingAddress, newConnection);
   }

void NetFTPServer::processConnectedNotifyEvent( NetSocket sock, U32 state )
   {
   NetFTPServer *tcpo = NetFTPServer::find(sock);
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

//Client Submit File
void NetFTPServer::VerifyClientSubmit(String fileName, String crc)
   {
   //Check if the file exists
   if(Torque::FS::IsFile(fileName))
      {
      //Check to see if they are the same
      Torque::FS::FileNodeRef fileRef = Torque::FS::GetFileNode( fileName );

      U32 temp  = dAtoi(crc.c_str());

      //U32 temp =  (U32)&crc;
      if(fileRef->getChecksum() == temp)
         {
         //Send Deny Message and exit
         String str = String("denyWrite:") + fileName + String("\n");
         send((const U8*)str.c_str(), dStrlen(str.c_str()));
         return;
         }
      }

   //See if the file is currently being written too
   for(int i = 0; i < mFilesToGrab.size(); ++i)
      {
      if(mFilesToGrab[i] == fileName)
         {
         //Send Deny Message and exit
         String str = String("denyWrite:") + fileName + String("\n");
         send((const U8*)str.c_str(), dStrlen(str.c_str()));
         return;
         }
      }

   //Add the item to the list of files being written to
   this->mFilesToGrab.push_back(fileName);

   //Not found, accept write
   String str = String("acceptWrite:") + fileName + String("\n");
   send((const U8*)str.c_str(), dStrlen(str.c_str()));
   }

bool NetFTPServer::_ftpPrepareWrite(String param1,String param2)
   {
   if (isWriteable(param1))
      {
      fs = new FileObject();
      fs->openForWrite(param1.c_str(),false);
      setDumpFile(fs,dAtoui(param2.c_str()));
      return true;
      }
   return false;
   }

static char sgScriptFilenameBuffer[1024];
bool NetFTPServer::isWriteable(const char* fileName)
   {
   String filename(Torque::Path::CleanSeparators(fileName));
   Con::expandScriptFilename(sgScriptFilenameBuffer, sizeof(sgScriptFilenameBuffer), filename.c_str());

   Torque::Path givenPath(Torque::Path::CompressPath(sgScriptFilenameBuffer));
   Torque::FS::FileSystemRef fs = Torque::FS::GetFileSystem(givenPath);
   Torque::Path path = fs->mapTo(givenPath);

   return !Torque::FS::IsReadOnly(path);
   }

#endif