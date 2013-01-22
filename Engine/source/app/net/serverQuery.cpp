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

//-----------------------------------------------------------------------------
// Server Query States:
//    1: Master server query status  -> wait for master response
//    2: Master server packet status -> wait for master packets to arrive
//    3: Server ping status          -> wait for servers to respond to pings
//    4: Server query status         -> wait for servers to respond to queries
//    5: Done

// Master Server Packets:
// Header
//    message           Message id
//    flags             Query flags
//    sequenceNumber    Packet sequence id

// Server Query Filter Packet
//    packetIndex       Request specific page # (rest empty)
//    gameType          Game type string
//    missionType       Mission type string
//    minPlayers        At least this many players
//    maxPlayers        No more than this many
//    regions           Region mask, 0 = all
//    version           Server version, 0 = any
//    filterFlags       Server flags (dedicated, etc), 0 = any
//    maxBots           No more than maxBots
//    minCPUSpeed       At least this fast
//    playerCount       Buddy list search
//    playerList[playerCount]

// Master Server Info Packet
//    gameType          Game type string
//    missionType       Mission type string
//    maxPlayers        Max allowed
//    regions           Region mask
//    version           Server version #
//    infoFlags         Server flags (dedicated, etc)
//    numBots           Current bot count
//    CPUSpeed          Server CPU speed
//    playerCount       Current player count
//    playerList[playerCount]

// Game Info Query Packet
//    gameType          Game type string
//    missionType       Mission type string
//    missionName       You get one guess...
//    satusFlags        Dedicated, etc.
//    playerCount       Current player count
//    maxPlayers        Max allowed
//    numBots           Current bot count
//    CPUSpeed          Server CPU speed
//    statusString      Server info message
//    statusString      Server status message

// Accessed Environment Vars
//    Server::MissionType
//    Server::MissionName
//    Server::GameType
//    Server::ServerType
//    Server::PlayerCount
//    Server::BotCount
//    Server::GuidList[playerCount]
//    Server::Dedicated
//    Server::Status
//    Pref::Server::Name
//    Pref::Server::Password
//    Pref::Server::Info
//    Pref::Server::MaxPlayers
//    Pref::Server::RegionMask
//    Pref::Net::RegionMask
//    Pref::Client::Master[n]
//    Pref::Client::ServerFavoriteCount
//    Pref::Client::ServerFavorite[ServerFavoriteCount]
//-----------------------------------------------------------------------------

#include "app/net/serverQuery.h"

#include "platform/platform.h"
#include "core/dnet.h"
#include "core/util/tVector.h"
#include "core/stream/bitStream.h"
#include "console/console.h"
#include "console/simBase.h"
#include "app/banList.h"
#include "app/auth.h"
#include "sim/netConnection.h"
#include "sim/netInterface.h"

// cafTODO: breaks T2D
#include "T3D/gameBase/gameConnection.h"

// This is basically the server query protocol version now:
static const char* versionString = "VER1";

struct MasterInfo
{
   NetAddress address;
   U32 region;
};

Vector<ServerInfo> gServerList(__FILE__, __LINE__);
static Vector<MasterInfo> gMasterServerList(__FILE__, __LINE__);
static Vector<NetAddress> gFinishedList(__FILE__, __LINE__); // timed out servers and finished servers go here
NetAddress gMasterServerQueryAddress;
bool gServerBrowserDirty = false;

static const U32 gHeartbeatInterval = 120000;
static const S32 gMasterServerRetryCount = 3;
static const S32 gMasterServerTimeout = 2000;
static const S32 gPacketRetryCount = 4;
static const S32 gPacketTimeout = 1000;
static const S32 gMaxConcurrentPings = 10;
static const S32 gMaxConcurrentQueries = 2;
static const S32 gPingRetryCount = 4;
static const S32 gPingTimeout = 800;
static const S32 gQueryRetryCount = 4;
static const S32 gQueryTimeout = 1000;

// State variables:
static bool sgServerQueryActive = false;
static S32 gPingSession = 0;
static S32 gKey = 0;
static bool gGotFirstListPacket = false;

// Variables used for the interface:
static U32 gServerPingCount = 0;
static U32 gServerQueryCount = 0;
static U32 gHeartbeatSeq = 0;

class DemoNetInterface : public NetInterface
{
public:
   void handleInfoPacket(const NetAddress *address, U8 packetType, BitStream *stream);
};

DemoNetInterface gNetInterface;

ConsoleFunctionGroupBegin( ServerQuery, "Functions which allow you to query the LAN or a master server for online games.");

//-----------------------------------------------------------------------------

struct Ping
{
   NetAddress address;
   S32 session;
   S32 key;
   U32 time;
   U32 tryCount;
   bool broadcast;
};

static Ping gMasterServerPing;
static Vector<Ping> gPingList(__FILE__, __LINE__);
static Vector<Ping> gQueryList(__FILE__, __LINE__);

//-----------------------------------------------------------------------------

struct PacketStatus
{
   U8  index;
   S32 key;
   U32 time;
   U32 tryCount;

   PacketStatus() : index( 0 ), key(-1), time(0), tryCount( gPacketRetryCount ) {};

   PacketStatus( U8 _index, S32 _key, U32 _time )
   {
      index = _index;
      key = _key;
      time = _time;
      tryCount = gPacketRetryCount;
   }
};

static Vector<PacketStatus> gPacketStatusList(__FILE__, __LINE__);

//-----------------------------------------------------------------------------

struct ServerFilter
{
   enum Type
   {
      Normal      = 0,
      Buddy       = 1,
      Offline     = 2,
      Favorites   = 3,
   };

   Type  type;
   char* gameType;
   char* missionType;

   enum // Query Flags
   {
      OnlineQuery       = 0,        // Authenticated with master
      OfflineQuery      = BIT(0),   // On our own
      NoStringCompress  = BIT(1),
   };

   enum // Filter flags:
   {
      Dedicated         = BIT(0),
      NotPassworded     = BIT(1),
      Linux             = BIT(2),
      CurrentVersion    = BIT(7),
      NotXenon          = BIT(6)
   };

   U8    queryFlags;
   U8    minPlayers;
   U8    maxPlayers;
   U8    maxBots;
   U32   regionMask;
   U32   maxPing;
   U8    filterFlags;
   U16   minCPU;
   U8    buddyCount;
   U32*  buddyList;

   ServerFilter()
   {
      queryFlags = 0;
      gameType = NULL;
      missionType = NULL;
      minPlayers = 0;
      maxPlayers = 255;
      maxBots = 16;
      regionMask = 0xFFFFFFFF;
      maxPing = 0;
      filterFlags = 0;
      minCPU = 0;
      buddyCount = 0;
      buddyList = NULL;
   }

   ~ServerFilter()
   {
      if ( gameType )
         dFree( gameType );
      if ( missionType )
         dFree( missionType );
      if ( buddyList )
         dFree( buddyList );
   }
};

static ServerFilter sActiveFilter;


//-----------------------------------------------------------------------------
// Forward function declarations:
//-----------------------------------------------------------------------------

static void pushPingRequest( const NetAddress *addr );
static void pushPingBroadcast( const NetAddress *addr );
static void pushServerFavorites();
static bool pickMasterServer();
static S32 findPingEntry( Vector<Ping> &v, const NetAddress* addr );
static bool addressFinished( const NetAddress* addr );
static ServerInfo* findServerInfo( const NetAddress* addr );
static ServerInfo* findOrCreateServerInfo( const NetAddress* addr );
static void removeServerInfo( const NetAddress* addr );
static void sendPacket( U8 pType, const NetAddress* addr, U32 key, U32 session, U8 flags );
static void writeCString( BitStream* stream, const char* string );
static void readCString( BitStream* stream, char* buffer );
static void writeLongCString( BitStream* stream, const char* string );
static void readLongCString( BitStream* stream, char* buffer );
static void processMasterServerQuery( U32 session );
static void processPingsAndQueries( U32 session, bool schedule = true);
static void processServerListPackets( U32 session );
static void processHeartbeat(U32);
static void updatePingProgress();
static void updateQueryProgress();
Vector<MasterInfo>* getMasterServerList();
bool pickMasterServer();
void clearServerList();


//-----------------------------------------------------------------------------
// Events
//-----------------------------------------------------------------------------

