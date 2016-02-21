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

#ifndef _NETCONNECTION_H_
#define _NETCONNECTION_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif
#ifndef _NETSTRINGTABLE_H_
#include "sim/netStringTable.h"
#endif
#ifndef _DNET_H_
#include "core/dnet.h"
#endif

#ifndef _H_CONNECTIONSTRINGTABLE
#include "sim/connectionStringTable.h"
#endif

class NetConnection;
class NetObject;
class BitStream;
class ResizeBitStream;
class Stream;
class Point3F;

struct GhostInfo;
struct SubPacketRef; // defined in NetConnection subclass

//#define DEBUG_NET

#ifdef TORQUE_DEBUG_NET
#define DEBUG_LOG(x) if(mLogging){Con::printf x;}
#else
#define DEBUG_LOG(x)
#endif

DECLARE_SCOPE( NetAPI );

//----------------------------------------------------------------------------

class NetEvent;

struct NetEventNote
{
   NetEvent *mEvent;
   S32 mSeqCount;
   NetEventNote *mNextEvent;
};

/// An event to be sent over the network.
///
/// @note Torque implements two methods of network data passing; this is one of them.
/// See NetConnection for details of the other, which is referred to as ghosting.
///
/// Torque's network layer lets you pass events to/from the server. There are three
/// types of events:
///      - <b>Unguaranteed events</b> are events which are sent once. If they don't
///        make it through the link, they are not resent. This is good for quick,
///        frequent status updates which are of transient interest, like position
///        updates or voice communication.
///      - <b>Guaranteed events</b> are events which are guaranteed to be
///        delivered. If they don't make it through the link, they are sent as
///        needed. This is good for important, one-time information,
///        like which team a user wants to play on, or the current weather.
///      - <b>GuaranteedOrdered events</b> are events which are guaranteed not
///        only to be delivered, but to be delivered in order. This is good for
///        information which is not only important, but also order-critical, like
///        chat messages.
///
/// There are 6 methods that you need to implement if you want to make a
/// basic NetEvent subclass, and 2 macros you need to call.
///
/// @code
/// // A simple NetEvent to transmit a string over the network.
/// // This is based on the code in netTest.cc
/// class SimpleMessageEvent : public NetEvent
/// {
///    typedef NetEvent Parent;
///    char *msg;
/// public:
///    SimpleMessageEvent(const char *message = NULL);
///    ~SimpleMessageEvent();
///
///    virtual void pack   (NetConnection *conn, BitStream *bstream);
///    virtual void write  (NetConnection *conn, BitStream *bstream);
///    virtual void unpack (NetConnection *conn, BitStream *bstream);
///    virtual void process(NetConnection *conn);
///
///    DECLARE_CONOBJECT(SimpleMessageEvent);
/// };
///
/// IMPLEMENT_CO_NETEVENT_V1(SimpleMessageEvent);
/// @endcode
///
/// Notice the two macros which we call. The first, DECLARE_CONOBJECT() is there
/// because we're a ConsoleObject. The second, IMPLEMENT_CO_NETEVENT_V1(), is there
/// to register this event type with Torque's networking layer, so that it can be
/// properly transmitted over the wire. There are three macros which you might use:
///      - <b>IMPLEMENT_CO_NETEVENT_V1</b>, which indicates an event which may be sent
///        in either direction, from the client to the server, or from the server to the
///        client.
///      - <b>IMPLEMENT_CO_CLIENTEVENT_V1</b>, which indicates an event which may only
///        be sent to the client.
///      - <b>IMPLEMENT_CO_SERVEREVENT_V1</b>, which indicates an event which may only
///        be sent to the server.
///
/// Choosing the right macro is a good way to make your game more resistant to hacking; for instance,
/// PathManager events are marked as CLIENTEVENTs, because they would cause the server to crash if
/// a client sent them.
///
/// @note Torque allows you to call NetConnection::setLastError() on the NetConnection passed to
///       your NetEvent. You can cause the connection to abort if invalid data is received, specifying
///       a reason to the user.
///
/// Now, the 6 methods which we have above; the constructor and destructor need only do
/// whatever book-keeping is needed for your specific implementation. In our case, we
/// just need to allocate/deallocate the space for our string:
///
/// @code
///    SimpleMessageEvent::SimpleMessageEvent(const char *message = NULL)
///    {
///       // If we wanted to make this not be a GuaranteedOrdered event, we'd
///       // put a line like this in the constructor:
///       // mGuaranteeType = Guaranteed;
///       // (or whatever type you wanted.)
///       if(message)
///          msg = dStrdup(message);
///       else
///          msg = NULL;
///    }
///
///    SimpleMessageEvent::~SimpleMessageEvent()
///    {
///      dFree(msg);
///    }
/// @endcode
///
/// Simple as that! Now, onto pack(), write(), unpack(), process().
///
/// <b>pack()</b> is responsible for packing the event over the wire:
///
/// @code
/// void SimpleMessageEvent::pack(NetConnection* conn, BitStream *bstream)
/// {
///   bstream->writeString(msg);
/// }
/// @endcode
///
/// <b>unpack()</b> is responsible for unpacking the event on the other end:
///
/// @code
/// // The networking layer takes care of instantiating a new
/// // SimpleMessageEvent, which saves us a bit of effort.
/// void SimpleMessageEvent::unpack(NetConnection *conn, BitStream *bstream)
/// {
///   char buf[256];
///   bstream->readString(buf);
///   msg = dStrdup(buf);
/// }
/// @endcode
///
/// <b>process()</b> is called when the network layer is finished with things.
/// A typical case is that a GuaranteedOrdered event is unpacked and stored, but
/// not processed until the events preceding it in the sequence have also been
/// dealt with.
///
/// @code
/// // This just prints the event in the console. You might
/// // want to do something more clever here -- BJG
/// void SimpleMessageEvent::process(NetConnection *conn)
/// {
///   Con::printf("RMSG %d  %s", mSourceId, msg);
/// }
/// @endcode
///
/// <b>write()</b> is called if a demo recording is started, and the event has not yet been
/// processed, but it has been unpacked. It should be identical in its output to the bitstream
/// compared to pack(), but since it is called after unpack() some lookups may not need to be
/// performed. In normal demo recording, whole network packets are recorded, meaning that most
/// of the time write() will not be called.
///
/// In our case, it's entirely identical to pack():
///
/// @code
/// virtual void write(NetConnection*, BitStream *bstream)
/// {
///   bstream->writeString(msg);
/// }
/// @endcode
///
/// The NetEvent is sent over the wire in a straightforward way (assuming you have a
/// handle to a NetConnection):
///
/// @code
/// NetConnection *conn; // We assume you have filled this in.
///
/// con->postNetEvent(new SimpleMessageEvent("This is a test!"));
/// @endcode
///
/// @see GhostAlwaysObjectEvent for an example of dissimilar write()/pack() methods.
///
/// Finally, for more advanced applications, notifySent() is called whenever the event is
/// sent over the wire, in NetConnection::eventWritePacket(). notifyDelivered() is called
/// when the packet is finally received or (in the case of Unguaranteed packets) dropped.
///
/// @note IMPLEMENT_CO_NETEVENT_V1 and co. have sibling macros which allow you to specify a
///       groupMask; see ConsoleObject for a further discussion of this.
class NetEvent : public ConsoleObject
{
public:

