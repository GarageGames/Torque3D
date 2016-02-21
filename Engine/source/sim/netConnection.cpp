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

#include "platform/platform.h"
#include "core/dnet.h"
#include "console/simBase.h"
#include "sim/netConnection.h"
#include "core/stream/bitStream.h"
#include "core/stream/fileStream.h"
#ifndef TORQUE_TGB_ONLY
#include "scene/pathManager.h"
#endif
#include "console/consoleTypes.h"
#include "sim/netInterface.h"
#include "console/engineAPI.h"
#include <stdarg.h>


IMPLEMENT_SCOPE( NetAPI, Net,, "Networking functionality." );

IMPLEMENT_NONINSTANTIABLE_CLASS( NetEvent,
   "An event to be sent over the network." )
END_IMPLEMENT_CLASS;


S32 gNetBitsSent = 0;
extern S32 gNetBitsReceived;
U32 gGhostUpdates = 0;

enum NetConnectionConstants {
   PingTimeout = 4500, ///< milliseconds
   DefaultPingRetryCount = 15,
};

SimObjectPtr<NetConnection> NetConnection::mServerConnection;
SimObjectPtr<NetConnection> NetConnection::mLocalClientConnection;

//----------------------------------------------------------------------
/// ConnectionMessageEvent
///
/// This event is used inside by the connection and subclasses to message
/// itself when sequencing events occur.  Right now, the message event
/// only uses 6 bits to transmit the message, so
class ConnectionMessageEvent : public NetEvent
{
   U32 sequence;
   U32 message;
   U32 ghostCount;
public:
   typedef NetEvent Parent;
   ConnectionMessageEvent(U32 msg=0, U32 seq=0, U32 gc=0)
      { message = msg; sequence = seq; ghostCount = gc;}
   void pack(NetConnection *, BitStream *bstream)
   {
      bstream->write(sequence);
      bstream->writeInt(message, 3);
      bstream->writeInt(ghostCount, NetConnection::GhostIdBitSize + 1);
   }
   void write(NetConnection *, BitStream *bstream)
   {
      bstream->write(sequence);
      bstream->writeInt(message, 3);
      bstream->writeInt(ghostCount, NetConnection::GhostIdBitSize + 1);
   }
   void unpack(NetConnection *, BitStream *bstream)
   {
      bstream->read(&sequence);
      message = bstream->readInt(3);
      ghostCount = bstream->readInt(NetConnection::GhostIdBitSize + 1);
   }
   void process(NetConnection *ps)
   {
      ps->handleConnectionMessage(message, sequence, ghostCount);
   }
   DECLARE_CONOBJECT(ConnectionMessageEvent);
};

IMPLEMENT_CO_NETEVENT_V1(ConnectionMessageEvent);

