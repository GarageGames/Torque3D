
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
#include "afx/ce/afxStaticShape.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_StaticShape 

class afxEA_StaticShape : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  StaticShapeData*  shape_data;
  afxStaticShape*   static_shape;
  bool              fade_out_started;
  bool              do_spawn;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_StaticShape();
  /*D*/             ~afxEA_StaticShape();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
  virtual void      ea_set_scope_status(bool flag);
  virtual void      onDeleteNotify(SimObject*);

  virtual void              getUpdatedBoxCenter(Point3F& pos);
  virtual TSShape*          getTSShape();
  virtual TSShapeInstance*  getTSShapeInstance();
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_StaticShape::afxEA_StaticShape()
{
  shape_data = 0;
  static_shape = 0;
  fade_out_started = false;
  do_spawn = true;
}

afxEA_StaticShape::~afxEA_StaticShape()
{
  if (static_shape)
    static_shape->deleteObject();
  if (shape_data && shape_data->isTempClone())
    delete shape_data;
  shape_data = 0;
}

void afxEA_StaticShape::ea_set_datablock(SimDataBlock* db)
{
  shape_data = dynamic_cast<StaticShapeData*>(db);
  afxStaticShapeData* afx_shape_data =  dynamic_cast<afxStaticShapeData*>(shape_data);
  do_spawn = (afx_shape_data) ? afx_shape_data->do_spawn : false;
}

bool afxEA_StaticShape::ea_start()
{
  if (!shape_data)
  {
    Con::errorf("afxEA_StaticShape::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  // fades are handled using startFade() calls.
  do_fades = false;

  return true;
}

bool afxEA_StaticShape::ea_update(F32 dt)
{
  if (!static_shape)
  {
    // create and register effect
    static_shape = new afxStaticShape();
    if (datablock->use_ghost_as_cons_obj && datablock->effect_name != ST_NULLSTRING)
      static_shape->init(choreographer->getChoreographerId(), datablock->effect_name);

    static_shape->onNewDataBlock(shape_data, false);
    if (!static_shape->registerObject())
    {
      delete static_shape;
      static_shape = 0;
      Con::errorf("afxEA_StaticShape::ea_update() -- effect failed to register.");
      return false;
    }
    deleteNotify(static_shape);
    registerForCleanup(static_shape);

    if (ew_timing.fade_in_time > 0.0f)
      static_shape->startFade(ew_timing.fade_in_time, 0, false);
  }

  if (static_shape)
  {
    if (!fade_out_started && elapsed > fade_out_start)
    {
      if (!do_spawn)
      {
        if (ew_timing.fade_out_time > 0.0f)
          static_shape->startFade(ew_timing.fade_out_time, 0, true);
      }
      fade_out_started = true;
    }

    if (in_scope)
    {
      static_shape->setTransform(updated_xfm);
      static_shape->setScale(updated_scale);
    }
  }

  return true;
}

void afxEA_StaticShape::ea_finish(bool was_stopped)
{
  if (!static_shape)
    return;
  
  if (do_spawn)
  {
    Con::executef(shape_data, "onSpawn", static_shape->getIdString(), datablock->effect_name);
    clearNotify(static_shape);
  }
  else
    static_shape->deleteObject();

  static_shape = 0;
}

void afxEA_StaticShape::ea_set_scope_status(bool in_scope)
{
  if (static_shape)
    static_shape->setVisibility(do_spawn || in_scope);
}

void afxEA_StaticShape::onDeleteNotify(SimObject* obj)
{
  if (static_shape == dynamic_cast<afxStaticShape*>(obj))
    static_shape = 0;

  Parent::onDeleteNotify(obj);
}

void afxEA_StaticShape::getUpdatedBoxCenter(Point3F& pos)
{
  if (static_shape)
    pos = static_shape->getBoxCenter();
}


TSShape* afxEA_StaticShape::getTSShape()
{
  return (static_shape) ? ((TSShape*)static_shape->getShape()) : 0;
}

TSShapeInstance* afxEA_StaticShape::getTSShapeInstance()
{
  return (static_shape) ? static_shape->getShapeInstance() : 0;
}

void afxEA_StaticShape::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (shape_data->getSubstitutionCount() > 0)
  {
    if (typeid(afxStaticShapeData) == typeid(*shape_data))
    {
      afxStaticShapeData* orig_db = (afxStaticShapeData*)shape_data;
      shape_data = new afxStaticShapeData(*orig_db, true);
      orig_db->performSubstitutions(shape_data, choreographer, group_index);
    }
    else
    {
      StaticShapeData* orig_db = shape_data;
      shape_data = new StaticShapeData(*orig_db, true);
      orig_db->performSubstitutions(shape_data, choreographer, group_index);
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_StaticShapeDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_StaticShapeDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return true; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return false; }

  virtual afxEffectWrapper* create() const { return new afxEA_StaticShape; }
};

afxEA_StaticShapeDesc afxEA_StaticShapeDesc::desc;

bool afxEA_StaticShapeDesc::testEffectType(const SimDataBlock* db) const
{
  if (typeid(StaticShapeData) == typeid(*db))
     return true;
  if (typeid(afxStaticShapeData) == typeid(*db))
     return true;
  return false;
}

bool afxEA_StaticShapeDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//