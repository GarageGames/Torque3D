
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

class afxXM_OscillateData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;

public:
  U32           mask;
  Point3F       min;
  Point3F       max;
  F32           speed;
  Point3F       axis;
  bool          additive_scale;
  bool          local_offset;

public:
  /*C*/         afxXM_OscillateData();
  /*C*/         afxXM_OscillateData(const afxXM_OscillateData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_OscillateData);
  DECLARE_CATEGORY("AFX");
};

class afxXM_Oscillate_rot : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  afxXM_OscillateData* db;

public:
  /*C*/         afxXM_Oscillate_rot(afxXM_OscillateData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

class afxXM_Oscillate_scale : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  afxXM_OscillateData* db;

public:
  /*C*/         afxXM_Oscillate_scale(afxXM_OscillateData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

class afxXM_Oscillate_position : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  afxXM_OscillateData* db;

public:
  /*C*/         afxXM_Oscillate_position(afxXM_OscillateData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

class afxXM_Oscillate_position2 : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  afxXM_OscillateData* db;

public:
  /*C*/         afxXM_Oscillate_position2(afxXM_OscillateData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

class afxXM_Oscillate : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  afxXM_OscillateData* db;

public:
  /*C*/         afxXM_Oscillate(afxXM_OscillateData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_OscillateData);

ConsoleDocClass( afxXM_OscillateData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_OscillateData::afxXM_OscillateData()
{
  mask = POSITION;
  min.set(0,0,0);
  max.set(1,1,1);
  speed = 1.0f;
  axis.set(0,0,1);
  additive_scale = false;
  local_offset = true;
}

afxXM_OscillateData::afxXM_OscillateData(const afxXM_OscillateData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
  mask = other.mask;
  min = other.min;
  max = other.max;
  speed = other.speed;
  axis = other.axis;
  additive_scale = other.additive_scale;
  local_offset = other.local_offset;
}

void afxXM_OscillateData::initPersistFields()
{
  addField("mask",                TypeS32,      Offset(mask,  afxXM_OscillateData),
    "...");
  addField("min",                 TypePoint3F,  Offset(min,   afxXM_OscillateData),
    "...");
  addField("max",                 TypePoint3F,  Offset(max,   afxXM_OscillateData),
    "...");
  addField("speed",               TypeF32,      Offset(speed, afxXM_OscillateData),
    "...");
  addField("axis",                TypePoint3F,  Offset(axis,  afxXM_OscillateData),
    "...");
  addField("additiveScale",       TypeBool,     Offset(additive_scale, afxXM_OscillateData),
    "...");
  addField("localOffset",         TypeBool,     Offset(local_offset,   afxXM_OscillateData),
    "...");

  Parent::initPersistFields();
}

void afxXM_OscillateData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(mask);
  mathWrite(*stream, min);
  mathWrite(*stream, max);
  stream->write(speed);
  mathWrite(*stream, axis);
  stream->writeFlag(additive_scale);
  stream->writeFlag(local_offset);
}

void afxXM_OscillateData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&mask);
  mathRead(*stream, &min);
  mathRead(*stream, &max);
  stream->read(&speed);
  mathRead(*stream, &axis);
  additive_scale = stream->readFlag();
  local_offset = stream->readFlag();
}

afxXM_Base* afxXM_OscillateData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_OscillateData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_OscillateData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  if (datablock->mask == ORIENTATION)
    return new afxXM_Oscillate_rot(datablock, fx);
  if (datablock->mask == SCALE)
	  return new afxXM_Oscillate_scale(datablock, fx);
  if (datablock->mask == POSITION)
    return new afxXM_Oscillate_position(datablock, fx);
  if (datablock->mask == POSITION2)
    return new afxXM_Oscillate_position2(datablock, fx);
  return new afxXM_Oscillate(datablock, fx);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

inline F32 lerp(F32 t, F32 a, F32 b)
{
  return a + t * (b - a);
}

inline Point3F lerpV(F32 t, const Point3F& a, const Point3F& b)
{
  return Point3F( a.x + t * (b.x - a.x),
                  a.y + t * (b.y - a.y),
                  a.z + t * (b.z - a.z) );
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Oscillate_rot::afxXM_Oscillate_rot(afxXM_OscillateData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  this->db = db; 
}

void afxXM_Oscillate_rot::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  F32 wt_factor = calc_weight_factor(elapsed);

  F32 t = mSin(db->speed*elapsed);  // [-1,1]
  F32 theta = lerp((t+1)/2, db->min.x*wt_factor, db->max.x*wt_factor);
  theta = mDegToRad(theta);

  AngAxisF rot_aa(db->axis, theta);
  MatrixF rot_xfm; rot_aa.setMatrix(&rot_xfm);
  
  params.ori.mul(rot_xfm);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Oscillate_scale::afxXM_Oscillate_scale(afxXM_OscillateData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  this->db = db; 
}

void afxXM_Oscillate_scale::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  F32 wt_factor = calc_weight_factor(elapsed);

  F32 t = mSin(db->speed*elapsed);  // [-1,1]
  F32 s = lerp((t+1)/2, db->min.x*wt_factor, db->max.x*wt_factor);
  Point3F xm_scale = db->axis*s;
  
  if (db->additive_scale)
    params.scale += xm_scale;
  else
    params.scale *= xm_scale;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Oscillate_position::afxXM_Oscillate_position(afxXM_OscillateData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  this->db = db; 
}

void afxXM_Oscillate_position::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  F32 wt_factor = calc_weight_factor(elapsed);

  F32 t = mSin(db->speed*elapsed);  // [-1,1]
  Point3F offset = lerpV(t, db->min*wt_factor, db->max*wt_factor);
  
  if (db->local_offset)
  {
    params.ori.mulV(offset);
    params.pos += offset;
  }
  else
    params.pos += offset;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Oscillate_position2::afxXM_Oscillate_position2(afxXM_OscillateData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  this->db = db; 
}

void afxXM_Oscillate_position2::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  F32 wt_factor = calc_weight_factor(elapsed);

  F32 t = mSin(db->speed*elapsed);  // [-1,1]
  Point3F offset = lerpV(t, db->min*wt_factor, db->max*wt_factor);

  params.pos2 += offset;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Oscillate::afxXM_Oscillate(afxXM_OscillateData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  this->db = db; 
}

void afxXM_Oscillate::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  F32 wt_factor = calc_weight_factor(elapsed);

  F32 t = mSin(db->speed*elapsed);  // [-1,1]

  if (db->mask & POSITION)
  {
    Point3F offset = lerpV(t, db->min*wt_factor, db->max*wt_factor);
    if (db->local_offset)
    {
      params.ori.mulV(offset);
      params.pos += offset;
    }
    else
      params.pos += offset;
  }

  if (db->mask & POSITION2)
  {
    Point3F offset = lerpV(t, db->min*wt_factor, db->max*wt_factor);
    params.pos2 += offset;
  }

  if (db->mask & SCALE)
  {
    F32 s = lerp((t+1)/2, db->min.x*wt_factor, db->max.x*wt_factor);
    Point3F xm_scale = db->axis*s;
    if (db->additive_scale)
      params.scale += xm_scale;
    else
      params.scale *= xm_scale;
  }
  
  if (db->mask & ORIENTATION)
  {
    F32 theta = lerp((t+1)/2, db->min.x*wt_factor, db->max.x*wt_factor);
    theta = mDegToRad(theta);
    AngAxisF rot_aa(db->axis, theta);
    MatrixF rot_xfm; rot_aa.setMatrix(&rot_xfm);
    params.ori.mul(rot_xfm);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

