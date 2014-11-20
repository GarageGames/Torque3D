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

#include "NetFTPClient.h"
#include "platform/platform.h"
#include "console/simBase.h"
#include "console/consoleInternal.h"
#include "core/strings/stringUnit.h"
#include "console/engineAPI.h"
#include "core/fileObject.h"
#include "T3D/gameBase/gameBase.h"
#include "platform/platformNet.h"
#include "console/simObject.h"
#include "platform\platformNet.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiControl.h"
#include "gui/controls/guiTextCtrl.h"
#include "gui/game/guiProgressCtrl.h"



static NetFTPClient* ftpclient = NULL;
NetFTPClient *NetFTPClient::table[NetFTPClient::TableSize] = {0, };

IMPLEMENT_CONOBJECT(NetFTPClient);

NetFTPClient *NetFTPClient::find(NetSocket tag)
   {
   for(NetFTPClient *walk = table[U32(tag) & TableMask]; walk; walk = walk->mNext)
      if(walk->mTag == tag)
         return walk;
   return NULL;
   }

void NetFTPClient::addToTable(NetSocket newTag)
   {
   removeFromTable();
   mTag = newTag;
   mNext = table[U32(mTag) & TableMask];
   table[U32(mTag) & TableMask] = this;
   }

void NetFTPClient::removeFromTable()
   {
   for(NetFTPClient **walk = &table[U32(mTag) & TableMask]; *walk; walk = &((*walk)->mNext))
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

S32 gNetFTPClientCount = 0;

NetFTPClient::NetFTPClient()
   {
   mDropRest=false;
   mDialogPushed=false;
   mBuffer = NULL;
   mBufferSize = 0;
   mPort = 0;
   mTag = InvalidSocket;
   mState = Disconnected;

   gNetFTPClientCount++;

   if(gNetFTPClientCount == 1)
      {
      Net::smConnectionAccept.notify(processConnectedAcceptEvent);
      Net::smConnectionReceive.notify(processConnectedReceiveEvent);
      Net::smConnectionNotify.notify(processConnectedNotifyEvent);
      }
   pDumpFile = NULL ;
   mDataSize = 0 ;
   mOrigSize = 0;
   fs=0;
   }

NetFTPClient::~NetFTPClient()
   {
   disconnect();
   dFree(mBuffer);

   gNetFTPClientCount--;

   if(gNetFTPClientCount == 0)
      {
      Net::smConnectionAccept.remove(processConnectedAcceptEvent);
      Net::smConnectionReceive.remove(processConnectedReceiveEvent);
      Net::smConnectionNotify.remove(processConnectedNotifyEvent);
      }
   }

bool NetFTPClient::processArguments(S32 argc, const char **argv)
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

bool NetFTPClient::onAdd()
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

U32 NetFTPClient::onReceive(U8 *buffer, U32 bufferLen)
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

void NetFTPClient::dumpToFile(U8 *buffer, U32 bufferLen)
   {   
   if(pDumpFile)
      pDumpFile->writeBinary(bufferLen, buffer) ;

   mDataSize -= bufferLen ;

   if(mDataSize == 0)
      onDownloadComplete();
   else
      dynamic_cast<GuiProgressCtrl*>(mProgressBar)->SetScriptValue(1.00 - ((F32)mDataSize)/((F32)mOrigSize));
   }

void NetFTPClient::setDumpFile(FileObject *pFile, U32 nDataSize)
   { 
   pDumpFile = pFile ; 
   mOrigSize = nDataSize ;
   mDataSize = nDataSize ;
   }

void NetFTPClient::parseLine(U8 *buffer, U32 *start, U32 bufferLen)
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

void NetFTPClient::onConnectionRequest(const NetAddress *addr, U32 connectId)
   {
   char idBuf[16];
   char addrBuf[256];
   Net::addressToString(addr, addrBuf);
   dSprintf(idBuf, sizeof(idBuf), "%d", connectId);
   }

bool NetFTPClient::processLine(UTF8 *line)
   {
   if (mDropRest)
      return false;

   String cmd = String("");
   String param1 = String("");
   String param2 = String("");

   String sline = String(line);
   S32 p=0;
   for (int i = 0;i<sline.length();i++)
      {
      if (sline.substr(i,1)==String(":"))
         p++;
      else 
         {
         if (p==0)
            cmd +=sline[i];
         else if (p==1)
            param1 +=sline[i];
         else if (p==2)
            param2 += sline[i];
         }
      }
   if (cmd.equal("requestsubmit",1))
      {
      if (mMsg)
         dynamic_cast<GuiTextCtrl*>(mMsg)->setText((String("Queueing File '") + param1 + String ("' for download.")).c_str());

      if(Torque::FS::IsFile(param1))
         {
         //Check to see if they are the same
         Torque::FS::FileNodeRef fileRef = Torque::FS::GetFileNode( param1 );

         U32 temp = dAtoi(param2.c_str());

         if(fileRef->getChecksum() != temp)
            {
            //push file to mfilestograb
            mFilesToGrab.push(param1);
            }
         }
      else
         mFilesToGrab.push(param1);
      }
   else if (cmd.equal("finished",1)) 
      {
      if (mFilesToGrab.size()>0)
         {
         String str = String ("get:") +  mFilesToGrab.top() + String("\n");
         mFilesToGrab.pop();
         send((const U8*)str.c_str(), dStrlen(str.c_str()));
         }
      else
         {
         onDownloadComplete();
         }
      }
   else if (cmd.equal("writefile",1))
      {
      if (mMsg)
         dynamic_cast<GuiTextCtrl*>(mMsg)->setText((String("Downloading File '") + param1 + String ("' (") + param2 + String(" kb).")).c_str());

      if (!_ftpPrepareWrite(param1,param2))
         {
         mDropRest=true;
         popdialog();
         String errmsg = String("Could not update local file '") + param1 + String("', It is ReadOnly.");
         mGameConnection->onConnectionRejected(errmsg.c_str());
         disconnect();
         return false;
         }
      }
   else if(cmd.equal("acceptWrite", 1))
      {
      //Send the file
      SubmitFile(mFileToSend);
      }

   return true;
   }

static char sgScriptFilenameBuffer[1024];
bool NetFTPClient::isWriteable(const char* fileName)
   {
   String filename(Torque::Path::CleanSeparators(fileName));
   Con::expandScriptFilename(sgScriptFilenameBuffer, sizeof(sgScriptFilenameBuffer), filename.c_str());

   Torque::Path givenPath(Torque::Path::CompressPath(sgScriptFilenameBuffer));
   Torque::FS::FileSystemRef fs = Torque::FS::GetFileSystem(givenPath);
   Torque::Path path = fs->mapTo(givenPath);

   return !Torque::FS::IsReadOnly(path);
   }

static String LastFile;

bool NetFTPClient::_ftpPrepareWrite(String param1,String param2)
   {
   if (isWriteable(param1))
      {
      fs = new FileObject();
      LastFile =String( param1);
      fs->openForWrite(param1.c_str(),false);
      setDumpFile(fs,dAtoui(param2.c_str()));
      return true;
      }
   return false;
   }


void NetFTPClient::onDNSResolved()
   {
   mState = DNSResolved;
   }

void NetFTPClient::onDNSFailed()
   {
   mState = Disconnected;
   mGameConnection->onConnectionRejected("Could not resolve Game server.");
   }

void NetFTPClient::pushDialog()
   {
   if (!(mCanvas && mDlg))
      return;
   dynamic_cast<GuiCanvas* >(mCanvas)->pushDialogControl(dynamic_cast<GuiControl*>(mDlg),0,true);
   dynamic_cast<GuiProgressCtrl*>(mProgressBar)->SetScriptValue(0);
   mDialogPushed=true;
   }

void NetFTPClient::popdialog()
   {
   if (mCanvas && mDlg && mDialogPushed)
      dynamic_cast<GuiCanvas* >( mCanvas)->popDialogControl(dynamic_cast<GuiControl*>(mDlg));
   }

void NetFTPClient::onConnected()
   {
   Con::printf("Connected!");
   mState = Connected;
   String str = String ("list\n");
   send((const U8*)str.c_str(), dStrlen(str.c_str()));
   mCanvas =Sim::findObject("Canvas");
   mDlg = Sim::findObject("FTPProgressGui");
   mMsg = Sim::findObject("Progress_Message");
   mProgressBar = Sim::findObject("Progress_FTPClientProgress");
   pushDialog();
}

void NetFTPClient::onConnectFailed()
   {
   popdialog();
   mState = Disconnected;
   mGameConnection->onConnectionRejected("Could not connect to OneWorld File Server.");
   }

void NetFTPClient::finishLastLine()
   {
   if (!mBufferSize)
      return;
   mBuffer[mBufferSize] = 0;
   processLine((UTF8*)mBuffer);
   dFree(mBuffer);
   mBuffer = 0;
   mBufferSize = 0;
   }

void NetFTPClient::onDisconnect()
   {
   popdialog();
   finishLastLine();
   mState = Disconnected;

   pDumpFile = NULL ; 
   mOrigSize = 0 ;
   mDataSize = 0 ;
   if (!fs)
      return;
   fs->close();
   fs->deleteObject();
   }




class ConnectToParent_SysEvent : public SimEvent
   {
   void process(SimObject* obj)
      {
        NetFTPClient* client = dynamic_cast<NetFTPClient*>(obj);
      if (client)
         {
         client->mGameConnection->ParentConnect(client->mRemoteAddress);
         }
      }
   };


void NetFTPClient::onDownloadComplete()
   {
#if defined(TORQUE_DEBUG)
   Con::printf("!!!!!!!!!DOWNLOAD COMPLETE!!!!!!!!!");
#endif
   if (fs)
      {
      fs->close();
      fs->deleteObject();
      if (LastFile.endsWith(".cs"))
         {
         String tmp = String("exec (\"") + String(LastFile.c_str()) + String("\");");
         Con::evaluate(tmp.c_str());
         }
      }
   if (mFilesToGrab.size()>0)
      {
      String str = String ("get:") +  mFilesToGrab.top() + String("\n");
      mFilesToGrab.pop();
      send((const U8*)str.c_str(), dStrlen(str.c_str()));
      }
   else
      {
      popdialog();
      Sim::postEvent(this,new ConnectToParent_SysEvent(),Sim::getTargetTime() + 20);
      this->disconnect();
      }
   }

void NetFTPClient::listen(U16 port)
   {
   mState = Listening;
   U32 newTag = Net::openListenPort(port);
   addToTable(newTag);
   }

void NetFTPClient::connect(const char *address)
   {
   NetSocket newTag = Net::openConnectTo(address);
   addToTable(newTag);
   }

void NetFTPClient::disconnect()
   {
   if( mTag != InvalidSocket ) 
      Net::closeConnectTo(mTag);
   removeFromTable();
   }

void NetFTPClient::send(const U8 *buffer, U32 len)
   {
   Net::Error err = Net::sendtoSocket(mTag, buffer, S32(len));
   while (err == Net::Error::WouldBlock)
	{
      err = Net::sendtoSocket(mTag, buffer, S32(len));
	}
   }

void NetFTPClient::processConnectedReceiveEvent(NetSocket sock, RawData incomingData)
   {
   NetFTPClient *tcpo = NetFTPClient::find(sock);
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

void NetFTPClient::processConnectedAcceptEvent(NetSocket listeningPort, NetSocket newConnection, NetAddress originatingAddress)
   {
   NetFTPClient *tcpo = NetFTPClient::find(listeningPort);
   if(!tcpo)
      return;

   tcpo->onConnectionRequest(&originatingAddress, newConnection);
   }

void NetFTPClient::processConnectedNotifyEvent( NetSocket sock, U32 state )
   {
   NetFTPClient *tcpo = NetFTPClient::find(sock);
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

void NetFTPClient::ConnectAndDownload(const char* remoteAddress,GameConnection* gc)
   {
	String ra = String(remoteAddress);
	String name = ra.substr(0,ra.find(":", 3 ));
	String portstring = ra.substr(ra.find(":", 3 )+1);
	S32 port = dAtoi(portstring.c_str());
	port +=1;
	String newAddress = name + ":" + String(Con::getIntArg(port));
	remoteAddress = StringTable->insert(newAddress.c_str());
   _CreateGui();

   ftpclient = new NetFTPClient();

   ftpclient->mGameConnection = gc;
   ftpclient->mRemoteAddress = remoteAddress;
   ftpclient->registerObject();

   SimSet* miscu = dynamic_cast<SimSet *>(Sim::findObject("MissionCleanup"));
   if (miscu)
      miscu->addObject(ftpclient);
   ftpclient->connect(remoteAddress);
   }

void NetFTPClient::_CreateGui()
   {
   if (!Sim::findObject("FTPProgressGui"))
      {
      String gui =  String("");
      gui = gui + String("%guiContent = new GuiControl(FTPProgressGui) {");
      gui = gui + String("   position = \"0 0\";");
      gui = gui + String("   extent = \"1024 768\";");
      gui = gui + String("   minExtent = \"8 2\";");
      gui = gui + String("   horizSizing = \"right\";");
      gui = gui + String("   vertSizing = \"bottom\";");
      gui = gui + String("   profile = \"GuiDefaultProfile\";");
      gui = gui + String("   visible = \"1\";");
      gui = gui + String("   active = \"1\";");
      gui = gui + String("   tooltipProfile = \"GuiToolTipProfile\";");
      gui = gui + String("   hovertime = \"1000\";");
      gui = gui + String("   isContainer = \"1\";");
      gui = gui + String("   canSave = \"1\";");
      gui = gui + String("   canSaveDynamicFields = \"1\";");
      gui = gui + String("      guidesHorizontal = \"298\";");
      gui = gui + String("   new GuiPanel(SimSet_2691) {");
      gui = gui + String("      docking = \"None\";");
      gui = gui + String("      margin = \"0 0 0 0\";");
      gui = gui + String("      padding = \"0 0 0 0\";");
      gui = gui + String("      anchorTop = \"1\";");
      gui = gui + String("      anchorBottom = \"0\";");
      gui = gui + String("      anchorLeft = \"1\";");
      gui = gui + String("      anchorRight = \"0\";");
      gui = gui + String("      position = \"249 247\";");
      gui = gui + String("      extent = \"400 129\";");
      gui = gui + String("      minExtent = \"16 16\";");
      gui = gui + String("      horizSizing = \"right\";");
      gui = gui + String("      vertSizing = \"bottom\";");
      gui = gui + String("      profile = \"GuiDefaultProfile\";");
      gui = gui + String("      visible = \"1\";");
      gui = gui + String("      active = \"1\";");
      gui = gui + String("      tooltipProfile = \"GuiToolTipProfile\";");
      gui = gui + String("      hovertime = \"1000\";");
      gui = gui + String("      isContainer = \"1\";");
      gui = gui + String("      canSave = \"1\";");
      gui = gui + String("      canSaveDynamicFields = \"0\";");
      gui = gui + String("");
      gui = gui + String("      new GuiProgressCtrl(Progress_FTPClientProgress) {");
      gui = gui + String("         maxLength = \"1024\";");
      gui = gui + String("         margin = \"0 0 0 0\";");
      gui = gui + String("         padding = \"0 0 0 0\";");
      gui = gui + String("         anchorTop = \"1\";");
      gui = gui + String("         anchorBottom = \"0\";");
      gui = gui + String("         anchorLeft = \"1\";");
      gui = gui + String("         anchorRight = \"0\";");
      gui = gui + String("         position = \"5 89\";");
      gui = gui + String("         extent = \"390 22\";");
      gui = gui + String("         minExtent = \"8 2\";");
      gui = gui + String("         horizSizing = \"right\";");
      gui = gui + String("         vertSizing = \"bottom\";");
      gui = gui + String("         profile = \"GuiProgressProfile\";");
      gui = gui + String("         visible = \"1\";");
      gui = gui + String("         active = \"1\";");
      gui = gui + String("         tooltipProfile = \"GuiToolTipProfile\";");
      gui = gui + String("         hovertime = \"1000\";");
      gui = gui + String("         isContainer = \"1\";");
      gui = gui + String("         canSave = \"1\";");
      gui = gui + String("         canSaveDynamicFields = \"0\";");
      gui = gui + String("      };");
      gui = gui + String("      new GuiTextCtrl(Progress_Message) {");
      gui = gui + String("         text = \"Status:\";");
      gui = gui + String("         maxLength = \"1024\";");
      gui = gui + String("         margin = \"0 0 0 0\";");
      gui = gui + String("         padding = \"0 0 0 0\";");
      gui = gui + String("         anchorTop = \"1\";");
      gui = gui + String("         anchorBottom = \"0\";");
      gui = gui + String("         anchorLeft = \"1\";");
      gui = gui + String("         anchorRight = \"0\";");
      gui = gui + String("         position = \"5 36\";");
      gui = gui + String("         extent = \"390 43\";");
      gui = gui + String("         minExtent = \"8 2\";");
      gui = gui + String("         horizSizing = \"right\";");
      gui = gui + String("         vertSizing = \"bottom\";");
      gui = gui + String("         profile = \"GuiTextProfile\";");
      gui = gui + String("         visible = \"1\";");
      gui = gui + String("         active = \"1\";");
      gui = gui + String("         tooltipProfile = \"GuiToolTipProfile\";");
      gui = gui + String("         hovertime = \"1000\";");
      gui = gui + String("         isContainer = \"1\";");
      gui = gui + String("         canSave = \"1\";");
      gui = gui + String("         canSaveDynamicFields = \"0\";");
      gui = gui + String("      };");
      gui = gui + String("      new GuiTextCtrl(SimSet_3690) {");
      gui = gui + String("         text = \"FTP CLient\";");
      gui = gui + String("         maxLength = \"1024\";");
      gui = gui + String("         margin = \"0 0 0 0\";");
      gui = gui + String("         padding = \"0 0 0 0\";");
      gui = gui + String("         anchorTop = \"1\";");
      gui = gui + String("         anchorBottom = \"0\";");
      gui = gui + String("         anchorLeft = \"1\";");
      gui = gui + String("         anchorRight = \"0\";");
      gui = gui + String("         position = \"170 8\";");
      gui = gui + String("         extent = \"60 11\";");
      gui = gui + String("         minExtent = \"8 2\";");
      gui = gui + String("         horizSizing = \"right\";");
      gui = gui + String("         vertSizing = \"bottom\";");
      gui = gui + String("         profile = \"GuiTextProfile\";");
      gui = gui + String("         visible = \"1\";");
      gui = gui + String("         active = \"1\";");
      gui = gui + String("         tooltipProfile = \"GuiToolTipProfile\";");
      gui = gui + String("         hovertime = \"1000\";");
      gui = gui + String("         isContainer = \"1\";");
      gui = gui + String("         canSave = \"1\";");
      gui = gui + String("         canSaveDynamicFields = \"0\";");
      gui = gui + String("      };");
      gui = gui + String("   };");
      gui = gui + String("};");
      Con::evaluate(gui.c_str());
      }
   }

void NetFTPClient::SubmitFile(String file)
   {
   FileObject* fs = new FileObject();
   if (!(fs->readMemory(file.c_str())))
      return;
   char idBuf[16];
   dSprintf(idBuf, sizeof(idBuf), "%u", fs->getSize());
   String str = String("writefile:") + file + String(":")+ String(idBuf) + String("\n");
   send((const U8*)str.c_str(), dStrlen(str.c_str()));
   send((const U8 *) fs->getBuffer(), fs->getSize()) ;
   }

void NetFTPClient::RequestSubmitFile(String name)
   {
   //Error  checking
   if (!(Torque::FS::IsFile(name)))
      return;
   
   //Check to see if they are the same
   Torque::FS::FileNodeRef fileRef = Torque::FS::GetFileNode( name );
   U32 crc = (fileRef->getChecksum());

   //Save the title of the file to be sent
   mFileToSend = name;

   String str = String("requestsubmit:") + name + String(":" ) +String( Con::getIntArg(crc)) + String("\n");
   send((const U8*)str.c_str(), dStrlen(str.c_str()));

   }

//User access via the console
DefineConsoleFunction(SubmitFile, void, (String fileName),,
   "Test call for users to submit files")
{
   ftpclient->RequestSubmitFile(fileName);
}

#endif