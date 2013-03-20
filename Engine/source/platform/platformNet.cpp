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
#include "core/strings/stringFunctions.h"

#if defined (TORQUE_OS_WIN32)
#define TORQUE_USE_WINSOCK
#include <errno.h>
#include <winsock.h>
#define EINPROGRESS             WSAEINPROGRESS
#define ioctl ioctlsocket

typedef int socklen_t;

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

typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr * PSOCKADDR;
typedef sockaddr SOCKADDR;
typedef in_addr IN_ADDR;

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

#define closesocket close

#elif defined TORQUE_OS_LINUX

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/ioctl.h>

typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr * PSOCKADDR;
typedef sockaddr SOCKADDR;
typedef in_addr IN_ADDR;

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

#define closesocket close

#elif defined( TORQUE_OS_XENON )

#include <Xtl.h>
#include <string>

#define TORQUE_USE_WINSOCK
#define EINPROGRESS WSAEINPROGRESS
#define ioctl ioctlsocket
typedef int socklen_t;

DWORD _getLastErrorAndClear()
{
   DWORD err = WSAGetLastError();
   WSASetLastError( 0 );

   return err;
}

#else

#endif

#if defined(TORQUE_USE_WINSOCK)
static const char* strerror_wsa( int code )
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

static Net::Error getLastError();
static S32 defaultPort = 28000;
static S32 netPort = 0;
static int udpSocket = InvalidSocket;

ConnectionNotifyEvent   Net::smConnectionNotify;
ConnectionAcceptedEvent Net::smConnectionAccept;
ConnectionReceiveEvent  Net::smConnectionReceive;
PacketReceiveEvent      Net::smPacketReceive;

// local enum for socket states for polled sockets
enum SocketState
{
   InvalidState,
   Connected,
   ConnectionPending,
   Listening,
   NameLookupRequired
};

// the Socket structure helps us keep track of the
// above states
struct Socket
{
   Socket()
   {
      fd = InvalidSocket;
      state = InvalidState;
      remoteAddr[0] = 0;
      remotePort = -1;
   }

   NetSocket fd;
   S32 state;
   char remoteAddr[256];
   S32 remotePort;
};

// list of polled sockets
static Vector<Socket*> gPolledSockets( __FILE__, __LINE__ );

static Socket* addPolledSocket(NetSocket& fd, S32 state,
                               char* remoteAddr = NULL, S32 port = -1)
{
   Socket* sock = new Socket();
   sock->fd = fd;
   sock->state = state;
   if (remoteAddr)
      dStrcpy(sock->remoteAddr, remoteAddr);
   if (port != -1)
      sock->remotePort = port;
   gPolledSockets.push_back(sock);
   return sock;
}

enum {
   MaxConnections = 1024,
};


bool netSocketWaitForWritable(NetSocket fd, S32 timeoutMs)
{  
   fd_set writefds;
   timeval timeout;

   FD_ZERO(&writefds);
   FD_SET( fd, &writefds );

   timeout.tv_sec = timeoutMs / 1000;
   timeout.tv_usec = ( timeoutMs % 1000 ) * 1000;

   if( select(fd + 1, NULL, &writefds, NULL, &timeout) > 0 )
      return true;

   return false;
}

static S32 initCount = 0;

bool Net::init()
{
#if defined(TORQUE_USE_WINSOCK)
   if(!initCount)
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
   initCount++;

   Process::notify(&Net::process, PROCESS_NET_ORDER);

   return(true);
}

void Net::shutdown()
{
   Process::remove(&Net::process);

   while (gPolledSockets.size() > 0)
      closeConnectTo(gPolledSockets[0]->fd);

   closePort();
   initCount--;

#if defined(TORQUE_USE_WINSOCK)
   if(!initCount)
   {
      WSACleanup();

#ifdef TORQUE_OS_XENON
      XNetCleanup();
#endif
   }
#endif
}

