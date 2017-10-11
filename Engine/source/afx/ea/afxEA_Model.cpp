
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

#include "ts/tsShapeInstance.h"

#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/afxResidueMgr.h"
#include "afx/ce/afxModel.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_Model -- This is the adapter for afxModel, a lightweight animated model effect.

class afxEA_Model : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxModelData*     model_data;
  afxModel*         model;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_Model();
  /*D*/             ~afxEA_Model();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
  virtual void      ea_set_scope_status(bool flag);
  virtual void      onDeleteNotify(SimObject*);

  virtual void      getUpdatedBoxCenter(Point3F& pos);

  virtual TSShape*          getTSShape();
  virtual TSShapeInstance*  getTSShapeInstance();
  virtual SceneObject*      ea_get_scene_object() const;
  virtual U32               ea_get_triggers() const;

  virtual U32       setAnimClip(const char* clip, F32 pos, F32 rate, F32 trans);
  virtual void      resetAnimation(U32 tag);
  virtual F32       getAnimClipDuration(const char* clip);
};


afxEA_Model::afxEA_Model()
{
  model_data = 0;
  model = 0;
}

afxEA_Model::~afxEA_Model()
{
  if (model)
    model->deleteObject();
  if (model_data && model_data->isTempClone())
    delete model_data;
  model_data = 0;
}

void afxEA_Model::ea_set_datablock(SimDataBlock* db)
{
  model_data = dynamic_cast<afxModelData*>(db);
}

bool afxEA_Model::ea_start()
{
  if (!model_data)
  {
    Con::errorf("afxEA_Model::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  return true;
}

bool afxEA_Model::ea_update(F32 dt)
{
  if (!model)
  {
    // create and register effect
    model = new afxModel();
    model->onNewDataBlock(model_data, false);
    if (!model->registerObject())
    {
      delete model;
      model = 0;
      Con::errorf("afxEA_Model::ea_update() -- effect failed to register.");
      return false;
    }
    deleteNotify(model);

    model->setSequenceRateFactor(datablock->rate_factor/prop_time_factor);
    model->setSortPriority(datablock->sort_priority);
  }

  if (model)
  {
    if (do_fades)
    {
      model->setFadeAmount(fade_value);
    }
    model->setTransform(updated_xfm);
    model->setScale(updated_scale);
  }

  return true;
}

void afxEA_Model::ea_finish(bool was_stopped)
{
  if (!model)
    return;
  
  if (in_scope && ew_timing.residue_lifetime > 0)
  {
    clearNotify(model);
    afxResidueMgr::add(ew_timing.residue_lifetime, ew_timing.residue_fadetime, model);
    model = 0;
  }
  else
  {
    model->deleteObject();
    model = 0;
  }
}

void afxEA_Model::ea_set_scope_status(bool in_scope)
{
  if (model)
    model->setVisibility(in_scope);
}

void afxEA_Model::onDeleteNotify(SimObject* obj)
{
  if (model == dynamic_cast<afxModel*>(obj))
    model = 0;

  Parent::onDeleteNotify(obj);
}

void afxEA_Model::getUpdatedBoxCenter(Point3F& pos)
{
  if (model)
    pos = model->getBoxCenter();
}

TSShape* afxEA_Model::getTSShape()
{
  return (model) ? model->getTSShape() : 0;
}

TSShapeInstance* afxEA_Model::getTSShapeInstance()
{
  return (model) ? model->getTSShapeInstance() : 0;
}

SceneObject* afxEA_Model::ea_get_scene_object() const
{
  return model;
}

U32 afxEA_Model::ea_get_triggers() const
{
  TSShapeInstance* shape_inst = model->getTSShapeInstance();
  return (shape_inst) ? shape_inst->getTriggerStateMask() : 0;
}

void afxEA_Model::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (model_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxModelData* orig_db = model_data;
    model_data = new afxModelData(*orig_db, true);
    orig_db->performSubstitutions(model_data, choreographer, group_index);
  }
}

U32 afxEA_Model::setAnimClip(const char* clip, F32 pos, F32 rate, F32 trans)
{
  return (model) ? model->setAnimClip(clip, pos, rate, trans) : 0;
}

void afxEA_Model::resetAnimation(U32 tag)
{
  if (model)
    model->resetAnimation(tag);
}

F32 afxEA_Model::getAnimClipDuration(const char* clip)
{
  return (model) ? model->getAnimClipDuration(clip) : 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_ModelDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_ModelDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }

  virtual afxEffectWrapper* create() const { return new afxEA_Model; }
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_ModelDesc afxEA_ModelDesc::desc;

bool afxEA_ModelDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxModelData) == typeid(*db));
}

bool afxEA_ModelDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//