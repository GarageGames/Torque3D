
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
#include "afx/afxResidueMgr.h"
#include "afx/ce/afxBillboard.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_Billboard -- This is the adapter for geometry primitives.

class afxEA_Billboard : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxBillboardData*  bb_data;
  afxBillboard*      bb;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_Billboard();
  /*D*/             ~afxEA_Billboard();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
  virtual void      ea_set_scope_status(bool flag);
  virtual void      onDeleteNotify(SimObject*);
  virtual void      getUpdatedBoxCenter(Point3F& pos);
  virtual void      getBaseColor(ColorF& color) { if (bb_data) color = bb_data->color; }
};


afxEA_Billboard::afxEA_Billboard()
{
  bb_data = 0;
  bb = 0;
}

afxEA_Billboard::~afxEA_Billboard()
{
  if (bb)
    bb->deleteObject();
  if (bb_data && bb_data->isTempClone())
    delete bb_data;
  bb_data = 0;
}

void afxEA_Billboard::ea_set_datablock(SimDataBlock* db)
{
  bb_data = dynamic_cast<afxBillboardData*>(db);
}

bool afxEA_Billboard::ea_start()
{
  if (!bb_data)
  {
    Con::errorf("afxEA_Billboard::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  return true;
}

bool afxEA_Billboard::ea_update(F32 dt)
{
  if (!bb)
  {
    // create and register effect
    bb = new afxBillboard();
    bb->onNewDataBlock(bb_data, false);
    if (!bb->registerObject())
    {
      delete bb;
      bb = 0;
      Con::errorf("afxEA_Billboard::ea_update() -- effect failed to register.");
      return false;
    }
    deleteNotify(bb);

    ///bb->setSequenceRateFactor(datablock->rate_factor/prop_time_factor);
    bb->setSortPriority(datablock->sort_priority);
  }

  if (bb)
  {
    bb->live_color = updated_color;
    if (do_fades)
    {
      bb->setFadeAmount(fade_value);
    }
    bb->setTransform(updated_xfm);
    bb->setScale(updated_scale);
  }

  return true;
}

void afxEA_Billboard::ea_finish(bool was_stopped)
{
  if (!bb)
    return;

  bb->deleteObject();
  bb = 0;
}

void afxEA_Billboard::ea_set_scope_status(bool in_scope)
{
  if (bb)
    bb->setVisibility(in_scope);
}

void afxEA_Billboard::onDeleteNotify(SimObject* obj)
{
  if (bb == dynamic_cast<afxBillboard*>(obj))
    bb = 0;

  Parent::onDeleteNotify(obj);
}

void afxEA_Billboard::getUpdatedBoxCenter(Point3F& pos)
{
  if (bb)
    pos = bb->getBoxCenter();
}

void afxEA_Billboard::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (bb_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxBillboardData* orig_db = bb_data;
    bb_data = new afxBillboardData(*orig_db, true);
    orig_db->performSubstitutions(bb_data, choreographer, group_index);
  }
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_BillboardDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_BillboardDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }

  virtual afxEffectWrapper* create() const { return new afxEA_Billboard; }
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_BillboardDesc afxEA_BillboardDesc::desc;

bool afxEA_BillboardDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxBillboardData) == typeid(*db));
}

bool afxEA_BillboardDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//