
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
#include "afx/xm/afxXfmMod.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxXM_BoxAdaptData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;

public:
  F32           scale_factor;
  Point2F       dim_range;

public:
  /*C*/         afxXM_BoxAdaptData();
  /*C*/         afxXM_BoxAdaptData(const afxXM_BoxAdaptData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);
  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_BoxAdaptData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxConstraint;

class afxXM_BoxAdapt : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  F32             scale_factor;
  Point2F         dim_range;

public:
  /*C*/           afxXM_BoxAdapt(afxXM_BoxAdaptData*, afxEffectWrapper*);

  virtual void    updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_BoxAdaptData);

ConsoleDocClass( afxXM_BoxAdaptData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_BoxAdaptData::afxXM_BoxAdaptData()
{
  scale_factor = 1.0f;
  dim_range.set(0.1f, 1000.0f);
}

afxXM_BoxAdaptData::afxXM_BoxAdaptData(const afxXM_BoxAdaptData& other, bool temp_clone) 
  : afxXM_WeightedBaseData(other, temp_clone)
{
  scale_factor = other.scale_factor;
  dim_range = other.dim_range;
}

void afxXM_BoxAdaptData::initPersistFields()
{
  addField("scaleFactor",     TypeF32,        Offset(scale_factor, afxXM_BoxAdaptData),
    "...");
  addField("dimensionRange",  TypePoint2F,    Offset(dim_range, afxXM_BoxAdaptData),
    "...");

  Parent::initPersistFields();
}

void afxXM_BoxAdaptData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(scale_factor);
  mathWrite(*stream, dim_range);
}

void afxXM_BoxAdaptData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&scale_factor);
  mathRead(*stream, &dim_range);
}

afxXM_Base* afxXM_BoxAdaptData::create(afxEffectWrapper* fx, bool on_server)
{
  return new afxXM_BoxAdapt(this, fx);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_BoxAdapt::afxXM_BoxAdapt(afxXM_BoxAdaptData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw)
{ 
  scale_factor = db->scale_factor;

  dim_range = db->dim_range;

  dim_range.x = getMax(0.001f, dim_range.x);
  dim_range.y = getMax(0.001f, dim_range.y);

  if (dim_range.x > dim_range.y)
  {
    F32 tmp = dim_range.y;
    dim_range.y = dim_range.x;
    dim_range.x = tmp;
  }
}

void afxXM_BoxAdapt::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  afxConstraint* pos_cons = fx_wrapper->getPosConstraint();
  if (!pos_cons)
    return;
      
  SceneObject* obj = pos_cons->getSceneObject();
  if (!obj)
    return;

  F32 wt_factor = calc_weight_factor(elapsed);

  const Box3F& obj_box = obj->getObjBox();
  const VectorF obj_scale = obj->getScale();

  F32 x_dim = obj_box.len_x()*obj_scale.x;
  F32 y_dim = obj_box.len_y()*obj_scale.y;

  F32 dim = mClampF(getMax(x_dim, y_dim), dim_range.x, dim_range.y);
  dim *= scale_factor*wt_factor*0.5f;

  //Con::printf("SET liveScaleFactor=%g x_dim=%g, y_dim=%g", dim, obj_box.len_x(), obj_box.len_y());
  fx_wrapper->setField("liveScaleFactor", avar("%g", dim));
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//