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

#include "platform/platformNet.h"
#include "platform/threads/mutex.h"
#include "core/strings/stringFunctions.h"
#include "core/util/hashFunction.h"
#include "console/consoleTypes.h"

// jamesu - debug DNS
//#define TORQUE_DEBUG_LOOKUPS


#if defined (TORQUE_OS_WIN)
#define TORQUE_USE_WINSOCK
#include <errno.h>
#include <ws2tcpip.h>

#ifndef EINPROGRESS
#define EINPROGRESS             WSAEINPROGRESS
#endif // EINPROGRESS

#define ioctl ioctlsocket

typedef S32 socklen_t;

#elif defined ( TORQUE_OS_MAC )

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>

typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr * PSOCKADDR;
typedef sockaddr SOCKADDR;
typedef in_addr IN_ADDR;
typedef int SOCKET;

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

#define closesocket close

#elif defined( TORQUE_OS_LINUX )

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>

typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr_in6 SOCKADDR_IN6;
typedef sockaddr * PSOCKADDR;
typedef sockaddr SOCKADDR;
typedef in_addr IN_ADDR;
typedef in6_addr IN6_ADDR;
typedef int SOCKET;

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

#define closesocket close

#elif defined( TORQUE_OS_XENON )

#include <Xtl.h>
#include <string>

#define TORQUE_USE_WINSOCK
#define EINPROGRESS WSAEINPROGRESS
#define ioctl ioctlsocket
typedef S32 socklen_t;

DWORD _getLastErrorAndClear()
{
   DWORD err = WSAGetLastError();
   WSASetLastError( 0 );

   return err;
}

#else

#endif

#if defined(TORQUE_USE_WINSOCK)
static const char* strerror_wsa( S32 code )
{
   switch( code )
   {
#define E( name ) case name: return #name;
      E( WSANOTINITIALISED );
      E( WSAENETDOWN );
      E( WSAEADDRINUSE );
      E( WSAEINPROGRESS );
      E( WSAEALREADY );
      E( WSAEADDRNOTAVAIL );
      E( WSAEAFNOSUPPORT );
      E( WSAEFAULT );
      E( WSAEINVAL );
      E( WSAEISCONN );
      E( WSAENETUNREACH );
      E( WSAEHOSTUNREACH );
      E( WSAENOBUFS );
      E( WSAENOTSOCK );
      E( WSAETIMEDOUT );
      E( WSAEWOULDBLOCK );
      E( WSAEACCES );
#undef E
      default:
         return "Unknown";
   }
}
#endif

#include "core/util/tVector.h"
#include "platform/platformNetAsync.h"
#include "console/console.h"
#include "core/util/journal/process.h"
#include "core/util/journal/journal.h"


NetSocket NetSocket::INVALID = NetSocket::fromHandle(-1);

template<class T> class ReservedSocketList
{
public:
   struct EntryType
   {
      T value;
      bool used;

      EntryType() : value(-1), used(false) { ; }

      bool operator==(const EntryType &e1)
      {
         return value == e1.value && used == e1.used;
      }

      bool operator!=(const EntryType &e1)
      {
         return !(value == e1.value && used == e1.used);
      }
   };

   Vector<EntryType> mSocketList;
   Mutex *mMutex;

   ReservedSocketList()
   {
      mMutex = new Mutex;
   }

   ~ReservedSocketList()
   {
      delete mMutex;
   }

   inline void modify() { Mutex::lockMutex(mMutex); }
   inline void endModify() { Mutex::unlockMutex(mMutex); }

   NetSocket reserve(SOCKET reserveId = -1, bool doLock = true);
   void remove(NetSocket socketToRemove, bool doLock = true);

   T activate(NetSocket socketToActivate, int family, bool useUDP, bool clearOnFail = false);
   T resolve(NetSocket socketToResolve);
};

const SOCKET InvalidSocketHandle = -1;

static void IPSocketToNetAddress(const struct sockaddr_in *sockAddr, NetAddress *address);
static void IPSocket6ToNetAddress(const struct sockaddr_in6 *sockAddr, NetAddress *address);

namespace PlatformNetState
{
   static S32 initCount = 0;

   static const S32 defaultPort = 28000;
   static S32 netPort = 0;

   static NetSocket udpSocket = NetSocket::INVALID;
   static NetSocket udp6Socket = NetSocket::INVALID;
   static NetSocket multicast6Socket = NetSocket::INVALID;

   static ipv6_mreq multicast6Group;

   static ReservedSocketList<SOCKET> smReservedSocketList;

   Net::Error getLastError()
   {
#if defined(TORQUE_USE_WINSOCK)
      S32 err = WSAGetLastError();
      switch (err)
      {
      case 0:
         return Net::NoError;
      case WSAEWOULDBLOCK:
         return Net::WouldBlock;
      default:
         return Net::UnknownError;
      }
#else
      int theError = errno;
      if (errno == EAGAIN)
         return Net::WouldBlock;
      if (errno == 0)
         return Net::NoError;
      if (errno == EINPROGRESS)
         return Net::WouldBlock;
      
      return Net::UnknownError;
#endif
   }

   S32 getDefaultGameProtocol()
   {
      // we turn off VDP in non-release builds because VDP does not support broadcast packets
      // which are required for LAN queries (PC->Xbox connectivity).  The wire protocol still
      // uses the VDP packet structure, though.
      S32 protocol = IPPROTO_UDP;
      bool useVDP = false;
#ifdef TORQUE_DISABLE_PC_CONNECTIVITY
      // Xbox uses a VDP (voice/data protocol) socket for networking
      protocol = IPPROTO_VDP;
      useVDP = true;
#endif

      return protocol;
   }

   struct addrinfo* pickAddressByProtocol(struct addrinfo* addr, int protocol)
   {
      for (addr; addr != NULL; addr = addr->ai_next)
      {
         if (addr->ai_family == protocol)
            return addr;
      }

      return NULL;
   }

   /// Extracts core address parts from an address string. Returns false if it's malformed.
   bool extractAddressParts(const char *addressString, char outAddress[256], int &outPort, int &outFamily)
   {
      outPort = 0;
      outFamily = AF_UNSPEC;

      if (!dStrnicmp(addressString, "ipx:", 4))
         // ipx support deprecated
         return false;

      if (!dStrnicmp(addressString, "ip:", 3))
      {
         addressString += 3;  // eat off the ip:
         outFamily = AF_INET;
      }
      else if (!dStrnicmp(addressString, "ip6:", 4))
      {
         addressString += 4;  // eat off the ip6:
         outFamily = AF_INET6;
      }

      if (strlen(addressString) > 255)
         return false;

      char *portString = NULL;

      if (addressString[0] == '[')
      {
         // Must be ipv6 notation
         dStrcpy(outAddress, addressString+1);
         addressString = outAddress;

         portString = dStrchr(outAddress, ']');
         if (portString)
         {
            // Sort out the :port after the ]
            *portString++ = '\0';
            if (*portString != ':')
            {
               portString = NULL;
            }
            else
            {
               *portString++ = '\0';
            }
         }

         if (outFamily == AF_UNSPEC)
         {
            outFamily = AF_INET6;
         }
      }
      else
      {
         dStrcpy(outAddress, addressString);
         addressString = outAddress;

         // Check to see if we have multiple ":" which would indicate this is an ipv6 address
         char* scan = outAddress;
         int colonCount = 0;
         while (*scan != '\0' && colonCount < 2)
         {
            if (*scan++ == ':')
               colonCount++;
         }
         if (colonCount <= 1)
         {
            // either ipv4 or host
            portString = dStrchr(outAddress, ':');

            if (portString)
            {
               *portString++ = '\0';
            }
         }
         else if (outFamily == AF_UNSPEC)
         {
            // Must be ipv6
            outFamily = AF_INET6;
         }
      }

      if (portString)
      {
         outPort = dAtoi(portString);
      }

      return true;
   }