//----------------------------------------------------------------
class ProcessMasterQueryEvent : public SimEvent
{
   U32 session;
   public:
      ProcessMasterQueryEvent( U32 _session )
      {
         session = _session;
      }
      void process( SimObject *object )
      {
         processMasterServerQuery( session );
      }
};

//----------------------------------------------------------------
class ProcessPingEvent : public SimEvent
{
   U32 session;
   public:
      ProcessPingEvent( U32 _session )
      {
         session = _session;
      }
      void process( SimObject *object )
      {
         processPingsAndQueries( session );
      }
};

//----------------------------------------------------------------
class ProcessPacketEvent : public SimEvent
{
   U32 session;
   public:
      ProcessPacketEvent( U32 _session )
      {
         session = _session;
      }

      void process( SimObject *object )
      {
         processServerListPackets( session );
      }
};

//----------------------------------------------------------------
class HeartbeatEvent : public SimEvent
{
   U32 mSeq;
   public:
      HeartbeatEvent(U32 seq)
      {
         mSeq = seq;
      }
      void process( SimObject *object )
      {
         processHeartbeat(mSeq);
      }
};


//-----------------------------------------------------------------------------
// Public query methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

void queryLanServers(U32 port, U8 flags, const char* gameType, const char* missionType,
      U8 minPlayers, U8 maxPlayers, U8 maxBots, U32 regionMask, U32 maxPing, U16 minCPU,
      U8 filterFlags)
{
   sgServerQueryActive = true;
   pushServerFavorites();

      sActiveFilter.type = ServerFilter::Offline;

      // Clear the filter:
      if ( !sActiveFilter.gameType || dStricmp( sActiveFilter.gameType, "Any" ) != 0 )
      {
         sActiveFilter.gameType = (char*) dRealloc( sActiveFilter.gameType, 4 );
         dStrcpy( sActiveFilter.gameType, "Any" );
      }
      if ( !sActiveFilter.missionType || dStricmp( sActiveFilter.missionType, "Any" ) != 0 )
      {
         sActiveFilter.missionType = (char*) dRealloc( sActiveFilter.missionType, 4 );
         dStrcpy( sActiveFilter.missionType, "Any" );
      }
      sActiveFilter.queryFlags   = 0;
   sActiveFilter.minPlayers   = minPlayers;
   sActiveFilter.maxPlayers   = maxPlayers;
   sActiveFilter.maxBots      = maxBots;
   sActiveFilter.regionMask   = regionMask;
   sActiveFilter.maxPing      = maxPing;
   sActiveFilter.minCPU       = minCPU;
   sActiveFilter.filterFlags  = filterFlags;

   NetAddress addr;
   char addrText[256];
   dSprintf( addrText, sizeof( addrText ), "IP:BROADCAST:%d", port );
   Net::stringToAddress( addrText, &addr );
   pushPingBroadcast( &addr );

   Con::executef("onServerQueryStatus", "start", "Querying LAN servers", "0");
   processPingsAndQueries( gPingSession );
}

//-----------------------------------------------------------------------------

ConsoleFunction( queryAllServers, void, 12, 12, "queryAllServers(...);" )
{
   TORQUE_UNUSED(argc);

   U32 lanPort = dAtoi(argv[1]);
   U8 flags = dAtoi(argv[2]);

   // It's not a good idea to hold onto args, recursive calls to
   // console exec will trash them.
   char* gameType = dStrdup(argv[3]);
   char* missionType = dStrdup(argv[4]);

   U8 minPlayers = dAtoi(argv[5]);
   U8 maxPlayers = dAtoi(argv[6]);
   U8 maxBots = dAtoi(argv[7]);
   U32 regionMask = dAtoi(argv[8]);
   U32 maxPing = dAtoi(argv[9]);
   U16 minCPU = dAtoi(argv[10]);
   U8 filterFlags = dAtoi(argv[11]);
   U32 buddyList = 0;

   clearServerList();

   queryMasterServer(flags,gameType,missionType,minPlayers,maxPlayers,
      maxBots,regionMask,maxPing,minCPU,filterFlags,0,&buddyList);

   queryLanServers(lanPort, flags, gameType, missionType, minPlayers, maxPlayers, maxBots,
	   regionMask, maxPing, minCPU, filterFlags);
   dFree(gameType);
   dFree(missionType);

}
ConsoleFunction( queryLanServers, void, 12, 12, "queryLanServers(...);" )
{
   TORQUE_UNUSED(argc);

   U32 lanPort = dAtoi(argv[1]);
   U8 flags = dAtoi(argv[2]);

   // It's not a good idea to hold onto args, recursive calls to
   // console exec will trash them.
   char* gameType = dStrdup(argv[3]);
   char* missionType = dStrdup(argv[4]);

   U8 minPlayers = dAtoi(argv[5]);
   U8 maxPlayers = dAtoi(argv[6]);
   U8 maxBots = dAtoi(argv[7]);
   U32 regionMask = dAtoi(argv[8]);
   U32 maxPing = dAtoi(argv[9]);
   U16 minCPU = dAtoi(argv[10]);
   U8 filterFlags = dAtoi(argv[11]);

   clearServerList();
   queryLanServers(lanPort, flags, gameType, missionType, minPlayers, maxPlayers, maxBots,
	   regionMask, maxPing, minCPU, filterFlags);

   dFree(gameType);
   dFree(missionType);
}

//-----------------------------------------------------------------------------

void queryMasterGameTypes()
{
   Vector<MasterInfo> *masterList = getMasterServerList();
   if ( masterList->size() != 0 )
   {
      U32 master = Sim::getCurrentTime() % masterList->size();
      // Send a request to the master server for the game types:
      Con::printf( "Requesting game types from the master server..." );
      sendPacket( NetInterface::MasterServerGameTypesRequest, &(*masterList)[master].address, gKey, gPingSession, 0 );
   }
}

//-----------------------------------------------------------------------------

void queryMasterServer(U8 flags, const char* gameType, const char* missionType,
      U8 minPlayers, U8 maxPlayers, U8 maxBots, U32 regionMask, U32 maxPing,
      U16 minCPU, U8 filterFlags, U8 buddyCount, U32* buddyList )
{
   // Reset the list packet flag:
   gGotFirstListPacket = false;
   sgServerQueryActive = true;

   Con::executef( "onServerQueryStatus", "start", "Querying master server", "0");

   if ( buddyCount == 0 )
   {
      sActiveFilter.type = ServerFilter::Normal;

      // Update the active filter:
      if ( !sActiveFilter.gameType || dStrcmp( sActiveFilter.gameType, gameType ) != 0 )
      {
         sActiveFilter.gameType = (char*) dRealloc( sActiveFilter.gameType, dStrlen( gameType ) + 1 );
         dStrcpy( sActiveFilter.gameType, gameType );
      }

      if ( !sActiveFilter.missionType || dStrcmp( sActiveFilter.missionType, missionType ) != 0 )
      {
         sActiveFilter.missionType = (char*) dRealloc( sActiveFilter.missionType, dStrlen( missionType ) + 1 );
         dStrcpy( sActiveFilter.missionType, missionType );
      }

      sActiveFilter.queryFlags   = flags;
      sActiveFilter.minPlayers   = minPlayers;
      sActiveFilter.maxPlayers   = maxPlayers;
      sActiveFilter.maxBots      = maxBots;
      sActiveFilter.regionMask   = regionMask;
      sActiveFilter.maxPing      = maxPing;
      sActiveFilter.minCPU       = minCPU;
      sActiveFilter.filterFlags  = filterFlags;
      sActiveFilter.buddyCount   = buddyCount;
      dFree( sActiveFilter.buddyList );
      sActiveFilter.buddyList = NULL;
   }
   else
   {
      sActiveFilter.type = ServerFilter::Buddy;
      sActiveFilter.buddyCount = buddyCount;
      sActiveFilter.buddyList = (U32*) dRealloc( sActiveFilter.buddyList, buddyCount * 4 );
      dMemcpy( sActiveFilter.buddyList, buddyList, buddyCount * 4 );
      clearServerList();
   }

   // Pick a random master server from the list:
   gMasterServerList.clear();
   Vector<MasterInfo> *masterList = getMasterServerList();
   for ( U32 i = 0; i < masterList->size(); i++ )
      gMasterServerList.push_back( (*masterList)[i] );

   // Clear the master server ping:
   gMasterServerPing.time = 0;
   gMasterServerPing.tryCount = gMasterServerRetryCount;

   if ( !pickMasterServer() )
      Con::errorf( "No master servers found!" );
   else
      processMasterServerQuery( gPingSession );
}

