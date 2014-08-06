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

#include "particleSystemNode.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "T3D/fx/ParticleSystem/particleSystem.h"
#include "math/mathIO.h"
#include "sim/netConnection.h"
#include "console/engineAPI.h"

IMPLEMENT_CO_DATABLOCK_V1(ParticleSystemNodeData);
IMPLEMENT_CO_NETOBJECT_V1(ParticleSystemNode);

ConsoleDocClass(ParticleSystemNodeData,
   "@brief Contains additional data to be associated with a ParticleSystemNode."
   "@ingroup FX\n"
   );

ConsoleDocClass(ParticleSystemNode,
   "@brief A particle System object that can be positioned in the world and "
   "dynamically enabled or disabled.\n\n"

   "@tsexample\n"
   "datablock ParticleSystemNodeData( SimpleSystemNodeData )\n"
   "{\n"
   "   timeMultiple = 1.0;\n"
   "};\n\n"

   "%System = new ParticleSystemNode()\n"
   "{\n"
   "   datablock = SimpleSystemNodeData;\n"
   "   active = true;\n"
   "   System = FireSystemData;\n"
   "   velocity = 3.5;\n"
   "};\n\n"

   "// Dynamically change System datablock\n"
   "%System.setSystemDataBlock( DustSystemData );\n"
   "@endtsexample\n"

   "@note To change the System field dynamically (after the ParticleSystemNode "
   "object has been created) you must use the setSystemDataBlock() method or the "
   "change will not be replicated to other clients in the game.\n"
   "Similarly, use the setActive() method instead of changing the active field "
   "directly. When changing velocity, you need to toggle setActive() on and off "
   "to force the state change to be transmitted to other clients.\n\n"

   "@ingroup FX\n"
   "@see ParticleSystemNodeData\n"
   "@see ParticleSystemData\n"
   );


//-----------------------------------------------------------------------------
// ParticleSystemNodeData
//-----------------------------------------------------------------------------
ParticleSystemNodeData::ParticleSystemNodeData()
{
   timeMultiple = 1.0;
}

ParticleSystemNodeData::~ParticleSystemNodeData()
{

}

//-----------------------------------------------------------------------------
// initPersistFields
//-----------------------------------------------------------------------------
void ParticleSystemNodeData::initPersistFields()
{
   addField("timeMultiple", TYPEID< F32 >(), Offset(timeMultiple, ParticleSystemNodeData),
      "@brief Time multiplier for particle System nodes.\n\n"
      "Increasing timeMultiple is like running the System at a faster rate - single-shot "
      "Systems will complete in a shorter time, and continuous Systems will generate "
      "particles more quickly.\n\n"
      "Valid range is 0.01 - 100.");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// onAdd
//-----------------------------------------------------------------------------
bool ParticleSystemNodeData::onAdd()
{
   if (!Parent::onAdd())
      return false;

   if (timeMultiple < 0.01 || timeMultiple > 100)
   {
      Con::warnf("ParticleSystemNodeData::onAdd(%s): timeMultiple must be between 0.01 and 100", getName());
      timeMultiple = timeMultiple < 0.01 ? 0.01 : 100;
   }

   return true;
}


//-----------------------------------------------------------------------------
// preload
//-----------------------------------------------------------------------------
bool ParticleSystemNodeData::preload(bool server, String &errorStr)
{
   if (Parent::preload(server, errorStr) == false)
      return false;

   return true;
}


//-----------------------------------------------------------------------------
// packData
//-----------------------------------------------------------------------------
void ParticleSystemNodeData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(timeMultiple);
}

//-----------------------------------------------------------------------------
// unpackData
//-----------------------------------------------------------------------------
void ParticleSystemNodeData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&timeMultiple);
}


//-----------------------------------------------------------------------------
// ParticleSystemNode
//-----------------------------------------------------------------------------
ParticleSystemNode::ParticleSystemNode()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);
   mTypeMask |= EnvironmentObjectType;

   mActive = true;

   mDataBlock = NULL;
   mSystemDatablock = NULL;
   mSystemDatablockId = 0;
   mSystem = NULL;
   mVelocity = 1.0;
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
ParticleSystemNode::~ParticleSystemNode()
{
   //
}

