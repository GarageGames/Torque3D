
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

#include "T3D/player.h"

#include "afx/afxChoreographer.h"
#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/ce/afxFootSwitch.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_FootSwitch 

class afxEA_FootSwitch : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxFootSwitchData* footfall_data;
  Player*           player;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_FootSwitch();

  void              set_overrides(Player*);
  void              clear_overrides(Player*);

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);

};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_FootSwitch::afxEA_FootSwitch()
{
  footfall_data = 0;
  player = 0;
}

inline void afxEA_FootSwitch::set_overrides(Player* player)
{
  if (footfall_data->override_all)
    player->overrideFootfallFX();
  else
    player->overrideFootfallFX(footfall_data->override_decals, 
                               footfall_data->override_sounds, 
                               footfall_data->override_dust);
}

inline void afxEA_FootSwitch::clear_overrides(Player* player)
{
  if (footfall_data->override_all)
    player->restoreFootfallFX();
  else
    player->restoreFootfallFX(footfall_data->override_decals, 
                              footfall_data->override_sounds, 
                              footfall_data->override_dust);
}

void afxEA_FootSwitch::ea_set_datablock(SimDataBlock* db)
{
  footfall_data = dynamic_cast<afxFootSwitchData*>(db);
}

bool afxEA_FootSwitch::ea_start()
{
  if (!footfall_data)
  {
    Con::errorf("afxEA_FootSwitch::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  afxConstraint* pos_cons = getPosConstraint();
  player = (pos_cons) ? dynamic_cast<Player*>(pos_cons->getSceneObject()) : 0;
  if (player)
    set_overrides(player);

  return true;
}

bool afxEA_FootSwitch::ea_update(F32 dt)
{
  if (!player)
    return true;

  afxConstraint* pos_cons = getPosConstraint();
  Player* temp_player = (pos_cons) ? dynamic_cast<Player*>(pos_cons->getSceneObject()) : 0;
  if (temp_player && temp_player != player)
  {
    player = temp_player;
    if (player)
      set_overrides(player);
  }

  return true;
}

void afxEA_FootSwitch::ea_finish(bool was_stopped)
{
  if (!player)
    return;

  afxConstraint* pos_cons = getPosConstraint();
  Player* temp_player = (pos_cons) ? dynamic_cast<Player*>(pos_cons->getSceneObject()) : 0;
  if (temp_player == player)
    clear_overrides(player);
}

void afxEA_FootSwitch::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (footfall_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxFootSwitchData* orig_db = footfall_data;
    footfall_data = new afxFootSwitchData(*orig_db, true);
    orig_db->performSubstitutions(footfall_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_FootfallSwitchDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_FootfallSwitchDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }
  virtual bool  isPositional(const afxEffectWrapperData*) const { return false; }

  virtual afxEffectWrapper* create() const { return new afxEA_FootSwitch; }
};

afxEA_FootfallSwitchDesc afxEA_FootfallSwitchDesc::desc;

bool afxEA_FootfallSwitchDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxFootSwitchData) == typeid(*db));
}

bool afxEA_FootfallSwitchDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//