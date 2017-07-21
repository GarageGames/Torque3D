
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
// WAVE SCALAR INTERPOLATORS

class afxXM_WaveInterp_Scalar : public afxXM_WaveInterp
{
protected:
  F32   a_set, b_set;
  F32   a_var, b_var;
  F32   a, b;
  bool  sync_var;

public:
  afxXM_WaveInterp_Scalar(); 

  void set(F32 a, F32 b, F32 a_var, F32 b_var, bool sync_var);

  virtual void interpolate(F32 t, afxXM_Params& params)=0;
  virtual void pulse();
};

afxXM_WaveInterp_Scalar::afxXM_WaveInterp_Scalar() 
{ 
  a_set = 0.0f; 
  b_set = 1.0f;
  a_var = 0.0f;
  b_var = 0.0;
  sync_var = false;
  a = 0.0f; 
  b = 1.0f;
}

void afxXM_WaveInterp_Scalar::set(F32 a, F32 b, F32 a_var, F32 b_var, bool sync_var)
{
  a_set = a; 
  b_set = b;
  this->a_var = a_var;
  this->b_var = b_var;
  this->sync_var = sync_var;
  this->a = a; 
  this->b = b;
}

inline void afxXM_WaveInterp_Scalar::pulse()
{
  F32 rand_t = gRandGen.randF()*2.0f;
  a = a_set + rand_t*a_var - a_var;
  if (!sync_var) 
    rand_t = gRandGen.randF()*2.0f;
  b = b_set + rand_t*b_var - b_var;
}

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_Add : public afxXM_WaveInterp_Scalar
{
protected:
  U32 offset;
public:
  afxXM_WaveInterp_Scalar_Add(U32 o) : afxXM_WaveInterp_Scalar() { offset = o; }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    *((F32*)(((char*)(&params)) + offset)) += lerp(t, a, b);
  }
};

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_Mul : public afxXM_WaveInterp_Scalar
{
protected:
  U32 offset;
public:
  afxXM_WaveInterp_Scalar_Mul(U32 o) : afxXM_WaveInterp_Scalar() { offset = o; }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    *((F32*)(((char*)(&params)) + offset)) *= lerp(t, a, b);
  }
};

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_Rep : public afxXM_WaveInterp_Scalar
{
protected:
  U32 offset;
public:
  afxXM_WaveInterp_Scalar_Rep(U32 o) : afxXM_WaveInterp_Scalar() { offset = o; }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    *((F32*)(((char*)(&params)) + offset)) = lerp(t, a, b);
  }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_PointAdd : public afxXM_WaveInterp_Scalar
{
protected:
  U32 offset;
public:
  afxXM_WaveInterp_Scalar_PointAdd(U32 o) : afxXM_WaveInterp_Scalar() { offset = o; }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    F32 scalar_at_t = lerp(t, a, b);
    Point3F point_at_t(scalar_at_t, scalar_at_t, scalar_at_t);
    *((Point3F*)(((char*)(&params)) + offset)) += point_at_t;
  }
};

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_PointMul : public afxXM_WaveInterp_Scalar
{
protected:
  U32 offset;
public:
  afxXM_WaveInterp_Scalar_PointMul(U32 o) : afxXM_WaveInterp_Scalar() { offset = o; }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    *((Point3F*)(((char*)(&params)) + offset)) *= lerp(t, a, b);
  }
};

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_PointRep : public afxXM_WaveInterp_Scalar
{
protected:
  U32 offset;
public:
  afxXM_WaveInterp_Scalar_PointRep(U32 o) : afxXM_WaveInterp_Scalar() { offset = o; }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    F32 scalar_at_t = lerp(t, a, b);
    Point3F point_at_t(scalar_at_t, scalar_at_t, scalar_at_t);
    *((Point3F*)(((char*)(&params)) + offset)) = point_at_t;
  }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_Axis_PointAdd : public afxXM_WaveInterp_Scalar
{
protected:
  Point3F   axis;
  U32       offset;
public:
  afxXM_WaveInterp_Scalar_Axis_PointAdd(U32 o, Point3F ax) : afxXM_WaveInterp_Scalar() { offset = o; axis = ax; }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    Point3F point_at_t = axis*lerp(t, a, b);
    *((Point3F*)(((char*)(&params)) + offset)) += point_at_t;
  }
};