//-----------------------------------------------------------------------------
// initPersistFields
//-----------------------------------------------------------------------------
void ParticleSystemNode::initPersistFields()
{
   addField("active", TYPEID< bool >(), Offset(mActive, ParticleSystemNode),
      "Controls whether particles are emitted from this node.");
   addField("System", TYPEID< ParticleSystemData >(), Offset(mSystemDatablock, ParticleSystemNode),
      "Datablock to use when emitting particles.");
   addField("velocity", TYPEID< F32 >(), Offset(mVelocity, ParticleSystemNode),
      "Velocity to use when emitting particles (in the direction of the "
      "ParticleSystemNode object's up (Z) axis).");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// onAdd
//-----------------------------------------------------------------------------
bool ParticleSystemNode::onAdd()
{
   if (!Parent::onAdd())
      return false;

   if (!mSystemDatablock && mSystemDatablockId != 0)
   {
      if (Sim::findObject(mSystemDatablockId, mSystemDatablock) == false)
         Con::errorf(ConsoleLogEntry::General, "ParticleSystemNode::onAdd: Invalid packet, bad datablockId(mSystemDatablock): %d", mSystemDatablockId);
   }

   if (isClientObject())
   {
      setSystemDataBlock(mSystemDatablock);
   }
   else
   {
      setMaskBits(StateMask | SystemDBMask);
   }

   mObjBox.minExtents.set(-0.5, -0.5, -0.5);
   mObjBox.maxExtents.set(0.5, 0.5, 0.5);
   resetWorldBox();
   addToScene();

   return true;
}

//-----------------------------------------------------------------------------
// onRemove
//-----------------------------------------------------------------------------
void ParticleSystemNode::onRemove()
{
   removeFromScene();
   if (isClientObject())
   {
      if (mSystem)
      {
         mSystem->deleteWhenEmpty();
         mSystem = NULL;
      }
   }

   Parent::onRemove();
}

//-----------------------------------------------------------------------------
// onNewDataBlock
//-----------------------------------------------------------------------------
bool ParticleSystemNode::onNewDataBlock(GameBaseData *dptr, bool reload)
{
   mDataBlock = dynamic_cast<ParticleSystemNodeData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
      return false;

   // Todo: Uncomment if this is a "leaf" class
   scriptOnNewDataBlock();
   return true;
}

//-----------------------------------------------------------------------------
void ParticleSystemNode::inspectPostApply()
{
   Parent::inspectPostApply();
   setMaskBits(StateMask | SystemDBMask);
}

//-----------------------------------------------------------------------------
// advanceTime
//-----------------------------------------------------------------------------
void ParticleSystemNode::processTick(const Move* move)
{
   Parent::processTick(move);

   if (isMounted())
   {
      MatrixF mat;
      mMount.object->getMountTransform(mMount.node, mMount.xfm, &mat);
      setTransform(mat);
   }
}

void ParticleSystemNode::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   if (!mActive || mSystem.isNull() || !mDataBlock)
      return;

   Point3F emitPoint, emitVelocity;
   Point3F emitAxis(0, 0, 1);
   getTransform().mulV(emitAxis);
   getTransform().getColumn(3, &emitPoint);
   emitVelocity = emitAxis * mVelocity;

   mSystem->emitParticles(emitPoint, emitPoint,
      emitAxis,
      emitVelocity, (U32)(dt * mDataBlock->timeMultiple * 1000.0f));
}

//-----------------------------------------------------------------------------
// packUpdate
//-----------------------------------------------------------------------------
U32 ParticleSystemNode::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & InitialUpdateMask))
   {
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
   }

   if (stream->writeFlag(mask & SystemDBMask))
   {
      if (stream->writeFlag(mSystemDatablock != NULL))
      {
         stream->writeRangedU32(mSystemDatablock->getId(), DataBlockObjectIdFirst,
            DataBlockObjectIdLast);
      }
   }

   if (stream->writeFlag(mask & StateMask))
   {
      stream->writeFlag(mActive);
      stream->write(mVelocity);
   }

   return retMask;
}

//-----------------------------------------------------------------------------
// unpackUpdate
//-----------------------------------------------------------------------------
void ParticleSystemNode::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag())
   {
      MatrixF temp;
      Point3F tempScale;
      mathRead(*stream, &temp);
      mathRead(*stream, &tempScale);

      setScale(tempScale);
      setTransform(temp);
   }

   if (stream->readFlag())
   {
      mSystemDatablockId = stream->readFlag() ?
         stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast) : 0;

      ParticleSystemData *SystemDB = NULL;
      Sim::findObject(mSystemDatablockId, SystemDB);
      if (isProperlyAdded())
         setSystemDataBlock(SystemDB);
   }

   if (stream->readFlag())
   {
      mActive = stream->readFlag();
      stream->read(&mVelocity);
   }
}

void ParticleSystemNode::setSystemDataBlock(ParticleSystemData* data)
{
   if (isServerObject())
   {
      setMaskBits(SystemDBMask);
   }
   else
   {
      ParticleSystem* pSystem = NULL;
      if (data)
      {
         // Create System with new datablock
         pSystem = new ParticleSystem;
         pSystem->onNewDataBlock(data, false);
         if (pSystem->registerObject() == false)
         {
            Con::warnf(ConsoleLogEntry::General, "Could not register base System for particle of class: %s", data->getName() ? data->getName() : data->getIdString());
            delete pSystem;
            return;
         }
      }

      // Replace System
      if (mSystem)
         mSystem->deleteWhenEmpty();

      mSystem = pSystem;
   }

   mSystemDatablock = data;
}

DefineEngineMethod(ParticleSystemNode, setSystemDataBlock, void, (ParticleSystemData* SystemDatablock), (0),
   "Assigns the datablock for this System node.\n"
   "@param SystemDatablock ParticleSystemData datablock to assign\n"
   "@tsexample\n"
   "// Assign a new System datablock\n"
   "%System.setSystemDatablock( %SystemDatablock );\n"
   "@endtsexample\n")
{
   if (!SystemDatablock)
   {
      Con::errorf("ParticleSystemData datablock could not be found when calling setSystemDataBlock in ParticleSystemNode.");
      return;
   }

   object->setSystemDataBlock(SystemDatablock);
}

DefineEngineMethod(ParticleSystemNode, setActive, void, (bool active), ,
   "Turns the System on or off.\n"
   "@param active New System state\n")
{
   object->setActive(active);
}