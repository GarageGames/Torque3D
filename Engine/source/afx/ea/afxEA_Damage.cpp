
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
#include "afx/ce/afxDamage.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_Damage 

class afxEA_Damage : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxDamageData*    damage_data;
  bool              started;
  U8                repeat_cnt;
  U32               dot_delta_ms;
  U32               next_dot_time;
  Point3F           impact_pos;
  SimObjectId       impacted_obj_id;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_Damage();
  /*C*/             ~afxEA_Damage();

  virtual bool      isDone();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_Damage::afxEA_Damage()
{
  damage_data = 0;
  started = false;
  repeat_cnt = 0;
  dot_delta_ms = 0; 
  next_dot_time = 0;
  impact_pos.zero();
  impacted_obj_id = 0;
}

afxEA_Damage::~afxEA_Damage()
{
  if (damage_data && damage_data->isTempClone())
    delete damage_data;
  damage_data = 0;
}

bool afxEA_Damage::isDone() 
{ 
  return (damage_data) ? (repeat_cnt >= damage_data->repeats) : true;
}

void afxEA_Damage::ea_set_datablock(SimDataBlock* db)
{
  damage_data = dynamic_cast<afxDamageData*>(db);
}

bool afxEA_Damage::ea_start()
{
  if (!damage_data)
  {
    Con::errorf("afxEA_Damage::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  if (damage_data->repeats > 1)
  {
    dot_delta_ms = full_lifetime/(damage_data->repeats - 1);
    next_dot_time = dot_delta_ms;
  }

  return true;
}

bool afxEA_Damage::ea_update(F32 dt)
{
  if (!started)
  {
    started = true;

    afxConstraint* pos_cons = getPosConstraint();
    if (pos_cons)
      pos_cons->getPosition(impact_pos);

    afxConstraint* aim_cons = getAimConstraint();
    if (aim_cons && aim_cons->getSceneObject())
      impacted_obj_id = aim_cons->getSceneObject()->getId();

    if (choreographer)
      choreographer->inflictDamage(damage_data->label, damage_data->flavor, impacted_obj_id, damage_data->amount, 
                                   repeat_cnt, damage_data->ad_amount, damage_data->radius, impact_pos, 
                                   damage_data->impulse);
    repeat_cnt++;
  }
  else if (repeat_cnt < damage_data->repeats)
  {
    if (next_dot_time <= life_elapsed)
    {
      // CONSTRAINT REMAPPING <<
      afxConstraint* aim_cons = getAimConstraint();
      if (aim_cons && aim_cons->getSceneObject())
        impacted_obj_id = aim_cons->getSceneObject()->getId();
      // CONSTRAINT REMAPPING >>

      if (choreographer)
        choreographer->inflictDamage(damage_data->label, damage_data->flavor, impacted_obj_id, damage_data->amount, 
                                     repeat_cnt, 0, 0, impact_pos, 0);
      next_dot_time += dot_delta_ms;
      repeat_cnt++;
    }
  }

  return true;
}

void afxEA_Damage::ea_finish(bool was_stopped)
{
  if (started && (repeat_cnt < damage_data->repeats))
  {
    if (next_dot_time <= life_elapsed)
    {
      if (choreographer)
        choreographer->inflictDamage(damage_data->label, damage_data->flavor, impacted_obj_id, damage_data->amount, 
                                     repeat_cnt, 0, 0, impact_pos, 0);
    }
  }

  started = false;
}

void afxEA_Damage::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (damage_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxDamageData* orig_db = damage_data;
    damage_data = new afxDamageData(*orig_db, true);
    orig_db->performSubstitutions(damage_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_DamageDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_DamageDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const { return false; }
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return true; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return false; }

  virtual afxEffectWrapper* create() const { return new afxEA_Damage; }
};

afxEA_DamageDesc afxEA_DamageDesc::desc;

bool afxEA_DamageDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxDamageData) == typeid(*db));
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//