Net::Error getLastError()
{
#if defined(TORQUE_USE_WINSOCK)
   S32 err = WSAGetLastError();
   switch(err)
   {
   case 0:
      return Net::NoError;
   case WSAEWOULDBLOCK:
      return Net::WouldBlock;
   default:
      return Net::UnknownError;
   }
#else
   if (errno == EAGAIN)
      return Net::WouldBlock;
   if (errno == 0)
      return Net::NoError;
   return Net::UnknownError;
#endif
}

static void netToIPSocketAddress(const NetAddress *address, struct sockaddr_in *sockAddr)
{
   dMemset(sockAddr, 0, sizeof(struct sockaddr_in));
   sockAddr->sin_family = AF_INET;
   sockAddr->sin_port = htons(address->port);
   char tAddr[20];
   dSprintf(tAddr, 20, "%d.%d.%d.%d", address->netNum[0],  address->netNum[1], address->netNum[2], address->netNum[3]);
   //fprintf(stdout,"netToIPSocketAddress(): %s\n",tAddr);fflush(NULL);
   sockAddr->sin_addr.s_addr = inet_addr(tAddr);
}

static void IPSocketToNetAddress(const struct sockaddr_in *sockAddr,  NetAddress *address)
{
   address->type = NetAddress::IPAddress;
   address->port = htons(sockAddr->sin_port);
#ifndef TORQUE_OS_XENON
   char *tAddr;
   tAddr = inet_ntoa(sockAddr->sin_addr);
   //fprintf(stdout,"IPSocketToNetAddress(): %s\n",tAddr);fflush(NULL);
   U8 nets[4];
   nets[0] = atoi(strtok(tAddr, "."));
   nets[1] = atoi(strtok(NULL, "."));
   nets[2] = atoi(strtok(NULL, "."));
   nets[3] = atoi(strtok(NULL, "."));
   //fprintf(stdout,"0 = %d, 1 = %d, 2 = %d, 3 = %d\n", nets[0], nets[1],  nets[2], nets[3]);
   address->netNum[0] = nets[0];
   address->netNum[1] = nets[1];
   address->netNum[2] = nets[2];
   address->netNum[3] = nets[3];
#else
   address->netNum[0] = sockAddr->sin_addr.s_net;
   address->netNum[1] = sockAddr->sin_addr.s_host;
   address->netNum[2] = sockAddr->sin_addr.s_lh;
   address->netNum[3] = sockAddr->sin_addr.s_impno;
#endif
}

NetSocket Net::openListenPort(U16 port)
{
   if(Journal::IsPlaying())
   {
      U32 ret;
      Journal::Read(&ret);
      return NetSocket(ret);
   }

   NetSocket sock = openSocket();
   if (sock == InvalidSocket)
   {
      Con::errorf("Unable to open listen socket: %s", strerror(errno));
      return InvalidSocket;
   }

   if (bind(sock, port) != NoError)
   {
      Con::errorf("Unable to bind port %d: %s", port, strerror(errno));
      ::closesocket(sock);
      return InvalidSocket;
   }
   if (listen(sock, 4) != NoError)
   {
      Con::errorf("Unable to listen on port %d: %s", port,  strerror(errno));
      ::closesocket(sock);
      return InvalidSocket;
   }

   setBlocking(sock, false);
   addPolledSocket(sock, Listening);

   if(Journal::IsRecording())
      Journal::Write(U32(sock));

   return sock;
}

