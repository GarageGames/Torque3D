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
#include "console/simBase.h"
#include "core/dnet.h"
#include "sim/netConnection.h"
#include "sim/netObject.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(NetObject);

// More information can be found in the Torque Manual (CHM)
ConsoleDocClass( NetObject,
				"@brief Superclass for all ghostable networked objects.\n\n"
				"@ingroup Networking\n");

//----------------------------------------------------------------------------
NetObject *NetObject::mDirtyList = NULL;

NetObject::NetObject()
{
	// netFlags will clear itself to 0
	mNetIndex = U32(-1);
   mFirstObjectRef = NULL;
   mPrevDirtyList = NULL;
   mNextDirtyList = NULL;
   mDirtyMaskBits = 0;
}

NetObject::~NetObject()
{
   if(mDirtyMaskBits)
   {
      if(mPrevDirtyList)
         mPrevDirtyList->mNextDirtyList = mNextDirtyList;
      else
         mDirtyList = mNextDirtyList;
      if(mNextDirtyList)
         mNextDirtyList->mPrevDirtyList = mPrevDirtyList;
   }
}

String NetObject::describeSelf() const
{
   String desc = Parent::describeSelf();

   if( isClientObject() )
      desc += "|net: client";
   else
      desc += "|net: server";

   return desc;
}

void NetObject::setMaskBits(U32 orMask)
{
   AssertFatal(orMask != 0, "Invalid net mask bits set.");
   AssertFatal(mDirtyMaskBits == 0 || (mPrevDirtyList != NULL || mNextDirtyList != NULL || mDirtyList == this), "Invalid dirty list state.");
   if(!mDirtyMaskBits)
   {
      AssertFatal(mNextDirtyList == NULL && mPrevDirtyList == NULL, "Object with zero mask already in list.");
      if(mDirtyList)
      {
         mNextDirtyList = mDirtyList;
         mDirtyList->mPrevDirtyList = this;
      }
      mDirtyList = this;
   }
   mDirtyMaskBits |= orMask;
   AssertFatal(mDirtyMaskBits == 0 || (mPrevDirtyList != NULL || mNextDirtyList != NULL || mDirtyList == this), "Invalid dirty list state.");
}

void NetObject::clearMaskBits(U32 orMask)
{
   if(isDeleted())
      return;
   if(mDirtyMaskBits)
   {
      mDirtyMaskBits &= ~orMask;
      if(!mDirtyMaskBits)
      {
         if(mPrevDirtyList)
            mPrevDirtyList->mNextDirtyList = mNextDirtyList;
         else
            mDirtyList = mNextDirtyList;
         if(mNextDirtyList)
            mNextDirtyList->mPrevDirtyList = mPrevDirtyList;
         mNextDirtyList = mPrevDirtyList = NULL;
      }
   }

   for(GhostInfo *walk = mFirstObjectRef; walk; walk = walk->nextObjectRef)
   {
      if(walk->updateMask && walk->updateMask == orMask)
      {
         walk->updateMask = 0;
         walk->connection->ghostPushToZero(walk);
      }
      else
         walk->updateMask &= ~orMask;
   }
}

void NetObject::collapseDirtyList()
{
#ifdef TORQUE_DEBUG
   Vector<NetObject *> tempV;
   for(NetObject *t = mDirtyList; t; t = t->mNextDirtyList)
      tempV.push_back(t);
#endif

   for(NetObject *obj = mDirtyList; obj; )
   {
      NetObject *next = obj->mNextDirtyList;
      U32 dirtyMask = obj->mDirtyMaskBits;

      obj->mNextDirtyList = NULL;
      obj->mPrevDirtyList = NULL;
      obj->mDirtyMaskBits = 0;

      if(!obj->isDeleted() && dirtyMask)
      {
         for(GhostInfo *walk = obj->mFirstObjectRef; walk; walk = walk->nextObjectRef)
         {
            U32 orMask = obj->filterMaskBits(dirtyMask,walk->connection);
            if(!walk->updateMask && orMask)
            {
               walk->updateMask = orMask;
               walk->connection->ghostPushNonZero(walk);
            }
            else
               walk->updateMask |= orMask;
         }
      }
      obj = next;
   }
   mDirtyList = NULL;
#ifdef TORQUE_DEBUG
   for(U32 i = 0; i < tempV.size(); i++)
   {
      AssertFatal(tempV[i]->mNextDirtyList == NULL && tempV[i]->mPrevDirtyList == NULL && tempV[i]->mDirtyMaskBits == 0, "Error in collapse");
   }
#endif
}

