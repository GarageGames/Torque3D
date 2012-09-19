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
#if 0

#include "platformX86UNIX/platformX86UNIX.h"
#include "platform/platform.h"
#include "platform/event.h"
#include "platform/platformNetAsync.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>

/* for PROTO_IPX */
#if defined(__linux__)
#include <net/if_ppp.h>
#include <sys/ioctl.h>   /* ioctl() */
#include <net/ppp_defs.h>
#elif defined(__OpenBSD__) || defined(__FreeBSD__)
#include <sys/ioctl.h>   /* ioctl() */
#include <net/ppp_defs.h>
#endif

#include <netipx/ipx.h>
#include <stdlib.h>

#include "console/console.h"
#include "platform/gameInterface.h"
#include "core/fileStream.h"
#include "core/tVector.h"

static Net::Error getLastError();
static S32 defaultPort = 28000;
static S32 netPort = 0;
static int ipxSocket = InvalidSocket;
static int udpSocket = InvalidSocket;

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
static Vector<Socket*> gPolledSockets;

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

S32 Poll(NetSocket fd, S32 eventMask, S32 timeoutMs)
{
   pollfd pfd;
   S32 retVal;
   pfd.fd = fd;
   pfd.events = eventMask;

   retVal = poll(&pfd, 1, timeoutMs);
   return retVal;
   if (retVal <= 0)
      return retVal;
   else
      return pfd.revents;
}

bool Net::init()
{
   NetAsync::startAsync();
   return(true);
}

void Net::shutdown()
{
   while (gPolledSockets.size() > 0)
      closeConnectTo(gPolledSockets[0]->fd);
   
   closePort();
   NetAsync::stopAsync();
}

static void netToIPSocketAddress(const NetAddress *address, struct sockaddr_in *sockAddr)
{
   dMemset(sockAddr, 0, sizeof(struct sockaddr_in));
   sockAddr->sin_family = AF_INET;
   sockAddr->sin_port = htons(address->port);
   char tAddr[20];
   dSprintf(tAddr, 20, "%d.%d.%d.%d\n", address->netNum[0], address->netNum[1], address->netNum[2], address->netNum[3]);
//fprintf(stdout,"netToIPSocketAddress(): %s\n",tAddr);fflush(NULL);
   sockAddr->sin_addr.s_addr = inet_addr(tAddr);
//   sockAddr->sin_addr.s_addr = address->netNum[0];  // hopefully this will work.
}

static void IPSocketToNetAddress(const struct sockaddr_in *sockAddr, NetAddress *address)
{
   address->type = NetAddress::IPAddress;
   address->port = htons(sockAddr->sin_port);
   char *tAddr;
   tAddr = inet_ntoa(sockAddr->sin_addr);
//fprintf(stdout,"IPSocketToNetAddress(): %s\n",tAddr);fflush(NULL);
   U8 nets[4];
   nets[0] = atoi(strtok(tAddr, "."));
   nets[1] = atoi(strtok(NULL, "."));
   nets[2] = atoi(strtok(NULL, "."));
   nets[3] = atoi(strtok(NULL, "."));
//fprintf(stdout,"0 = %d, 1 = %d, 2 = %d, 3 = %d\n", nets[0], nets[1], nets[2], nets[3]);
   address->netNum[0] = nets[0];
   address->netNum[1] = nets[1];
   address->netNum[2] = nets[2];
   address->netNum[3] = nets[3];
}

static void netToIPXSocketAddress(const NetAddress *address, sockaddr_ipx *sockAddr)
{
#if !defined(__FreeBSD__)
   dMemset(sockAddr, 0, sizeof(sockaddr_ipx));
   sockAddr->sipx_family = AF_INET;
   sockAddr->sipx_port = htons(address->port);
   sockAddr->sipx_network = address->netNum[0];
   sockAddr->sipx_node[0] = address->nodeNum[0];
   sockAddr->sipx_node[1] = address->nodeNum[1];
   sockAddr->sipx_node[2] = address->nodeNum[2];
   sockAddr->sipx_node[3] = address->nodeNum[3];
   sockAddr->sipx_node[4] = address->nodeNum[4];
   sockAddr->sipx_node[5] = address->nodeNum[5];
#endif
}

