
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
// VELOCITY  OFFSET

class afxXM_VelocityOffsetData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;
  
public:
  F32           offset_factor;
  bool          offset_pos2;
  bool          normalize;
  
public:
  /*C*/         afxXM_VelocityOffsetData();
  /*C*/         afxXM_VelocityOffsetData(const afxXM_VelocityOffsetData&, bool = false);
  
  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();
  
  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);
  
  DECLARE_CONOBJECT(afxXM_VelocityOffsetData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_VelocityOffset_weighted : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  afxConstraint*  cons;
  F32             offset_factor;
  bool            normalize;
  
public:
  /*C*/           afxXM_VelocityOffset_weighted(afxXM_VelocityOffsetData*, afxEffectWrapper*);
  
  virtual void    start(F32 timestamp);
  virtual void    updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

// this fixed variation is used when
// the weight factors are constant.

class afxXM_VelocityOffset_fixed : public afxXM_Base
{
  typedef afxXM_Base Parent;

  afxConstraint*  cons;
  F32             offset_factor;
  bool            normalize;
  
public:
  /*C*/           afxXM_VelocityOffset_fixed(afxXM_VelocityOffsetData*, afxEffectWrapper*);

  virtual void    start(F32 timestamp);
  virtual void    updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_VelocityOffset2_weighted : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  afxConstraint*  cons;
  F32             offset_factor;
  bool            normalize;
  
public:
  /*C*/           afxXM_VelocityOffset2_weighted(afxXM_VelocityOffsetData*, afxEffectWrapper*);
  
  virtual void    start(F32 timestamp);
  virtual void    updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

// this fixed variation is used when
// the weight factors are constant.

class afxXM_VelocityOffset2_fixed : public afxXM_Base
{
  typedef afxXM_Base Parent;

  afxConstraint*  cons;
  F32             offset_factor;
  bool            normalize;
  
public:
  /*C*/           afxXM_VelocityOffset2_fixed(afxXM_VelocityOffsetData*, afxEffectWrapper*);

  virtual void    start(F32 timestamp);
  virtual void    updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// VELOCITY OFFSET

IMPLEMENT_CO_DATABLOCK_V1(afxXM_VelocityOffsetData);

ConsoleDocClass( afxXM_VelocityOffsetData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_VelocityOffsetData::afxXM_VelocityOffsetData()
{
  offset_factor = 1.0f;
  offset_pos2 = false;
  normalize = false;
}

afxXM_VelocityOffsetData::afxXM_VelocityOffsetData(const afxXM_VelocityOffsetData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
  offset_factor = other.offset_factor;
  offset_pos2 = other.offset_pos2;
  normalize = other.normalize;
}

void afxXM_VelocityOffsetData::initPersistFields()
{
  addField("offsetFactor",    TypeF32,      Offset(offset_factor, afxXM_VelocityOffsetData),
    "...");
  addField("offsetPos2",      TypeBool,     Offset(offset_pos2, afxXM_VelocityOffsetData),
    "...");
  addField("normalize",       TypeBool,     Offset(normalize, afxXM_VelocityOffsetData),
    "...");

  Parent::initPersistFields();
}

void afxXM_VelocityOffsetData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(offset_factor);
  stream->write(offset_pos2);
}

void afxXM_VelocityOffsetData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&offset_factor);
  stream->read(&offset_pos2);
}

afxXM_Base* afxXM_VelocityOffsetData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_VelocityOffsetData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_VelocityOffsetData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  if (datablock->offset_pos2)
  {
    if (datablock->hasFixedWeight())
      return new afxXM_VelocityOffset2_fixed(datablock, fx);
    else
      return new afxXM_VelocityOffset2_weighted(datablock, fx);
  }
  else
  {
    if (datablock->hasFixedWeight())
      return new afxXM_VelocityOffset_fixed(datablock, fx);
    else
      return new afxXM_VelocityOffset_weighted(datablock, fx);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_VelocityOffset_weighted::afxXM_VelocityOffset_weighted(afxXM_VelocityOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  cons = 0;
  offset_factor = db->offset_factor*db->getWeightFactor();
  normalize = db->normalize;
}

void afxXM_VelocityOffset_weighted::start(F32 timestamp)
{
  Parent::start(timestamp);

  cons = fx_wrapper->getPosConstraint();
  if (!cons)
    Con::errorf(ConsoleLogEntry::General, 
                "afxXM_VelocityOffset: failed to find a SceneObject derived constraint source.");
}

void afxXM_VelocityOffset_weighted::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (!cons)
    return;

  SceneObject* scene_obj = cons->getSceneObject();
  if (scene_obj)
  {
    Point3F vel_vec = scene_obj->getVelocity();
    if (!vel_vec.isZero())
    {
      if (normalize)
        vel_vec.normalize();
      F32 wt_factor = calc_weight_factor(elapsed);
      params.pos += vel_vec*offset_factor*wt_factor;
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_VelocityOffset_fixed::afxXM_VelocityOffset_fixed(afxXM_VelocityOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw) 
{ 
  cons = 0;
  offset_factor = db->offset_factor*db->getWeightFactor();
  normalize = db->normalize;
}

void afxXM_VelocityOffset_fixed::start(F32 timestamp)
{
  Parent::start(timestamp);

  cons = fx_wrapper->getPosConstraint();
  if (!cons)
    Con::errorf(ConsoleLogEntry::General, 
                "afxXM_VelocityOffset: failed to find a SceneObject derived constraint source.");
}

void afxXM_VelocityOffset_fixed::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (!cons)
    return;

  SceneObject* scene_obj = cons->getSceneObject();
  if (scene_obj)
  {
    Point3F vel_vec = scene_obj->getVelocity();
    if (!vel_vec.isZero())
    {
      if (normalize)
        vel_vec.normalize(offset_factor);
      params.pos += vel_vec*offset_factor;
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_VelocityOffset2_weighted::afxXM_VelocityOffset2_weighted(afxXM_VelocityOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  cons = 0;
  offset_factor = db->offset_factor*db->getWeightFactor();
  normalize = db->normalize;
}

void afxXM_VelocityOffset2_weighted::start(F32 timestamp)
{
  Parent::start(timestamp);

  cons = fx_wrapper->getAimConstraint();
  if (!cons)
    Con::errorf(ConsoleLogEntry::General, 
                "afxXM_VelocityOffset: failed to find a SceneObject derived constraint source.");
}

void afxXM_VelocityOffset2_weighted::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (!cons)
    return;

  SceneObject* scene_obj = cons->getSceneObject();
  if (scene_obj)
  {
    Point3F vel_vec = scene_obj->getVelocity();
    if (!vel_vec.isZero())
    {
      if (normalize)
        vel_vec.normalize();
      F32 wt_factor = calc_weight_factor(elapsed);
      params.pos2 += vel_vec*offset_factor*wt_factor;
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_VelocityOffset2_fixed::afxXM_VelocityOffset2_fixed(afxXM_VelocityOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw) 
{ 
  cons = 0;
  offset_factor = db->offset_factor*db->getWeightFactor();
  normalize = db->normalize;
}

void afxXM_VelocityOffset2_fixed::start(F32 timestamp)
{
  Parent::start(timestamp);

  cons = fx_wrapper->getAimConstraint();
  if (!cons)
    Con::errorf(ConsoleLogEntry::General, 
                "afxXM_VelocityOffset: failed to find a SceneObject derived constraint source.");
}

void afxXM_VelocityOffset2_fixed::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (!cons)
    return;

  SceneObject* scene_obj = cons->getSceneObject();
  if (scene_obj)
  {
    Point3F vel_vec = scene_obj->getVelocity();
    if (!vel_vec.isZero())
    {
      if (normalize)
        vel_vec.normalize(offset_factor);
      params.pos2 += vel_vec*offset_factor;
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//