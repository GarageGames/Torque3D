
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
#include "afx/afxChoreographer.h"
#include "afx/xm/afxXfmMod.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxXM_FreezeData : public afxXM_BaseData
{
  typedef afxXM_BaseData Parent;

public:
  U32           mask;
  F32           delay;

public:
  /*C*/         afxXM_FreezeData() : mask(POSITION | ORIENTATION | POSITION2), delay(0.0f) { }
  /*C*/         afxXM_FreezeData(const afxXM_FreezeData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_FreezeData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_Freeze : public afxXM_Base
{
  typedef afxXM_Base Parent;

  U32           mask;
  bool          first;
  Point3F       frozen_pos;
  MatrixF       frozen_ori;
  Point3F       frozen_aim;
  F32           delay;

public:
  /*C*/         afxXM_Freeze(afxXM_FreezeData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_Freeze_all_but_scale : public afxXM_Base
{
  typedef afxXM_Base Parent;

  bool          first;
  Point3F       frozen_pos;
  MatrixF       frozen_ori;
  Point3F       frozen_aim;
  F32           delay;

public:
  /*C*/         afxXM_Freeze_all_but_scale(afxXM_FreezeData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_Freeze_pos : public afxXM_Base
{
  typedef afxXM_Base Parent;

  bool          first;
  Point3F       frozen_pos;
  F32           delay;

public:
  /*C*/         afxXM_Freeze_pos(afxXM_FreezeData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_Freeze_pos2 : public afxXM_Base
{
  typedef afxXM_Base Parent;

  bool          first;
  Point3F       frozen_pos2;
  F32           delay;

public:
  /*C*/         afxXM_Freeze_pos2(afxXM_FreezeData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_Freeze_ori : public afxXM_Base
{
  typedef afxXM_Base Parent;

  bool          first;
  MatrixF       frozen_ori;
  F32           delay;

public:
  /*C*/         afxXM_Freeze_ori(afxXM_FreezeData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_FreezeData);

ConsoleDocClass( afxXM_FreezeData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_FreezeData::afxXM_FreezeData(const afxXM_FreezeData& other, bool temp_clone) : afxXM_BaseData(other, temp_clone)
{
  mask = other.mask;
  delay = other.delay;
}

void afxXM_FreezeData::initPersistFields()
{
  addField("mask",  TypeS32,    Offset(mask, afxXM_FreezeData),
    "...");
  addField("delay", TypeF32,    Offset(delay, afxXM_FreezeData),
    "...");

  Parent::initPersistFields();
}

void afxXM_FreezeData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(mask);
  stream->write(delay);
}

void afxXM_FreezeData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&mask);
  stream->read(&delay);
}

afxXM_Base* afxXM_FreezeData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_FreezeData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_FreezeData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  if (datablock->mask == ALL_BUT_SCALE)
    return new afxXM_Freeze_all_but_scale(datablock, fx);
  if (datablock->mask == POSITION)
    return new afxXM_Freeze_pos(datablock, fx);
  if (datablock->mask == ORIENTATION)
    return new afxXM_Freeze_ori(datablock, fx);
  if (datablock->mask == POSITION2)
    return new afxXM_Freeze_pos2(datablock, fx);
  return new afxXM_Freeze(datablock, fx);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Freeze::afxXM_Freeze(afxXM_FreezeData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw)
{ 
  mask = db->mask;
  first = true;
  delay = db->delay;
}

void afxXM_Freeze::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (elapsed < delay) return;

  if (first)
  {
    if (mask & POSITION)
      frozen_pos = params.pos;
    if (mask & ORIENTATION)
      frozen_ori = params.ori;
    if (mask & POSITION2)
      frozen_aim = params.pos2;
    first = false;
  }
  else
  {
    if (mask & POSITION)
      params.pos = frozen_pos;
    if (mask & ORIENTATION)
      params.ori = frozen_ori;
    if (mask & POSITION2)
      params.pos2 = frozen_aim;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Freeze_all_but_scale::afxXM_Freeze_all_but_scale(afxXM_FreezeData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw)
{ 
  first = true;
  delay = db->delay;
}

void afxXM_Freeze_all_but_scale::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (elapsed < delay) return;

  if (first)
  {
    frozen_pos = params.pos;
    frozen_ori = params.ori;
    frozen_aim = params.pos2;
    first = false;
  }
  else
  {
    params.pos = frozen_pos;
    params.ori = frozen_ori;
    params.pos2 = frozen_aim;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Freeze_pos::afxXM_Freeze_pos(afxXM_FreezeData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw)
{ 
  first = true;
  delay = db->delay;
}

void afxXM_Freeze_pos::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (elapsed < delay) return;

  if (first)
  {
    frozen_pos = params.pos;
    first = false;
  }
  else
    params.pos = frozen_pos;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Freeze_pos2::afxXM_Freeze_pos2(afxXM_FreezeData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw)
{ 
  first = true;
  delay = db->delay;
}

void afxXM_Freeze_pos2::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (elapsed < delay) return;

  if (first)
  {
    frozen_pos2 = params.pos2;
    first = false;
  }
  else
    params.pos2 = frozen_pos2;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Freeze_ori::afxXM_Freeze_ori(afxXM_FreezeData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw)
{ 
  first = true;
  delay = db->delay;
}

void afxXM_Freeze_ori::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (elapsed < delay) return;

  if (first)
  {
    frozen_ori = params.ori;
    first = false;
  }
  else
    params.ori = frozen_ori;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//