
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

#include "math/mathUtils.h"

#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/xm/afxXfmMod.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxXM_AimData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;

public:
  bool          aim_z_only;

public:
  /*C*/         afxXM_AimData();
  /*C*/         afxXM_AimData(const afxXM_AimData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_AimData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxConstraint;

class afxXM_Aim_weighted : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

public:
  /*C*/           afxXM_Aim_weighted(afxXM_AimData*, afxEffectWrapper*);

  virtual void    updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_Aim_weighted_z : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

public:
  /*C*/           afxXM_Aim_weighted_z(afxXM_AimData*, afxEffectWrapper*);

  virtual void    updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_Aim_fixed : public afxXM_Base
{
  typedef afxXM_Base Parent;

public:
  /*C*/           afxXM_Aim_fixed(afxXM_AimData*, afxEffectWrapper*);

  virtual void    updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_Aim_fixed_z : public afxXM_Base
{
  typedef afxXM_Base Parent;

public:
  /*C*/           afxXM_Aim_fixed_z(afxXM_AimData*, afxEffectWrapper*);

  virtual void    updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_AimData);

ConsoleDocClass( afxXM_AimData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_AimData::afxXM_AimData()
{
  aim_z_only = false;
}

afxXM_AimData::afxXM_AimData(const afxXM_AimData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
  aim_z_only = other.aim_z_only;
}

void afxXM_AimData::initPersistFields()
{
  addField("aimZOnly",          TypeBool,    Offset(aim_z_only, afxXM_AimData),
    "...");

  Parent::initPersistFields();
}

void afxXM_AimData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->writeFlag(aim_z_only);
}

void afxXM_AimData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  aim_z_only = stream->readFlag();
}

afxXM_Base* afxXM_AimData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_AimData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_AimData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  if (datablock->aim_z_only)
  {
    if (datablock->hasFixedWeight())
      return new afxXM_Aim_fixed_z(datablock, fx);
    else
      return new afxXM_Aim_weighted_z(datablock, fx);
  }
  else
  {
    if (datablock->hasFixedWeight())
      return new afxXM_Aim_fixed(datablock, fx);
    else
      return new afxXM_Aim_weighted(datablock, fx);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Aim_weighted::afxXM_Aim_weighted(afxXM_AimData* db, afxEffectWrapper* fxw) 
  : afxXM_WeightedBase(db, fxw)
{ 
}

void afxXM_Aim_weighted::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{ 
  VectorF line_of_sight = params.pos2 - params.pos;
  line_of_sight.normalize();

  F32 wt_factor = calc_weight_factor(elapsed); 

  QuatF qt_ori_incoming(params.ori);
  
  MatrixF ori_outgoing = MathUtils::createOrientFromDir(line_of_sight); 
  QuatF qt_ori_outgoing(ori_outgoing);

  QuatF qt_ori = qt_ori_incoming.slerp(qt_ori_outgoing, wt_factor);

  qt_ori.setMatrix(&params.ori);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Aim_weighted_z::afxXM_Aim_weighted_z(afxXM_AimData* db, afxEffectWrapper* fxw) 
  : afxXM_WeightedBase(db, fxw)
{ 
}

void afxXM_Aim_weighted_z::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  Point3F aim_at_pos = params.pos2;
  aim_at_pos.z = params.pos.z;

  VectorF line_of_sight = aim_at_pos - params.pos;
  line_of_sight.normalize();

  F32 wt_factor = calc_weight_factor(elapsed); 

  QuatF qt_ori_incoming(params.ori);
  
  MatrixF ori_outgoing = MathUtils::createOrientFromDir(line_of_sight); 
  QuatF qt_ori_outgoing( ori_outgoing );

  QuatF qt_ori = qt_ori_incoming.slerp(qt_ori_outgoing, wt_factor);

  qt_ori.setMatrix(&params.ori);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Aim_fixed::afxXM_Aim_fixed(afxXM_AimData* db, afxEffectWrapper* fxw) 
  : afxXM_Base(db, fxw)
{ 
}

void afxXM_Aim_fixed::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  VectorF line_of_sight = params.pos2 - params.pos;
  line_of_sight.normalize();  
  params.ori = MathUtils::createOrientFromDir(line_of_sight); 
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Aim_fixed_z::afxXM_Aim_fixed_z(afxXM_AimData* db, afxEffectWrapper* fxw) 
  : afxXM_Base(db, fxw)
{ 
}

void afxXM_Aim_fixed_z::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  Point3F aim_at_pos = params.pos2;    
  aim_at_pos.z = params.pos.z;

  VectorF line_of_sight = aim_at_pos - params.pos;
  line_of_sight.normalize();

  params.ori = MathUtils::createOrientFromDir(line_of_sight);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//