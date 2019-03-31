
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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxXM_RandomRotData : public afxXM_BaseData
{
  typedef afxXM_BaseData Parent;

public:
  Point3F       axis;
  F32           theta_min;
  F32           theta_max;
  F32           phi_min;
  F32           phi_max;

public:
  /*C*/         afxXM_RandomRotData(); 
  /*C*/         afxXM_RandomRotData(const afxXM_RandomRotData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);
  bool          onAdd();

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_RandomRotData);
  DECLARE_CATEGORY("AFX");
};

class afxXM_RandomRot : public afxXM_Base
{
  typedef afxXM_Base Parent;

  MatrixF       rand_ori;

public:
  /*C*/         afxXM_RandomRot(afxXM_RandomRotData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_RandomRotData);

ConsoleDocClass( afxXM_RandomRotData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_RandomRotData::afxXM_RandomRotData()
{
  axis.set(0,0,1);
  theta_min = 0.0f; 
  theta_max = 360.0f;
  phi_min = 0.0f; 
  phi_max = 360.0f;
}

afxXM_RandomRotData::afxXM_RandomRotData(const afxXM_RandomRotData& other, bool temp_clone) : afxXM_BaseData(other, temp_clone)
{
  axis = other.axis;
  theta_min = other.theta_min;
  theta_max = other.theta_max;
  phi_min = other.phi_min;
  phi_max = other.phi_max;
}

void afxXM_RandomRotData::initPersistFields()
{
  addField("axis",      TypePoint3F,  Offset(axis, afxXM_RandomRotData),
    "...");
  addField("thetaMin",  TypeF32,      Offset(theta_min, afxXM_RandomRotData),
    "...");
  addField("thetaMax",  TypeF32,      Offset(theta_max, afxXM_RandomRotData),
    "...");
  addField("phiMin",    TypeF32,      Offset(phi_min, afxXM_RandomRotData),
    "...");
  addField("phiMax",    TypeF32,      Offset(phi_max, afxXM_RandomRotData),
    "...");

  Parent::initPersistFields();
}

void afxXM_RandomRotData::packData(BitStream* stream)
{
  Parent::packData(stream);
  mathWrite(*stream, axis);
  stream->write(theta_min);
  stream->write(theta_max);
  stream->write(phi_min);
  stream->write(phi_max);
}

void afxXM_RandomRotData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  mathRead(*stream, &axis);
  stream->read(&theta_min);
  stream->read(&theta_max);
  stream->read(&phi_min);
  stream->read(&phi_max);
}

bool afxXM_RandomRotData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  axis.normalizeSafe();

  return true;
}

afxXM_Base* afxXM_RandomRotData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_RandomRotData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_RandomRotData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  return new afxXM_RandomRot(datablock, fx);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_RandomRot::afxXM_RandomRot(afxXM_RandomRotData* db, afxEffectWrapper* fxw)
: afxXM_Base(db, fxw) 
{ 
  Point3F rand_dir = MathUtils::randomDir(db->axis, db->theta_min, db->theta_max, db->phi_min, db->phi_max);
  rand_ori = MathUtils::createOrientFromDir(rand_dir);
}

void afxXM_RandomRot::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  params.ori = rand_ori;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//