ConsoleFunction( queryMasterServer, void, 11, 11, "queryMasterServer(...);" )
{
   TORQUE_UNUSED(argc);

   U8 flags = dAtoi(argv[1]);

   // It's not a good idea to hold onto args, recursive calls to
   // console exec will trash them.
   char* gameType = dStrdup(argv[2]);
   char* missionType = dStrdup(argv[3]);

   U8 minPlayers = dAtoi(argv[4]);
   U8 maxPlayers = dAtoi(argv[5]);
   U8 maxBots = dAtoi(argv[6]);
   U32 regionMask = dAtoi(argv[7]);
   U32 maxPing = dAtoi(argv[8]);
   U16 minCPU = dAtoi(argv[9]);
   U8 filterFlags = dAtoi(argv[10]);
   U32 buddyList = 0;

   clearServerList();
   queryMasterServer(flags,gameType,missionType,minPlayers,maxPlayers,
      maxBots,regionMask,maxPing,minCPU,filterFlags,0,&buddyList);

   dFree(gameType);
   dFree(missionType);
}

//-----------------------------------------------------------------------------

ConsoleFunction( querySingleServer, void, 3, 3, "querySingleServer(address, flags);" )
{
   TORQUE_UNUSED(argc);

   NetAddress addr;
   char* addrText;

   addrText = dStrdup(argv[1]);
   U8 flags = dAtoi(argv[2]);


   Net::stringToAddress( addrText, &addr );
   querySingleServer(&addr,flags);
}

//-----------------------------------------------------------------------------

void queryFavoriteServers( U8 /*flags*/ )
{
   sgServerQueryActive = true;
   clearServerList();
   sActiveFilter.type = ServerFilter::Favorites;
   pushServerFavorites();

   Con::executef( "onServerQueryStatus", "start", "Query favorites...", "0" );
   processPingsAndQueries( gPingSession );
}

//-----------------------------------------------------------------------------

void querySingleServer( const NetAddress* addr, U8 /*flags*/ )
{
   sgServerQueryActive = true;
   ServerInfo* si = findServerInfo( addr );
   if ( si )
      si->status = ServerInfo::Status_New | ServerInfo::Status_Updating;

   // Remove the server from the finished list (if it's there):
   for ( U32 i = 0; i < gFinishedList.size(); i++ )
   {
      if ( Net::compareAddresses( addr, &gFinishedList[i] ) )
      {
         gFinishedList.erase( i );
         break;
      }
   }

   Con::executef( "onServerQueryStatus", "start", "Refreshing server...", "0" );
   gServerPingCount = gServerQueryCount = 0;
   pushPingRequest( addr );
   processPingsAndQueries( gPingSession );
}

//-----------------------------------------------------------------------------

void cancelServerQuery()
{
   // Cancel the current query, if there is anything left
   // on the ping list, it's dropped.
   if ( sgServerQueryActive )
   {
      Con::printf( "Server query canceled." );
      ServerInfo* si;

      // Clear the master server packet list:
      gPacketStatusList.clear();

      // Clear the ping list:
      while ( gPingList.size() )
      {
         si = findServerInfo( &gPingList[0].address );
         if ( si && !si->status.test( ServerInfo::Status_Responded ) )
            si->status = ServerInfo::Status_TimedOut;

         gPingList.erase( U32( 0 ) );
      }

      // Clear the query list:
      while ( gQueryList.size() )
      {
         si = findServerInfo( &gQueryList[0].address );
         if ( si && !si->status.test( ServerInfo::Status_Responded ) )
            si->status = ServerInfo::Status_TimedOut;

         gQueryList.erase( U32( 0 ) );
      }

      sgServerQueryActive = false;
      gServerBrowserDirty = true;
   }
}

ConsoleFunction( cancelServerQuery, void, 1, 1, "cancelServerQuery()" )
{
   TORQUE_UNUSED(argc); TORQUE_UNUSED(argv);
   cancelServerQuery();
}

//-----------------------------------------------------------------------------

void stopServerQuery()
{
   // Cancel the current query, anything left on the ping
   // list is moved to the finished list as "done".
   if ( sgServerQueryActive )
   {
      gPacketStatusList.clear();

      if ( gPingList.size() )
      {
         while ( gPingList.size() )
         {
            gFinishedList.push_back( gPingList[0].address );
            gPingList.erase( U32( 0 ) );
         }
      }
      else
         cancelServerQuery();
   }
}

ConsoleFunction( stopServerQuery, void, 1, 1, "stopServerQuery()" )
{
   TORQUE_UNUSED(argc); TORQUE_UNUSED(argv);
   stopServerQuery();
}

//-----------------------------------------------------------------------------

ConsoleFunction(startHeartbeat, void, 1, 1, "startHeartbeat()")
{
   TORQUE_UNUSED(argc); TORQUE_UNUSED(argv);

   if (validateAuthenticatedServer()) {
      gHeartbeatSeq++;
      processHeartbeat(gHeartbeatSeq);  // thump-thump...
   }
}

ConsoleFunction(stopHeartbeat, void, 1, 1, "stopHeartbeat();")
{
   TORQUE_UNUSED(argc); TORQUE_UNUSED(argv);
   gHeartbeatSeq++;
}

//-----------------------------------------------------------------------------

ConsoleFunction( getServerCount, int, 1, 1, "getServerCount();" )
{
   TORQUE_UNUSED(argv); TORQUE_UNUSED(argc);
   return gServerList.size();
}

ConsoleFunction( setServerInfo, bool, 2, 2, "setServerInfo(index);" )
{
   TORQUE_UNUSED(argc);
   U32 index = dAtoi(argv[1]);
   if (index >= 0 && index < gServerList.size()) {
      ServerInfo& info = gServerList[index];

      char addrString[256];
      Net::addressToString( &info.address, addrString );

      Con::setIntVariable("ServerInfo::Status",info.status);
      Con::setVariable("ServerInfo::Address",addrString);
      Con::setVariable("ServerInfo::Name",info.name);
      Con::setVariable("ServerInfo::GameType",info.gameType);
      Con::setVariable("ServerInfo::MissionName",info.missionName);
      Con::setVariable("ServerInfo::MissionType",info.missionType);
      Con::setVariable("ServerInfo::State",info.statusString);
      Con::setVariable("ServerInfo::Info",info.infoString);
      Con::setIntVariable("ServerInfo::PlayerCount",info.numPlayers);
      Con::setIntVariable("ServerInfo::MaxPlayers",info.maxPlayers);
      Con::setIntVariable("ServerInfo::BotCount",info.numBots);
      Con::setIntVariable("ServerInfo::Version",info.version);
      Con::setIntVariable("ServerInfo::Ping",info.ping);
      Con::setIntVariable("ServerInfo::CPUSpeed",info.cpuSpeed);
      Con::setBoolVariable("ServerInfo::Favorite",info.isFavorite);
      Con::setBoolVariable("ServerInfo::Dedicated",info.isDedicated());
      Con::setBoolVariable("ServerInfo::Password",info.isPassworded());
      return true;
   }
   return false;
}


//-----------------------------------------------------------------------------
// Internal
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

ServerInfo::~ServerInfo()
{
   if ( name )
      dFree( name );
   if ( gameType )
      dFree( gameType );
   if ( missionType )
      dFree( missionType );
   if( missionName )
      dFree( missionName );
   if ( statusString )
      dFree( statusString );
   if ( infoString )
      dFree( infoString );
}

//-----------------------------------------------------------------------------

Vector<MasterInfo>* getMasterServerList()
{
   // This code used to get the master server list from the
   // WON.net directory.
   static Vector<MasterInfo> masterList;
   masterList.clear();

   for (U32 i = 0; i < 10; i++) {
      char buffer[50];
      dSprintf(buffer,sizeof(buffer),"pref::Master%d",i);
      const char* master = Con::getVariable(buffer);
      if (master && *master) {
         NetAddress address;
         // Format for master server variable:
         //    regionMask:netAddress
         U32 region = 1; // needs to default to something > 0
         dSscanf(master,"%d:",&region);
         const char* madd = dStrchr(master,':') + 1;
         if (region && Net::stringToAddress(madd,&address)) {
            masterList.increment();
            MasterInfo& info = masterList.last();
            info.address = address;
            info.region = region;
         }
         else
            Con::errorf("Bad master server address: %s",master);
      }
   }

   if (!masterList.size())
      Con::errorf("No master servers found");

   return &masterList;
}


