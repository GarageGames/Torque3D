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

#include "netFileClient.h"
#include "netFileUtils.h"
#include "gui/core/guiControl.h"
#include "gui/controls/guiTextCtrl.h"
#include "gui/game/guiProgressCtrl.h"
#include "gui/core/guiCanvas.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(netFileClient);

static String LastFile;

static netFileClient* ftpclient = NULL;

class ConnectToParent_SysEvent : public SimEvent
{
   void process(SimObject* obj)
   {
      netFileClient* client = dynamic_cast<netFileClient*>(obj);
      if (client)
         client->getGameConnection()->ParentConnect(client->getRemoteAddress());
   }
};

netFileClient::netFileClient()
{
   dropRest = false;
   isDownloadDialogPushed = false;
   mBuffer = NULL;
   mBufferSize = 0;
   xferFile = NULL ;
   dataSize = 0 ;
   totalDataSize = 0;
   fs = 0;
}

U32 netFileClient::onReceive(U8 *buffer, U32 bufferLen)
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

void netFileClient::toFile(U8 *buffer, U32 bufferLen)
{   
   if(xferFile)
      xferFile->writeBinary(bufferLen, buffer) ;
   dataSize -= bufferLen ;
   if(dataSize == 0)
      onDownloadComplete();
   else
      dynamic_cast<GuiProgressCtrl*>(pbDownloadProgress)->SetScriptValue(1.00f - ((F32)dataSize)/((F32)totalDataSize));
}

void netFileClient::setXferFile(FileObject *pFile, U32 nDataSize)
{ 
   xferFile = pFile ; 
   totalDataSize = nDataSize ;
   dataSize = nDataSize ;
}

bool netFileClient::processLine(UTF8 *line)
{
   if (dropRest)
      return false;

   String cmd = String("");
   String param1 = String("");
   String param2 = String("");

   String sline = String(line);
   S32 p = 0;
   for (int i = 0; i < sline.length(); i++)
   {
      if (sline.substr(i,1) == String(":"))
         p++;
      else 
      {
         if (p==0)
            cmd += sline[i];
         else if ( p == 1 )
            param1 += sline[i];
         else if ( p==2 )
            param2 += sline[i];
      }
   }
   if ( cmd.equal( netFileCommands::requestsubmit, 1 ) )
   {
      if ( Torque::FS::IsFile( param1 ) )
      {
         //Check to see if they are the same
         Torque::FS::FileNodeRef fileRef = Torque::FS::GetFileNode( param1 );
         U32 temp = dAtoui(param2.c_str());
			U32 chksum = fileRef->getChecksum();
         if(chksum != temp)
         {
            filesToDownload.push(param1);
            if (gtcDisplayMessage)
               dynamic_cast<GuiTextCtrl*>(gtcDisplayMessage)->setText((String("Queueing File '") + param1 + String ("' for download.")).c_str());
         }
      }
      else
      {
         filesToDownload.push(param1);
         if ( gtcDisplayMessage )
            dynamic_cast<GuiTextCtrl*>(gtcDisplayMessage)->setText((String("Queueing File '") + param1 + String ("' for download.")).c_str());
      }
   }
   else if ( cmd.equal( netFileCommands::finished, 1 ) ) 
   {
      if ( filesToDownload.size() > 0 )
      {
         String str = String (netFileCommands::get + ":") +  filesToDownload.top() + String("\n");
         filesToDownload.pop();
         send((const U8*)str.c_str(), dStrlen(str.c_str()));
      }
      else
         onDownloadComplete();
   }
   else if ( cmd.equal( netFileCommands::writefile, 1 ) )
   {
      if ( gtcDisplayMessage )
         dynamic_cast<GuiTextCtrl*>(gtcDisplayMessage)->setText((String("Downloading File '") + param1 + String ("' (") + param2 + String(" kb).")).c_str());

      if ( !prepareWrite( param1.c_str(), dAtoui(param2.c_str() ) ) )
      {
         dropRest=true;
         popdialog();
         String errmsg = String("Could not update local file '") + param1 + String("', It is ReadOnly.");
         mGameConnection->onConnectionRejected(errmsg.c_str());
         disconnect();
         return false;
      }
   }
   else if ( cmd.equal( netFileCommands::acceptWrite, 1 ) )
      SubmitFile(fileToSend);

   return true;
}