class afxXM_WaveInterp_Scalar_LocalAxis_PointAdd : public afxXM_WaveInterp_Scalar
{
protected:
  Point3F   axis;
  U32       offset;
public:
  afxXM_WaveInterp_Scalar_LocalAxis_PointAdd(U32 o, Point3F ax) : afxXM_WaveInterp_Scalar() { offset = o; axis = ax; }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    Point3F local_axis(axis); 
    params.ori.mulV(local_axis);
    Point3F point_at_t = local_axis*lerp(t, a, b);
    *((Point3F*)(((char*)(&params)) + offset)) += point_at_t;
  }
};

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_Axis_PointMul : public afxXM_WaveInterp_Scalar
{
protected:
  Point3F   axis;
  U32       offset;
public:
  afxXM_WaveInterp_Scalar_Axis_PointMul(U32 o, Point3F ax) : afxXM_WaveInterp_Scalar() { offset = o; axis = ax; }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    Point3F point_at_t = axis*lerp(t, a, b);
    *((Point3F*)(((char*)(&params)) + offset)) *= point_at_t;
  }
};

class afxXM_WaveInterp_Scalar_LocalAxis_PointMul : public afxXM_WaveInterp_Scalar
{
protected:
  Point3F   axis;
  U32       offset;
public:
  afxXM_WaveInterp_Scalar_LocalAxis_PointMul(U32 o, Point3F ax) : afxXM_WaveInterp_Scalar() { offset = o; axis = ax; }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    Point3F local_axis(axis); 
    params.ori.mulV(local_axis);
    Point3F point_at_t = local_axis*lerp(t, a, b);
    *((Point3F*)(((char*)(&params)) + offset)) *= point_at_t;
  }
};

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_Axis_PointRep : public afxXM_WaveInterp_Scalar
{
protected:
  Point3F   axis;
  U32       offset;
public:
  afxXM_WaveInterp_Scalar_Axis_PointRep(U32 o, Point3F ax) : afxXM_WaveInterp_Scalar() { offset = o; axis = ax; }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    Point3F point_at_t = axis*lerp(t, a, b);
    *((Point3F*)(((char*)(&params)) + offset)) = point_at_t;
  }
};

class afxXM_WaveInterp_Scalar_LocalAxis_PointRep : public afxXM_WaveInterp_Scalar
{
protected:
  Point3F   axis;
  U32       offset;
public:
  afxXM_WaveInterp_Scalar_LocalAxis_PointRep(U32 o, Point3F ax) : afxXM_WaveInterp_Scalar() { offset = o; axis = ax; }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    Point3F local_axis(axis); 
    params.ori.mulV(local_axis);
    Point3F point_at_t = local_axis*lerp(t, a, b);
    *((Point3F*)(((char*)(&params)) + offset)) = point_at_t;
  }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_ColorAdd : public afxXM_WaveInterp_Scalar
{
public:
  afxXM_WaveInterp_Scalar_ColorAdd() : afxXM_WaveInterp_Scalar() { }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    F32 scalar_at_t = lerp(t, a, b);
    ColorF color_at_t(scalar_at_t, scalar_at_t, scalar_at_t, scalar_at_t);
    params.color += color_at_t;
  }
};

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_ColorMul : public afxXM_WaveInterp_Scalar
{
public:
  afxXM_WaveInterp_Scalar_ColorMul() : afxXM_WaveInterp_Scalar() { }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    params.color *= lerp(t, a, b);
  }
};

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_ColorRep : public afxXM_WaveInterp_Scalar
{
public:
  afxXM_WaveInterp_Scalar_ColorRep() : afxXM_WaveInterp_Scalar() { }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    F32 scalar_at_t = lerp(t, a, b);
    params.color.set(scalar_at_t, scalar_at_t, scalar_at_t, scalar_at_t);
  }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_OriMul : public afxXM_WaveInterp_Scalar
{
protected:
  Point3F axis;
public:
  afxXM_WaveInterp_Scalar_OriMul(Point3F& ax) : afxXM_WaveInterp_Scalar() { axis = ax;  }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    F32 theta = mDegToRad(lerp(t, a, b));
    AngAxisF rot_aa(axis, theta);
    MatrixF rot_xfm; rot_aa.setMatrix(&rot_xfm);
    params.ori.mul(rot_xfm);  
  }
};

//~~~~~~~~~~~~~~~~~~~~//