//-----------------------------------------------------------------------------

bool pickMasterServer()
{
   // Reset the master server ping:
   gMasterServerPing.time = 0;
   gMasterServerPing.key = 0;
   gMasterServerPing.tryCount = gMasterServerRetryCount;
   gMasterServerPing.session = gPingSession;

   char addrString[256];
   U32 serverCount = gMasterServerList.size();
   if ( !serverCount )
   {
      // There are no more servers left to try...:(
      return( false );
   }

   U32 region = Con::getIntVariable( "$pref::Net::RegionMask" );
   U32 index = Sim::getCurrentTime() % serverCount;

   // First try to find a master server in the same region:
   for ( U32 i = 0; i < serverCount; i++ )
   {
      if ( gMasterServerList[index].region == region )
      {
         Net::addressToString( &gMasterServerList[index].address, addrString );
         Con::printf( "Found master server %s in same region.", addrString );
         gMasterServerPing.address = gMasterServerList[index].address;
         return( true );
      }

      index = index < serverCount - 1 ? index + 1 : 0;
   }

   // Settle for the one we first picked:
   Net::addressToString( &gMasterServerList[index].address, addrString );
   Con::printf( "No master servers found in this region, trying %s.", addrString );
   gMasterServerPing.address = gMasterServerList[index].address;

   return( true );
}

//-----------------------------------------------------------------------------

void clearServerList()
{
   gPacketStatusList.clear();
   gServerList.clear();
   gFinishedList.clear();
   gPingList.clear();
   gQueryList.clear();
   gServerPingCount = gServerQueryCount = 0;

   gPingSession++;
}

//-----------------------------------------------------------------------------

void sendHeartbeat( U8 flags )
{
   // send heartbeats to all of the master servers:
   Vector<MasterInfo> *masterList = getMasterServerList();
   for(U32 i = 0; i < masterList->size(); i++)
   {
      char buffer[256];
      Net::addressToString(&(*masterList)[i].address, buffer);
      // Send a request to the master server for the game types:
      Con::printf( "Sending heartbeat to master server [%s]", buffer );
      sendPacket( NetInterface::GameHeartbeat, &(*masterList)[i].address, 0, gPingSession, flags );
   }
}

//-----------------------------------------------------------------------------

static void pushPingRequest( const NetAddress* addr )
{
   if( addressFinished( addr ) )
      return;

   Ping p;
   p.address = *addr;
   p.session = gPingSession;
   p.key = 0;
   p.time = 0;
   p.tryCount = gPingRetryCount;
   p.broadcast = false;
   gPingList.push_back( p );
   gServerPingCount++;
}

//-----------------------------------------------------------------------------

static void pushPingBroadcast( const NetAddress* addr )
{
   if( addressFinished( addr ) )
      return;

   Ping p;
   p.address = *addr;
   p.session = gPingSession;
   p.key = 0;
   p.time = 0;
   p.tryCount = 1; // only try this once
   p.broadcast = true;
   gPingList.push_back( p );
   // Don't increment gServerPingCount, broadcasts are not
   // counted as requests.
}

//-----------------------------------------------------------------------------

static S32 countPingRequests()
{
   // Need a function here because the ping list also includes
   // broadcast pings we don't want counted.
   U32 pSize = gPingList.size(), count = pSize;
   for (U32 i = 0; i < pSize; i++)
      if (gPingList[i].broadcast)
         count--;
   return count;
}


//-----------------------------------------------------------------------------

static void pushServerFavorites()
{
   S32 count = Con::getIntVariable( "$pref::Client::ServerFavoriteCount" );
   if ( count < 0 )
   {
      Con::setIntVariable( "$pref::Client::ServerFavoriteCount", 0 );
      return;
   }

   NetAddress addr;
   const char* server = NULL;
   char buf[256], serverName[25], addrString[256];
   U32 sz, len;
   for ( S32 i = 0; i < count; i++ )
   {
      dSprintf( buf, sizeof( buf ), "pref::Client::ServerFavorite%d", i );
      server = Con::getVariable( buf );
      if ( server )
      {
         sz = dStrcspn( server, "\t" );
         if ( sz > 0 )
         {
            len = sz > 24 ? 24 : sz;
            dStrncpy( serverName, server, len );
            serverName[len] = 0;
            dStrncpy( addrString, server + ( sz + 1 ), 255 );

            //Con::errorf( "Pushing server favorite \"%s\" - %s...", serverName, addrString );
            Net::stringToAddress( addrString, &addr );
            ServerInfo* si = findOrCreateServerInfo( &addr );
            AssertFatal(si, "pushServerFavorites - failed to create Server Info!" );
            si->name = (char*) dRealloc( (void*) si->name, dStrlen( serverName ) + 1 );
            dStrcpy( si->name, serverName );
            si->isFavorite = true;
            pushPingRequest( &addr );
         }
      }
   }
}

//-----------------------------------------------------------------------------

static S32 findPingEntry( Vector<Ping> &v, const NetAddress* addr )
{
   for ( U32 i = 0; i < v.size(); i++ )
      if ( Net::compareAddresses( addr, &v[i].address ) )
         return S32( i );
   return -1;
}

//-----------------------------------------------------------------------------

static bool addressFinished( const NetAddress* addr )
{
   for ( U32 i = 0; i < gFinishedList.size(); i++ )
      if ( Net::compareAddresses( addr, &gFinishedList[i] ) )
         return true;
   return false;
}

//-----------------------------------------------------------------------------

static ServerInfo* findServerInfo( const NetAddress* addr )
{
   for ( U32 i = 0; i < gServerList.size(); i++ )
      if ( Net::compareAddresses( addr, &gServerList[i].address ) )
         return &gServerList[i];
   return NULL;
}

//-----------------------------------------------------------------------------

static ServerInfo* findOrCreateServerInfo( const NetAddress* addr )
{
   ServerInfo* ret = findServerInfo( addr );
   if ( ret )
      return ret;

   ServerInfo si;
   si.address = *addr;
   gServerList.push_back( si );

   return &gServerList.last();
}

//-----------------------------------------------------------------------------

static void removeServerInfo( const NetAddress* addr )
{
   for ( U32 i = 0; i < gServerList.size(); i++ )
   {
      if ( Net::compareAddresses( addr, &gServerList[i].address ) )
      {
         gServerList.erase( i );
         gServerBrowserDirty = true;
      }
   }
}

//-----------------------------------------------------------------------------

#if defined(TORQUE_DEBUG)
// This function is solely for testing the functionality of the server browser
// with more servers in the list.
void addFakeServers( S32 howMany )
{
   static S32 sNumFakeServers = 1;
   ServerInfo newServer;

   for ( S32 i = 0; i < howMany; i++ )
   {
      newServer.numPlayers = (U8)(Platform::getRandom() * 64.0f);
      newServer.maxPlayers = 64;
      char buf[256];
      dSprintf( buf, 255, "Fake server #%d", sNumFakeServers );
      newServer.name = (char*) dMalloc( dStrlen( buf ) + 1 );
      dStrcpy( newServer.name, buf );
      newServer.gameType = (char*) dMalloc( 5 );
      dStrcpy( newServer.gameType, "Fake" );
      newServer.missionType = (char*) dMalloc( 4 );
      dStrcpy( newServer.missionType, "FakeMissionType" );
      newServer.missionName = (char*) dMalloc( 14 );
      dStrcpy( newServer.missionName, "FakeMapName" );
      Net::stringToAddress( "IP:198.74.33.35:28000", &newServer.address );
      newServer.ping = (U32)( Platform::getRandom() * 200.0f );
      newServer.cpuSpeed = 470;
      newServer.status = ServerInfo::Status_Responded;

      gServerList.push_back( newServer );
      sNumFakeServers++;
   }

   gServerBrowserDirty = true;
}
#endif // DEBUG

//-----------------------------------------------------------------------------

static void sendPacket( U8 pType, const NetAddress* addr, U32 key, U32 session, U8 flags )
{
   BitStream *out = BitStream::getPacketStream();
   out->clearStringBuffer();
   out->write( pType );
   out->write( flags );
   out->write( U32( ( session << 16 ) | ( key & 0xFFFF ) ) );

   BitStream::sendPacketStream(addr);
}

