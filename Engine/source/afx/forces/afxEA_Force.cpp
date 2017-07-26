
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

#include "afx/arcaneFX.h"
#include "afx/afxChoreographer.h"
#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/forces/afxForce.h"
#include "afx/forces/afxForceSet.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_Force 

class afxEA_Force : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxForceData*     force_data;
  afxForce*         force;
  afxForceSetMgr*   force_set_mgr;

  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_Force();
  /*D*/             ~afxEA_Force();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_Force::afxEA_Force()
{
  force_data = 0;
  force = 0;
  force_set_mgr = 0;
}

afxEA_Force::~afxEA_Force()
{
  if (force)
  {
    if (force_set_mgr) 
      force_set_mgr->unregisterForce(force_data->force_set_name, force);
    delete force;
  }

  if (force_data && force_data->isTempClone())
  {
    delete force_data;
    force_data = 0;
  }

  force_set_mgr = 0;
}

void afxEA_Force::ea_set_datablock(SimDataBlock* db)
{
  force_data = dynamic_cast<afxForceData*>(db);
}

bool afxEA_Force::ea_start()
{
  if (!force_data)
  {
    Con::errorf("afxEA_Force::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  force_set_mgr = choreographer->getForceSetMgr();

  return true;
}

bool afxEA_Force::ea_update(F32 dt)
{
  if (!force)
  {
    force = (force_data->force_desc) ? force_data->force_desc->create() : 0;
    if (!force)
    {
      delete force;
      force = 0;
      Con::errorf(ConsoleLogEntry::General, "Force effect failed to instantiate. (%s)", datablock->getName());
      return false;
    }
    force->onNewDataBlock(force_data, false);

    if (force)
    {
      force_set_mgr->registerForce(force_data->force_set_name, force);
      force->start();
    }
  }

  if (force) // && in_scope)
  {
    if (do_fades)
      force->setFadeAmount(fade_value);
    
    force->update(dt);
  }

  return true;
}

void afxEA_Force::ea_finish(bool was_stopped)
{
  if (!force)
    return;

  if (force_set_mgr) 
    force_set_mgr->unregisterForce(force_data->force_set_name, force);
  delete force;
  force = 0;
}

void afxEA_Force::do_runtime_substitutions()
{
  force_data = force_data->cloneAndPerformSubstitutions(choreographer, group_index);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_ForceDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_ForceDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }

  virtual afxEffectWrapper* create() const { return new afxEA_Force; }
};

afxEA_ForceDesc afxEA_ForceDesc::desc;

bool afxEA_ForceDesc::testEffectType(const SimDataBlock* db) const
{
  if (dynamic_cast<const afxForceData*>(db) != 0)
    return afxForceDesc::identifyForce((afxForceData*) db);

  return false;
}

bool afxEA_ForceDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//