class afxXM_WaveInterp_Scalar_OriRep : public afxXM_WaveInterp_Scalar
{
protected:
  Point3F axis;
public:
  afxXM_WaveInterp_Scalar_OriRep(Point3F& ax) : afxXM_WaveInterp_Scalar() { axis = ax;  }
  virtual void interpolate(F32 t, afxXM_Params& params)
  {
    F32 theta = mDegToRad(lerp(t, a, b));
    AngAxisF rot_aa(axis, theta);
    rot_aa.setMatrix(&params.ori);
  }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVE SCALAR BASE DATABLOCK

class afxXM_WaveScalarData_Common : public virtual afxXM_Defs
{
  static afxXM_WaveInterp_Scalar* alloc_interp(U32 param, S32 comp, U32 op, U32 off, 
                                               Point3F& axis, bool loc_axis, afxXM_BaseData*);
  static bool needs_offset(U32 param, S32 component);
  static bool needs_axis(U32 param, S32 component);

protected:
  static afxXM_WaveInterp_Scalar* createInterp(U32 param, U32 op, Point3F axis, bool loc_axis,
                                               afxXM_BaseData*);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

bool afxXM_WaveScalarData_Common::needs_offset(U32 param, S32 component)
{
  switch (param)
  {
  case ORIENTATION:
    return false;

  case POSITION:
  case POSITION2:
  case SCALE:
  case VISIBILITY:
    return true;

  case COLOR:
    return (component != -1);
  }

  return false;
}

bool afxXM_WaveScalarData_Common::needs_axis(U32 param, S32 component)
{
  switch (param)
  {
  case ORIENTATION:
    return true;

  case POSITION:
  case POSITION2:
  case SCALE:
  case COLOR:
  case VISIBILITY:
    return false;
  }

  return true;
}

afxXM_WaveInterp_Scalar* 
afxXM_WaveScalarData_Common::alloc_interp(U32 param, S32 component, U32 op, U32 offset, Point3F& axis, bool loc_axis, afxXM_BaseData* db) 
{ 
  afxXM_WaveInterp_Scalar* interpolator = 0;

  switch (param)
  {
  case ORIENTATION:
    switch (op)
    {
    case afxXM_WaveBaseData::OP_ADD:
      Con::errorf("%s::%s -- invalid orientation op.", db->getClassName(), db->getName());
      return 0;
    case afxXM_WaveBaseData::OP_MULTIPLY:
      interpolator = new afxXM_WaveInterp_Scalar_OriMul(axis);
      break;
    case afxXM_WaveBaseData::OP_REPLACE:
      interpolator = new afxXM_WaveInterp_Scalar_OriRep(axis);
      break;
    }
    break;
  case POSITION:
  case POSITION2:
  case SCALE:
    if (component == -1)
    {
      if (axis.isZero())
      {
        switch (op)
        {
        case afxXM_WaveBaseData::OP_ADD:
          interpolator = new afxXM_WaveInterp_Scalar_PointAdd(offset);
          break;
        case afxXM_WaveBaseData::OP_MULTIPLY:
          interpolator = new afxXM_WaveInterp_Scalar_PointMul(offset);
          break;
        case afxXM_WaveBaseData::OP_REPLACE:
          interpolator = new afxXM_WaveInterp_Scalar_PointRep(offset);
          break;
        }
      }
      else if (loc_axis)
      {
        switch (op)
        {
        case afxXM_WaveBaseData::OP_ADD:
          interpolator = new afxXM_WaveInterp_Scalar_LocalAxis_PointAdd(offset, axis);
          break;
        case afxXM_WaveBaseData::OP_MULTIPLY:
          interpolator = new afxXM_WaveInterp_Scalar_LocalAxis_PointMul(offset, axis);
          break;
        case afxXM_WaveBaseData::OP_REPLACE:
          interpolator = new afxXM_WaveInterp_Scalar_LocalAxis_PointRep(offset, axis);
          break;
        }
      }
      else
      {
        switch (op)
        {
        case afxXM_WaveBaseData::OP_ADD:
          interpolator = new afxXM_WaveInterp_Scalar_Axis_PointAdd(offset, axis);
          break;
        case afxXM_WaveBaseData::OP_MULTIPLY:
          interpolator = new afxXM_WaveInterp_Scalar_Axis_PointMul(offset, axis);
          break;
        case afxXM_WaveBaseData::OP_REPLACE:
          interpolator = new afxXM_WaveInterp_Scalar_Axis_PointRep(offset, axis);
          break;
        }
      }
    }
    else
    {
      switch (op)
      {
      case afxXM_WaveBaseData::OP_ADD:
        interpolator = new afxXM_WaveInterp_Scalar_Add(offset);
        break;
      case afxXM_WaveBaseData::OP_MULTIPLY:
        interpolator = new afxXM_WaveInterp_Scalar_Mul(offset);
        break;
      case afxXM_WaveBaseData::OP_REPLACE:
        interpolator = new afxXM_WaveInterp_Scalar_Rep(offset);
        break;
      }
    }
    break;  

  case COLOR:
    if (component == -1)
    {
      switch (op)
      {
      case afxXM_WaveBaseData::OP_ADD:
        interpolator = new afxXM_WaveInterp_Scalar_ColorAdd();
        break;
      case afxXM_WaveBaseData::OP_MULTIPLY:
        interpolator = new afxXM_WaveInterp_Scalar_ColorMul();
        break;
      case afxXM_WaveBaseData::OP_REPLACE:
        interpolator = new afxXM_WaveInterp_Scalar_ColorRep();
        break;
      }
    }
    else
    {
      switch (op)
      {
      case afxXM_WaveBaseData::OP_ADD:
        interpolator = new afxXM_WaveInterp_Scalar_Add(offset);
        break;
      case afxXM_WaveBaseData::OP_MULTIPLY:
        interpolator = new afxXM_WaveInterp_Scalar_Mul(offset);
        break;
      case afxXM_WaveBaseData::OP_REPLACE:
        interpolator = new afxXM_WaveInterp_Scalar_Rep(offset);
        break;
      }
    }
    break;  

  case VISIBILITY:
    switch (op)
    {
    case afxXM_WaveBaseData::OP_ADD:
      interpolator = new afxXM_WaveInterp_Scalar_Add(offset);
      break;
    case afxXM_WaveBaseData::OP_MULTIPLY:
      interpolator = new afxXM_WaveInterp_Scalar_Mul(offset);
      break;
    case afxXM_WaveBaseData::OP_REPLACE:
      interpolator = new afxXM_WaveInterp_Scalar_Rep(offset);
      break;
    }
  }

  if (!interpolator)
    Con::errorf("%s::%s -- failed to allocate wave interpolator.", db->getClassName(), db->getName());

  return interpolator; 
}

afxXM_WaveInterp_Scalar* 
afxXM_WaveScalarData_Common::createInterp(U32 parameter, U32 op, Point3F axis, bool loc_axis, afxXM_BaseData* db) 
{
  S32 component; U32 param_bit;
  afxXM_WaveBaseData::initParamInfo(parameter, param_bit, component);

  if (param_bit == 0)
  {
    Con::errorf("%s::%s -- unknown parameter specified.", db->getClassName(), db->getName());
    return 0;
  }

  if (axis.isZero() && needs_axis(param_bit, component))
  {
    Con::errorf("%s::%s -- axis required.", db->getClassName(), db->getName());
    return 0;
  }

  if (!axis.isZero())
    axis.normalize();

  U32 offset = afxXM_Params::BAD_OFFSET;
  if (needs_offset(param_bit, component))
  {
    offset = afxXM_Params::getParameterOffset(param_bit, component);
    if (offset == afxXM_Params::BAD_OFFSET)
    {
      Con::errorf("%s::%s -- bad component offset.", db->getClassName(), db->getName());
      return 0;
    }
  }

  return alloc_interp(param_bit, component, op, offset, axis, loc_axis, db);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVE SCALAR DATABLOCK

class afxXM_WaveScalarData : public afxXM_WaveBaseData, afxXM_WaveScalarData_Common
{
  typedef afxXM_WaveBaseData Parent;

public:
  F32           a, b;
  F32           a_var, b_var;
  bool          sync_var;

public:
  /*C*/         afxXM_WaveScalarData();
  /*C*/         afxXM_WaveScalarData(const afxXM_WaveScalarData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_WaveScalarData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_WaveScalarData);

ConsoleDocClass( afxXM_WaveScalarData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_WaveScalarData::afxXM_WaveScalarData()
{
  a = 0.0f;
  b = 1.0f;
  a_var = 0.0f;
  b_var = 0.0f;
  sync_var = false;
}

afxXM_WaveScalarData::afxXM_WaveScalarData(const afxXM_WaveScalarData& other, bool temp_clone) : afxXM_WaveBaseData(other, temp_clone)
{
  a = other.a;
  b = other.b;
  a_var = other.a_var;
  b_var = other.b_var;
  sync_var = other.sync_var;
}

void afxXM_WaveScalarData::initPersistFields()
{
  addField("a",               TypeF32,      Offset(a, afxXM_WaveScalarData),
    "...");
  addField("b",               TypeF32,      Offset(b, afxXM_WaveScalarData),
    "...");
  addField("aVariance",       TypeF32,      Offset(a_var, afxXM_WaveScalarData),
    "...");
  addField("bVariance",       TypeF32,      Offset(b_var, afxXM_WaveScalarData),
    "...");
  addField("syncVariances",   TypeBool,     Offset(sync_var, afxXM_WaveScalarData),
    "...");

  Parent::initPersistFields();
}

void afxXM_WaveScalarData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(a);
  stream->write(b);
  if (stream->writeFlag(a_var != 0.0f || b_var != 0.0f))
  {
    stream->write(a_var);
    stream->write(b_var);
    stream->writeFlag(sync_var);
  }
}

void afxXM_WaveScalarData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&a);
  stream->read(&b);
  if (stream->readFlag())
  {
    stream->read(&a_var);
    stream->read(&b_var);
    sync_var = stream->readFlag();
  }
  else
  {
    a_var = b_var = 0.0f;
    sync_var = false;
  }
}

afxXM_Base* afxXM_WaveScalarData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_WaveScalarData* dblock = this;

