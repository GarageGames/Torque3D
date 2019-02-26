
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

#ifndef _AFX_XFM_WAVE_BASE_H_
#define _AFX_XFM_WAVE_BASE_H_

#include "afx/xm/afxXfmMod.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVEFORM

class afxXM_Waveform
{
public:
  virtual F32 evaluate(F32 t) = 0;
};

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveformSine : public afxXM_Waveform
{
public:
  virtual F32 evaluate(F32 t);
};

class afxXM_WaveformSquare : public afxXM_Waveform
{
public:
  virtual F32 evaluate(F32 t);
};

class afxXM_WaveformTriangle : public afxXM_Waveform
{
public:
  virtual F32 evaluate(F32 t);
};

class afxXM_WaveformSawtooth : public afxXM_Waveform
{
public:
  virtual F32 evaluate(F32 t) { return t; }
};

class afxXM_WaveformNoise : public afxXM_Waveform
{
public:
  virtual F32 evaluate(F32 t) { return gRandGen.randF(); };
};

class afxXM_WaveformOne : public afxXM_Waveform
{
public:
  virtual F32 evaluate(F32 t) { return 1.0f; };
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVE INTERPOLATOR

class afxXM_WaveInterp
{
public:
  virtual void interpolate(F32 t, afxXM_Params& params)=0;
  virtual void pulse()=0;

  static F32 lerp(F32 t, F32 a, F32 b) { return a + t * (b - a); }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVE BASE DATABLOCK

class afxXM_WaveBaseData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;
  friend class afxXM_WaveRiderBaseData;

public:
  enum  WaveFormType
  {
    WAVEFORM_NONE = 0,
    WAVEFORM_SINE,
    WAVEFORM_SQUARE,
    WAVEFORM_TRIANGLE,
    WAVEFORM_SAWTOOTH,
    WAVEFORM_NOISE,
    WAVEFORM_ONE,
    WAVEFORM_BITS = 3
  };

  enum WaveOpType
  {
    OP_ADD = 0,
    OP_MULTIPLY,
    OP_REPLACE,
    OP_BITS = 2
  };

  enum WaveParamType
  {
     PARAM_NONE = 0,
     PARAM_POS,
        PARAM_POS_X,
        PARAM_POS_Y,
        PARAM_POS_Z,
     PARAM_ORI,
     PARAM_POS2,
        PARAM_POS2_X,
        PARAM_POS2_Y,
        PARAM_POS2_Z,
     PARAM_SCALE,
        PARAM_SCALE_X,
        PARAM_SCALE_Y,
        PARAM_SCALE_Z,
     PARAM_COLOR,
        PARAM_COLOR_R,
        PARAM_COLOR_G,
        PARAM_COLOR_B,
        PARAM_COLOR_A,
     PARAM_VIS,
     PARAM_BITS = 5,
  };  
  
  U32             waveform_type;
  U32             parameter;
  U32             op;
  F32             speed;
  F32             speed_vari;
  F32             accel;
  F32             phase_shift;
  F32             duty_cycle;
  F32             duty_shift;
  F32             off_duty_t;

  ByteRange       waves_per_pulse;
  ByteRange       waves_per_rest;
  F32             rest_dur;
  F32             rest_dur_vari;

  Point3F         axis;
  bool            local_axis;

public:
  /*C*/           afxXM_WaveBaseData();
  /*C*/           afxXM_WaveBaseData(const afxXM_WaveBaseData&, bool = false);

  void            packData(BitStream* stream);
  void            unpackData(BitStream* stream);

  static void     initPersistFields();

  static void     initParamInfo(U32 parameter, U32& parambit, S32& component);
  static afxXM_Waveform* getWaveform(U32 waveform_type);

