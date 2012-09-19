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

#ifndef _GAMEBASE_H_
#define _GAMEBASE_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _PROCESSLIST_H_
#include "T3D/gameBase/processList.h"
#endif
#ifndef _TICKCACHE_H_
#include "T3D/gameBase/tickCache.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif

class NetConnection;
class ProcessList;
class GameBase;
struct Move;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

/// Scriptable, demo-able datablock.
///
/// This variant of SimDataBlock performs these additional tasks:
///   - Linking datablock's namepsaces to the namespace of their C++ class, so
///     that datablocks can expose script functionality.
///   - Linking datablocks to a user defined scripting namespace, by setting the
///     'class' field at datablock definition time.
///   - Adds a category field; this is used by the world creator in the editor to
///     classify creatable shapes. Creatable shapes are placed under the Shapes
///     node in the treeview for this; additional levels are created, named after
///     the category fields.
///   - Adds support for demo stream recording. This support takes the form
///     of the member variable packed. When a demo is being recorded by a client,
///     data is unpacked, then packed again to the data stream, then, in the case
///     of datablocks, preload() is called to process the data. It is occasionally
///     the case that certain references in the datablock stream cannot be resolved
///     until preload is called, in which case a raw ID field is stored in the variable
///     which will eventually be used to store a pointer to the object. However, if
///     packData() is called before we resolve this ID, trying to call getID() on the
///     objecct ID would be a fatal error. Therefore, in these cases, we test packed;
///     if it is true, then we know we have to write the raw data, instead of trying
///     to resolve an ID.
///
/// @see SimDataBlock for further details about datablocks.
/// @see http://hosted.tribalwar.com/t2faq/datablocks.shtml for an excellent
///      explanation of the basics of datablocks from a scripting perspective.
/// @nosubgrouping
struct GameBaseData : public SimDataBlock 
{
private:

   typedef SimDataBlock Parent;

public:

   bool packed;
   StringTableEntry category;

   // Signal triggered when this datablock is modified.
   // GameBase objects referencing this datablock notify with this signal.
   Signal<void(void)> mReloadSignal;

   // Triggers the reload signal.
   void inspectPostApply();

   bool onAdd();   

   // The derived class should provide the following:
   DECLARE_CONOBJECT(GameBaseData);
   GameBaseData();
   static void initPersistFields();
   bool preload(bool server, String &errorStr);
   void unpackData(BitStream* stream);

   /// @name Callbacks
   /// @{
   DECLARE_CALLBACK( void, onAdd, ( GameBase* obj ) );
   DECLARE_CALLBACK( void, onRemove, ( GameBase* obj ) );
   DECLARE_CALLBACK( void, onNewDataBlock, ( GameBase* obj ) );
   DECLARE_CALLBACK( void, onMount, ( GameBase* obj, SceneObject* mountObj, S32 node ) );
   DECLARE_CALLBACK( void, onUnmount, ( GameBase* obj, SceneObject* mountObj, S32 node ) );
   /// @}
};

//----------------------------------------------------------------------------
// A few utility methods for sending datablocks over the net
//----------------------------------------------------------------------------

bool UNPACK_DB_ID(BitStream *, U32 & id);
bool PACK_DB_ID(BitStream *, U32 id);
bool PRELOAD_DB(U32 & id, SimDataBlock **, bool server, const char * clientMissing = NULL, const char * serverMissing = NULL);

//----------------------------------------------------------------------------
class GameConnection;
class WaterObject;
class MoveList;

// For truly it is written: "The wise man extends GameBase for his purposes,
// while the fool has the ability to eject shell casings from the belly of his
// dragon." -- KillerBunny

/// Base class for game objects which use datablocks, networking, are editable,
/// and need to process ticks.
///
/// @section GameBase_process GameBase and ProcessList
///
/// GameBase adds two kinds of time-based updates. Torque works off of a concept
/// of ticks. Ticks are slices of time 32 milliseconds in length. There are three
/// methods which are used to update GameBase objects that are registered with
/// the ProcessLists:
///      - processTick(Move*) is called on each object once for every tick, regardless
///        of the "real" framerate.
///      - interpolateTick(float) is called on client objects when they need to interpolate
///        to match the next tick.
///      - advanceTime(float) is called on client objects so they can do time-based behaviour,
///        like updating animations.
///
/// Torque maintains a server and a client processing list; in a local game, both
/// are populated, while in multiplayer situations, either one or the other is
/// populated.
///
/// You can control whether an object is considered for ticking by means of the
/// setProcessTick() method.
///
/// @section GameBase_datablock GameBase and Datablocks
///
/// GameBase adds support for datablocks. Datablocks are secondary classes which store
/// static data for types of game elements. For instance, this means that all "light human
/// male armor" type Players share the same datablock. Datablocks typically store not only
/// raw data, but perform precalculations, like finding nodes in the game model, or
/// validating movement parameters.
///
/// There are three parts to the datablock interface implemented in GameBase:
///      - <b>getDataBlock()</b>, which gets a pointer to the current datablock. This is
///        mostly for external use; for in-class use, it's better to directly access the
///        mDataBlock member.
///      - <b>setDataBlock()</b>, which sets mDataBlock to point to a new datablock; it
///        uses the next part of the interface to inform subclasses of this.
///      - <b>onNewDataBlock()</b> is called whenever a new datablock is assigned to a GameBase.
///
/// Datablocks are also usable through the scripting language.
///
/// @see SimDataBlock for more details.
///
/// @section GameBase_networking GameBase and Networking
///
/// writePacketData() and readPacketData() are called to transfer information needed for client
/// side prediction. They are usually used when updating a client of its control object state.
///
/// Subclasses of GameBase usually transmit positional and basic status data in the packUpdate()
/// functions, while giving velocity, momentum, and similar state information in the writePacketData().
///
/// writePacketData()/readPacketData() are called <i>in addition</i> to packUpdate/unpackUpdate().
///
/// @nosubgrouping
class GameBase : public SceneObject
{      
   typedef SceneObject Parent;