NetSocket Net::openConnectTo(const char *addressString)
{
   if(!dStrnicmp(addressString, "ipx:", 4))
      // ipx support deprecated
      return InvalidSocket;
   if(!dStrnicmp(addressString, "ip:", 3))
      addressString += 3;  // eat off the ip:
   char remoteAddr[256];
   dStrcpy(remoteAddr, addressString);

   char *portString = dStrchr(remoteAddr, ':');

   U16 port;
   if(portString)
   {
      *portString++ = 0;
      port = htons(dAtoi(portString));
   }
   else
      port = htons(defaultPort);

   if(!dStricmp(remoteAddr, "broadcast"))
      return InvalidSocket;

   if(Journal::IsPlaying())
   {
      U32 ret;
      Journal::Read(&ret);
      return NetSocket(ret);
   }
   NetSocket sock = openSocket();
   setBlocking(sock, false);

   sockaddr_in ipAddr;
   dMemset(&ipAddr, 0, sizeof(ipAddr));
   ipAddr.sin_addr.s_addr = inet_addr(remoteAddr);

   if(ipAddr.sin_addr.s_addr != INADDR_NONE)
   {
      ipAddr.sin_port = port;
      ipAddr.sin_family = AF_INET;
      if(::connect(sock, (struct sockaddr *)&ipAddr, sizeof(ipAddr)) ==  -1)
      {
         S32 err = getLastError();
         if(err != Net::WouldBlock)
         {
            Con::errorf("Error connecting %s: %s",
               addressString, strerror(err));
            ::closesocket(sock);
            sock = InvalidSocket;
         }
      }
      if(sock != InvalidSocket) 
      {
         // add this socket to our list of polled sockets
         addPolledSocket(sock, ConnectionPending);
      }
   }
   else
   {
      // need to do an asynchronous name lookup.  first, add the socket
      // to the polled list
      addPolledSocket(sock, NameLookupRequired, remoteAddr, port);
      // queue the lookup
      gNetAsync.queueLookup(remoteAddr, sock);
   }
   if(Journal::IsRecording())
      Journal::Write(U32(sock));
   return sock;
}

void Net::closeConnectTo(NetSocket sock)
{
   if(Journal::IsPlaying())
      return;

   // if this socket is in the list of polled sockets, remove it
   for (int i = 0; i < gPolledSockets.size(); ++i)
   {
      if (gPolledSockets[i]->fd == sock)
      {
         delete gPolledSockets[i];
         gPolledSockets.erase(i);
         break;
      }
   }

   closeSocket(sock);
}

Net::Error Net::sendtoSocket(NetSocket socket, const U8 *buffer, int  bufferSize)
{
   if(Journal::IsPlaying())
   {
      U32 e;
      Journal::Read(&e);

      return (Net::Error) e;
   }

   Net::Error e = send(socket, buffer, bufferSize);

   if(Journal::IsRecording())
      Journal::Write(U32(e));

   return e;
}

bool Net::openPort(S32 port, bool doBind)
{
   if(udpSocket != InvalidSocket)
      ::closesocket(udpSocket);

   // we turn off VDP in non-release builds because VDP does not support broadcast packets
   // which are required for LAN queries (PC->Xbox connectivity).  The wire protocol still
   // uses the VDP packet structure, though.
   int protocol = 0;
   bool useVDP = false;
#ifdef TORQUE_DISABLE_PC_CONNECTIVITY
   // Xbox uses a VDP (voice/data protocol) socket for networking
   protocol = IPPROTO_VDP;
   useVDP = true;
#endif

   udpSocket = socket(AF_INET, SOCK_DGRAM, protocol);

   if(udpSocket != InvalidSocket)
   {
      Net::Error error = NoError;
	  if (doBind)
	  {
        error = bind(udpSocket, port);
	  }

      if(error == NoError)
         error = setBufferSize(udpSocket, 32768);

      if(error == NoError && !useVDP)
         error = setBroadcast(udpSocket, true);

      if(error == NoError)
         error = setBlocking(udpSocket, false);

      if(error == NoError)
         Con::printf("UDP initialized on port %d", port);
      else
      {
         ::closesocket(udpSocket);
         udpSocket = InvalidSocket;
         Con::printf("Unable to initialize UDP - error %d", error);
      }
   }
   netPort = port;
   return udpSocket != InvalidSocket;
}

NetSocket Net::getPort()

{

   return udpSocket;

}


