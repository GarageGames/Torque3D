
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
#include "afx/xm/afxXM_WaveBase.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVE COLOR INTERPOLATORS

class afxXM_WaveInterp_Color : public afxXM_WaveInterp
{
protected:
  ColorF   a_set, b_set;
  ColorF   a_var, b_var;
  ColorF   a, b;
  bool     sync_var;

public:
  afxXM_WaveInterp_Color(); 

  void set(ColorF& a, ColorF& b, ColorF& a_var, ColorF& b_var, bool sync_var);

  virtual void interpolate(F32 t, afxXM_Params& params)=0;
  virtual void pulse();
};

afxXM_WaveInterp_Color::afxXM_WaveInterp_Color() 
{ 
  a_set.set(0.0f, 0.0f, 0.0f, 0.0f); 
  b_set.set(1.0f, 1.0f, 1.0f, 1.0f); 
  a_var.set(0.0f, 0.0f, 0.0f, 0.0f); 
  b_var.set(0.0f, 0.0f, 0.0f, 0.0f); 
  sync_var = false;
  a.set(0.0f, 0.0f, 0.0f, 0.0f);
  b.set(1.0f, 1.0f, 1.0f, 1.0f);
}

void afxXM_WaveInterp_Color::set(ColorF& a, ColorF& b, ColorF& a_var, ColorF& b_var, bool sync_var)
{
  a_set = a; 
  b_set = b;
  this->a_var = a_var;
  this->b_var = b_var;
  this->sync_var = sync_var;
  this->a = a; 
  this->b = b;
}

inline void afxXM_WaveInterp_Color::pulse()
{
  ColorF temp_color;
  F32 rand_t = gRandGen.randF()*2.0f;
  temp_color.interpolate(-a_var, a_var, rand_t);
  a = a_set + temp_color;
  if (!sync_var) 
    rand_t = gRandGen.randF()*2.0f;
  temp_color.interpolate(-b_var, b_var, rand_t);
  b = b_set + temp_color;
}

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Color_Add : public afxXM_WaveInterp_Color
{
public:
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    ColorF temp_color;
    temp_color.interpolate(a, b, t);
    params.color += temp_color;
  }
};

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Color_Mul : public afxXM_WaveInterp_Color
{
public:
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    ColorF temp_color;
    temp_color.interpolate(a, b, t);
    params.color *= temp_color;
  }
};

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Color_Rep : public afxXM_WaveInterp_Color
{
public:
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    params.color.interpolate(a, b, t);
  }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVE SCALAR BASE DATABLOCK

class afxXM_WaveColorData_Common : public virtual afxXM_Defs
{
protected:
  static afxXM_WaveInterp_Color* createInterp(U32 op, afxXM_BaseData*);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

afxXM_WaveInterp_Color* 
afxXM_WaveColorData_Common::createInterp(U32 op, afxXM_BaseData* db) 
{
  afxXM_WaveInterp_Color* interpolator = 0;

  switch (op)
  {
  case afxXM_WaveBaseData::OP_ADD:
    interpolator = new afxXM_WaveInterp_Color_Add();
    return 0;
  case afxXM_WaveBaseData::OP_MULTIPLY:
    interpolator = new afxXM_WaveInterp_Color_Mul();
    break;
  case afxXM_WaveBaseData::OP_REPLACE:
    interpolator = new afxXM_WaveInterp_Color_Rep();
    break;
  }

  if (!interpolator)
    Con::errorf("%s::%s -- failed to allocate wave color interpolator.", db->getClassName(), db->getName());

  return interpolator; 
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVE COLOR DATABLOCK

class afxXM_WaveColorData : public afxXM_WaveBaseData, afxXM_WaveColorData_Common
{
  typedef afxXM_WaveBaseData Parent;

public:
  ColorF        a, b;
  ColorF        a_var, b_var;
  bool          sync_var;

public:
  /*C*/         afxXM_WaveColorData();
  /*C*/         afxXM_WaveColorData(const afxXM_WaveColorData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_WaveColorData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_WaveColorData);

ConsoleDocClass( afxXM_WaveColorData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_WaveColorData::afxXM_WaveColorData()
{
  a.set(0.0f, 0.0f, 0.0f, 0.0f);
  b.set(1.0f, 1.0f, 1.0f, 1.0f);
  a_var.set(0.0f, 0.0f, 0.0f, 0.0f);
  b_var.set(0.0f, 0.0f, 0.0f, 0.0f);
  sync_var = false;
}

afxXM_WaveColorData::afxXM_WaveColorData(const afxXM_WaveColorData& other, bool temp_clone) : afxXM_WaveBaseData(other, temp_clone)
{
  a = other.a;
  b = other.b;
  a_var = other.a_var;
  b_var = other.b_var;
  sync_var = other.sync_var;
}

void afxXM_WaveColorData::initPersistFields()
{
  addField("a",               TypeColorF,      Offset(a, afxXM_WaveColorData),
    "...");
  addField("b",               TypeColorF,      Offset(b, afxXM_WaveColorData),
    "...");
  addField("aVariance",       TypeColorF,      Offset(a_var, afxXM_WaveColorData),
    "...");
  addField("bVariance",       TypeColorF,      Offset(b_var, afxXM_WaveColorData),
    "...");
  addField("syncVariances",   TypeBool,        Offset(sync_var, afxXM_WaveColorData),
    "...");

  Parent::initPersistFields();
}

void afxXM_WaveColorData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(a);
  stream->write(b);
  stream->write(a_var);
  stream->write(b_var);
  stream->writeFlag(sync_var);
}

void afxXM_WaveColorData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&a);
  stream->read(&b);
  stream->read(&a_var);
  stream->read(&b_var);
  sync_var = stream->readFlag();
}

afxXM_Base* afxXM_WaveColorData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_WaveColorData* dblock = this;