//-----------------------------------------------------------------------------
DefineEngineMethod( NetObject, scopeToClient, void, ( NetConnection* client),,
   "@brief Cause the NetObject to be forced as scoped on the specified NetConnection.\n\n"

   "@param client The connection this object will always be scoped to\n\n"

   "@tsexample\n"
      "// Called to create new cameras in TorqueScript\n"
      "// %this - The active GameConnection\n"
      "// %spawnPoint - The spawn point location where we creat the camera\n"
      "function GameConnection::spawnCamera(%this, %spawnPoint)\n"
      "{\n"
      "	// If this connection's camera exists\n"
      "	if(isObject(%this.camera))\n"
      "	{\n"
      "		// Add it to the mission group to be cleaned up later\n"
      "		MissionCleanup.add( %this.camera );\n\n"
      "		// Force it to scope to the client side\n"
      "		%this.camera.scopeToClient(%this);\n"
      "	}\n"
      "}\n"
   "@endtsexample\n\n"
   
   "@see clearScopeToClient()\n")
{
	if(!client)
	{
		Con::errorf(ConsoleLogEntry::General, "NetObject::scopeToClient: Couldn't find connection %s", client);
		return;
	}
	client->objectLocalScopeAlways(object);
}

//ConsoleMethod(NetObject,scopeToClient,void,3,3,"(NetConnection %client)"
//              "Cause the NetObject to be forced as scoped on the specified NetConnection.")
//{
//   TORQUE_UNUSED(argc);
//   NetConnection *conn;
//   if(!Sim::findObject(argv[2], conn))
//   {
//      Con::errorf(ConsoleLogEntry::General, "NetObject::scopeToClient: Couldn't find connection %s", argv[2]);
//      return;
//   }
//   conn->objectLocalScopeAlways(object);
//}

DefineEngineMethod( NetObject, clearScopeToClient, void, ( NetConnection* client),,
   "@brief Undo the effects of a scopeToClient() call.\n\n"

   "@param client The connection to remove this object's scoping from \n\n"
   
   "@see scopeToClient()\n")
{
   if(!client)
   {
      Con::errorf(ConsoleLogEntry::General, "NetObject::clearScopeToClient: Couldn't find connection %s", client);
      return;
   }
   client->objectLocalClearAlways(object);
}

//ConsoleMethod(NetObject,clearScopeToClient,void,3,3,"clearScopeToClient(%client)"
//              "Undo the effects of a scopeToClient() call.")
//{
//   TORQUE_UNUSED(argc);
//   NetConnection *conn;
//   if(!Sim::findObject(argv[2], conn))
//   {
//      Con::errorf(ConsoleLogEntry::General, "NetObject::clearScopeToClient: Couldn't find connection %s", argv[2]);
//      return;
//   }
//   conn->objectLocalClearAlways(object);
//}

DefineEngineMethod( NetObject, setScopeAlways, void, (),,
   "@brief Always scope this object on all connections.\n\n"

   "The object is marked as ScopeAlways and is immediately ghosted to "
   "all active connections.  This function has no effect if the object "
   "is not marked as Ghostable.\n\n")
{
	object->setScopeAlways();
}

//ConsoleMethod(NetObject,setScopeAlways,void,2,2,"Always scope this object on all connections.")
//{
//   TORQUE_UNUSED(argc); TORQUE_UNUSED(argv);
//   object->setScopeAlways();
//}

void NetObject::setScopeAlways()
{
   if(mNetFlags.test(Ghostable) && !mNetFlags.test(IsGhost))
   {
      mNetFlags.set(ScopeAlways);

      // if it's a ghost always object, add it to the ghost always set
      // for ClientReps created later.

      Sim::getGhostAlwaysSet()->addObject(this);

      // add it to all Connections that already exist.

      SimGroup *clientGroup = Sim::getClientGroup();
      SimGroup::iterator i;
      for(i = clientGroup->begin(); i != clientGroup->end(); i++)
      {
         NetConnection *con = (NetConnection *) (*i);
         if(con->isGhosting())
            con->objectInScope(this);
      }
   }
}

void NetObject::clearScopeAlways()
{
   if(!mNetFlags.test(IsGhost))
   {
      mNetFlags.clear(ScopeAlways);
      Sim::getGhostAlwaysSet()->removeObject(this);

      // Un ghost this object from all the connections
      while(mFirstObjectRef)
         mFirstObjectRef->connection->detachObject(mFirstObjectRef);
   }
}

bool NetObject::onAdd()
{
   if(mNetFlags.test(ScopeAlways))
      setScopeAlways();

   return Parent::onAdd();
}

void NetObject::onRemove()
{
   while(mFirstObjectRef)
      mFirstObjectRef->connection->detachObject(mFirstObjectRef);

   Parent::onRemove();
}

//-----------------------------------------------------------------------------

