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
#include "netFileClient.h"
#include "netFileUtils.h"
#include "gui/core/guiControl.h"
#include "gui/controls/guiTextCtrl.h"
#include "gui/game/guiProgressCtrl.h"
#include "gui/core/guiCanvas.h"
#include "console/engineAPI.h"
#include "console/fileSystemFunctions.h"

IMPLEMENT_CONOBJECT(netFileClient);

IMPLEMENT_CALLBACK(netFileClient, onProgress, void, ( F32 percent), ( percent ),
   "@brief Called whenever progress is made in communications between the client and server.\n\n"
   );

IMPLEMENT_CALLBACK(netFileClient, onFileTransferMessage, void, ( const char* msg ), ( msg ),
   "@brief Called whenever a event message occurs.\n\n"
   );

IMPLEMENT_CALLBACK(netFileClient, onFileTransferComplete, void, ( ), ( ),
   "@brief Called when all file transfers have completed.\n\n"
   );

IMPLEMENT_CALLBACK(netFileClient, onFileTransferError, void, ( const char* msg ), ( msg ),
   "@brief Called when an error has occured in the file transfer.\n\n"
   );



static String LastFile;

netFileClient::netFileClient()
{
   dropRest = false;
   xferFile = NULL;
   expectedDataSize = 0;
   totalDataSize = 0;
}

U32 netFileClient::onReceive(U8 *buffer, U32 bufferLen)
{
   U32 start = 0;
   U32 extractSize = 0;
   if(xferFile && expectedDataSize)
   {      
      if(bufferLen < expectedDataSize)
         extractSize = bufferLen;
      else
         extractSize = expectedDataSize;
      toFile(buffer, extractSize);  
      start = extractSize;
   }
   else
      parseLine(buffer, &start, bufferLen);
   return start;
}

void netFileClient::toFile(U8 *buffer, U32 bufferLen)
{   
   if(xferFile)
      xferFile->writeBinary(bufferLen, buffer);
   expectedDataSize -= bufferLen;
   //If the datasize is 0 then we are done the download.
   if(expectedDataSize == 0)
   {
      xferFile->close();
      xferFile->deleteObject();
      onDownloadComplete();
   }
   else
      onProgress_callback( 1.00f - ( (F32) expectedDataSize ) / ( ( F32 ) totalDataSize ) );
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

   if ( cmd.equal( netFileCommands::progress,1))
   {
      F32 val = (float)atoi(param1.c_str()) / 100.00f;
      onProgress_callback( val );
   }

   else if ( cmd.equal( netFileCommands::requestsubmit, 1 ) )
   {
      if ( Torque::FS::IsFile( param1 ) )
      {
         //Check to see if they are the same
         Torque::FS::FileNodeRef fileRef = Torque::FS::GetFileNode( param1 );
         U32 temp = dAtoui(param2.c_str());
         U32 chksum = fileRef->getChecksum();
         if(chksum != temp)
            filesToDownload.push(param1);
      }
      else
         filesToDownload.push(param1);
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
      onFileTransferMessage_callback(StringTable->insert((String("Downloading File '") + param1 + String ("' (") + param2 + String(" kb).")).c_str()));

      if ( !prepareClientWrite( param1.c_str(), dAtoui(param2.c_str() ) ) )
      {
         dropRest=true;
         onFileTransferError_callback((String("Could not update local file '") + param1 + String("', It is ReadOnly.")).c_str());
         return false;
      }
   }
   else if ( cmd.equal( netFileCommands::acceptWrite, 1 ) )
      SubmitFile(fileToSend);

   return true;
}

bool netFileClient::prepareClientWrite(const char* filename,U32 size)
{
   if ( Torque::FileSystem::isWriteableFileName( filename ) )
   {
      xferFile = new FileObject();
      xferFile->openForWrite(filename, false);
      totalDataSize = size;
      expectedDataSize = size;
      return true;
   }
   return false;
}

void netFileClient::SubmitFile(String file)
{
   FileObject* fs = new FileObject();
   if (!(fs->readMemory(file.c_str())))
      return;
   char idBuf[16];
   dSprintf(idBuf, sizeof(idBuf), "%u", fs->getSize());
   String str = netFileCommands::writefile + String(":") + file + String(":")+ String(idBuf) + String("\n");
   send((const U8*)str.c_str(), dStrlen(str.c_str()));
   send((const U8 *) fs->getBuffer(), fs->getSize());
}

void netFileClient::onDisconnect()
{
   Parent::onDisconnect();
   xferFile = NULL; 
   totalDataSize = 0;
   expectedDataSize = 0;
   if (!xferFile)
      return;
   xferFile->close();
   xferFile->deleteObject();
}

void netFileClient::send(const U8 *buffer, U32 len)
{
   Net::Error err = Net::sendtoSocket(mTag, buffer, S32(len));
   while (err == 3) //WouldBlock
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
      onFileTransferComplete_callback();
   }
}

void netFileClient::onDNSFailed()
{
   Parent::onDNSFailed();
}

void netFileClient::onConnected()
{
   Parent::onConnected();
   onFileTransferMessage_callback(StringTable->insert("Checking server for newer files."));
   send((const U8*)netFileCommands::listn.c_str(), dStrlen(netFileCommands::listn.c_str()));
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
   String str =netFileCommands::requestsubmit + String(":") + name + String(":" ) +String( netFileUtils::uinttochar(crc)) + String("\n");
   send((const U8*)str.c_str(), dStrlen(str.c_str()));
}

DefineConsoleMethod( netFileClient, SendFileToServer, void, ( const char* filename ),,
   "Loads a directory and pattern into the download queue.\n"
   "@pattern - Path and pattern to search for.\n"
   "@verbose - will echo to the console each file it adds.\n"
   )
{
   object->RequestSubmitFile(filename);
}