   DECLARE_ABSTRACT_CLASS( NetEvent, ConsoleObject );
   DECLARE_INSCOPE( NetAPI );
   
   /// @name Implementation Details
   ///
   /// These are internal fields which you won't need to manipulate, except for mGuaranteeType.
   /// @{

   ///
   typedef ConsoleObject Parent;
   enum {
      GuaranteedOrdered = 0,
      Guaranteed = 1,
      Unguaranteed = 2
   } mGuaranteeType;
   NetConnectionId mSourceId;

   void incRef()
   {
      incRefCount();
   }
   void decRef()
   {
      decRefCount();
   }

#ifdef TORQUE_DEBUG_NET
   virtual const char *getDebugName();
#endif
   /// @}

   /// @name Things To Subclass
   /// @{

   ///
   NetEvent() { mGuaranteeType = GuaranteedOrdered; }
   virtual ~NetEvent();

   virtual void write(NetConnection *ps, BitStream *bstream) = 0;
   virtual void pack(NetConnection *ps, BitStream *bstream) = 0;
   virtual void unpack(NetConnection *ps, BitStream *bstream) = 0;
   virtual void process(NetConnection *ps) = 0;
   virtual void notifySent(NetConnection *ps);
   virtual void notifyDelivered(NetConnection *ps, bool madeit);
   /// @}
};