F32 NetObject::getUpdatePriority(CameraScopeQuery*, U32, S32 updateSkips)
{
   return F32(updateSkips) * 0.1;
}

U32 NetObject::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
{
   return 0;
}

void NetObject::unpackUpdate(NetConnection*, BitStream*)
{
}

void NetObject::onCameraScopeQuery(NetConnection *cr, CameraScopeQuery* /*camInfo*/)
{
   // default behavior -
   // ghost everything that is ghostable

   for (SimSetIterator obj(Sim::getRootGroup()); *obj; ++obj)
   {
		NetObject* nobj = dynamic_cast<NetObject*>(*obj);
		if (nobj)
		{
			AssertFatal(!nobj->mNetFlags.test(NetObject::Ghostable) || !nobj->mNetFlags.test(NetObject::IsGhost),
			   "NetObject::onCameraScopeQuery: object marked both ghostable and as ghost");

			// Some objects don't ever want to be ghosted
			if (!nobj->mNetFlags.test(NetObject::Ghostable))
				continue;
         if (!nobj->mNetFlags.test(NetObject::ScopeAlways))
         {
            // it's in scope...
            cr->objectInScope(nobj);
         }
      }
   }
}

//-----------------------------------------------------------------------------

void NetObject::initPersistFields()
{
   Parent::initPersistFields();
}

DefineEngineMethod( NetObject, getGhostID, S32, (),,
   "@brief Get the ghost index of this object from the server.\n\n"

   "@returns The ghost ID of this NetObject on the server\n"

   "@tsexample\n"
      "%ghostID = LocalClientConnection.getGhostId( %serverObject );\n"
   "@endtsexample\n\n")
{
	return object->getNetIndex();
}

//ConsoleMethod( NetObject, getGhostID, S32, 2, 2, "")
//{
//   return object->getNetIndex();
//}

DefineEngineMethod( NetObject, getClientObject, S32, (),,
   "@brief Returns a pointer to the client object when on a local connection.\n\n"

   "Short-Circuit-Networking: this is only valid for a local-client / singleplayer situation.\n\n"

   "@returns the SimObject ID of the client object.\n"

   "@tsexample\n"
      "// Psuedo-code, some values left out for this example\n"
      "%node = new ParticleEmitterNode(){};\n"
      "%clientObject = %node.getClientObject();\n"
      "if(isObject(%clientObject)\n"
      "	%clientObject.setTransform(\"0 0 0\");\n\n"
   "@endtsexample\n\n"
   
   "@see @ref local_connections")
{
	NetObject *obj = object->getClientObject();
	if ( obj )
		return obj->getId();
	
	return NULL;
}

//ConsoleMethod( NetObject, getClientObject, S32, 2, 2, "Short-Circuit-Netorking: this is only valid for a local-client / singleplayer situation." )
//{
//   NetObject *obj = object->getClientObject();
//   if ( obj )
//      return obj->getId();
//
//   return NULL;
//}

DefineEngineMethod( NetObject, getServerObject, S32, (),,
   "@brief Returns a pointer to the client object when on a local connection.\n\n"

   "Short-Circuit-Netorking: this is only valid for a local-client / singleplayer situation.\n\n"
   
   "@returns The SimObject ID of the server object.\n"
   "@tsexample\n"
      "// Psuedo-code, some values left out for this example\n"
      "%node = new ParticleEmitterNode(){};\n"
      "%serverObject = %node.getServerObject();\n"
      "if(isObject(%serverObject)\n"
      "	%serverObject.setTransform(\"0 0 0\");\n\n"
   "@endtsexample\n\n"
   
   "@see @ref local_connections")
{
	NetObject *obj = object->getServerObject();
	if ( obj )
		return obj->getId();
	
	return NULL;
}

//ConsoleMethod( NetObject, getServerObject, S32, 2, 2, "Short-Circuit-Netorking: this is only valid for a local-client / singleplayer situation." )
//{
//   NetObject *obj = object->getServerObject();
//   if ( obj )
//      return obj->getId();
//
//   return NULL;
//}

DefineEngineMethod( NetObject, isClientObject, bool, (),,
   "@brief Called to check if an object resides on the clientside.\n\n"
   "@return True if the object resides on the client, false otherwise.")
{
	return object->isClientObject();
}

//ConsoleMethod( NetObject, isClientObject, bool, 2, 2, "Return true for client-side objects." )
//{
//   return object->isClientObject();
//}

DefineEngineMethod( NetObject, isServerObject, bool, (),,
   "@brief Checks if an object resides on the server.\n\n"
   "@return True if the object resides on the server, false otherwise.")
{
	return object->isServerObject();
}

//ConsoleMethod( NetObject, isServerObject, bool, 2, 2, "Return true for client-side objects." )
//{
//   return object->isServerObject();
//}
