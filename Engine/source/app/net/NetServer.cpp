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
#include "console/fileSystemFunctions.h"

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
      mGameRunning=true;
      mServer = new netFileServer();
      mServer->registerObject();
      SimSet* miscu = dynamic_cast<SimSet *>(Sim::findObject("MissionCleanup"));
      if (miscu)
         miscu->addObject(mServer);
      
      U16 port = Con::getIntVariable("$Pref::Server::Port") ;
      while (!mServer->listen(port))
         port++;
      Con::printf("FILE TRANSFER SUBSYSTEM STARTED ON PORT (%u)",port);
      PushFiles();
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
      mServer->FilesPush(String(forest->getPath()));

   String decalfile = String(Con::getVariable("$Server::MissionFile"))  + String(".decals");
   if (Torque::FS::IsFile(decalfile))
      mServer->FilesPush(decalfile);

   fileSystemFunctions::buildFileList("*.*",true,true);
   for (int i=0;i<fileSystemFunctions::sgFindFilesResults.size();i++)
      {
      if ((fileSystemFunctions::sgFindFilesResults[i].startsWith("art/")) //|| 
         //(sgFindFilesResults[i].startsWith("level/")) || 
         //(sgFindFilesResults[i].startsWith("shaders/common")) 
			)
            mServer->FilesPush(fileSystemFunctions::sgFindFilesResults[i]);
      }
   }
#endif