   Net::Error getSocketAddress(SOCKET socketFd, int requiredFamily, NetAddress *outAddress)
   {
      Net::Error error = Net::UnknownError;

      if (requiredFamily == AF_INET)
      {
         sockaddr_in ipAddr;
         socklen_t len = sizeof(ipAddr);
         if (getsockname(socketFd, (struct sockaddr*)&ipAddr, &len) >= 0)
         {
            IPSocketToNetAddress(&ipAddr, outAddress);
            error = Net::NoError;
         }
         else
         {
            error = getLastError();
         }
      }
      else if (requiredFamily == AF_INET6)
      {
         sockaddr_in6 ipAddr;
         socklen_t len = sizeof(ipAddr);
         if (getsockname(socketFd, (struct sockaddr*)&ipAddr, &len) >= 0)
         {
            IPSocket6ToNetAddress(&ipAddr, outAddress);
            error = Net::NoError;
         }
         else
         {
            error = getLastError();
         }
      }

      return error;
   }
};



template<class T> NetSocket ReservedSocketList<T>::reserve(SOCKET reserveId, bool doLock)
{
   MutexHandle handle;
   if (doLock)
   {
      handle.lock(mMutex, true);
   }

   S32 idx = mSocketList.find_next(EntryType());
   if (idx == -1)
   {
      EntryType entry;
      entry.value = reserveId;
      entry.used = true;
      mSocketList.push_back(entry);
      return NetSocket::fromHandle(mSocketList.size() - 1);
   }
   else
   {
      EntryType &entry = mSocketList[idx];
      entry.used = true;
      entry.value = reserveId;
   }

   return NetSocket::fromHandle(idx);
}

template<class T> void ReservedSocketList<T>::remove(NetSocket socketToRemove, bool doLock)
{
   MutexHandle handle;
   if (doLock)
   {
      handle.lock(mMutex, true);
   }

   if ((U32)socketToRemove.getHandle() >= (U32)mSocketList.size())
      return;

   mSocketList[socketToRemove.getHandle()] = EntryType();
}

template<class T> T ReservedSocketList<T>::activate(NetSocket socketToActivate, int family, bool useUDP, bool clearOnFail)
{
   MutexHandle h;
   h.lock(mMutex, true);

   int typeID = useUDP ? SOCK_DGRAM : SOCK_STREAM;
   int protocol = useUDP ? PlatformNetState::getDefaultGameProtocol() : 0;

   if ((U32)socketToActivate.getHandle() >= (U32)mSocketList.size())
      return -1;

   EntryType &entry = mSocketList[socketToActivate.getHandle()];
   if (!entry.used)
      return -1;

   T socketFd = entry.value;
   if (socketFd == -1)
   {
      socketFd = ::socket(family, typeID, protocol);

      if (socketFd == InvalidSocketHandle)
      {
         if (clearOnFail)
         {
            remove(socketToActivate, false);
         }
         return InvalidSocketHandle;
      }
      else
      {
         entry.used = true;
         entry.value = socketFd;
         return socketFd;
      }
   }

   return socketFd;
}

template<class T> T ReservedSocketList<T>::resolve(NetSocket socketToResolve)
{
   MutexHandle h;
   h.lock(mMutex, true);

   if ((U32)socketToResolve.getHandle() >= (U32)mSocketList.size())
      return -1;

   EntryType &entry = mSocketList[socketToResolve.getHandle()];
   return entry.used ? entry.value : -1;
}

static ConnectionNotifyEvent*   smConnectionNotify = NULL;
static ConnectionAcceptedEvent* smConnectionAccept = NULL;
static ConnectionReceiveEvent*  smConnectionReceive = NULL;
static PacketReceiveEvent*      smPacketReceive = NULL;

ConnectionNotifyEvent& Net::getConnectionNotifyEvent()
{
   return *smConnectionNotify;
}

ConnectionAcceptedEvent& Net::getConnectionAcceptedEvent()
{
   return *smConnectionAccept;
}

ConnectionReceiveEvent& Net::getConnectionReceiveEvent()
{
   return *smConnectionReceive;
}

PacketReceiveEvent& Net::getPacketReceiveEvent()
{
   return *smPacketReceive;
}

// Multicast stuff
bool Net::smMulticastEnabled = true;
//
// Protocol Stuff
bool Net::smIpv4Enabled = true;
bool Net::smIpv6Enabled = false;
//

// the Socket structure helps us keep track of the
// above states
struct PolledSocket
{
   // local enum for socket states for polled sockets
   enum SocketState
   {
      InvalidState,
      Connected,
      ConnectionPending,
      Listening,
      NameLookupRequired
   };

   PolledSocket()
   {
      fd = -1;
      handleFd = NetSocket::INVALID;
      state = InvalidState;
      remoteAddr[0] = 0;
      remotePort = -1;
   }

   SOCKET fd;
   NetSocket handleFd;
   S32 state;
   char remoteAddr[256];
   S32 remotePort;
};

// list of polled sockets
static Vector<PolledSocket*> gPolledSockets( __FILE__, __LINE__ );

static PolledSocket* addPolledSocket(NetSocket handleFd, SOCKET fd, S32 state,
                               char* remoteAddr = NULL, S32 port = -1)
{
   PolledSocket* sock = new PolledSocket();
   sock->fd = fd;
   sock->handleFd = handleFd;
   sock->state = state;
   if (remoteAddr)
      dStrcpy(sock->remoteAddr, remoteAddr);
   if (port != -1)
      sock->remotePort = port;
   gPolledSockets.push_back(sock);
   return sock;
}

bool netSocketWaitForWritable(NetSocket handleFd, S32 timeoutMs)
{  
   fd_set writefds;
   timeval timeout;
   SOCKET socketFd = PlatformNetState::smReservedSocketList.resolve(handleFd);

   FD_ZERO( &writefds );
   FD_SET( socketFd, &writefds );

   timeout.tv_sec = timeoutMs / 1000;
   timeout.tv_usec = ( timeoutMs % 1000 ) * 1000;

   if( select(socketFd + 1, NULL, &writefds, NULL, &timeout) > 0 )
      return true;

   return false;
}

bool Net::init()
{
#if defined(TORQUE_USE_WINSOCK)
   if(!PlatformNetState::initCount)
   {
#ifdef TORQUE_OS_XENON
      // Configure startup parameters
      XNetStartupParams xnsp;
      memset( &xnsp, 0, sizeof( xnsp ) );
      xnsp.cfgSizeOfStruct = sizeof( XNetStartupParams );

#ifndef TORQUE_DISABLE_PC_CONNECTIVITY
      xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
      Con::warnf("XNET_STARTUP_BYPASS_SECURITY enabled! This build can talk to PCs!");
#endif

      AssertISV( !XNetStartup( &xnsp ), "Net::init - failed to init XNet" );
#endif

      WSADATA stWSAData;
      AssertISV( !WSAStartup( 0x0101, &stWSAData ), "Net::init - failed to init WinSock!" );

      //logprintf("Winsock initialization %s", success ? "succeeded." : "failed!");
   }
#endif
   PlatformNetState::initCount++;

   smConnectionNotify = new ConnectionNotifyEvent();
   smConnectionAccept = new ConnectionAcceptedEvent();
   smConnectionReceive = new ConnectionReceiveEvent();
   smPacketReceive = new PacketReceiveEvent();


   Process::notify(&Net::process, PROCESS_NET_ORDER);

   return(true);
}