void Net::closePort()
{
   if(udpSocket != InvalidSocket)
      ::closesocket(udpSocket);
}

Net::Error Net::sendto(const NetAddress *address, const U8 *buffer, S32  bufferSize)
{
   if(Journal::IsPlaying())
      return NoError;

   if(address->type == NetAddress::IPAddress)
   {
      sockaddr_in ipAddr;
      netToIPSocketAddress(address, &ipAddr);
      if(::sendto(udpSocket, (const char*)buffer, bufferSize, 0,
         (sockaddr *) &ipAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
         return getLastError();
      else
         return NoError;
   }
   else
   {
      SOCKADDR_IN ipAddr;
      netToIPSocketAddress(address, &ipAddr);
      if(::sendto(udpSocket, (const char*)buffer, bufferSize, 0,
         (PSOCKADDR) &ipAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
         return getLastError();
      else
         return NoError;
   }
}

void Net::process()
{
   sockaddr sa;
   sa.sa_family = AF_UNSPEC;
   NetAddress srcAddress;
   RawData tmpBuffer;
   tmpBuffer.alloc(MaxPacketDataSize);

   for(;;)
   {
      socklen_t addrLen = sizeof(sa);
      S32 bytesRead = -1;

      if(udpSocket != InvalidSocket)
         bytesRead = recvfrom(udpSocket, (char *) tmpBuffer.data, MaxPacketDataSize, 0, &sa, &addrLen);

      if(bytesRead == -1)
         break;

      if(sa.sa_family == AF_INET)
         IPSocketToNetAddress((sockaddr_in *) &sa,  &srcAddress);
      else
         continue;

      if(bytesRead <= 0)
         continue;

      if(srcAddress.type == NetAddress::IPAddress &&
         srcAddress.netNum[0] == 127 &&
         srcAddress.netNum[1] == 0 &&
         srcAddress.netNum[2] == 0 &&
         srcAddress.netNum[3] == 1 &&
         srcAddress.port == netPort)
         continue;

      tmpBuffer.size = bytesRead;

      Net::smPacketReceive.trigger(srcAddress, tmpBuffer);
   }

   // process the polled sockets.  This blob of code performs functions
   // similar to WinsockProc in winNet.cc

   if (gPolledSockets.size() == 0)
      return;

   S32 optval;
   socklen_t optlen = sizeof(S32);
   S32 bytesRead;
   Net::Error err;
   bool removeSock = false;
   Socket *currentSock = NULL;
   sockaddr_in ipAddr;
   NetSocket incoming = InvalidSocket;
   char out_h_addr[1024];
   int out_h_length = 0;
   RawData readBuff;

   for (S32 i = 0; i < gPolledSockets.size();
      /* no increment, this is done at end of loop body */)
   {
      removeSock = false;
      currentSock = gPolledSockets[i];
      switch (currentSock->state)
      {
      case ::InvalidState:
         Con::errorf("Error, InvalidState socket in polled sockets  list");
         break;
      case ::ConnectionPending:
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

            Net::smConnectionNotify.trigger(currentSock->fd, Net::ConnectFailed);
            removeSock = true;
         }
         else
         {
            if (optval == EINPROGRESS)
               // still connecting...
               break;

            if (optval == 0)
            {
               // poll for writable status to be sure we're connected.
               bool ready = netSocketWaitForWritable(currentSock->fd,0);
               if(!ready)
                  break;

               currentSock->state = ::Connected;
               Net::smConnectionNotify.trigger(currentSock->fd, Net::Connected);
            }
            else
            {
               // some kind of error
               Con::errorf("Error connecting: %s", strerror(errno));
               Net::smConnectionNotify.trigger(currentSock->fd, Net::ConnectFailed);
               removeSock = true;
            }
         }
         break;
      case ::Connected:

         // try to get some data
         bytesRead = 0;
         readBuff.alloc(MaxPacketDataSize);
         err = Net::recv(currentSock->fd, (U8*)readBuff.data, MaxPacketDataSize, &bytesRead);
         if(err == Net::NoError)
         {
            if (bytesRead > 0)
            {
               // got some data, post it
               readBuff.size = bytesRead;
               Net::smConnectionReceive.trigger(currentSock->fd, readBuff);
            }
            else
            {
               // ack! this shouldn't happen
               if (bytesRead < 0)
                  Con::errorf("Unexpected error on socket: %s", strerror(errno));

               // zero bytes read means EOF
               Net::smConnectionNotify.trigger(currentSock->fd, Net::Disconnected);

               removeSock = true;
            }
         }
         else if (err != Net::NoError && err != Net::WouldBlock)
         {
            Con::errorf("Error reading from socket: %s",  strerror(errno));
            Net::smConnectionNotify.trigger(currentSock->fd, Net::Disconnected);
            removeSock = true;
         }
         break;
      case ::NameLookupRequired:
         // is the lookup complete?
         if (!gNetAsync.checkLookup(
            currentSock->fd, out_h_addr, &out_h_length,
            sizeof(out_h_addr)))
            break;

         U32 newState;
         if (out_h_length == -1)
         {
            Con::errorf("DNS lookup failed: %s",  currentSock->remoteAddr);
            newState = Net::DNSFailed;
            removeSock = true;
         }
         else
         {
            // try to connect
            dMemcpy(&(ipAddr.sin_addr.s_addr), out_h_addr,  out_h_length);
            ipAddr.sin_port = currentSock->remotePort;
            ipAddr.sin_family = AF_INET;
            if(::connect(currentSock->fd, (struct sockaddr *)&ipAddr,
               sizeof(ipAddr)) == -1)
            {
               int errorCode;
#if defined(TORQUE_USE_WINSOCK)
               errorCode = WSAGetLastError();
               if( errorCode == WSAEINPROGRESS || errorCode == WSAEWOULDBLOCK )
#else
               errorCode = errno;
               if (errno == EINPROGRESS)
#endif
               {
                  newState = Net::DNSResolved;
                  currentSock->state = ::ConnectionPending;
               }
               else
               {
                  const char* errorString;
#if defined(TORQUE_USE_WINSOCK)
                  errorString = strerror_wsa( errorCode );
#else
                  errorString = strerror( errorCode );
#endif
                  Con::errorf("Error connecting to %s: %s (%i)",
                     currentSock->remoteAddr,  errorString, errorCode);
                  newState = Net::ConnectFailed;
                  removeSock = true;
               }
            }
            else
            {
               newState = Net::Connected;
               currentSock->state = Net::Connected;
            }
         }

         Net::smConnectionNotify.trigger(currentSock->fd, newState);
         break;
      case ::Listening:
         NetAddress incomingAddy;

         incoming = Net::accept(currentSock->fd, &incomingAddy);
         if(incoming != InvalidSocket)
         {
            setBlocking(incoming, false);
            addPolledSocket(incoming, Connected);
            Net::smConnectionAccept.trigger(currentSock->fd, incoming, incomingAddy);
         }
         break;
      }

      // only increment index if we're not removing the connection,  since
      // the removal will shift the indices down by one
      if (removeSock)
         closeConnectTo(currentSock->fd);
      else
         i++;
   }
}

NetSocket Net::openSocket()
{
   int retSocket;
   retSocket = socket(AF_INET, SOCK_STREAM, 0);

   if(retSocket == InvalidSocket)
      return InvalidSocket;
   else
      return retSocket;
}

Net::Error Net::closeSocket(NetSocket socket)
{
   if(socket != InvalidSocket)
   {
      if(!closesocket(socket))
         return NoError;
      else
         return getLastError();
   }
   else
      return NotASocket;
}

Net::Error Net::connect(NetSocket socket, const NetAddress *address)
{
   if(address->type != NetAddress::IPAddress)
      return WrongProtocolType;
   sockaddr_in socketAddress;
   netToIPSocketAddress(address, &socketAddress);
   if(!::connect(socket, (sockaddr *) &socketAddress,  sizeof(socketAddress)))
      return NoError;
   return getLastError();
}

Net::Error Net::listen(NetSocket socket, S32 backlog)
{
   if(!::listen(socket, backlog))
      return NoError;
   return getLastError();
}

NetSocket Net::accept(NetSocket acceptSocket, NetAddress *remoteAddress)
{
   sockaddr_in socketAddress;
   socklen_t addrLen = sizeof(socketAddress);

   int retVal = ::accept(acceptSocket, (sockaddr *) &socketAddress,  &addrLen);
   if(retVal != InvalidSocket)
   {
      IPSocketToNetAddress(&socketAddress, remoteAddress);
      return retVal;
   }
   return InvalidSocket;
}

Net::Error Net::bind(NetSocket socket, U16 port)
{
   S32 error;

   sockaddr_in socketAddress;
   dMemset((char *)&socketAddress, 0, sizeof(socketAddress));
   socketAddress.sin_family = AF_INET;
   // It's entirely possible that there are two NIC cards.
   // We let the user specify which one the server runs on.

   // thanks to [TPG]P1aGu3 for the name
   const char* serverIP = Con::getVariable( "pref::Net::BindAddress" );
   // serverIP is guaranteed to be non-0.
   AssertFatal( serverIP, "serverIP is NULL!" );

   if( serverIP[0] != '\0' ) {
      // we're not empty
      socketAddress.sin_addr.s_addr = inet_addr( serverIP );

      if( socketAddress.sin_addr.s_addr != INADDR_NONE ) {
         Con::printf( "Binding server port to %s", serverIP );
      } else {
         Con::warnf( ConsoleLogEntry::General,
            "inet_addr() failed for %s while binding!",
            serverIP );
         socketAddress.sin_addr.s_addr = INADDR_ANY;
      }

   } else {
      Con::printf( "Binding server port to default IP" );
      socketAddress.sin_addr.s_addr = INADDR_ANY;
   }

   socketAddress.sin_port = htons(port);
   error = ::bind(socket, (sockaddr *) &socketAddress,  sizeof(socketAddress));

   if(!error)
      return NoError;
   return getLastError();
}

Net::Error Net::setBufferSize(NetSocket socket, S32 bufferSize)
{
   S32 error;
   error = setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char *)  &bufferSize, sizeof(bufferSize));
   if(!error)
      error = setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *)  &bufferSize, sizeof(bufferSize));
   if(!error)
      return NoError;
   return getLastError();
}

