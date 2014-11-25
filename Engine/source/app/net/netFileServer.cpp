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

#include "netFileServer.h"
#include "netFileUtils.h"
#include "console/fileSystemFunctions.h"

IMPLEMENT_CONOBJECT(netFileServer);

ConsoleDocClass( netFileServer,"");

Vector<String> netFileServer::files;

IMPLEMENT_CALLBACK(netFileServer, onFileUploadRequest, bool, ( const char* filename), ( filename ),
   "@brief Called whenever a user wants to upload a file.\n\n"
   );

/*********************************************************************
This event class is used to break up the transmission of the file list
to each client.  This allows multiple clients to receive there file
list at the same time.
*********************************************************************/
class FileList_SysEvent : public SimEvent
{
public:
   S32 Idx;
   S32 fileCount;
   FileList_SysEvent(S32 idx,S32 filecount )
   {
      Idx = idx;
      fileCount = filecount;
   }
   void process(SimObject* obj)
   {
      netFileServer* mobj = dynamic_cast<netFileServer*>(obj);
      if (mobj)
      {
         if (Idx < fileCount)
         {
            int percent = ((float)Idx/ (float)fileCount * 100.00f);
            char * buff = Con::getReturnBuffer(10);
            dSprintf(buff,10,"%i",percent);
            
            //Notify the client of progress
            String str = netFileCommands::progress + String(":") + String(buff)  + String("\n");
            mobj->send((const U8*)str.c_str(), dStrlen(str.c_str()));

            //Get CRC for the file.
            String filename = mobj->FilesAt(Idx);
            Torque::FS::FileNodeRef fileRef = Torque::FS::GetFileNode( filename );
            U32 crc = (fileRef->getChecksum());
            
            //Send the details about the file
            str = netFileCommands::requestsubmit + String(":") + filename + String(":" ) +String( netFileUtils::uinttochar(crc)) + String("\n");
            mobj->send((const U8*)str.c_str(), dStrlen(str.c_str()));

            //Post the next iteration to the simevent queue to fire in 25 ms.
            Sim::postEvent(mobj,new FileList_SysEvent(Idx+1,fileCount),Sim::getCurrentTime() + 25);
         }
         else
         {
               //We are finished with the file list, notify the client.
               String str = netFileCommands::finished + String (":\n");
               mobj->send((const U8*)str.c_str(), dStrlen(str.c_str()));
         }
      }
   }
};

netFileServer::netFileServer()
{
   xferFile = NULL ;
   expectedDataSize = 0 ;
}

U32 netFileServer::onReceive(U8 *buffer, U32 bufferLen)
{
   U32 start = 0;
   U32 extractSize = 0 ;

   //This handles the switching of modes between a raw data mode
   //and a character mode.
   //If the xferFile is valid and the 
   if ( xferFile && expectedDataSize )
   {      
      if ( bufferLen < expectedDataSize )
         extractSize = bufferLen ;
      else
         extractSize = expectedDataSize ;
      toFile( buffer , extractSize );
      start = extractSize ;
   }
   else
      parseLine( buffer , &start , bufferLen );
   return start;
}

void netFileServer::toFile(U8 *buffer, U32 bufferLen)
{
   if( xferFile )
      xferFile->writeBinary( bufferLen , buffer ) ;
   expectedDataSize -= bufferLen ;
   if ( expectedDataSize == 0 )
   {
      xferFile->close();
      xferFile->deleteObject();
      onDownloadComplete();
   }
}

void netFileServer::onDownloadComplete()
{
   if (currentlyUploadingFiles.size()>0)
   {
      String str = netFileCommands::get + String (":") +  currentlyUploadingFiles.first() + String("\n");
      currentlyUploadingFiles.erase(currentlyUploadingFiles.begin());
      send((const U8*)str.c_str(), dStrlen(str.c_str()));
   }
}

void netFileServer::onConnectionRequest(const NetAddress *addr, NetSocket connectId)
{
      /*
      This can be somewhat confusing.  Every time a client connects to the server
      we create a new instance of the NetFileServer.  This instance handles the 
      communications for that socket.

      When you call the addToTable function, it updates the Tag on the new
      instance of NetFileServer to the socket # the client is talking on.
      */
	   netFileServer* newconn = new netFileServer();
	   newconn->registerObject();
	   newconn->addToTable(connectId);
      SimSet* miscu = dynamic_cast<SimSet *>(Sim::findObject("MissionCleanup"));
      if (miscu)
         miscu->addObject(newconn);
}

bool netFileServer::processLine(UTF8 *line)
{

   String sline = String(line);
   String cmd = String("");
   String param = String("");
   String param2 = String("");

   S32 p = 0;
   for ( int i = 0; i < sline.length() ; i++ )
   {
      if ( sline.substr(i,1) == String( ":" ) )
         p++; 
      else 
      {
         if ( p == 0 )
            cmd +=sline[i];
         else if ( p == 1 )
            param +=sline[i];
         else if ( p == 2 )
            param2 += sline[i];
      }
   }
   if ( cmd.equal( netFileCommands::list , 1 ) )
      SendFileListToClient();

   else if ( cmd.equal( netFileCommands::get , 1 ) ) 
      SendFileToClient( param );

   else if ( cmd.equal( netFileCommands::finished, 1 ) ) 
   {
      if ( currentlyUploadingFiles.size() > 0 )
      {
         String str = netFileCommands::get + String (":") +  currentlyUploadingFiles.first() + String("\n");
         currentlyUploadingFiles.erase(currentlyUploadingFiles.begin());
         send((const U8*)str.c_str(), dStrlen(str.c_str()));
      }
   }

   else if ( cmd.equal( netFileCommands::writefile , 1) )
   {
      if ( !prepareWrite(param.c_str() , dAtoui( param2.c_str() ) ) )
         {
         dropRest=true;
         String errmsg = String("Could not update local file '") + param + String("', It is ReadOnly.");
         disconnect();
         return false;
         }
   }

   else if ( cmd.equal( netFileCommands::requestsubmit ) )
      VerifyClientSubmit( param , param2 );

   return true;
}

