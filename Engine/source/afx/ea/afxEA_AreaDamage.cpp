
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
#include "afx/ce/afxAreaDamage.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_AreaDamage 

class afxEA_AreaDamage : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxAreaDamageData*  damage_data;
  Point3F             impact_pos;
  bool                damage_is_done;
  SceneObject*        cons_obj;

  void                do_runtime_substitutions();
  void                deal_area_damage();
  void                apply_damage(ShapeBase*, F32 damage, const char* flavor, Point3F& pos);
  void                apply_impulse(ShapeBase*, F32 impulse, Point3F& pos);
  void                notify_damage_source(ShapeBase* damaged, F32 damage, const char* flavor, Point3F& pos);

public:
  /*C*/               afxEA_AreaDamage();
  /*C*/               ~afxEA_AreaDamage();

  virtual bool        isDone();

  virtual void        ea_set_datablock(SimDataBlock*);
  virtual bool        ea_start();
  virtual bool        ea_update(F32 dt);
  virtual void        ea_finish(bool was_stopped);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_AreaDamage::afxEA_AreaDamage()
{
  damage_data = 0;
  impact_pos.zero();
  damage_is_done = false;
  cons_obj = 0;
}

afxEA_AreaDamage::~afxEA_AreaDamage()
{
  if (damage_data && damage_data->isTempClone())
    delete damage_data;
  damage_data = 0;
}

bool afxEA_AreaDamage::isDone() 
{ 
  return damage_is_done;
}

void afxEA_AreaDamage::ea_set_datablock(SimDataBlock* db)
{
  damage_data = dynamic_cast<afxAreaDamageData*>(db);
}

bool afxEA_AreaDamage::ea_start()
{
  if (!damage_data)
  {
    Con::errorf("afxEA_AreaDamage::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  return true;
}

bool afxEA_AreaDamage::ea_update(F32 dt)
{
  if (!damage_is_done)
  {
    afxConstraint* pos_cons = getPosConstraint();
    if (pos_cons)
    {
      pos_cons->getPosition(impact_pos);
      cons_obj = pos_cons->getSceneObject();
    }

    deal_area_damage();

    damage_is_done = true;
  }

  return true;
}

void afxEA_AreaDamage::ea_finish(bool was_stopped)
{
  damage_is_done = false;
}

void afxEA_AreaDamage::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (damage_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxAreaDamageData* orig_db = damage_data;
    damage_data = new afxAreaDamageData(*orig_db, true);
    orig_db->performSubstitutions(damage_data, choreographer, group_index);
  }
}

// radiusDamage(%sourceObject, %position, %radius, %damage, %damageType, %impulse, %excluded)

void afxEA_AreaDamage::deal_area_damage()
{ 
  // initContainerRadiusSearch -- afterwards Container::mSearchList contains objects within radius sorted by distance
  gServerContainer.initRadiusSearch(impact_pos, damage_data->radius, ShapeBaseObjectType);
  
  F32 halfradius = damage_data->radius*0.5f;

  const Vector<SimObjectPtr<SceneObject>*>& list = gServerContainer.getRadiusSearchList();
  for (S32 i = 0; i < list.size(); i++)
  {
    if (!list[i]->isNull())
    {
      ShapeBase* shape = dynamic_cast<ShapeBase*>((SceneObject*)(*list[i]));
      if (!shape || (shape->getTypeMask() & CameraObjectType))
        continue;

      if (damage_data->exclude_cons_obj && cons_obj == *list[i])
        continue;

#if 0 // AFX_T3D_DISABLED -- calcExplosionCoverage() is a script function
      // so we currently assign a coverage value of 1.0.

      U32 mask = InteriorObjectType | TerrainObjectType | VehicleObjectType;
      F32 coverage = calcExplosionCoverage(impact_pos, shape, mask);
      if (coverage == 0.0f)
        continue;
#else
      F32 coverage = 1.0f;
#endif

      // calulate distance
      Point3F pos;
      shape->getWorldBox().getCenter(&pos);
      F32 dist = (pos - impact_pos).len();

      F32 min_dist = shape->getWorldBox().len_x();
      if (shape->getWorldBox().len_y() < min_dist)
        min_dist = shape->getWorldBox().len_y();
      if (shape->getWorldBox().len_z() < min_dist)
        min_dist = shape->getWorldBox().len_z();

      dist -= min_dist;
      if (dist < 0)
        dist = 0;

      F32 dist_scale = (dist < halfradius) ? 1.0f : 1.0f - ((dist - halfradius)/halfradius);

      F32 damage = damage_data->amount*coverage*dist_scale;
      apply_damage(shape, damage, damage_data->flavor, impact_pos);
      apply_impulse(shape, damage_data->impulse*dist_scale, impact_pos);

      if (damage_data->notify_damage_src)
        notify_damage_source(shape, damage, damage_data->flavor, impact_pos);
    }
  }
}

void afxEA_AreaDamage::notify_damage_source(ShapeBase* damaged, F32 damage, const char* flavor, Point3F& pos)
{
  if (mIsZero(damage))
    return;

  char *posArg = Con::getArgBuffer(64);
  dSprintf(posArg, 64, "%f %f %f", pos.x, pos.y, pos.z);

  Con::executef(choreographer->getDataBlock(), "onInflictedAreaDamage", 
                choreographer->getIdString(),
                damaged->getIdString(),
                Con::getFloatArg(damage),
                flavor,
                posArg);
}

void afxEA_AreaDamage::apply_damage(ShapeBase* shape, F32 damage, const char* flavor, Point3F& pos)
{
  if (mIsZero(damage))
    return;

  char *posArg = Con::getArgBuffer(64);
  dSprintf(posArg, 64, "%f %f %f", pos.x, pos.y, pos.z);

  Con::executef(shape, "damage", 
                choreographer->getIdString(),
                posArg,
                Con::getFloatArg(damage), 
                flavor);
}

void afxEA_AreaDamage::apply_impulse(ShapeBase* shape, F32 impulse, Point3F& pos)
{
  if (impulse <= 0.0f)
    return;

  Point3F center; shape->getWorldBox().getCenter(&center);
  VectorF impulse_vec = center - pos;
  impulse_vec.normalizeSafe();
  impulse_vec *= impulse;
  shape->applyImpulse(pos, impulse_vec); 
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_AreaDamageDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_AreaDamageDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const { return false; }
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return true; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return false; }

  virtual afxEffectWrapper* create() const { return new afxEA_AreaDamage; }
};

afxEA_AreaDamageDesc afxEA_AreaDamageDesc::desc;

bool afxEA_AreaDamageDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxAreaDamageData) == typeid(*db));
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//