Net::Error Net::setBroadcast(NetSocket socket, bool broadcast)
{
   S32 bc = broadcast;
   S32 error = setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char*)&bc,  sizeof(bc));
   if(!error)
      return NoError;
   return getLastError();
}

Net::Error Net::setBlocking(NetSocket socket, bool blockingIO)
{
   unsigned long notblock = !blockingIO;
   S32 error = ioctl(socket, FIONBIO, &notblock);
   if(!error)
      return NoError;
   return getLastError();
}

Net::Error Net::send(NetSocket socket, const U8 *buffer, S32 bufferSize)
{
   errno = 0;
   S32 bytesWritten = ::send(socket, (const char*)buffer, bufferSize, 0);
   if(bytesWritten == -1)
#if defined(TORQUE_USE_WINSOCK)
      Con::errorf("Could not write to socket. Error: %s",strerror_wsa( WSAGetLastError() ));
#else
      Con::errorf("Could not write to socket. Error: %s",strerror(errno));
#endif

   return getLastError();
}

Net::Error Net::recv(NetSocket socket, U8 *buffer, S32 bufferSize, S32  *bytesRead)
{
   *bytesRead = ::recv(socket, (char*)buffer, bufferSize, 0);
   if(*bytesRead == -1)
      return getLastError();
   return NoError;
}

