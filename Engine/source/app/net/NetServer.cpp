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

#include "NetServer.h"
#include "T3D/gameBase/gameConnection.h"
#include "platform\platformNet.h"
#include "terrain\terrData.h"
#include "forest/forest.h"

#include "platform/platform.h"

MODULE_BEGIN( NetServer )
    
   MODULE_INIT
   {
   ManagedSingleton< NetServer >::createSingleton();
   }

MODULE_SHUTDOWN
   {
   ManagedSingleton< NetServer >::deleteSingleton();
   }

MODULE_END;



NetServer::NetServer()
   {
   mGameRunning=false;
   mServer=NULL;
   }

NetServer::~NetServer()
   {

   }


const char* NetServer::getSingletonName() 
   {
   return "NetServer"; 
   }


void NetServer::processTick()
   {
   if (GameConnection::getConnectionToServer()!=NULL)
      if (!GameConnection::getConnectionToServer()->isLocalConnection())
         return;

   if (Con::getIntVariable("$missionRunning",0) == 0)
      {
      mGameRunning=false;
      if (mServer)
         {
         mServer->FilesClear();
         mServer = NULL;
         }
      return;
      }

   if (!mGameRunning)
      {
#if defined(TORQUE_DEBUG)
      Con::printf("Start the FTP server");
#endif
      mGameRunning=true;
      mServer = new NetFTPServer();
      mServer->assignName("ftpConnection");
      U16 port = Con::getIntVariable("$Pref::Server::Port") + 1;
#if defined(TORQUE_DEBUG)
      Con::printf("Trying port %u",port);
#endif
      while (!mServer->listen(port))
         {
         port++;
#if defined(TORQUE_DEBUG)
         Con::printf("Trying port %u",port);
#endif
         }

      Con::printf("FILE TRANSFER SUBSYSTEM STARTED ON PORT (%u)",port);

      PushFiles();

      mServer->registerObject();
      SimSet* miscu = dynamic_cast<SimSet *>(Sim::findObject("MissionCleanup"));
      if (miscu)
         miscu->addObject(mServer);
      }   
   };

void NetServer::PushFiles()
   {
   //mServer->FilesPush(String(Con::getVariable("$Server::MissionFile")));

   TerrainBlock* tb=NULL;
   gServerContainer.initTypeSearch( TerrainObjectType );
   tb =dynamic_cast<TerrainBlock*>(Sim::findObject( gServerContainer.containerSearchNext()));
   while (tb)
      {
      mServer->FilesPush(tb->getTerrainFile());
      tb =dynamic_cast<TerrainBlock*>(Sim::findObject( gServerContainer.containerSearchNext()));
      }
   SimObject* tforest = Sim::findObject("theForest");
   Forest* forest = dynamic_cast<Forest*>(tforest);
   if (forest)
      {
      mServer->FilesPush(String(forest->getPath()));
      }

   String decalfile = String(Con::getVariable("$Server::MissionFile"))  + String(".decals");
   if (Torque::FS::IsFile(decalfile))
      {
      mServer->FilesPush(decalfile);
      }

   buildFileList("*.*",true,true);
   for (int i=0;i<sgFindFilesResults.size();i++)
      {
      if ((sgFindFilesResults[i].startsWith("art/")) //|| 
         //(sgFindFilesResults[i].startsWith("level/")) || 
         //(sgFindFilesResults[i].startsWith("shaders/common")) 
			)
            mServer->FilesPush(sgFindFilesResults[i]);
      }
   }

void NetServer::processonConnectionRequest_Event(const char* address, const char* ID)
   {
   
   }


S32  NetServer::buildFileList(const char* pattern, bool recurse, bool multiMatch)
{
   static const String sSlash( "/" );

   sgFindFilesResults.clear();

   String sPattern(Torque::Path::CleanSeparators(pattern));
   if(sPattern.isEmpty())
   {
      Con::errorf("findFirstFile() requires a search pattern");
      return -1;
   }

   if(!Con::expandScriptFilename(sgScriptFilenameBuffer, sizeof(sgScriptFilenameBuffer), sPattern.c_str()))
   {
      Con::errorf("findFirstFile() given initial directory cannot be expanded: '%s'", pattern);
      return -1;
   }
   sPattern = String::ToString(sgScriptFilenameBuffer);

   String::SizeType slashPos = sPattern.find('/', 0, String::Right);
//    if(slashPos == String::NPos)
//    {
//       Con::errorf("findFirstFile() missing search directory or expression: '%s'", sPattern.c_str());
//       return -1;
//    }

   // Build the initial search path
   Torque::Path givenPath(Torque::Path::CompressPath(sPattern));
   givenPath.setFileName("*");
   givenPath.setExtension("*");

   if(givenPath.getPath().length() > 0 && givenPath.getPath().find('*', 0, String::Right) == givenPath.getPath().length()-1)
   {
      // Deal with legacy searches of the form '*/*.*'
      String suspectPath = givenPath.getPath();
      String::SizeType newLen = suspectPath.length()-1;
      if(newLen > 0 && suspectPath.find('/', 0, String::Right) == suspectPath.length()-2)
      {
         --newLen;
      }
      givenPath.setPath(suspectPath.substr(0, newLen));
   }

   Torque::FS::FileSystemRef fs = Torque::FS::GetFileSystem(givenPath);
   //Torque::Path path = fs->mapTo(givenPath);
   Torque::Path path = givenPath;
   
   // Make sure that we have a root so the correct file system can be determined when using zips
   if(givenPath.isRelative())
      path = Torque::Path::Join(Torque::FS::GetCwd(), '/', givenPath);
   
   path.setFileName(String::EmptyString);
   path.setExtension(String::EmptyString);
   if(!Torque::FS::IsDirectory(path))
   {
      Con::errorf("findFirstFile() invalid initial search directory: '%s'", path.getFullPath().c_str());
      return -1;
   }

   // Build the search expression
   const String expression(slashPos != String::NPos ? sPattern.substr(slashPos+1) : sPattern);
   if(expression.isEmpty())
   {
      Con::errorf("findFirstFile() requires a search expression: '%s'", sPattern.c_str());
      return -1;
   }

   S32 results = Torque::FS::FindByPattern(path, expression, recurse, sgFindFilesResults, multiMatch );
   if(givenPath.isRelative() && results > 0)
   {
      // Strip the CWD out of the returned paths
      // MakeRelativePath() returns incorrect results (it adds a leading ..) so doing this the dirty way
      const String cwd = Torque::FS::GetCwd().getFullPath();
      for(S32 i = 0;i < sgFindFilesResults.size();++i)
      {
         String str = sgFindFilesResults[i];
         if(str.compare(cwd, cwd.length(), String::NoCase) == 0)
            str = str.substr(cwd.length());
         sgFindFilesResults[i] = str;
      }
   }
   return results;
}


#endif