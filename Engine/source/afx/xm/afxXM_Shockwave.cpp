
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
#include "afx/afxChoreographer.h"
#include "afx/xm/afxXfmMod.h"
#include "afx/util/afxPath3D.h"
#include "afx/util/afxPath.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxXM_ShockwaveData : public afxXM_BaseData
{
  typedef afxXM_BaseData Parent;

public:
  F32           rate;
  bool          aim_z_only;

public:
  /*C*/         afxXM_ShockwaveData();
  /*C*/         afxXM_ShockwaveData(const afxXM_ShockwaveData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_ShockwaveData);
  DECLARE_CATEGORY("AFX");
};

class afxConstraint;

class afxXM_Shockwave : public afxXM_Base
{
  typedef afxXM_Base Parent;

  afxXM_ShockwaveData*  db;
  bool                  first;
  Point3F               fixed_pos;

public:
  /*C*/         afxXM_Shockwave(afxXM_ShockwaveData*, afxEffectWrapper*);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_ShockwaveData);

ConsoleDocClass( afxXM_ShockwaveData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_ShockwaveData::afxXM_ShockwaveData()
{
  rate = 1.0f;
  aim_z_only = false;
}

afxXM_ShockwaveData::afxXM_ShockwaveData(const afxXM_ShockwaveData& other, bool temp_clone) : afxXM_BaseData(other, temp_clone)
{
  rate = other.rate;
  aim_z_only = other.aim_z_only;
}

void afxXM_ShockwaveData::initPersistFields()
{
  addField("rate",      TypeF32,      Offset(rate, afxXM_ShockwaveData),
    "...");
  addField("aimZOnly",  TypeBool,     Offset(aim_z_only, afxXM_ShockwaveData),
    "...");

  Parent::initPersistFields();
}

void afxXM_ShockwaveData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(rate);
  stream->write(aim_z_only);
}

void afxXM_ShockwaveData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&rate);
  stream->read(&aim_z_only);
}

afxXM_Base* afxXM_ShockwaveData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_ShockwaveData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_ShockwaveData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  return new afxXM_Shockwave(datablock, fx);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_Shockwave::afxXM_Shockwave(afxXM_ShockwaveData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw)
{ 
  this->db = db; 
  first = true;
  fixed_pos.zero();
}

void afxXM_Shockwave::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (first)
  {
    fixed_pos = params.pos;
    first = false;
  }

  Point3F aim_at_pos = params.pos2;
  if (db->aim_z_only)
    aim_at_pos.z = fixed_pos.z;
  
  VectorF line_of_sight = aim_at_pos - fixed_pos;
  line_of_sight.normalize();

  params.pos = fixed_pos + line_of_sight*(elapsed*db->rate);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//