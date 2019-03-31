
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
// LOCAL OFFSET

class afxXM_LocalOffsetData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;
  
public:
  Point3F       local_offset;
  bool          offset_pos2;
  
public:
  /*C*/         afxXM_LocalOffsetData();
  /*C*/         afxXM_LocalOffsetData(const afxXM_LocalOffsetData&, bool = false);
  
  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();
  
  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);
  
  DECLARE_CONOBJECT(afxXM_LocalOffsetData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_LocalOffset_weighted : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  Point3F       local_offset;
  
public:
  /*C*/         afxXM_LocalOffset_weighted(afxXM_LocalOffsetData*, afxEffectWrapper*);
  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

// this fixed variation is used when
// the weight factors are constant.

class afxXM_LocalOffset_fixed : public afxXM_Base
{
  typedef afxXM_Base Parent;

  Point3F       local_offset;
  
public:
  /*C*/         afxXM_LocalOffset_fixed(afxXM_LocalOffsetData*, afxEffectWrapper*);
  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_LocalOffset2_weighted : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  Point3F       local_offset;
  
public:
  /*C*/         afxXM_LocalOffset2_weighted(afxXM_LocalOffsetData*, afxEffectWrapper*);
  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

// this fixed variation is used when
// the weight factors are constant.

class afxXM_LocalOffset2_fixed : public afxXM_Base
{
  typedef afxXM_Base Parent;

  Point3F       local_offset;
  
public:
  /*C*/         afxXM_LocalOffset2_fixed(afxXM_LocalOffsetData*, afxEffectWrapper*);
  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WORLD OFFSET

class afxXM_WorldOffsetData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;
  
public:
  Point3F       world_offset;
  bool          offset_pos2;
  
public:
  /*C*/         afxXM_WorldOffsetData();
  /*C*/         afxXM_WorldOffsetData(const afxXM_WorldOffsetData&, bool = false);
  
  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();
  
  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);
  
  DECLARE_CONOBJECT(afxXM_WorldOffsetData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WorldOffset_weighted : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;
  Point3F       world_offset;
  
public:
  /*C*/         afxXM_WorldOffset_weighted(afxXM_WorldOffsetData*, afxEffectWrapper*);
  
  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

// this fixed variation is used when
// the weight factors are constant.

class afxXM_WorldOffset_fixed : public afxXM_Base
{
  typedef afxXM_WeightedBase Parent;
  Point3F       world_offset;
  
public:
  /*C*/         afxXM_WorldOffset_fixed(afxXM_WorldOffsetData*, afxEffectWrapper*);
  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WorldOffset2_weighted : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;
  Point3F       world_offset;
  
public:
  /*C*/         afxXM_WorldOffset2_weighted(afxXM_WorldOffsetData*, afxEffectWrapper*);
  
  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

// this fixed variation is used when
// the weight factors are constant.

class afxXM_WorldOffset2_fixed : public afxXM_Base
{
  typedef afxXM_WeightedBase Parent;
  Point3F       world_offset;
  
public:
  /*C*/         afxXM_WorldOffset2_fixed(afxXM_WorldOffsetData*, afxEffectWrapper*);
  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// LOCAL OFFSET

IMPLEMENT_CO_DATABLOCK_V1(afxXM_LocalOffsetData);

ConsoleDocClass( afxXM_LocalOffsetData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_LocalOffsetData::afxXM_LocalOffsetData()
{
  local_offset.zero();
  offset_pos2 = false;
}

afxXM_LocalOffsetData::afxXM_LocalOffsetData(const afxXM_LocalOffsetData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
  local_offset = other.local_offset;
  offset_pos2 = other.offset_pos2;
}

void afxXM_LocalOffsetData::initPersistFields()
{
  addField("localOffset",  TypePoint3F,   Offset(local_offset, afxXM_LocalOffsetData),
    "...");
  addField("offsetPos2",   TypeBool,      Offset(offset_pos2, afxXM_LocalOffsetData),
    "...");

  Parent::initPersistFields();
}

void afxXM_LocalOffsetData::packData(BitStream* stream)
{
  Parent::packData(stream);
  mathWrite(*stream, local_offset);
  stream->writeFlag(offset_pos2);
}

void afxXM_LocalOffsetData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  mathRead(*stream, &local_offset);
  offset_pos2 = stream->readFlag();
}

afxXM_Base* afxXM_LocalOffsetData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_LocalOffsetData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_LocalOffsetData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  if (datablock->offset_pos2)
  {
    if (datablock->hasFixedWeight())
      return new afxXM_LocalOffset2_fixed(datablock, fx);
    else
      return new afxXM_LocalOffset2_weighted(datablock, fx);
  }
  else
  {
    if (datablock->hasFixedWeight())
      return new afxXM_LocalOffset_fixed(datablock, fx);
    else
      return new afxXM_LocalOffset_weighted(datablock, fx);
  }
}

//~~~~~~~~~~~~~~~~~~~~//

afxXM_LocalOffset_weighted::afxXM_LocalOffset_weighted(afxXM_LocalOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  local_offset = db->local_offset*db->getWeightFactor(); 
}

void afxXM_LocalOffset_weighted::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  F32 wt_factor = calc_weight_factor(elapsed);
  Point3F offset(local_offset*wt_factor);
  params.ori.mulV(offset);
  params.pos += offset;
}

//~~~~~~~~~~~~~~~~~~~~//

afxXM_LocalOffset_fixed::afxXM_LocalOffset_fixed(afxXM_LocalOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw) 
{ 
  local_offset = db->local_offset*db->getWeightFactor(); 
}

void afxXM_LocalOffset_fixed::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  Point3F offset(local_offset);
  params.ori.mulV(offset);
  params.pos += offset;
}

//~~~~~~~~~~~~~~~~~~~~//

afxXM_LocalOffset2_weighted::afxXM_LocalOffset2_weighted(afxXM_LocalOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  local_offset = db->local_offset*db->getWeightFactor(); 
}

void afxXM_LocalOffset2_weighted::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  F32 wt_factor = calc_weight_factor(elapsed);
  Point3F offset(local_offset*wt_factor);

  afxConstraint* pos2_cons = fx_wrapper->getAimConstraint();
  if (pos2_cons)
  {
    MatrixF ori2;
    pos2_cons->getTransform(ori2);
    ori2.mulV(offset);
  }

  params.pos2 += offset;
}

//~~~~~~~~~~~~~~~~~~~~//

afxXM_LocalOffset2_fixed::afxXM_LocalOffset2_fixed(afxXM_LocalOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw) 
{ 
  local_offset = db->local_offset*db->getWeightFactor(); 
}

void afxXM_LocalOffset2_fixed::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  Point3F offset(local_offset);

  afxConstraint* pos2_cons = fx_wrapper->getAimConstraint();
  if (pos2_cons)
  {
    MatrixF ori2;
    pos2_cons->getTransform(ori2);
    ori2.mulV(offset);
  }

  params.pos2 += offset;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WORLD OFFSET

IMPLEMENT_CO_DATABLOCK_V1(afxXM_WorldOffsetData);

ConsoleDocClass( afxXM_WorldOffsetData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_WorldOffsetData::afxXM_WorldOffsetData()
{
  world_offset.zero();
  offset_pos2 = false;
}

afxXM_WorldOffsetData::afxXM_WorldOffsetData(const afxXM_WorldOffsetData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
  world_offset = other.world_offset;
  offset_pos2 = other.offset_pos2;
}

void afxXM_WorldOffsetData::initPersistFields()
{
  addField("worldOffset",   TypePoint3F,  Offset(world_offset, afxXM_WorldOffsetData),
    "...");
  addField("offsetPos2",    TypeBool,     Offset(offset_pos2, afxXM_WorldOffsetData),
    "...");

  Parent::initPersistFields();
}

void afxXM_WorldOffsetData::packData(BitStream* stream)
{
  Parent::packData(stream);
  mathWrite(*stream, world_offset);
  stream->writeFlag(offset_pos2);
}

void afxXM_WorldOffsetData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  mathRead(*stream, &world_offset);
  offset_pos2 = stream->readFlag();
}

afxXM_Base* afxXM_WorldOffsetData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_WorldOffsetData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_WorldOffsetData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  if (datablock->offset_pos2)
  {
    if (datablock->hasFixedWeight())
      return new afxXM_WorldOffset2_fixed(datablock, fx);
    else
      return new afxXM_WorldOffset2_weighted(datablock, fx);
  }
  else
  {
    if (datablock->hasFixedWeight())
      return new afxXM_WorldOffset_fixed(datablock, fx);
    else
      return new afxXM_WorldOffset_weighted(datablock, fx);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_WorldOffset_weighted::afxXM_WorldOffset_weighted(afxXM_WorldOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  world_offset = db->world_offset*db->getWeightFactor();
}

void afxXM_WorldOffset_weighted::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  F32 wt_factor = calc_weight_factor(elapsed);
  params.pos += world_offset*wt_factor;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_WorldOffset_fixed::afxXM_WorldOffset_fixed(afxXM_WorldOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw) 
{ 
  world_offset = db->world_offset*db->getWeightFactor();
}

void afxXM_WorldOffset_fixed::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  params.pos += world_offset;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_WorldOffset2_weighted::afxXM_WorldOffset2_weighted(afxXM_WorldOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  world_offset = db->world_offset*db->getWeightFactor();
}

void afxXM_WorldOffset2_weighted::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  F32 wt_factor = calc_weight_factor(elapsed);
  params.pos2 += world_offset*wt_factor;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_WorldOffset2_fixed::afxXM_WorldOffset2_fixed(afxXM_WorldOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw) 
{ 
  world_offset = db->world_offset*db->getWeightFactor();
}

void afxXM_WorldOffset2_fixed::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  params.pos2 += world_offset;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//