//-----------------------------------------------------------------------------

static void writeCString( BitStream* stream, const char* string )
{
   U8 strLen = ( string != NULL ) ? dStrlen( string ) : 0;
   stream->write( strLen );
   for ( U32 i = 0; i < strLen; i++ )
      stream->write( U8( string[i] ) );
}

//-----------------------------------------------------------------------------

static void readCString( BitStream* stream, char* buffer )
{
   U32 i;
   U8 strLen;
   stream->read( &strLen );
   for ( i = 0; i < strLen; i++ )
   {
      U8* ptr = (U8*) buffer;
      stream->read( &ptr[i] );
   }
   buffer[i] = 0;
}

//-----------------------------------------------------------------------------

static void writeLongCString( BitStream* stream, const char* string )
{
   U16 strLen = ( string != NULL ) ? dStrlen( string ) : 0;
   stream->write( strLen );
   for ( U32 i = 0; i < strLen; i++ )
      stream->write( U8( string[i] ) );
}

//-----------------------------------------------------------------------------

static void readLongCString( BitStream* stream, char* buffer )
{
   U32 i;
   U16 strLen;
   stream->read( &strLen );
   for ( i = 0; i < strLen; i++ )
   {
      U8* ptr = (U8*) buffer;
      stream->read( &ptr[i] );
   }
   buffer[i] = 0;
}

//-----------------------------------------------------------------------------
// Event processing
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

static void processMasterServerQuery( U32 session )
{
   if ( session != gPingSession || !sgServerQueryActive )
      return;

   if ( !gGotFirstListPacket )
   {
      bool keepGoing = true;
      U32 time = Platform::getVirtualMilliseconds();
      char addressString[256];

      if ( gMasterServerPing.time + gMasterServerTimeout < time )
      {
         Net::addressToString( &gMasterServerPing.address, addressString );
         if ( !gMasterServerPing.tryCount )
         {
            // The query timed out.
            Con::printf( "Server list request to %s timed out.", addressString );

            // Remove this server from the list:
            for ( U32 i = 0; i < gMasterServerList.size(); i++ )
            {
               if ( Net::compareAddresses( &gMasterServerList[i].address, &gMasterServerPing.address ) )
               {
                  gMasterServerList.erase( i );
                  break;
               }
            }

            // Pick a new master server to try:
            keepGoing = pickMasterServer();
            if ( keepGoing )
            {
               Con::executef( "onServerQueryStatus", "update", "Switching master servers...", "0" );
               Net::addressToString( &gMasterServerPing.address, addressString );
            }
         }

         if ( keepGoing )
         {
            gMasterServerPing.tryCount--;
            gMasterServerPing.time = time;
            gMasterServerPing.key = gKey++;

            // Send a request to the master server for the server list:
            BitStream *out = BitStream::getPacketStream();
            out->clearStringBuffer();
            out->write( U8( NetInterface::MasterServerListRequest ) );
            out->write( U8( sActiveFilter.queryFlags) );
            out->write( ( gMasterServerPing.session << 16 ) | ( gMasterServerPing.key & 0xFFFF ) );
            out->write( U8( 255 ) );
            writeCString( out, sActiveFilter.gameType );
            writeCString( out, sActiveFilter.missionType );
            out->write( sActiveFilter.minPlayers );
            out->write( sActiveFilter.maxPlayers );
            out->write( sActiveFilter.regionMask );
            U32 version = ( sActiveFilter.filterFlags & ServerFilter::CurrentVersion ) ? getVersionNumber() : 0;
            out->write( version );
            out->write( sActiveFilter.filterFlags );
            out->write( sActiveFilter.maxBots );
            out->write( sActiveFilter.minCPU );
            out->write( sActiveFilter.buddyCount );
            for ( U32 i = 0; i < sActiveFilter.buddyCount; i++ )
               out->write( sActiveFilter.buddyList[i] );

            BitStream::sendPacketStream( &gMasterServerPing.address );

            Con::printf( "Requesting the server list from master server %s (%d tries left)...", addressString, gMasterServerPing.tryCount );
            if ( gMasterServerPing.tryCount < gMasterServerRetryCount - 1 )
               Con::executef( "onServerQueryStatus", "update", "Retrying the master server...", "0" );
         }
      }

      if ( keepGoing )
      {
         // schedule another check:
         Sim::postEvent( Sim::getRootGroup(), new ProcessMasterQueryEvent( session ), Sim::getTargetTime() + 1 );
      }
      else
      {
         Con::errorf( "There are no more master servers to try!" );
         Con::executef( "onServerQueryStatus", "done", "No master servers found.", "0" );
      }
   }
}

//-----------------------------------------------------------------------------

static void processPingsAndQueries( U32 session, bool schedule )
{
   if( session != gPingSession )
      return;

   U32 i = 0;
   U32 time = Platform::getVirtualMilliseconds();
   char addressString[256];
   U8 flags = ServerFilter::OnlineQuery;
   bool waitingForMaster = ( sActiveFilter.type == ServerFilter::Normal ) && !gGotFirstListPacket && sgServerQueryActive;

   for ( i = 0; i < gPingList.size() && i < gMaxConcurrentPings; )
   {
      Ping &p = gPingList[i];

      if ( p.time + gPingTimeout < time )
      {
         if ( !p.tryCount )
         {
            // it's timed out.
            if (!p.broadcast)
            {
               Net::addressToString( &p.address, addressString );
               Con::printf( "Ping to server %s timed out.", addressString );
            }

            // If server info is in list (favorite), set its status:
            ServerInfo* si = findServerInfo( &p.address );
            if ( si )
            {
               si->status = ServerInfo::Status_TimedOut;
               gServerBrowserDirty = true;
            }

            gFinishedList.push_back( p.address );
            gPingList.erase( i );

            if ( !waitingForMaster )
               updatePingProgress();
         }
         else
         {
            p.tryCount--;
            p.time = time;
            p.key = gKey++;

            Net::addressToString( &p.address, addressString );

            if (p.broadcast)
               Con::printf( "LAN server ping: %s...", addressString );
            else
               Con::printf( "Pinging Server %s (%d)...", addressString, p.tryCount );
            sendPacket( NetInterface::GamePingRequest, &p.address, p.key, p.session, flags );
            i++;
         }
      }
      else
         i++;
   }

   if ( !gPingList.size() && !waitingForMaster )
   {
      // Start the query phase:
      for ( U32 i = 0; i < gQueryList.size() && i < gMaxConcurrentQueries; )
      {
         Ping &p = gQueryList[i];
         if ( p.time + gPingTimeout < time )
         {
            ServerInfo* si = findServerInfo( &p.address );
            if ( !si )
            {
               // Server info not found, so remove the query:
               gQueryList.erase( i );
               gServerBrowserDirty = true;
               continue;
            }

            Net::addressToString( &p.address, addressString );
            if ( !p.tryCount )
            {
               Con::printf( "Query to server %s timed out.", addressString );
               si->status = ServerInfo::Status_TimedOut;
               gQueryList.erase( i );
               gServerBrowserDirty = true;
            }
            else
            {
               p.tryCount--;
               p.time = time;
               p.key = gKey++;

               Con::printf( "Querying Server %s (%d)...", addressString, p.tryCount );
               sendPacket( NetInterface::GameInfoRequest, &p.address, p.key, p.session, flags );
               if ( !si->isQuerying() )
               {
                  si->status |= ServerInfo::Status_Querying;
                  gServerBrowserDirty = true;
               }
               i++;
            }
         }
         else
            i++;
      }
   }

   if ( gPingList.size() || gQueryList.size() || waitingForMaster )
   {
      // The LAN query function doesn't always want to schedule
      // the next ping.
      if (schedule)
         Sim::postEvent( Sim::getRootGroup(), new ProcessPingEvent( session ), Sim::getTargetTime() + 1 );
   }
   else
   {
      // All done!
      char msg[64];
      U32 foundCount = gServerList.size();
      if ( foundCount == 0 )
         dStrcpy( msg, "No servers found." );
      else if ( foundCount == 1 )
         dStrcpy( msg, "One server found." );
      else
         dSprintf( msg, sizeof( msg ), "%d servers found.", foundCount );

      Con::executef( "onServerQueryStatus", "done", msg, "1");
   }
}

//-----------------------------------------------------------------------------

