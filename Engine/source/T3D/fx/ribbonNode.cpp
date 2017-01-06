//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#include "ribbonNode.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "T3D/fx/ribbon.h"
#include "math/mathIO.h"
#include "sim/netConnection.h"
#include "console/engineAPI.h"

IMPLEMENT_CO_DATABLOCK_V1(RibbonNodeData);
IMPLEMENT_CO_NETOBJECT_V1(RibbonNode);

ConsoleDocClass( RibbonNodeData,
   "@brief Contains additional data to be associated with a RibbonNode."
   "@ingroup FX\n"
   );

ConsoleDocClass( RibbonNode, ""
   );

//-----------------------------------------------------------------------------
// RibbonNodeData
//-----------------------------------------------------------------------------
RibbonNodeData::RibbonNodeData()
{
}

RibbonNodeData::~RibbonNodeData()
{

}

//-----------------------------------------------------------------------------
// initPersistFields
//-----------------------------------------------------------------------------
void RibbonNodeData::initPersistFields()
{
   Parent::initPersistFields();
}


//-----------------------------------------------------------------------------
// RibbonNode
//-----------------------------------------------------------------------------
RibbonNode::RibbonNode()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);
   mTypeMask |= EnvironmentObjectType;

   mActive = true;

   mDataBlock          = NULL;
   mRibbonDatablock   = NULL;
   mRibbonDatablockId = 0;
   mRibbon            = NULL;
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
RibbonNode::~RibbonNode()
{
   //
}

//-----------------------------------------------------------------------------
// initPersistFields
//-----------------------------------------------------------------------------
void RibbonNode::initPersistFields()
{
   addField( "active", TYPEID< bool >(), Offset(mActive,RibbonNode),
      "Controls whether ribbon is emitted from this node." );
   addField( "ribbon",  TYPEID< RibbonData >(), Offset(mRibbonDatablock, RibbonNode),
      "Datablock to use for the ribbon." );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// onAdd
//-----------------------------------------------------------------------------
bool RibbonNode::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   if( !mRibbonDatablock && mRibbonDatablockId != 0 )
   {
      if( Sim::findObject(mRibbonDatablockId, mRibbonDatablock) == false )
         Con::errorf(ConsoleLogEntry::General, "RibbonNode::onAdd: Invalid packet, bad datablockId(mRibbonDatablock): %d", mRibbonDatablockId);
   }

   if( isClientObject() )
   {
      setRibbonDatablock( mRibbonDatablock );
   }
   else
   {
      setMaskBits( StateMask | EmitterDBMask );
   }

   mObjBox.minExtents.set(-0.5, -0.5, -0.5);
   mObjBox.maxExtents.set( 0.5,  0.5,  0.5);
   resetWorldBox();
   addToScene();

   return true;
}

//-----------------------------------------------------------------------------
// onRemove
//-----------------------------------------------------------------------------
void RibbonNode::onRemove()
{
   removeFromScene();
   if( isClientObject() )
   {
      if( mRibbon )
      {
         mRibbon->deleteOnEnd();
         mRibbon = NULL;
      }
   }

   Parent::onRemove();
}

//-----------------------------------------------------------------------------
// onNewDataBlock
//-----------------------------------------------------------------------------
bool RibbonNode::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   mDataBlock = dynamic_cast<RibbonNodeData*>( dptr );
   if ( !mDataBlock || !Parent::onNewDataBlock( dptr, reload ) )
      return false;

   // Todo: Uncomment if this is a "leaf" class
   scriptOnNewDataBlock();
   return true;
}

//-----------------------------------------------------------------------------
void RibbonNode::inspectPostApply()
{
   Parent::inspectPostApply();
   setMaskBits(StateMask | EmitterDBMask);
}

//-----------------------------------------------------------------------------
// processTick
//-----------------------------------------------------------------------------
void RibbonNode::processTick(const Move* move)
{
   Parent::processTick(move);

   if ( isMounted() )
   {
      MatrixF mat;
      mMount.object->getMountTransform( mMount.node, mMount.xfm, &mat );
      setTransform( mat );
   }
}

//-----------------------------------------------------------------------------
// advanceTime
//-----------------------------------------------------------------------------
void RibbonNode::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   if(!mActive || mRibbon.isNull() || !mDataBlock)
      return;

   MatrixF trans(getTransform());
   Point3F pos = getPosition();
   mRibbon->addSegmentPoint(pos, trans);
}

//-----------------------------------------------------------------------------
// packUpdate
//-----------------------------------------------------------------------------
U32 RibbonNode::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if ( stream->writeFlag( mask & InitialUpdateMask ) )
   {
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
   }

   if ( stream->writeFlag( mask & EmitterDBMask ) )
   {
      if( stream->writeFlag(mRibbonDatablock != NULL) )
      {
         stream->writeRangedU32(mRibbonDatablock->getId(), DataBlockObjectIdFirst,
            DataBlockObjectIdLast);
      }
   }

   if ( stream->writeFlag( mask & StateMask ) )
   {
      stream->writeFlag( mActive );
   }

   return retMask;
}

//-----------------------------------------------------------------------------
// unpackUpdate
//-----------------------------------------------------------------------------
void RibbonNode::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if ( stream->readFlag() )
   {
      MatrixF temp;
      Point3F tempScale;
      mathRead(*stream, &temp);
      mathRead(*stream, &tempScale);

      setScale(tempScale);
      setTransform(temp);
   }

   if ( stream->readFlag() )
   {
      mRibbonDatablockId = stream->readFlag() ?
         stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast) : 0;

      RibbonData *emitterDB = NULL;
      Sim::findObject( mRibbonDatablockId, emitterDB );
      if ( isProperlyAdded() )
         setRibbonDatablock( emitterDB );
   }

   if ( stream->readFlag() )
   {
      mActive = stream->readFlag();
   }
}

//-----------------------------------------------------------------------------
// setRibbonDatablock
//-----------------------------------------------------------------------------
void RibbonNode::setRibbonDatablock(RibbonData* data)
{
   if ( isServerObject() )
   {
      setMaskBits( EmitterDBMask );
   }
   else
   {
      Ribbon* pRibbon = NULL;
      if ( data )
      {
         // Create emitter with new datablock
         pRibbon = new Ribbon;
         pRibbon->onNewDataBlock( data, false );
         if( pRibbon->registerObject() == false )
         {
            Con::warnf(ConsoleLogEntry::General, "Could not register base ribbon of class: %s", data->getName() ? data->getName() : data->getIdString() );
            delete pRibbon;
            return;
         }
      }

      // Replace emitter
      if ( mRibbon )
         mRibbon->deleteOnEnd();

      mRibbon = pRibbon;
   }

   mRibbonDatablock = data;
}

DefineEngineMethod(RibbonNode, setRibbonDatablock, void, (RibbonData* ribbonDatablock), (nullAsType<RibbonData*>()),
   "Assigns the datablock for this ribbon node.\n"
   "@param ribbonDatablock RibbonData datablock to assign\n"
   "@tsexample\n"
   "// Assign a new emitter datablock\n"
   "%emitter.setRibbonDatablock( %ribbonDatablock );\n"
   "@endtsexample\n" )
{
   if ( !ribbonDatablock )
   {
      Con::errorf("RibbonData datablock could not be found when calling setRibbonDataBlock in ribbonNode.");
      return;
   }

   object->setRibbonDatablock(ribbonDatablock);
}

DefineEngineMethod(RibbonNode, setActive, void, (bool active),,
   "Turns the ribbon on or off.\n"
   "@param active New ribbon state\n" )
{
   object->setActive( active );
}
