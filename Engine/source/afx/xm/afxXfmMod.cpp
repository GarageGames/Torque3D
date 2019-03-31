
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

#include "afx/afxEffectWrapper.h"
#include "afx/util/afxEase.h"
#include "afx/xm/afxXfmMod.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

afxXM_Params::afxXM_Params()
{
  pos.zero();
  ori.identity();
  scale.set(1.0f,1.0f,1.0f);
  pos2.zero();
  color.set(0.0f,0.0f,0.0f,0.0f);
  vis = 0.0;
}

U32 afxXM_Params::getParameterOffset(U32 param, S32 component)
{
  switch (param)
  {

  case POSITION:
    switch (component)
    {
    case 0:
      return Offset(pos.x, afxXM_Params);
    case 1:
      return Offset(pos.y, afxXM_Params);
    case 2:
      return Offset(pos.z, afxXM_Params);
    default:
      return Offset(pos, afxXM_Params);
    }
    break;

  case ORIENTATION:
    return Offset(ori, afxXM_Params);
       
  case POSITION2:
    switch (component)
    {
    case 0:
      return Offset(pos2.x, afxXM_Params);
    case 1:
      return Offset(pos2.y, afxXM_Params);
    case 2:
      return Offset(pos2.z, afxXM_Params);
    default:
      return Offset(pos2, afxXM_Params);
    }
    break;
  
  case SCALE:
    switch (component)
    {
    case 0:
      return Offset(scale.x, afxXM_Params);
    case 1:
      return Offset(scale.y, afxXM_Params);
    case 2:
      return Offset(scale.z, afxXM_Params);
    default:
      return Offset(scale, afxXM_Params);
    }
    break;

  case COLOR:
    switch (component)
    {
    case 0:
      return Offset(color.red, afxXM_Params);
    case 1:
      return Offset(color.green, afxXM_Params);
    case 2:
      return Offset(color.blue, afxXM_Params);
    case 3:
      return Offset(color.alpha, afxXM_Params);
    default:
      return Offset(color, afxXM_Params);
    }
    break;

  case VISIBILITY:
    return Offset(vis, afxXM_Params);
  }

  return BAD_OFFSET;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// BASE CLASSES
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_BaseData);

afxXM_BaseData::afxXM_BaseData()
{
  ignore_time_factor = false;
}

afxXM_BaseData::afxXM_BaseData(const afxXM_BaseData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  ignore_time_factor = other.ignore_time_factor;
}

void afxXM_BaseData::initPersistFields()
{
  addField("ignoreTimeFactor",  TypeBool,   Offset(ignore_time_factor, afxXM_BaseData),
    "...");

  Parent::initPersistFields();

  Con::setIntVariable("$afxXfmMod::POS",            POSITION);
  Con::setIntVariable("$afxXfmMod::ORI",            ORIENTATION);
  Con::setIntVariable("$afxXfmMod::POS2",           POSITION2);
  Con::setIntVariable("$afxXfmMod::SCALE",          SCALE);
  Con::setIntVariable("$afxXfmMod::ALL_BUT_SCALE",  ALL_BUT_SCALE);
  Con::setIntVariable("$afxXfmMod::ALL",            ALL);
}

void afxXM_BaseData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->write(ignore_time_factor);
}

void afxXM_BaseData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&ignore_time_factor);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Base::afxXM_Base(afxXM_BaseData* db, afxEffectWrapper* fxw)
{
  fx_wrapper = fxw;
  time_factor = (db->ignore_time_factor) ? 1.0f : fxw->getTimeFactor();
  datablock = db;
}

afxXM_Base::~afxXM_Base()
{
  if (datablock && datablock->isTempClone())
    delete datablock;
  datablock = 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_WeightedBaseData);

afxXM_WeightedBaseData::afxXM_WeightedBaseData()
{
  delay = 0;
  lifetime = afxEffectDefs::INFINITE_LIFETIME;
  fade_in_time = 0;
  fade_out_time = 0;
  fadein_ease.set(0.0f, 1.0f);
  fadeout_ease.set(0.0f, 1.0f);
  life_bias = 1.0f;
}

afxXM_WeightedBaseData::afxXM_WeightedBaseData(const afxXM_WeightedBaseData& other, bool temp_clone) : afxXM_BaseData(other, temp_clone)
{
  delay = other.delay;
  lifetime = other.lifetime;
  fade_in_time = other.fade_in_time;
  fade_out_time = other.fade_out_time;
  fadein_ease = other.fadein_ease;
  fadeout_ease = other.fadeout_ease;
  life_bias = other.life_bias;
}