bool netFileClient::prepareWrite(const char* filename,U32 size)
{
   if ( netFileUtils::isWriteable( filename ) )
   {
      fs = new FileObject();
      fs->openForWrite(filename, false);
      setXferFile(fs, size);
      return true;
   }
   return false;
}

void netFileClient::pushDialog()
{
   if ( ! ( canvas && gcDownloadDisplayDlg ) )
      return;
   dynamic_cast<GuiCanvas* >(canvas)->pushDialogControl(dynamic_cast<GuiControl*>(gcDownloadDisplayDlg),0,true);
   dynamic_cast<GuiProgressCtrl*>(pbDownloadProgress)->SetScriptValue(0);
   isDownloadDialogPushed = true;
}

void netFileClient::popdialog()
{
   if (canvas && gcDownloadDisplayDlg && isDownloadDialogPushed)
      dynamic_cast<GuiCanvas* >( canvas)->popDialogControl(dynamic_cast<GuiControl*>(gcDownloadDisplayDlg));
}

void netFileClient::SubmitFile(String file)
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

void netFileClient::onDisconnect()
{
   popdialog();
   Parent::onDisconnect();
   xferFile = NULL ; 
   totalDataSize = 0 ;
   dataSize = 0 ;
   if (!fs)
      return;
   fs->close();
   fs->deleteObject();
}

void netFileClient::send(const U8 *buffer, U32 len)
{
   Net::Error err = Net::sendtoSocket(mTag, buffer, S32(len));
   while (err == Net::Error::WouldBlock)
      err = Net::sendtoSocket(mTag, buffer, S32(len));
}

void netFileClient::onDownloadComplete()
{
   if (filesToDownload.size()>0)
   {
      String str = String (netFileCommands::get + ":") +  filesToDownload.top() + String("\n");
      filesToDownload.pop();
      send((const U8*)str.c_str(), dStrlen(str.c_str()));
   }
   else
   {
      popdialog();
      Sim::postEvent(this,new ConnectToParent_SysEvent(),Sim::getTargetTime() + 20);
      this->disconnect();
   }
}

void netFileClient::onDNSFailed()
{
   Parent::onDNSFailed();
   mGameConnection->onConnectionRejected("Could not resolve Game server.");
}

void netFileClient::onConnected()
{
   Parent::onConnected();
   send((const U8*)netFileCommands::list.c_str(), dStrlen(netFileCommands::list.c_str()));
   canvas = Sim::findObject("Canvas");
   gcDownloadDisplayDlg = Sim::findObject("FTPProgressGui");
   gtcDisplayMessage = Sim::findObject("Progress_Message");
   pbDownloadProgress = Sim::findObject("Progress_FTPClientProgress");
   pushDialog();
}

void netFileClient::onConnectFailed()
{
   popdialog();
   Parent::onConnectFailed();
   mGameConnection->onConnectionRejected("Could not connect to File Server.");
}

void netFileClient::ConnectAndDownload(const char* remoteAddress, GameConnection* gc)
{
   _CreateGui();
   ftpclient = new netFileClient();
   ftpclient->mGameConnection = gc;
   ftpclient->remoteAddress = remoteAddress;
   ftpclient->registerObject();
   SimSet* miscu = dynamic_cast<SimSet *>(Sim::findObject("MissionCleanup"));
   if (miscu)
      miscu->addObject(ftpclient);
   ftpclient->connect(remoteAddress);
}

void netFileClient::_CreateGui()
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
      gui = gui + String("         text = \"File System Client\";");
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
   
void netFileClient::RequestSubmitFile(String name)
{
   //Error  checking
   if (!(Torque::FS::IsFile(name)))
      return;
   //Check to see if they are the same
   Torque::FS::FileNodeRef fileRef = Torque::FS::GetFileNode( name );
   U32 crc = (fileRef->getChecksum());
   //Save the title of the file to be sent
   fileToSend = name;
   String str =netFileCommands::requestsubmit + String(":") + name + String(":" ) +String( Con::getIntArg(crc)) + String("\n");
   send((const U8*)str.c_str(), dStrlen(str.c_str()));
}

//User access via the console
DefineConsoleFunction(netFileClient, void, (String fileName),,
   "Test call for users to submit files")
{
   ftpclient->RequestSubmitFile(fileName);
}

#endif