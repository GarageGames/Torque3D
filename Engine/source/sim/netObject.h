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

#ifndef _NETOBJECT_H_
#define _NETOBJECT_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _MMATH_H_
#include "math/mMath.h"
#endif


//-----------------------------------------------------------------------------
class NetConnection;
class NetObject;

//-----------------------------------------------------------------------------

struct CameraScopeQuery
{
   NetObject *camera;       ///< Pointer to the viewing object.
   Point3F pos;             ///< Position in world space
   Point3F orientation;     ///< Viewing vector in world space
   F32 fov;                 ///< Viewing angle/2
   F32 sinFov;              ///< sin(fov/2);
   F32 cosFov;              ///< cos(fov/2);
   F32 visibleDistance;     ///< Visible distance.
};

struct GhostInfo;


//-----------------------------------------------------------------------------
/// Superclass for ghostable networked objects.
///
/// @section NetObject_intro Introduction To NetObject And Ghosting
///
/// One of the most powerful aspects of Torque's networking code is its support
/// for ghosting and prioritized, most-recent-state network updates. The way
/// this works is a bit complex, but it is immensely efficient. Let's run
/// through the steps that the server goes through for each client in this part
/// of Torque's networking:
///      - First, the server determines what objects are in-scope for the client.
///        This is done by calling onCameraScopeQuery() on the object which is
///        considered the "scope" object. This is usually the player object, but
///        it can be something else. (For instance, the current vehicle, or a
///        object we're remote controlling.)
///      - Second, it ghosts them to the client; this is implemented in netGhost.cc.
///      - Finally, it sends updates as needed, by checking the dirty list and packing
///        updates.
///
/// There several significant advantages to using this networking system:
///      - Efficient network usage, since we only send data that has changed. In addition,
///        since we only care about most-recent data, if a packet is dropped, we don't waste
///        effort trying to deliver stale data.
///      - Cheating protection; since we don't deliver information about game objects which
///        aren't in scope, we dramatically reduce the ability of clients to hack the game and
///        gain a meaningful advantage. (For instance, they can't find out about things behind
///        them, since objects behind them don't fall in scope.) In addition, since ghost IDs are
///        assigned per-client, it's difficult for any sort of co-ordination between cheaters to
///        occur.
///
/// NetConnection contains the Ghost Manager implementation, which deals with transferring data to
/// the appropriate clients and keeping state in synch.
///
/// @section NetObject_Implementation An Example Implementation
///
/// The basis of the ghost implementation in Torque is NetObject. It tracks the dirty flags for the
/// various states that the object trackers, and does some other book-keeping to allow more efficient
/// operation of the networking layer.
///
/// Using a NetObject is very simple; let's go through a simple example implementation:
///
/// @code
/// class SimpleNetObject : public NetObject
/// {
/// public:
///   typedef NetObject Parent;
///   DECLARE_CONOBJECT(SimpleNetObject);
/// @endcode
///
/// Above is the standard boilerplate code for a Torque class. You can find out more about this in SimObject.
///
/// @code
///    char message1[256];
///    char message2[256];
///    enum States {
///       Message1Mask = BIT(0),
///       Message2Mask = BIT(1),
///    };
/// @endcode
///
/// For our example, we're having two "states" that we keep track of, message1 and message2. In a real
/// object, we might map our states to health and position, or some other set of fields. You have 32
/// bits to work with, so it's possible to be very specific when defining states. In general, you
/// should try to use as few states as possible (you never know when you'll need to expand your object's
/// functionality!), and in fact, most of your fields will end up changing all at once, so it's not worth
/// it to be too fine-grained. (As an example, position and velocity on Player are controlled by the same
/// bit, as one rarely changes without the other changing, too.)
///
/// @code
///    SimpleNetObject()
///    {
///       // in order for an object to be considered by the network system,
///       // the Ghostable net flag must be set.
///       // the ScopeAlways flag indicates that the object is always scoped
///       // on all active connections.
///       mNetFlags.set(ScopeAlways | Ghostable);
///       dStrcpy(message1, "Hello World 1!");
///       dStrcpy(message2, "Hello World 2!");
///    }
/// @endcode
///
/// Here is the constructor. Here, you see that we initialize our net flags to show that
/// we should always be scoped, and that we're to be taken into consideration for ghosting. We
/// also provide some initial values for the message fields.
///
/// @code
///    U32 packUpdate(NetConnection *, U32 mask, BitStream *stream)
///    {
///       // check which states need to be updated, and update them
///       if(stream->writeFlag(mask & Message1Mask))
///          stream->writeString(message1);
///       if(stream->writeFlag(mask & Message2Mask))
///          stream->writeString(message2);
///
///       // the return value from packUpdate can set which states still
///       // need to be updated for this object.
///       return 0;
///    }
/// @endcode
///
/// Here's half of the meat of the networking code, the packUpdate() function. (The other half, unpackUpdate(),
/// we'll get to in a second.) The comments in the code pretty much explain everything, however, notice that the
/// code follows a pattern of if(writeFlag(mask & StateMask)) { ... write data ... }. The packUpdate()/unpackUpdate()
/// functions are responsible for reading and writing the dirty bits to the bitstream by themselves.
///
/// @code
///    void unpackUpdate(NetConnection *, BitStream *stream)
///    {
///       // the unpackUpdate function must be symmetrical to packUpdate
///       if(stream->readFlag())
///       {
///          stream->readString(message1);
///          Con::printf("Got message1: %s", message1);
///       }
///       if(stream->readFlag())
///       {
///          stream->readString(message2);
///          Con::printf("Got message2: %s", message2);
///       }
///    }
/// @endcode
///
/// The other half of the networking code in any NetObject, unpackUpdate(). In our simple example, all that
/// the code does is print the new messages to the console; however, in a more advanced object, you might
/// trigger animations, update complex object properties, or even spawn new objects, based on what packet
/// data you unpack.
///
/// @code
///    void setMessage1(const char *msg)
///    {
///       setMaskBits(Message1Mask);
///       dStrcpy(message1, msg);
///    }
///    void setMessage2(const char *msg)
///    {
///       setMaskBits(Message2Mask);
///       dStrcpy(message2, msg);
///    }
/// @endcode
///
/// Here are the accessors for the two properties. It is good to encapsulate your state
/// variables, so that you don't have to remember to make a call to setMaskBits every time you change
/// anything; the accessors can do it for you. In a more complex object, you might need to set
/// multiple mask bits when you change something; this can be done using the | operator, for instance,
/// setMaskBits( Message1Mask | Message2Mask ); if you changed both messages.
///
/// @code
/// IMPLEMENT_CO_NETOBJECT_V1(SimpleNetObject);
///
/// ConsoleMethod(SimpleNetObject, setMessage1, void, 3, 3, "(string msg) Set message 1.")
/// {
///    object->setMessage1(argv[2]);
/// }
///
/// ConsoleMethod(SimpleNetObject, setMessage2, void, 3, 3, "(string msg) Set message 2.")
/// {
///    object->setMessage2(argv[2]);
/// }
/// @endcode
///
/// Finally, we use the NetObject implementation macro, IMPLEMENT_CO_NETOBJECT_V1(), to implement our
/// NetObject. It is important that we use this, as it makes Torque perform certain initialization tasks
/// that allow us to send the object over the network. IMPLEMENT_CONOBJECT() doesn't perform these tasks, see
/// the documentation on AbstractClassRep for more details.
///
/// @nosubgrouping
class NetObject: public SimObject
{
   // The Ghost Manager needs read/write access
   friend class  NetConnection;
   friend struct GhostInfo;
   friend class  ProcessList;

