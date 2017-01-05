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
#include "T3D/gameBase/gameBase.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "console/consoleInternal.h"
#include "core/stream/bitStream.h"
#include "sim/netConnection.h"
#include "T3D/gameBase/gameConnection.h"
#include "math/mathIO.h"
#include "T3D/gameBase/moveManager.h"
#include "T3D/gameBase/gameProcess.h"

#ifdef TORQUE_DEBUG_NET_MOVES
#include "T3D/aiConnection.h"
#endif

//----------------------------------------------------------------------------
// Ghost update relative priority values

static F32 sUpFov       = 1.0;
static F32 sUpDistance  = 0.4f;
static F32 sUpVelocity  = 0.4f;
static F32 sUpSkips     = 0.2f;
static F32 sUpInterest  = 0.2f;

//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(GameBaseData);

ConsoleDocClass( GameBaseData,
   "@brief Scriptable, demo-able datablock.  Used by GameBase objects.\n\n"
   "@see GameBase\n"
   "@ingroup gameObjects\n"
);

IMPLEMENT_CALLBACK( GameBaseData, onAdd, void, ( GameBase* obj ), ( obj ),
   "@brief Called when the object is added to the scene.\n\n"

   "@param obj the GameBase object\n\n"

   "@tsexample\n"
      "datablock GameBaseData(MyObjectData)\n"
      "{\n"
      "   category = \"Misc\";\n"
      "};\n\n"
      "function MyObjectData::onAdd( %this, %obj )\n"
      "{\n"
      "   echo( \"Added \" @ %obj.getName() @ \" to the scene.\" );\n"
      "}\n\n"
      "function MyObjectData::onNewDataBlock( %this, %obj )\n"
      "{\n"
      "   echo( \"Assign \" @ %this.getName() @ \" datablock to \" %obj.getName() );\n"
      "}\n\n"
      "function MyObjectData::onRemove( %this, %obj )\n"
      "{\n"
      "   echo( \"Removed \" @ %obj.getName() @ \" to the scene.\" );\n"
      "}\n\n"
      "function MyObjectData::onMount( %this, %obj, %mountObj, %node )\n"
      "{\n"
      "   echo( %obj.getName() @ \" mounted to \" @ %mountObj.getName() );\n"
      "}\n\n"
      "function MyObjectData::onUnmount( %this, %obj, %mountObj, %node )\n"
      "{\n"
      "   echo( %obj.getName() @ \" unmounted from \" @ %mountObj.getName() );\n"
      "}\n\n"
   "@endtsexample\n" );

IMPLEMENT_CALLBACK( GameBaseData, onNewDataBlock, void, ( GameBase* obj ), ( obj ),
   "@brief Called when the object has a new datablock assigned.\n\n"
   "@param obj the GameBase object\n\n"
   "@see onAdd for an example\n" );

IMPLEMENT_CALLBACK( GameBaseData, onRemove, void, ( GameBase* obj ), ( obj ),
   "@brief Called when the object is removed from the scene.\n\n"
   "@param obj the GameBase object\n\n"
   "@see onAdd for an example\n" );

IMPLEMENT_CALLBACK( GameBaseData, onMount, void, ( SceneObject* obj, SceneObject* mountObj, S32 node ), ( obj, mountObj, node ),
   "@brief Called when the object is mounted to another object in the scene.\n\n"
   "@param obj the GameBase object being mounted\n"
   "@param mountObj the object we are mounted to\n"
   "@param node the mountObj node we are mounted to\n\n"
   "@see onAdd for an example\n" );

IMPLEMENT_CALLBACK( GameBaseData, onUnmount, void, ( SceneObject* obj, SceneObject* mountObj, S32 node ), ( obj, mountObj, node ),
   "@brief Called when the object is unmounted from another object in the scene.\n\n"
   "@param obj the GameBase object being unmounted\n"
   "@param mountObj the object we are unmounted from\n"
   "@param node the mountObj node we are unmounted from\n\n"
   "@see onAdd for an example\n" );

