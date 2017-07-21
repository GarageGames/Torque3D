
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

#include "T3D/fx/explosion.h"

#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_Explosion 

class afxEA_Explosion : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  ExplosionData*    explosion_data;
  Explosion*        explosion;
  bool              exploded;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_Explosion();

  virtual bool      isDone() { return exploded; }

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_Explosion::afxEA_Explosion()
{
  explosion_data = 0;
  explosion = 0;
  exploded = false;
}

void afxEA_Explosion::ea_set_datablock(SimDataBlock* db)
{
  explosion_data = dynamic_cast<ExplosionData*>(db);
}

bool afxEA_Explosion::ea_start()
{
  if (!explosion_data)
  {
    Con::errorf("afxEA_Explosion::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  explosion = new Explosion();
  explosion->setSubstitutionData(choreographer, group_index);
  explosion->setDataBlock(explosion_data);

  return true;
}

bool afxEA_Explosion::ea_update(F32 dt)
{
  if (!exploded && explosion)
  {
    if (in_scope)
    {
      Point3F norm(0,0,1); updated_xfm.mulV(norm);
      explosion->setInitialState(updated_pos, norm);
      if (!explosion->registerObject())
      {
        delete explosion;
        explosion = 0;
        Con::errorf("afxEA_Explosion::ea_update() -- effect failed to register.");
        return false;
      }
    }
    exploded = true;
  }

  return true;
}

void afxEA_Explosion::ea_finish(bool was_stopped)
{
  explosion = 0;
  exploded = false;
}

void afxEA_Explosion::do_runtime_substitutions()
{
  explosion_data = explosion_data->cloneAndPerformSubstitutions(choreographer, group_index);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_ExplosionDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_ExplosionDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const { return false; }
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }

  virtual afxEffectWrapper* create() const { return new afxEA_Explosion; }
};

afxEA_ExplosionDesc afxEA_ExplosionDesc::desc;

bool afxEA_ExplosionDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(ExplosionData) == typeid(*db));
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//