  if (getSubstitutionCount() > 0)
  {
    dblock = new afxXM_WaveScalarData(*this, true);
    this->performSubstitutions(dblock, fx->getChoreographer(), fx->getGroupIndex());
  }

  afxXM_WaveInterp_Scalar* interp;
  interp = createInterp(dblock->parameter, dblock->op, dblock->axis, dblock->local_axis, dblock);
  if (!interp)
    return 0;

  interp->set(dblock->a, dblock->b, dblock->a_var, dblock->b_var, dblock->sync_var);

  return new afxXM_WaveBase(dblock, fx, interp);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// WAVE RIDER SCALAR DATABLOCK

class afxXM_WaveRiderScalarData : public afxXM_WaveRiderBaseData, afxXM_WaveScalarData_Common
{
  typedef afxXM_WaveRiderBaseData Parent;

public:
  F32           a, b;
  F32           a_var, b_var;
  bool          sync_var;

public:
  /*C*/         afxXM_WaveRiderScalarData();
  /*C*/         afxXM_WaveRiderScalarData(const afxXM_WaveRiderScalarData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_WaveRiderScalarData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_WaveRiderScalarData);

ConsoleDocClass( afxXM_WaveRiderScalarData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_WaveRiderScalarData::afxXM_WaveRiderScalarData()
{
  a = 0.0f;
  b = 1.0f;
  a_var = 0.0f;
  b_var = 0.0f;
  sync_var = false;
}

afxXM_WaveRiderScalarData::afxXM_WaveRiderScalarData(const afxXM_WaveRiderScalarData& other, bool temp_clone) : afxXM_WaveRiderBaseData(other, temp_clone)
{
  a = other.a;
  b = other.b;
  a_var = other.a_var;
  b_var = other.b_var;
  sync_var = other.sync_var;
}

void afxXM_WaveRiderScalarData::initPersistFields()
{
  addField("a",               TypeF32,      Offset(a, afxXM_WaveRiderScalarData),
    "...");
  addField("b",               TypeF32,      Offset(b, afxXM_WaveRiderScalarData),
    "...");
  addField("aVariance",       TypeF32,      Offset(a_var, afxXM_WaveRiderScalarData),
    "...");
  addField("bVariance",       TypeF32,      Offset(b_var, afxXM_WaveRiderScalarData),
    "...");
  addField("syncVariances",   TypeBool,     Offset(sync_var, afxXM_WaveRiderScalarData),
    "...");

  Parent::initPersistFields();
}

void afxXM_WaveRiderScalarData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(a);
  stream->write(b);
  if (stream->writeFlag(a_var != 0.0f || b_var != 0.0f))
  {
    stream->write(a_var);
    stream->write(b_var);
    stream->writeFlag(sync_var);
  }
}

void afxXM_WaveRiderScalarData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&a);
  stream->read(&b);
  if (stream->readFlag())
  {
    stream->read(&a_var);
    stream->read(&b_var);
    sync_var = stream->readFlag();
  }
  else
  {
    a_var = b_var = 0.0f;
    sync_var = false;
  }
}

afxXM_Base* afxXM_WaveRiderScalarData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_WaveRiderScalarData* dblock = this;

  if (getSubstitutionCount() > 0)
  {
    dblock = new afxXM_WaveRiderScalarData(*this, true);
    this->performSubstitutions(dblock, fx->getChoreographer(), fx->getGroupIndex());
  }

  afxXM_WaveInterp_Scalar* interp;
  interp = createInterp(dblock->parameter, dblock->op, dblock->axis, dblock->local_axis, dblock);
  if (!interp)
    return 0;

  interp->set(dblock->a, dblock->b, dblock->a_var, dblock->b_var, dblock->sync_var);

  return new afxXM_WaveRiderBase(dblock, fx, interp);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