void Net::shutdown()
{
   Process::remove(&Net::process);

   while (gPolledSockets.size() > 0)
   {
      if (gPolledSockets[0] == NULL)
         gPolledSockets.erase(gPolledSockets.begin());
      else
         closeConnectTo(gPolledSockets[0]->handleFd);
   }

   closePort();
   PlatformNetState::initCount--;

   // Destroy event handlers
   delete smConnectionNotify;
   delete smConnectionAccept;
   delete smConnectionReceive;
   delete smPacketReceive;

#if defined(TORQUE_USE_WINSOCK)
   if(!PlatformNetState::initCount)
   {
      WSACleanup();

#ifdef TORQUE_OS_XENON
      XNetCleanup();
#endif
   }
#endif
}

// ipv4 version of name routines

static void NetAddressToIPSocket(const NetAddress *address, struct sockaddr_in *sockAddr)
{
   dMemset(sockAddr, 0, sizeof(struct sockaddr_in));
   sockAddr->sin_family = AF_INET;
   sockAddr->sin_port = htons(address->port);
   #if defined(TORQUE_OS_BSD) || defined(TORQUE_OS_MAC)
   sockAddr->sin_len = sizeof(struct sockaddr_in);
   #endif
   if (address->type == NetAddress::IPBroadcastAddress)
   {
      sockAddr->sin_addr.s_addr = htonl(INADDR_BROADCAST);
   }
   else
   {
      dMemcpy(&sockAddr->sin_addr, &address->address.ipv4.netNum[0], 4);
   }
}

static void IPSocketToNetAddress(const struct sockaddr_in *sockAddr, NetAddress *address)
{
   address->type = NetAddress::IPAddress;
   address->port = ntohs(sockAddr->sin_port);
   dMemcpy(&address->address.ipv4.netNum[0], &sockAddr->sin_addr, 4);
}

// ipv6 version of name routines

static void NetAddressToIPSocket6(const NetAddress *address, struct sockaddr_in6 *sockAddr)
{
   dMemset(sockAddr, 0, sizeof(struct sockaddr_in6));
#ifdef SIN6_LEN
   sockAddr->sin6_len = sizeof(struct sockaddr_in6);
#endif
   sockAddr->sin6_family = AF_INET6;
   sockAddr->sin6_port = ntohs(address->port);

   if (address->type == NetAddress::IPV6MulticastAddress)
   {
      sockAddr->sin6_addr = PlatformNetState::multicast6Group.ipv6mr_multiaddr;
      sockAddr->sin6_scope_id = PlatformNetState::multicast6Group.ipv6mr_interface;
   }
   else
   {
      sockAddr->sin6_flowinfo = address->address.ipv6.netFlow;
      sockAddr->sin6_scope_id = address->address.ipv6.netScope;
      dMemcpy(&sockAddr->sin6_addr, address->address.ipv6.netNum, sizeof(address->address.ipv6.netNum));
   }
}

static void IPSocket6ToNetAddress(const struct sockaddr_in6 *sockAddr, NetAddress *address)
{
   address->type = NetAddress::IPV6Address;
   address->port = ntohs(sockAddr->sin6_port);
   dMemcpy(address->address.ipv6.netNum, &sockAddr->sin6_addr, sizeof(address->address.ipv6.netNum));
   address->address.ipv6.netFlow = sockAddr->sin6_flowinfo;
   address->address.ipv6.netScope = sockAddr->sin6_scope_id;
}

//

NetSocket Net::openListenPort(U16 port, NetAddress::Type addressType)
{
   if(Journal::IsPlaying())
   {
      U32 ret;
      Journal::Read(&ret);
      return NetSocket::fromHandle(ret);
   }

   Net::Error error = NoError;
   NetAddress address;
   if (Net::getListenAddress(addressType, &address) != Net::NoError)
      error = Net::WrongProtocolType;

   NetSocket handleFd = NetSocket::INVALID;
   SOCKET sockId = InvalidSocketHandle;

   if (error == NoError)
   {
      handleFd = openSocket();
      sockId = PlatformNetState::smReservedSocketList.activate(handleFd, address.type == NetAddress::IPAddress ? AF_INET : AF_INET6, false, true);
   }

   if (error == NoError && (handleFd == NetSocket::INVALID || sockId == InvalidSocketHandle))
   {
      Con::errorf("Unable to open listen socket: %s", strerror(errno));
      error = NotASocket;
      handleFd = NetSocket::INVALID;
   }

   if (error == NoError)
   {
      address.port = port;
      error = bindAddress(address, handleFd, false);
      if (error != NoError)
      {
         Con::errorf("Unable to bind port %d: %s", port, strerror(errno));
         closeSocket(handleFd);
         handleFd = NetSocket::INVALID;
      }
   }

   if (error == NoError)
   {
      error = listen(handleFd, 4);
      if (error != NoError)
      {
         Con::errorf("Unable to listen on port %d: %s", port, strerror(errno));
         closeSocket(handleFd);
         handleFd = NetSocket::INVALID;
      }
   }

   if (error == NoError)
   {
      setBlocking(handleFd, false);
      addPolledSocket(handleFd, sockId, PolledSocket::Listening);
   }

   if(Journal::IsRecording())
      Journal::Write(U32(handleFd.getHandle()));

   return handleFd;
}

NetSocket Net::openConnectTo(const char *addressString)
{
   if (Journal::IsPlaying())
   {
      U32 ret;
      Journal::Read(&ret);
      return NetSocket::fromHandle(ret);
   }

   NetAddress address;
   NetSocket handleFd = NetSocket::INVALID;
   Net::Error error = NoError;

   error = Net::stringToAddress(addressString, &address, false);

   if (error == NoError && address.type != NetAddress::IPAddress && address.type != NetAddress::IPV6Address)
   {
      error = Net::WrongProtocolType;
   }

   if (error != NoError || error == NeedHostLookup)
   {
      handleFd = openSocket();
   }

   // Attempt to connect or queue a lookup
   if (error == NoError && address.type == NetAddress::IPAddress)
   {
      sockaddr_in ipAddr;
      NetAddressToIPSocket(&address, &ipAddr);
      SOCKET socketFd = PlatformNetState::smReservedSocketList.activate(handleFd, AF_INET, false, true);
      if (socketFd != InvalidSocketHandle)
      {
         setBlocking(handleFd, false);
         if (::connect(socketFd, (struct sockaddr *)&ipAddr, sizeof(ipAddr)) == -1 &&
            errno != EINPROGRESS)
         {
            Con::errorf("Error connecting %s: %s",
               addressString, strerror(errno));
            closeSocket(handleFd);
            handleFd = NetSocket::INVALID;
         }
      }
      else
      {
         PlatformNetState::smReservedSocketList.remove(handleFd);
         handleFd = NetSocket::INVALID;
      }

      if (handleFd != NetSocket::INVALID)
      {
         // add this socket to our list of polled sockets
         addPolledSocket(handleFd, socketFd, PolledSocket::ConnectionPending);
      }
   }
   else if (error == NoError && address.type == NetAddress::IPV6Address)
   {
      sockaddr_in6 ipAddr6;
      NetAddressToIPSocket6(&address, &ipAddr6);
      SOCKET socketFd = PlatformNetState::smReservedSocketList.activate(handleFd, AF_INET6, false, true);
      if (::connect(socketFd, (struct sockaddr *)&ipAddr6, sizeof(ipAddr6)) == -1 &&
         errno != EINPROGRESS)
      {
         setBlocking(handleFd, false);
         Con::errorf("Error connecting %s: %s",
            addressString, strerror(errno));
         closeSocket(handleFd);
         handleFd = NetSocket::INVALID;
      }
      else
      {
         PlatformNetState::smReservedSocketList.remove(handleFd);
         handleFd = NetSocket::INVALID;
      }

      if (handleFd != NetSocket::INVALID)
      {
         // add this socket to our list of polled sockets
         addPolledSocket(handleFd, socketFd, PolledSocket::ConnectionPending);
      }
   }
   else if (error == Net::NeedHostLookup)
   {
      // need to do an asynchronous name lookup.  first, add the socket
      // to the polled list
      char addr[256];
      int port = 0;
      int actualFamily = AF_UNSPEC;
      if (PlatformNetState::extractAddressParts(addressString, addr, port, actualFamily))
      {
         addPolledSocket(handleFd, InvalidSocketHandle, PolledSocket::NameLookupRequired, addr, port);
         // queue the lookup
         gNetAsync.queueLookup(addressString, handleFd);
      }
      else
      {
         closeSocket(handleFd);
         handleFd = NetSocket::INVALID;
      }
   }
   else
   {
      closeSocket(handleFd);
      handleFd = NetSocket::INVALID;
   }

   if (Journal::IsRecording())
      Journal::Write(U32(handleFd.getHandle()));
   return handleFd;
}

