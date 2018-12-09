
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

#include "math/mathIO.h"
#include "math/mathUtils.h"

#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/xm/afxXfmMod.h"

#include "afx/afxEffectDefs.h"
#include "afx/forces/afxForce.h"
#include "afx/forces/afxForceSet.h"

//#define ECHO_DEBUG_INFO

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxXM_ForceData : public afxXM_WeightedBaseData, public afxEffectDefs
{
  typedef afxXM_WeightedBaseData Parent;

public:
  StringTableEntry  force_set_name;
  F32               update_dt;

public:
  /*C*/         afxXM_ForceData();
  /*C*/         afxXM_ForceData(const afxXM_ForceData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

   bool         preload(bool server, String &errorStr);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_ForceData);
  DECLARE_CATEGORY("AFX");
};

class afxXM_Force : public afxXM_WeightedBase, public afxEffectDefs
{
  typedef afxXM_WeightedBase Parent;

  afxForceSet*  force_set;

  Point3F       pos_local;
  Point3F       velocity;

  bool          first;

  F32           mass;
  F32           mass_inverse;

  afxXM_ForceData* db;

public:
  /*C*/         afxXM_Force(afxXM_ForceData*, afxEffectWrapper*);

  virtual void  start(F32 timestamp);
  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_ForceData);

ConsoleDocClass( afxXM_ForceData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxExperimental\n"
   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_ForceData::afxXM_ForceData()
{
  force_set_name = ST_NULLSTRING;
  update_dt = -1.0f;
}

afxXM_ForceData::afxXM_ForceData(const afxXM_ForceData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
  force_set_name = other.force_set_name;
  update_dt = other.update_dt;
}


void afxXM_ForceData::initPersistFields()
{
  addField("forceSetName",    TypeString, Offset(force_set_name, afxXM_ForceData),
    "...");
  addField("updateDT",        TypeF32,    Offset(update_dt, afxXM_ForceData),
    "...");

  Parent::initPersistFields();
}

void afxXM_ForceData::packData(BitStream* stream)
{
  Parent::packData(stream);
 
  stream->writeString(force_set_name);
  if (stream->writeFlag(update_dt < 0.0f))
    stream->write(update_dt);
}

void afxXM_ForceData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  
  force_set_name = stream->readSTString();
  if (stream->readFlag())
    stream->read(&update_dt);
  else
    update_dt = -1.0f;
}

bool afxXM_ForceData::preload(bool server, String &errorStr)
{
  if(!Parent::preload(server, errorStr))
    return false;
  
  return true;
}

afxXM_Base* afxXM_ForceData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_ForceData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_ForceData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  return new afxXM_Force(datablock, fx);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Force::afxXM_Force(afxXM_ForceData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  this->db = db; 

  force_set = 0;

  pos_local.zero();
  velocity.zero();

  mass = 1.0f;
  mass_inverse = 1.0f;

  first = true;
}

void afxXM_Force::start(F32 timestamp)
{
  Parent::start(timestamp);

  afxForceSetMgr* force_set_mgr = fx_wrapper->getChoreographer()->getForceSetMgr();
  force_set = (force_set_mgr) ? force_set_mgr->findForceSet(db->force_set_name) : 0;
  if (!force_set)
  {
    Con::errorf(ConsoleLogEntry::General,
           "afxXM_Force::start() -- unable to find afxForceSet %s", db->force_set_name);
    return;
  }

  mass = fx_wrapper->getMass();

  // compute mass_inverse safely
  mass_inverse = (mass > 0.0001f) ? (1.0f/mass) : 1.0f/0.0001f;

  F32 update_dt = (db->update_dt < 0.0f) ? 1.0f/30.0f : db->update_dt;
  if (force_set->getUpdateDT() > update_dt)
    force_set->setUpdateDT(update_dt);
}

// JTF Note: answer these questions?
// Can mass be removed from the force and acceleration calculations?
// XFM Weight is not accounted for (yet).
void afxXM_Force::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (!force_set)
    return;

#ifdef ECHO_DEBUG_INFO
  Con::printf("afxXM_Force: elapsed=%f (dt=%f)", elapsed,dt);
#endif

  if (first)
  {
    velocity = fx_wrapper->getDirection();
    velocity.normalizeSafe();
    params.ori.mulP(velocity);
    velocity *= fx_wrapper->getSpeed();

#ifdef ECHO_DEBUG_INFO
    Con::printf("INITIAL VELOCITY : %f %f %f", velocity.x, velocity.y, velocity.z);
    Con::printf("MASS             : %f %f", mass, mass_inverse );
#endif

    first = false;
  }

  S32 num_updates = force_set->updateDT(dt);
  if (num_updates == 0)
  {
    params.pos += pos_local;
    return;
  }

  for (S32 j = 0; j < num_updates; j++)
  {
    Point3F F_net(0,0,0);
    for (S32 i = 0; i < force_set->count(); i++)
    {
      afxForce* force = force_set->getForce(i);
#ifdef ECHO_DEBUG_INFO
      Point3F F = force->evaluate(params.pos+pos_local, velocity, mass);
      F_net += F;
      Con::printf("(%d) F %i: %f %f %f", this, i, F.x, F.y, F.z);
#else
      F_net += force->evaluate(params.pos+pos_local, velocity, mass);
#endif
    }

    Point3F acceleration = F_net * mass_inverse * force_set->getUpdateDT();
    velocity += acceleration;

    pos_local += velocity;    
  }
  params.pos += pos_local;

#ifdef ECHO_DEBUG_INFO
  Con::printf("velocity : %f %f %f", velocity.x, velocity.y, velocity.z);
  Con::printf("pos:       %f %f %f", params.pos.x, params.pos.y, params.pos.z);
#endif
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