   // Not the best way to do this, but the event needs access to mNetFlags
   friend class GhostAlwaysObjectEvent;

private:
   typedef SimObject Parent;

   /// Mask indicating which states are dirty and need to be retransmitted on this
   /// object.
   U32 mDirtyMaskBits;

   /// @name Dirty List
   ///
   /// Whenever a NetObject becomes "dirty", we add it to the dirty list.
   /// We also remove ourselves on the destructor.
   ///
   /// This is done so that when we want to send updates (in NetConnection),
   /// it's very fast to find the objects that need to be updated.
   /// @{

   /// Static pointer to the head of the dirty NetObject list.
   static NetObject *mDirtyList;

   /// Next item in the dirty list...
   NetObject *mPrevDirtyList;

   /// Previous item in the dirty list...
   NetObject *mNextDirtyList;

   /// @}
protected:

   /// Pointer to the server object on a local connection.
   /// @see getServerObject
   SimObjectPtr<NetObject> mServerObject;

   /// Pointer to the client object on a local connection.
   /// @see getClientObject
   SimObjectPtr<NetObject> mClientObject;

   enum NetFlags
   {
      IsGhost           =  BIT(1),   ///< This is a ghost.
      ScopeAlways       =  BIT(6),  ///< Object always ghosts to clients.
      ScopeLocal        =  BIT(7),  ///< Ghost only to local client.
      Ghostable         =  BIT(8),  ///< Set if this object CAN ghost.

      MaxNetFlagBit     =  15
   };

   BitSet32 mNetFlags;              ///< Flag values from NetFlags
   U32 mNetIndex;                   ///< The index of this ghost in the GhostManager on the server.

   GhostInfo *mFirstObjectRef;      ///< Head of a linked list storing GhostInfos referencing this NetObject.

public:
   NetObject();
   ~NetObject();

   virtual String describeSelf() const;

   /// @name Miscellaneous
   /// @{
   DECLARE_CONOBJECT(NetObject);
   static void initPersistFields();
   bool onAdd();
   void onRemove();
   /// @}

   static void collapseDirtyList();

   /// Used to mark a bit as dirty; ie, that its corresponding set of fields need to be transmitted next update.
   ///
   /// @param   orMask   Bit(s) to set
   virtual void setMaskBits(U32 orMask);

   /// Clear the specified bits from the dirty mask.
   ///
   /// @param   orMask   Bits to clear
   virtual void clearMaskBits(U32 orMask);
   virtual U32 filterMaskBits(U32 mask, NetConnection * connection) { return mask; }