static void IPXSocketToNetAddress(const sockaddr_ipx *sockAddr, NetAddress *address)
{
#if !defined(__FreeBSD__)
   address->type = NetAddress::IPXAddress;
   address->port = htons(sockAddr->sipx_port);
   address->netNum[0]  = sockAddr->sipx_network;
   address->nodeNum[0] = sockAddr->sipx_node[0];
   address->nodeNum[1] = sockAddr->sipx_node[1];
   address->nodeNum[2] = sockAddr->sipx_node[2];
   address->nodeNum[3] = sockAddr->sipx_node[3];
   address->nodeNum[4] = sockAddr->sipx_node[4];
   address->nodeNum[5] = sockAddr->sipx_node[5];
#endif
}

NetSocket Net::openListenPort(U16 port)
{
   if(Game->isJournalReading())
   {
      U32 ret;
      Game->journalRead(&ret);
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
      ::close(sock);
      return InvalidSocket;
   }
   if (listen(sock, 4) != NoError)
   {
      Con::errorf("Unable to listen on port %d: %s", port, strerror(errno));
      ::close(sock);
      return InvalidSocket;
   }

   setBlocking(sock, false);
   addPolledSocket(sock, Listening);
   if (Game->isJournalWriting())
      Game->journalWrite(U32(sock));
   return sock;
}