void Net::closeConnectTo(NetSocket handleFd)
{
   if(Journal::IsPlaying())
      return;

   // if this socket is in the list of polled sockets, remove it
   for (S32 i = 0; i < gPolledSockets.size(); ++i)
   {
      if (gPolledSockets[i] && gPolledSockets[i]->handleFd == handleFd)
      {
         delete gPolledSockets[i];
         gPolledSockets[i] = NULL;
         break;
      }
   }

   closeSocket(handleFd);
}

Net::Error Net::sendtoSocket(NetSocket handleFd, const U8 *buffer, S32  bufferSize, S32 *outBufferWritten)
{
   if(Journal::IsPlaying())
   {
      U32 e;
      S32 outBytes;
      Journal::Read(&e);
      Journal::Read(&outBytes);
      if (outBufferWritten)
         *outBufferWritten = outBytes;

      return (Net::Error) e;
   }

   S32 outBytes = 0;
   Net::Error e = send(handleFd, buffer, bufferSize, &outBytes);

   if (Journal::IsRecording())
   {
      Journal::Write(U32(e));
      Journal::Write(outBytes);
   }

   if (outBufferWritten)
      *outBufferWritten = outBytes;

   return e;
}

bool Net::openPort(S32 port, bool doBind)
{
   if (PlatformNetState::udpSocket != NetSocket::INVALID)
   {
      closeSocket(PlatformNetState::udpSocket);
      PlatformNetState::udpSocket = NetSocket::INVALID;
   }
   if (PlatformNetState::udp6Socket != NetSocket::INVALID)
   {
      closeSocket(PlatformNetState::udp6Socket);
      PlatformNetState::udp6Socket = NetSocket::INVALID;
   }

   // Update prefs
   Net::smMulticastEnabled = Con::getBoolVariable("pref::Net::Multicast6Enabled", true);
   Net::smIpv4Enabled = Con::getBoolVariable("pref::Net::IPV4Enabled", true);
   Net::smIpv6Enabled = Con::getBoolVariable("pref::Net::IPV6Enabled", false);

   // we turn off VDP in non-release builds because VDP does not support broadcast packets
   // which are required for LAN queries (PC->Xbox connectivity).  The wire protocol still
   // uses the VDP packet structure, though.
   S32 protocol = PlatformNetState::getDefaultGameProtocol();

   SOCKET socketFd = InvalidSocketHandle;
   NetAddress address;
   NetAddress listenAddress;
   char listenAddressStr[256];

   if (Net::smIpv4Enabled)
   {
      if (Net::getListenAddress(NetAddress::IPAddress, &address) == Net::NoError)
      {
         address.port = port;
         socketFd = ::socket(AF_INET, SOCK_DGRAM, protocol);
         
         if (socketFd != InvalidSocketHandle)
         {
            PlatformNetState::udpSocket = PlatformNetState::smReservedSocketList.reserve(socketFd);
            Net::Error error = NoError;
            if (doBind)
            {
               error = bindAddress(address, PlatformNetState::udpSocket, true);
            }
            
            if (error == NoError)
               error = setBufferSize(PlatformNetState::udpSocket, 32768 * 8);
               
#ifndef TORQUE_DISABLE_PC_CONNECTIVITY
            if (error == NoError)
               error = setBroadcast(PlatformNetState::udpSocket, true);
#endif
               
            if (error == NoError)
               error = setBlocking(PlatformNetState::udpSocket, false);

            if (error == NoError)
            {
               error = PlatformNetState::getSocketAddress(socketFd, AF_INET, &listenAddress);
               if (error == NoError)
               {
                  Net::addressToString(&listenAddress, listenAddressStr);
                  Con::printf("UDP initialized on ipv4 %s", listenAddressStr);
               }
            }

            if (error != NoError)
            {
               closeSocket(PlatformNetState::udpSocket);
               PlatformNetState::udpSocket = NetSocket::INVALID;
               Con::printf("Unable to initialize UDP on ipv4 - error %d", error);
            }
         }
      }
      else
      {
         Con::errorf("Unable to initialize UDP on ipv4 - invalid address.");
         PlatformNetState::udpSocket = NetSocket::INVALID;
      }
   }
   
   if (Net::smIpv6Enabled)
   {
      if (Net::getListenAddress(NetAddress::IPV6Address, &address) == Net::NoError)
      {
         address.port = port;
         socketFd = ::socket(AF_INET6, SOCK_DGRAM, protocol);

         if (socketFd != InvalidSocketHandle)
         {
            PlatformNetState::udp6Socket = PlatformNetState::smReservedSocketList.reserve(socketFd);

            Net::Error error = NoError;

            int v = 1;
            setsockopt(socketFd, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&v, sizeof(v));
            PlatformNetState::getLastError();

            if (doBind)
            {
               error = bindAddress(address, PlatformNetState::udp6Socket, true);
            }

            if (error == NoError)
               error = setBufferSize(PlatformNetState::udp6Socket, 32768 * 8);

            if (error == NoError)
               error = setBlocking(PlatformNetState::udp6Socket, false);

            if (error == NoError)
            {
               error = PlatformNetState::getSocketAddress(socketFd, AF_INET6, &listenAddress);
               if (error == NoError)
               {
                  Net::addressToString(&listenAddress, listenAddressStr);
                  Con::printf("UDP initialized on ipv6 %s", listenAddressStr);
               }
            }
            
            if (error != NoError)
            {
               closeSocket(PlatformNetState::udp6Socket);
               PlatformNetState::udp6Socket = NetSocket::INVALID;
               Con::printf("Unable to initialize UDP on ipv6 - error %d", error);
            }

            if (Net::smMulticastEnabled && doBind)
            {
               Net::enableMulticast();
            }
            else
            {
               Net::disableMulticast();
            }
         }
      }
   }

   PlatformNetState::netPort = port;

   return PlatformNetState::udpSocket != NetSocket::INVALID || PlatformNetState::udp6Socket != NetSocket::INVALID;
}

NetSocket Net::getPort()
{
   return PlatformNetState::udpSocket;
}

void Net::closePort()
{
   if (PlatformNetState::udpSocket != NetSocket::INVALID)
      closeSocket(PlatformNetState::udpSocket);
   if (PlatformNetState::udp6Socket != NetSocket::INVALID)
      closeSocket(PlatformNetState::udp6Socket);
}