IMPLEMENT_CALLBACK( GameBase, setControl, void, ( bool controlled ), ( controlled ),
   "@brief Called when the client controlling the object changes.\n\n"
   "@param controlled true if a client now controls this object, false if no "
   "client controls this object.\n" );


GameBaseData::GameBaseData()
{
   category = "";
   packed = false;
}

void GameBaseData::inspectPostApply()
{
   Parent::inspectPostApply();

   // Tell interested parties ( like objects referencing this datablock )
   // that we have been modified and they might want to rebuild...
   mReloadSignal.trigger();
}

bool GameBaseData::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

void GameBaseData::initPersistFields()
{
   addGroup("Scripting");

      addField( "category", TypeCaseString, Offset( category, GameBaseData ),
         "The group that this datablock will show up in under the \"Scripted\" "
         "tab in the World Editor Library." );

   endGroup("Scripting");

   Parent::initPersistFields();
}

bool GameBaseData::preload(bool server, String &errorStr)
{
   if (!Parent::preload(server, errorStr))
      return false;
   packed = false;
   return true;
}

void GameBaseData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   packed = true;
}

//----------------------------------------------------------------------------
bool UNPACK_DB_ID(BitStream * stream, U32 & id)
{
   if (stream->readFlag())
   {
      id = stream->readRangedU32(DataBlockObjectIdFirst,DataBlockObjectIdLast);
      return true;
   }
   return false;
}

bool PACK_DB_ID(BitStream * stream, U32 id)
{
   if (stream->writeFlag(id))
   {
      stream->writeRangedU32(id,DataBlockObjectIdFirst,DataBlockObjectIdLast);
      return true;
   }
   return false;
}

bool PRELOAD_DB(U32 & id, SimDataBlock ** data, bool server, const char * clientMissing, const char * serverMissing)
{
   if (server)
   {
      if (*data)
         id = (*data)->getId();
      else if (server && serverMissing)
      {
         Con::errorf(ConsoleLogEntry::General,serverMissing);
         return false;
      }
   }
   else
   {
      if (id && !Sim::findObject(id,*data) && clientMissing)
      {
         Con::errorf(ConsoleLogEntry::General,clientMissing);
         return false;
      }
   }
   return true;
}
//----------------------------------------------------------------------------

bool GameBase::gShowBoundingBox = false;

//----------------------------------------------------------------------------
IMPLEMENT_CO_NETOBJECT_V1(GameBase);

ConsoleDocClass( GameBase,
   "@brief Base class for game objects which use datablocks, networking, are "
   "editable, and need to process ticks.\n\n"
   "@ingroup gameObjects\n"
);

GameBase::GameBase()
: mDataBlock( NULL ),  
  mControllingClient( NULL ),
  mCurrentWaterObject( NULL )
{
   mNetFlags.set(Ghostable);
   mTypeMask |= GameBaseObjectType;
   mProcessTag = 0;

   // From ProcessObject   
   mIsGameBase = true;
   
#ifdef TORQUE_DEBUG_NET_MOVES
   mLastMoveId = 0;
   mTicksSinceLastMove = 0;
   mIsAiControlled = false;
#endif
}

GameBase::~GameBase()
{
}


//----------------------------------------------------------------------------

bool GameBase::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   // Datablock must be initialized on the server.
   // Client datablock are initialized by the initial update.
   if ( isServerObject() && mDataBlock && !onNewDataBlock( mDataBlock, false ) )
      return false;

   setProcessTick( true );

   return true;
}

void GameBase::onRemove()
{
   // EDITOR FEATURE: Remove us from the reload signal of our datablock.
   if ( mDataBlock )
      mDataBlock->mReloadSignal.remove( this, &GameBase::_onDatablockModified );

   Parent::onRemove();
}

bool GameBase::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   // EDITOR FEATURE: Remove us from old datablock's reload signal and
   // add us to the new one.
   if ( !reload )
   {
      if ( mDataBlock )
         mDataBlock->mReloadSignal.remove( this, &GameBase::_onDatablockModified );
      if ( dptr )
         dptr->mReloadSignal.notify( this, &GameBase::_onDatablockModified );   
   }
   

   mDataBlock = dptr;

   if ( !mDataBlock )
      return false;

   setMaskBits(DataBlockMask);
   return true;
}