  if (getSubstitutionCount() > 0)
  {
    dblock = new afxXM_WaveColorData(*this, true);
    this->performSubstitutions(dblock, fx->getChoreographer(), fx->getGroupIndex());
  }

  afxXM_WaveInterp_Color* interp;
  interp = createInterp(dblock->op, dblock);
  if (!interp)
    return 0;

  interp->set(dblock->a, dblock->b, dblock->a_var, dblock->b_var, dblock->sync_var);

  return new afxXM_WaveBase(dblock, fx, interp);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVE RIDER COLOR DATABLOCK

class afxXM_WaveRiderColorData : public afxXM_WaveRiderBaseData, afxXM_WaveColorData_Common
{
  typedef afxXM_WaveRiderBaseData Parent;

public:
  ColorF        a, b;
  ColorF        a_var, b_var;
  bool          sync_var;

public:
  /*C*/         afxXM_WaveRiderColorData();
  /*C*/         afxXM_WaveRiderColorData(const afxXM_WaveRiderColorData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_WaveRiderColorData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_WaveRiderColorData);

ConsoleDocClass( afxXM_WaveRiderColorData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_WaveRiderColorData::afxXM_WaveRiderColorData()
{
  a.set(0.0f, 0.0f, 0.0f, 0.0f);
  b.set(1.0f, 1.0f, 1.0f, 1.0f);
  a_var.set(0.0f, 0.0f, 0.0f, 0.0f);
  b_var.set(0.0f, 0.0f, 0.0f, 0.0f);
  sync_var = false;
}

afxXM_WaveRiderColorData::afxXM_WaveRiderColorData(const afxXM_WaveRiderColorData& other, bool temp_clone) : afxXM_WaveRiderBaseData(other, temp_clone)
{
  a = other.a;
  b = other.b;
  a_var = other.a_var;
  b_var = other.b_var;
  sync_var = other.sync_var;
}

void afxXM_WaveRiderColorData::initPersistFields()
{
  addField("a",               TypeColorF,      Offset(a, afxXM_WaveRiderColorData),
    "...");
  addField("b",               TypeColorF,      Offset(b, afxXM_WaveRiderColorData),
    "...");
  addField("aVariance",       TypeColorF,      Offset(a_var, afxXM_WaveRiderColorData),
    "...");
  addField("bVariance",       TypeColorF,      Offset(b_var, afxXM_WaveRiderColorData),
    "...");
  addField("syncVariances",   TypeBool,        Offset(sync_var, afxXM_WaveRiderColorData),
    "...");

  Parent::initPersistFields();
}

void afxXM_WaveRiderColorData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(a);
  stream->write(b);
  stream->write(a_var);
  stream->write(b_var);
  stream->writeFlag(sync_var);
}

void afxXM_WaveRiderColorData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&a);
  stream->read(&b);
  stream->read(&a_var);
  stream->read(&b_var);
  sync_var = stream->readFlag();
}

afxXM_Base* afxXM_WaveRiderColorData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_WaveRiderColorData* dblock = this;

  if (getSubstitutionCount() > 0)
  {
    dblock = new afxXM_WaveRiderColorData(*this, true);
    this->performSubstitutions(dblock, fx->getChoreographer(), fx->getGroupIndex());
  }

  afxXM_WaveInterp_Color* interp;
  interp = createInterp(dblock->op, dblock);
  if (!interp)
    return 0;

  interp->set(dblock->a, dblock->b, dblock->a_var, dblock->b_var, dblock->sync_var);

  return new afxXM_WaveRiderBase(dblock, fx, interp);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
