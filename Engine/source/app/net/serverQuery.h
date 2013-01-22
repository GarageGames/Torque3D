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

#ifndef _SERVERQUERY_H_
#define _SERVERQUERY_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _BITSET_H_
#include "core/bitSet.h"
#endif

#include "platform/platformNet.h"

//-----------------------------------------------------------------------------
// Game Server Information

struct ServerInfo
{
   enum StatusFlags
   {
      // Info flags (0-7):
      Status_Dedicated  = BIT(0),
      Status_Passworded = BIT(1),
      Status_Linux      = BIT(2),
      Status_Xenon      = BIT(6),

      // Status flags:
      Status_New         = 0,
      Status_Querying   = BIT(28),
      Status_Updating   = BIT(29),
      Status_Responded  = BIT(30),
      Status_TimedOut   = BIT(31),
   };

   U8          numPlayers;
   U8          maxPlayers;
   U8          numBots;
   char*       name;
   char*       gameType;
   char*       missionName;
   char*       missionType;
   char*       statusString;
   char*       infoString;
   NetAddress  address;
   U32         version;
   U32         ping;
   U32         cpuSpeed;
   bool        isFavorite;
   BitSet32    status;

   ServerInfo()
   {
      numPlayers = 0;
      maxPlayers = 0;
      numBots = 0;
      name = NULL;
      gameType = NULL;
      missionType = NULL;
      missionName = NULL;
      statusString = NULL;
      infoString = NULL;
      version = 0;
      ping = 0;
      cpuSpeed = 0;
      isFavorite = false;
      status = Status_New;
   }
   ~ServerInfo();

   bool isNew()            { return( status == Status_New ); }
   bool isQuerying()       { return( status.test( Status_Querying ) ); }
   bool isUpdating()       { return( status.test( Status_Updating ) ); }
   bool hasResponded()     { return( status.test( Status_Responded ) ); }
   bool isTimedOut()       { return( status.test( Status_TimedOut ) ); }

   bool isDedicated()      { return( status.test( Status_Dedicated ) ); }
   bool isPassworded()     { return( status.test( Status_Passworded ) ); }
   bool isLinux()          { return( status.test( Status_Linux ) ); }
   bool isXenon()          { return( status.test( Status_Xenon ) ); }
};


//-----------------------------------------------------------------------------

extern Vector<ServerInfo> gServerList;
extern bool gServerBrowserDirty;
extern void clearServerList();
extern void queryLanServers(U32 port, U8 flags, const char* gameType, const char* missionType,
      U8 minPlayers, U8 maxPlayers, U8 maxBots, U32 regionMask, U32 maxPing, U16 minCPU,
      U8 filterFlags);
extern void queryMasterGameTypes();
extern void queryMasterServer(U8 flags, const char* gameType, const char* missionType,
      U8 minPlayers, U8 maxPlayers, U8 maxBots, U32 regionMask, U32 maxPing, U16 minCPU,
      U8 filterFlags, U8 buddyCount, U32* buddyList );
extern void queryFavoriteServers( U8 flags );
extern void querySingleServer(const NetAddress* addr, U8 flags);
extern void startHeartbeat();
extern void sendHeartbeat( U8 flags );

#ifdef TORQUE_DEBUG
extern void addFakeServers( S32 howMany );
#endif // DEBUG

#endif
