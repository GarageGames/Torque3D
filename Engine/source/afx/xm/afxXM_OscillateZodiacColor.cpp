
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
#include "afx/xm/afxXfmMod.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxXM_OscillateZodiacColorData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;

public:
  ColorF        color_a;
  ColorF        color_b;
  F32           speed;

public:
  /*C*/         afxXM_OscillateZodiacColorData();

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);
  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_OscillateZodiacColorData);
  DECLARE_CATEGORY("AFX");
};

class afxXM_OscillateZodiacColor : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  ColorF        color_a;
  ColorF        color_b;
  F32           speed;

  ColorF*       liveColor_ptr;
  F32*          liveColorFactor_ptr;

public:
  /*C*/         afxXM_OscillateZodiacColor(afxXM_OscillateZodiacColorData*, afxEffectWrapper*);

  virtual void  update(F32 dt, F32 elapsed, Point3F& pos, MatrixF& ori, Point3F& pos2, 
                       Point3F& scale);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_OscillateZodiacColorData);

ConsoleDocClass( afxXM_OscillateZodiacColorData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_OscillateZodiacColorData::afxXM_OscillateZodiacColorData()
{
  color_a.set(0.0f, 0.0f, 0.0f, 0.0f);
  color_b.set(1.0f, 1.0f, 1.0f, 1.0f);
  speed = 1.0f;
}

void afxXM_OscillateZodiacColorData::initPersistFields()
{
  addField("colorA",              TypeColorF,   Offset(color_a,   afxXM_OscillateZodiacColorData),
    "...");
  addField("colorB",              TypeColorF,   Offset(color_b,   afxXM_OscillateZodiacColorData),
    "...");
  addField("speed",               TypeF32,      Offset(speed, afxXM_OscillateZodiacColorData),
    "...");

  Parent::initPersistFields();
}

void afxXM_OscillateZodiacColorData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(color_a);
  stream->write(color_b);
  stream->write(speed);
}

void afxXM_OscillateZodiacColorData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&color_a);
  stream->read(&color_b);
  stream->read(&speed);
}

afxXM_Base* afxXM_OscillateZodiacColorData::create(afxEffectWrapper* fx, bool on_server)
{
  return new afxXM_OscillateZodiacColor(this, fx);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_OscillateZodiacColor::afxXM_OscillateZodiacColor(afxXM_OscillateZodiacColorData* db, afxEffectWrapper* fxw) 
: afxXM_WeightedBase(db, fxw) 
{ 
  color_a = db->color_a;
  color_b = db->color_b;
  speed = db->speed;

  const AbstractClassRep::Field* field;
  field = fxw->getClassRep()->findField(StringTable->insert("liveColor"));
  if (field && field->type == TypeColorF)
    liveColor_ptr = (ColorF*)(((const char *)(fxw)) + field->offset);
  else
    liveColor_ptr = 0;

  field = fxw->getClassRep()->findField(StringTable->insert("liveColorFactor"));
  if (field && field->type == TypeF32)
    liveColorFactor_ptr = (F32*)(((const char *)(fxw)) + field->offset);
  else
    liveColorFactor_ptr = 0;
}

void afxXM_OscillateZodiacColor::update(F32 dt, F32 elapsed, Point3F& pos, MatrixF& ori, Point3F& pos2, Point3F& scale)
{
  F32 wt_factor = calc_weight_factor(elapsed);

  if (liveColor_ptr)
  {
    F32 t = (1.0f + mSin((3.0f*M_PI_F)/2.0f + speed*elapsed*M_2PI_F))*0.5f;
    liveColor_ptr->interpolate(color_a, color_b, t);
  }

  if (liveColorFactor_ptr)
    *liveColorFactor_ptr = wt_factor;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