void GameBase::_onDatablockModified()
{   
   AssertFatal( mDataBlock, "GameBase::onDatablockModified - mDataBlock is NULL." );
   onNewDataBlock( mDataBlock, true );
}

void GameBase::inspectPostApply()
{
   Parent::inspectPostApply();
   setMaskBits(ExtendedInfoMask);
}

//----------------------------------------------------------------------------

void GameBase::processTick(const Move * move)
{
#ifdef TORQUE_DEBUG_NET_MOVES
   if (!move)
      mTicksSinceLastMove++;

   const char * srv = isClientObject() ? "client" : "server";
   const char * who = "";
   if (isClientObject())
   {
      if (this == (GameBase*)GameConnection::getConnectionToServer()->getControlObject())
         who = " player";
      else
         who = " ghost";
      if (mIsAiControlled)
         who = " ai";
   }
   if (isServerObject())
   {
      if (dynamic_cast<AIConnection*>(getControllingClient()))
      {
         who = " ai";
         mIsAiControlled = true;
      }
      else if (getControllingClient())
      {
         who = " player";
         mIsAiControlled = false;
      }
      else
      {
         who = "";
         mIsAiControlled = false;
      }
   }
   U32 moveid = mLastMoveId+mTicksSinceLastMove;
   if (move)
      moveid = move->id;

   if (getTypeMask() & GameBaseHiFiObjectType)
   {
      if (move)
         Con::printf("Processing (%s%s id %i) move %i",srv,who,getId(), move->id);
      else
         Con::printf("Processing (%s%s id %i) move %i (%i)",srv,who,getId(),mLastMoveId+mTicksSinceLastMove,mTicksSinceLastMove);
   }

   if (move)
   {
      mLastMoveId = move->id;
      mTicksSinceLastMove=0;
   }
#endif
}

//----------------------------------------------------------------------------

F32 GameBase::getUpdatePriority(CameraScopeQuery *camInfo, U32 updateMask, S32 updateSkips)
{
   TORQUE_UNUSED(updateMask);

   // Calculate a priority used to decide if this object
   // will be updated on the client.  All the weights
   // are calculated 0 -> 1  Then weighted together at the
   // end to produce a priority.
   Point3F pos;
   getWorldBox().getCenter(&pos);
   pos -= camInfo->pos;
   F32 dist = pos.len();
   if (dist == 0.0f) dist = 0.001f;
   pos *= 1.0f / dist;

   // Weight based on linear distance, the basic stuff.
   F32 wDistance = (dist < camInfo->visibleDistance)?
      1.0f - (dist / camInfo->visibleDistance): 0.0f;

   // Weight by field of view, objects directly in front
   // will be weighted 1, objects behind will be 0
   F32 dot = mDot(pos,camInfo->orientation);

   bool inFov = dot > camInfo->cosFov * 1.5f;

   F32 wFov = inFov? 1.0f: 0;

   // Weight by linear velocity parallel to the viewing plane
   // (if it's the field of view, 0 if it's not).
   F32 wVelocity = 0.0f;
   if (inFov)
   {
      Point3F vec;
      mCross(camInfo->orientation,getVelocity(),&vec);
      wVelocity = (vec.len() * camInfo->fov) /
         (camInfo->fov * camInfo->visibleDistance);
      if (wVelocity > 1.0f)
         wVelocity = 1.0f;
   }

   // Weight by interest.
   F32 wInterest;
   if (getTypeMask() & (PlayerObjectType || VehicleObjectType ))
      wInterest = 0.75f;
   else if (getTypeMask() & ProjectileObjectType)
   {
      // Projectiles are more interesting if they
      // are heading for us.
      wInterest = 0.30f;
      F32 dot = -mDot(pos,getVelocity());
      if (dot > 0.0f)
         wInterest += 0.20 * dot;
   }
   else
   {
      if (getTypeMask() & ItemObjectType)
         wInterest = 0.25f;
      else
         // Everything else is less interesting.
         wInterest = 0.0f;
   }

   // Weight by updateSkips
   F32 wSkips = updateSkips * 0.5;

   // Calculate final priority, should total to about 1.0f
   //
   return
      wFov       * sUpFov +
      wDistance  * sUpDistance +
      wVelocity  * sUpVelocity +
      wSkips     * sUpSkips +
      wInterest  * sUpInterest;
}

