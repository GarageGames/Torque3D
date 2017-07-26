
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

#include "gui/core/guiControl.h"
#include "gui/3d/guiTSControl.h"
#include "T3D/gameBase/gameConnection.h"
#include "gui/game/guiProgressCtrl.h"

#include "afx/afxChoreographer.h"
#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/ce/afxGuiController.h"
#include "afx/ui/afxProgressBase.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_GuiController 

class afxEA_GuiController : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxGuiControllerData* controller_data;
  GuiControl*           gui_control;
  GuiTSCtrl*            ts_ctrl;
  afxProgressBase*      progress_base;
  GuiProgressCtrl*      progress_ctrl;

  void                  do_runtime_substitutions();

public:
  /*C*/           afxEA_GuiController();

  virtual void    ea_set_datablock(SimDataBlock*);
  virtual bool    ea_start();
  virtual bool    ea_update(F32 dt);
  virtual void    ea_finish(bool was_stopped);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_GuiController::afxEA_GuiController()
{
  controller_data = 0;
  gui_control = 0;
  ts_ctrl = 0;
  progress_base = 0;
  progress_ctrl = 0;
}

void afxEA_GuiController::ea_set_datablock(SimDataBlock* db)
{
  controller_data = dynamic_cast<afxGuiControllerData*>(db);
}

bool afxEA_GuiController::ea_start()
{
  if (!controller_data)
  {
    Con::errorf("afxEA_GuiController::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  if (controller_data->ctrl_client_only)
  {
    afxConstraint* pos_cons = getPosConstraint();
    if (!pos_cons)
      return false; // not an error condition
    GameBase* gamebase_obj = dynamic_cast<GameBase*>(pos_cons->getSceneObject());
    if (!gamebase_obj || !gamebase_obj->getControllingClient())
      return false; // not an error condition
  }

  if (controller_data->control_name == ST_NULLSTRING || controller_data->control_name[0] == '\0')
  {
    Con::errorf("afxEA_GuiController::ea_start() -- empty control name.");
    return false;
  }

  gui_control = dynamic_cast<GuiControl*>(Sim::findObject(controller_data->control_name));
  if (!gui_control)
  {
    Con::errorf("afxEA_GuiController::ea_start() -- failed to find control \"%s\".", controller_data->control_name);
    return false;
  }

  gui_control->setVisible(true);

  progress_base = dynamic_cast<afxProgressBase*>(gui_control);
  if (progress_base)
  {
    progress_base->setProgress(0.0f);
    progress_ctrl = 0;
  }
  else
  {
    progress_ctrl = (GuiProgressCtrl*)gui_control;
    if (progress_ctrl)
    {
      progress_ctrl->setScriptValue(0);
      progress_base = 0;
    }
  }

  ts_ctrl = 0;
  for (GuiControl* ctrl = gui_control->getParent(); ctrl != 0; ctrl->getParent())
  {
    if (dynamic_cast<GuiTSCtrl*>(ctrl))
    {
      ts_ctrl = (GuiTSCtrl*) ctrl;
      break;
    }
  }

  return true;
}

bool afxEA_GuiController::ea_update(F32 dt)
{
  if (ts_ctrl && !controller_data->preserve_pos)
  {
    Point3F screen_pos;
    if (ts_ctrl->project(updated_pos, &screen_pos))
    {
      const Point2I ext = gui_control->getExtent();
      Point2I newpos(screen_pos.x - ext.x/2, screen_pos.y - ext.y/2);
      gui_control->setPosition(newpos);
    }
  }

  if (progress_base)
    progress_base->setProgress((ew_timing.lifetime > 0.0) ? life_elapsed/ew_timing.lifetime : 0.0f);
  else if (progress_ctrl)
    progress_ctrl->setScriptValue((ew_timing.lifetime > 0.0) ? avar("%g", life_elapsed/ew_timing.lifetime) : 0);

  if (do_fades)
    gui_control->setFadeAmount(fade_value);

  return true;
}

void afxEA_GuiController::ea_finish(bool was_stopped)
{
  if (progress_base)
    progress_base->setProgress(1.0f);
  else if (progress_ctrl)
    progress_ctrl->setScriptValue("1");
  gui_control->setVisible(false);
}

void afxEA_GuiController::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (controller_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxGuiControllerData* orig_db = controller_data;
    controller_data = new afxGuiControllerData(*orig_db, true);
    orig_db->performSubstitutions(controller_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_GuiControllerDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_GuiControllerDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }

  virtual afxEffectWrapper* create() const { return new afxEA_GuiController; }
};

afxEA_GuiControllerDesc afxEA_GuiControllerDesc::desc;

bool afxEA_GuiControllerDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxGuiControllerData) == typeid(*db));
}

bool afxEA_GuiControllerDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//