Net::Error Net::sendto(const NetAddress *address, const U8 *buffer, S32  bufferSize)
{
   if(Journal::IsPlaying())
      return NoError;

   SOCKET socketFd;

   if(address->type == NetAddress::IPAddress || address->type == NetAddress::IPBroadcastAddress)
   {
      socketFd = PlatformNetState::smReservedSocketList.resolve(PlatformNetState::udpSocket);
      if (socketFd != InvalidSocketHandle)
      {
         sockaddr_in ipAddr;
         NetAddressToIPSocket(address, &ipAddr);

         if (::sendto(socketFd, (const char*)buffer, bufferSize, 0,
            (sockaddr *)&ipAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
            return PlatformNetState::getLastError();
         else
            return NoError;
      }
      else
      {
         return NotASocket;
      }
   }
   else if (address->type == NetAddress::IPV6Address || address->type == NetAddress::IPV6MulticastAddress)
   {
      socketFd = PlatformNetState::smReservedSocketList.resolve(address->type == NetAddress::IPV6MulticastAddress ? PlatformNetState::multicast6Socket : PlatformNetState::udp6Socket);

      if (socketFd != InvalidSocketHandle)
      {
         sockaddr_in6 ipAddr;
         NetAddressToIPSocket6(address, &ipAddr);
         if (::sendto(socketFd, (const char*)buffer, bufferSize, 0,
          (struct sockaddr *) &ipAddr, sizeof(sockaddr_in6)) == SOCKET_ERROR)
            return PlatformNetState::getLastError();
         else
            return NoError;
      }
      else
      {
         return NotASocket;
      }
   }

   return WrongProtocolType;
}

void Net::process()
{
   // Process listening sockets
   processListenSocket(PlatformNetState::udpSocket);
   processListenSocket(PlatformNetState::udp6Socket);

   // process the polled sockets.  This blob of code performs functions
   // similar to WinsockProc in winNet.cc

   if (gPolledSockets.size() == 0)
      return;

   S32 optval;
   socklen_t optlen = sizeof(S32);
   S32 bytesRead;
   Net::Error err;
   bool removeSock = false;
   PolledSocket *currentSock = NULL;
   NetSocket incomingHandleFd = NetSocket::INVALID;
   NetAddress out_h_addr;
   S32 out_h_length = 0;
   RawData readBuff;
   NetSocket removeSockHandle;

   for (S32 i = 0; i < gPolledSockets.size();
      /* no increment, this is done at end of loop body */)
   {
      removeSock = false;
      currentSock = gPolledSockets[i];
      
      // Cleanup if we've removed it
      if (currentSock == NULL)
      {
         gPolledSockets.erase(i);
         continue;
      }
      
      switch (currentSock->state)
      {
      case PolledSocket::InvalidState:
         Con::errorf("Error, InvalidState socket in polled sockets  list");
         break;
      case PolledSocket::ConnectionPending:
         // see if it is now connected
#ifdef TORQUE_OS_XENON
         // WSASetLastError has no return value, however part of the SO_ERROR behavior
         // is to clear the last error, so this needs to be done here.
         if( ( optval = _getLastErrorAndClear() ) == -1 ) 
#else
         if (getsockopt(currentSock->fd, SOL_SOCKET, SO_ERROR,
            (char*)&optval, &optlen) == -1)
#endif
         {
            Con::errorf("Error getting socket options: %s",  strerror(errno));
            
            removeSock = true;
            removeSockHandle = currentSock->handleFd;

            smConnectionNotify->trigger(currentSock->handleFd, Net::ConnectFailed);
         }
         else
         {
            if (optval == EINPROGRESS)
               // still connecting...
               break;

            if (optval == 0)
            {
               // poll for writable status to be sure we're connected.
               bool ready = netSocketWaitForWritable(currentSock->handleFd,0);
               if(!ready)
                  break;

               currentSock->state = PolledSocket::Connected;
               smConnectionNotify->trigger(currentSock->handleFd, Net::Connected);
            }
            else
            {
               // some kind of error
               Con::errorf("Error connecting: %s", strerror(errno));
               
               removeSock = true;
               removeSockHandle = currentSock->handleFd;
               
               smConnectionNotify->trigger(currentSock->handleFd, Net::ConnectFailed);
            }
         }
         break;
      case PolledSocket::Connected:

         // try to get some data
         bytesRead = 0;
         readBuff.alloc(MaxPacketDataSize);
         err = Net::recv(currentSock->handleFd, (U8*)readBuff.data, MaxPacketDataSize, &bytesRead);
         if(err == Net::NoError)
         {
            if (bytesRead > 0)
            {
               // got some data, post it
               readBuff.size = bytesRead;
               smConnectionReceive->trigger(currentSock->handleFd, readBuff);
            }
            else
            {
               // ack! this shouldn't happen
               if (bytesRead < 0)
                  Con::errorf("Unexpected error on socket: %s", strerror(errno));
               
               removeSock = true;
               removeSockHandle = currentSock->handleFd;

               // zero bytes read means EOF
               smConnectionNotify->trigger(currentSock->handleFd, Net::Disconnected);
            }
         }
         else if (err != Net::NoError && err != Net::WouldBlock)
         {
            Con::errorf("Error reading from socket: %s",  strerror(errno));
            
            removeSock = true;
            removeSockHandle = currentSock->handleFd;
            
            smConnectionNotify->trigger(currentSock->handleFd, Net::Disconnected);
         }
         break;
      case PolledSocket::NameLookupRequired:
         U32 newState;

         // is the lookup complete?
         if (!gNetAsync.checkLookup(
            currentSock->handleFd, &out_h_addr, &out_h_length,
            sizeof(out_h_addr)))
            break;

         if (out_h_length == -1)
         {
            Con::errorf("DNS lookup failed: %s", currentSock->remoteAddr);
            newState = Net::DNSFailed;
            removeSock = true;
            removeSockHandle = currentSock->handleFd;
         }
         else
         {
            // try to connect
            out_h_addr.port = currentSock->remotePort;
            const sockaddr *ai_addr = NULL;
            int ai_addrlen = 0;
            sockaddr_in socketAddress;
            sockaddr_in6 socketAddress6;

            if (out_h_addr.type == NetAddress::IPAddress)
            {
               ai_addr = (const sockaddr*)&socketAddress;
               ai_addrlen = sizeof(socketAddress);
               NetAddressToIPSocket(&out_h_addr, &socketAddress);

               currentSock->fd = PlatformNetState::smReservedSocketList.activate(currentSock->handleFd, AF_INET, false);
               setBlocking(currentSock->handleFd, false);

#ifdef TORQUE_DEBUG_LOOKUPS
               char addrString[256];
               NetAddress addr;
               IPSocketToNetAddress(&socketAddress, &addr);
               Net::addressToString(&addr, addrString);
               Con::printf("DNS: lookup resolved to %s", addrString);
#endif
            }
            else if (out_h_addr.type == NetAddress::IPV6Address)
            {
               ai_addr = (const sockaddr*)&socketAddress6;
               ai_addrlen = sizeof(socketAddress6);
               NetAddressToIPSocket6(&out_h_addr, &socketAddress6);

               currentSock->fd = PlatformNetState::smReservedSocketList.activate(currentSock->handleFd, AF_INET6, false);
               setBlocking(currentSock->handleFd, false);

#ifdef TORQUE_DEBUG_LOOKUPS
               char addrString[256];
               NetAddress addr;
               IPSocket6ToNetAddress(&socketAddress6, &addr);
               Net::addressToString(&addr, addrString);
               Con::printf("DNS: lookup resolved to %s", addrString);
#endif
            }
            else
            {
               Con::errorf("Error connecting to %s: Invalid Protocol",
               currentSock->remoteAddr);
               newState = Net::ConnectFailed;
               removeSock = true;
               removeSockHandle = currentSock->handleFd;
            }

            if (ai_addr)
            {
               if (::connect(currentSock->fd, ai_addr,
                  ai_addrlen) == -1)
               {
                  err = PlatformNetState::getLastError();
                  if (err != Net::WouldBlock)
                  {
                     Con::errorf("Error connecting to %s: %u",
                     currentSock->remoteAddr, err);
                     newState = Net::ConnectFailed;
                     removeSock = true;
                     removeSockHandle = currentSock->handleFd;
                  }
                  else
                  {
                     newState = Net::DNSResolved;
                     currentSock->state = PolledSocket::ConnectionPending;
                  }
               }
               else
               {
                  newState = Net::Connected;
                  currentSock->state = Connected;
               }
            }
         }

         smConnectionNotify->trigger(currentSock->handleFd, newState);
         break;
      case PolledSocket::Listening:
         NetAddress incomingAddy;

         incomingHandleFd = Net::accept(currentSock->handleFd, &incomingAddy);
         if(incomingHandleFd != NetSocket::INVALID)
         {
            setBlocking(incomingHandleFd, false);
            addPolledSocket(incomingHandleFd, PlatformNetState::smReservedSocketList.resolve(incomingHandleFd), Connected);
            smConnectionAccept->trigger(currentSock->handleFd, incomingHandleFd, incomingAddy);
         }
         break;
      }

      // only increment index if we're not removing the connection,  since
      // the removal will shift the indices down by one
      if (removeSock)
         closeConnectTo(removeSockHandle);
      else
         i++;
   }
}

void Net::processListenSocket(NetSocket socketHandle)
{
   if (socketHandle == NetSocket::INVALID)
      return;

   sockaddr_storage sa;
   sa.ss_family = AF_UNSPEC;
   NetAddress srcAddress;
   RawData tmpBuffer;
   tmpBuffer.alloc(Net::MaxPacketDataSize);

   SOCKET socketFd = PlatformNetState::smReservedSocketList.resolve(socketHandle);

   for (;;)
   {
      socklen_t addrLen = sizeof(sa);
      S32 bytesRead = -1;

      if (socketHandle != NetSocket::INVALID)
         bytesRead = ::recvfrom(socketFd, (char *)tmpBuffer.data, Net::MaxPacketDataSize, 0, (struct sockaddr*)&sa, &addrLen);

      if (bytesRead == -1)
         break;

      if (sa.ss_family == AF_INET)
         IPSocketToNetAddress((sockaddr_in *)&sa, &srcAddress);
      else if (sa.ss_family == AF_INET6)
        IPSocket6ToNetAddress((sockaddr_in6 *)&sa, &srcAddress);
     else
         continue;

      if (bytesRead <= 0)
         continue;

      if (srcAddress.type == NetAddress::IPAddress &&
         srcAddress.address.ipv4.netNum[0] == 127 &&
         srcAddress.address.ipv4.netNum[1] == 0 &&
         srcAddress.address.ipv4.netNum[2] == 0 &&
         srcAddress.address.ipv4.netNum[3] == 1 &&
         srcAddress.port == PlatformNetState::netPort)
         continue;

      tmpBuffer.size = bytesRead;

      smPacketReceive->trigger(srcAddress, tmpBuffer);
   }
}

NetSocket Net::openSocket()
{
   return PlatformNetState::smReservedSocketList.reserve();
}

Net::Error Net::closeSocket(NetSocket handleFd)
{
   if(handleFd != NetSocket::INVALID)
   {
      SOCKET socketFd = PlatformNetState::smReservedSocketList.resolve(handleFd);
     PlatformNetState::smReservedSocketList.remove(handleFd);

      if(!::closesocket(socketFd))
         return NoError;
      else
         return PlatformNetState::getLastError();
   }
   else
      return NotASocket;
}

Net::Error Net::connect(NetSocket handleFd, const NetAddress *address)
{
   if(!(address->type == NetAddress::IPAddress || address->type == NetAddress::IPV6Address))
      return WrongProtocolType;

   SOCKET socketFd = PlatformNetState::smReservedSocketList.resolve(handleFd);

   if (address->type == NetAddress::IPAddress)
   {
      sockaddr_in socketAddress;
      NetAddressToIPSocket(address, &socketAddress);

      if (socketFd == InvalidSocketHandle)
      {
         socketFd = PlatformNetState::smReservedSocketList.activate(handleFd, AF_INET, false);
      }

      if (!::connect(socketFd, (struct sockaddr *) &socketAddress, sizeof(socketAddress)))
         return NoError;
   }
   else if (address->type == NetAddress::IPV6Address)
   {
      sockaddr_in6 socketAddress;
      NetAddressToIPSocket6(address, &socketAddress);

      if (socketFd == InvalidSocketHandle)
      {
         socketFd = PlatformNetState::smReservedSocketList.activate(handleFd, AF_INET6, false);
      }

      if (!::connect(socketFd, (struct sockaddr *) &socketAddress, sizeof(socketAddress)))
         return NoError;
   }

   return PlatformNetState::getLastError();
}

Net::Error Net::listen(NetSocket handleFd, S32 backlog)
{
   SOCKET socketFd = PlatformNetState::smReservedSocketList.resolve(handleFd);
   if (socketFd == InvalidSocketHandle)
      return NotASocket;

   if(!::listen(socketFd, backlog))
      return NoError;
   return PlatformNetState::getLastError();
}

NetSocket Net::accept(NetSocket handleFd, NetAddress *remoteAddress)
{
   sockaddr_storage addr;
   socklen_t addrLen = sizeof(addr);

   SOCKET socketFd = PlatformNetState::smReservedSocketList.resolve(handleFd);
   if (socketFd == InvalidSocketHandle)
      return NetSocket::INVALID;

   SOCKET acceptedSocketFd = ::accept(socketFd, (sockaddr *)&addr, &addrLen);
   if (acceptedSocketFd != InvalidSocketHandle)
   {
      if (addr.ss_family == AF_INET)
      {
         // ipv4
         IPSocketToNetAddress(((struct sockaddr_in*)&addr), remoteAddress);
      }
      else if (addr.ss_family == AF_INET6)
      {
         // ipv6
         IPSocket6ToNetAddress(((struct sockaddr_in6*)&addr), remoteAddress);
      }

      NetSocket newHandleFd = PlatformNetState::smReservedSocketList.reserve(acceptedSocketFd);
      return newHandleFd;
   }

   return NetSocket::INVALID;
}

Net::Error Net::bindAddress(const NetAddress &address, NetSocket handleFd, bool useUDP)
{
   int error = 0;
   sockaddr_storage socketAddress;

   dMemset(&socketAddress, '\0', sizeof(socketAddress));

   SOCKET socketFd = PlatformNetState::smReservedSocketList.resolve(handleFd);
   if (socketFd == InvalidSocketHandle)
   {
      if (handleFd.getHandle() == -1)
         return NotASocket;
   }

   if (address.type == NetAddress::IPAddress)
   {
      socketFd = PlatformNetState::smReservedSocketList.activate(handleFd, AF_INET, useUDP);
      NetAddressToIPSocket(&address, (struct sockaddr_in*)&socketAddress);
      error = ::bind(socketFd, (struct sockaddr*)&socketAddress, sizeof(sockaddr_in));
   }
   else if (address.type == NetAddress::IPV6Address)
   {
      socketFd = PlatformNetState::smReservedSocketList.activate(handleFd, AF_INET6, useUDP);
      NetAddressToIPSocket6(&address, (struct sockaddr_in6*)&socketAddress);
      error = ::bind(socketFd, (struct sockaddr*)&socketAddress, sizeof(sockaddr_in6));
   }

   if (!error)
      return NoError;
   return PlatformNetState::getLastError();
}

Net::Error Net::setBufferSize(NetSocket handleFd, S32 bufferSize)
{
   S32 error;
   SOCKET socketFd = PlatformNetState::smReservedSocketList.resolve(handleFd);
   if (socketFd == InvalidSocketHandle)
      return NotASocket;

   error = ::setsockopt(socketFd, SOL_SOCKET, SO_RCVBUF, (char *)  &bufferSize, sizeof(bufferSize));
   if(!error)
      error = ::setsockopt(socketFd, SOL_SOCKET, SO_SNDBUF, (char *)  &bufferSize, sizeof(bufferSize));
   if(!error)
      return NoError;
   return PlatformNetState::getLastError();
}

Net::Error Net::setBroadcast(NetSocket handleFd, bool broadcast)
{
   S32 bc = broadcast;
   SOCKET socketFd = PlatformNetState::smReservedSocketList.resolve(handleFd);
   if (socketFd == InvalidSocketHandle)
      return NotASocket;
   S32 error = ::setsockopt(socketFd, SOL_SOCKET, SO_BROADCAST, (char*)&bc,  sizeof(bc));
   if(!error)
      return NoError;
   return PlatformNetState::getLastError();
}

Net::Error Net::setBlocking(NetSocket handleFd, bool blockingIO)
{
   SOCKET socketFd = PlatformNetState::smReservedSocketList.resolve(handleFd);
   if (socketFd == InvalidSocketHandle)
      return NotASocket;

   unsigned long notblock = !blockingIO;
   S32 error = ioctl(socketFd, FIONBIO, &notblock);
   if(!error)
      return NoError;
   return PlatformNetState::getLastError();
}

Net::Error Net::getListenAddress(const NetAddress::Type type, NetAddress *address, bool forceDefaults)
{
   if (type == NetAddress::IPAddress)
   {
      const char* serverIP = forceDefaults ? NULL : Con::getVariable("pref::Net::BindAddress");
      if (!serverIP || serverIP[0] == '\0')
      {
         address->type = type;
         address->port = 0;
         *((U32*)address->address.ipv4.netNum) = INADDR_ANY;
         return Net::NoError;
      }
      else
      {
         return Net::stringToAddress(serverIP, address, false);
      }
   }
   else if (type == NetAddress::IPBroadcastAddress)
   {
      address->type = type;
      address->port = 0;
      *((U32*)address->address.ipv4.netNum) = INADDR_BROADCAST;
      return Net::NoError;
   }
   else if (type == NetAddress::IPV6Address)
   {
      const char* serverIP6 = forceDefaults ? NULL : Con::getVariable("pref::Net::BindAddress6");
      if (!serverIP6 || serverIP6[0] == '\0')
      {
         sockaddr_in6 addr;
         dMemset(&addr, '\0', sizeof(addr));

         addr.sin6_port = 0;
         addr.sin6_addr = in6addr_any;

         IPSocket6ToNetAddress(&addr, address);
         return Net::NoError;
      }
      else
      {
         return Net::stringToAddress(serverIP6, address, false);
      }
   }
   else if (type == NetAddress::IPV6MulticastAddress)
   {
      const char* multicastAddressValue = forceDefaults ? NULL : Con::getVariable("pref::Net::Multicast6Address");
      if (!multicastAddressValue || multicastAddressValue[0] == '\0')
      {
         multicastAddressValue = TORQUE_NET_DEFAULT_MULTICAST_ADDRESS;
      }

      return Net::stringToAddress(multicastAddressValue, address, false);
   }
   else
   {
      return Net::WrongProtocolType;
   }
}

void Net::getIdealListenAddress(NetAddress *address)
{
   dMemset(address, '\0', sizeof(NetAddress));

   if (Net::smIpv6Enabled)
   {
      if (Net::getListenAddress(NetAddress::IPV6Address, address) == NeedHostLookup)
      {
         Net::getListenAddress(NetAddress::IPV6Address, address, true);
      }
   }
   else
   {
      if (Net::getListenAddress(NetAddress::IPAddress, address) == NeedHostLookup)
      {
         Net::getListenAddress(NetAddress::IPAddress, address, true);
      }
   }
}

Net::Error Net::send(NetSocket handleFd, const U8 *buffer, S32 bufferSize, S32 *outBytesWritten)
{
   SOCKET socketFd = PlatformNetState::smReservedSocketList.resolve(handleFd);
   if (socketFd == InvalidSocketHandle)
      return NotASocket;

   errno = 0;
   S32 bytesWritten = ::send(socketFd, (const char*)buffer, bufferSize, 0);

   if (outBytesWritten)
   {
      *outBytesWritten = outBytesWritten < 0 ? 0 : bytesWritten;
   }

   return PlatformNetState::getLastError();
}

Net::Error Net::recv(NetSocket handleFd, U8 *buffer, S32 bufferSize, S32  *bytesRead)
{
   SOCKET socketFd = PlatformNetState::smReservedSocketList.resolve(handleFd);
   if (socketFd == InvalidSocketHandle)
      return NotASocket;

   *bytesRead = ::recv(socketFd, (char*)buffer, bufferSize, 0);
   if(*bytesRead == -1)
      return PlatformNetState::getLastError();
   return NoError;
}

bool Net::compareAddresses(const NetAddress *a1, const NetAddress *a2)
{
   return a1->isSameAddressAndPort(*a2);
}

Net::Error Net::stringToAddress(const char *addressString, NetAddress  *address, bool hostLookup, int requiredFamily)
{
   char addr[256];
   int port = 0;
   int actualFamily = AF_UNSPEC;
   if (!PlatformNetState::extractAddressParts(addressString, addr, port, actualFamily))
   {
      return WrongProtocolType;
   }
   
   // Make sure family matches (in cast we have IP: stuff in address)
   if (requiredFamily != AF_UNSPEC && actualFamily != AF_UNSPEC && (actualFamily != requiredFamily))
   {
      return WrongProtocolType;
   }
   
   if (actualFamily == AF_UNSPEC)
   {
      actualFamily = requiredFamily;
   }
   
   addressString = addr;
   dMemset(address, '\0', sizeof(NetAddress));
   
   if (!dStricmp(addressString, "broadcast"))
   {
      address->type = NetAddress::IPBroadcastAddress;
      if (!(actualFamily == AF_UNSPEC || actualFamily == AF_INET))
         return WrongProtocolType;
      
      if (port != 0)
         address->port = port;
      else
         address->port = PlatformNetState::defaultPort;
   }
   else if (!dStricmp(addressString, "multicast"))
   {
      address->type = NetAddress::IPV6MulticastAddress;
      if (!(actualFamily == AF_UNSPEC || actualFamily == AF_INET6))
         return WrongProtocolType;
      
      if (port != 0)
         address->port = port;
      else
         address->port = PlatformNetState::defaultPort;
   }
   else
   {
      sockaddr_in ipAddr;
      sockaddr_in6 ipAddr6;
      
      dMemset(&ipAddr, 0, sizeof(ipAddr));
      dMemset(&ipAddr6, 0, sizeof(ipAddr6));
      
      bool hasInterface = dStrchr(addressString, '%') != NULL; // if we have an interface, best use getaddrinfo to parse
      
      // Check if we've got a simple ipv4 / ipv6
      
      if (inet_pton(AF_INET, addressString, &ipAddr.sin_addr) == 1)
      {
         if (!(actualFamily == AF_UNSPEC || actualFamily == AF_INET))
            return WrongProtocolType;
         IPSocketToNetAddress(((struct sockaddr_in*)&ipAddr), address);
         
         if (port != 0)
            address->port = port;
         else
            address->port = PlatformNetState::defaultPort;
         
         return NoError;
      }
      else if (!hasInterface && inet_pton(AF_INET6, addressString, &ipAddr6.sin6_addr) == 1)
      {
         if (!(actualFamily == AF_UNSPEC || actualFamily == AF_INET6))
            return WrongProtocolType;
         IPSocket6ToNetAddress(((struct sockaddr_in6*)&ipAddr6), address);
         
         if (port != 0)
            address->port = port;
         else
            address->port = PlatformNetState::defaultPort;
         
         return NoError;
      }
      else
      {
         if (!hostLookup && !hasInterface)
            return NeedHostLookup;
         
         struct addrinfo hint, *res = NULL;
         dMemset(&hint, 0, sizeof(hint));
         hint.ai_family = actualFamily;
         hint.ai_flags = hostLookup ? 0 : AI_NUMERICHOST;
         
         if (getaddrinfo(addressString, NULL, &hint, &res) == 0)
         {
            if (actualFamily != AF_UNSPEC)
            {
               // Prefer desired protocol
               res = PlatformNetState::pickAddressByProtocol(res, actualFamily);
            }
            
            if (res && res->ai_family == AF_INET)
            {
               // ipv4
               IPSocketToNetAddress(((struct sockaddr_in*)res->ai_addr), address);
            }
            else if (res && res->ai_family == AF_INET6)
            {
               // ipv6
               IPSocket6ToNetAddress(((struct sockaddr_in6*)res->ai_addr), address);
            }
            else
            {
               // unknown
               return UnknownError;
            }
            
            if (port != 0)
               address->port = port;
            else
               address->port = PlatformNetState::defaultPort;
         }
      }
   }
   
   return NoError;
}

void Net::addressToString(const NetAddress *address, char  addressString[256])
{
   if(address->type == NetAddress::IPAddress || address->type == NetAddress::IPBroadcastAddress)
   {
      sockaddr_in ipAddr;
      NetAddressToIPSocket(address, &ipAddr);
      
      if (ipAddr.sin_addr.s_addr == htonl(INADDR_BROADCAST) || address->type == NetAddress::IPBroadcastAddress)
      {
         if (ipAddr.sin_port == 0)
            dSprintf(addressString, 256, "IP:Broadcast");
         else
            dSprintf(addressString, 256, "IP:Broadcast:%d", ntohs(ipAddr.sin_port));
      }
      else
      {
         char buffer[256];
         buffer[0] = '\0';
         sockaddr_in ipAddr;
         NetAddressToIPSocket(address, &ipAddr);
         inet_ntop(AF_INET, &(ipAddr.sin_addr), buffer, sizeof(buffer));
         if (ipAddr.sin_port == 0)
            dSprintf(addressString, 256, "IP:%s", buffer);
         else
            dSprintf(addressString, 256, "IP:%s:%i", buffer, ntohs(ipAddr.sin_port));
      }
   }
   else if (address->type == NetAddress::IPV6Address)
   {
      char buffer[256];
      buffer[0] = '\0';
      sockaddr_in6 ipAddr;
      NetAddressToIPSocket6(address, &ipAddr);
      inet_ntop(AF_INET6, &(ipAddr.sin6_addr), buffer, sizeof(buffer));
      if (ipAddr.sin6_port == 0)
         dSprintf(addressString, 256, "IP6:%s", buffer);
      else
         dSprintf(addressString, 256, "IP6:[%s]:%i", buffer, ntohs(ipAddr.sin6_port));
   }
   else if (address->type == NetAddress::IPV6MulticastAddress)
   {
      if (address->port == 0)
         dSprintf(addressString, 256, "IP6:Multicast");
      else
         dSprintf(addressString, 256, "IP6:Multicast:%d", address->port);
   }
   else
   {
      *addressString = 0;
      return;
   }
}

void Net::enableMulticast()
{
   SOCKET socketFd;

   if (Net::smIpv6Enabled)
   {
      socketFd = PlatformNetState::smReservedSocketList.resolve(PlatformNetState::udp6Socket);

      if (socketFd != InvalidSocketHandle)
      {
         PlatformNetState::multicast6Socket = PlatformNetState::udp6Socket;

         Net::Error error = NoError;

         if (error == NoError)
         {
            unsigned long multicastTTL = 1;

            if (setsockopt(socketFd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
               (char*)&multicastTTL, sizeof(multicastTTL)) < 0)
            {
               error = PlatformNetState::getLastError();
            }
         }

         // Find multicast to bind to...

         NetAddress multicastAddress;
         sockaddr_in6 multicastSocketAddress;

         const char *multicastAddressValue = Con::getVariable("pref::Net::Multicast6Address");
         if (!multicastAddressValue || multicastAddressValue[0] == '\0')
         {
            multicastAddressValue = TORQUE_NET_DEFAULT_MULTICAST_ADDRESS;
         }

         error = Net::stringToAddress(multicastAddressValue, &multicastAddress, false);

         if (error == NoError)
         {
            dMemset(&PlatformNetState::multicast6Group, '\0', sizeof(&PlatformNetState::multicast6Group));
            NetAddressToIPSocket6(&multicastAddress, &multicastSocketAddress);
            dMemcpy(&PlatformNetState::multicast6Group.ipv6mr_multiaddr, &multicastSocketAddress.sin6_addr, sizeof(PlatformNetState::multicast6Group.ipv6mr_multiaddr));
         }

         // Setup group

         if (error == NoError)
         {
            const char *multicastInterface = Con::getVariable("pref::Net::Multicast6Interface");

            if (multicastInterface && multicastInterface[0] != '\0')
            {
#ifdef TORQUE_USE_WINSOCK
               PlatformNetState::multicast6Group.ipv6mr_interface = dAtoi(multicastInterface);
#else
               PlatformNetState::multicast6Group.ipv6mr_interface = if_nametoindex(multicastInterface);
#endif
            }
            else
            {
               PlatformNetState::multicast6Group.ipv6mr_interface = 0; // 0 == accept from any interface
            }

            if (PlatformNetState::multicast6Group.ipv6mr_interface && error == NoError)
            {
               if (setsockopt(socketFd, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char *)&PlatformNetState::multicast6Group.ipv6mr_interface, sizeof(PlatformNetState::multicast6Group.ipv6mr_interface)) < 0)
               {
                  error = PlatformNetState::getLastError();
               }
            }

            if (error == NoError && setsockopt(socketFd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char*)&PlatformNetState::multicast6Group, sizeof(PlatformNetState::multicast6Group)) < 0)
            {
               error = PlatformNetState::getLastError();
            }
         }


         if (error == NoError)
         {
            NetAddress listenAddress;
            char listenAddressStr[256];
            Net::addressToString(&multicastAddress, listenAddressStr);
            Con::printf("Multicast initialized on %s", listenAddressStr);
         }

          if (error != NoError)
         {
            PlatformNetState::multicast6Socket = NetSocket::INVALID;
            Con::printf("Unable to multicast UDP - error %d", error);
         }
      }
   }
}