//----------------------------------------------------------------------------
bool GameBase::setDataBlock(GameBaseData* dptr)
{
   if (isGhost() || isProperlyAdded()) {
      if (mDataBlock != dptr)
         return onNewDataBlock(dptr,false);
   }
   else
      mDataBlock = dptr;
   return true;
}


//--------------------------------------------------------------------------
void GameBase::scriptOnAdd()
{
   // Script onAdd() must be called by the leaf class after
   // everything is ready.
   if (mDataBlock && !isGhost())
      mDataBlock->onAdd_callback( this );
}

void GameBase::scriptOnNewDataBlock()
{
   // Script onNewDataBlock() must be called by the leaf class
   // after everything is loaded.
   if (mDataBlock && !isGhost())
      mDataBlock->onNewDataBlock_callback( this );
}

void GameBase::scriptOnRemove()
{
   // Script onRemove() must be called by leaf class while
   // the object state is still valid.
   if (!isGhost() && mDataBlock)
      mDataBlock->onRemove_callback( this );
}

//----------------------------------------------------------------------------

void GameBase::setControllingClient(GameConnection* client)
{
   if (isClientObject())
   {
      if (mControllingClient)
         setControl_callback( 0 );
      if (client)
         setControl_callback( 1 );
   }

   mControllingClient = client;
}

U32 GameBase::getPacketDataChecksum(GameConnection * connection)
{
   // just write the packet data into a buffer
   // then we can CRC the buffer.  This should always let us
   // know when there is a checksum problem.

   static U8 buffer[1500] = { 0, };
   BitStream stream(buffer, sizeof(buffer));

   writePacketData(connection, &stream);
   U32 byteCount = stream.getPosition();
   U32 ret = CRC::calculateCRC(buffer, byteCount, 0xFFFFFFFF);
   dMemset(buffer, 0, byteCount);
   return ret;
}

void GameBase::writePacketData(GameConnection*, BitStream*)
{
}

void GameBase::readPacketData(GameConnection*, BitStream*)
{
}

U32 GameBase::packUpdate( NetConnection *connection, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( connection, mask, stream );
       
   if ( stream->writeFlag( mask & ScaleMask ) )  
   {
      // Only write one bit if the scale is one.
      if ( stream->writeFlag( mObjScale != Point3F::One ) )
         mathWrite( *stream, mObjScale );   
   }

   if ( stream->writeFlag( ( mask & DataBlockMask ) && mDataBlock != NULL ) ) 
   {
      stream->writeRangedU32( mDataBlock->getId(),
                              DataBlockObjectIdFirst,
                              DataBlockObjectIdLast );
      if ( stream->writeFlag( mNetFlags.test( NetOrdered ) ) )
         stream->writeInt( mOrderGUID, 16 );
   }

#ifdef TORQUE_DEBUG_NET_MOVES
   stream->write(mLastMoveId);
   stream->writeFlag(mIsAiControlled);
#endif

   return retMask;
}

void GameBase::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate( con, stream );
   
   // ScaleMask
   if ( stream->readFlag() ) 
   {
      if ( stream->readFlag() )
      {
         VectorF scale;
         mathRead( *stream, &scale );
         setScale( scale );
      }
      else
         setScale( Point3F::One );
   }

   // DataBlockMask
   if ( stream->readFlag() )
   {
      GameBaseData *dptr = 0;
      SimObjectId id = stream->readRangedU32( DataBlockObjectIdFirst,
                                              DataBlockObjectIdLast );
      if ( stream->readFlag() )
         mOrderGUID = stream->readInt( 16 );

      if ( !Sim::findObject( id, dptr ) || !setDataBlock( dptr ) )
         con->setLastError( "Invalid packet GameBase::unpackUpdate()" );
   }

#ifdef TORQUE_DEBUG_NET_MOVES
   stream->read(&mLastMoveId);
   mTicksSinceLastMove = 0;
   mIsAiControlled = stream->readFlag();
