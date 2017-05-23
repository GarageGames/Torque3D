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

#include "particleEmitterNode.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "T3D/fx/particleEmitter.h"
#include "math/mathIO.h"
#include "sim/netConnection.h"
#include "console/engineAPI.h"

IMPLEMENT_CO_DATABLOCK_V1(ParticleEmitterNodeData);
IMPLEMENT_CO_NETOBJECT_V1(ParticleEmitterNode);

ConsoleDocClass( ParticleEmitterNodeData,
   "@brief Contains additional data to be associated with a ParticleEmitterNode."
   "@ingroup FX\n"
);

ConsoleDocClass( ParticleEmitterNode,
   "@brief A particle emitter object that can be positioned in the world and "
   "dynamically enabled or disabled.\n\n"

   "@tsexample\n"
   "datablock ParticleEmitterNodeData( SimpleEmitterNodeData )\n"
   "{\n"
   "   timeMultiple = 1.0;\n"
   "};\n\n"

   "%emitter = new ParticleEmitterNode()\n"
   "{\n"
   "   datablock = SimpleEmitterNodeData;\n"
   "   active = true;\n"
   "   emitter = FireEmitterData;\n"
   "   velocity = 3.5;\n"
   "};\n\n"

   "// Dynamically change emitter datablock\n"
   "%emitter.setEmitterDataBlock( DustEmitterData );\n"
   "@endtsexample\n"

   "@note To change the emitter field dynamically (after the ParticleEmitterNode "
   "object has been created) you must use the setEmitterDataBlock() method or the "
   "change will not be replicated to other clients in the game.\n"
   "Similarly, use the setActive() method instead of changing the active field "
   "directly. When changing velocity, you need to toggle setActive() on and off "
   "to force the state change to be transmitted to other clients.\n\n"

   "@ingroup FX\n"
   "@see ParticleEmitterNodeData\n"
   "@see ParticleEmitterData\n"
);


//-----------------------------------------------------------------------------
// ParticleEmitterNodeData
//-----------------------------------------------------------------------------
ParticleEmitterNodeData::ParticleEmitterNodeData()
{
   timeMultiple = 1.0;
}

ParticleEmitterNodeData::~ParticleEmitterNodeData()
{

}

//-----------------------------------------------------------------------------
// initPersistFields
//-----------------------------------------------------------------------------
void ParticleEmitterNodeData::initPersistFields()
{
   addField( "timeMultiple", TYPEID< F32 >(), Offset(timeMultiple, ParticleEmitterNodeData),
      "@brief Time multiplier for particle emitter nodes.\n\n"
      "Increasing timeMultiple is like running the emitter at a faster rate - single-shot "
      "emitters will complete in a shorter time, and continuous emitters will generate "
      "particles more quickly.\n\n"
      "Valid range is 0.01 - 100." );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// onAdd
//-----------------------------------------------------------------------------
bool ParticleEmitterNodeData::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   if( timeMultiple < 0.01 || timeMultiple > 100 )
   {
      Con::warnf("ParticleEmitterNodeData::onAdd(%s): timeMultiple must be between 0.01 and 100", getName());
      timeMultiple = timeMultiple < 0.01 ? 0.01 : 100;
   }

   return true;
}


//-----------------------------------------------------------------------------
// preload
//-----------------------------------------------------------------------------
bool ParticleEmitterNodeData::preload(bool server, String &errorStr)
{
   if( Parent::preload(server, errorStr) == false )
      return false;

   return true;
}


//-----------------------------------------------------------------------------
// packData
//-----------------------------------------------------------------------------
void ParticleEmitterNodeData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(timeMultiple);
}

//-----------------------------------------------------------------------------
// unpackData
//-----------------------------------------------------------------------------
void ParticleEmitterNodeData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&timeMultiple);
}


//-----------------------------------------------------------------------------
// ParticleEmitterNode
//-----------------------------------------------------------------------------
ParticleEmitterNode::ParticleEmitterNode()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);
   mTypeMask |= EnvironmentObjectType;

   mActive = true;

   mDataBlock          = NULL;
   mEmitterDatablock   = NULL;
   mEmitterDatablockId = 0;
   mEmitter            = NULL;
   mVelocity           = 1.0;
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
ParticleEmitterNode::~ParticleEmitterNode()
{
   //
}