bool afxXM_WeightedBaseData::hasFixedWeight() const
{
  return (delay == 0.0f && lifetime == afxEffectDefs::INFINITE_LIFETIME && fade_in_time == 0.0f && fade_out_time == 0.0f);
}

F32 afxXM_WeightedBaseData::getWeightFactor() const
{
  return 1.0f;
}

void afxXM_WeightedBaseData::initPersistFields()
{
  addField("delay",         TypeF32,      Offset(delay,         afxXM_WeightedBaseData),
    "...");
  addField("lifetime",      TypeF32,      Offset(lifetime,      afxXM_WeightedBaseData),
    "...");
  addField("fadeInTime",    TypeF32,      Offset(fade_in_time,  afxXM_WeightedBaseData),
    "...");
  addField("fadeOutTime",   TypeF32,      Offset(fade_out_time, afxXM_WeightedBaseData),
    "...");
  addField("fadeInEase",    TypePoint2F,  Offset(fadein_ease,   afxXM_WeightedBaseData),
    "...");
  addField("fadeOutEase",   TypePoint2F,  Offset(fadeout_ease,  afxXM_WeightedBaseData),
    "...");
  addField("lifetimeBias",  TypeF32,      Offset(life_bias,     afxXM_WeightedBaseData),
    "...");

  Parent::initPersistFields();
}

void afxXM_WeightedBaseData::packData(BitStream* stream)
{
  Parent::packData(stream);

  if (stream->writeFlag(!hasFixedWeight()))
  {
    stream->write(delay);
    stream->write(lifetime);
    stream->write(fade_in_time);
    stream->write(fade_out_time);
    if (stream->writeFlag(fadein_ease.x != 0.0f || fadein_ease.y != 1.0f))
    {
      stream->writeFloat(fadein_ease.x, 16);
      stream->writeFloat(fadein_ease.y, 16);
    }
    if (stream->writeFlag(fadeout_ease.x != 0.0f || fadeout_ease.y != 1.0f))
    {
      stream->writeFloat(fadeout_ease.x, 16);
      stream->writeFloat(fadeout_ease.y, 16);
    }
    stream->write(life_bias);
  }
}

void afxXM_WeightedBaseData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  if (stream->readFlag()) // WEIGHTED?
  {
    stream->read(&delay);
    stream->read(&lifetime);
    stream->read(&fade_in_time);
    stream->read(&fade_out_time);
    if (stream->readFlag()) // FADE-IN EASED?
    {
      fadein_ease.x = stream->readFloat(16);
      fadein_ease.y = stream->readFloat(16);
    }
    else
      fadein_ease.set(0.0f, 1.0f);
    if (stream->readFlag()) // FADE-OUT EASED?
    {
      fadeout_ease.x = stream->readFloat(16);
      fadeout_ease.y = stream->readFloat(16);
    }
    else
      fadeout_ease.set(0.0f, 1.0f);
    stream->read(&life_bias);
  }
  else
  {
    delay = 0.0f;
    lifetime = afxEffectDefs::INFINITE_LIFETIME;
    fade_in_time = 0.0f;
    fade_out_time = 0.0f;
    fadein_ease.set(0.0f, 1.0f);
    fadeout_ease.set(0.0f, 1.0f);
    life_bias = 1.0f;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_WeightedBase::afxXM_WeightedBase(afxXM_WeightedBaseData* db, afxEffectWrapper* fxw)
: afxXM_Base(db, fxw) 
{
  wt_fadein = db->fade_in_time*db->life_bias;
  wt_fadeout = db->fade_out_time*db->life_bias;
  wt_fadein_ease = db->fadein_ease;
  wt_fadeout_ease = db->fadeout_ease;
  wt_start_time = db->delay;
  wt_full_time = wt_start_time + wt_fadein;
  wt_fade_time = wt_start_time + db->lifetime*db->life_bias;
  wt_done_time = wt_fade_time + wt_fadeout;
}

F32 afxXM_WeightedBase::calc_weight_factor(F32 elapsed)
{
  if (elapsed < wt_start_time)     // pre
    return 0.0f;
  else if (elapsed < wt_full_time) // fade-in
  {
    F32 t = (elapsed - wt_start_time)/wt_fadein;
    return afxEase::t(t, wt_fadein_ease.x, wt_fadein_ease.y);
  }
  else if (elapsed < wt_fade_time) // full
    return 1.0f;
  else if (elapsed < wt_done_time) // fade-out
  {
    F32 t = (wt_done_time - elapsed)/wt_fadeout;
    return afxEase::t(t, wt_fadeout_ease.x, wt_fadeout_ease.y);
  }
  else                             // post
    return 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//


