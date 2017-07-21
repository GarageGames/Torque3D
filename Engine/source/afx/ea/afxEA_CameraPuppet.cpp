
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

#include "T3D/gameBase/gameConnection.h"

#include "afx/afxChoreographer.h"
#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxCamera.h"
#include "afx/ce/afxCameraPuppet.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_CameraPuppet 

class afxEA_CameraPuppet : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxCameraPuppetData* puppet_data;
  afxConstraint*    cam_cons;
  bool              was_1st_person;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_CameraPuppet();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);

  virtual void      getUnconstrainedPosition(Point3F& pos);
  virtual void      getUnconstrainedTransform(MatrixF& xfm);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_CameraPuppet::afxEA_CameraPuppet()
{
  puppet_data = 0;
  cam_cons = 0;
  was_1st_person = false;
}

void afxEA_CameraPuppet::ea_set_datablock(SimDataBlock* db)
{
  puppet_data = dynamic_cast<afxCameraPuppetData*>(db);
}

bool afxEA_CameraPuppet::ea_start()
{
  if (!puppet_data)
  {
    Con::errorf("afxEA_CameraPuppet::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  afxConstraintID obj_id = cons_mgr->getConstraintId(puppet_data->cam_def);
  cam_cons = cons_mgr->getConstraint(obj_id);

  SceneObject* obj = (cam_cons) ? cam_cons->getSceneObject() : 0;
  if (obj && obj->isClientObject())
  {
    GameConnection* conn = GameConnection::getConnectionToServer();
    if (conn)
    {
      was_1st_person = conn->isFirstPerson();
      if (was_1st_person)
        conn->setFirstPerson(false);
    }
  }

  return true;
}

bool afxEA_CameraPuppet::ea_update(F32 dt)
{
  SceneObject* obj = (cam_cons) ? cam_cons->getSceneObject() : 0;

  if (obj && in_scope)
  {
    obj->setTransform(updated_xfm);
  }

  return true;
}

void afxEA_CameraPuppet::ea_finish(bool was_stopped)
{
  afxCamera* afx_cam = dynamic_cast<afxCamera*>((cam_cons) ? cam_cons->getSceneObject() : 0);
  if (afx_cam && afx_cam->isClientObject())
    afx_cam->setThirdPersonSnapClient();

  if (was_1st_person)
  {
    GameConnection* conn = GameConnection::getConnectionToServer();
    if (conn)
      conn->setFirstPerson(true);
  }
}

void afxEA_CameraPuppet::getUnconstrainedPosition(Point3F& pos)
{
  SceneObject* obj = (cam_cons) ? cam_cons->getSceneObject() : 0;
  if (obj)
    pos = obj->getRenderPosition();
  else
    pos.zero();
}

void afxEA_CameraPuppet::getUnconstrainedTransform(MatrixF& xfm)
{
  SceneObject* obj = (cam_cons) ? cam_cons->getSceneObject() : 0;
  if (obj)
    xfm = obj->getRenderTransform();
  else
    xfm.identity();
}

void afxEA_CameraPuppet::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (puppet_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxCameraPuppetData* orig_db = puppet_data;
    puppet_data = new afxCameraPuppetData(*orig_db, true);
    orig_db->performSubstitutions(puppet_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_CameraPuppetDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_CameraPuppetDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const;
  virtual bool  runsOnClient(const afxEffectWrapperData*) const;

  virtual afxEffectWrapper* create() const { return new afxEA_CameraPuppet; }
};

afxEA_CameraPuppetDesc afxEA_CameraPuppetDesc::desc;

bool afxEA_CameraPuppetDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxCameraPuppetData) == typeid(*db));
}

bool afxEA_CameraPuppetDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

bool afxEA_CameraPuppetDesc::runsOnServer(const afxEffectWrapperData* ew) const
{
  U8 networking = ((const afxCameraPuppetData*)ew->effect_data)->networking;
  return ((networking & (SERVER_ONLY | SERVER_AND_CLIENT)) != 0);
}

bool afxEA_CameraPuppetDesc::runsOnClient(const afxEffectWrapperData* ew) const
{
  U8 networking = ((const afxCameraPuppetData*)ew->effect_data)->networking;
  return ((networking & (CLIENT_ONLY | SERVER_AND_CLIENT)) != 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//