#endif
}

void GameBase::onMount( SceneObject *obj, S32 node )
{      
   deleteNotify( obj );

   // Are we mounting to a GameBase object?
   GameBase *gbaseObj = dynamic_cast<GameBase*>( obj );

   if ( gbaseObj && gbaseObj->getControlObject() != this && gbaseObj->getControllingObject() != this)
      processAfter( gbaseObj );

   if (!isGhost()) {
      setMaskBits(MountedMask);
      mDataBlock->onMount_callback( this, obj, node );
   }
}

void GameBase::onUnmount( SceneObject *obj, S32 node )
{
   clearNotify(obj);

   GameBase *gbaseObj = dynamic_cast<GameBase*>( obj );

   if ( gbaseObj && gbaseObj->getControlObject() != this )
      clearProcessAfter();

   if (!isGhost()) {
      setMaskBits(MountedMask);
      mDataBlock->onUnmount_callback( this, obj, node );
   }
}

bool GameBase::setDataBlockProperty( void *obj, const char *index, const char *db)
{
   if( db == NULL || !db[ 0 ] )
   {
      Con::errorf( "GameBase::setDataBlockProperty - Can't unset datablock on GameBase objects" );
      return false;
   }
   
   GameBase* object = static_cast< GameBase* >( obj );
   GameBaseData* data;
   if( Sim::findObject( db, data ) )
      return object->setDataBlock( data );
   
   Con::errorf( "GameBase::setDatablockProperty - Could not find data block \"%s\"", db );
   return false;
}

MoveList* GameBase::getMoveList()
{ 
   return mControllingClient ? mControllingClient->mMoveList : NULL; 
}

//----------------------------------------------------------------------------
DefineEngineMethod( GameBase, getDataBlock, S32, (),,
   "@brief Get the datablock used by this object.\n\n"
   "@return the datablock this GameBase is using."
   "@see setDataBlock()\n")
{
   return object->getDataBlock()? object->getDataBlock()->getId(): 0;
}

//----------------------------------------------------------------------------
DefineEngineMethod( GameBase, setDataBlock, bool, ( GameBaseData* data ),,
   "@brief Assign this GameBase to use the specified datablock.\n\n"
   "@param data new datablock to use\n"
   "@return true if successful, false if failed."
   "@see getDataBlock()\n")
{
   return ( data && object->setDataBlock(data) );
}

//----------------------------------------------------------------------------

void GameBase::initPersistFields()
{
   addGroup( "Game" );

      addProtectedField( "dataBlock", TYPEID< GameBaseData >(), Offset(mDataBlock, GameBase),
         &setDataBlockProperty, &defaultProtectedGetFn, 
         "Script datablock used for game objects." );

   endGroup( "Game" );

   Parent::initPersistFields();
}

void GameBase::consoleInit()
{
#ifdef TORQUE_DEBUG
   Con::addVariable( "GameBase::boundingBox", TypeBool, &gShowBoundingBox,
      "@brief Toggles on the rendering of the bounding boxes for certain types of objects in scene.\n\n"
      "@ingroup GameBase" );
#endif
}

DefineEngineMethod( GameBase, applyImpulse, bool, ( Point3F pos, VectorF vel ),,
   "@brief Apply an impulse to this object as defined by a world position and velocity vector.\n\n"

   "@param pos impulse world position\n"
   "@param vel impulse velocity (impulse force F = m * v)\n"
   "@return Always true\n"

   "@note Not all objects that derrive from GameBase have this defined.\n")
{
   object->applyImpulse(pos,vel);
   return true;
}

DefineEngineMethod( GameBase, applyRadialImpulse, void, ( Point3F origin, F32 radius, F32 magnitude ),,
   "@brief Applies a radial impulse to the object using the given origin and force.\n\n"

   "@param origin World point of origin of the radial impulse.\n"
   "@param radius The radius of the impulse area.\n"
   "@param magnitude The strength of the impulse.\n"
   
   "@note Not all objects that derrive from GameBase have this defined.\n")
{
   object->applyRadialImpulse( origin, radius, magnitude );
}
