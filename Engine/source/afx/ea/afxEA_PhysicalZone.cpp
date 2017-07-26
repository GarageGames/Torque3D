
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

#include "T3D/physicalZone.h"

#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/ce/afxPhysicalZone.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_PhysicalZone 

class afxEA_PhysicalZone : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxPhysicalZoneData* zone_data;
  PhysicalZone*     physical_zone;
  SceneObject*      cons_obj;

  void              do_runtime_substitutions();
  void              set_cons_object(SceneObject*);

public:
  /*C*/             afxEA_PhysicalZone();
  /*D*/             ~afxEA_PhysicalZone();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
  virtual void      ea_set_scope_status(bool flag);
  virtual void      onDeleteNotify(SimObject*);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_PhysicalZone::afxEA_PhysicalZone()
{
  zone_data = 0;
  physical_zone = 0;
  cons_obj = 0;
}

afxEA_PhysicalZone::~afxEA_PhysicalZone()
{
  if (physical_zone)
    physical_zone->deleteObject();
  if (zone_data && zone_data->isTempClone())
    delete zone_data;
  zone_data = 0;
}

void afxEA_PhysicalZone::ea_set_datablock(SimDataBlock* db)
{
  zone_data = dynamic_cast<afxPhysicalZoneData*>(db);
}

bool afxEA_PhysicalZone::ea_start()
{
  if (!zone_data)
  {
    Con::errorf("afxEA_PhysicalZone::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  return true;
}

bool afxEA_PhysicalZone::ea_update(F32 dt)
{
  if (!physical_zone)
  {
    // create and register effect
    physical_zone = new PhysicalZone();
    physical_zone->mVelocityMod = zone_data->mVelocityMod;
    physical_zone->mGravityMod = zone_data->mGravityMod;
    physical_zone->mAppliedForce = zone_data->mAppliedForce;
    physical_zone->force_type = zone_data->force_type;
    physical_zone->orient_force = zone_data->orient_force;
    physical_zone->setField("polyhedron", zone_data->mPolyhedron);

    if (!physical_zone->registerObject())
    {
      delete physical_zone;
      physical_zone = 0;
      Con::errorf("afxEA_PhysicalZone::ea_update() -- effect failed to register.");
      return false;
    }
    deleteNotify(physical_zone);
    physical_zone->activate();
  }

  if (physical_zone)
  {
    if (zone_data->exclude_cons_obj)
    {
      afxConstraint* pos_constraint = getPosConstraint();
      set_cons_object((pos_constraint) ? pos_constraint->getSceneObject() : 0);
    }

    if (do_fades)
      physical_zone->setFadeAmount(fade_value);
    physical_zone->setTransform(updated_xfm);
  }

  return true;
}

void afxEA_PhysicalZone::ea_finish(bool was_stopped)
{
  if (physical_zone)
  {
    set_cons_object(0);
    physical_zone->deleteObject();
    physical_zone = 0;
  }
}

void afxEA_PhysicalZone::ea_set_scope_status(bool in_scope)
{
  if (physical_zone)
  {
    if (in_scope && !physical_zone->isActive())
      physical_zone->activate();
    else if (!in_scope && physical_zone->isActive())
      physical_zone->deactivate();
  }
}

void afxEA_PhysicalZone::onDeleteNotify(SimObject* obj)
{
  if (physical_zone == dynamic_cast<PhysicalZone*>(obj))
    physical_zone = 0;

  Parent::onDeleteNotify(obj);
}

void afxEA_PhysicalZone::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (zone_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxPhysicalZoneData* orig_db = zone_data;
    zone_data = new afxPhysicalZoneData(*orig_db, true);
    orig_db->performSubstitutions(zone_data, choreographer, group_index);
  }
}

void afxEA_PhysicalZone::set_cons_object(SceneObject* new_obj)
{
  if (cons_obj == new_obj)
    return;

  if (cons_obj)
  {
    physical_zone->unregisterExcludedObject(cons_obj);
    //clearNotify(shape);
  }
  
  cons_obj = new_obj;

  if (cons_obj)
  {
    //deleteNotify(shape);
    physical_zone->registerExcludedObject(cons_obj);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_PhysicalZoneDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_PhysicalZoneDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return true; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return false; }

  virtual afxEffectWrapper* create() const { return new afxEA_PhysicalZone; }
};

afxEA_PhysicalZoneDesc afxEA_PhysicalZoneDesc::desc;

bool afxEA_PhysicalZoneDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxPhysicalZoneData) == typeid(*db));
}

bool afxEA_PhysicalZoneDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//