
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

#ifndef _AFX_XFM_MOD_BASE_H_
#define _AFX_XFM_MOD_BASE_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "math/mPoint3.h"
#include "math/mMatrix.h"
#include "math/mMathFn.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// BASE CLASSES
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class BitStream;
class afxEffectWrapper;

class afxXM_Defs
{
public:
  enum
  {
    POSITION      = BIT(0),
    ORIENTATION   = BIT(1),
    POSITION2     = BIT(2),
    SCALE         = BIT(3),
    COLOR         = BIT(4),
    VISIBILITY    = BIT(5),
    ALL_BUT_SCALE = (POSITION | ORIENTATION | POSITION2),
    ALL           = (ALL_BUT_SCALE | SCALE),
  };
};

struct afxXM_Params : public afxXM_Defs
{
  Point3F   pos;
  MatrixF   ori;
  Point3F   scale;
  Point3F   pos2;
  ColorF    color;
  F32       vis;

  enum { BAD_OFFSET = S32_MAX };

  static U32 getParameterOffset(U32 param, S32 component=-1);
  afxXM_Params();
};

class afxXM_Base;

class afxXM_BaseData : public  GameBaseData, public afxXM_Defs
{
  typedef GameBaseData Parent;

public:
  bool          ignore_time_factor;

public:
  /*C*/         afxXM_BaseData();
  /*C*/         afxXM_BaseData(const afxXM_BaseData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  static void   initPersistFields();

  virtual afxXM_Base* create(afxEffectWrapper* fx, bool on_server) { return 0; }

  DECLARE_CONOBJECT(afxXM_BaseData);
  DECLARE_CATEGORY("AFX");
};

class afxXM_Base : public afxXM_Defs
{
protected:
  afxEffectWrapper* fx_wrapper;
  afxXM_BaseData*   datablock;
  F32               time_factor;

public:
  /*C*/             afxXM_Base(afxXM_BaseData*, afxEffectWrapper*);
  virtual           ~afxXM_Base();

  virtual void      start(F32 timestamp) { }
  virtual void      updateParams(F32 dt, F32 elapsed, afxXM_Params& p);

  // old update form for backwards compatibility (deprecated)
  virtual void      update(F32 dt, F32 elapsed, Point3F& pos, MatrixF& ori, Point3F& pos2, Point3F& scale) { };
};

// New subclasses should define own updateParams() and should *not* call this thru Parent. 
// This calls old form of update() for backwards compatibility.
inline void afxXM_Base::updateParams(F32 dt, F32 elapsed, afxXM_Params& p) 
{ 
  update(dt, elapsed, p.pos, p.ori, p.pos2, p.scale); 
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxXM_WeightedBaseData : public  afxXM_BaseData
{
  typedef afxXM_BaseData Parent;

public:
  F32           lifetime;
  F32           delay;
  F32           fade_in_time;
  F32           fade_out_time;
  Point2F       fadein_ease;
  Point2F       fadeout_ease;
  F32           life_bias;

public:
  /*C*/         afxXM_WeightedBaseData();
  /*C*/         afxXM_WeightedBaseData(const afxXM_WeightedBaseData&, bool = false);

  bool          hasFixedWeight() const;
  F32           getWeightFactor() const;

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  static void   initPersistFields();

  DECLARE_CONOBJECT(afxXM_WeightedBaseData);
  DECLARE_CATEGORY("AFX");
};

class afxXM_WeightedBase : public afxXM_Base
{
  typedef afxXM_Base Parent;

protected:
  F32           wt_fadein;
  F32           wt_fadeout;
  Point2F       wt_fadein_ease;
  Point2F       wt_fadeout_ease;
  F32           wt_start_time;
  F32           wt_full_time;
  F32           wt_fade_time;
  F32           wt_done_time;

  F32           calc_weight_factor(F32 elapsed);

public:
  /*C*/         afxXM_WeightedBase(afxXM_WeightedBaseData*, afxEffectWrapper*);
  virtual       ~afxXM_WeightedBase() { }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_XFM_MOD_BASE_H_
