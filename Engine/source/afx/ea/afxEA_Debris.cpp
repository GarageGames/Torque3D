
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

#include "T3D/debris.h"

#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_Debris 

class afxEA_Debris : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  DebrisData*       debris_data;
  Debris*           debris;
  bool              exploded;
  bool              debris_done;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_Debris();
  /*D*/             ~afxEA_Debris();

  virtual bool      isDone();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);

  virtual void      onDeleteNotify(SimObject*);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_Debris::afxEA_Debris()
{
  debris_data = 0;
  debris = 0;
  exploded = false;
  debris_done = false;
}

afxEA_Debris::~afxEA_Debris()
{
  if (debris)
    clearNotify(debris);
}

bool afxEA_Debris::isDone()
{
  return (datablock->use_as_cons_obj) ? debris_done : exploded;
}

void afxEA_Debris::ea_set_datablock(SimDataBlock* db)
{
  debris_data = dynamic_cast<DebrisData*>(db);
}

bool afxEA_Debris::ea_start()
{
  if (!debris_data)
  {
    Con::errorf("afxEA_Debris::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  debris = new Debris();
  debris->onNewDataBlock(debris_data, false);

  return true;
}

bool afxEA_Debris::ea_update(F32 dt)
{
  if (exploded && debris)
  {
    if (in_scope)
    {
      updated_xfm = debris->getRenderTransform();
      updated_xfm.getColumn(3, &updated_pos);
    }
  }

  if (!exploded && debris)
  {
    if (in_scope)
    {     
      Point3F dir_vec(0,1,0);
      updated_xfm.mulV(dir_vec);

      debris->init(updated_pos, dir_vec);
      if (!debris->registerObject())
      {
        delete debris;
        debris = 0;
        Con::errorf("afxEA_Debris::ea_update() -- effect failed to register.");
        return false;
      }
      deleteNotify(debris);
    }
    exploded = true;
  }

  return true;
}

void afxEA_Debris::ea_finish(bool was_stopped)
{
  if (debris)
  {
    clearNotify(debris);
    debris = 0;
  }
  exploded = false;
}

void afxEA_Debris::onDeleteNotify(SimObject* obj)
{
  // debris deleted?
  Debris* del_debris = dynamic_cast<Debris*>(obj);
  if (del_debris == debris)
  {
    debris = NULL;
    debris_done = true;
  }
}

void afxEA_Debris::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (debris_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    DebrisData* orig_db = debris_data;
    debris_data = new DebrisData(*orig_db, true);
    orig_db->performSubstitutions(debris_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_DebrisDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_DebrisDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }

  virtual afxEffectWrapper* create() const { return new afxEA_Debris; }
};

afxEA_DebrisDesc afxEA_DebrisDesc::desc;

bool afxEA_DebrisDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(DebrisData) == typeid(*db));
}

bool afxEA_DebrisDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (ew->use_as_cons_obj && timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//