
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

class afxXM_GroundConformData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;

public:
  F32           height;
  bool          do_terrain;
  bool          do_interiors;
  bool          do_orientation;

public:
  /*C*/         afxXM_GroundConformData();
  /*C*/         afxXM_GroundConformData(const afxXM_GroundConformData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_GroundConformData);
  DECLARE_CATEGORY("AFX");
};

class afxXM_GroundConform : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  afxXM_GroundConformData*  db;
  SceneContainer*           container;

public:
  /*C*/         afxXM_GroundConform(afxXM_GroundConformData*, afxEffectWrapper*, bool on_server);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_GroundConformData);

ConsoleDocClass( afxXM_GroundConformData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_GroundConformData::afxXM_GroundConformData()
{
  height = 0.0f;
  do_terrain = true;
  do_interiors = true;
  do_orientation = false;
}

afxXM_GroundConformData::afxXM_GroundConformData(const afxXM_GroundConformData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
  height = other.height;
  do_terrain = other.do_terrain;
  do_interiors = other.do_interiors;
  do_orientation = other.do_orientation;
}

void afxXM_GroundConformData::initPersistFields()
{
  addField("height",              TypeF32,      Offset(height, afxXM_GroundConformData),
    "...");
  addField("conformToTerrain",    TypeBool,     Offset(do_terrain, afxXM_GroundConformData),
    "...");
  addField("conformToInteriors",  TypeBool,     Offset(do_interiors, afxXM_GroundConformData),
    "...");
  addField("conformOrientation",  TypeBool,     Offset(do_orientation, afxXM_GroundConformData),
    "...");

  Parent::initPersistFields();
}

void afxXM_GroundConformData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(height);
  stream->write(do_terrain);
  stream->write(do_interiors);
  stream->write(do_orientation);
}

void afxXM_GroundConformData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&height);
  stream->read(&do_terrain);
  stream->read(&do_interiors);
  stream->read(&do_orientation);
}

afxXM_Base* afxXM_GroundConformData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_GroundConformData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_GroundConformData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  return new afxXM_GroundConform(datablock, fx, on_server);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_GroundConform::afxXM_GroundConform(afxXM_GroundConformData* db, afxEffectWrapper* fxw, bool on_server) 
: afxXM_WeightedBase(db, fxw) 
{ 
  this->db = db;
  this->container = (on_server) ? &gServerContainer : &gClientContainer;
}

void afxXM_GroundConform::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  RayInfo rInfo;
  bool hit = false;
  
  if (db->do_interiors)
  {
    U32 mask = InteriorLikeObjectType;
    if (db->do_terrain)
    {
      mask |= TerrainObjectType | TerrainLikeObjectType;
    }
    
    Point3F above_pos(params.pos); above_pos.z += 0.1f;
    Point3F below_pos(params.pos); below_pos.z -= 10000;
    hit = container->castRay(above_pos, below_pos, mask, &rInfo);
    if (!hit)
    {
      above_pos.z = params.pos.z + 10000;
      below_pos.z = params.pos.z - 0.1f;
      hit = container->castRay(below_pos, above_pos, mask, &rInfo);
    }
  }
  else if (db->do_terrain)
  {
    U32 mask = TerrainObjectType | TerrainLikeObjectType;
    Point3F above_pos(params.pos); above_pos.z += 10000;
    Point3F below_pos(params.pos); below_pos.z -= 10000;
    hit = container->castRay(above_pos, below_pos, mask, &rInfo);
  }
  
  if (hit)
  {
    F32 terrain_z = rInfo.point.z;
    F32 wt_factor = calc_weight_factor(elapsed);
    F32 old_z = params.pos.z;
    F32 new_z = terrain_z + db->height;
    params.pos.z = ((1-wt_factor)*old_z) + ((wt_factor)*new_z);
    
    if (db->do_orientation)
    {
      Point3F x,y,z;
      z = rInfo.normal;
      z.normalize();
      params.ori.getColumn(1,&y);
      mCross(y,z,&x);
      x.normalize();
      mCross(z,x,&y);
      params.ori.setColumn(0,x);
      params.ori.setColumn(1,y);
      params.ori.setColumn(2,z);
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

