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

#ifndef TORQUE_NET_DEFAULT_MULTICAST_ADDRESS
#define TORQUE_NET_DEFAULT_MULTICAST_ADDRESS "ff04::7467::656E::6574::776B"
#endif

typedef S32 NetConnectionId;

/// Generic network address
///
/// This is used to represent IP addresses.
struct NetAddress 
{
   S32 type;        ///< Type of address (IPAddress currently)

   /// Acceptable NetAddress types.
   enum Type
   {
      IPAddress,
      IPV6Address,

      IPBroadcastAddress,
      IPV6MulticastAddress
   };

   union
   {
      struct {
         U8 netNum[4];
      } ipv4;

      struct {
         U8 netNum[16];
         U32 netFlow;
         U32 netScope;
      } ipv6;

      struct {
         U8 netNum[16];
         U8 netFlow[4];
         U8 netScope[4];
      } ipv6_raw;

   } address;

   U16 port;

   bool isSameAddress(const NetAddress &other) const
   {
      if (type != other.type)
         return false;

      switch (type)
      {
      case NetAddress::IPAddress:
         return (dMemcmp(other.address.ipv4.netNum, address.ipv4.netNum, 4) == 0);
         break;
      case NetAddress::IPV6Address:
         return (dMemcmp(other.address.ipv6.netNum, address.ipv6.netNum, 16) == 0);
         break;
      case NetAddress::IPBroadcastAddress:
         return true;
         break;
      case NetAddress::IPV6MulticastAddress:
         return true;
         break;
      }

      return false;
   }

   bool isSameAddressAndPort(const NetAddress &other) const
   {
      if (type != other.type)
         return false;

      switch (type)
      {
      case NetAddress::IPAddress:
         return (dMemcmp(other.address.ipv4.netNum, address.ipv4.netNum, 4) == 0) && other.port == port;
         break;
      case NetAddress::IPV6Address:
         return (dMemcmp(other.address.ipv6.netNum, address.ipv6.netNum, 16) == 0) && other.port == port;
         break;
      case NetAddress::IPBroadcastAddress:
         return true;
         break;
      case NetAddress::IPV6MulticastAddress:
         return true;
         break;
      }

      return false;
   }

   bool isEqual(const NetAddress &other) const
   {
      if (type != other.type)
         return false;

      switch (type)
      {
      case NetAddress::IPAddress:
         return other.port == port && (dMemcmp(other.address.ipv4.netNum, address.ipv4.netNum, 4) == 0);
         break;
      case NetAddress::IPV6Address:
         return other.port == port && other.address.ipv6.netFlow == address.ipv6.netFlow && other.address.ipv6.netScope == address.ipv6.netScope && (dMemcmp(other.address.ipv6.netNum, address.ipv6.netNum, 16) == 0);
         break;
      case NetAddress::IPBroadcastAddress:
         return other.port == port;
         break;
      case NetAddress::IPV6MulticastAddress:
         return other.port == port;
         break;
      }

      return false;
   }

   U32 getHash() const;
};

class NetSocket
{
protected:
   S32 mHandle;

public:
   NetSocket() : mHandle(-1) { ; }

   inline void setHandle(S32 handleId) { mHandle = handleId; }
   inline S32 getHandle() const { return mHandle;  }
   inline U32 getHash() const { return mHandle; }

   bool operator==(const NetSocket &other) const { return mHandle == other.mHandle; }
   bool operator!=(const NetSocket &other) const { return mHandle != other.mHandle; }

   static NetSocket fromHandle(S32 handleId) { NetSocket ret; ret.mHandle = handleId; return ret; }
   static NetSocket INVALID;
};

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
      UnknownError,
     NeedHostLookup
   };

   enum ConnectionState {
      DNSResolved,
      DNSFailed,
      Connected,
      ConnectFailed,
      Disconnected
   };

   static const S32 MaxPacketDataSize = MAXPACKETSIZE;

   static ConnectionNotifyEvent&   getConnectionNotifyEvent();
   static ConnectionAcceptedEvent& getConnectionAcceptedEvent();
   static ConnectionReceiveEvent&  getConnectionReceiveEvent();
   static PacketReceiveEvent&      getPacketReceiveEvent();

   static bool smMulticastEnabled;
   static bool smIpv4Enabled;
   static bool smIpv6Enabled;

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
   static NetSocket openListenPort(U16 port, NetAddress::Type = NetAddress::IPAddress);
   static NetSocket openConnectTo(const char *stringAddress); // does the DNS resolve etc.
   static void closeConnectTo(NetSocket socket);
   static Error sendtoSocket(NetSocket socket, const U8 *buffer, S32 bufferSize, S32 *bytesWritten=NULL);

   static bool compareAddresses(const NetAddress *a1, const NetAddress *a2);
   static Net::Error stringToAddress(const char *addressString, NetAddress *address, bool hostLookup=true, int family=0);
   static void addressToString(const NetAddress *address, char addressString[256]);

   // lower level socked based network functions
   static NetSocket openSocket();
   static Error closeSocket(NetSocket socket);

   static Error send(NetSocket socket, const U8 *buffer, S32 bufferSize, S32 *outBytesWritten=NULL);
   static Error recv(NetSocket socket, U8 *buffer, S32 bufferSize, S32 *bytesRead);

   static Error connect(NetSocket socket, const NetAddress *address);
   static Error listen(NetSocket socket, S32 maxConcurrentListens);
   static NetSocket accept(NetSocket acceptSocket, NetAddress *remoteAddress);

   static Error bindAddress(const NetAddress &address, NetSocket socket, bool useUDP=false);
   static Error setBufferSize(NetSocket socket, S32 bufferSize);
   static Error setBroadcast(NetSocket socket, bool broadcastEnable);
   static Error setBlocking(NetSocket socket, bool blockingIO);

   /// Gets the desired default listen address for a specified address type
   static Net::Error getListenAddress(const NetAddress::Type type, NetAddress *address, bool forceDefaults=false);
   static void getIdealListenAddress(NetAddress *address);

   // Multicast for ipv6 local net browsing
   static void enableMulticast();
   static void disableMulticast();
   static bool isMulticastEnabled();

   // Protocol state
   static bool isAddressTypeAvailable(NetAddress::Type addressType);

private:
   static void process();
   static void processListenSocket(NetSocket socket);

};

#endif
