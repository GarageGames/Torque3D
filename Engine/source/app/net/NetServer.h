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

#ifndef NetServer_h
#define NetServer_h


#include "core/module.h"
#include "core/iTickable.h"

#include "core/util/tVector.h"
#include "console\sim.h"
#include "console\simObject.h"
#include "console\simEvents.h"
#include "NetFTPServer.h"

class NetServer: public virtual ITickable
   {
   private:
      bool mGameRunning;
      NetFTPServer* mServer;
   public:
      NetServer();
      ~NetServer();

      static const char* getSingletonName();
      virtual void interpolateTick( F32 delta ){};
      virtual void processTick();
      virtual void advanceTime( F32 timeDelta ){};
      void PushFiles();

      static void processonConnectionRequest_Event(const char* address, const char* ID);
      S32  buildFileList(const char* pattern, bool recurse, bool multiMatch);
      
      Vector<String>   sgFindFilesResults;
      U32              sgFindFilesPos;
      char sgScriptFilenameBuffer[1024];
   };

#define NETSERVER ManagedSingleton<NetServer>::instance()


#endif

#endif