NetSocket Net::openConnectTo(const char *addressString)
{
   if(!dStrnicmp(addressString, "ipx:", 4))
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

   if(Game->isJournalReading())
   {
      U32 ret;
      Game->journalRead(&ret);
      return NetSocket(ret);
   }
   NetSocket sock = openSocket();
   setBlocking(sock, false);

   sockaddr_in ipAddr;
   dMemset(&ipAddr, 0, sizeof(ipAddr));
   
   if (inet_aton(remoteAddr, &ipAddr.sin_addr) != 0)
   {
      ipAddr.sin_port = port;
      ipAddr.sin_family = AF_INET;
      if(::connect(sock, (struct sockaddr *)&ipAddr, sizeof(ipAddr)) == -1 &&
         errno != EINPROGRESS)
      {
         Con::errorf("Error connecting %s: %s", 
		     addressString, strerror(errno));
         ::close(sock);
         sock = InvalidSocket;
      }
      if(sock != InvalidSocket) {
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
   if(Game->isJournalWriting())
      Game->journalWrite(U32(sock));
   return sock;
}

void Net::closeConnectTo(NetSocket sock)
{
   if(Game->isJournalReading())
      return;

   // if this socket is in the list of polled sockets, remove it
   for (int i = 0; i < gPolledSockets.size(); ++i)
      if (gPolledSockets[i]->fd == sock)
      {
         delete gPolledSockets[i];
         gPolledSockets.erase(i);
         break;
      }
   
   closeSocket(sock);
}

Net::Error Net::sendtoSocket(NetSocket socket, const U8 *buffer, int bufferSize)
{
   if(Game->isJournalReading())
   {
      U32 e;
      Game->journalRead(&e);
      
      return (Net::Error) e;
   }
   Net::Error e = send(socket, buffer, bufferSize);
   if(Game->isJournalWriting())
      Game->journalWrite(U32(e));
   return e;
}

bool Net::openPort(S32 port)
{
   if(udpSocket != InvalidSocket)
      close(udpSocket);
   if(ipxSocket != InvalidSocket)
      close(ipxSocket);
      
   udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
   ipxSocket = socket(AF_IPX, SOCK_DGRAM, 0);

   if(udpSocket != InvalidSocket)
   {
      Net::Error error;
      error = bind(udpSocket, port);
      if(error == NoError)
         error = setBufferSize(udpSocket, 32768);
      if(error == NoError)
         error = setBroadcast(udpSocket, true);
      if(error == NoError)
         error = setBlocking(udpSocket, false);
      if(error == NoError)
         Con::printf("UDP initialized on port %d", port);
      else
      {
         close(udpSocket);
         udpSocket = InvalidSocket;
         Con::printf("Unable to initialize UDP - error %d", error);
      }
   }
   if(ipxSocket != InvalidSocket)
   {
      Net::Error error = NoError;
      sockaddr_ipx ipxAddress;   
      memset((char *)&ipxAddress, 0, sizeof(ipxAddress));
      ipxAddress.sipx_family = AF_IPX;
      ipxAddress.sipx_port = htons(port);
      S32 err = ::bind(ipxSocket, (struct sockaddr *)&ipxAddress, sizeof(ipxAddress));
      if(err)
         error = getLastError();
      if(error == NoError)
         error = setBufferSize(ipxSocket, 32768);
      if(error == NoError)
         error = setBroadcast(ipxSocket, true);
      if(error == NoError)
         error = setBlocking(ipxSocket, false);
      if(error == NoError)
         Con::printf("IPX initialized on port %d", port);
      else
      {
         close(ipxSocket);
         ipxSocket = InvalidSocket;
         Con::printf("Unable to initialize IPX - error %d", error);
      }
   }
   netPort = port;
   return ipxSocket != InvalidSocket || udpSocket != InvalidSocket;
}

void Net::closePort()
{
   if(ipxSocket != InvalidSocket)
      close(ipxSocket);
   if(udpSocket != InvalidSocket)
      close(udpSocket);
}

Net::Error Net::sendto(const NetAddress *address, const U8 *buffer, S32 bufferSize)
{
   if(Game->isJournalReading())
      return NoError;

   if(address->type == NetAddress::IPXAddress)
   {
      sockaddr_ipx ipxAddr;
      netToIPXSocketAddress(address, &ipxAddr);
      if(::sendto(ipxSocket, (const char*)buffer, bufferSize, 0,
                  (sockaddr *) &ipxAddr, sizeof(sockaddr_ipx)) == -1)
         return getLastError();
      else
         return NoError;
   }
   else
   {
      sockaddr_in ipAddr;
      netToIPSocketAddress(address, &ipAddr);
      if(::sendto(udpSocket, (const char*)buffer, bufferSize, 0,
                  (sockaddr *) &ipAddr, sizeof(sockaddr_in)) == -1)
         return getLastError();
      else
         return NoError;
   }
}

void Net::process()
{
   sockaddr sa;

   PacketReceiveEvent receiveEvent;
   for(;;)
   {
      U32 addrLen = sizeof(sa);
      S32 bytesRead = -1;
      if(udpSocket != InvalidSocket)
         bytesRead = recvfrom(udpSocket, (char *) receiveEvent.data, MaxPacketDataSize, 0, &sa, &addrLen);
      if(bytesRead == -1 && ipxSocket != InvalidSocket)
      {
         addrLen = sizeof(sa);
         bytesRead = recvfrom(ipxSocket, (char *) receiveEvent.data, MaxPacketDataSize, 0, &sa, &addrLen);
      }
      
      if(bytesRead == -1)
         break;
      
      if(sa.sa_family == AF_INET)
         IPSocketToNetAddress((sockaddr_in *) &sa, &receiveEvent.sourceAddress);
      else if(sa.sa_family == AF_IPX)
         IPXSocketToNetAddress((sockaddr_ipx *) &sa, &receiveEvent.sourceAddress);
      else
         continue;
         
      NetAddress &na = receiveEvent.sourceAddress;
      if(na.type == NetAddress::IPAddress &&
         na.netNum[0] == 127 &&
         na.netNum[1] == 0 &&
         na.netNum[2] == 0 &&
         na.netNum[3] == 1 &&
         na.port == netPort)
         continue;
      if(bytesRead <= 0)
         continue;
      receiveEvent.size = PacketReceiveEventHeaderSize + bytesRead;
      Game->postEvent(receiveEvent);
   }

   // process the polled sockets.  This blob of code performs functions
   // similar to WinsockProc in winNet.cc

   if (gPolledSockets.size() == 0)
      return;

   static ConnectedNotifyEvent notifyEvent;
   static ConnectedAcceptEvent acceptEvent;
   static ConnectedReceiveEvent cReceiveEvent;

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

   for (S32 i = 0; i < gPolledSockets.size(); 
        /* no increment, this is done at end of loop body */)
   {
      removeSock = false;
      currentSock = gPolledSockets[i];
      switch (currentSock->state)
      {
         case InvalidState:
            Con::errorf("Error, InvalidState socket in polled sockets list");
            break;
         case ConnectionPending:
            notifyEvent.tag = currentSock->fd;
            // see if it is now connected
            if (getsockopt(currentSock->fd, SOL_SOCKET, SO_ERROR, 
                           &optval, &optlen) == -1)
            {
               Con::errorf("Error getting socket options: %s", strerror(errno));
               notifyEvent.state = ConnectedNotifyEvent::ConnectFailed;
               Game->postEvent(notifyEvent);
               removeSock = true;
            }
            else
            {
               if (optval == EINPROGRESS)
                  // still connecting...
                  break;

               if (optval == 0)
               {
                  // connected
                  notifyEvent.state = ConnectedNotifyEvent::Connected;
                  Game->postEvent(notifyEvent);
                  currentSock->state = Connected;
               }
               else
               {
                  // some kind of error
                  Con::errorf("Error connecting: %s", strerror(errno));
                  notifyEvent.state = ConnectedNotifyEvent::ConnectFailed;
                  Game->postEvent(notifyEvent);
                  removeSock = true;
               }
            }
            break;
         case Connected:
            bytesRead = 0;
            // try to get some data
            err = Net::recv(currentSock->fd, cReceiveEvent.data, 
                            MaxPacketDataSize, &bytesRead);
            if(err == Net::NoError)
            {
               if (bytesRead > 0)
               {
                  // got some data, post it
                  cReceiveEvent.tag = currentSock->fd;
                  cReceiveEvent.size = ConnectedReceiveEventHeaderSize + 
                     bytesRead;
                  Game->postEvent(cReceiveEvent);
               }
               else 
               {
                  // zero bytes read means EOF
                  if (bytesRead < 0)
                     // ack! this shouldn't happen
                     Con::errorf("Unexpected error on socket: %s", 
                                 strerror(errno));

                  notifyEvent.tag = currentSock->fd;
                  notifyEvent.state = ConnectedNotifyEvent::Disconnected;
                  Game->postEvent(notifyEvent);
                  removeSock = true;
               }
            }
            else if (err != Net::NoError && err != Net::WouldBlock)
            {
               Con::errorf("Error reading from socket: %s", strerror(errno));
               notifyEvent.tag = currentSock->fd;
               notifyEvent.state = ConnectedNotifyEvent::Disconnected;
               Game->postEvent(notifyEvent);
               removeSock = true;
            }
            break;
         case NameLookupRequired:
            // is the lookup complete?
            if (!gNetAsync.checkLookup(
                   currentSock->fd, out_h_addr, &out_h_length, 
                   sizeof(out_h_addr)))
               break;
            
            notifyEvent.tag = currentSock->fd;
            if (out_h_length == -1)
            {
               Con::errorf("DNS lookup failed: %s", currentSock->remoteAddr);
               notifyEvent.state = ConnectedNotifyEvent::DNSFailed;
               removeSock = true;
            }
            else
            {
               // try to connect
               dMemcpy(&(ipAddr.sin_addr.s_addr), out_h_addr, out_h_length);
               ipAddr.sin_port = currentSock->remotePort;
               ipAddr.sin_family = AF_INET;
               if(::connect(currentSock->fd, (struct sockaddr *)&ipAddr, 
                            sizeof(ipAddr)) == -1)
               {
                  if (errno == EINPROGRESS)
                  {
                     notifyEvent.state = ConnectedNotifyEvent::DNSResolved;
                     currentSock->state = ConnectionPending;
                  }
                  else
                  {
                     Con::errorf("Error connecting to %s: %s", 
                                 currentSock->remoteAddr, strerror(errno));
                     notifyEvent.state = ConnectedNotifyEvent::ConnectFailed;
                     removeSock = true;
                  }
               }
               else
               {
                  notifyEvent.state = ConnectedNotifyEvent::Connected;
                  currentSock->state = Connected;
               }
            }
            Game->postEvent(notifyEvent);			
            break;
    	 case Listening:
            incoming = 
               Net::accept(currentSock->fd, &acceptEvent.address);
            if(incoming != InvalidSocket)
            {
               acceptEvent.portTag = currentSock->fd;
               acceptEvent.connectionTag = incoming;
               setBlocking(incoming, false);
               addPolledSocket(incoming, Connected);
               Game->postEvent(acceptEvent);
            }
            break;
      }

      // only increment index if we're not removing the connection, since 
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
      if(!close(socket))
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
   if(!::connect(socket, (sockaddr *) &socketAddress, sizeof(socketAddress)))
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
   U32 addrLen = sizeof(socketAddress);
   
   int retVal = ::accept(acceptSocket, (sockaddr *) &socketAddress, &addrLen);
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
   const char* serverIP = Con::getVariable( "Pref::Net::BindAddress" );
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
   error = ::bind(socket, (sockaddr *) &socketAddress, sizeof(socketAddress));

   if(!error)
      return NoError;
   return getLastError();
}

Net::Error Net::setBufferSize(NetSocket socket, S32 bufferSize)
{
   S32 error;
   error = setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char *) &bufferSize, sizeof(bufferSize));
   if(!error)
      error = setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *) &bufferSize, sizeof(bufferSize));
   if(!error)
      return NoError;
   return getLastError();
}

