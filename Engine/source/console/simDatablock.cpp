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
#include "console/simDatablock.h"

#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "T3D/gameBase/gameConnectionEvents.h"
#include "T3D/gameBase/gameConnection.h"


IMPLEMENT_CO_DATABLOCK_V1(SimDataBlock);
SimObjectId SimDataBlock::sNextObjectId = DataBlockObjectIdFirst;
S32 SimDataBlock::sNextModifiedKey = 0;

ConsoleDocClass( SimDataBlock,
   "@brief \n"
   "@ingroup \n"
   
   "@section Datablock_Networking Datablocks and Networking\n"
   
   "@section Datablock_ClientSide Client-Side Datablocks\n"
);



//-----------------------------------------------------------------------------

SimDataBlock::SimDataBlock()
{
   setModDynamicFields(true);
   setModStaticFields(true);
}

//-----------------------------------------------------------------------------

bool SimDataBlock::onAdd()
{
   Parent::onAdd();

   // This initialization is done here, and not in the constructor,
   // because some jokers like to construct and destruct objects
   // (without adding them to the manager) to check what class
   // they are.
   modifiedKey = ++sNextModifiedKey;
   AssertFatal(sNextObjectId <= DataBlockObjectIdLast,
      "Exceeded maximum number of data blocks");

   // add DataBlock to the DataBlockGroup unless it is client side ONLY DataBlock
   if ( !isClientOnly() )
      if (SimGroup* grp = Sim::getDataBlockGroup())
         grp->addObject(this);
         
   Sim::getDataBlockSet()->addObject( this );

   return true;
}

//-----------------------------------------------------------------------------

void SimDataBlock::assignId()
{
   // We don't want the id assigned by the manager, but it may have
   // already been assigned a correct data block id.
   if ( isClientOnly() )
      setId(sNextObjectId++);
}

//-----------------------------------------------------------------------------

void SimDataBlock::onStaticModified(const char* slotName, const char* newValue)
{
   modifiedKey = sNextModifiedKey++;
}

//-----------------------------------------------------------------------------

void SimDataBlock::packData(BitStream*)
{
}

//-----------------------------------------------------------------------------

void SimDataBlock::unpackData(BitStream*)
{
}

//-----------------------------------------------------------------------------

bool SimDataBlock::preload(bool, String&)
{
   return true;
}

//-----------------------------------------------------------------------------

void SimDataBlock::write(Stream &stream, U32 tabStop, U32 flags)
{
   // Only output selected objects if they want that.
   if((flags & SelectedOnly) && !isSelected())
      return;

   stream.writeTabs(tabStop);
   char buffer[1024];

   // Client side datablocks are created with 'new' while
   // regular server datablocks use the 'datablock' keyword.
   if ( isClientOnly() )
      dSprintf(buffer, sizeof(buffer), "new %s(%s) {\r\n", getClassName(), getName() ? getName() : "");
   else
      dSprintf(buffer, sizeof(buffer), "datablock %s(%s) {\r\n", getClassName(), getName() ? getName() : "");

   stream.write(dStrlen(buffer), buffer);
   writeFields(stream, tabStop + 1);

   stream.writeTabs(tabStop);
   stream.write(4, "};\r\n");
}

//=============================================================================
//    API.
//=============================================================================
// MARK: ---- API ----

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimDataBlock, reloadOnLocalClient, void, (),,
   "Reload the datablock.  This can only be used with a local client configuration." )
{
   // Make sure we're running a local client.

   GameConnection* localClient = GameConnection::getLocalClientConnection();
   if( !localClient )
      return;

   // Do an in-place pack/unpack/preload.

   if( !object->preload( true, NetConnection::getErrorBuffer() ) )
   {
      Con::errorf( NetConnection::getErrorBuffer() );
      return;
   }

   U8 buffer[ 16384 ];
   BitStream stream( buffer, 16384 );

   object->packData( &stream );
   stream.setPosition(0);
   object->unpackData( &stream );

   if( !object->preload( false, NetConnection::getErrorBuffer() ) )
   {
      Con::errorf( NetConnection::getErrorBuffer() );
      return;
   }

   // Trigger a post-apply so that change notifications respond.
   object->inspectPostApply();
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( preloadClientDataBlocks, void, (),,
   "Preload all datablocks in client mode.\n\n"
   "(Server parameter is set to false).  This will take some time to complete.")
{
   // we go from last to first because we cut 'n pasted the loop from deleteDataBlocks
   SimGroup *grp = Sim::getDataBlockGroup();
   String errorStr;
   for(S32 i = grp->size() - 1; i >= 0; i--)
   {
      AssertFatal(dynamic_cast<SimDataBlock*>((*grp)[i]), "Doh! non-datablock in datablock group!");
      SimDataBlock *obj = (SimDataBlock*)(*grp)[i];
      if (!obj->preload(false, errorStr))
         Con::errorf("Failed to preload client datablock, %s: %s", obj->getName(), errorStr.c_str());
   }
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( deleteDataBlocks, void, (),,
   "Delete all the datablocks we've downloaded.\n\n"
   "This is usually done in preparation of downloading a new set of datablocks, "
   "such as occurs on a mission change, but it's also good post-mission cleanup." )
{
   // delete from last to first:
   SimGroup *grp = Sim::getDataBlockGroup();
   grp->deleteAllObjects();
   SimDataBlock::sNextObjectId = DataBlockObjectIdFirst;
   SimDataBlock::sNextModifiedKey = 0;
}