static void processServerListPackets( U32 session )
{
   if ( session != gPingSession || !sgServerQueryActive )
      return;

   U32 currentTime = Platform::getVirtualMilliseconds();

   // Loop through the packet status list and resend packet requests where necessary:
   for ( U32 i = 0; i < gPacketStatusList.size(); i++ )
   {
      PacketStatus &p = gPacketStatusList[i];
      if ( p.time + gPacketTimeout < currentTime )
      {
         if ( !p.tryCount )
         {
            // Packet timed out :(
            Con::printf( "Server list packet #%d timed out.", p.index + 1 );
            gPacketStatusList.erase( i );
         }
         else
         {
            // Try again...
            Con::printf( "Rerequesting server list packet #%d...", p.index + 1 );
            p.tryCount--;
            p.time = currentTime;
            p.key = gKey++;

            BitStream *out = BitStream::getPacketStream();
            out->clearStringBuffer();
            out->write( U8( NetInterface::MasterServerListRequest ) );
            out->write( U8( sActiveFilter.queryFlags ) );   // flags
            out->write( ( session << 16) | ( p.key & 0xFFFF ) );
            out->write( p.index );  // packet index
            out->write( U8( 0 ) );  // game type
            out->write( U8( 0 ) );  // mission type
            out->write( U8( 0 ) );  // minPlayers
            out->write( U8( 0 ) );  // maxPlayers
            out->write( U32( 0 ) ); // region mask
            out->write( U32( 0 ) ); // version
            out->write( U8( 0 ) );  // filter flags
            out->write( U8( 0 ) );  // max bots
            out->write( U16( 0 ) ); // min CPU
            out->write( U8( 0 ) );  // buddy count

            BitStream::sendPacketStream(&gMasterServerQueryAddress);
         }
      }
   }

   if ( gPacketStatusList.size() )
      Sim::postEvent( Sim::getRootGroup(), new ProcessPacketEvent( session ), Sim::getCurrentTime() + 30 );
   else
      processPingsAndQueries( gPingSession );
}

//-----------------------------------------------------------------------------

static void processHeartbeat(U32 seq)
{
   if(seq != gHeartbeatSeq)
      return;
   sendHeartbeat( 0 );
   Sim::postEvent( Sim::getRootGroup(), new HeartbeatEvent(seq), Sim::getCurrentTime() + gHeartbeatInterval );
}

//-----------------------------------------------------------------------------

static void updatePingProgress()
{
   if ( !gPingList.size() )
   {
      updateQueryProgress();
      return;
   }

   char msg[64];
   U32 pingsLeft = countPingRequests();
   dSprintf( msg, sizeof(msg),
      (!pingsLeft && gPingList.size())?
         "Waiting for lan servers...":
         "Pinging servers: %d left...",
      pingsLeft );

   // Ping progress is 0 -> 0.5
   F32 progress = 0.0f;
   if ( gServerPingCount )
      progress = F32( gServerPingCount - pingsLeft ) / F32( gServerPingCount * 2 );

   //Con::errorf( ConsoleLogEntry::General, "Ping progress - %d of %d left - progress = %.2f", pingsLeft, gServerPingCount, progress );
   Con::executef( "onServerQueryStatus", "ping", msg, Con::getFloatArg( progress ) );
}

//-----------------------------------------------------------------------------

static void updateQueryProgress()
{
   if ( gPingList.size() )
      return;

   char msg[64];
   U32 queriesLeft = gQueryList.size();
   dSprintf( msg, sizeof( msg ), "Querying servers: %d left...", queriesLeft );

   // Query progress is 0.5 -> 1
   F32 progress = 0.5f;
   if ( gServerQueryCount )
      progress += ( F32( gServerQueryCount - queriesLeft ) / F32( gServerQueryCount * 2 ) );

   //Con::errorf( ConsoleLogEntry::General, "Query progress - %d of %d left - progress = %.2f", queriesLeft, gServerQueryCount, progress );
   Con::executef( "onServerQueryStatus", "query", msg, Con::getFloatArg( progress ) );
}


//-----------------------------------------------------------------------------
// Server packet handlers:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

static void handleMasterServerGameTypesResponse( BitStream* stream, U32 /*key*/, U8 /*flags*/ )
{
   Con::printf( "Received game type list from the master server." );

   U32 i;
   U8 temp;
   char stringBuf[256];
   stream->read( &temp );
   Con::executef("onClearGameTypes");
   for ( i = 0; i < U32( temp ); i++ )
   {
      readCString( stream, stringBuf );
      Con::executef("onAddGameType", stringBuf);
   }

   stream->read( &temp );
   Con::executef("onClearMissionTypes");
   for ( i = 0; i < U32( temp ); i++ )
   {
      readCString( stream, stringBuf );
      Con::executef("onAddMissionType", stringBuf);
   }
}

//-----------------------------------------------------------------------------

static void handleMasterServerListResponse( BitStream* stream, U32 key, U8 /*flags*/ )
{
   U8 packetIndex, packetTotal;
   U32 i;
   U16 serverCount, port;
   U8 netNum[4];
   char addressBuffer[256];
   NetAddress addr;

   stream->read( &packetIndex );
   // Validate the packet key:
   U32 packetKey = gMasterServerPing.key;
   if ( gGotFirstListPacket )
   {
      for ( i = 0; i < gPacketStatusList.size(); i++ )
      {
         if ( gPacketStatusList[i].index == packetIndex )
         {
            packetKey = gPacketStatusList[i].key;
            break;
         }
      }
   }

   U32 testKey = ( gPingSession << 16 ) | ( packetKey & 0xFFFF );
   if ( testKey != key )
      return;

   stream->read( &packetTotal );
   stream->read( &serverCount );

   Con::printf( "Received server list packet %d of %d from the master server (%d servers).", ( packetIndex + 1 ), packetTotal, serverCount );

   // Enter all of the servers in this packet into the ping list:
   for ( i = 0; i < serverCount; i++ )
   {
      stream->read( &netNum[0] );
      stream->read( &netNum[1] );
      stream->read( &netNum[2] );
      stream->read( &netNum[3] );
      stream->read( &port );

      dSprintf( addressBuffer, sizeof( addressBuffer ), "IP:%d.%d.%d.%d:%d", netNum[0], netNum[1], netNum[2], netNum[3], port );
      Net::stringToAddress( addressBuffer, &addr );
      pushPingRequest( &addr );
   }

   // If this is the first list packet we have received, fill the packet status list
   // and start processing:
   if ( !gGotFirstListPacket )
   {
      gGotFirstListPacket = true;
      gMasterServerQueryAddress = gMasterServerPing.address;
      U32 currentTime = Platform::getVirtualMilliseconds();
      for ( i = 0; i < packetTotal; i++ )
      {
         if ( i != packetIndex )
         {
            PacketStatus* p = new PacketStatus( i, gMasterServerPing.key, currentTime );
            gPacketStatusList.push_back( *p );
         }
      }

      processServerListPackets( gPingSession );
   }
   else
   {
      // Remove the packet we just received from the status list:
      for ( i = 0; i < gPacketStatusList.size(); i++ )
      {
         if ( gPacketStatusList[i].index == packetIndex )
         {
            gPacketStatusList.erase( i );
            break;
         }
      }
   }
}

//-----------------------------------------------------------------------------

