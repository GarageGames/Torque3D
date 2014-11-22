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

#include "netFileServer.h"
#include "netFileUtils.h"

IMPLEMENT_CONOBJECT(netFileServer);

ConsoleDocClass( netFileServer,"");

Vector<String> netFileServer::files;

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
      netFileServer* mobj = dynamic_cast<netFileServer*>(obj);
      if (mobj)
         {
         if (mIdx < mFileSize)
            {   
            String filename = mobj->FilesAt(mIdx);
            Torque::FS::FileNodeRef fileRef = Torque::FS::GetFileNode( filename );
            U32 crc = (fileRef->getChecksum());
            String str = netFileCommands::requestsubmit + String(":") + filename + String(":" ) +String( netFileUtils::uinttochar(crc)) + String("\n");
            mobj->send((const U8*)str.c_str(), dStrlen(str.c_str()));
            Sim::postEvent(mobj,new FileList_SysEvent(mIdx+1,mFileSize),Sim::getCurrentTime() + 25);
            }
         else
            {
            String str = netFileCommands::finished + String (":\n");
            mobj->send((const U8*)str.c_str(), dStrlen(str.c_str()));
            }
         }
      }
   };


netFileServer::netFileServer()
{
   xferFile = NULL ;
   dataSize = 0 ;
}

U32 netFileServer::onReceive(U8 *buffer, U32 bufferLen)
   {
   // we got a raw buffer event
   // default action is to split the buffer into lines of text
   // and call processLine on each
   // for any incomplete lines we have mBuffer
   U32 start = 0;
   U32 extractSize = 0 ;

   if(xferFile && dataSize)
      {      
      // could have used start but it's a bit confusing because of the name
      if(bufferLen < dataSize)
         extractSize = bufferLen ;
      else
         extractSize = dataSize ;

      toFile(buffer, extractSize) ;      

      start = extractSize ;
      }
   else
      parseLine(buffer, &start, bufferLen);
   return start;
   }

void netFileServer::toFile(U8 *buffer, U32 bufferLen)
   {
   if(xferFile)
      xferFile->writeBinary(bufferLen, buffer) ;

   dataSize -= bufferLen ;

   if(dataSize == 0)
      onDownloadComplete();
   }

void netFileServer::onDownloadComplete()
   {
   if (fs)
      {
      fs->close();
      fs->deleteObject();
      }
   if (currentlyUploadingFiles.size()>0)
      {
      String str = netFileCommands::get + String (":") +  currentlyUploadingFiles.first() + String("\n");
      currentlyUploadingFiles.erase(currentlyUploadingFiles.begin());
      send((const U8*)str.c_str(), dStrlen(str.c_str()));
      }
   }

void netFileServer::setXferFile(FileObject *pFile, U32 nDataSize)
   { 
   xferFile = pFile ; 
   dataSize = nDataSize ;
   }

void netFileServer::onConnectionRequest(const NetAddress *addr, NetSocket connectId)
   {
	netFileServer* newconn = new netFileServer();
	newconn->registerObject();
	newconn->addToTable(connectId);
   }

bool netFileServer::processLine(UTF8 *line)
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
   if (cmd.equal(netFileCommands::list,1))
      GetFileList();
   else if (cmd.equal(netFileCommands::get,1))
      GetFile(param);
   else if (cmd.equal(netFileCommands::finished,1))
      {
      if (currentlyUploadingFiles.size()>0)
         {
         String str = netFileCommands::get + String (":") +  currentlyUploadingFiles.first() + String("\n");
         currentlyUploadingFiles.erase(currentlyUploadingFiles.begin());
         send((const U8*)str.c_str(), dStrlen(str.c_str()));
         }
      }
   else if (cmd.equal(netFileCommands::writefile,1))
      {
         if (!prepareWrite(param.c_str(),dAtoui(param2.c_str())))
         {
         dropRest=true;
         String errmsg = String("Could not update local file '") + param + String("', It is ReadOnly.");
         disconnect();
         return false;
         }
      }
   else if (cmd.equal(netFileCommands::requestsubmit))
      VerifyClientSubmit(param, param2);

   return true;
   }

void netFileServer::GetFile(String file)
   {
   FileObject* fs = new FileObject();
   if (!fs->readMemory(file.c_str()))
      return;
   char idBuf[16];
   dSprintf(idBuf, sizeof(idBuf), "%u", fs->getSize());
   String str = netFileCommands::writefile + String(":") + file + String(":")+ String(idBuf) + String("\n");
   send((const U8*)str.c_str(), dStrlen(str.c_str()));
   send((const U8 *) fs->getBuffer(), fs->getSize()) ;
   }

void netFileServer::GetFileList()
   {
   Sim::postEvent(this,new FileList_SysEvent(0,FilesSize()),Sim::getCurrentTime() + 25);
   }

bool netFileServer::prepareWrite(const char* filename,U32 size)
   {
   if (netFileUtils::isWriteable(filename))
      {
      fs = new FileObject();
      fs->openForWrite(filename, false);
      setXferFile(fs, size);
      return true;
      }
   return false;
   }

void netFileServer::VerifyClientSubmit(String fileName, String crc)
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
         String str = netFileCommands::denyWrite + String(":") + fileName + String("\n");
         send((const U8*)str.c_str(), dStrlen(str.c_str()));
         return;
         }
      }

   //See if the file is currently being written too
   for(int i = 0; i < currentlyUploadingFiles.size(); ++i)
      {
      if(currentlyUploadingFiles[i] == fileName)
         {
         //Send Deny Message and exit
         String str = netFileCommands::denyWrite + String(":") + fileName + String("\n");
         send((const U8*)str.c_str(), dStrlen(str.c_str()));
         return;
         }
      }

   //Add the item to the list of files being written to
   this->currentlyUploadingFiles.push_back(fileName);

   //Not found, accept write
   String str = netFileCommands::acceptWrite + String(":") + fileName + String("\n");
   send((const U8*)str.c_str(), dStrlen(str.c_str()));
   }

void netFileServer::send(const U8 *buffer, U32 len)
   {
   if (mState == Connected)
      {
      Net::Error err = Net::sendtoSocket(mTag, buffer, S32(len));
      while (err == Net::Error::WouldBlock)
         err = Net::sendtoSocket(mTag, buffer, S32(len));
      }
   else
      disconnect();
   }

void netFileServer::SendChatToClient(const char* msg)
   {
   String str = netFileCommands::send + String (":") + String(msg) + String("\n");
   send((const U8*)str.c_str(), dStrlen(str.c_str()));
   }

void netFileServer::onDisconnect()
{
   Parent::onDisconnect();
   this->destroySelf();
}

#endif