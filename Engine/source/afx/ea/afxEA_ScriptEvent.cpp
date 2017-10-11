
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
#include "afx/ce/afxScriptEvent.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_ScriptEvent 

class afxEA_ScriptEvent : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxScriptEventData* script_data;
  bool              ran_script;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_ScriptEvent();
  /*D*/             ~afxEA_ScriptEvent();

  virtual bool      isDone() { return ran_script; }

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_ScriptEvent::afxEA_ScriptEvent()
{
  script_data = 0;
  ran_script = false;
}

afxEA_ScriptEvent::~afxEA_ScriptEvent()
{
  if (script_data && script_data->isTempClone())
    delete script_data;
  script_data = 0;
}

void afxEA_ScriptEvent::ea_set_datablock(SimDataBlock* db)
{
  script_data = dynamic_cast<afxScriptEventData*>(db);
}

bool afxEA_ScriptEvent::ea_start()
{
  if (!script_data)
  {
    Con::errorf("afxEA_ScriptEvent::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  ran_script = (script_data->method_name == ST_NULLSTRING);

  return true;
}

bool afxEA_ScriptEvent::ea_update(F32 dt)
{
  if (!ran_script && choreographer != NULL)
  {
    afxConstraint* pos_constraint = getPosConstraint();
    choreographer->executeScriptEvent(script_data->method_name, pos_constraint, updated_xfm, 
                                      script_data->script_data);
    ran_script = true;
  }

  return true;
}

void afxEA_ScriptEvent::ea_finish(bool was_stopped)
{
  ran_script = false;
}

void afxEA_ScriptEvent::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (script_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxScriptEventData* orig_db = script_data;
    script_data = new afxScriptEventData(*orig_db, true);
    orig_db->performSubstitutions(script_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_ScriptEventDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_ScriptEventDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const { return false; }
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return true; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return false; }
  virtual bool  isPositional(const afxEffectWrapperData*) const { return false; }

  virtual afxEffectWrapper* create() const { return new afxEA_ScriptEvent; }
};

afxEA_ScriptEventDesc afxEA_ScriptEventDesc::desc;

bool afxEA_ScriptEventDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxScriptEventData) == typeid(*db));
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//