static void handleGameMasterInfoRequest( const NetAddress* address, U32 key, U8 flags )
{
   if ( GNet->doesAllowConnections() )
   {
      U8 temp8;
      U32 temp32;

      char netString[256];
      Net::addressToString(address, netString);

      Vector<MasterInfo> *masterList = getMasterServerList();
      const NetAddress *masterAddr;
      bool fromMaster = false;
      for(U32 i = 0; i < masterList->size(); i++)
      {
         masterAddr = &(*masterList)[i].address;
         if (*(U32*)(masterAddr->netNum) == *(U32*)(address->netNum))
         {
            fromMaster = true;
            break;
         }
      }

      Con::printf( "Received info request from %s [%s].", fromMaster?"a master server":"a machine", netString);

      BitStream *out = BitStream::getPacketStream();
      out->clearStringBuffer();

      out->write( U8( NetInterface::GameMasterInfoResponse ) );
      out->write( U8( flags ) );
      out->write( key );

      writeCString( out, Con::getVariable( "Server::GameType" ) );
      writeCString( out, Con::getVariable( "Server::MissionType" ) );
      temp8 = U8( Con::getIntVariable( "pref::Server::MaxPlayers" ) );
      out->write( temp8 );
      temp32 = Con::getIntVariable( "pref::Server::RegionMask" );
      out->write( temp32 );
      temp32 = getVersionNumber();
      out->write( temp32 );
      temp8 = 0;
#if defined(TORQUE_OS_LINUX) || defined(TORQUE_OS_OPENBSD)
      temp8 |= ServerInfo::Status_Linux;
#endif
#if defined(TORQUE_OS_XENON)
      temp8 |= ServerInfo::Status_Xenon;
#endif
      if ( Con::getBoolVariable( "Server::Dedicated" ) )
         temp8 |= ServerInfo::Status_Dedicated;
      if ( dStrlen( Con::getVariable( "pref::Server::Password" ) ) > 0 )
         temp8 |= ServerInfo::Status_Passworded;
      out->write( temp8 );
      temp8 = U8( Con::getIntVariable( "Server::BotCount" ) );
      out->write( temp8 );
      out->write( Platform::SystemInfo.processor.mhz );

      U8 playerCount = U8( Con::getIntVariable( "Server::PlayerCount" ) );
      out->write( playerCount );

      const char* guidList = Con::getVariable( "Server::GuidList" );
      char* buf = new char[dStrlen( guidList ) + 1];
      dStrcpy( buf, guidList );
      char* temp = dStrtok( buf, "\t" );
      temp8 = 0;
      for ( ; temp && temp8 < playerCount; temp8++ )
      {
         out->write( U32( dAtoi( temp ) ) );
         temp = dStrtok( NULL, "\t" );
         temp8++;
      }

      for ( ; temp8 < playerCount; temp8++ )
         out->write( U32( 0 ) );

      delete [] buf;

      BitStream::sendPacketStream(address);
   }
}

//-----------------------------------------------------------------------------

static void handleGamePingRequest( const NetAddress* address, U32 key, U8 flags )
{
   // Do not respond if a mission is not running:
   if ( GNet->doesAllowConnections() )
   {
      // Do not respond if this is a single-player game:
      if ( dStricmp( Con::getVariable( "Server::ServerType" ), "SinglePlayer" ) == 0 )
         return;

      // Do not respond to offline queries if this is an online server:
      if (  flags & ServerFilter::OfflineQuery  )
         return;

      // some banning code here (?)

      BitStream *out = BitStream::getPacketStream();
      out->clearStringBuffer();

      out->write( U8( NetInterface::GamePingResponse ) );
      out->write( flags );
      out->write( key );
      if ( flags & ServerFilter::NoStringCompress )
         writeCString( out, versionString );
      else
         out->writeString( versionString );
      out->write( GameConnection::CurrentProtocolVersion );
      out->write( GameConnection::MinRequiredProtocolVersion );
      out->write( getVersionNumber() );

      // Enforce a 24-character limit on the server name:
      char serverName[25];
      dStrncpy( serverName, Con::getVariable( "pref::Server::Name" ), 24 );
      serverName[24] = 0;
      if ( flags & ServerFilter::NoStringCompress )
         writeCString( out, serverName );
      else
         out->writeString( serverName );

      BitStream::sendPacketStream(address);
   }
}

//-----------------------------------------------------------------------------

static void handleGamePingResponse( const NetAddress* address, BitStream* stream, U32 key, U8 /*flags*/ )
{
   // Broadcast has timed out or query has been cancelled:
   if( !gPingList.size() )
      return;

   S32 index = findPingEntry( gPingList, address );
   if( index == -1 )
   {
      // an anonymous ping response - if it's not already timed
      // out or finished, ping it.  Probably from a broadcast
      if( !addressFinished( address ) )
         pushPingRequest( address );
      return;
   }
   Ping &p = gPingList[index];
   U32 infoKey = ( p.session << 16 ) | ( p.key & 0xFFFF );
   if( infoKey != key )
      return;

   // Find if the server info already exists (favorite or refreshing):
   ServerInfo* si = findServerInfo( address );
   bool applyFilter = false;
   if ( sActiveFilter.type == ServerFilter::Normal )
      applyFilter = si ? !si->isUpdating() : true;

   char addrString[256];
   Net::addressToString( address, addrString );
   bool waitingForMaster = ( sActiveFilter.type == ServerFilter::Normal ) && !gGotFirstListPacket;

   // Verify the version:
   char buf[256];
   stream->readString( buf );
   if ( dStrcmp( buf, versionString ) != 0 )
   {
      // Version is different, so remove it from consideration:
      Con::printf( "Server %s is a different version.", addrString );
      Con::printf( "Wanted version %s, got version %s", versionString, buf);
      gFinishedList.push_back( *address );
      gPingList.erase( index );
      if ( si )
      {
         si->status = ServerInfo::Status_TimedOut;
         gServerBrowserDirty = true;
      }
      if ( !waitingForMaster )
         updatePingProgress();
      return;
   }

   // See if the server meets our minimum protocol:
   U32 temp32;
   stream->read( &temp32 );
   if ( temp32 < GameConnection::MinRequiredProtocolVersion )
   {
      Con::printf( "Protocol for server %s does not meet minimum protocol.", addrString );
      gFinishedList.push_back( *address );
      gPingList.erase( index );
      if ( si )
      {
         si->status = ServerInfo::Status_TimedOut;
         gServerBrowserDirty = true;
      }
      if ( !waitingForMaster )
         updatePingProgress();
      return;
   }

   // See if we meet the server's minimum protocol:
   stream->read( &temp32 );
   if ( GameConnection::CurrentProtocolVersion < temp32 )
   {
      Con::printf( "You do not meet the minimum protocol for server %s.", addrString );
      gFinishedList.push_back( *address );
      gPingList.erase( index );
      if ( si )
      {
         si->status = ServerInfo::Status_TimedOut;
         gServerBrowserDirty = true;
      }
      if ( !waitingForMaster )
         updatePingProgress();
      return;
   }

   // Calculate the ping:
   U32 time = Platform::getVirtualMilliseconds();
   U32 ping = ( time > p.time ) ? time - p.time : 0;

   // Check for max ping filter:
   if ( applyFilter && sActiveFilter.maxPing > 0 && ping > sActiveFilter.maxPing )
   {
      // Ping is too high, so remove this server from consideration:
      Con::printf( "Server %s filtered out by maximum ping.", addrString );
      gFinishedList.push_back( *address );
      gPingList.erase( index );
      if ( si )
         removeServerInfo( address );
      if ( !waitingForMaster )
         updatePingProgress();
      return;
   }

   // Get the server build version:
   stream->read( &temp32 );
   if ( applyFilter
     && ( sActiveFilter.filterFlags & ServerFilter::CurrentVersion )
     && ( temp32 != getVersionNumber() ) )
   {
      Con::printf( "Server %s filtered out by version number.", addrString );
      gFinishedList.push_back( *address );
      gPingList.erase( index );
      if ( si )
         removeServerInfo( address );
      if ( !waitingForMaster )
         updatePingProgress();
      return;
   }

   // OK, we can finally create the server info structure:
   if ( !si )
      si = findOrCreateServerInfo( address );
   si->ping = ping;
   si->version = temp32;

   // Get the server name:
   stream->readString( buf );
   if ( !si->name )
   {
      si->name = (char*) dMalloc( dStrlen( buf ) + 1 );
      dStrcpy( si->name, buf );
   }

   // Set the server up to be queried:
   gFinishedList.push_back( *address );
   p.key = 0;
   p.time = 0;
   p.tryCount = gQueryRetryCount;
   gQueryList.push_back( p );
   gServerQueryCount++;
   gPingList.erase( index );
   if ( !waitingForMaster )
      updatePingProgress();

   // Update the server browser gui!
   gServerBrowserDirty = true;
}

//-----------------------------------------------------------------------------

