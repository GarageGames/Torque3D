
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
#include "afx/xm/afxXfmMod.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// BOX HEIGHT OFFSET

class afxXM_BoxHeightOffsetData : public afxXM_BaseData
{
  typedef afxXM_BaseData Parent;
  
public:
  Point3F       offset;
  
public:
  /*C*/         afxXM_BoxHeightOffsetData();
  /*C*/         afxXM_BoxHeightOffsetData(const afxXM_BoxHeightOffsetData&, bool = false);
  
  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);
  static void   initPersistFields();
  
#if defined(AFX_VERSION)
  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);
#else
  afxXM_Base*   create(afxEffectWrapper* fx);
#endif
  
  DECLARE_CONOBJECT(afxXM_BoxHeightOffsetData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_BoxHeightOffset : public afxXM_Base
{
  typedef afxXM_Base Parent;
  Point3F       offset;
  
public:
  /*C*/         afxXM_BoxHeightOffset(afxXM_BoxHeightOffsetData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// BOX HEIGHT OFFSET

IMPLEMENT_CO_DATABLOCK_V1(afxXM_BoxHeightOffsetData);

ConsoleDocClass( afxXM_BoxHeightOffsetData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_BoxHeightOffsetData::afxXM_BoxHeightOffsetData()
{
  offset.zero();
}

afxXM_BoxHeightOffsetData::afxXM_BoxHeightOffsetData(const afxXM_BoxHeightOffsetData& other, bool temp_clone) 
  : afxXM_BaseData(other, temp_clone)
{
  offset = other.offset;
}

void afxXM_BoxHeightOffsetData::initPersistFields()
{
  addField("offset",   TypePoint3F,  Offset(offset, afxXM_BoxHeightOffsetData));

  Parent::initPersistFields();
}

void afxXM_BoxHeightOffsetData::packData(BitStream* stream)
{
  Parent::packData(stream);
  mathWrite(*stream, offset);
}

void afxXM_BoxHeightOffsetData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  mathRead(*stream, &offset);
}

#if defined(AFX_VERSION)
afxXM_Base* afxXM_BoxHeightOffsetData::create(afxEffectWrapper* fx, bool on_server)
#else
afxXM_Base* afxXM_BoxHeightOffsetData::create(afxEffectWrapper* fx)
#endif
{
  return new afxXM_BoxHeightOffset(this, fx);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_BoxHeightOffset::afxXM_BoxHeightOffset(afxXM_BoxHeightOffsetData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw) 
{ 
  offset = db->offset;
}

void afxXM_BoxHeightOffset::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  afxConstraint* pos_cons = fx_wrapper->getPosConstraint();
  SceneObject* scn_obj = (pos_cons) ? pos_cons->getSceneObject() : 0;

  if (scn_obj)
    params.pos.z += scn_obj->getWorldBox().maxExtents.z - scn_obj->getWorldBox().minExtents.z;

  params.pos += offset;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//