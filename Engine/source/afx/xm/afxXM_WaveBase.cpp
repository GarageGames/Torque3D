
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

#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/xm/afxXM_WaveBase.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_WaveBaseData);

ConsoleDocClass( afxXM_WaveBaseData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_WaveBaseData::afxXM_WaveBaseData()
{
  waveform_type = WAVEFORM_SINE;
  parameter = PARAM_NONE;
  op = OP_ADD;
  speed = 1.0f;
  speed_vari = 0.0;
  accel = 0.0f;
  phase_shift = 0.0f;
  duty_cycle = 1.0f;
  duty_shift = 0.0f;
  off_duty_t = 0.0f;

  waves_per_pulse.set(1,1);
  waves_per_rest.set(0,0);
  rest_dur = 0.0f;
  rest_dur_vari = 0.0f;

  axis.zero();
  local_axis = true;
}

afxXM_WaveBaseData::afxXM_WaveBaseData(const afxXM_WaveBaseData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
  waveform_type = other.waveform_type;
  parameter = other.parameter;
  op = other.op;
  speed = other.speed;
  speed_vari = other.speed;
  accel = other.accel;
  phase_shift = other.phase_shift;
  duty_cycle = other.duty_cycle;
  duty_shift = other.duty_shift;
  off_duty_t = other.off_duty_t;

  waves_per_pulse = other.waves_per_pulse;
  waves_per_rest = other.waves_per_rest;
  rest_dur = other.rest_dur;
  rest_dur_vari = other.rest_dur_vari;

  axis = other.axis;
  local_axis = true;
}

ImplementEnumType( afxXM_WaveFormType, "Possible waveform types.\n" "@ingroup afxXM_WaveBase\n\n" )
  { afxXM_WaveBaseData::WAVEFORM_NONE,      "none",      "..." },
  { afxXM_WaveBaseData::WAVEFORM_SINE,      "sine",      "..." },
  { afxXM_WaveBaseData::WAVEFORM_SQUARE,    "square",    "..." },
  { afxXM_WaveBaseData::WAVEFORM_TRIANGLE,  "triangle",  "..." },
  { afxXM_WaveBaseData::WAVEFORM_SAWTOOTH,  "sawtooth",  "..." },
  { afxXM_WaveBaseData::WAVEFORM_NOISE,     "noise",     "..." },
  { afxXM_WaveBaseData::WAVEFORM_ONE,       "one",       "..." },
EndImplementEnumType;

ImplementEnumType( afxXM_WaveParamType, "Possible wave parameter types.\n" "@ingroup afxXM_WaveBase\n\n" )
  { afxXM_WaveBaseData::PARAM_NONE,     "none",                "..." },
  { afxXM_WaveBaseData::PARAM_POS,      "pos",                 "..." },
  {   afxXM_WaveBaseData::PARAM_POS_X,    "pos.x",             "..." },
  {   afxXM_WaveBaseData::PARAM_POS_Y,    "pos.y",             "..." },
  {   afxXM_WaveBaseData::PARAM_POS_Z,    "pos.z",             "..." },
  { afxXM_WaveBaseData::PARAM_ORI,      "ori",                 "..." },
  { afxXM_WaveBaseData::PARAM_POS2,     "pos2",                "..." },
  {   afxXM_WaveBaseData::PARAM_POS2_X,   "pos2.x",            "..." },
  {   afxXM_WaveBaseData::PARAM_POS2_Y,   "pos2.y",            "..." },
  {   afxXM_WaveBaseData::PARAM_POS2_Z,   "pos2.z",            "..." },
  { afxXM_WaveBaseData::PARAM_SCALE,      "scale",             "..." },
  {   afxXM_WaveBaseData::PARAM_SCALE_X,      "scale.x",       "..." },
  {   afxXM_WaveBaseData::PARAM_SCALE_Y,      "scale.y",       "..." },
  {   afxXM_WaveBaseData::PARAM_SCALE_Z,      "scale.z",       "..." },
  { afxXM_WaveBaseData::PARAM_COLOR,      "color",             "..." },
  {   afxXM_WaveBaseData::PARAM_COLOR_R,      "color.red",     "..." },
  {   afxXM_WaveBaseData::PARAM_COLOR_G,      "color.green",   "..." },
  {   afxXM_WaveBaseData::PARAM_COLOR_B,      "color.blue",    "..." },
  {   afxXM_WaveBaseData::PARAM_COLOR_A,      "color.alpha",   "..." },
  { afxXM_WaveBaseData::PARAM_VIS,        "vis",               "..." },
  { afxXM_WaveBaseData::PARAM_POS,        "position",          "..." },
  {   afxXM_WaveBaseData::PARAM_POS_X,    "position.x",        "..." },
  {   afxXM_WaveBaseData::PARAM_POS_Y,    "position.y",        "..." },
  {   afxXM_WaveBaseData::PARAM_POS_Z,    "position.z",        "..." },
  { afxXM_WaveBaseData::PARAM_ORI,        "orientation",       "..." },
  { afxXM_WaveBaseData::PARAM_POS2,       "position2",         "..." },
  {   afxXM_WaveBaseData::PARAM_POS2_X,   "position2.x",       "..." },
  {   afxXM_WaveBaseData::PARAM_POS2_Y,   "position2.y",       "..." },
  {   afxXM_WaveBaseData::PARAM_POS2_Z,   "position2.z",       "..." },
  {   afxXM_WaveBaseData::PARAM_COLOR_R,  "color.r",           "..." },
  {   afxXM_WaveBaseData::PARAM_COLOR_G,  "color.g",           "..." },
  {   afxXM_WaveBaseData::PARAM_COLOR_B,  "color.b",           "..." },
  {   afxXM_WaveBaseData::PARAM_COLOR_A,  "color.a",           "..." },
  { afxXM_WaveBaseData::PARAM_VIS,        "visibility",        "..." },
EndImplementEnumType;

ImplementEnumType( afxXM_WaveOpType, "Possible wave operation types.\n" "@ingroup afxXM_WaveBase\n\n" )
  { afxXM_WaveBaseData::OP_ADD,      "add",        "..." },
  { afxXM_WaveBaseData::OP_MULTIPLY, "multiply",   "..." },
  { afxXM_WaveBaseData::OP_REPLACE,  "replace",    "..." },
  { afxXM_WaveBaseData::OP_MULTIPLY, "mult",       "..." },
EndImplementEnumType;

void afxXM_WaveBaseData::initPersistFields()
{
  addField("waveform",      TYPEID< afxXM_WaveBaseData::WaveFormType >(),  Offset(waveform_type, afxXM_WaveBaseData),
    "...");
  addField("parameter",     TYPEID< afxXM_WaveBaseData::WaveParamType >(), Offset(parameter, afxXM_WaveBaseData),
    "...");
  addField("op",            TYPEID< afxXM_WaveBaseData::WaveOpType >(),    Offset(op, afxXM_WaveBaseData),
    "...");

  addField("speed",         TypeF32,      Offset(speed, afxXM_WaveBaseData), 
    "waves per second");
  addField("speedVariance", TypeF32,      Offset(speed_vari, afxXM_WaveBaseData),
    "...");
  addField("acceleration",  TypeF32,      Offset(accel, afxXM_WaveBaseData),
    "...");
  addField("phaseShift",    TypeF32,      Offset(phase_shift, afxXM_WaveBaseData),
    "...");
  addField("dutyCycle",     TypeF32,      Offset(duty_cycle, afxXM_WaveBaseData),
    "...");
  addField("dutyShift",     TypeF32,      Offset(duty_shift, afxXM_WaveBaseData),
    "...");
  addField("offDutyT",      TypeF32,      Offset(off_duty_t, afxXM_WaveBaseData),
    "...");

  addField("wavesPerPulse", TypeByteRange2,      Offset(waves_per_pulse, afxXM_WaveBaseData),
    "...");
  addField("wavesPerRest",  TypeByteRange2,      Offset(waves_per_rest, afxXM_WaveBaseData),
    "...");
  addField("restDuration",          TypeF32,      Offset(rest_dur, afxXM_WaveBaseData),
    "...");
  addField("restDurationVariance",  TypeF32,      Offset(rest_dur_vari, afxXM_WaveBaseData),
    "...");

  addField("axis",          TypePoint3F,  Offset(axis, afxXM_WaveBaseData),
    "...");
  addField("axisIsLocal",   TypeBool,     Offset(local_axis, afxXM_WaveBaseData),
    "...");

  Parent::initPersistFields();
}

void afxXM_WaveBaseData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->writeInt(waveform_type, WAVEFORM_BITS);
  stream->writeInt(parameter, PARAM_BITS);
  stream->writeInt(op, OP_BITS);
  stream->write(speed);
  stream->write(speed_vari);
  stream->write(accel);
  stream->write(phase_shift);
  stream->write(duty_cycle);
  stream->write(duty_shift);
  stream->write(off_duty_t);

  stream->write(waves_per_pulse.low);
  stream->write(waves_per_pulse.high);
  stream->write(waves_per_rest.low);
  stream->write(waves_per_rest.high);

  stream->write(rest_dur);
  stream->write(rest_dur_vari);

  mathWrite(*stream, axis);
  stream->writeFlag(local_axis);
}

