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

#ifndef _PLATFORM_PLATFORMNET_H_
#define _PLATFORM_PLATFORMNET_H_

#include "platform/platform.h"
#include "core/util/rawData.h"
#include "core/util/journal/journaledSignal.h"

#ifndef MAXPACKETSIZE
#define MAXPACKETSIZE 1500
#endif

typedef int NetConnectionId;

/// Generic network address
///
/// This is used to represent IP addresses.
struct NetAddress 
{
   int type;        ///< Type of address (IPAddress currently)

   /// Acceptable NetAddress types.
   enum 
   {
      IPAddress,
   };

   U8 netNum[4];    ///< For IP:  sin_addr<br>
   U8 nodeNum[6];   ///< For IP:  Not used.<br>
   U16  port;       ///< For IP:  sin_port<br>
};

typedef S32 NetSocket;
const NetSocket InvalidSocket = -1;

/// void event(NetSocket sock, U32 state) 
typedef JournaledSignal<void(NetSocket,U32)> ConnectionNotifyEvent;

/// void event(NetSocket listeningPort, NetSocket newConnection, NetAddress originatingAddress)
typedef JournaledSignal<void(NetSocket,NetSocket,NetAddress)> ConnectionAcceptedEvent;

/// void event(NetSocket connection, RawData incomingData)
typedef JournaledSignal<void(NetSocket,RawData)> ConnectionReceiveEvent;

/// void event(NetAddress originator, RawData incomingData)
typedef JournaledSignal<void(NetAddress,RawData)> PacketReceiveEvent;

/// Platform-specific network operations.
struct Net
{
   enum Error
   {
      NoError,
      WrongProtocolType,
      InvalidPacketProtocol,
      WouldBlock,
      NotASocket,
      UnknownError
   };

   enum ConnectionState {
      DNSResolved,
      DNSFailed,
      Connected,
      ConnectFailed,
      Disconnected
   };

   enum Protocol
   {
      UDPProtocol,
      TCPProtocol
   };

   static const int MaxPacketDataSize = MAXPACKETSIZE;

   static ConnectionNotifyEvent   smConnectionNotify;
   static ConnectionAcceptedEvent smConnectionAccept;
   static ConnectionReceiveEvent  smConnectionReceive;
   static PacketReceiveEvent      smPacketReceive;

   static bool init();
   static void shutdown();

   // Unreliable net functions (UDP)
   // sendto is for sending data
   // all incoming data comes in on packetReceiveEventType
   // App can only open one unreliable port... who needs more? ;)

   static bool openPort(S32 connectPort, bool doBind = true);
   static NetSocket getPort();

   static void closePort();
   static Error sendto(const NetAddress *address, const U8 *buffer, S32 bufferSize);

   // Reliable net functions (TCP)
   // all incoming messages come in on the Connected* events
   static NetSocket openListenPort(U16 port);
   static NetSocket openConnectTo(const char *stringAddress); // does the DNS resolve etc.
   static void closeConnectTo(NetSocket socket);
   static Error sendtoSocket(NetSocket socket, const U8 *buffer, S32 bufferSize);

   static bool compareAddresses(const NetAddress *a1, const NetAddress *a2);
   static bool stringToAddress(const char *addressString, NetAddress *address);
   static void addressToString(const NetAddress *address, char addressString[256]);

   // lower level socked based network functions
   static NetSocket openSocket();
   static Error closeSocket(NetSocket socket);

   static Error send(NetSocket socket, const U8 *buffer, S32 bufferSize);
   static Error recv(NetSocket socket, U8 *buffer, S32 bufferSize, S32 *bytesRead);

   static Error connect(NetSocket socket, const NetAddress *address);
   static Error listen(NetSocket socket, S32 maxConcurrentListens);
   static NetSocket accept(NetSocket acceptSocket, NetAddress *remoteAddress);

   static Error bind(NetSocket socket, U16 port);
   static Error setBufferSize(NetSocket socket, S32 bufferSize);
   static Error setBroadcast(NetSocket socket, bool broadcastEnable);
   static Error setBlocking(NetSocket socket, bool blockingIO);


private:
   static void process();

};

#endif