  DECLARE_CONOBJECT(afxXM_WaveBaseData);
  DECLARE_CATEGORY("AFX");
};

typedef afxXM_WaveBaseData::WaveFormType afxXM_WaveFormType;
DefineEnumType( afxXM_WaveFormType );

typedef afxXM_WaveBaseData::WaveOpType afxXM_WaveOpType;
DefineEnumType( afxXM_WaveOpType );

typedef afxXM_WaveBaseData::WaveParamType afxXM_WaveParamType;
DefineEnumType( afxXM_WaveParamType );

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVE RIDER BASE DATABLOCK

class afxXM_WaveRiderBaseData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;

public:  
  U32             waveform_type;
  F32             off_duty_t;
  U32             parameter;
  U32             op;

  Point3F         axis;
  bool            local_axis;

public:
  /*C*/           afxXM_WaveRiderBaseData();
  /*C*/           afxXM_WaveRiderBaseData(const afxXM_WaveRiderBaseData&, bool = false);

  void            packData(BitStream* stream);
  void            unpackData(BitStream* stream);

  static void     initPersistFields();

  DECLARE_CONOBJECT(afxXM_WaveRiderBaseData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVE BASE 

class afxXM_WaveBase : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;
  friend class afxXM_WaveRiderBase;

protected:
  static bool           last_was_pulsed;
  static bool           last_was_off_duty;
  static F32            last_t;
  static F32            last_wave_t;

  afxXM_WaveInterp*     interpolator;

protected:
  afxXM_Waveform*       waveform;

  afxXM_WaveBaseData*   db;
  F32                   speed;
  bool                  fixed_weight;

  bool                  speed_is_randomized;
  bool                  is_resting;
  F32                   cur_pulse_time;
  F32                   next_pulse_time;

  F32                   calc_initial_speed();
  F32                   calc_new_speed();
  F32                   calc_new_restDur();
  S32                   calc_new_wavesPerPulse();
  S32                   calc_new_wavesPerRest();

public:
  /*C*/                 afxXM_WaveBase(afxXM_WaveBaseData*, afxEffectWrapper*, afxXM_WaveInterp*);
  /*D*/                 ~afxXM_WaveBase();

  virtual void          updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

inline F32 afxXM_WaveBase::calc_initial_speed()
{
  return (!speed_is_randomized) ? 
                db->speed : 
                (db->speed + gRandGen.randF()*2.0f*db->speed_vari - db->speed_vari);
}

inline F32 afxXM_WaveBase::calc_new_speed()
{
  return mClampF((!speed_is_randomized) ? 
                (speed + speed*db->accel) : 
                (db->speed + gRandGen.randF()*2.0f*db->speed_vari - db->speed_vari), 0.001f, 200.0f);
}

inline F32 afxXM_WaveBase::calc_new_restDur()
{
  return db->rest_dur + gRandGen.randF()*2.0f*db->rest_dur_vari - db->rest_dur_vari;
}

inline S32 afxXM_WaveBase::calc_new_wavesPerPulse()
{
  return (db->waves_per_pulse.getSpan() == 0) ? 
                db->waves_per_pulse.low : 
                gRandGen.randI(db->waves_per_pulse.low, db->waves_per_pulse.high);
}

inline S32 afxXM_WaveBase::calc_new_wavesPerRest()
{
  return (db->waves_per_rest.getSpan() == 0) ? 
                db->waves_per_rest.low : 
                gRandGen.randI(db->waves_per_rest.low, db->waves_per_rest.high);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVE RIDER BASE 

class afxXM_WaveRiderBase : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

protected:
  afxXM_WaveInterp*         interpolator;
  afxXM_WaveRiderBaseData*  db;
  afxXM_Waveform*           waveform;
  bool                      fixed_weight;

public:
  /*C*/                 afxXM_WaveRiderBase(afxXM_WaveRiderBaseData*, afxEffectWrapper*, afxXM_WaveInterp*);
  /*D*/                 ~afxXM_WaveRiderBase();

  virtual void          updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_XFM_WAVE_BASE_H_