   /// @name Datablock
   /// @{

   GameBaseData*     mDataBlock;

   /// @}

   TickCache mTickCache;
   
   // Control interface
   GameConnection* mControllingClient;

public:

   static bool gShowBoundingBox;    ///< Should we render bounding boxes?
  
protected:

   F32 mCameraFov;

   /// The WaterObject we are currently within.
   WaterObject *mCurrentWaterObject;
   
   static bool setDataBlockProperty( void *object, const char *index, const char *data );

#ifdef TORQUE_DEBUG_NET_MOVES
   U32 mLastMoveId;
   U32 mTicksSinceLastMove;
   bool mIsAiControlled;
#endif   

public:

   GameBase();
   ~GameBase();

   enum GameBaseMasks {      
      DataBlockMask     = Parent::NextFreeMask << 0,
      ExtendedInfoMask  = Parent::NextFreeMask << 1,
      NextFreeMask      = Parent::NextFreeMask << 2
   };

   // net flags added by game base
   enum
   {
      NetOrdered        = BIT(Parent::MaxNetFlagBit+1), /// Process in same order on client and server.
      NetNearbyAdded    = BIT(Parent::MaxNetFlagBit+2), /// Is set during client catchup when neighbors have been checked.
      GhostUpdated      = BIT(Parent::MaxNetFlagBit+3), /// Is set whenever ghost updated (and reset) on the client, for hifi objects.
      TickLast          = BIT(Parent::MaxNetFlagBit+4), /// Tick this object after all others.
      NewGhost          = BIT(Parent::MaxNetFlagBit+5), /// This ghost was just added during the last update.
      HiFiPassive       = BIT(Parent::MaxNetFlagBit+6), /// Do not interact with other hifi passive objects.
      MaxNetFlagBit     = Parent::MaxNetFlagBit+6
   };

   /// @name Inherited Functionality.
   /// @{

   bool onAdd();
   void onRemove();
   void inspectPostApply();
   static void initPersistFields();
   static void consoleInit();

   /// @}

   ///@name Datablock
   ///@{

   /// Assigns this object a datablock and loads attributes with onNewDataBlock.
   ///
   /// @see onNewDataBlock
   /// @param   dptr   Datablock
   bool          setDataBlock( GameBaseData *dptr );

   /// Returns the datablock for this object.
   GameBaseData* getDataBlock()  { return mDataBlock; }

   /// Called when a new datablock is set. This allows subclasses to
   /// appropriately handle new datablocks.
   ///
   /// @see    setDataBlock()
   /// @param  dptr     New datablock
   /// @param  reload   Is this a new datablock or are we reloading one
   ///                  we already had.
   virtual bool onNewDataBlock( GameBaseData *dptr, bool reload );   
   ///@}

   /// @name Script
   /// The scriptOnXX methods are invoked by the leaf classes
   /// @{

   /// Executes the 'onAdd' script function for this object.
   /// @note This must be called after everything is ready
   void scriptOnAdd();

   /// Executes the 'onNewDataBlock' script function for this object.
   ///
   /// @note This must be called after everything is loaded.
   void scriptOnNewDataBlock();

   /// Executes the 'onRemove' script function for this object.
   /// @note This must be called while the object is still valid
   void scriptOnRemove();

   /// @}

   // ProcessObject override
   void processTick( const Move *move ); 

   /// @name GameBase NetFlags & Hifi-Net Interface   
   /// @{
   
   /// Set or clear the GhostUpdated bit in our NetFlags.
   /// @see GhostUpdated
   void setGhostUpdated( bool b ) { if (b) mNetFlags.set(GhostUpdated); else mNetFlags.clear(GhostUpdated); }

   /// Returns true if the GhostUpdated bit in our NetFlags is set.
   /// @see GhostUpdated
   bool isGhostUpdated() const { return mNetFlags.test(GhostUpdated); }

   /// Set or clear the NewGhost bit in our NetFlags.
   /// @see NewGhost
   void setNewGhost( bool n ) { if (n) mNetFlags.set(NewGhost); else mNetFlags.clear(NewGhost); }

   /// Returns true if the NewGhost bit in out NetFlags is set.
   /// @see NewGhost
   bool isNewGhost() const { return mNetFlags.test(NewGhost); }