ConsoleDocClass( ConnectionMessageEvent,
				"@brief This event is used inside by the connection and subclasses to message itself when sequencing events occur.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

void NetConnection::sendConnectionMessage(U32 message, U32 sequence, U32 ghostCount)
{
   postNetEvent(new ConnectionMessageEvent(message, sequence, ghostCount));
}

//--------------------------------------------------------------------
IMPLEMENT_CONOBJECT(NetConnection);

ConsoleDocClass( NetConnection,
   "@brief Provides the basis for implementing a multiplayer game protocol.\n\n"

   "NetConnection combines a low-level notify protocol implemented in ConnectionProtocol with a SimGroup, "
   "and implements several distinct subsystems:\n\n"

   "- <b>Event Manager</b>  This is responsible for transmitting NetEvents over the wire.  "
   "It deals with ensuring that the various types of NetEvents are delivered appropriately, "
   "and with notifying the event of its delivery status.\n\n"
   "- <b>Move Manager</b>  This is responsible for transferring a Move to the server 32 "
   "times a second (on the client) and applying it to the control object (on the server).\n\n"
   "- <b>Ghost Manager</b>  This is responsible for doing scoping calculations (on the server "
   "side) and transmitting most-recent ghost information to the client.\n\n"
   "- <b>File Transfer</b>  It is often the case that clients will lack important files when "
   "connecting to a server which is running a mod or new map. This subsystem allows the "
   "server to transfer such files to the client.\n\n"
   "- <b>Networked String Table</b>  String data can easily soak up network bandwidth, so for "
   "efficiency, we implement a networked string table. We can then notify the connection "
   "of strings we will reference often, such as player names, and transmit only a tag, "
   "instead of the whole string.\n\n"
   "- <b>Demo Recording</b>  A demo in Torque is a log of the network traffic between client "
   "and server; when a NetConnection records a demo, it simply logs this data to a file. When "
   "it plays a demo back, it replays the logged data.\n\n"
   "- <b>Connection Database</b>  This is used to keep track of all the NetConnections; it can "
   "be iterated over (for instance, to send an event to all active connections), or queried "
   "by address.\n\n"

   "The NetConnection is a SimGroup. On the client side, it contains all the objects which have been "
   "ghosted to that client. On the server side, it is empty; it can be used (typically in script) to "
   "hold objects related to the connection. For instance, you might place an observation camera in the "
   "NetConnnection. In both cases, when the connection is destroyed, so are the contained objects.\n\n"

   "The NetConnection also has the concept of local connections.  These are used when the client and "
   "server reside in the same process.  A local connection is typically required to use the standard "
   "Torque world building tools.  A local connection is also required when building a single player "
   "game.\n\n"

   "@see @ref Networking, @ref ghosting_scoping, @ref netconnection_simgroup, @ref local_connections, GameConnection, AIConnection, and AIClient.\n\n"

   "@ingroup Networking\n");

NetConnection* NetConnection::mConnectionList = NULL;
NetConnection* NetConnection::mHashTable[NetConnection::HashTableSize] = { NULL, };

bool NetConnection::mFilesWereDownloaded = false;

static inline U32 HashNetAddress(const NetAddress *addr)
{
   return *((U32 *)addr->netNum) % NetConnection::HashTableSize;
}

NetConnection *NetConnection::lookup(const NetAddress *addr)
{
   U32 hashIndex = HashNetAddress(addr);
   for(NetConnection *walk = mHashTable[hashIndex]; walk; walk = walk->mNextTableHash)
      if(Net::compareAddresses(addr, walk->getNetAddress()))
         return walk;
   return NULL;
}

void NetConnection::netAddressTableInsert()
{
   U32 hashIndex = HashNetAddress(&mNetAddress);
   mNextTableHash = mHashTable[hashIndex];
   mHashTable[hashIndex] = this;
}

void NetConnection::netAddressTableRemove()
{
   U32 hashIndex = HashNetAddress(&mNetAddress);
   NetConnection **walk = &mHashTable[hashIndex];
   while(*walk)
   {
      if(*walk == this)
      {
         *walk = mNextTableHash;
         mNextTableHash = NULL;
         return;
      }
      walk = &((*walk)->mNextTableHash);
   }
}

void NetConnection::setNetAddress(const NetAddress *addr)
{
   mNetAddress = *addr;
}

const NetAddress *NetConnection::getNetAddress()
{
   return &mNetAddress;
}

void NetConnection::setSequence(U32 sequence)
{
   mConnectSequence = sequence;
}

U32 NetConnection::getSequence()
{
   return mConnectSequence;
}

static U32 gPacketRateToServer = 32;
static U32 gPacketUpdateDelayToServer = 32;
static U32 gPacketRateToClient = 10;
static U32 gPacketSize = 508;

void NetConnection::consoleInit()
{
   Con::addVariable("$pref::Net::PacketRateToServer", TypeS32, &gPacketRateToServer,
      "@brief Sets how often packets are sent from the client to the server.\n\n"

      "It is possible to control how often packets may be sent to the server.  This may be "
      "used to throttle the amount of bandwidth being used, but should be adjusted with "
      "caution.\n\n"

      "The actual formula used to calculate the delay between sending packets to the server is:\n"

      "@code\n"
      "Packet Update Delay To Server = 1024 / $pref::Net::PacketRateToServer"
      "@endcode\n"

      "with the result in ms.  A minimum rate of 8 is enforced in the source code.  The default "
      "value is 32.\n\n"

      "@note When using a local connection (@ref local_connections) be aware that this variable "
      "is always forced to 128.\n\n"

      "@ingroup Networking");

   Con::addVariable("$pref::Net::PacketRateToClient", TypeS32, &gPacketRateToClient,
      "@brief Sets how often packets are sent from the server to a client.\n\n"

      "It is possible to control how often packets may be sent to the clients.  This may be "
      "used to throttle the amount of bandwidth being used, but should be adjusted with "
      "caution.\n\n"

      "The actual formula used to calculate the delay between sending packets to a client is:\n"

      "@code\n"
      "Packet Update Delay To Client = 1024 / $pref::Net::PacketRateToClient"
      "@endcode\n"

      "with the result in ms.  A minimum rate of 1 is enforced in the source code.  The default "
      "value is 10.\n\n"

      "@note When using a local connection (@ref local_connections) be aware that this variable "
      "is always forced to 128.\n\n"

      "@ingroup Networking");

   Con::addVariable("$pref::Net::PacketSize", TypeS32, &gPacketSize,
      "@brief Sets the maximum size in bytes an individual network packet may be.\n\n"

      "It is possible to control how large each individual network packet may be.  Increasing "
      "its size from the default allows for more data to be sent on each network send.  "
      "However, this value should be changed with caution as too large a value will cause "
      "packets to be split up by the networking platform or hardware, which is something "
      "Torque cannot handle.\n\n"

      "A minimum packet size of 100 bytes is enforced in the source code.  There is no "
      "enforced maximum.  The default value is 200 bytes.\n\n"

      "@note When using a local connection (@ref local_connections) be aware that this variable "
      "is always forced to 1024 bytes.\n\n"

      "@ingroup Networking");

   Con::addVariable("$Stats::netBitsSent", TypeS32, &gNetBitsSent,
      "@brief The number of bytes sent during the last packet send operation.\n\n"

      "@note Even though this variable has 'Bits' in it, the value is indeed reported in bytes.  This name "
      "is a legacy holdover and remains for compatibility reasons.\n"

      "@ingroup Networking");

   Con::addVariable("$Stats::netBitsReceived", TypeS32, &gNetBitsReceived,
      "@brief The number of bytes received during the last packet process operation.\n\n"

      "@note Even though this variable has 'Bits' in it, the value is indeed reported in bytes.  This name "
      "is a legacy holdover and remains for compatibility reasons.\n"

      "@ingroup Networking");

   Con::addVariable("$Stats::netGhostUpdates", TypeS32, &gGhostUpdates,
      "@brief The total number of ghosts added, removed, and/or updated on the client "
      "during the last packet process operation.\n\n"

      "@ingroup Networking");
}

void NetConnection::checkMaxRate()
{
   // Enforce some minimum limits to the network settings.
   gPacketRateToServer = getMax( gPacketRateToServer, (U32)8 );
   gPacketRateToClient = getMax( gPacketRateToClient, (U32)1 );
   gPacketSize = getMax( gPacketSize, (U32)100 );

   U32 packetRateToServer = gPacketRateToServer;
   U32 packetRateToClient = gPacketRateToClient;
   U32 packetSize = gPacketSize;

   if (isLocalConnection())
   {
      packetRateToServer = 128;
      packetRateToClient = 128;
      packetSize = 1024;
   }

   gPacketUpdateDelayToServer = 1024 / packetRateToServer;
   U32 toClientUpdateDelay = 1024 / packetRateToClient;

   if(mMaxRate.updateDelay != toClientUpdateDelay || mMaxRate.packetSize != packetSize)
   {
      mMaxRate.updateDelay = toClientUpdateDelay;
      mMaxRate.packetSize = packetSize;
      mMaxRate.changed = true;
   }
}

void NetConnection::setSendingEvents(bool sending)
{
   AssertFatal(!mEstablished, "Error, cannot change event behavior after a connection has been established.");
   mSendingEvents = sending;
}

void NetConnection::setTranslatesStrings(bool xl)
{
   AssertFatal(!mEstablished, "Error, cannot change event behavior after a connection has been established.");
   mTranslateStrings = xl;
   if(mTranslateStrings)
      mStringTable = new ConnectionStringTable(this);
}

void NetConnection::setNetClassGroup(U32 grp)
{
   AssertFatal(!mEstablished, "Error, cannot change net class group after a connection has been established.");
   mNetClassGroup = grp;
}

NetConnection::NetConnection()
 : mNetAddress()
{
   mTranslateStrings = false;
   mConnectSequence = 0;

   mStringTable = NULL;
   mSendingEvents = true;
   mNetClassGroup = NetClassGroupGame;
   AssertFatal(mNetClassGroup >= NetClassGroupGame && mNetClassGroup < NetClassGroupsCount,
            "Invalid net event class type.");

   mSimulatedPing = 0;
   mSimulatedPacketLoss = 0;
#ifdef TORQUE_DEBUG_NET
   mLogging = false;
#endif
   mEstablished = false;
   mLastUpdateTime = 0;
   mRoundTripTime = 0;
   mPacketLoss = 0;
   mNextTableHash = NULL;
   mSendDelayCredit = 0;
   mConnectionState = NotConnected;

   mCurrentDownloadingFile = NULL;
   mCurrentFileBuffer = NULL;

   mNextConnection = NULL;
   mPrevConnection = NULL;

   mNotifyQueueHead = NULL;
   mNotifyQueueTail = NULL;

   mCurRate.updateDelay = 102;
   mCurRate.packetSize = 200;
   mCurRate.changed = false;
   mMaxRate.updateDelay = 102;
   mMaxRate.packetSize = 200;
   mMaxRate.changed = false;
   checkMaxRate();

   // event management data:

   mNotifyEventList = NULL;
   mSendEventQueueHead = NULL;
   mSendEventQueueTail = NULL;
   mUnorderedSendEventQueueHead = NULL;
   mUnorderedSendEventQueueTail = NULL;
   mWaitSeqEvents = NULL;

   mNextSendEventSeq = FirstValidSendEventSeq;
   mNextRecvEventSeq = FirstValidSendEventSeq;
   mLastAckedEventSeq = -1;

   // ghost management data:

   mScopeObject = NULL;
   mGhostingSequence = 0;
   mGhosting = false;
   mScoping = false;
   mGhostArray = NULL;
   mGhostRefs = NULL;
   mGhostLookupTable = NULL;
   mLocalGhosts = NULL;

   mGhostsActive = 0;

   mMissionPathsSent = false;
   mDemoWriteStream = NULL;
   mDemoReadStream = NULL;

   mPingSendCount = 0;
   mPingRetryCount = DefaultPingRetryCount;
   mLastPingSendTime = Platform::getVirtualMilliseconds();

   mCurrentDownloadingFile = NULL;
   mCurrentFileBuffer = NULL;
   mCurrentFileBufferSize = 0;
   mCurrentFileBufferOffset = 0;
   mNumDownloadedFiles = 0;

   // Disable starting a new journal recording or playback from here on
   Journal::Disable();

   // Ensure NetAddress is cleared
   dMemset(&mNetAddress, '\0', sizeof(NetAddress));
}

NetConnection::~NetConnection()
{
   AssertFatal(mNotifyQueueHead == NULL, "Uncleared notifies remain.");
   netAddressTableRemove();

   dFree(mCurrentFileBuffer);
   if(mCurrentDownloadingFile)
      delete mCurrentDownloadingFile;

   delete[] mLocalGhosts;
   delete[] mGhostLookupTable;
   delete[] mGhostRefs;
   delete[] mGhostArray;
   delete mStringTable;
   if(mDemoWriteStream)
      delete mDemoWriteStream;
   if(mDemoReadStream)
      delete mDemoReadStream;
}

NetConnection::PacketNotify::PacketNotify()
{
   rateChanged = false;
   maxRateChanged = false;
   sendTime = 0;
   eventList = 0;
   ghostList = 0;
}

bool NetConnection::checkTimeout(U32 time)
{
   if(!isNetworkConnection())
      return false;

   if(time > mLastPingSendTime + PingTimeout)
   {
      if(mPingSendCount >= mPingRetryCount)
         return true;
      mLastPingSendTime = time;
      mPingSendCount++;
      sendPingPacket();
   }
   return false;
}

void NetConnection::keepAlive()
{
   mLastPingSendTime = Platform::getVirtualMilliseconds();
   mPingSendCount = 0;
}

void NetConnection::handleConnectionEstablished()
{
}

//--------------------------------------------------------------------------
#ifndef TORQUE_TGB_ONLY
DefineEngineMethod( NetConnection, transmitPaths, void, (),,
   "@brief Sent by the server during phase 2 of the mission download to update motion spline paths.\n\n"

   "The server transmits all spline motion paths that are within the mission (Path) separate from "
   "other objects.  This is due to the potentially large number of nodes within each path, which may "
   "saturate a packet sent to the client.  By managing this step separately, Torque has finer control "
   "over how packets are organised vs. doing it during the ghosting stage.\n\n"

   "Internally a PathManager is used to track all paths defined within a mission on the server, and each "
   "one is transmitted using a PathManagerEvent.  The client side collects these events and builds the "
   "given paths within its own PathManager.  This is typically done during the standard mission start "
   "phase 2 when following Torque's example mission startup sequence.\n\n"

   "When a mission is ended, all paths need to be cleared from their respective path managers."

   "@tsexample\n"
   "function serverCmdMissionStartPhase2Ack(%client, %seq, %playerDB)\n"
   "{\n"
   "   // Make sure to ignore calls from a previous mission load\n"
   "   if (%seq != $missionSequence || !$MissionRunning)\n"
   "      return;\n"
   "   if (%client.currentPhase != 1.5)\n"
   "      return;\n"
   "   %client.currentPhase = 2;\n"
   "\n"
   "   // Set the player datablock choice\n"
   "   %client.playerDB = %playerDB;\n"
   "\n"
   "   // Update mission paths (SimPath), this needs to get there before the objects.\n"
   "   %client.transmitPaths();\n"
   "\n"
   "   // Start ghosting objects to the client\n"
   "   %client.activateGhosting();\n"
   "}\n"
   "@endtsexample\n"
   
   "@see clearPaths()\n"
   "@see Path\n")
{
   gServerPathManager->transmitPaths(object);
   object->setMissionPathsSent(true);
}

DefineEngineMethod( NetConnection, clearPaths, void, (),,
   "@brief On the server, resets the connection to indicate that motion spline paths have not been transmitted.\n\n"

   "Typically when a mission has ended on the server, all connected clients are informed of this change "
   "and their connections are reset back to a starting state.  This method resets a connection on the "
   "server to indicate that motion spline paths have not been transmitted.\n\n"

   "@tsexample\n"
   "   // Inform the clients\n"
   "   for (%clientIndex = 0; %clientIndex < ClientGroup.getCount(); %clientIndex++)\n"
   "   {\n"
   "      // clear ghosts and paths from all clients\n"
   "      %cl = ClientGroup.getObject(%clientIndex);\n"
   "      %cl.endMission();\n"
   "      %cl.resetGhosting();\n"
   "      %cl.clearPaths();\n"
   "   }\n"
   "@endtsexample\n"
   
   "@see transmitPaths()\n"
   "@see Path\n")
{
   object->setMissionPathsSent(false);
}
#endif

DefineEngineMethod( NetConnection, getAddress, const char *, (),,
   "@brief Returns the far end network address for the connection.\n\n"

   "The address will be in one of the following forms:\n"
   "- <b>IP:Broadcast:&lt;port&gt;</b> for broadcast type addresses\n"
   "- <b>IP:&lt;address&gt;:&lt;port&gt;</b> for IP addresses\n"
   "- <b>local</b> when connected locally (server and client running in same process\n")
{
   if(object->isLocalConnection())
      return "local";
   char *buffer = Con::getReturnBuffer(256);
   Net::addressToString(object->getNetAddress(), buffer);
   return buffer;
}

DefineEngineMethod( NetConnection, setSimulatedNetParams, void, (F32 packetLoss, S32 delay),,
   "@brief Simulate network issues on the connection for testing.\n\n"

   "@param packetLoss The fraction of packets that will be lost.  Ranges from 0.0 (no loss) to 1.0 (complete loss)\n"
   "@param delay Delays packets being transmitted by simulating a particular ping.  This is an absolute "
   "integer, measured in ms.\n")
{
   object->setSimulatedNetParams(packetLoss, delay);
}

DefineEngineMethod( NetConnection, getPing, S32, (),,
   "@brief Returns the average round trip time (in ms) for the connection.\n\n"

   "The round trip time is recalculated every time a notify packet is received.  Notify "
   "packets are used to information the connection that the far end successfully received "
   "the sent packet.\n")
{
   return( S32( object->getRoundTripTime() ) );
}

DefineEngineMethod( NetConnection, getPacketLoss, S32, (),,
   "@brief Returns the percentage of packets lost per tick.\n\n"

   "@note This method is not yet hooked up.\n")
{
   return( S32( 100 * object->getPacketLoss() ) );
}

DefineEngineMethod( NetConnection, checkMaxRate, void, (),,
   "@brief Ensures that all configured packet rates and sizes meet minimum requirements.\n\n"

   "This method is normally only called when a NetConnection class is first constructed.  It need "
   "only be manually called if the global variables that set the packet rate or size have changed.\n\n"

   "@note If @$pref::Net::PacketRateToServer, @$pref::Net::PacketRateToClient or @$pref::Net::PacketSize "
   "have been changed since a NetConnection has been created, this method must be called on "
   "all connections for them to follow the new rates or size.\n")
{
   object->checkMaxRate();
}

#ifdef TORQUE_DEBUG_NET

DefineEngineMethod( NetConnection, setLogging, void, (bool state),,
   "@brief Sets if debug statements should be written to the console log.\n\n"
   "@note Only valid if the executable has been compiled with TORQUE_DEBUG_NET.\n")
{
   object->setLogging(state);
}

#endif

//--------------------------------------------------------------------

void NetConnection::setEstablished()
{
   AssertFatal(!mEstablished, "NetConnection::setEstablished - Error, this NetConnection has already been established.");

   mEstablished = true;
   mNextConnection = mConnectionList;
   if(mConnectionList)
      mConnectionList->mPrevConnection = this;
   mConnectionList = this;

   if(isNetworkConnection())
      netAddressTableInsert();

}

void NetConnection::onRemove()
{
   // delete any ghosts that may exist for this connection, but aren't added
   while(mGhostAlwaysSaveList.size())
   {
      delete mGhostAlwaysSaveList[0].ghost;
      mGhostAlwaysSaveList.pop_front();
   }
   if(mNextConnection)
      mNextConnection->mPrevConnection = mPrevConnection;
   if(mPrevConnection)
      mPrevConnection->mNextConnection = mNextConnection;
   if(mConnectionList == this)
      mConnectionList = mNextConnection;
   while(mNotifyQueueHead)
      handleNotify(false);

   ghostOnRemove();
   eventOnRemove();

   Parent::onRemove();
}

String NetConnection::mErrorBuffer;

void NetConnection::setLastError(const char *fmt, ...)
{
   va_list argptr;
   va_start(argptr, fmt);
   mErrorBuffer = String::ToString(fmt, argptr);
   va_end(argptr);

#ifdef TORQUE_DEBUG_NET
   // setLastErrors assert in net_debug builds
   AssertFatal(false, mErrorBuffer.c_str());
#endif

}

//--------------------------------------------------------------------

void NetConnection::handleNotify(bool recvd)
{
//   Con::printf("NET  %d: NOTIFY - %d %s", getId(), gPacketId, recvd ? "RECVD" : "DROPPED");

   PacketNotify *note = mNotifyQueueHead;
   AssertFatal(note != NULL, "Error: got a notify with a null notify head.");
   mNotifyQueueHead = mNotifyQueueHead->nextPacket;

   if(note->rateChanged && !recvd)
      mCurRate.changed = true;
   if(note->maxRateChanged && !recvd)
      mMaxRate.changed = true;

   if(recvd) 
   {
      // Running average of roundTrip time
      U32 curTime = Platform::getVirtualMilliseconds();
      mRoundTripTime = (mRoundTripTime + (curTime - note->sendTime)) * 0.5;
      packetReceived(note);
   }
   else
      packetDropped(note);

   delete note;
}

void NetConnection::processRawPacket(BitStream *bstream)
{
   if(mDemoWriteStream)
      recordBlock(BlockTypePacket, bstream->getReadByteSize(), bstream->getBuffer());

   ConnectionProtocol::processRawPacket(bstream);
}

void NetConnection::handlePacket(BitStream *bstream)
{
//   Con::printf("NET  %d: RECV - %d", getId(), mLastSeqRecvd);
   // clear out any errors

   mErrorBuffer = String();

   if(bstream->readFlag())
   {
      mCurRate.updateDelay = bstream->readInt(12);
      mCurRate.packetSize = bstream->readInt(12);
   }

   if(bstream->readFlag())
   {
      U32 omaxDelay = bstream->readInt(12);
      S32 omaxSize = bstream->readInt(12);
      if(omaxDelay < mMaxRate.updateDelay)
         omaxDelay = mMaxRate.updateDelay;
      if(omaxSize > mMaxRate.packetSize)
         omaxSize = mMaxRate.packetSize;
      if(omaxDelay != mCurRate.updateDelay || omaxSize != mCurRate.packetSize)
      {
         mCurRate.updateDelay = omaxDelay;
         mCurRate.packetSize = omaxSize;
         mCurRate.changed = true;
      }
   }
   readPacket(bstream);

   if(mErrorBuffer.isNotEmpty())
      connectionError(mErrorBuffer);
}

void NetConnection::connectionError(const char *errorString)
{
   TORQUE_UNUSED(errorString);
}

//--------------------------------------------------------------------

NetConnection::PacketNotify *NetConnection::allocNotify()
{
   return new PacketNotify;
}

/// Used when simulating lag.
///
/// We post this SimEvent when we want to send a packet; it delays for a bit, then
/// sends the actual packet.
class NetDelayEvent : public SimEvent
{
   U8 buffer[Net::MaxPacketDataSize];
   BitStream stream;
public:
   NetDelayEvent(BitStream *inStream) : stream(NULL, 0)
   {
      dMemcpy(buffer, inStream->getBuffer(), inStream->getPosition());
      stream.setBuffer(buffer, inStream->getPosition());
      stream.setPosition(inStream->getPosition());
   }
   void process(SimObject *object)
   {
      ((NetConnection *) object)->sendPacket(&stream);
   }
};

void NetConnection::checkPacketSend(bool force)
{
   U32 curTime = Platform::getVirtualMilliseconds();
   U32 delay = isConnectionToServer() ? gPacketUpdateDelayToServer : mCurRate.updateDelay;

   if(!force)
   {
      if(curTime < mLastUpdateTime + delay - mSendDelayCredit)
         return;

      mSendDelayCredit = curTime - (mLastUpdateTime + delay - mSendDelayCredit);
      if(mSendDelayCredit > 1000)
         mSendDelayCredit = 1000;

      if(mDemoWriteStream)
         recordBlock(BlockTypeSendPacket, 0, 0);
   }
   if(windowFull())
      return;

   BitStream *stream = BitStream::getPacketStream(mCurRate.packetSize);
   buildSendPacketHeader(stream);

   mLastUpdateTime = curTime;

   PacketNotify *note = allocNotify();
   if(!mNotifyQueueHead)
      mNotifyQueueHead = note;
   else
      mNotifyQueueTail->nextPacket = note;
   mNotifyQueueTail = note;
   note->nextPacket = NULL;
   note->sendTime = curTime;

   note->rateChanged = mCurRate.changed;
   note->maxRateChanged = mMaxRate.changed;

   if(stream->writeFlag(mCurRate.changed))
   {
      stream->writeInt(mCurRate.updateDelay, 12);
      stream->writeInt(mCurRate.packetSize, 12);
      mCurRate.changed = false;
   }
   if(stream->writeFlag(mMaxRate.changed))
   {
      stream->writeInt(mMaxRate.updateDelay, 12);
      stream->writeInt(mMaxRate.packetSize, 12);
      mMaxRate.changed = false;
   }
#ifdef TORQUE_DEBUG_NET
   U32 start = stream->getCurPos();
#endif

   DEBUG_LOG(("PKLOG %d START", getId()) );
   writePacket(stream, note);
   DEBUG_LOG(("PKLOG %d END - %d", getId(), stream->getCurPos() - start) );
   if(mSimulatedPacketLoss && Platform::getRandom() < mSimulatedPacketLoss)
   {
      //Con::printf("NET  %d: SENDDROP - %d", getId(), mLastSendSeq);
      return;
   }
   if(mSimulatedPing)
   {
      Sim::postEvent(getId(), new NetDelayEvent(stream), Sim::getCurrentTime() + mSimulatedPing);
      return;
   }
   sendPacket(stream);
}

Net::Error NetConnection::sendPacket(BitStream *stream)
{
   //Con::printf("NET  %d: SEND - %d", getId(), mLastSendSeq);
   // do nothing on send if this is a demo replay.
   if(mDemoReadStream)
      return Net::NoError;

   gNetBitsSent = stream->getPosition();

   if(isLocalConnection())
   {
      // short circuit connection to the other side.
      // handle the packet, then force a notify.
      stream->setBuffer(stream->getBuffer(), stream->getPosition(), stream->getPosition());
      mRemoteConnection->processRawPacket(stream);

      return Net::NoError;
   }
   else
   {
      return Net::sendto(getNetAddress(), stream->getBuffer(), stream->getPosition());
   }
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

// these are the virtual function defs for Connection -
// if your subclass has additional data to read / write / notify, add it in these functions.

void NetConnection::readPacket(BitStream *bstream)
{
   eventReadPacket(bstream);
   ghostReadPacket(bstream);
}

void NetConnection::writePacket(BitStream *bstream, PacketNotify *note)
{
   eventWritePacket(bstream, note);
   ghostWritePacket(bstream, note);
}

void NetConnection::packetReceived(PacketNotify *note)
{
   eventPacketReceived(note);
   ghostPacketReceived(note);
}

void NetConnection::packetDropped(PacketNotify *note)
{
   eventPacketDropped(note);
   ghostPacketDropped(note);
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

void NetConnection::writeDemoStartBlock(ResizeBitStream* stream)
{
   ConnectionProtocol::writeDemoStartBlock(stream);

   stream->write(mRoundTripTime);
   stream->write(mPacketLoss);
#ifndef TORQUE_TGB_ONLY
   // Write all the current paths to the stream...
   gClientPathManager->dumpState(stream);
#endif
   stream->validate();
   mStringTable->writeDemoStartBlock(stream);

   U32 start = 0;
   PacketNotify *note = mNotifyQueueHead;
   while(note)
   {
      start++;
      note = note->nextPacket;
   }
   stream->write(start);

   eventWriteStartBlock(stream);
   ghostWriteStartBlock(stream);
}

bool NetConnection::readDemoStartBlock(BitStream* stream)
{
   ConnectionProtocol::readDemoStartBlock(stream);

   stream->read(&mRoundTripTime);
   stream->read(&mPacketLoss);

#ifndef TORQUE_TGB_ONLY
   // Read
   gClientPathManager->readState(stream);
#endif

   mStringTable->readDemoStartBlock(stream);
   U32 pos;
   stream->read(&pos); // notify count
   for(U32 i = 0; i < pos; i++)
   {
      PacketNotify *note = allocNotify();
      note->nextPacket = NULL;
      if(!mNotifyQueueHead)
         mNotifyQueueHead = note;
      else
         mNotifyQueueTail->nextPacket = note;
      mNotifyQueueTail = note;
   }
   eventReadStartBlock(stream);
   ghostReadStartBlock(stream);
   return true;
}

bool NetConnection::startDemoRecord(const char *fileName)
{
   FileStream *fs = new FileStream;

   if((fs = FileStream::createAndOpen( fileName, Torque::FS::File::Write )) == NULL)
      return false;

   mDemoWriteStream = fs;
   mDemoWriteStream->write(mProtocolVersion);
   ResizeBitStream bs;

   // then write out the start block
   writeDemoStartBlock(&bs);
   U32 size = bs.getPosition() + 1;
   mDemoWriteStream->write(size);
   mDemoWriteStream->write(size, bs.getBuffer());
   return true;
}

bool NetConnection::replayDemoRecord(const char *fileName)
{
   Stream *fs;
   if((fs = FileStream::createAndOpen( fileName, Torque::FS::File::Read )) == NULL)
      return false;

   mDemoReadStream = fs;
   mDemoReadStream->read(&mProtocolVersion);
   U32 size;
   mDemoReadStream->read(&size);
   U8 *block = new U8[size];
   mDemoReadStream->read(size, block);
   BitStream bs(block, size);

   bool res = readDemoStartBlock(&bs);
   delete[] block;
   if(!res)
      return false;

   // prep for first block read
   // type/size stored in U16: [type:4][size:12]
   U16 typeSize;
   mDemoReadStream->read(&typeSize);

   mDemoNextBlockType = typeSize >> 12;
   mDemoNextBlockSize = typeSize & 0xFFF;

   if(mDemoReadStream->getStatus() != Stream::Ok)
      return false;
   return true;
}

void NetConnection::stopRecording()
{
   if(mDemoWriteStream)
   {
      delete mDemoWriteStream;
      mDemoWriteStream = NULL;
   }
}

void NetConnection::recordBlock(U32 type, U32 size, void *data)
{
   AssertFatal(type < MaxNumBlockTypes, "NetConnection::recordBlock: invalid type");
   AssertFatal(size < MaxBlockSize, "NetConnection::recordBlock: invalid size");
   if((type >= MaxNumBlockTypes) || (size >= MaxBlockSize))
      return;

   if(mDemoWriteStream)
   {
      // store type/size in U16: [type:4][size:12]
      U16 typeSize = (type << 12) | size;
      mDemoWriteStream->write(typeSize);
      if(size)
         mDemoWriteStream->write(size, data);
   }
}

void NetConnection::handleRecordedBlock(U32 type, U32 size, void *data)
{
   switch(type)
   {
      case BlockTypePacket: {
         BitStream bs(data, size);
         processRawPacket(&bs);
         break;
      }
      case BlockTypeSendPacket:
         checkPacketSend(true);
         break;
   }
}

void NetConnection::demoPlaybackComplete()
{
}

void NetConnection::stopDemoPlayback()
{
   demoPlaybackComplete();
   deleteObject();
}

bool NetConnection::processNextBlock()
{
   U8 buffer[Net::MaxPacketDataSize];

   // read in and handle
   if(mDemoReadStream->read(mDemoNextBlockSize, buffer))
      handleRecordedBlock(mDemoNextBlockType, mDemoNextBlockSize, buffer);

   U16 typeSize;
   mDemoReadStream->read(&typeSize);

   mDemoNextBlockType = typeSize >> 12;
   mDemoNextBlockSize = typeSize & 0xFFF;

   if(mDemoReadStream->getStatus() != Stream::Ok)
   {
      stopDemoPlayback();
      return false;
   }
   return true;
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

// some handy string functions for compressing strings over a connection:
enum NetStringConstants
{
   NullString = 0,
   CString,
   TagString,
   Integer
};

void NetConnection::validateSendString(const char *str)
{
   if(U8(*str) == StringTagPrefixByte)
   {
      NetStringHandle strHandle(dAtoi(str + 1));
      checkString(strHandle);
   }
}

void NetConnection::packString(BitStream *stream, const char *str)
{
   if(!*str)
   {
      stream->writeInt(NullString, 2);
      return;
   }
   if(U8(str[0]) == StringTagPrefixByte)
   {
      stream->writeInt(TagString, 2);
      stream->writeInt(dAtoi(str + 1), ConnectionStringTable::EntryBitSize);
      return;
   }
   if(str[0] == '-' || (str[0] >= '0' && str[0] <= '9'))
   {
      char buf[16];
      S32 num = dAtoi(str);
      dSprintf(buf, sizeof(buf), "%d", num);
      if(!dStrcmp(buf, str))
      {
         stream->writeInt(Integer, 2);
         if(stream->writeFlag(num < 0))
            num = -num;
         if(stream->writeFlag(num < 128))
         {
            stream->writeInt(num, 7);
            return;
         }
         if(stream->writeFlag(num < 32768))
         {
            stream->writeInt(num, 15);
            return;
         }
         else
         {
            stream->writeInt(num, 31);
            return;
         }
      }
   }
   stream->writeInt(CString, 2);
   stream->writeString(str);
}

void NetConnection::unpackString(BitStream *stream, char readBuffer[1024])
{
   U32 code = stream->readInt(2);
   switch(code)
   {
      case NullString:
         readBuffer[0] = 0;
         return;
      case CString:
         {
            stream->readString(readBuffer);
            return;
         }
      case TagString:
         U32 tag;
         tag = stream->readInt(ConnectionStringTable::EntryBitSize);
         readBuffer[0] = StringTagPrefixByte;
         dSprintf(readBuffer+1, 1023, "%d", tag);
         return;
      case Integer:
         bool neg;
         neg = stream->readFlag();
         S32 num;
         if(stream->readFlag())
            num = stream->readInt(7);
         else if(stream->readFlag())
            num = stream->readInt(15);
         else
            num = stream->readInt(31);
         if(neg)
            num = -num;
         dSprintf(readBuffer, 1024, "%d", num);
   }
}

void NetConnection::packNetStringHandleU(BitStream *stream, NetStringHandle &h)
{
   if(stream->writeFlag(h.isValidString() ))
   {
      bool isReceived;
      U32 netIndex = checkString(h, &isReceived);
      if(stream->writeFlag(isReceived))
         stream->writeInt(netIndex, ConnectionStringTable::EntryBitSize);
      else
         stream->writeString(h.getString());
   }
}

NetStringHandle NetConnection::unpackNetStringHandleU(BitStream *stream)
{
   NetStringHandle ret;
   if(stream->readFlag())
   {
      if(stream->readFlag())
         ret = mStringTable->lookupString(stream->readInt(ConnectionStringTable::EntryBitSize));
      else
      {
         char buf[256];
         stream->readString(buf);
         ret = NetStringHandle(buf);
      }
   }
   return ret;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void NetConnection::setAddressDigest(U32 digest[4])
{
   mAddressDigest[0] = digest[0];
   mAddressDigest[1] = digest[1];
   mAddressDigest[2] = digest[2];
   mAddressDigest[3] = digest[3];
}

void NetConnection::getAddressDigest(U32 digest[4])
{
   digest[0] = mAddressDigest[0];
   digest[1] = mAddressDigest[1];
   digest[2] = mAddressDigest[2];
   digest[3] = mAddressDigest[3];
}

bool NetConnection::canRemoteCreate()
{
   return false;
}

void NetConnection::onTimedOut()
{

}

void NetConnection::connect(const NetAddress *address)
{
   mNetAddress = *address;
   GNet->startConnection(this);
}

void NetConnection::onConnectTimedOut()
{

}

void NetConnection::sendDisconnectPacket(const char *reason)
{
   GNet->sendDisconnectPacket(this, reason);
}

void NetConnection::onDisconnect(const char *reason)
{
   TORQUE_UNUSED(reason);
}

void NetConnection::onConnectionRejected(const char *reason)
{
}

void NetConnection::onConnectionEstablished(bool isInitiator)
{

}

void NetConnection::handleStartupError(const char *errorString)
{

}

void NetConnection::writeConnectRequest(BitStream *stream)
{
   stream->write(mNetClassGroup);
   stream->write(U32(AbstractClassRep::getClassCRC(mNetClassGroup)));
}

bool NetConnection::readConnectRequest(BitStream *stream, const char **errorString)
{
   U32 classGroup, classCRC;
   stream->read(&classGroup);
   stream->read(&classCRC);

   if(classGroup == mNetClassGroup && classCRC == AbstractClassRep::getClassCRC(mNetClassGroup))
      return true;

   *errorString = "CHR_INVALID";
   return false;
}

void NetConnection::writeConnectAccept(BitStream *stream)
{
   TORQUE_UNUSED(stream);
}

bool NetConnection::readConnectAccept(BitStream *stream, const char **errorString)
{
   TORQUE_UNUSED(stream);
   TORQUE_UNUSED(errorString);
   return true;
}

DefineEngineMethod( NetConnection, resolveGhostID, S32, (S32 ghostID),,
   "@brief On the client, convert a ghost ID from this connection to a real SimObject ID.\n\n"

   "Torque's network ghosting system only exchanges ghost ID's between the server and client.  Use "
   "this method on the client to discover an object's local SimObject ID when you only have a "
   "ghost ID.\n"

   "@param ghostID The ghost ID of the object as sent by the server.\n"
   "@returns The SimObject ID of the object, or 0 if it could not be resolved.\n\n"

   "@tsexample\n"
      "%object = ServerConnection.resolveGhostID( %ghostId );\n"
   "@endtsexample\n\n"

   "@see @ref ghosting_scoping for a description of the ghosting system.\n\n")
{
   // Safety check
   if(ghostID < 0 || ghostID > NetConnection::MaxGhostCount) return 0;

   NetObject *foo = object->resolveGhost(ghostID);

   if(foo)
      return foo->getId();
   else
      return 0;
}

DefineEngineMethod( NetConnection, resolveObjectFromGhostIndex, S32, (S32 ghostID),,
   "@brief On the server, convert a ghost ID from this connection to a real SimObject ID.\n\n"

   "Torque's network ghosting system only exchanges ghost ID's between the server and client.  Use "
   "this method on the server to discover an object's local SimObject ID when you only have a "
   "ghost ID.\n"

   "@param ghostID The ghost ID of the object as sent by the server.\n"
   "@returns The SimObject ID of the object, or 0 if it could not be resolved.\n\n"

   "@tsexample\n"
      "%object = %client.resolveObjectFromGhostIndex( %ghostId );\n"
   "@endtsexample\n\n"

   "@see @ref ghosting_scoping for a description of the ghosting system.\n\n")
{
   // Safety check
   if(ghostID < 0 || ghostID > NetConnection::MaxGhostCount) return 0;

   NetObject *foo = object->resolveObjectFromGhostIndex(ghostID);

   if(foo)
      return foo->getId();
   else
      return 0;
}

DefineEngineMethod( NetConnection, getGhostID, S32, (S32 realID),,
   "@brief On server or client, convert a real id to the ghost id for this connection.\n\n"

   "Torque's network ghosting system only exchanges ghost ID's between the server and client.  Use "
   "this method on the server or client to discover an object's ghost ID based on its real SimObject ID.\n"

   "@param realID The real SimObject ID of the object.\n"
   "@returns The ghost ID of the object for this connection, or -1 if it could not be resolved.\n\n"

   "@see @ref ghosting_scoping for a description of the ghosting system.\n\n")
{
   NetObject * foo;

   if(Sim::findObject(realID, foo))
   {
      return object->getGhostIndex(foo);
   }
   else
   {
      Con::errorf("NetConnection::serverToGhostID - could not find specified object");
      return -1;
   }
}

DefineEngineMethod( NetConnection, connect, void, (const char* remoteAddress),,
   "@brief Connects to the remote address.\n\n"

   "Attempts to connect with another NetConnection on the given address.  Typically once "
   "connected, a game's information is passed along from the server to the client, followed "
   "by the player entering the game world.  The actual procedure is dependent on "
   "the NetConnection subclass that is used.  i.e. GameConnection.\n"

   "@param remoteAddress The address to connect to in the form of IP:&lt;address&gt;:&lt;port&rt; "
   "although the <i>IP:</i> portion is optional.  The <i>address</i> portion may be in the form "
   "of w.x.y.z or as a host name, in which case a DNS lookup will be performed.  You may also "
   "substitue the word <i>broadcast</i> for the address to broadcast the connect request over "
   "the local subnet.\n\n"

   "@see NetConnection::connectLocal() to connect to a server running within the same process "
   "as the client.\n"
   )
{
   NetAddress addr;
   if(!Net::stringToAddress(remoteAddress, &addr))
   {
      Con::errorf("NetConnection::connect: invalid address - %s", remoteAddress);
      return;
   }
   object->connect(&addr);
}

DefineEngineMethod( NetConnection, connectLocal, const char*, (),,
   "@brief Connects with the server that is running within the same process as the client.\n\n"

   "@returns An error text message upon failure, or an empty string when successful.\n\n"

   "@see See @ref local_connections for a description of local connections and their use.  See "
   "NetConnection::connect() to connect to a server running in another process (on the same machine or not).\n")
{
   ConsoleObject *co = ConsoleObject::create(object->getClassName());
   NetConnection *client = object;
   NetConnection *server = dynamic_cast<NetConnection *>(co);
   BitStream *stream = BitStream::getPacketStream();

   if(!server || !server->canRemoteCreate())
   {
      delete co;
      return "error";
   }
      
   server->registerObject();
   server->setIsLocalClientConnection();

   server->setSequence(0);
   client->setSequence(0);
   client->setRemoteConnectionObject(server);
   server->setRemoteConnectionObject(client);

   //We need to reset the maxrate's here, because we
   // can't test if it is a local connection until RemoteConnectionObject
   // has been set
   server->checkMaxRate();  
   client->checkMaxRate();

   stream->setPosition(0);
   client->writeConnectRequest(stream);
   stream->setPosition(0);
   
   const char* error;
   if( !server->readConnectRequest( stream, &error ) )
   {
      client->onConnectionRejected( error );
      server->deleteObject();
      return "error";
   }

   stream->setPosition(0);
   server->writeConnectAccept(stream);
   stream->setPosition(0);

   if( !client->readConnectAccept( stream, &error ) )
   {
      client->handleStartupError( error );
      server->deleteObject();
      return "error";
   }

   client->onConnectionEstablished(true);
   server->onConnectionEstablished(false);
   client->setEstablished();
   server->setEstablished();
   client->setConnectSequence(0);
   server->setConnectSequence(0);
   NetConnection::setLocalClientConnection(server);
   server->assignName("LocalClientConnection");
   
   return "";
}
