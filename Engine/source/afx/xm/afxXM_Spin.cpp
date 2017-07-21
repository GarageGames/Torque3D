
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
#include "afx/util/afxPath3D.h"
#include "afx/util/afxPath.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxXM_SpinData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;

public:
  Point3F       spin_axis;
  F32           spin_angle;
  F32           spin_angle_var;
  F32           spin_rate;
  F32           spin_rate_var;

public:
  /*C*/         afxXM_SpinData() : spin_axis(0,0,1), spin_angle(0),  spin_angle_var(0), spin_rate(0), spin_rate_var(0) { }
  /*C*/         afxXM_SpinData(const afxXM_SpinData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);
  bool          onAdd();

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_SpinData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_Spin_weighted : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  Point3F       spin_axis;
  F32           spin_rate;
  F32           theta;

public:
  /*C*/         afxXM_Spin_weighted(afxXM_SpinData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_Spin_fixed : public afxXM_Base
{
  typedef afxXM_Base Parent;

  Point3F       spin_axis;
  F32           spin_rate;
  F32           theta;

public:
  /*C*/         afxXM_Spin_fixed(afxXM_SpinData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_SpinData);

ConsoleDocClass( afxXM_SpinData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_SpinData::afxXM_SpinData(const afxXM_SpinData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
  spin_axis = other.spin_axis;
  spin_angle = other.spin_angle;
  spin_angle_var = other.spin_angle_var;
  spin_rate = other.spin_rate;
  spin_rate_var = other.spin_rate_var;
}

void afxXM_SpinData::initPersistFields()
{
  addField("spinAxis",            TypePoint3F,  Offset(spin_axis, afxXM_SpinData),
    "...");
  addField("spinAngle",           TypeF32,      Offset(spin_angle, afxXM_SpinData),
    "...");
  addField("spinAngleVariance",   TypeF32,      Offset(spin_angle_var, afxXM_SpinData),
    "...");
  addField("spinRate",            TypeF32,      Offset(spin_rate, afxXM_SpinData),
    "...");
  addField("spinRateVariance",    TypeF32,      Offset(spin_rate_var, afxXM_SpinData),
    "...");

  Parent::initPersistFields();
}

void afxXM_SpinData::packData(BitStream* stream)
{
  Parent::packData(stream);
  mathWrite(*stream, spin_axis);
  stream->write(spin_angle);
  stream->write(spin_angle_var);
  stream->write(spin_rate);
  stream->write(spin_rate_var);
}

void afxXM_SpinData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  mathRead(*stream, &spin_axis);
  stream->read(&spin_angle);
  stream->read(&spin_angle_var);
  stream->read(&spin_rate);
  stream->read(&spin_rate_var);
}

bool afxXM_SpinData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  spin_axis.normalizeSafe();

  return true;
}

afxXM_Base* afxXM_SpinData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_SpinData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_SpinData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  if (datablock->hasFixedWeight())
    return new afxXM_Spin_fixed(datablock, fx);
  else
    return new afxXM_Spin_weighted(datablock, fx);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Spin_weighted::afxXM_Spin_weighted(afxXM_SpinData* db, afxEffectWrapper* fxw)
    : afxXM_WeightedBase(db, fxw) 
{ 
  spin_axis = db->spin_axis;

  spin_rate = db->spin_rate;
  if (db->spin_rate_var != 0.0f)
    spin_rate += gRandGen.randF()*2.0f*db->spin_rate_var - db->spin_rate_var;
  spin_rate *= db->getWeightFactor()/time_factor;

  F32 spin_angle = db->spin_angle;
  if (db->spin_angle_var != 0.0f)
    spin_angle += gRandGen.randF()*2.0f*db->spin_angle_var - db->spin_angle_var;
  theta = mDegToRad(spin_angle);
}

void afxXM_Spin_weighted::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  F32 wt_factor = calc_weight_factor(elapsed);
  F32 rate = spin_rate*wt_factor;
  theta += mDegToRad(dt*rate);

  AngAxisF spin_aa(spin_axis, theta);
  MatrixF spin_xfm; spin_aa.setMatrix(&spin_xfm);
  
  params.ori.mul(spin_xfm);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Spin_fixed::afxXM_Spin_fixed(afxXM_SpinData* db, afxEffectWrapper* fxw)
    : afxXM_Base(db, fxw) 
{ 
  spin_axis = db->spin_axis;

  spin_rate = db->spin_rate;
  if (db->spin_rate_var != 0.0f)
    spin_rate += gRandGen.randF()*2.0f*db->spin_rate_var - db->spin_rate_var;
  spin_rate *= db->getWeightFactor()/time_factor;

  F32 spin_angle = db->spin_angle;
  if (db->spin_angle_var != 0.0f)
    spin_angle += gRandGen.randF()*2.0f*db->spin_angle_var - db->spin_angle_var;
  theta = mDegToRad(spin_angle);
}

void afxXM_Spin_fixed::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  theta += mDegToRad(dt*spin_rate);

  AngAxisF spin_aa(spin_axis, theta);
  MatrixF spin_xfm; spin_aa.setMatrix(&spin_xfm);
  
  params.ori.mul(spin_xfm);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//