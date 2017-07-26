
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

#include "T3D/fx/cameraFXMgr.h"

#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/ce/afxCameraShake.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_CameraShake 

class afxEA_CameraShake : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxCameraShakeData* shake_data;
  CameraShake*        camera_shake;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_CameraShake();
  /*D*/             ~afxEA_CameraShake();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_CameraShake::afxEA_CameraShake()
{
  shake_data = 0;
  camera_shake = 0;
}

afxEA_CameraShake::~afxEA_CameraShake()
{
  delete camera_shake;
  if (shake_data && shake_data->isTempClone())
    delete shake_data;
  shake_data = 0;
}

void afxEA_CameraShake::ea_set_datablock(SimDataBlock* db)
{
  shake_data = dynamic_cast<afxCameraShakeData*>(db);
}

bool afxEA_CameraShake::ea_start()
{
  if (!shake_data)
  {
    Con::errorf("afxEA_CameraShake::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  afxConstraint* pos_constraint = getPosConstraint();
  afxConstraint* aim_constraint = getAimConstraint();

  if (aim_constraint && pos_constraint)
  {
    if (full_lifetime <= 0 || full_lifetime == INFINITE_LIFETIME)
    {
      Con::errorf("afxEA_CameraShake::ea_start() --  effect requires a finite lifetime.");
      return false;
    }

    SceneObject* shaken = aim_constraint->getSceneObject();
    if (shaken)
    {
      Point3F pos; pos_constraint->getPosition(pos);
      VectorF diff = shaken->getPosition() - pos;
      F32 dist = diff.len();
      if (dist < shake_data->camShakeRadius)
      {
        camera_shake = new CameraShake;
        camera_shake->setDuration(full_lifetime);
        camera_shake->setFrequency(shake_data->camShakeFreq);

        F32 falloff =  dist/shake_data->camShakeRadius;
        falloff = 1 + falloff*10.0;
        falloff = 1.0 / (falloff*falloff);

        VectorF shakeAmp = shake_data->camShakeAmp*falloff;
        camera_shake->setAmplitude(shakeAmp);
        camera_shake->setFalloff(shake_data->camShakeFalloff);
        camera_shake->init();
      }
    }
  }

  return true;
}

bool afxEA_CameraShake::ea_update(F32 dt)
{
  afxConstraint* aim_constraint = getAimConstraint();
  if (camera_shake && aim_constraint)
  {
    camera_shake->update(dt);
    
    SceneObject* shaken = aim_constraint->getSceneObject();
    if (shaken)
    {
      MatrixF fxTrans = camera_shake->getTrans();
      MatrixF curTrans = shaken->getRenderTransform();
      curTrans.mul(fxTrans);
      
      Point3F	cameraPosWorld;
      curTrans.getColumn(3,&cameraPosWorld);
      shaken->setPosition(cameraPosWorld);
    }
  }
  
  return true;
}

void afxEA_CameraShake::ea_finish(bool was_stopped)
{
  delete camera_shake;
  camera_shake = 0;
}

void afxEA_CameraShake::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (shake_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxCameraShakeData* orig_db = shake_data;
    shake_data = new afxCameraShakeData(*orig_db, true);
    orig_db->performSubstitutions(shake_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_CameraShakeDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_CameraShakeDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }
  virtual bool  isPositional(const afxEffectWrapperData*) const { return false; }

  virtual afxEffectWrapper* create() const { return new afxEA_CameraShake; }
};

afxEA_CameraShakeDesc afxEA_CameraShakeDesc::desc;

bool afxEA_CameraShakeDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxCameraShakeData) == typeid(*db));
}

bool afxEA_CameraShakeDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//