void Net::disableMulticast()
{
   if (PlatformNetState::multicast6Socket != NetSocket::INVALID)
   {
      PlatformNetState::multicast6Socket = NetSocket::INVALID;
   }
}

bool Net::isMulticastEnabled()
{
   return PlatformNetState::multicast6Socket != NetSocket::INVALID;
}

U32 NetAddress::getHash() const
{
   U32 value = 0;
   switch (type)
   {
   case NetAddress::IPAddress:
      value = Torque::hash((const U8*)&address.ipv4.netNum, sizeof(address.ipv4.netNum), 0);
      break;
   case NetAddress::IPV6Address:
      value = Torque::hash((const U8*)address.ipv6.netNum, sizeof(address.ipv6.netNum), 0);
      break;
   default:
      value = 0;
      break;
   }
   return value;
}

bool Net::isAddressTypeAvailable(NetAddress::Type addressType)
{
   switch (addressType)
   {
   case NetAddress::IPAddress:
      return PlatformNetState::udpSocket != NetSocket::INVALID;
   case NetAddress::IPV6Address:
      return PlatformNetState::udp6Socket != NetSocket::INVALID;
   case NetAddress::IPBroadcastAddress:
      return PlatformNetState::udpSocket != NetSocket::INVALID;
   case NetAddress::IPV6MulticastAddress:
      return PlatformNetState::multicast6Socket != NetSocket::INVALID;
   default:
      return false;
   }
}