void afxXM_WaveBaseData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  waveform_type = stream->readInt(WAVEFORM_BITS);
  parameter = stream->readInt(PARAM_BITS);
  op = stream->readInt(OP_BITS);
  stream->read(&speed);
  stream->read(&speed_vari);
  stream->read(&accel);
  stream->read(&phase_shift);
  stream->read(&duty_cycle);
  stream->read(&duty_shift);
  stream->read(&off_duty_t);

  stream->read(&waves_per_pulse.low);
  stream->read(&waves_per_pulse.high);
  stream->read(&waves_per_rest.low);
  stream->read(&waves_per_rest.high);

  stream->read(&rest_dur);
  stream->read(&rest_dur_vari);

  mathRead(*stream, &axis);
  local_axis = stream->readFlag();
}

void afxXM_WaveBaseData::initParamInfo(U32 parameter, U32& parambit, S32& component)
{
  switch (parameter)
  {
  case PARAM_POS:
    parambit = POSITION;
    component = -1;
    break;
  case PARAM_POS_X:
    parambit = POSITION;
    component = 0;
    break;
  case PARAM_POS_Y:
    parambit = POSITION;
    component = 1;
    break;
  case PARAM_POS_Z:
    parambit = POSITION;
    component = 2;
    break;

  case PARAM_ORI:
    parambit = ORIENTATION;
    component = -1;
    break;

  case PARAM_POS2:
    parambit = POSITION2;
    component = -1;
    break;
  case PARAM_POS2_X:
    parambit = POSITION2;
    component = 0;
    break;
  case PARAM_POS2_Y:
    parambit = POSITION2;
    component = 1;
    break;
  case PARAM_POS2_Z:
    parambit = POSITION2;
    component = 2;
    break;

  case PARAM_SCALE:
    parambit = SCALE;
    component = -1;
    break;
  case PARAM_SCALE_X:
    parambit = SCALE;
    component = 0;
    break;
  case PARAM_SCALE_Y:
    parambit = SCALE;
    component = 1;
    break;
  case PARAM_SCALE_Z:
    parambit = SCALE;
    component = 2;
    break;

  case PARAM_COLOR:
    parambit = COLOR;
    component = -1;
    break;
  case PARAM_COLOR_R:
    parambit = COLOR;
    component = 0;
    break;
  case PARAM_COLOR_G:
    parambit = COLOR;
    component = 1;
    break;
  case PARAM_COLOR_B:
    parambit = COLOR;
    component = 2;
    break;
  case PARAM_COLOR_A:
    parambit = COLOR;
    component = 3;
    break;

  case PARAM_VIS:
    parambit = VISIBILITY;
    component = -1;
    break;

  default:
    parambit = 0;
    component = -1;
    break;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_WaveRiderBaseData);

ConsoleDocClass( afxXM_WaveRiderBaseData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_WaveRiderBaseData::afxXM_WaveRiderBaseData()
{
  waveform_type = afxXM_WaveBaseData::WAVEFORM_NONE;
  parameter = afxXM_WaveBaseData::PARAM_NONE;
  op = afxXM_WaveBaseData::OP_ADD;
  off_duty_t = 0.0f;

  axis.zero();
  local_axis = true;
}

afxXM_WaveRiderBaseData::afxXM_WaveRiderBaseData(const afxXM_WaveRiderBaseData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
  waveform_type = other.waveform_type;
  parameter = other.parameter;
  op = other.op;
  off_duty_t = other.off_duty_t;

  axis = other.axis;
  local_axis = true;
}

void afxXM_WaveRiderBaseData::initPersistFields()
{
  addField("waveform",      TYPEID< afxXM_WaveBaseData::WaveFormType >(),  Offset(waveform_type, afxXM_WaveRiderBaseData),
    "...");
  addField("parameter",     TYPEID< afxXM_WaveBaseData::WaveParamType >(), Offset(parameter, afxXM_WaveRiderBaseData),
    "...");
  addField("op",            TYPEID< afxXM_WaveBaseData::WaveOpType >(),    Offset(op, afxXM_WaveRiderBaseData),
    "...");

  addField("offDutyT",      TypeF32,      Offset(off_duty_t, afxXM_WaveRiderBaseData),
    "...");

  addField("axis",          TypePoint3F,  Offset(axis, afxXM_WaveRiderBaseData),
    "...");
  addField("axisIsLocal",   TypeBool,     Offset(local_axis, afxXM_WaveRiderBaseData),
    "...");

  Parent::initPersistFields();
}

void afxXM_WaveRiderBaseData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->writeInt(waveform_type, afxXM_WaveBaseData::WAVEFORM_BITS);
  stream->writeInt(parameter, afxXM_WaveBaseData::PARAM_BITS);
  stream->writeInt(op, afxXM_WaveBaseData::OP_BITS);
  stream->write(off_duty_t);

  mathWrite(*stream, axis);
  stream->writeFlag(local_axis);
}

void afxXM_WaveRiderBaseData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  waveform_type = stream->readInt(afxXM_WaveBaseData::WAVEFORM_BITS);
  parameter = stream->readInt(afxXM_WaveBaseData::PARAM_BITS);
  op = stream->readInt(afxXM_WaveBaseData::OP_BITS);
  stream->read(&off_duty_t);

  mathRead(*stream, &axis);
  local_axis = stream->readFlag();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//
// WAVEFORMS

F32 afxXM_WaveformSine::evaluate(F32 t)
{
  t = (0.75f + t)*Float_2Pi;
  return 0.5f*(1.0f + mSin(t));
}

F32 afxXM_WaveformSquare::evaluate(F32 t)
{
  return (t < 0.25f || t >= 0.75) ? 0.0f : 1.0f;
}

F32 afxXM_WaveformTriangle::evaluate(F32 t)
{
  return (t < 0.5f) ? 2.0f*t : 2.0f*(1.0f - t); 
}

//~~~~~~~~~~~~~~~~~~~~//

afxXM_Waveform* afxXM_WaveBaseData::getWaveform(U32 waveform_type)
{ 
  static afxXM_WaveformSine sine;
  static afxXM_WaveformSquare square;
  static afxXM_WaveformTriangle triangle;
  static afxXM_WaveformSawtooth sawtooth;
  static afxXM_WaveformNoise noise;
  static afxXM_WaveformOne one;

  switch (waveform_type)
  {
  case WAVEFORM_SINE:
    return &sine; 
  case WAVEFORM_SQUARE:
    return &square; 
  case WAVEFORM_TRIANGLE:
    return &triangle; 
  case WAVEFORM_SAWTOOTH:
    return &sawtooth; 
  case WAVEFORM_NOISE:
    return &noise; 
  case WAVEFORM_ONE:
    return &one; 
  default:
    // error condition
    return &sine; 
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

bool afxXM_WaveBase::last_was_pulsed = false;
bool afxXM_WaveBase::last_was_off_duty = true;
F32 afxXM_WaveBase::last_t = 0.0f;
F32 afxXM_WaveBase::last_wave_t = 0.0f;

afxXM_WaveBase::afxXM_WaveBase(afxXM_WaveBaseData* db, afxEffectWrapper* fxw, afxXM_WaveInterp* interp) 
  : afxXM_WeightedBase(db, fxw) 
{ 
  this->db = db;
  interpolator = interp;
  waveform = afxXM_WaveBaseData::getWaveform(db->waveform_type); 

  speed_is_randomized = !mIsZero(db->speed_vari);
  speed = calc_initial_speed();

  fixed_weight = db->hasFixedWeight();

  is_resting = false;
  cur_pulse_time = db->delay;
  next_pulse_time = cur_pulse_time + ((F32)calc_new_wavesPerPulse())/speed;
  interpolator->pulse();

  last_was_pulsed = false;
  last_was_off_duty = true;
  last_t = 0.0f;
  last_wave_t = 0.0f;
}

afxXM_WaveBase::~afxXM_WaveBase() 
{ 
  delete interpolator;
}

void afxXM_WaveBase::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  elapsed -= db->delay;

  if (elapsed > next_pulse_time)
  {
    is_resting = !is_resting;
    cur_pulse_time = next_pulse_time;

    if (is_resting)
    {
      F32 rest_dt = ((F32)(calc_new_wavesPerRest())/speed) + calc_new_restDur();
      if (rest_dt < 0.01)
        is_resting = false;
      else
        next_pulse_time = cur_pulse_time + rest_dt;
    }

    if (!is_resting)
    {
      speed = calc_new_speed();
      next_pulse_time = cur_pulse_time + ((F32)calc_new_wavesPerPulse())/speed;
      interpolator->pulse();
      last_was_pulsed = true;
    }
  }

  if (is_resting)
  {
    last_was_off_duty = true;
    interpolator->interpolate(db->off_duty_t, params);
    return;
  }

  F32 n_waves = db->phase_shift + (elapsed - cur_pulse_time)*speed;
  F32 wave_t = (n_waves - mFloor(n_waves))/db->duty_cycle;

  // we are beyond the duty portion of the wave, use off_duty_t 
  if (wave_t > 1.0f)
  {
    last_was_off_duty = true;
    interpolator->interpolate(db->off_duty_t, params);
    return;
  }

  if (db->duty_shift > 0.0f)
  {
    wave_t += db->duty_shift;
    if (wave_t > 1.0)
      wave_t -= 1.0f;
  }

  last_was_off_duty = false;
  last_wave_t = wave_t;

  last_t = waveform->evaluate(wave_t);
  
  if (fixed_weight)
  {
    interpolator->interpolate(last_t, params);
  }
  else
  {
    F32 wt_factor = calc_weight_factor(elapsed);
    F32 final_t = afxXM_WaveInterp::lerp(wt_factor, db->off_duty_t, last_t);
    interpolator->interpolate(final_t, params);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

afxXM_WaveRiderBase::afxXM_WaveRiderBase(afxXM_WaveRiderBaseData* db, afxEffectWrapper* fxw, afxXM_WaveInterp* interp) 
  : afxXM_WeightedBase(db, fxw) 
{ 
  this->db = db;
  interpolator = interp;
  waveform = afxXM_WaveBaseData::getWaveform(db->waveform_type); 
  fixed_weight = db->hasFixedWeight();
  interpolator->pulse();
}

afxXM_WaveRiderBase::~afxXM_WaveRiderBase() 
{ 
  delete interpolator;
}

void afxXM_WaveRiderBase::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (afxXM_WaveBase::last_was_pulsed)
    interpolator->pulse();

  if (afxXM_WaveBase::last_was_off_duty)
  {
    interpolator->interpolate(db->off_duty_t, params);
    return;
  }

  F32 t;
  if (db->waveform_type != afxXM_WaveBaseData::WAVEFORM_NONE)
    t = waveform->evaluate(afxXM_WaveBase::last_wave_t);
  else
    t = afxXM_WaveBase::last_t;

  if (fixed_weight)
    interpolator->interpolate(t, params);
  else
  {
    F32 wt_factor = calc_weight_factor(elapsed);
    F32 final_t = afxXM_WaveInterp::lerp(wt_factor, db->off_duty_t, t);
    interpolator->interpolate(final_t, params);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//