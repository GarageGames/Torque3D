
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"

#include "T3D/fx/particleEmitter.h"

#include "afx/afxChoreographer.h"
#include "afx/util/afxParticlePool.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxParticlePoolData);

ConsoleDocClass( afxParticlePoolData,
   "@brief A ParticlePool datablock.\n\n"

   "@ingroup afxUtil\n"
   "@ingroup AFX\n"
);

Vector<afxParticlePool::SortParticlePool> afxParticlePool::orderedVector;

afxParticlePoolData::afxParticlePoolData()            
{
  pool_type = POOL_NORMAL;
  base_color.set(0.0f, 0.0f, 0.0f, 1.0f);
  blend_weight = 1.0f;
}

afxParticlePoolData::afxParticlePoolData(const afxParticlePoolData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  pool_type = other.pool_type;
  base_color = other.base_color;
  blend_weight = other.blend_weight;
}

ImplementEnumType( afxParticlePool_PoolType, "Possible particle pool types.\n" "@ingroup afxParticlePool\n\n" )
  { afxParticlePoolData::POOL_NORMAL,  "normal",      "..." },
  { afxParticlePoolData::POOL_TWOPASS, "two-pass",    "..." },
EndImplementEnumType;

afxParticlePoolData::~afxParticlePoolData()
{
}

void afxParticlePoolData::initPersistFields()
{
  addField("poolType",    TYPEID< afxParticlePoolData::PoolType >(),    Offset(pool_type,    afxParticlePoolData),
    "...");
  addField("baseColor",   TypeColorF,  Offset(base_color,   afxParticlePoolData),
    "...");
  addField("blendWeight", TypeF32,     Offset(blend_weight, afxParticlePoolData),
    "...");

  Parent::initPersistFields();
};

void afxParticlePoolData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->write(pool_type);
  stream->write(base_color);
  stream->write(blend_weight);
};

void afxParticlePoolData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&pool_type);
  stream->read(&base_color);
  stream->read(&blend_weight);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_NETOBJECT_V1(afxParticlePool);

ConsoleDocClass( afxParticlePool,
   "@brief A ParticlePool object as defined by an afxParticlePoolData datablock.\n\n"

   "@ingroup afxUtil\n"
   "@ingroup AFX\n"
);

afxParticlePool::afxParticlePool()
{
  mDataBlock = 0;
  key_block = 0;
  key_index = 0;
  choreographer = 0;
  sort_priority = S8_MAX;

  mNetFlags.set(IsGhost);
  mTypeMask |= StaticObjectType;

  mCurBuffSize = mCurBuffSize2 = 0;
};

afxParticlePool::~afxParticlePool()
{
  for (S32 i = 0; i < emitters.size(); i++)
    if (emitters[i])
      emitters[i]->clearPool();

  if (choreographer)
    choreographer->unregisterParticlePool(this);

  if (mDataBlock && mDataBlock->isTempClone())
  {
    delete mDataBlock;
    mDataBlock = 0;
  }
}

bool afxParticlePool::onNewDataBlock(GameBaseData* dptr, bool reload)
{
  mDataBlock = dynamic_cast<afxParticlePoolData*>(dptr);
  if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
    return false;

  return true;
}

bool afxParticlePool::onAdd()
{
  if (!Parent::onAdd())
    return false;

  mObjBox.minExtents.set(-0.5, -0.5, -0.5);
  mObjBox.maxExtents.set( 0.5,  0.5,  0.5);

  resetWorldBox();

  addToScene();

  return true;
};

void afxParticlePool::onRemove()
{
  removeFromScene();

  Parent::onRemove();
};

void afxParticlePool::addParticleEmitter(ParticleEmitter* emitter)
{
  emitters.push_back(emitter);
}

void afxParticlePool::removeParticleEmitter(ParticleEmitter* emitter)
{ 
  for (U32 i=0; i < emitters.size(); i++)
    if (emitters[i] == emitter)
    {
      emitters.erase(i);
      break;
    }

  if (emitters.empty())
  {
    if (choreographer)
    {
      choreographer->unregisterParticlePool(this);
      choreographer = 0;
    }
    Sim::postEvent(this, new ObjectDeleteEvent, Sim::getCurrentTime() + 500);
  }
}

void afxParticlePool::updatePoolBBox(ParticleEmitter* emitter)
{
  if (emitter->mObjBox.minExtents.x < mObjBox.minExtents.x) 
    mObjBox.minExtents.x = emitter->mObjBox.minExtents.x;
  if (emitter->mObjBox.minExtents.y < mObjBox.minExtents.y) 
    mObjBox.minExtents.y = emitter->mObjBox.minExtents.y;
  if (emitter->mObjBox.minExtents.z < mObjBox.minExtents.z) 
    mObjBox.minExtents.z = emitter->mObjBox.minExtents.z;
  if (emitter->mObjBox.maxExtents.x > mObjBox.maxExtents.x) 
    mObjBox.maxExtents.x = emitter->mObjBox.maxExtents.x;
  if (emitter->mObjBox.maxExtents.y > mObjBox.maxExtents.y) 
    mObjBox.maxExtents.y = emitter->mObjBox.maxExtents.y;
  if (emitter->mObjBox.maxExtents.z > mObjBox.maxExtents.z) 
    mObjBox.maxExtents.z = emitter->mObjBox.maxExtents.z;

  resetWorldBox();
}

void afxParticlePool::setSortPriority(S8 priority)
{
  if (priority < sort_priority)
     sort_priority = (priority == 0) ? 1 : priority;
}

int QSORT_CALLBACK afxParticlePool::cmpSortParticlePool(const void* p1, const void* p2)
{
  const SortParticlePool* sp1 = (const SortParticlePool*)p1;
  const SortParticlePool* sp2 = (const SortParticlePool*)p2;
  if (sp2->k > sp1->k)
    return 1;
  else if (sp2->k == sp1->k)
    return 0;
  else
    return -1;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

