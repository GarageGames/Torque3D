
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
#include "afx/ce/afxMooring.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_Mooring 

class afxEA_Mooring : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxMooringData*   mooring_data;
  afxMooring*       obj;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_Mooring();
  /*D*/             ~afxEA_Mooring();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
  virtual void      onDeleteNotify(SimObject*);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_Mooring::afxEA_Mooring()
{
  mooring_data = 0;
  obj = 0;
}

afxEA_Mooring::~afxEA_Mooring()
{
  if (obj)
    obj->deleteObject();
  if (mooring_data && mooring_data->isTempClone())
    delete mooring_data;
  mooring_data = 0;
}

void afxEA_Mooring::ea_set_datablock(SimDataBlock* db)
{
  mooring_data = dynamic_cast<afxMooringData*>(db);
}

bool afxEA_Mooring::ea_start()
{
  if (!mooring_data)
  {
    Con::errorf("afxEA_Mooring::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  return true;
}

bool afxEA_Mooring::ea_update(F32 dt)
{
  if (!obj)
  {
    if (datablock->use_ghost_as_cons_obj && datablock->effect_name != ST_NULLSTRING)
    {
      obj = new afxMooring(mooring_data->networking, 
                           choreographer->getChoreographerId(), 
                           datablock->effect_name);
    }
    else
    {
      obj = new afxMooring(mooring_data->networking, 0, ST_NULLSTRING);
    }

    obj->onNewDataBlock(mooring_data, false);
    if (!obj->registerObject())
    {
      delete obj;
      obj = 0;
      Con::errorf("afxEA_Mooring::ea_update() -- effect failed to register.");
      return false;
    }
    deleteNotify(obj);
  }

  if (obj)
  {
    obj->setTransform(updated_xfm);
  }

  return true;
}

void afxEA_Mooring::ea_finish(bool was_stopped)
{
}

void afxEA_Mooring::onDeleteNotify(SimObject* obj)
{
  if (this->obj == obj)
    obj = 0;

  Parent::onDeleteNotify(obj);
}

void afxEA_Mooring::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (mooring_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxMooringData* orig_db = mooring_data;
    mooring_data = new afxMooringData(*orig_db, true);
    orig_db->performSubstitutions(mooring_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_MooringDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_MooringDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const;
  virtual bool  runsOnClient(const afxEffectWrapperData*) const;

  virtual afxEffectWrapper* create() const { return new afxEA_Mooring; }
};

afxEA_MooringDesc afxEA_MooringDesc::desc;

bool afxEA_MooringDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxMooringData) == typeid(*db));
}

bool afxEA_MooringDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

bool afxEA_MooringDesc::runsOnServer(const afxEffectWrapperData* ew) const
{
  U8 networking = ((const afxMooringData*)ew->effect_data)->networking;
  return ((networking & CLIENT_ONLY) == 0);
}

bool afxEA_MooringDesc::runsOnClient(const afxEffectWrapperData* ew) const
{ 
  U8 networking = ((const afxMooringData*)ew->effect_data)->networking;
  return ((networking & CLIENT_ONLY) != 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//