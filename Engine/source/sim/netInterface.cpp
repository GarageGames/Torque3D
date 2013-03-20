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
#include "sim/netConnection.h"
#include "sim/netInterface.h"
#include "core/stream/bitStream.h"
#include "math/mRandom.h"
#include "core/util/journal/journal.h"

#ifdef GGC_PLUGIN
#include "GGCNatTunnel.h" 
extern void HandleGGCPacket(NetAddress* addr, unsigned char* data, U32 dataSize);
#endif

NetInterface *GNet = NULL;

NetInterface::NetInterface()
{
   AssertFatal(GNet == NULL, "ERROR: Multiple net interfaces declared.");
   GNet = this;

   mLastTimeoutCheckTime = 0;
   mAllowConnections = true;

}

void NetInterface::initRandomData()
{
   mRandomDataInitialized = true;
   U32 seed = Platform::getRealMilliseconds();

   if(Journal::IsPlaying())
      Journal::Read(&seed);
   else if(Journal::IsRecording())
      Journal::Write(seed);

   MRandomR250 myRandom(seed);
   for(U32 i = 0; i < 12; i++)
      mRandomHashData[i] = myRandom.randI();
}

void NetInterface::addPendingConnection(NetConnection *connection)
{
   Con::printf("Adding a pending connection");
   mPendingConnections.push_back(connection);
}

void NetInterface::removePendingConnection(NetConnection *connection)
{
   for(U32 i = 0; i < mPendingConnections.size(); i++)
      if(mPendingConnections[i] == connection)
         mPendingConnections.erase(i);
}

NetConnection *NetInterface::findPendingConnection(const NetAddress *address, U32 connectSequence)
{
   for(U32 i = 0; i < mPendingConnections.size(); i++)
      if(Net::compareAddresses(address, mPendingConnections[i]->getNetAddress()) &&
            connectSequence == mPendingConnections[i]->getSequence())
         return mPendingConnections[i];
   return NULL;
}