void netFileServer::SendFileToClient(String file)
{
   FileObject* fs = new FileObject();
   if ( !fs->readMemory( file.c_str() ) )
      return;

   char* filelengthbuffer = netFileUtils::uinttochar(fs->getSize());
   String str = netFileCommands::writefile + String(":") + file + String(":")+ String(filelengthbuffer) + String("\n");

   //Let the client know what to expect for the file length
   send((const U8*)str.c_str(), dStrlen(str.c_str()));

   //Send the file.
   send((const U8 *) fs->getBuffer(), fs->getSize()) ;
}

void netFileServer::SendFileListToClient()
{
   Sim::postEvent( this , new FileList_SysEvent( 0 , FilesSize() ) , Sim::getCurrentTime() + 25 );
}

bool netFileServer::prepareWrite(const char* filename,U32 size)
{
   if ( fileSystemFunctions::isWriteableFileName( filename ) )
      {
         //Create a new File Object
         xferFile = new FileObject();
         //Mark it open for write
         xferFile->openForWrite(filename, false);
         //Set the expected size of the file.
         expectedDataSize = size ;
         return true;
      }
   return false;
}

void netFileServer::VerifyClientSubmit(String fileName, String crc)
{
   if (!onFileUploadRequest_callback(fileName.c_str()))
   {
      //Send Deny Message and exit
      String str = netFileCommands::denyWrite + String(":") + fileName + String("\n");
      send((const U8*)str.c_str(), dStrlen(str.c_str()));
      return;
   }
   //Check if the file exists
   if( Torque::FS::IsFile( fileName ) )
   {
      //Check to see if they are the same
      Torque::FS::FileNodeRef fileRef = Torque::FS::GetFileNode( fileName );

      U32 temp  = dAtoi( crc.c_str() );

      if( fileRef->getChecksum() == temp )
      {
         //Send Deny Message and exit
         String str = netFileCommands::denyWrite + String(":") + fileName + String("\n");
         send((const U8*)str.c_str(), dStrlen(str.c_str()));
         return;
      }
   }

   //See if the file is currently being written too
   for( int i = 0; i < currentlyUploadingFiles.size() ; ++i )
   {
      if( currentlyUploadingFiles[i] == fileName )
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
   if ( mState == Connected )
   {
      S32 err = Net::sendtoSocket(mTag, buffer, S32(len));
      while ( err == 3 ) //WouldBlock
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
   this->deleteObject();
}

bool netFileServer::start( S32 port )
{
   if (port == 0)
      port = Con::getIntVariable("$Pref::Server::Port") ;
   return this->listen(port);
}

void netFileServer::stop()
{
   this->disconnect();
}

void netFileServer::LoadPathPattern(const char* pattern, bool recursive, bool multiMatch, bool verbose)
{
   fileSystemFunctions::buildFileList(pattern, recursive, multiMatch);
   for (int i=0;i<fileSystemFunctions::sgFindFilesResults.size();i++)
   {
      if (verbose)
         Con::printf("Adding file %s to download Queue", fileSystemFunctions::sgFindFilesResults[i].c_str());
      FilesPush(fileSystemFunctions::sgFindFilesResults[i]);
   }
}

bool netFileServer::LoadFile(const char* filename)
{
   if (fileSystemFunctions::isFile(filename))
   {
      FilesPush(filename);
      return true;
   }
   return false;
}

DefineConsoleMethod( netFileServer, stop, void, ( ),,
   "Stops the server listing for connections .\n"
   )
{
   object->stop();
}

DefineConsoleMethod( netFileServer, start, void, ( S32 port ),( 0 ),
   "Starts the server listing for connections on specified port, if no port is passed it will use $Pref::Server::Port.\n"
   "@port - tcpip port to use.\n"
   "@return - true if able to listen on port.\n"
   )
{
   object->start(port);
}

DefineConsoleMethod( netFileServer, LoadPath, void, (const char* pattern, bool recursive, bool multiMatch, bool verbose), ( false ),
   "Loads a directory and pattern into the download queue.\n"
   "@pattern - Path and pattern to search for.\n"
   "@verbose - will echo to the console each file it adds.\n"
   )
{
   object->LoadPathPattern(pattern, recursive, multiMatch, verbose);
}

DefineConsoleMethod( netFileServer, LoadFile, bool, ( const char* filename ),,
   "Loads a directory and pattern into the download queue.\n"
   "@pattern - Path and pattern to search for.\n"
   "@verbose - will echo to the console each file it adds.\n"
   )
{
   return object->LoadFile(filename);
}


