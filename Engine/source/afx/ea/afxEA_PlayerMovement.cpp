
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
#include "afx/ce/afxPlayerMovement.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_PlayerMovement 

class afxEA_PlayerMovement : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxPlayerMovementData*  movement_data;
  U32                     tag;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_PlayerMovement();
  /*C*/             ~afxEA_PlayerMovement();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_PlayerMovement::afxEA_PlayerMovement()
{
  movement_data = 0;
  tag = 0;
}

afxEA_PlayerMovement::~afxEA_PlayerMovement()
{
  if (movement_data && movement_data->isTempClone())
    delete movement_data;
  movement_data = 0;
}

void afxEA_PlayerMovement::ea_set_datablock(SimDataBlock* db)
{
  movement_data = dynamic_cast<afxPlayerMovementData*>(db);
}

bool afxEA_PlayerMovement::ea_start()
{
  if (!movement_data)
  {
    Con::errorf("afxEA_PlayerMovement::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();
  tag = 0;

  afxConstraint* pos_cons = getPosConstraint();
  if (!pos_cons)
  {
    Con::warnf("afxEA_PlayerMovement::ea_start() -- missing position constraint.");
    return false;
  }

  Player* player = dynamic_cast<Player*>(pos_cons->getSceneObject());
  if (!player)
  {
    Con::warnf("afxEA_PlayerMovement::ea_start() -- position constraint is not a Player.");
    return false;
  }

  // setup player overrides
  if (movement_data->hasMovementOverride())
    tag = player->setMovementOverride(movement_data->speed_bias, &movement_data->movement, movement_data->movement_op);
  else
    tag = player->setMovementOverride(movement_data->speed_bias);

  return true;
}

bool afxEA_PlayerMovement::ea_update(F32 dt)
{
  return true;
}

void afxEA_PlayerMovement::ea_finish(bool was_stopped)
{
  afxConstraint* pos_cons = getPosConstraint();
  if (!pos_cons)
    return;

  Player* player = dynamic_cast<Player*>(pos_cons->getSceneObject());
  if (!player)
    return;

  // restore player overrides
  player->restoreMovement(tag);
}

void afxEA_PlayerMovement::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (movement_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxPlayerMovementData* orig_db = movement_data;
    movement_data = new afxPlayerMovementData(*orig_db, true);
    orig_db->performSubstitutions(movement_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_PlayerMovementDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_PlayerMovementDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return true; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return false; }

  virtual afxEffectWrapper* create() const { return new afxEA_PlayerMovement; }
};

afxEA_PlayerMovementDesc afxEA_PlayerMovementDesc::desc;

bool afxEA_PlayerMovementDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxPlayerMovementData) == typeid(*db));
}

bool afxEA_PlayerMovementDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//