void NetInterface::processPacketReceiveEvent(NetAddress srcAddress, RawData packetData)
{

   U32 dataSize = packetData.size;
   BitStream pStream(packetData.data, dataSize);

   // Determine what to do with this packet:

   if(packetData.data[0] & 0x01) // it's a protocol packet...
   {
      // if the LSB of the first byte is set, it's a game data packet
      // so pass it to the appropriate connection.

      // lookup the connection in the addressTable
      NetConnection *conn = NetConnection::lookup(&srcAddress);
      if(conn)
         conn->processRawPacket(&pStream);
   }
   else
   {
      // Otherwise, it's either a game info packet or a
      // connection handshake packet.

      U8 packetType;
      pStream.read(&packetType);
      NetAddress *addr = &srcAddress;

      if(packetType <= GameHeartbeat)
         handleInfoPacket(addr, packetType, &pStream);
#ifdef GGC_PLUGIN
      else if (packetType == GGCPacket)
      {
         HandleGGCPacket(addr, (U8*)packetData.data, dataSize);
      }
#endif
      else
      {
         // check if there's a connection already:
         switch(packetType)
         {
            case ConnectChallengeRequest:
               handleConnectChallengeRequest(addr, &pStream);
               break;
            case ConnectRequest:
               handleConnectRequest(addr, &pStream);
               break;
            case ConnectChallengeResponse:
               handleConnectChallengeResponse(addr, &pStream);
               break;
            case ConnectAccept:
               handleConnectAccept(addr, &pStream);
               break;
            case Disconnect:
               handleDisconnect(addr, &pStream);
               break;
            case ConnectReject:
               handleConnectReject(addr, &pStream);
               break;
         }
      }
   }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Connection handshaking basic overview:
// The torque engine does a two phase connect handshake to
// prevent a spoofed source address Denial-of-Service (DOS) attack
//
// Basically, the initiator of a connection (client) sends a
// Connect Challenge Request packet to the server to initiate the connection
// The server then hashes the source address of the client request
// with some random magic server data to come up with a 16-byte key that
// the client can then use to gain entry to the server.
// This way there are no partially active connection records on the
// server at all.
//
// The client then sends a Connect Request packet to the server,
// including any game specific data necessary to start a connection (a
// server password, for instance), along with the key the server sent
// on the Connect Challenge Response packet.
//
// The server, on receipt of the Connect Request, compares the
// entry key with a computed key, makes sure it can create the requested
// NetConnection subclass, and then passes all processing on to the connection
// instance.
//
// If the subclass reads and accepts he connect request successfully, the
// server sends a Connect Accept packet - otherwise the connection
// is rejected with the sendConnectReject function
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


void NetInterface::sendConnectChallengeRequest(NetConnection *conn)
{
   Con::printf("Sending Connect challenge Request");
   BitStream *out = BitStream::getPacketStream();

   out->write(U8(ConnectChallengeRequest));
   out->write(conn->getSequence());

   conn->mConnectSendCount++;
   conn->mConnectLastSendTime = Platform::getVirtualMilliseconds();

   BitStream::sendPacketStream(conn->getNetAddress());
}

void NetInterface::handleConnectChallengeRequest(const NetAddress *addr, BitStream *stream)
{
   char buf[256];
   Net::addressToString(addr, buf);
   Con::printf("Got Connect challenge Request from %s", buf);
   if(!mAllowConnections)
      return;

   U32 connectSequence;
   stream->read(&connectSequence);

   if(!mRandomDataInitialized)
      initRandomData();

   U32 addressDigest[4];
   computeNetMD5(addr, connectSequence, addressDigest);

   BitStream *out = BitStream::getPacketStream();
   out->write(U8(ConnectChallengeResponse));
   out->write(connectSequence);
   out->write(addressDigest[0]);
   out->write(addressDigest[1]);
   out->write(addressDigest[2]);
   out->write(addressDigest[3]);

   BitStream::sendPacketStream(addr);
}

//-----------------------------------------------------------------------------

void NetInterface::handleConnectChallengeResponse(const NetAddress *address, BitStream *stream)
{
   Con::printf("Got Connect challenge Response");
   U32 connectSequence;
   stream->read(&connectSequence);

   NetConnection *conn = findPendingConnection(address, connectSequence);
   if(!conn || conn->getConnectionState() != NetConnection::AwaitingChallengeResponse)
      return;

   U32 addressDigest[4];
   stream->read(&addressDigest[0]);
   stream->read(&addressDigest[1]);
   stream->read(&addressDigest[2]);
   stream->read(&addressDigest[3]);
   conn->setAddressDigest(addressDigest);

   conn->setConnectionState(NetConnection::AwaitingConnectResponse);
   conn->mConnectSendCount = 0;
   Con::printf("Sending Connect Request");
   sendConnectRequest(conn);
}

//-----------------------------------------------------------------------------

void NetInterface::sendConnectRequest(NetConnection *conn)
{
   BitStream *out = BitStream::getPacketStream();
   out->write(U8(ConnectRequest));
   out->write(conn->getSequence());

   U32 addressDigest[4];
   conn->getAddressDigest(addressDigest);
   out->write(addressDigest[0]);
   out->write(addressDigest[1]);
   out->write(addressDigest[2]);
   out->write(addressDigest[3]);

   out->writeString(conn->getClassName());
   conn->writeConnectRequest(out);
   conn->mConnectSendCount++;
   conn->mConnectLastSendTime = Platform::getVirtualMilliseconds();

   BitStream::sendPacketStream(conn->getNetAddress());
}

//-----------------------------------------------------------------------------

void NetInterface::handleConnectRequest(const NetAddress *address, BitStream *stream)
{
   if(!mAllowConnections)
      return;
   Con::printf("Got Connect Request");
   U32 connectSequence;
   stream->read(&connectSequence);

   // see if the connection is in the main connection table:

   NetConnection *connect = NetConnection::lookup(address);
   if(connect && connect->getSequence() == connectSequence)
   {
      sendConnectAccept(connect);
      return;
   }
   U32 addressDigest[4];
   U32 computedAddressDigest[4];

   stream->read(&addressDigest[0]);
   stream->read(&addressDigest[1]);
   stream->read(&addressDigest[2]);
   stream->read(&addressDigest[3]);

   computeNetMD5(address, connectSequence, computedAddressDigest);
   if(addressDigest[0] != computedAddressDigest[0] ||
      addressDigest[1] != computedAddressDigest[1] ||
      addressDigest[2] != computedAddressDigest[2] ||
      addressDigest[3] != computedAddressDigest[3])
      return; // bogus connection attempt

   if(connect)
   {
      if(connect->getSequence() > connectSequence)
         return; // the existing connection should be kept - the incoming request is stale.
      else
         connect->deleteObject(); // disconnect this one, and allow the new one to be created.
   }

   char connectionClass[255];
   stream->readString(connectionClass);

   ConsoleObject *co = ConsoleObject::create(connectionClass);
   NetConnection *conn = dynamic_cast<NetConnection *>(co);
   if(!conn || !conn->canRemoteCreate())
   {
      delete co;
      return;
   }
   conn->registerObject();
   conn->setNetAddress(address);
   conn->setNetworkConnection(true);
   conn->setSequence(connectSequence);

   const char *errorString = NULL;
   if(!conn->readConnectRequest(stream, &errorString))
   {
      sendConnectReject(conn, errorString);
      conn->deleteObject();
      return;
   }
   conn->setNetworkConnection(true);
   conn->onConnectionEstablished(false);
   conn->setEstablished();
   conn->setConnectSequence(connectSequence);
   sendConnectAccept(conn);
}

//-----------------------------------------------------------------------------

void NetInterface::sendConnectAccept(NetConnection *conn)
{
   BitStream *out = BitStream::getPacketStream();
   out->write(U8(ConnectAccept));
   out->write(conn->getSequence());
   conn->writeConnectAccept(out);
   BitStream::sendPacketStream(conn->getNetAddress());
}

void NetInterface::handleConnectAccept(const NetAddress *address, BitStream *stream)
{
   U32 connectSequence;
   stream->read(&connectSequence);
   NetConnection *conn = findPendingConnection(address, connectSequence);
   if(!conn || conn->getConnectionState() != NetConnection::AwaitingConnectResponse)
      return;
   const char *errorString = NULL;
   if(!conn->readConnectAccept(stream, &errorString))
   {
      conn->handleStartupError(errorString);
      removePendingConnection(conn);
      conn->deleteObject();
      return;
   }

   removePendingConnection(conn); // remove from the pending connection list
   conn->setNetworkConnection(true);
   conn->onConnectionEstablished(true); // notify the connection that it has been established
   conn->setEstablished(); // installs the connection in the connection table, and causes pings/timeouts to happen
   conn->setConnectSequence(connectSequence);
}

void NetInterface::sendConnectReject(NetConnection *conn, const char *reason)
{
   if(!reason)
      return; // if the stream is NULL, we reject silently

   BitStream *out = BitStream::getPacketStream();
   out->write(U8(ConnectReject));
   out->write(conn->getSequence());
   out->writeString(reason);
   BitStream::sendPacketStream(conn->getNetAddress());
}

void NetInterface::handleConnectReject(const NetAddress *address, BitStream *stream)
{
   U32 connectSequence;
   stream->read(&connectSequence);
   NetConnection *conn = findPendingConnection(address, connectSequence);
   if(!conn || (conn->getConnectionState() != NetConnection::AwaitingChallengeResponse &&
                conn->getConnectionState() != NetConnection::AwaitingConnectResponse))
      return;
   removePendingConnection(conn);
   char reason[256];
   stream->readString(reason);
   conn->onConnectionRejected(reason);
   conn->deleteObject();
}

void NetInterface::handleDisconnect(const NetAddress *address, BitStream *stream)
{
   NetConnection *conn = NetConnection::lookup(address);
   if(!conn)
      return;

   U32 connectSequence;
   char reason[256];

   stream->read(&connectSequence);
   stream->readString(reason);

   if(conn->getSequence() != connectSequence)
      return;

   conn->onDisconnect(reason);
   conn->deleteObject();
}

void NetInterface::handleInfoPacket(const NetAddress *address, U8 packetType, BitStream *stream)
{
}

void NetInterface::processClient()
{
   NetObject::collapseDirtyList(); // collapse all the mask bits...
   for(NetConnection *walk = NetConnection::getConnectionList();
      walk; walk = walk->getNext())
   {
      if(walk->isConnectionToServer() && (walk->isLocalConnection() || walk->isNetworkConnection()))
         walk->checkPacketSend(false);
   }
}

void NetInterface::processServer()
{
   NetObject::collapseDirtyList(); // collapse all the mask bits...
   for(NetConnection *walk = NetConnection::getConnectionList();
      walk; walk = walk->getNext())
   {
      if(!walk->isConnectionToServer() && (walk->isLocalConnection() || walk->isNetworkConnection()))
         walk->checkPacketSend(false);
   }
}

void NetInterface::startConnection(NetConnection *conn)
{
   addPendingConnection(conn);
   conn->mConnectionSendCount = 0;
   conn->setConnectSequence(Platform::getVirtualMilliseconds());
   conn->setConnectionState(NetConnection::AwaitingChallengeResponse);

   // This is a the client side of the connection, so set the connection to
   // server flag. We need to set this early so that if the connection times
   // out, its onRemove() will handle the cleanup properly.
   conn->setIsConnectionToServer();

   // Everything set, so send off the request.
   sendConnectChallengeRequest(conn);
}

void NetInterface::sendDisconnectPacket(NetConnection *conn, const char *reason)
{
   Con::printf("Issuing Disconnect packet.");

   // send a disconnect packet...
   U32 connectSequence = conn->getSequence();

   BitStream *out = BitStream::getPacketStream();
   out->write(U8(Disconnect));
   out->write(connectSequence);
   out->writeString(reason);

   BitStream::sendPacketStream(conn->getNetAddress());
}

void NetInterface::checkTimeouts()
{
   U32 time = Platform::getVirtualMilliseconds();
   if(time > mLastTimeoutCheckTime + TimeoutCheckInterval)
   {
      for(U32 i = 0; i < mPendingConnections.size();)
      {
         NetConnection *pending = mPendingConnections[i];

         if(pending->getConnectionState() == NetConnection::AwaitingChallengeResponse &&
            time > pending->mConnectLastSendTime + ChallengeRetryTime)
         {
            if(pending->mConnectSendCount > ChallengeRetryCount)
            {
               pending->onConnectTimedOut();
               removePendingConnection(pending);
               pending->deleteObject();
               continue;
            }
            else
               sendConnectChallengeRequest(pending);
         }
         else if(pending->getConnectionState() == NetConnection::AwaitingConnectResponse &&
            time > pending->mConnectLastSendTime + ConnectRetryTime)
         {
            if(pending->mConnectSendCount > ConnectRetryCount)
            {
               pending->onConnectTimedOut();
               removePendingConnection(pending);
               pending->deleteObject();
               continue;
            }
            else
               sendConnectRequest(pending);
         }
         i++;
      }
      mLastTimeoutCheckTime = time;
      NetConnection *walk = NetConnection::getConnectionList();

      while(walk)
      {
         NetConnection *next = walk->getNext();
         if(walk->checkTimeout(time))
         {
            // this baddie timed out
            walk->onTimedOut();
            walk->deleteObject();
         }
         walk = next;
      }
   }
}

#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

inline U32 rotlFixed(U32 x, unsigned int y)
{
   return (x >> y) | (x << (32 - y));
}

#define MD5STEP(f, w, x, y, z, data, s) w = rotlFixed(w + f(x, y, z) + data, s) + x

void NetInterface::computeNetMD5(const NetAddress *address, U32 connectSequence, U32 digest[4])
{
	digest[0] = 0x67452301L;
	digest[1] = 0xefcdab89L;
	digest[2] = 0x98badcfeL;
	digest[3] = 0x10325476L;


   U32 a, b, c, d;

   a=digest[0];
   b=digest[1];
   c=digest[2];
   d=digest[3];

   U32 in[16];
   in[0] = address->type;
   in[1] = (U32(address->netNum[0]) << 24) |
           (U32(address->netNum[1]) << 16) |
           (U32(address->netNum[2]) << 8)  |
           (U32(address->netNum[3]));
   in[2] = address->port;
   in[3] = connectSequence;
   for(U32 i = 0; i < 12; i++)
      in[i + 4] = mRandomHashData[i];

   MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
   MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
   MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
   MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
   MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
   MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
   MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
   MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
   MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
   MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
   MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
   MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
   MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
   MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
   MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
   MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

   MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
   MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
   MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
   MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
   MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
   MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
   MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
   MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
   MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
   MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
   MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
   MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
   MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
   MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
   MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
   MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

   MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
   MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
   MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
   MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
   MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
   MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
   MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
   MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
   MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
   MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
   MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
   MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
   MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
   MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
   MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
   MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

   MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
   MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
   MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
   MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
   MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
   MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
   MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
   MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
   MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
   MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
   MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
   MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
   MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
   MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
   MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
   MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

	digest[0]+=a;
	digest[1]+=b;
	digest[2]+=c;
	digest[3]+=d;
}

ConsoleFunctionGroupBegin(NetInterface, "Global control functions for the netInterfaces.");

ConsoleFunction(allowConnections,void,2,2,"allowConnections(bool allow);"
   "@brief Sets whether or not the global NetInterface allows connections from remote hosts.\n\n"

   "@param allow Set to true to allow remote connections.\n"

   "@ingroup Networking\n")
{
   TORQUE_UNUSED(argc);
   GNet->setAllowsConnections(dAtob(argv[1]));
}

ConsoleFunctionGroupEnd(NetInterface);

