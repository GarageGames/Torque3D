
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

class afxXM_HeightSamplerData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;

public:
  /*C*/         afxXM_HeightSamplerData();
  /*C*/         afxXM_HeightSamplerData(const afxXM_HeightSamplerData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_HeightSamplerData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxConstraint;

class afxXM_HeightSampler : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

public:
  /*C*/           afxXM_HeightSampler(afxXM_HeightSamplerData*, afxEffectWrapper*);

  virtual void    updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_HeightSamplerData);

ConsoleDocClass( afxXM_HeightSamplerData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_HeightSamplerData::afxXM_HeightSamplerData()
{
}

afxXM_HeightSamplerData::afxXM_HeightSamplerData(const afxXM_HeightSamplerData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
}

void afxXM_HeightSamplerData::initPersistFields()
{
  Parent::initPersistFields();
}

void afxXM_HeightSamplerData::packData(BitStream* stream)
{
  Parent::packData(stream);
}

void afxXM_HeightSamplerData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
}

afxXM_Base* afxXM_HeightSamplerData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_HeightSamplerData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_HeightSamplerData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  return new afxXM_HeightSampler(datablock, fx);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_HeightSampler::afxXM_HeightSampler(afxXM_HeightSamplerData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw)
{ 
}

void afxXM_HeightSampler::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  afxConstraint* pos_cons = fx_wrapper->getPosConstraint();
  if (!pos_cons)
    return;
      
  Point3F base_pos;
  pos_cons->getPosition(base_pos);
  
  F32 range = 0.5f;
  F32 height = (base_pos.z > params.pos.z) ? (base_pos.z - params.pos.z) : 0.0f;
  F32 factor = mClampF(1.0f - (height/range), 0.0f, 1.0f);

  //Con::printf("SET height=%g liveScaleFactor=%g", height, factor);
  fx_wrapper->setField("liveScaleFactor", avar("%g", factor));
  fx_wrapper->setField("liveFadeFactor", avar("%g", factor));
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//