#define IMPLEMENT_CO_NETEVENT_V1(className)                    \
   IMPLEMENT_CLASS( className, NULL )                                                              \
   END_IMPLEMENT_CLASS;                                                                            \
   S32 className::_smTypeId; \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; } \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; } \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   AbstractClassRep* className::getContainerChildStaticClassRep() { return NULL; }                 \
   AbstractClassRep::WriteCustomTamlSchema className::getStaticWriteCustomTamlSchema() { return NULL; }  \
   ConcreteClassRep<className> className::dynClassRep( #className, "Type" #className, &_smTypeId, NetClassGroupGameMask, NetClassTypeEvent, NetEventDirAny, className::getParentStaticClassRep(), &Parent::__description)

#define IMPLEMENT_CO_CLIENTEVENT_V1(className)                    \
   IMPLEMENT_CLASS( className, NULL )                                                              \
   END_IMPLEMENT_CLASS;                                                                            \
   S32 className::_smTypeId; \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; } \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; } \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   AbstractClassRep* className::getContainerChildStaticClassRep() { return NULL; }                 \
   AbstractClassRep::WriteCustomTamlSchema className::getStaticWriteCustomTamlSchema() { return NULL; }            \
   ConcreteClassRep<className> className::dynClassRep(#className, "Type" #className, &_smTypeId,NetClassGroupGameMask, NetClassTypeEvent, NetEventDirServerToClient, className::getParentStaticClassRep(), &Parent::__description)

#define IMPLEMENT_CO_SERVEREVENT_V1(className)                    \
   IMPLEMENT_CLASS( className, NULL )                                                              \
   END_IMPLEMENT_CLASS;                                                                            \
   S32 className::_smTypeId; \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; } \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; } \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   AbstractClassRep* className::getContainerChildStaticClassRep() { return NULL; }                 \
   AbstractClassRep::WriteCustomTamlSchema className::getStaticWriteCustomTamlSchema() { return NULL; }            \
   ConcreteClassRep<className> className::dynClassRep(#className, "Type" #className, &_smTypeId, NetClassGroupGameMask, NetClassTypeEvent, NetEventDirClientToServer, className::getParentStaticClassRep(), &Parent::__description)

#define IMPLEMENT_CO_NETEVENT(className,groupMask)                    \
   IMPLEMENT_CLASS( className, NULL )                                                              \
   END_IMPLEMENT_CLASS;                                                                            \
   S32 className::_smTypeId; \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; } \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; } \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   AbstractClassRep* className::getContainerChildStaticClassRep() { return NULL; }                 \
   AbstractClassRep::WriteCustomTamlSchema className::getStaticWriteCustomTamlSchema() { return NULL; }            \
   ConcreteClassRep<className> className::dynClassRep(#className, "Type" #className, &_smTypeId, groupMask, NetClassTypeEvent, NetEventDirAny, className::getParentStaticClassRep(), &Parent::__description)

#define IMPLEMENT_CO_CLIENTEVENT(className,groupMask)                    \
   IMPLEMENT_CLASS( className, NULL )                                                              \
   END_IMPLEMENT_CLASS;                                                                            \
   S32 className::_smTypeId; \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; } \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; } \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   AbstractClassRep* className::getContainerChildStaticClassRep() { return NULL; }                 \
   AbstractClassRep::WriteCustomTamlSchema className::getStaticWriteCustomTamlSchema() { return NULL; }            \
   ConcreteClassRep<className> className::dynClassRep(#className, "Type" #className, &_smTypeId, groupMask, NetClassTypeEvent, NetEventDirServerToClient, className::getParentStaticClassRep(), &Parent::__description)

#define IMPLEMENT_CO_SERVEREVENT(className,groupMask)                    \
   IMPLEMENT_CLASS( className, className, __scope, NULL )                                          \
   END_IMPLEMENT_CLASS;                                                                            \
   S32 className::_smTypeId; \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; } \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; } \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   AbstractClassRep* className::getContainerChildStaticClassRep() { return NULL; }                 \
   AbstractClassRep::WriteCustomTamlSchema className::getStaticWriteCustomTamlSchema() { return NULL; }            \
   ConcreteClassRep<className> className::dynClassRep(#className, "Type" #className, &_smTypeId, groupMask, NetClassTypeEvent, NetEventDirClientToServer, className::getParentStaticClassRep(), &Parent::__description)


//----------------------------------------------------------------------------

/// Torque network connection.
///
/// @section NetConnection_intro Introduction
///
/// NetConnection is the glue that binds a networked Torque game together. It combines
/// the low-level notify protocol implemented in ConnectionProtocol with a SimGroup to
/// provide a powerful basis for implementing a multiplayer game protocol.
///
/// On top of this basis it implements several distinct subsystems:
///   - <b>Event manager</b>, which is responsible for transmitting NetEvents over the wire.
///     It deals with ensuring that the various types of NetEvents are delivered appropriately,
///     and with notifying the event of its delivery status.
///   - <b>Move manager</b>, which is responsible for transferring a Move to the server 32
///     times a second (on the client) and applying it to the control object (on the server).
///   - <b>Ghost manager</b>, which is responsible for doing scoping calculations (on the server
///     side) and transmitting most-recent ghost information to the client.
///   - <b>File transfer</b>; it is often the case that clients will lack important files when
///     connecting to a server which is running a mod or new map. This subsystem allows the
///     server to transfer such files to the client.
///   - <b>Networked String Table</b>; string data can easily soak up network bandwidth, so for
///     efficiency, we implement a networked string table. We can then notify the connection
///     of strings we will reference often, such as player names, and transmit only a tag,
///     instead of the whole string.
///   - <b>Demo Recording</b> is also implemented in NetConnection. A demo in Torque is a log
///     of the network traffic between client and server; when a NetConnection records a demo,
///     it simply logs this data to a file. When it plays a demo back, it replays the logged
///     data.
///   - The <b>Connection Database</b> is used to keep track of all the NetConnections; it can
///     be iterated over (for instance, to send an event to all active connections), or queried
///     by address.
///
/// @section NetConnection_events   On Events
///
/// The Event Manager is exposed to the outside world via postNetEvent(), which accepts NetEvents.
///
/// @see NetEvent for a more thorough explanation of how to use events.
///
/// @section NetConnection_ghosting On Ghosting and Scoping
///
/// Ghosting is the most complex, and most powerful, part of Torque's networking capabilities. It
/// allows the information sent to clients to be very precisely matched to what they need, so that
/// no excess bandwidth is wasted. The control object's onCameraScopeQuery() is called, to determine
/// scoping information for the client; then objects which are in scope are then transmitted to the
/// client, prioritized by the results of their getPriority() method.
///
/// There is a cap on the maximum number of ghosts; ghost IDs are currently sent via a 12-bit field,
/// ergo, there is a cap of 4096 objects ghosted per client. This can be easily raised; see the
/// GhostConstants enum.
///
/// Each object ghosted is assigned a ghost ID; the client is _only_ aware of the ghost ID. This acts
/// to enhance game security, as it becomes difficult to map objects from one connection to another, or
/// to reliably identify objects from ID alone. IDs are also reassigned based on need, making it hard
/// to track objects that have fallen out of scope (as any object which the player shouldn't see would).
///
/// resolveGhost() is used on the client side, and resolveObjectFromGhostIndex() on the server side, to
/// turn ghost IDs into object references.
///
/// The NetConnection is a SimGroup. On the client side, it contains all the objects which have been
/// ghosted to that client. On the server side, it is empty; it can be used (typically in script) to
/// hold objects related to the connection. For instance, you might place an observation camera in the
/// NetConnnection. In both cases, when the connection is destroyed, so are the contained objects.
///
/// @see NetObject, which is the superclass for ghostable objects, and ShapeBase, which is the base
///      for player and vehicle classes.
///
/// @nosubgrouping
class NetConnection : public SimGroup, public ConnectionProtocol
{
   friend class NetInterface;

   typedef SimGroup Parent;

public:
   /// Structure to track ghost references in packets.
   ///
   /// Every packet we send out with an update from a ghost causes one of these to be
   /// allocated. mask is used to track what states were sent; that way if a packet is
   /// dropped, we can easily manipulate the stored states and figure out what if any data
   /// we need to resend.
   ///
   struct GhostRef
   {
      U32 mask;                  ///< States we transmitted.
      U32 ghostInfoFlags;        ///< Flags from GhostInfo::Flags
      GhostInfo *ghost;          ///< Reference to the GhostInfo we're from.
      GhostRef *nextRef;         ///< Next GhostRef in this packet.
      GhostRef *nextUpdateChain; ///< Next update we sent for this ghost.
   };

   enum Constants
   {
      HashTableSize = 127,
   };

   void sendDisconnectPacket(const char *reason);

   virtual bool canRemoteCreate();

   virtual void onTimedOut();
   virtual void onConnectTimedOut();
   virtual void onDisconnect(const char *reason);
   virtual void onConnectionRejected(const char *reason);
   virtual void onConnectionEstablished(bool isInitiator);
   virtual void handleStartupError(const char *errorString);

   virtual void writeConnectRequest(BitStream *stream);
   virtual bool  readConnectRequest(BitStream *stream, const char **errorString);

   virtual void writeConnectAccept(BitStream *stream);
   virtual bool  readConnectAccept(BitStream *stream, const char **errorString);

   void connect(const NetAddress *address);

   //----------------------------------------------------------------
   /// @name Global Connection List
   /// @{

private:
   ///
   NetConnection *mNextConnection;        ///< Next item in list.
   NetConnection *mPrevConnection;        ///< Previous item in list.
   static NetConnection *mConnectionList; ///< Head of list.
public:
   static NetConnection *getConnectionList() { return mConnectionList; }
   NetConnection *getNext() { return mNextConnection; }
   /// @}
   //----------------------------------------------------------------

   enum NetConnectionFlags
   {
      ConnectionToServer      = BIT(0),
      ConnectionToClient      = BIT(1),
      LocalClientConnection   = BIT(2),
      NetworkConnection       = BIT(3),
   };

private:
   BitSet32 mTypeFlags;

   U32 mNetClassGroup;  ///< The NetClassGroup of this connection.

   /// @name Statistics
   /// @{

   /// Last time a packet was sent in milliseconds. 
   /// @see Platform::getVirtualMilliseconds()
   U32 mLastUpdateTime; 

   F32 mRoundTripTime;
   F32 mPacketLoss;
   U32 mSimulatedPing;
   F32 mSimulatedPacketLoss;

   /// @}

   /// @name State
   /// @{

   U32 mProtocolVersion;
   U32 mSendDelayCredit;
   U32 mConnectSequence;
   U32 mAddressDigest[4];

   bool mEstablished;
   bool mMissionPathsSent;

   struct NetRate
   {
      U32 updateDelay;
      S32 packetSize;
      bool changed;
   };

   NetRate mCurRate;
   NetRate mMaxRate;

   /// If we're doing a "short circuited" connection, this stores
   /// a pointer to the other side.
   SimObjectPtr<NetConnection> mRemoteConnection;

   NetAddress mNetAddress;

   /// @}


   /// @name Timeout Management
   /// @{

   U32 mPingSendCount;
   U32 mPingRetryCount;
   U32 mLastPingSendTime;
   /// @}

   /// @name Connection Table
   ///
   /// We store our connections on a hash table so we can
   /// quickly find them.
   /// @{

   NetConnection *mNextTableHash;
   static NetConnection *mHashTable[HashTableSize];

   /// @}

protected:
   static SimObjectPtr<NetConnection> mServerConnection;
   static SimObjectPtr<NetConnection> mLocalClientConnection;

   static bool mFilesWereDownloaded;

   U32 mConnectSendCount;
   U32 mConnectLastSendTime;

   SimObjectPtr<NetConnection> getRemoteConnection() { return mRemoteConnection; }

public:
   static NetConnection *getConnectionToServer() { return mServerConnection; }

   static NetConnection *getLocalClientConnection() { return mLocalClientConnection; }
   static void setLocalClientConnection(NetConnection *conn) { mLocalClientConnection = conn; }

   U32 getNetClassGroup() { return mNetClassGroup; }
   static bool filesWereDownloaded() { return mFilesWereDownloaded; }
   static String &getErrorBuffer() { return mErrorBuffer; }

#ifdef TORQUE_DEBUG_NET
   bool mLogging;
   void setLogging(bool logging) { mLogging = logging; }
#endif

   void setSimulatedNetParams(F32 packetLoss, U32 ping)
      { mSimulatedPacketLoss = packetLoss; mSimulatedPing = ping; }

   bool isConnectionToServer()           { return mTypeFlags.test(ConnectionToServer); }
   bool isLocalConnection()            { return !mRemoteConnection.isNull() ; }
   bool isNetworkConnection()          { return mTypeFlags.test(NetworkConnection); }

   void setIsConnectionToServer()        { mTypeFlags.set(ConnectionToServer); }
   void setIsLocalClientConnection()   { mTypeFlags.set(LocalClientConnection); }
   void setNetworkConnection(bool net) { mTypeFlags.set(BitSet32(NetworkConnection), net); }

   virtual void setEstablished();

   /// Call this if the "connection" is local to this app. This short-circuits the protocol layer.
   void setRemoteConnectionObject(NetConnection *connection) { mRemoteConnection = connection; };

   void setSequence(U32 connectSequence);

   void setAddressDigest(U32 digest[4]);
   void getAddressDigest(U32 digest[4]);

   U32 getSequence();

   void setProtocolVersion(U32 protocolVersion) { mProtocolVersion = protocolVersion; }
   U32 getProtocolVersion()                     { return mProtocolVersion; }
   F32 getRoundTripTime()                       { return mRoundTripTime; }
   F32 getPacketLoss()                          { return( mPacketLoss ); }

   static String mErrorBuffer;
   static void setLastError(const char *fmt,...);

   void checkMaxRate();
   void handlePacket(BitStream *stream);
   void processRawPacket(BitStream *stream);
   void handleNotify(bool recvd);
   void handleConnectionEstablished();
   void keepAlive();

   const NetAddress *getNetAddress();
   void setNetAddress(const NetAddress *address);
   Net::Error sendPacket(BitStream *stream);

private:
   void netAddressTableInsert();
   void netAddressTableRemove();

public:
   /// Find a NetConnection, if any, with the specified address.
   static NetConnection *lookup(const NetAddress *remoteAddress);

   bool checkTimeout(U32 time); ///< returns true if the connection timed out

   void checkPacketSend(bool force);

   bool missionPathsSent() const          { return mMissionPathsSent; }
   void setMissionPathsSent(const bool s) { mMissionPathsSent = s; }

   static void consoleInit();

   void onRemove();

   NetConnection();
   ~NetConnection();

public:
   enum NetConnectionState
   {
      NotConnected,
      AwaitingChallengeResponse, ///< We've sent a challenge request, awaiting the response.
      AwaitingConnectRequest,    ///< We've received a challenge request and sent a challenge response.
      AwaitingConnectResponse,   ///< We've received a challenge response and sent a connect request.
      Connected,                 ///< We've accepted a connect request, or we've received a connect response accept.
   };

   U32 mConnectionSendCount;  ///< number of connection messages we've sent.
   U32 mConnectionState;      ///< State of the connection, from NetConnectionState.

   void setConnectionState(U32 state) { mConnectionState = state; }
   U32 getConnectionState() { return mConnectionState; }


   void setGhostFrom(bool ghostFrom);     ///< Sets whether ghosts transmit from this side of the connection.
   void setGhostTo(bool ghostTo);         ///< Sets whether ghosts are allowed from the other side of the connection.
   void setSendingEvents(bool sending);   ///< Sets whether this side actually sends the events that are posted to it.
   void setTranslatesStrings(bool xl);    ///< Sets whether this connection is capable of translating strings.
   void setNetClassGroup(U32 group);      ///< Sets the group of NetClasses this connection traffics in.
   bool isEstablished() { return mEstablished; }   ///< Is the connection established?

   DECLARE_CONOBJECT(NetConnection);
   DECLARE_INSCOPE( NetAPI );

   /// Structure to track packets and what we sent over them.
   ///
   /// We need to know what is sent in each packet, so that if a packet is
   /// dropped, we know what to resend. This is the structure we use to track
   /// this data.
   struct PacketNotify
   {
      bool rateChanged;       ///< Did the rate change on this packet?
      bool maxRateChanged;    ///< Did the max rate change on this packet?
      U32  sendTime;          ///< Timestampe, when we sent this packet.

      NetEventNote *eventList;    ///< Linked list of events sent over this packet.
      GhostRef *ghostList;    ///< Linked list of ghost updates we sent in this packet.
      SubPacketRef *subList;  ///< Defined by subclass - used as desired.

      PacketNotify *nextPacket;  ///< Next packet sent.
      PacketNotify();
   };
   virtual PacketNotify *allocNotify();
   PacketNotify *mNotifyQueueHead;  ///< Head of packet notify list.
   PacketNotify *mNotifyQueueTail;  ///< Tail of packet notify list.

protected:
   virtual void readPacket(BitStream *bstream);
   virtual void writePacket(BitStream *bstream, PacketNotify *note);
   virtual void packetReceived(PacketNotify *note);
   virtual void packetDropped(PacketNotify *note);
   virtual void connectionError(const char *errorString);

//----------------------------------------------------------------
/// @name Event Manager
/// @{

private:
   NetEventNote *mSendEventQueueHead;
   NetEventNote *mSendEventQueueTail;
   NetEventNote *mUnorderedSendEventQueueHead;
   NetEventNote *mUnorderedSendEventQueueTail;
   NetEventNote *mWaitSeqEvents;
   NetEventNote *mNotifyEventList;

   static FreeListChunker<NetEventNote> mEventNoteChunker;

   bool mSendingEvents;

   S32 mNextSendEventSeq;
   S32 mNextRecvEventSeq;
   S32 mLastAckedEventSeq;

   enum NetEventConstants {
      InvalidSendEventSeq = -1,
      FirstValidSendEventSeq = 0
   };

   void eventOnRemove();

   void eventPacketDropped(PacketNotify *notify);
   void eventPacketReceived(PacketNotify *notify);

   void eventWritePacket(BitStream *bstream, PacketNotify *notify);
   void eventReadPacket(BitStream *bstream);

   void eventWriteStartBlock(ResizeBitStream *stream);
   void eventReadStartBlock(BitStream *stream);
public:
   /// Post an event to this connection.
   bool postNetEvent(NetEvent *event);

/// @}

//----------------------------------------------------------------
/// @name Networked string table
/// @{

private:
   bool mTranslateStrings;
   ConnectionStringTable *mStringTable;
public:
   void mapString(U32 netId, NetStringHandle &string)
      { mStringTable->mapString(netId, string); }
   U32  checkString(NetStringHandle &string, bool *isOnOtherSide = NULL)
      { if(mStringTable) return mStringTable->checkString(string, isOnOtherSide); else return 0; }
   U32  getNetSendId(NetStringHandle &string)
      { if(mStringTable) return mStringTable->getNetSendId(string); else return 0;}
   void confirmStringReceived(NetStringHandle &string, U32 index)
      { if(!isRemoved()) mStringTable->confirmStringReceived(string, index); }

   NetStringHandle translateRemoteStringId(U32 id) { return mStringTable->lookupString(id); }
   void         validateSendString(const char *str);

   void   packString(BitStream *stream, const char *str);
   void unpackString(BitStream *stream, char readBuffer[1024]);

   void           packNetStringHandleU(BitStream *stream, NetStringHandle &h);
   NetStringHandle unpackNetStringHandleU(BitStream *stream);
/// @}

//----------------------------------------------------------------
/// @name Ghost manager
/// @{

protected:
   enum GhostStates
   {
      GhostAlwaysDone,
      ReadyForNormalGhosts,
      EndGhosting,
      GhostAlwaysStarting,
      SendNextDownloadRequest,
      FileDownloadSizeMessage,
      NumConnectionMessages,
   };
   GhostInfo **mGhostArray;    ///< Linked list of ghostInfos ghosted by this side of the connection

   U32 mGhostZeroUpdateIndex;  ///< Index in mGhostArray of first ghost with 0 update mask.
   U32 mGhostFreeIndex;        ///< Index in mGhostArray of first free ghost.

   U32 mGhostsActive;			///- Track actve ghosts on client side

   bool mGhosting;             ///< Am I currently ghosting objects?
   bool mScoping;              ///< am I currently scoping objects?
   U32  mGhostingSequence;     ///< Sequence number describing this ghosting session.

   NetObject **mLocalGhosts;  ///< Local ghost for remote object.
                              ///
                              /// mLocalGhosts pointer is NULL if mGhostTo is false

   GhostInfo *mGhostRefs;           ///< Allocated array of ghostInfos. Null if ghostFrom is false.
   GhostInfo **mGhostLookupTable;   ///< Table indexed by object id to GhostInfo. Null if ghostFrom is false.

   /// The object around which we are scoping this connection.
   ///
   /// This is usually the player object, or a related object, like a vehicle
   /// that the player is driving.
   SimObjectPtr<NetObject> mScopeObject;

   void clearGhostInfo();
   bool validateGhostArray();

   void ghostPacketDropped(PacketNotify *notify);
   void ghostPacketReceived(PacketNotify *notify);

   void ghostWritePacket(BitStream *bstream, PacketNotify *notify);
   void ghostReadPacket(BitStream *bstream);
   void freeGhostInfo(GhostInfo *);

   void ghostWriteStartBlock(ResizeBitStream *stream);
   void ghostReadStartBlock(BitStream *stream);

   virtual void ghostWriteExtra(NetObject *,BitStream *) {}
   virtual void ghostReadExtra(NetObject *,BitStream *, bool newGhost) {}
   virtual void ghostPreRead(NetObject *, bool newGhost) {}
   
   /// Called when 'EndGhosting' message is received from server.
   virtual void onEndGhosting() {}

public:
   /// Some configuration values.
   enum GhostConstants
   {
      GhostIdBitSize = 18, //262,144 ghosts
      MaxGhostCount = 1 << GhostIdBitSize, //4096,
      GhostLookupTableSize = 1 << GhostIdBitSize, //4096
      GhostIndexBitSize = 4 // number of bits GhostIdBitSize-3 fits into
   };

   U32 getGhostsActive() { return mGhostsActive;};

   /// Are we ghosting to someone?
   bool isGhostingTo() { return mLocalGhosts != NULL; };

   /// Are we ghosting from someone?
   bool isGhostingFrom() { return mGhostArray != NULL; };

   /// Called by onRemove, to shut down the ghost subsystem.
   void ghostOnRemove();

   /// Called when we're done with normal scoping.
   ///
   /// This gives subclasses a chance to shove things into scope, such as
   /// the results of a sensor network calculation, that would otherwise
   /// be awkward to add.
   virtual void doneScopingScene() { /* null */ }

   /// Set the object around which we are currently scoping network traffic.
   void setScopeObject(NetObject *object);

   /// Get the object around which we are currently scoping network traffic.
   NetObject *getScopeObject();

   /// Add an object to scope.
   void objectInScope(NetObject *object);

   /// Add an object to scope, marking that it should always be scoped to this connection.
   void objectLocalScopeAlways(NetObject *object);

   /// Mark an object that is being ghosted as not always needing to be scoped.
   ///
   /// This undoes objectLocalScopeAlways(), but doesn't immediately flush it from scope.
   ///
   /// Instead, the standard scoping mechanisms will clear it from scope when it is appropos
   /// to do so.
   void objectLocalClearAlways(NetObject *object);

   /// Get a NetObject* from a ghost ID (on client side).
   NetObject *resolveGhost(S32 id);

   /// Get a NetObject* from a ghost index (on the server side).
   NetObject *resolveObjectFromGhostIndex(S32 id);

   /// Get the ghost index corresponding to a given NetObject. This is only
   /// meaningful on the server side.
   S32 getGhostIndex(NetObject *object);

   /// Move a GhostInfo into the nonzero portion of the list (so that we know to update it).
   void ghostPushNonZero(GhostInfo *gi);

   /// Move a GhostInfo into the zero portion of the list (so that we know not to update it).
   void ghostPushToZero(GhostInfo *gi);

   /// Move a GhostInfo from the zero portion of the list to the free portion.
   void ghostPushZeroToFree(GhostInfo *gi);

   /// Move a GhostInfo from the free portion of the list to the zero portion.
   inline void ghostPushFreeToZero(GhostInfo *info);

   /// Stop all ghosting activity and inform the other side about this.
   ///
   /// Turns off ghosting.
   void resetGhosting();

   /// Activate ghosting, once it's enabled.
   void activateGhosting();

   /// Are we ghosting?
   bool isGhosting() { return mGhosting; }

   /// Begin to stop ghosting an object.
   void detachObject(GhostInfo *info);

   /// Mark an object to be always ghosted. Index is the ghost index of the object.
   void setGhostAlwaysObject(NetObject *object, U32 index);


   /// Send ghost connection handshake message.
   ///
   /// As part of the ghost connection process, extensive hand-shaking must be performed.
   ///
   /// This is done by passing ConnectionMessageEvents; this is a helper function
   /// to more effectively perform this task. Messages are dealt with by
   /// handleConnectionMessage().
   ///
   /// @param  message     One of GhostStates
   /// @param  sequence    A sequence number, if any.
   /// @param  ghostCount  A count of ghosts relating to this message.
   void sendConnectionMessage(U32 message, U32 sequence = 0, U32 ghostCount = 0);

   /// Handle message from sendConnectionMessage().
   ///
   /// This is called to handle messages sent via sendConnectionMessage.
   ///
   /// @param  message     One of GhostStates
   /// @param  sequence    A sequence number, if any.
   /// @param  ghostCount  A count of ghosts relating to this message.
   virtual void handleConnectionMessage(U32 message, U32 sequence, U32 ghostCount);

   /// Sends a signal to any object that needs to wait till everything has been ghosted
   /// before performing an operation.
   static Signal<void()> smGhostAlwaysDone;

   /// @}
public:
//----------------------------------------------------------------
/// @name File transfer
/// @{

protected:
   /// List of files missing for this connection.
   ///
   /// The currently downloading file is always first in the list (ie, [0]).
   Vector<char *> mMissingFileList;

   /// Stream for currently uploading file (if any).
   Stream *mCurrentDownloadingFile;

   /// Storage for currently downloading file.
   void *mCurrentFileBuffer;

   /// Size of currently downloading file in bytes.
   U32 mCurrentFileBufferSize;

   /// Our position in the currently downloading file in bytes.
   U32 mCurrentFileBufferOffset;

   /// Number of files we have downloaded.
   U32 mNumDownloadedFiles;

   /// Error storage for file transfers.
   String mLastFileErrorBuffer;

   /// Structure to track ghost-always objects and their ghost indices.
   struct GhostSave {
      NetObject *ghost;
      U32 index;
   };

   /// List of objects to ghost-always.
   Vector<GhostSave> mGhostAlwaysSaveList;

public:
   /// Start sending the specified file over the link.
   bool startSendingFile(const char *fileName);

   /// Called when we receive a FileChunkEvent.
   void chunkReceived(U8 *chunkData, U32 chunkLen);

   /// Get the next file...
   void sendNextFileDownloadRequest();

   /// Post the next FileChunkEvent.
   void sendFileChunk();

   /// Called when we finish downloading file data.
   virtual void fileDownloadSegmentComplete();

   /// This is part of the file transfer logic; basically, we call this
   /// every time we finish downloading new files. It attempts to load
   /// the GhostAlways objects; if they fail, it marks an error and we
   /// have chance to retry.
   void loadNextGhostAlwaysObject(bool hadNewFiles);
/// @}

//----------------------------------------------------------------
/// @name Demo Recording
/// @{

private:
   Stream *mDemoWriteStream;
   Stream *mDemoReadStream;
   U32 mDemoNextBlockType;
   U32 mDemoNextBlockSize;

   U32 mDemoWriteStartTime;
   U32 mDemoReadStartTime;
   U32 mDemoLastWriteTime;

   U32 mDemoRealStartTime;

public:
   enum DemoBlockTypes {
      BlockTypePacket,
      BlockTypeSendPacket,
      NetConnectionBlockTypeCount
   };

   enum DemoConstants {
      MaxNumBlockTypes = 16,
      MaxBlockSize = 0x1000,
   };

   bool isRecording()
      { return mDemoWriteStream != NULL; }
   bool isPlayingBack()
      { return mDemoReadStream != NULL; }

   U32 getNextBlockType() { return mDemoNextBlockType; }
   void recordBlock(U32 type, U32 size, void *data);
   virtual void handleRecordedBlock(U32 type, U32 size, void *data);
   bool processNextBlock();

   bool startDemoRecord(const char *fileName);
   bool replayDemoRecord(const char *fileName);
   void startDemoRead();
   void stopRecording();
   void stopDemoPlayback();

   virtual void writeDemoStartBlock(ResizeBitStream *stream);
   virtual bool readDemoStartBlock(BitStream *stream);
   virtual void demoPlaybackComplete();
/// @}
};


//----------------------------------------------------------------------------
/// Information about a ghosted object.
///
/// @note If the size of this structure changes, the
///       NetConnection::getGhostIndex function MUST be changed
///       to reflect the new size.
struct GhostInfo
{
public:  // required for MSVC
   NetObject *obj;                        ///< The object being ghosted.
   U32 updateMask;                        ///< Flags indicating what state data needs to be transferred.

   U32 updateSkipCount;                   ///< How many updates have we skipped this guy?
   U32 flags;                             ///< Flags from GhostInfo::Flags
   F32 priority;                          ///< A float value indicating the priority of this object for
                                          ///  updates.

   /// @name References
   ///
   /// The GhostInfo structure is used in several linked lists; these members are
   /// the implementation for this.
   /// @{

   NetConnection::GhostRef *updateChain;  ///< List of references in NetConnections to us.

   GhostInfo *nextObjectRef;              ///< Next ghosted object.
   GhostInfo *prevObjectRef;              ///< Previous ghosted object.
   NetConnection *connection;             ///< Connection that we're ghosting over.
   GhostInfo *nextLookupInfo;             ///< GhostInfo references are stored in a hash; this is the bucket
                                          ///  implementation.

   /// @}

   U32 index;
   U32 arrayIndex;

   /// Flags relating to the state of the object.
   enum Flags
   {
      Valid             = BIT(0),
      InScope           = BIT(1),
      ScopeAlways       = BIT(2),
      NotYetGhosted     = BIT(3),
      Ghosting          = BIT(4),
      KillGhost         = BIT(5),
      KillingGhost      = BIT(6),
      ScopedEvent       = BIT(7),
      ScopeLocalAlways  = BIT(8),
   };
};

inline void NetConnection::ghostPushNonZero(GhostInfo *info)
{
   AssertFatal(info->arrayIndex >= mGhostZeroUpdateIndex && info->arrayIndex < mGhostFreeIndex, "Out of range arrayIndex.");
   AssertFatal(mGhostArray[info->arrayIndex] == info, "Invalid array object.");
   if(info->arrayIndex != mGhostZeroUpdateIndex)
   {
      mGhostArray[mGhostZeroUpdateIndex]->arrayIndex = info->arrayIndex;
      mGhostArray[info->arrayIndex] = mGhostArray[mGhostZeroUpdateIndex];
      mGhostArray[mGhostZeroUpdateIndex] = info;
      info->arrayIndex = mGhostZeroUpdateIndex;
   }
   mGhostZeroUpdateIndex++;
   //AssertFatal(validateGhostArray(), "Invalid ghost array!");
}

inline void NetConnection::ghostPushToZero(GhostInfo *info)
{
   AssertFatal(info->arrayIndex < mGhostZeroUpdateIndex, "Out of range arrayIndex.");
   AssertFatal(mGhostArray[info->arrayIndex] == info, "Invalid array object.");
   mGhostZeroUpdateIndex--;
   if(info->arrayIndex != mGhostZeroUpdateIndex)
   {
      mGhostArray[mGhostZeroUpdateIndex]->arrayIndex = info->arrayIndex;
      mGhostArray[info->arrayIndex] = mGhostArray[mGhostZeroUpdateIndex];
      mGhostArray[mGhostZeroUpdateIndex] = info;
      info->arrayIndex = mGhostZeroUpdateIndex;
   }
   //AssertFatal(validateGhostArray(), "Invalid ghost array!");
}

inline void NetConnection::ghostPushZeroToFree(GhostInfo *info)
{
   AssertFatal(info->arrayIndex >= mGhostZeroUpdateIndex && info->arrayIndex < mGhostFreeIndex, "Out of range arrayIndex.");
   AssertFatal(mGhostArray[info->arrayIndex] == info, "Invalid array object.");
   mGhostFreeIndex--;
   if(info->arrayIndex != mGhostFreeIndex)
   {
      mGhostArray[mGhostFreeIndex]->arrayIndex = info->arrayIndex;
      mGhostArray[info->arrayIndex] = mGhostArray[mGhostFreeIndex];
      mGhostArray[mGhostFreeIndex] = info;
      info->arrayIndex = mGhostFreeIndex;
   }
   //AssertFatal(validateGhostArray(), "Invalid ghost array!");
}

inline void NetConnection::ghostPushFreeToZero(GhostInfo *info)
{
   AssertFatal(info->arrayIndex >= mGhostFreeIndex, "Out of range arrayIndex.");
   AssertFatal(mGhostArray[info->arrayIndex] == info, "Invalid array object.");
   if(info->arrayIndex != mGhostFreeIndex)
   {
      mGhostArray[mGhostFreeIndex]->arrayIndex = info->arrayIndex;
      mGhostArray[info->arrayIndex] = mGhostArray[mGhostFreeIndex];
      mGhostArray[mGhostFreeIndex] = info;
      info->arrayIndex = mGhostFreeIndex;
   }
   mGhostFreeIndex++;
   //AssertFatal(validateGhostArray(), "Invalid ghost array!");
}

#endif


