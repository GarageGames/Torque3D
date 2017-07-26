
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
#include "afx/ce/afxCollisionEvent.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_CollisionEvent 

class afxEA_CollisionEvent : public afxEffectWrapper, ShapeBase::CollisionEventCallback
{
  typedef afxEffectWrapper Parent;

  afxCollisionEventData*  script_data;
  ShapeBase*              shape;
  U32                     trigger_mask;
  bool                    triggered;

  void              do_runtime_substitutions();
  void              set_shape(ShapeBase*);

public:
  /*C*/             afxEA_CollisionEvent();
  /*D*/             ~afxEA_CollisionEvent();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);

  virtual void      collisionNotify(SceneObject* obj0, SceneObject* obj1, const VectorF& vel);
  virtual void      onDeleteNotify(SimObject*);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_CollisionEvent::afxEA_CollisionEvent()
{
  script_data = 0;
  shape = 0;
  trigger_mask = 0;
  triggered = false;
}

afxEA_CollisionEvent::~afxEA_CollisionEvent()
{
  if (shape)
    clearNotify(shape);
  if (script_data && script_data->isTempClone())
    delete script_data;
  script_data = 0;
}

void afxEA_CollisionEvent::ea_set_datablock(SimDataBlock* db)
{
  script_data = dynamic_cast<afxCollisionEventData*>(db);
}

bool afxEA_CollisionEvent::ea_start()
{
  if (!script_data)
  {
    Con::errorf("afxEA_CollisionEvent::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  if (script_data->gen_trigger && script_data->trigger_bit < 32)
    trigger_mask = 1 << script_data->trigger_bit;
  else
    trigger_mask = 0;

  triggered = false;

  return true;
}

bool afxEA_CollisionEvent::ea_update(F32 dt)
{
  afxConstraint* pos_constraint = getPosConstraint();
  set_shape((pos_constraint) ? dynamic_cast<ShapeBase*>(pos_constraint->getSceneObject()) : 0);

  if (choreographer && trigger_mask != 0)
  {
    if (triggered)
    {
      choreographer->setTriggerMask(trigger_mask | choreographer->getTriggerMask());
      triggered = false;
    }
    else
    {
      choreographer->setTriggerMask(~trigger_mask & choreographer->getTriggerMask());
    }
  }

  return true;
}

void afxEA_CollisionEvent::ea_finish(bool was_stopped)
{
  set_shape(0);
}

void afxEA_CollisionEvent::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (script_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxCollisionEventData* orig_db = script_data;
    script_data = new afxCollisionEventData(*orig_db, true);
    orig_db->performSubstitutions(script_data, choreographer, group_index);
  }
}

void afxEA_CollisionEvent::set_shape(ShapeBase* new_shape)
{
  if (shape == new_shape)
    return;

  if (shape)
  {
    shape->unregisterCollisionCallback(this);
    clearNotify(shape);
  }
  
  shape = new_shape;

  if (shape)
  {
   deleteNotify(shape);
   shape->registerCollisionCallback(this);
  }
}

void afxEA_CollisionEvent::collisionNotify(SceneObject* obj0, SceneObject* obj1, const VectorF& vel)
{
  if (obj0 != shape || !choreographer || !choreographer->getDataBlock())
    return;

  if (script_data->method_name != ST_NULLSTRING)
  {
    char *arg_buf = Con::getArgBuffer(64);
    dSprintf(arg_buf, 256, "%g %g %g", vel.x, vel.y, vel.z);

    // CALL SCRIPT afxChoreographerData::method(%spell, %obj0, %obj1, %velocity)
    Con::executef(choreographer->getDataBlock(), script_data->method_name, 
                  choreographer->getIdString(),
                  (obj0) ? obj0->getIdString() : "", 
                  (obj1) ? obj1->getIdString() : "",
                  arg_buf,
                  script_data->script_data);
  }

  if (!triggered && trigger_mask != 0)
    triggered = true;
}

void afxEA_CollisionEvent::onDeleteNotify(SimObject* obj)
{
  if (obj == shape)
  {
    shape->unregisterCollisionCallback(this);
    shape = 0;
  }

  Parent::onDeleteNotify(obj);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_CollisionEventDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_CollisionEventDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return true; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return false; }
  virtual bool  isPositional(const afxEffectWrapperData*) const { return false; }

  virtual afxEffectWrapper* create() const { return new afxEA_CollisionEvent; }
};

afxEA_CollisionEventDesc afxEA_CollisionEventDesc::desc;

bool afxEA_CollisionEventDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxCollisionEventData) == typeid(*db));
}

bool afxEA_CollisionEventDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//