//-----------------------------------------------------------------------------
// initPersistFields
//-----------------------------------------------------------------------------
void ParticleEmitterNode::initPersistFields()
{
   addField( "active", TYPEID< bool >(), Offset(mActive,ParticleEmitterNode),
      "Controls whether particles are emitted from this node." );
   addField( "emitter",  TYPEID< ParticleEmitterData >(), Offset(mEmitterDatablock, ParticleEmitterNode),
      "Datablock to use when emitting particles." );
   addField( "velocity", TYPEID< F32 >(), Offset(mVelocity, ParticleEmitterNode),
      "Velocity to use when emitting particles (in the direction of the "
      "ParticleEmitterNode object's up (Z) axis)." );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// onAdd
//-----------------------------------------------------------------------------
bool ParticleEmitterNode::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   if( !mEmitterDatablock && mEmitterDatablockId != 0 )
   {
      if( Sim::findObject(mEmitterDatablockId, mEmitterDatablock) == false )
         Con::errorf(ConsoleLogEntry::General, "ParticleEmitterNode::onAdd: Invalid packet, bad datablockId(mEmitterDatablock): %d", mEmitterDatablockId);
   }

   if( isClientObject() )
   {
      setEmitterDataBlock( mEmitterDatablock );
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
void ParticleEmitterNode::onRemove()
{
   removeFromScene();
   if( isClientObject() )
   {
      if( mEmitter )
      {
         mEmitter->deleteWhenEmpty();
         mEmitter = NULL;
      }
   }

   Parent::onRemove();
}

//-----------------------------------------------------------------------------
// onNewDataBlock
//-----------------------------------------------------------------------------
bool ParticleEmitterNode::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   mDataBlock = dynamic_cast<ParticleEmitterNodeData*>( dptr );
   if ( !mDataBlock || !Parent::onNewDataBlock( dptr, reload ) )
      return false;

   // Todo: Uncomment if this is a "leaf" class
   scriptOnNewDataBlock();
   return true;
}

//-----------------------------------------------------------------------------
void ParticleEmitterNode::inspectPostApply()
{
   Parent::inspectPostApply();
   setMaskBits(StateMask | EmitterDBMask);
}

//-----------------------------------------------------------------------------
// advanceTime
//-----------------------------------------------------------------------------
void ParticleEmitterNode::processTick(const Move* move)
{
   Parent::processTick(move);

   if ( isMounted() )
   {
      MatrixF mat;
      mMount.object->getMountTransform( mMount.node, mMount.xfm, &mat );
      setTransform( mat );
   }
}

void ParticleEmitterNode::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);
   
   if(!mActive || mEmitter.isNull() || !mDataBlock)
      return;

   Point3F emitPoint, emitVelocity;
   Point3F emitAxis(0, 0, 1);
   getTransform().mulV(emitAxis);
   getTransform().getColumn(3, &emitPoint);
   emitVelocity = emitAxis * mVelocity;

   mEmitter->emitParticles(emitPoint, emitPoint,
                           emitAxis,
                           emitVelocity, (U32)(dt * mDataBlock->timeMultiple * 1000.0f));
}

//-----------------------------------------------------------------------------
// packUpdate
//-----------------------------------------------------------------------------
U32 ParticleEmitterNode::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if ( stream->writeFlag( mask & InitialUpdateMask ) )
   {
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
   }

   if ( stream->writeFlag( mask & EmitterDBMask ) )
   {
      if( stream->writeFlag(mEmitterDatablock != NULL) )
      {
         stream->writeRangedU32(mEmitterDatablock->getId(), DataBlockObjectIdFirst,
            DataBlockObjectIdLast);
      }
   }

   if ( stream->writeFlag( mask & StateMask ) )
   {
      stream->writeFlag( mActive );
      stream->write( mVelocity );
   }

   return retMask;
}

//-----------------------------------------------------------------------------
// unpackUpdate
//-----------------------------------------------------------------------------
void ParticleEmitterNode::unpackUpdate(NetConnection* con, BitStream* stream)
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
      mEmitterDatablockId = stream->readFlag() ?
         stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast) : 0;

      ParticleEmitterData *emitterDB = NULL;
      Sim::findObject( mEmitterDatablockId, emitterDB );
      if ( isProperlyAdded() )
         setEmitterDataBlock( emitterDB );
   }

   if ( stream->readFlag() )
   {
      mActive = stream->readFlag();
      stream->read( &mVelocity );
   }
}

void ParticleEmitterNode::setEmitterDataBlock(ParticleEmitterData* data)
{
   if ( isServerObject() )
   {
      setMaskBits( EmitterDBMask );
   }
   else
   {
      ParticleEmitter* pEmitter = NULL;
      if ( data )
      {
         // Create emitter with new datablock
         pEmitter = new ParticleEmitter;
         pEmitter->onNewDataBlock( data, false );
         if( pEmitter->registerObject() == false )
         {
            Con::warnf(ConsoleLogEntry::General, "Could not register base emitter for particle of class: %s", data->getName() ? data->getName() : data->getIdString() );
            delete pEmitter;
            return;
         }
      }

      // Replace emitter
      if ( mEmitter )
         mEmitter->deleteWhenEmpty();

      mEmitter = pEmitter;
   }

   mEmitterDatablock = data;
}


DefineEngineMethod(ParticleEmitterNode, setEmitterDataBlock, void, (ParticleEmitterData* emitterDatablock), (nullAsType<ParticleEmitterData*>()),
   "Assigns the datablock for this emitter node.\n"
   "@param emitterDatablock ParticleEmitterData datablock to assign\n"
   "@tsexample\n"
   "// Assign a new emitter datablock\n"
   "%emitter.setEmitterDatablock( %emitterDatablock );\n"
   "@endtsexample\n" )
{
   if ( !emitterDatablock )
   {
      Con::errorf("ParticleEmitterData datablock could not be found when calling setEmitterDataBlock in particleEmitterNode.");
      return;
   }

   object->setEmitterDataBlock(emitterDatablock);
}

DefineEngineMethod(ParticleEmitterNode, setActive, void, (bool active),,
   "Turns the emitter on or off.\n"
   "@param active New emitter state\n" )
{
   object->setActive( active );
}
