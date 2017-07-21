
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

class afxXM_ScaleData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;

public:
  Point3F       scale;

public:
  /*C*/         afxXM_ScaleData();
  /*C*/         afxXM_ScaleData(const afxXM_ScaleData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_ScaleData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_Scale_weighted : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  Point3F       xm_scale;

public:
  /*C*/         afxXM_Scale_weighted(afxXM_ScaleData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_ScaleData);

ConsoleDocClass( afxXM_ScaleData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_ScaleData::afxXM_ScaleData()
{
  scale.set(0,0,0);
}

afxXM_ScaleData::afxXM_ScaleData(const afxXM_ScaleData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
  scale = other.scale;
}

void afxXM_ScaleData::initPersistFields()
{
  addField("scale",  TypePoint3F,   Offset(scale, afxXM_ScaleData),
    "...");

  Parent::initPersistFields();
}

void afxXM_ScaleData::packData(BitStream* stream)
{
  Parent::packData(stream);
  mathWrite(*stream, scale);
}

void afxXM_ScaleData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  mathRead(*stream, &scale);
}

afxXM_Base* afxXM_ScaleData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_ScaleData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_ScaleData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  if (datablock->hasFixedWeight())
    return new afxXM_Scale_weighted(datablock, fx);
  else
    return new afxXM_Scale_weighted(datablock, fx);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Scale_weighted::afxXM_Scale_weighted(afxXM_ScaleData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  xm_scale = db->scale; 
}

void afxXM_Scale_weighted::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  F32 wt_factor = calc_weight_factor(elapsed);
  params.scale += xm_scale*wt_factor;
  if (params.scale.x < 0.00001f) 
    params.scale.x = 0.00001f;
  if (params.scale.y < 0.00001f) 
    params.scale.y = 0.00001f;
  if (params.scale.z < 0.00001f) 
    params.scale.z = 0.00001f;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//