   ///  Scope the object to all connections.
   ///
   ///  The object is marked as ScopeAlways and is immediately ghosted to
   ///  all active connections.  This function has no effect if the object
   ///  is not marked as Ghostable.
   void setScopeAlways();

   /// Stop scoping the object to all connections.
   ///
   /// The object's ScopeAlways flag is cleared and the object is removed from
   /// all current active connections.
   void clearScopeAlways();

   /// This returns a value which is used to prioritize which objects need to be updated.
   ///
   /// In NetObject, our returned priority is 0.1 * updateSkips, so that less recently
   /// updated objects are more likely to be updated.
   ///
   /// In subclasses, this can be adjusted. For instance, ShapeBase provides priority
   /// based on proximity to the camera.
   ///
   /// @param  focusObject    Information from a previous call to onCameraScopeQuery.
   /// @param  updateMask     Current update mask.
   /// @param  updateSkips    Number of ticks we haven't been updated for.
   /// @returns A floating point value indicating priority. These are typically < 5.0.
   virtual F32 getUpdatePriority(CameraScopeQuery *focusObject, U32 updateMask, S32 updateSkips);

   /// Instructs this object to pack its state for transfer over the network.
   ///
   /// @param   conn    Net connection being used
   /// @param   mask    Mask indicating fields to transmit.
   /// @param   stream  Bitstream to pack data to
   ///
   /// @returns Any bits which were not dealt with. The value is stored by the networking
   ///          system. Don't set bits you weren't passed.
   virtual U32  packUpdate(NetConnection * conn, U32 mask, BitStream *stream);

   /// Instructs this object to read state data previously packed with packUpdate.
   ///
   /// @param   conn    Net connection being used
   /// @param   stream  stream to read from
   virtual void unpackUpdate(NetConnection * conn, BitStream *stream);

   /// Queries the object about information used to determine scope.
   ///
   /// Something that is 'in scope' is somehow interesting to the client.
   ///
   /// If we are a NetConnection's scope object, it calls this method to determine
   /// how things should be scoped; basically, we tell it our field of view with camInfo,
   /// and have the opportunity to manually mark items as "in scope" as we see fit.
   ///
   /// By default, we just mark all ghostable objects as in scope.
   ///
   /// @param   cr         Net connection requesting scope information.
   /// @param   camInfo    Information about what this object can see.
   virtual void onCameraScopeQuery(NetConnection *cr, CameraScopeQuery *camInfo);

   /// Get the ghost index of this object.
   U32 getNetIndex() { return mNetIndex; }

   bool isServerObject() const;  ///< Is this a server object?
   bool isClientObject() const;  ///< Is this a client object?

   bool isGhost() const;         ///< Is this is a ghost?
   bool isScopeLocal() const;    ///< Should this object only be visible to the client which created it?
   bool isScopeable() const;     ///< Is this object subject to scoping?
   bool isGhostable() const;     ///< Is this object ghostable?
   bool isGhostAlways() const;   ///< Should this object always be ghosted?


   /// @name Short-Circuited Networking
   ///
   /// When we are running with client and server on the same system (which can happen be either
   /// when we are doing a single player game, or if we're hosting a multiplayer game and having
   /// someone playing on the same instance), we can do some short circuited code to enhance
   /// performance.
   ///
   /// These variables are used to make it simpler; if we are running in short-circuited mode, 
   /// the ghosted client gets the server object while the server gets the client object.
   ///
   /// @note "Premature optimization is the root of all evil" - Donald Knuth. The current codebase
   ///       uses this feature in three small places, mostly for non-speed-related purposes.
   ///
   /// @{
   
   /// Returns a pointer to the server object when on a local connection.
   NetObject* getServerObject() const { return mServerObject; }

   /// Returns a pointer to the client object when on a local connection.
   NetObject* getClientObject() const { return mClientObject; }
   
   /// Template form for the callers convenience.
   template < class T >
   static T* getServerObject( T *netObj ) { return static_cast<T*>( netObj->getServerObject() ); }   

   /// Template form for the callers convenience.
   template < class T >
   static T* getClientObject( T *netObj ) { return static_cast<T*>( netObj->getClientObject() ); }

   /// @}
};

//-----------------------------------------------------------------------------

inline bool NetObject::isGhost() const
{
   return mNetFlags.test(IsGhost);
}

inline bool NetObject::isClientObject() const
{
   return mNetFlags.test(IsGhost);
}

inline bool NetObject::isServerObject() const
{
   return !mNetFlags.test(IsGhost);
}

inline bool NetObject::isScopeLocal() const
{
   return mNetFlags.test(ScopeLocal);
}

inline bool NetObject::isScopeable() const
{
   return mNetFlags.test(Ghostable) && !mNetFlags.test(ScopeAlways);
}

inline bool NetObject::isGhostable() const
{
   return mNetFlags.test(Ghostable);
}

inline bool NetObject::isGhostAlways() const
{
   AssertFatal(mNetFlags.test(Ghostable) || mNetFlags.test(ScopeAlways) == false,
               "That's strange, a ScopeAlways non-ghostable object?  Something wrong here");
   return mNetFlags.test(Ghostable) && mNetFlags.test(ScopeAlways);
}

#endif