   /// Set or clear the NetNearbyAdded bit in our NetFlags.
   /// @see NetNearbyAdded
   void setNetNearbyAdded( bool b ) { if (b) mNetFlags.set(NetNearbyAdded); else mNetFlags.clear(NetNearbyAdded); }

   /// Returns true if the NetNearby bit in our NetFlags is set.
   /// @see NetNearbyAdded
   bool isNetNearbyAdded() const { return mNetFlags.test(NetNearbyAdded); }

   /// Returns true if the HiFiPassive bit in our NetFlags is set.
   /// @see HiFiPassive
   bool isHifiPassive() const { return mNetFlags.test(HiFiPassive); }

   /// Returns true if the TickLast bit in our NetFlags is set.
   /// @see TickLast
   bool isTickLast() const { return mNetFlags.test(TickLast); }

   /// Returns true if the NetOrdered bit in our NetFlags is set.
   /// @see NetOrdered
   bool isNetOrdered() const { return mNetFlags.test(NetOrdered); }
   
   /// Called during client catchup under the hifi-net model.
   virtual void computeNetSmooth( F32 backDelta ) {}

   /// Returns TickCache used under the hifi-net model.
   TickCache& getTickCache() { return mTickCache; }
   /// @}

   /// @name Network
   /// @see NetObject, NetConnection
   /// @{

   F32  getUpdatePriority( CameraScopeQuery *focusObject, U32 updateMask, S32 updateSkips );
   U32  packUpdate  ( NetConnection *conn, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *conn,           BitStream *stream );

   /// Write state information necessary to perform client side prediction of an object.
   ///
   /// This information is sent only to the controlling object. For example, if you are a client
   /// controlling a Player, the server uses writePacketData() instead of packUpdate() to
   /// generate the data you receive.
   ///
   /// @param   conn     Connection for which we're generating this data.
   /// @param   stream   Bitstream for output.
   virtual void writePacketData( GameConnection *conn, BitStream *stream );

   /// Read data written with writePacketData() and update the object state.
   ///
   /// @param   conn    Connection for which we're generating this data.
   /// @param   stream  Bitstream to read.
   virtual void readPacketData( GameConnection *conn, BitStream *stream );

   /// Gets the checksum for packet data.
   ///
   /// Basically writes a packet, does a CRC check on it, and returns
   /// that CRC.
   ///
   /// @see writePacketData
   /// @param   conn   Game connection
   virtual U32 getPacketDataChecksum( GameConnection *conn );
   ///@}


   /// @name Mounted objects ( overrides )
   /// @{

public:

   virtual void onMount( SceneObject *obj, S32 node );   
   virtual void onUnmount( SceneObject *obj,S32 node ); 

   /// @}

   /// @name User control
   /// @{

   /// Returns the client controlling this object
   GameConnection *getControllingClient() { return mControllingClient; }
   const GameConnection *getControllingClient() const { return mControllingClient; }

   /// Returns the MoveList of the client controlling this object.
   /// If there is no client it returns NULL;
   MoveList* getMoveList();

   /// Sets the client controlling this object
   /// @param  client   Client that is now controlling this object
   virtual void setControllingClient( GameConnection *client );

   virtual GameBase * getControllingObject() { return NULL; }
   virtual GameBase * getControlObject() { return NULL; }
   virtual void setControlObject( GameBase * ) { }
   /// @}

   virtual F32 getDefaultCameraFov() { return 90.f; }
   virtual F32 getCameraFov() { return 90.f; }
   virtual void setCameraFov( F32 fov )   { }
   virtual bool isValidCameraFov( F32 fov ) { return true; }
   virtual bool useObjsEyePoint() const { return false; }
   virtual bool onlyFirstPerson() const { return false; }
   virtual F32 getDamageFlash() const { return 1.0f; }
   virtual F32 getWhiteOut() const { return 1.0f; }
   
   // Not implemented here, but should return the Camera to world transformation matrix
   virtual void getCameraTransform (F32 *pos, MatrixF *mat ) { *mat = MatrixF::Identity; }

   /// Returns the water object we are colliding with, it is up to derived
   /// classes to actually set this object.
   virtual WaterObject* getCurrentWaterObject() { return mCurrentWaterObject; }
   
   #ifdef TORQUE_DEBUG_NET_MOVES
   bool isAIControlled() const { return mIsAiControlled; }
   #endif

   DECLARE_CONOBJECT (GameBase );

   /// @name Callbacks
   /// @{
   DECLARE_CALLBACK( void, setControl, ( bool controlled ) );
   /// @}

private:

   /// This is called by the reload signal in our datablock when it is 
   /// modified in the editor.
   ///
   /// This method is private and is not virtual. To handle a datablock-modified
   /// even in a child-class specific way you should override onNewDatablock
   /// and handle the reload( true ) case.   
   ///
   /// Warning: For local-client, editor situations only.
   ///
   /// Warning: Do not attempt to call .remove or .notify on mDataBlock->mReloadSignal
   /// within this callback.
   ///   
   void _onDatablockModified();
};


#endif // _GAMEBASE_H_