static void handleGameInfoRequest( const NetAddress* address, U32 key, U8 flags )
{
   // Do not respond unless there is a server running:
   if ( GNet->doesAllowConnections() )
   {
      // Do not respond to offline queries if this is an online server:
      if ( flags & ServerFilter::OfflineQuery )
         return;

      bool compressStrings = !( flags & ServerFilter::NoStringCompress );
      BitStream *out = BitStream::getPacketStream();
      out->clearStringBuffer();

      out->write( U8( NetInterface::GameInfoResponse ) );
      out->write( flags );
      out->write( key );

      if ( compressStrings ) {
         out->writeString( Con::getVariable( "Server::GameType" ) );
         out->writeString( Con::getVariable( "Server::MissionType" ) );
         out->writeString( Con::getVariable( "Server::MissionName" ) );
      }
      else {
         writeCString( out, Con::getVariable( "Server::GameType" ) );
         writeCString( out, Con::getVariable( "Server::MissionType" ) );
         writeCString( out, Con::getVariable( "Server::MissionName" ) );
      }

      U8 status = 0;
#if defined(TORQUE_OS_LINUX) || defined(TORQUE_OS_OPENBSD)
      status |= ServerInfo::Status_Linux;
#endif
#if defined(TORQUE_OS_XENON)
      status |= ServerInfo::Status_Xenon;
#endif
      if ( Con::getBoolVariable( "Server::Dedicated" ) )
         status |= ServerInfo::Status_Dedicated;
      if ( dStrlen( Con::getVariable( "pref::Server::Password" ) ) )
         status |= ServerInfo::Status_Passworded;
      out->write( status );

      out->write( U8( Con::getIntVariable( "Server::PlayerCount" ) ) );
      out->write( U8( Con::getIntVariable( "pref::Server::MaxPlayers" ) ) );
      out->write( U8( Con::getIntVariable( "Server::BotCount" ) ) );
      out->write( U16( Platform::SystemInfo.processor.mhz ) );
      if ( compressStrings )
         out->writeString( Con::getVariable( "pref::Server::Info" ) );
      else
         writeCString( out, Con::getVariable( "pref::Server::Info" ) );
      writeLongCString( out, Con::evaluate( "onServerInfoQuery();" ) );

      BitStream::sendPacketStream(address);
   }
}

//-----------------------------------------------------------------------------

static void handleGameInfoResponse( const NetAddress* address, BitStream* stream, U32 /*key*/, U8 /*flags*/ )
{
   if ( !gQueryList.size() )
      return;

   S32 index = findPingEntry( gQueryList, address );
   if ( index == -1 )
      return;

   // Remove the server from the query list since it has been so kind as to respond:
   gQueryList.erase( index );
   updateQueryProgress();
   ServerInfo *si = findServerInfo( address );
   if ( !si )
      return;

   bool isUpdate = si->isUpdating();
   bool applyFilter = !isUpdate && ( sActiveFilter.type == ServerFilter::Normal );
   char addrString[256];
   Net::addressToString( address, addrString );

   // Get the rules set:
   char stringBuf[2048];   // Who knows how big this should be?
   stream->readString( stringBuf );
   if ( !si->gameType || dStricmp( si->gameType, stringBuf ) != 0 )
   {
      si->gameType = (char*) dRealloc( (void*) si->gameType, dStrlen( stringBuf ) + 1 );
      dStrcpy( si->gameType, stringBuf );

      // Test against the active filter:
      if ( applyFilter && dStricmp( sActiveFilter.gameType, "any" ) != 0
        && dStricmp( si->gameType, sActiveFilter.gameType ) != 0 )
      {
         Con::printf( "Server %s filtered out by rules set. (%s:%s)", addrString, sActiveFilter.gameType, si->gameType );
         removeServerInfo( address );
         return;
      }
   }

   // Get the mission type:
   stream->readString( stringBuf );
   if ( !si->missionType || dStrcmp( si->missionType, stringBuf ) != 0 )
   {
      si->missionType = (char*) dRealloc( (void*) si->missionType, dStrlen( stringBuf ) + 1 );
      dStrcpy( si->missionType, stringBuf );

      // Test against the active filter:
      if ( applyFilter && dStricmp( sActiveFilter.missionType, "any" ) != 0
        && dStricmp( si->missionType, sActiveFilter.missionType ) != 0 )
      {
         Con::printf( "Server %s filtered out by mission type. (%s:%s)", addrString, sActiveFilter.missionType, si->missionType );
         removeServerInfo( address );
         return;
      }
   }

   // Get the mission name:
   stream->readString( stringBuf );
   // Clip the file extension off:
   char* temp = dStrstr( static_cast<char*>( stringBuf ), const_cast<char*>( ".mis" ) );
   if ( temp )
      *temp = '\0';
   if ( !si->missionName || dStrcmp( si->missionName, stringBuf ) != 0 )
   {
      si->missionName = (char*) dRealloc( (void*) si->missionName, dStrlen( stringBuf ) + 1 );
      dStrcpy( si->missionName, stringBuf );
   }

   // Get the server status:
   U8 temp_U8;
   stream->read( &temp_U8 );
   si->status = temp_U8;

   // Filter by the flags:
   if ( applyFilter )
   {
      if ( sActiveFilter.filterFlags & ServerFilter::Dedicated && !si->isDedicated() )
      {
         Con::printf( "Server %s filtered out by dedicated flag.", addrString );
         removeServerInfo( address );
         return;
      }

      if ( sActiveFilter.filterFlags & ServerFilter::NotPassworded && si->isPassworded() )
      {
         Con::printf( "Server %s filtered out by no-password flag.", addrString );
         removeServerInfo( address );
         return;
      }

      if ( sActiveFilter.filterFlags & ServerFilter::NotXenon && si->isXenon() )
      {
         Con::printf( "Server %s filtered out by no-xenon flag.", addrString );
         removeServerInfo( address );
         return;
      }
   }
   si->status.set( ServerInfo::Status_Responded );

   // Get the player count:
   stream->read( &si->numPlayers );

   // Test player count against active filter:
   if ( applyFilter && ( si->numPlayers < sActiveFilter.minPlayers || si->numPlayers > sActiveFilter.maxPlayers ) )
   {
      Con::printf( "Server %s filtered out by player count.", addrString );
      removeServerInfo( address );
      return;
   }

   // Get the max players and bot count:
   stream->read( &si->maxPlayers );
   stream->read( &si->numBots );

   // Test bot count against active filter:
   if ( applyFilter && ( si->numBots > sActiveFilter.maxBots ) )
   {
      Con::printf( "Server %s filtered out by maximum bot count.", addrString );
      removeServerInfo( address );
      return;
   }

   // Get the CPU speed;
   U16 temp_U16;
   stream->read( &temp_U16 );
   si->cpuSpeed = temp_U16;

   // Test CPU speed against active filter:
   if ( applyFilter && ( si->cpuSpeed < sActiveFilter.minCPU ) )
   {
      Con::printf( "Server %s filtered out by minimum CPU speed.", addrString );
      removeServerInfo( address );
      return;
   }

   // Get the server info:
   stream->readString( stringBuf );
   if ( !si->statusString || ( isUpdate && dStrcmp( si->statusString, stringBuf ) != 0 ) )
   {
      si->infoString = (char*) dRealloc( (void*) si->infoString, dStrlen( stringBuf ) + 1 );
      dStrcpy( si->infoString, stringBuf );
   }

   // Get the content string:
   readLongCString( stream, stringBuf );
   if ( !si->statusString || ( isUpdate && dStrcmp( si->statusString, stringBuf ) != 0 ) )
   {
      si->statusString = (char*) dRealloc( (void*) si->statusString, dStrlen( stringBuf ) + 1 );
      dStrcpy( si->statusString, stringBuf );
   }

   // Update the server browser gui!
   gServerBrowserDirty = true;
}

//-----------------------------------------------------------------------------
// Packet Dispatch

void DemoNetInterface::handleInfoPacket( const NetAddress* address, U8 packetType, BitStream* stream )
{
   U8 flags;
   U32 key;

   stream->read( &flags );
   stream->read( &key );
   stream->clearStringBuffer();

   switch( packetType )
   {
      case GamePingRequest:
         handleGamePingRequest( address, key, flags );
         break;

      case GamePingResponse:
         handleGamePingResponse( address, stream, key, flags );
         break;

      case GameInfoRequest:
         handleGameInfoRequest( address, key, flags );
         break;

      case GameInfoResponse:
         handleGameInfoResponse( address, stream, key, flags );
         break;

      case MasterServerGameTypesResponse:
         handleMasterServerGameTypesResponse( stream, key, flags );
         break;

      case MasterServerListResponse:
         handleMasterServerListResponse( stream, key, flags );
         break;

      case GameMasterInfoRequest:
         handleGameMasterInfoRequest( address, key, flags );
         break;
   }
}


ConsoleFunctionGroupEnd( ServerQuery );
