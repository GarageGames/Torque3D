
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
#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/ce/afxProjectile.h"
#include "afx/ce/afxMachineGun.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_MachineGun 

class afxEA_MachineGun : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxMachineGunData* gun_data;
  ProjectileData*    bullet_data;

  bool          shooting;
  F32           start_time;
  F32           shot_gap;
  S32           shot_count;

  void          launch_projectile();
  void          do_runtime_substitutions();

public:
  /*C*/         afxEA_MachineGun();
  /*D*/         ~afxEA_MachineGun();

  virtual void  ea_set_datablock(SimDataBlock*);
  virtual bool  ea_start();
  virtual bool  ea_update(F32 dt);
  virtual void  ea_finish(bool was_stopped);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_MachineGun::afxEA_MachineGun()
{
  gun_data = 0;
  bullet_data = 0;
  shooting = false;
  start_time = 0.0f;
  shot_count = 0;
  shot_gap = 0.2f;
}

afxEA_MachineGun::~afxEA_MachineGun()
{
  if (gun_data && gun_data->isTempClone())
    delete gun_data;
  gun_data = 0;
}

void afxEA_MachineGun::ea_set_datablock(SimDataBlock* db)
{
  gun_data = dynamic_cast<afxMachineGunData*>(db);
  if (gun_data)
    bullet_data = gun_data->projectile_data;
}

bool afxEA_MachineGun::ea_start()
{
  if (!gun_data)
  {
    Con::errorf("afxEA_MachineGun::ea_start() -- missing or incompatible datablock.");
    return false;
  }
  if (!bullet_data)
  {
    Con::errorf("afxEA_MachineGun::ea_start() -- missing or incompatible ProjectileData.");
    return false;
  }

  do_runtime_substitutions();

  if (gun_data->rounds_per_minute > 0)
    shot_gap = 60.0f/gun_data->rounds_per_minute;

  return true;
}

bool afxEA_MachineGun::ea_update(F32 dt)
{
  if (!shooting)
  {
    start_time = elapsed;
    shooting = true;
  }
  else
  {
    F32 next_shot = start_time + (shot_count+1)*shot_gap;
    while (next_shot < elapsed)
    {
      if (in_scope)
        launch_projectile();
      next_shot += shot_gap;
      shot_count++;
    }
  }

  return true;
}

void afxEA_MachineGun::ea_finish(bool was_stopped)
{
}

void afxEA_MachineGun::launch_projectile()
{
  afxProjectile* projectile = new afxProjectile();

  ProjectileData* next_bullet = bullet_data;

  if (bullet_data->getSubstitutionCount() > 0)
  {
    next_bullet = new ProjectileData(*bullet_data, true);
    bullet_data->performSubstitutions(next_bullet, choreographer, group_index);
  }

  projectile->onNewDataBlock(next_bullet, false);

  F32 muzzle_vel = next_bullet->muzzleVelocity;

  afxConstraint* pos_cons = getPosConstraint();
  ShapeBase* src_obj = (pos_cons) ? (dynamic_cast<ShapeBase*>(pos_cons->getSceneObject())) : 0;

  Point3F dir_vec = updated_aim - updated_pos;
  dir_vec.normalizeSafe();
  dir_vec *= muzzle_vel;
  projectile->init(updated_pos, dir_vec, src_obj);
  if (!projectile->registerObject())
  {
    delete projectile;
    projectile = 0;
    Con::errorf("afxEA_MachineGun::launch_projectile() -- projectile failed to register.");
  }
  if (projectile)
    projectile->setDataField(StringTable->insert("afxOwner"), 0, choreographer->getIdString());
}

void afxEA_MachineGun::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (gun_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxMachineGunData* orig_db = gun_data;
    gun_data = new afxMachineGunData(*orig_db, true);
    orig_db->performSubstitutions(gun_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_MachineGunDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_MachineGunDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return true; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return false; }

  virtual afxEffectWrapper* create() const { return new afxEA_MachineGun; }
};

afxEA_MachineGunDesc afxEA_MachineGunDesc::desc;

bool afxEA_MachineGunDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxMachineGunData) == typeid(*db));
}

bool afxEA_MachineGunDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//