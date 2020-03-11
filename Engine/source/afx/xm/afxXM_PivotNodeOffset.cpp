
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

#include "ts/tsShapeInstance.h"

#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/xm/afxXfmMod.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxXM_PivotNodeOffsetData : public afxXM_BaseData
{
  typedef afxXM_BaseData Parent;

public:
  StringTableEntry  node_name;
  bool              node_is_static;

public:
  /*C*/         afxXM_PivotNodeOffsetData();
  /*C*/         afxXM_PivotNodeOffsetData(const afxXM_PivotNodeOffsetData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_PivotNodeOffsetData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_PivotNodeOffset : public afxXM_Base
{
  typedef afxXM_Base Parent;

  StringTableEntry  node_name;
  bool              node_is_static;
  S32               node_ID;
  Point3F           pivot_offset;
  bool              offset_calculated;

public:
  /*C*/         afxXM_PivotNodeOffset(afxXM_PivotNodeOffsetData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_PivotNodeOffsetData);

ConsoleDocClass( afxXM_PivotNodeOffsetData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_PivotNodeOffsetData::afxXM_PivotNodeOffsetData()
{ 
  node_name = ST_NULLSTRING;
  node_is_static = true;
}

afxXM_PivotNodeOffsetData::afxXM_PivotNodeOffsetData(const afxXM_PivotNodeOffsetData& other, bool temp_clone) : afxXM_BaseData(other, temp_clone)
{
  node_name = other.node_name;
  node_is_static = other.node_is_static;
}

void afxXM_PivotNodeOffsetData::initPersistFields()
{
  addField("nodeName",      TypeString,   Offset(node_name, afxXM_PivotNodeOffsetData),
    "...");
  addField("nodeIsStatic",  TypeBool, Offset(node_is_static, afxXM_PivotNodeOffsetData),
    "...");

  Parent::initPersistFields();
}

void afxXM_PivotNodeOffsetData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->writeString(node_name);
  stream->writeFlag(node_is_static);
}

void afxXM_PivotNodeOffsetData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  node_name = stream->readSTString();
  node_is_static = stream->readFlag();
}

afxXM_Base* afxXM_PivotNodeOffsetData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_PivotNodeOffsetData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_PivotNodeOffsetData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  return new afxXM_PivotNodeOffset(datablock, fx);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_PivotNodeOffset::afxXM_PivotNodeOffset(afxXM_PivotNodeOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw)
{ 
  node_name = db->node_name; 
  node_is_static = db->node_is_static;
  node_ID = -1;
  pivot_offset.set(0,0,0);
  offset_calculated = false;
}

void afxXM_PivotNodeOffset::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (node_ID < 0)
  {
    TSShape* ts_shape = fx_wrapper->getTSShape();
    node_ID = (ts_shape) ? ts_shape->findNode(node_name) : -1;
  }

  if (node_ID >= 0)
  {
    if (!node_is_static || !offset_calculated)
    {
      TSShapeInstance* ts_shape_inst = fx_wrapper->getTSShapeInstance();
      if (ts_shape_inst)
      {
        const MatrixF& pivot_xfm = ts_shape_inst->mNodeTransforms[node_ID];
        pivot_offset = -pivot_xfm.getPosition();
        offset_calculated = true;
      }
    }
  }

  // re-orient pivot offset then add to position
  Point3F pivot_offset_temp;
  params.ori.mulV(pivot_offset, &pivot_offset_temp);
  params.pos += pivot_offset_temp;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//