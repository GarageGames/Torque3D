
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

#include <typeinfo>
#include "afx/arcaneFX.h"

#if defined(STOCK_TGE_PARTICLES)
#include "game/fx/particleEngine.h"
#else
#include "afx/ce/afxParticleEmitter.h"
#endif

#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/ea/afxEA_ParticleEmitter.h"
#include "afx/util/afxParticlePool.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_ParticleEmitter

afxEA_ParticleEmitter::afxEA_ParticleEmitter()
{
  emitter_data = 0;
  emitter = 0;
  do_bbox_update = false;
}

afxEA_ParticleEmitter::~afxEA_ParticleEmitter()
{
  if (emitter)
  {
    clearNotify(emitter);
    emitter->deleteWhenEmpty();
    emitter = 0;
  }
}

void afxEA_ParticleEmitter::ea_set_datablock(SimDataBlock* db)
{
   emitter_data = dynamic_cast<ParticleEmitterData*>(db);
}

bool afxEA_ParticleEmitter::ea_start()
{
  if (!emitter_data)
  {
    Con::errorf("afxEA_ParticleEmitter::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

#if defined(STOCK_TGE_PARTICLES)
  emitter = new ParticleEmitter();
  emitter->onNewDataBlock(emitter_data);
#else
   afxParticleEmitterData* afx_emitter_db = dynamic_cast<afxParticleEmitterData*>(emitter_data);
   if (afx_emitter_db)
   {
      if (dynamic_cast<afxParticleEmitterVectorData*>(emitter_data))
      {
         afxParticleEmitterVector* pe = new afxParticleEmitterVector();
         pe->onNewDataBlock(afx_emitter_db, false);
         pe->setAFXOwner(choreographer);
         emitter = pe;
      }
      else if (dynamic_cast<afxParticleEmitterConeData*>(emitter_data))
      {
         afxParticleEmitterCone* pe = new afxParticleEmitterCone();
         pe->onNewDataBlock(afx_emitter_db, false);
         pe->setAFXOwner(choreographer);
         emitter = pe;
      }
      else if (dynamic_cast<afxParticleEmitterPathData*>(emitter_data))
      {
         afxParticleEmitterPath* pe = new afxParticleEmitterPath();
         pe->onNewDataBlock(afx_emitter_db, false);
         pe->setAFXOwner(choreographer);
         emitter = pe;
      }
      else if (dynamic_cast<afxParticleEmitterDiscData*>(emitter_data))
      {
         afxParticleEmitterDisc* pe = new afxParticleEmitterDisc();
         pe->onNewDataBlock(afx_emitter_db, false);
         pe->setAFXOwner(choreographer);
         emitter = pe;
      }
   }
   else
   {
      emitter = new ParticleEmitter();
      emitter->onNewDataBlock(emitter_data, false);
   }
#endif

#if defined(AFX_CAP_PARTICLE_POOLS)
  // here we find or create any required particle-pools
  if (emitter_data->pool_datablock)
  { 
    afxParticlePool* pool = choreographer->findParticlePool(emitter_data->pool_datablock, emitter_data->pool_index);
    if (!pool)
    {
      afxParticlePoolData* pool_data = emitter_data->pool_datablock;
      if (pool_data->getSubstitutionCount() > 0)
      {
        // clone the datablock and perform substitutions
        afxParticlePoolData* orig_db = pool_data;
        pool_data = new afxParticlePoolData(*orig_db, true);
        orig_db->performSubstitutions(pool_data, choreographer, group_index);
      }

      pool = new afxParticlePool();
      pool->onNewDataBlock(pool_data, false);
      pool->setKeyBlock(emitter_data->pool_datablock, emitter_data->pool_index);
      if (!pool->registerObject())
      {
        Con::errorf("afxEA_ParticleEmitter::ea_start() -- Failed to register Particle Pool.");
        delete pool;
        pool = 0;
      }
      if (pool)
      {
        pool->setChoreographer(choreographer);
        choreographer->registerParticlePool(pool);
      }
    }
    if (pool)
      emitter->setPool(pool);
  }
#endif

  if (!emitter->registerObject())
  {
    delete emitter;
    emitter = NULL;
    Con::errorf("afxEA_ParticleEmitter::ea_start() -- effect failed to register.");
    return false;
  }

  if (datablock->forced_bbox.isValidBox())
  {
    do_bbox_update = true;
  }

  emitter->setSortPriority(datablock->sort_priority);
  deleteNotify(emitter);

  return true;
}

bool afxEA_ParticleEmitter::ea_update(F32 dt)
{
  if (emitter && in_scope)
  {
    if (do_bbox_update)
    {
      Box3F bbox = emitter->getObjBox();

      bbox.minExtents = updated_pos + datablock->forced_bbox.minExtents;
      bbox.maxExtents = updated_pos + datablock->forced_bbox.maxExtents;

      emitter->setForcedObjBox(bbox);
      emitter->setTransform(emitter->getTransform());

      if (!datablock->update_forced_bbox)
        do_bbox_update = false;
    }

    if (do_fades)
      emitter->setFadeAmount(fade_value);
    
    emitter->emitParticlesExt(updated_xfm, updated_pos, Point3F(0.0,0.0,0.0), (U32)(dt*1000));
  }

  return true;
}

void afxEA_ParticleEmitter::ea_finish(bool was_stopped)
{
  if (arcaneFX::isShutdown())
    return;

  if (emitter)
  {
    // make sure particles are fully faded.
    //   note - fully faded particles are not always
    //     invisible, so they are still kept alive and 
    //     deleted via deleteWhenEmpty().
    if (ew_timing.fade_out_time > 0.0f)
      emitter->setFadeAmount(0.0f);
    if (dynamic_cast<afxParticleEmitter*>(emitter))
      ((afxParticleEmitter*)emitter)->setAFXOwner(0);
    clearNotify(emitter);
    emitter->deleteWhenEmpty();
    emitter = 0;
  }
}

void afxEA_ParticleEmitter::do_runtime_substitutions()
{
  bool clone_particles = false;
  for (S32 i = 0; i < emitter_data->particleDataBlocks.size(); i++)
  {
    if (emitter_data->particleDataBlocks[i] && (emitter_data->particleDataBlocks[i]->getSubstitutionCount() > 0))
    {
      clone_particles = true;
      break;
    }
  }
     
  if (clone_particles || (emitter_data->getSubstitutionCount() > 0))
  {
    afxParticleEmitterData* afx_emitter_db = dynamic_cast<afxParticleEmitterData*>(emitter_data);
    if (afx_emitter_db)
    {
      if (dynamic_cast<afxParticleEmitterVectorData*>(emitter_data))
      {
        afxParticleEmitterVectorData* orig_db = (afxParticleEmitterVectorData*)emitter_data;
        emitter_data = new afxParticleEmitterVectorData(*orig_db, true);
        orig_db->performSubstitutions(emitter_data, choreographer, group_index);
      }
      else if (dynamic_cast<afxParticleEmitterConeData*>(emitter_data))
      {
        afxParticleEmitterConeData* orig_db = (afxParticleEmitterConeData*)emitter_data;
        emitter_data = new afxParticleEmitterConeData(*orig_db, true);
        orig_db->performSubstitutions(emitter_data, choreographer, group_index);
     }
      else if (dynamic_cast<afxParticleEmitterPathData*>(emitter_data))
      {
        afxParticleEmitterPathData* orig_db = (afxParticleEmitterPathData*)emitter_data;
        emitter_data = new afxParticleEmitterPathData(*orig_db, true);
        orig_db->performSubstitutions(emitter_data, choreographer, group_index);
      }
      else if (dynamic_cast<afxParticleEmitterDiscData*>(emitter_data))
      {
        afxParticleEmitterDiscData* orig_db = (afxParticleEmitterDiscData*)emitter_data;
        emitter_data = new afxParticleEmitterDiscData(*orig_db, true);
        orig_db->performSubstitutions(emitter_data, choreographer, group_index);
      }
    }
    else
    {
      ParticleEmitterData* orig_db = emitter_data;
      emitter_data = new ParticleEmitterData(*orig_db, true);
      orig_db->performSubstitutions(emitter_data, choreographer, group_index);
    }

    if (clone_particles)
    {
      for (S32 i = 0; i < emitter_data->particleDataBlocks.size(); i++)
      {
        if (emitter_data->particleDataBlocks[i] && (emitter_data->particleDataBlocks[i]->getSubstitutionCount() > 0))
        {
          // clone the datablock and perform substitutions
          ParticleData* orig_db = emitter_data->particleDataBlocks[i];
          emitter_data->particleDataBlocks[i] = new ParticleData(*orig_db, true);
          orig_db->performSubstitutions(emitter_data->particleDataBlocks[i], choreographer, group_index);
        }
      }
    }
  }
}

void afxEA_ParticleEmitter::onDeleteNotify(SimObject* obj)
{
  if (emitter == dynamic_cast<ParticleEmitter*>(obj))
    emitter = 0;

  Parent::onDeleteNotify(obj);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_ParticleEmitterDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_ParticleEmitterDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }

  virtual afxEffectWrapper* create() const { return new afxEA_ParticleEmitter; }
};

afxEA_ParticleEmitterDesc afxEA_ParticleEmitterDesc::desc;

bool afxEA_ParticleEmitterDesc::testEffectType(const SimDataBlock* db) const
{
#if defined(STOCK_TGE_PARTICLES)
  return (typeid(ParticleEmitterData) == typeid(*db));
#else
  if (typeid(ParticleEmitterData) == typeid(*db))
     return true;
  if (typeid(afxParticleEmitterVectorData) == typeid(*db))
     return true;
  if (typeid(afxParticleEmitterConeData) == typeid(*db))
     return true;
  if (typeid(afxParticleEmitterPathData) == typeid(*db))
     return true;
  if (typeid(afxParticleEmitterDiscData) == typeid(*db))
     return true;

  return false;
#endif
}

bool afxEA_ParticleEmitterDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//