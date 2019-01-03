
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

class afxXM_AltitudeConformData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;

public:
  F32           height;
  bool          do_terrain;
  bool          do_interiors;
  U32           interior_types;
  U32           terrain_types;
  bool          do_freeze;

public:
  /*C*/         afxXM_AltitudeConformData();
  /*C*/         afxXM_AltitudeConformData(const afxXM_AltitudeConformData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);
  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_AltitudeConformData);
  DECLARE_CATEGORY("AFX");
};

class afxXM_AltitudeConform : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  afxXM_AltitudeConformData*  mConformData;
  SceneContainer*             mContainer;
  bool                        mDo_freeze;
  bool                        mIs_frozen;
  F32                         mTerrain_alt;
  F32                         mInterior_alt;
  Point3F                     mConformed_pos;

public:
  /*C*/         afxXM_AltitudeConform(afxXM_AltitudeConformData*, afxEffectWrapper*, bool on_server);

  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_AltitudeConformData);

ConsoleDocClass( afxXM_AltitudeConformData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_AltitudeConformData::afxXM_AltitudeConformData()
{
  height = 0.0f;
  do_terrain = false;
  do_interiors = true;
  do_freeze = false;
  interior_types = InteriorLikeObjectType;
  terrain_types = TerrainObjectType | TerrainLikeObjectType;
}

afxXM_AltitudeConformData::afxXM_AltitudeConformData(const afxXM_AltitudeConformData& other, bool temp_clone) 
  : afxXM_WeightedBaseData(other, temp_clone)
{
  height = other.height;
  do_terrain = other.do_terrain;
  do_interiors = other.do_interiors;
  do_freeze = other.do_freeze;
  interior_types = other.interior_types;
  terrain_types = other.terrain_types;
}

void afxXM_AltitudeConformData::initPersistFields()
{
  addField("height",              TypeF32,      Offset(height, afxXM_AltitudeConformData),
    "...");
  addField("conformToTerrain",    TypeBool,     Offset(do_terrain, afxXM_AltitudeConformData),
    "...");
  addField("conformToInteriors",  TypeBool,     Offset(do_interiors, afxXM_AltitudeConformData),
    "...");
  addField("freeze",              TypeBool,     Offset(do_freeze, afxXM_AltitudeConformData),
    "...");
  addField("interiorTypes",       TypeS32,      Offset(interior_types, afxXM_AltitudeConformData),
    "...");
  addField("terrainTypes",        TypeS32,      Offset(terrain_types, afxXM_AltitudeConformData),
    "...");

  Parent::initPersistFields();
}

void afxXM_AltitudeConformData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(height);
  stream->writeFlag(do_terrain);
  stream->writeFlag(do_interiors);
  stream->writeFlag(do_freeze);
  stream->write(interior_types);
  stream->write(terrain_types);
}

void afxXM_AltitudeConformData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&height);
  do_terrain = stream->readFlag();
  do_interiors = stream->readFlag();
  do_freeze = stream->readFlag();
  stream->read(&interior_types);
  stream->read(&terrain_types);
}

afxXM_Base* afxXM_AltitudeConformData::create(afxEffectWrapper* fx, bool on_server)
{
  return new afxXM_AltitudeConform(this, fx, on_server);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_AltitudeConform::afxXM_AltitudeConform(afxXM_AltitudeConformData* db, afxEffectWrapper* fxw, bool on_server) 
: afxXM_WeightedBase(db, fxw) 
{ 
  mConformData = db;
  mContainer = (on_server) ? &gServerContainer : &gClientContainer;
  mDo_freeze = db->do_freeze;
  mIs_frozen = false;
  mTerrain_alt = -1.0f;
  mInterior_alt = -1.0f;
  mConformed_pos.zero();
}

void afxXM_AltitudeConform::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (mIs_frozen)
  {
    if (mTerrain_alt >= 0.0f)
      fx_wrapper->setTerrainAltitude(mTerrain_alt);
    if (mInterior_alt >= 0.0f)
      fx_wrapper->setInteriorAltitude(mInterior_alt);
    params.pos = mConformed_pos;
    return;
  }

  RayInfo rInfo1, rInfo2;
  bool hit1 = false, hit2 = false;
  bool hit1_is_interior = false;

  // find primary ground
  Point3F above_pos(params.pos); above_pos.z += 0.1f;
  Point3F below_pos(params.pos); below_pos.z -= 10000;
  hit1 = mContainer->castRay(above_pos, below_pos, mConformData->interior_types | mConformData->terrain_types, &rInfo1);

  // find secondary ground
  if (hit1 && rInfo1.object)
  {
    hit1_is_interior = ((rInfo1.object->getTypeMask() & mConformData->interior_types) != 0);
    U32 mask = (hit1_is_interior) ? mConformData->terrain_types : mConformData->interior_types;
    hit2 = mContainer->castRay(above_pos, below_pos, mask, &rInfo2);
  }

  if (hit1)
  {
    F32 wt_factor = calc_weight_factor(elapsed);
    F32 incoming_z = params.pos.z;
    F32 ground1_z = rInfo1.point.z + mConformData->height;
    F32 pos_z = ground1_z + (1.0f - wt_factor)*(incoming_z - ground1_z);

    if (hit1_is_interior)
    {
      mInterior_alt = incoming_z - pos_z;
      fx_wrapper->setInteriorAltitude(mInterior_alt);
      if (mConformData->do_interiors)
        params.pos.z = pos_z;
    }
    else
    {
      mTerrain_alt = incoming_z - pos_z;
      fx_wrapper->setTerrainAltitude(mTerrain_alt);
      if (mConformData->do_terrain)
        params.pos.z = pos_z;
    }

    if (hit2)
    {
      F32 ground2_z = rInfo2.point.z + mConformData->height;
      F32 z2 = ground2_z + (1.0f - wt_factor)*(incoming_z - ground2_z);
      if (hit1_is_interior)
      {
        mTerrain_alt = incoming_z - z2;
        fx_wrapper->setTerrainAltitude(mTerrain_alt);
      }
      else
      {
        mInterior_alt = incoming_z - z2;
        fx_wrapper->setInteriorAltitude(mInterior_alt);
      }
    }

    // check for case where interior is underground
    else if (hit1_is_interior)
    {
      RayInfo rInfo0;
      Point3F lookup_from_pos(params.pos); lookup_from_pos.z -= 0.1f;
      Point3F lookup_to_pos(params.pos); lookup_to_pos.z += 10000;
      if (mContainer->castRay(lookup_from_pos, lookup_to_pos, TerrainObjectType, &rInfo0))
      {
        F32 ground2_z = rInfo0.point.z + mConformData->height;
        F32 z2 = ground2_z + (1.0f - wt_factor)*(incoming_z - ground2_z);
        mTerrain_alt = z2 - incoming_z;
        fx_wrapper->setTerrainAltitude(mTerrain_alt);
      }
    }

    if (mDo_freeze)
    {
      mConformed_pos = params.pos;
      mIs_frozen = true;
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