bool Net::compareAddresses(const NetAddress *a1, const NetAddress *a2)
{
   if((a1->type != a2->type)  ||
      (*((U32 *)a1->netNum) != *((U32 *)a2->netNum)) ||
      (a1->port != a2->port))
      return false;

   if(a1->type == NetAddress::IPAddress)
      return true;
   for(S32 i = 0; i < 6; i++)
      if(a1->nodeNum[i] != a2->nodeNum[i])
         return false;
   return true;
}

bool Net::stringToAddress(const char *addressString, NetAddress  *address)
{
   if(!dStrnicmp(addressString, "ipx:", 4))
      // ipx support deprecated
      return false;

   if(!dStrnicmp(addressString, "ip:", 3))
      addressString += 3;  // eat off the ip:

   sockaddr_in ipAddr;
   char remoteAddr[256];
   if(strlen(addressString) > 255)
      return false;

   dStrcpy(remoteAddr, addressString);

   char *portString = dStrchr(remoteAddr, ':');
   if(portString)
      *portString++ = '\0';

   if(!dStricmp(remoteAddr, "broadcast"))
      ipAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
   else
   {
      ipAddr.sin_addr.s_addr = inet_addr(remoteAddr);

      if (ipAddr.sin_addr.s_addr == INADDR_NONE) // error
      {
         // On the Xbox, 'gethostbyname' does not exist so...
#ifndef TORQUE_OS_XENON
         struct hostent *hp;
         if((hp = gethostbyname(remoteAddr)) == 0)
            return false;
         else
            memcpy(&ipAddr.sin_addr.s_addr, hp->h_addr,  sizeof(in_addr));
#else
         // On the Xbox do XNetDnsLookup
         XNDNS *pxndns = NULL;
         HANDLE hEvent = CreateEvent(NULL, false, false, NULL);
         XNetDnsLookup(remoteAddr, hEvent, &pxndns);

         // Wait for event (passing NULL as a handle to XNetDnsLookup will NOT
         // cause it to behave synchronously, so do not remove the handle/wait
         while(pxndns->iStatus == WSAEINPROGRESS) 
            WaitForSingleObject(hEvent, INFINITE);

         bool foundAddr = pxndns->iStatus == 0 && pxndns->cina > 0;
         if(foundAddr)
         {
            // Lets just grab the first address returned, for now
            memcpy(&ipAddr.sin_addr, pxndns->aina,  sizeof(IN_ADDR));
         }

         XNetDnsRelease(pxndns);
         CloseHandle(hEvent);

         // If we didn't successfully resolve the DNS lookup, bail after the
         // handles are released
         if(!foundAddr)
            return false;
#endif
      }
   }
   if(portString)
      ipAddr.sin_port = htons(dAtoi(portString));
   else
      ipAddr.sin_port = htons(defaultPort);
   ipAddr.sin_family = AF_INET;
   IPSocketToNetAddress(&ipAddr, address);
   return true;
}

void Net::addressToString(const NetAddress *address, char  addressString[256])
{
   if(address->type == NetAddress::IPAddress)
   {
      sockaddr_in ipAddr;
      netToIPSocketAddress(address, &ipAddr);

      if(ipAddr.sin_addr.s_addr == htonl(INADDR_BROADCAST))
         dSprintf(addressString, 256, "IP:Broadcast:%d",  ntohs(ipAddr.sin_port));
      else
      {
#ifndef TORQUE_OS_XENON
         dSprintf(addressString, 256, "IP:%s:%d",  inet_ntoa(ipAddr.sin_addr),
         ntohs(ipAddr.sin_port));
#else
         dSprintf(addressString, 256, "IP:%d.%d.%d.%d:%d", ipAddr.sin_addr.s_net,
            ipAddr.sin_addr.s_host, ipAddr.sin_addr.s_lh,
            ipAddr.sin_addr.s_impno, ntohs( ipAddr.sin_port ) );

#endif
      }
   }
   else
   {
      *addressString = 0;
      return;
   }
}

