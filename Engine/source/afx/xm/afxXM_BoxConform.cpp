
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
enum afxXM_BoxConformType
{
  X_POS, X_NEG, Y_POS, Y_NEG, Z_POS, Z_NEG
};
DefineEnumType( afxXM_BoxConformType );

class afxXM_BoxConformData : public afxXM_BaseData
{
  typedef afxXM_BaseData Parent;

public:
  S32           aabb_alignment;

public:
  /*C*/         afxXM_BoxConformData();
  /*C*/         afxXM_BoxConformData(const afxXM_BoxConformData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);
  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_BoxConformData);
  DECLARE_CATEGORY("AFX");
};

class afxXM_BoxConform : public afxXM_Base
{
  typedef afxXM_Base Parent;

  afxXM_BoxConformData*  db;

public:
  /*C*/         afxXM_BoxConform(afxXM_BoxConformData*, afxEffectWrapper*, bool on_server);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_BoxConformData);

ConsoleDocClass( afxXM_BoxConformData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_BoxConformData::afxXM_BoxConformData()
{
  aabb_alignment = Z_NEG;
}

afxXM_BoxConformData::afxXM_BoxConformData(const afxXM_BoxConformData& other, bool temp_clone) 
  : afxXM_BaseData(other, temp_clone)
{
  aabb_alignment = other.aabb_alignment;
}

ImplementEnumType( afxXM_BoxConformType, "Possible box conform alignment types.\n" "@ingroup afxXM_BoxConform\n\n" )
   { X_POS, "+x", "..." },
   { X_NEG, "-x", "..." },
   { Y_POS, "+y", "..." },
   { Y_NEG, "-y", "..." },
   { Z_POS, "+z", "..." },
   { Z_NEG, "-z", "..." },
   { X_POS, "x",  "..." },
   { Y_POS, "y",  "..." },
   { Z_POS, "z",  "..." },
EndImplementEnumType;

void afxXM_BoxConformData::initPersistFields()
{
  addField("boxAlignment",    TYPEID< afxXM_BoxConformType >(),    Offset(aabb_alignment, afxXM_BoxConformData),
    "...");

  Parent::initPersistFields();
}

void afxXM_BoxConformData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(aabb_alignment);
}

void afxXM_BoxConformData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&aabb_alignment);
}

afxXM_Base* afxXM_BoxConformData::create(afxEffectWrapper* fx, bool on_server)
{
  return new afxXM_BoxConform(this, fx, on_server);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_BoxConform::afxXM_BoxConform(afxXM_BoxConformData* db, afxEffectWrapper* fxw, bool on_server) 
: afxXM_Base(db, fxw) 
{ 
  this->db = db;
}

void afxXM_BoxConform::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  afxConstraint* pos_cons = fx_wrapper->getPosConstraint();
  if (!pos_cons)
    return;

  SceneObject* obj = pos_cons->getSceneObject();
  if (!obj)
    return;

  const Box3F& box = obj->getWorldBox();

  switch (db->aabb_alignment)
  {
  case X_POS:
    params.pos.x = box.maxExtents.x;
    break;
  case X_NEG:
    params.pos.x = box.minExtents.x;
    break;
  case Y_POS:
    params.pos.y = box.maxExtents.y;
    break;
  case Y_NEG:
    params.pos.y = box.minExtents.y;
    break;
  case Z_POS:
    params.pos.z = box.maxExtents.z;
    break;
  case Z_NEG:
    params.pos.z = box.minExtents.z;
    break;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