Net::Error Net::setBroadcast(NetSocket socket, bool broadcast)
{
   S32 bc = broadcast;
   S32 error = setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char*)&bc, sizeof(bc));
   if(!error)
      return NoError;
   return getLastError();   
}

Net::Error Net::setBlocking(NetSocket socket, bool blockingIO)
{
   int notblock = !blockingIO;
   S32 error = ioctl(socket, FIONBIO, &notblock);
   if(!error)
      return NoError;
   return getLastError();   
}

Net::Error Net::send(NetSocket socket, const U8 *buffer, S32 bufferSize)
{
   // Poll for write status.  this blocks.  should really
   // do this in a separate thread or set it up so that the data can
   // get queued and sent later
   // JMQTODO
   Poll(socket, POLLOUT, 10000);

   S32 error = ::send(socket, (const char*)buffer, bufferSize, 0);
   if(error != -1)
      return NoError;

   return getLastError();
}

Net::Error Net::recv(NetSocket socket, U8 *buffer, S32 bufferSize, S32 *bytesRead)
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

bool Net::stringToAddress(const char *addressString, NetAddress *address)
{
   if(dStrnicmp(addressString, "ipx:", 4))
   {
      // assume IP if it doesn't have ipx: at the front.
      
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
      
      struct hostent *hp;
      if(!dStricmp(remoteAddr, "broadcast"))
         ipAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
      else
      {
         if (inet_aton(remoteAddr,&ipAddr.sin_addr) == 0) // error
         {
            if((hp = gethostbyname(remoteAddr)) == 0)
               return false;
            else
               memcpy(&ipAddr.sin_addr.s_addr, hp->h_addr, sizeof(in_addr));
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
   else
   {
      S32 i;
      S32 port;

      address->type = NetAddress::IPXAddress;      
      for(i = 0; i < 6; i++)
         address->nodeNum[i] = 0xFF;
         
      // it's an IPX string
      addressString += 4;
      if(!dStricmp(addressString, "broadcast"))
      {
         address->port = defaultPort;
         return true;
      }
      else if(sscanf(addressString, "broadcast:%d", &port) == 1)
      {
         address->port = port;
         return true;
      }
      else
      {
         S32 nodeNum[6];
         S32 netNum[4];
         S32 count = dSscanf(addressString, "%2x%2x%2x%2x:%2x%2x%2x%2x%2x%2x:%d",
                             &netNum[0], &netNum[1], &netNum[2], &netNum[3],
                             &nodeNum[0], &nodeNum[1], &nodeNum[2], &nodeNum[3], &nodeNum[4], &nodeNum[5], 
                             &port);
      
         if(count == 10)
         {
            port = defaultPort;
            count++;
         }
         if(count != 11)
            return false;

         for(i = 0; i < 6; i++)
            address->nodeNum[i] = nodeNum[i];
         for(i = 0; i < 4; i++)
            address->netNum[i] = netNum[i];
         address->port = port;
         return true;
      }
   }
}

void Net::addressToString(const NetAddress *address, char addressString[256])
{
   if(address->type == NetAddress::IPAddress)
   {
      sockaddr_in ipAddr;
      netToIPSocketAddress(address, &ipAddr);
      
      if(ipAddr.sin_addr.s_addr == htonl(INADDR_BROADCAST))
         dSprintf(addressString, 256, "IP:Broadcast:%d", ntohs(ipAddr.sin_port));
      else
         dSprintf(addressString, 256, "IP:%s:%d", inet_ntoa(ipAddr.sin_addr),
                  ntohs(ipAddr.sin_port));
//         dSprintf(addressString, 256, "IP:%d:%d", ipAddr.sin_addr.s_addr,
//            ntohs(ipAddr.sin_port));
   }
   else
   {
      return;
      dSprintf(addressString, 256, "IPX:%.2X%.2X%.2X%.2X:%.2X%.2X%.2X%.2X%.2X%.2X:%d",
               address->netNum[0], address->netNum[1], address->netNum[2], address->netNum[3], 
               address->nodeNum[0], address->nodeNum[1], address->nodeNum[2], address->nodeNum[3], address->nodeNum[4], address->nodeNum[5], 
               address->port);
   }
}

Net::Error getLastError()
{
   if (errno == EAGAIN)
      return Net::WouldBlock;
   return Net::UnknownError;
}

#endif
