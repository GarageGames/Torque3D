
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

#include <typeinfo>
#include "afx/arcaneFX.h"

#include "math/mathUtils.h"

#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/afxResidueMgr.h"
#include "afx/util/afxEase.h"
#include "afx/ce/afxZodiacMgr.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_Zodiac 

class afxEA_Zodiac : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxZodiacData*    zode_data;
  Point3F           zode_pos;
  F32               zode_radius;
  Point2F           zode_vrange;
  ColorF            zode_color;
  F32               zode_angle;
  F32               zode_angle_offset;

  F32               live_color_factor;
  ColorF            live_color;
  bool              became_residue;
  bool              do_altitude_bias;
  F32               altitude_falloff_range;

  F32               calc_facing_angle();
  F32               calc_terrain_alt_bias();
  F32               calc_interior_alt_bias();
  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_Zodiac();
  /*C*/             ~afxEA_Zodiac();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);

  virtual bool      ea_is_enabled() { return true; }

  virtual void      getBaseColor(ColorF& color) { color = zode_data->color; }

  static void       initPersistFields();

  //DECLARE_CONOBJECT(afxEA_Zodiac);
  DECLARE_CATEGORY("AFX");
};

//IMPLEMENT_CONOBJECT(afxEA_Zodiac);

//~~~~~~~~~~~~~~~~~~~~//

F32 afxEA_Zodiac::calc_facing_angle() 
{
  // get direction player is facing
  VectorF shape_vec;
  MatrixF shape_xfm;

  afxConstraint* orient_constraint = getOrientConstraint();
  if (orient_constraint)
    orient_constraint->getTransform(shape_xfm);
  else
    shape_xfm.identity();

  shape_xfm.getColumn(1, &shape_vec);
  shape_vec.z = 0.0f;
  shape_vec.normalize();

  F32 pitch, yaw;
  MathUtils::getAnglesFromVector(shape_vec, yaw, pitch);

  return mRadToDeg(yaw); 
}

inline F32 afxEA_Zodiac::calc_terrain_alt_bias()
{
  if (terrain_altitude >= zode_data->altitude_max)
    return 0.0f;
  return 1.0f - (terrain_altitude - zode_data->altitude_falloff)/altitude_falloff_range;
}

inline F32 afxEA_Zodiac::calc_interior_alt_bias()
{
  if (interior_altitude >= zode_data->altitude_max)
    return 0.0f;
  return 1.0f - (interior_altitude - zode_data->altitude_falloff)/altitude_falloff_range;
}

afxEA_Zodiac::afxEA_Zodiac()
{
  zode_data = 0;
  zode_pos.zero();
  zode_radius = 1;
  zode_vrange.set(1,1);
  zode_color.set(1,1,1,1);
  zode_angle = 0;
  zode_angle_offset = 0;
  live_color.set(1,1,1,1);
  live_color_factor = 0.0f;
  do_altitude_bias = false;
  altitude_falloff_range = 0.0f;
  became_residue = false;
}

afxEA_Zodiac::~afxEA_Zodiac()
{
  if (!became_residue && zode_data && zode_data->isTempClone())
    delete zode_data;
  zode_data = 0;
}

void afxEA_Zodiac::ea_set_datablock(SimDataBlock* db)
{
  zode_data = dynamic_cast<afxZodiacData*>(db);
  if (zode_data)
  {
    do_altitude_bias = (zode_data->altitude_max > 0.0f && (zode_data->altitude_shrinks || zode_data->altitude_fades));
    altitude_falloff_range = zode_data->altitude_max - zode_data->altitude_falloff;
  }
}

bool afxEA_Zodiac::ea_start()
{
  if (!zode_data)
  {
    Con::errorf("afxEA_Zodiac::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  zode_angle_offset = calc_facing_angle();

  return true;
}

bool afxEA_Zodiac::ea_update(F32 dt)
{
  if (!in_scope)
    return true;

  //~~~~~~~~~~~~~~~~~~~~//
  // Zodiac Color

  zode_color = updated_color;

  if (live_color_factor > 0.0)
  {
     zode_color.interpolate(zode_color, live_color, live_color_factor);
     //Con::printf("LIVE-COLOR %g %g %g %g  FACTOR is %g", 
     //   live_color.red, live_color.green, live_color.blue, live_color.alpha, 
     //   live_color_factor);
  }
  else
  {
     //Con::printf("LIVE-COLOR-FACTOR is ZERO");
  }

  if (do_fades)
  {
    if (fade_value < 0.01f)
      return true; // too transparent

    if (zode_data->blend_flags == afxZodiacDefs::BLEND_SUBTRACTIVE)
      zode_color *= fade_value*live_fade_factor;
    else
      zode_color.alpha *= fade_value*live_fade_factor;
  }

  if (zode_color.alpha < 0.01f)
    return true;

  //~~~~~~~~~~~~~~~~~~~~//
  // Zodiac

  // scale and grow zode
  zode_radius = zode_data->radius_xy*updated_scale.x + life_elapsed*zode_data->growth_rate;

  // zode is growing
  if (life_elapsed < zode_data->grow_in_time)
  {
    F32 t = life_elapsed/zode_data->grow_in_time;
    zode_radius = afxEase::eq(t, 0.001f, zode_radius, 0.2f, 0.8f);
  }
  // zode is shrinking
  else if (full_lifetime - life_elapsed < zode_data->shrink_out_time)
  {
    F32 t = (full_lifetime - life_elapsed)/zode_data->shrink_out_time;
    zode_radius = afxEase::eq(t, 0.001f, zode_radius, 0.0f, 0.9f);
  }

  zode_radius *= live_scale_factor;

  if (zode_radius < 0.001f)
    return true; // too small

  zode_vrange = zode_data->vert_range;
  if (zode_data->scale_vert_range)
  {
    F32 scale_factor = zode_radius/zode_data->radius_xy;
    zode_vrange *= scale_factor;
  }

  //~~~~~~~~~~~~~~~~~~~~//
  // Zodiac Position

  zode_pos = updated_pos;

  //~~~~~~~~~~~~~~~~~~~~//
  // Zodiac Rotation 

  if (zode_data->respect_ori_cons)
  {
    afxConstraint* orient_constraint = getOrientConstraint();
    if (orient_constraint)
    {
      VectorF shape_vec;
      updated_xfm.getColumn(1, &shape_vec);
      shape_vec.z = 0.0f;
      shape_vec.normalize();
      F32 pitch, yaw;
      MathUtils::getAnglesFromVector(shape_vec, yaw, pitch);
      zode_angle_offset = mRadToDeg(yaw); 
    }
  }

  zode_angle = zode_data->calcRotationAngle(life_elapsed, datablock->rate_factor/prop_time_factor);
  zode_angle = mFmod(zode_angle + zode_angle_offset, 360.0f);     

  //~~~~~~~~~~~~~~~~~~~~//
  // post zodiac
  if ((zode_data->zflags & afxZodiacDefs::SHOW_ON_TERRAIN) != 0)
  {
    if (do_altitude_bias && terrain_altitude > zode_data->altitude_falloff)
    {
      F32 alt_bias = calc_terrain_alt_bias();
      if (alt_bias > 0.0f)
      {
        F32 alt_rad = zode_radius;
        if (zode_data->altitude_shrinks)
          alt_rad *= alt_bias;
        ColorF alt_clr(zode_color.red, zode_color.green, zode_color.blue, zode_color.alpha);
        if (zode_data->altitude_fades)
          alt_clr.alpha *= alt_bias;
        afxZodiacMgr::addTerrainZodiac(zode_pos, alt_rad, alt_clr, zode_angle, zode_data);
      }
    }
    else
    {
      afxZodiacMgr::addTerrainZodiac(zode_pos, zode_radius, zode_color, zode_angle, zode_data);
    }
  }

  if ((zode_data->zflags & afxZodiacDefs::SHOW_ON_INTERIORS) != 0)
  {
    if (do_altitude_bias && interior_altitude > zode_data->altitude_falloff)
    {
      F32 alt_bias = calc_interior_alt_bias();
      if (alt_bias > 0.0f)
      {
        F32 alt_rad = zode_radius;
        if (zode_data->altitude_shrinks)
          alt_rad *= alt_bias;
        ColorF alt_clr(zode_color.red, zode_color.green, zode_color.blue, zode_color.alpha);
        if (zode_data->altitude_fades)
          alt_clr.alpha *= alt_bias;
        afxZodiacMgr::addInteriorZodiac(zode_pos, alt_rad, zode_vrange, alt_clr, zode_angle, zode_data);
      }
    }
    else    
      afxZodiacMgr::addInteriorZodiac(zode_pos, zode_radius, zode_vrange, zode_color, zode_angle, zode_data);
  }

  return true;
}

void afxEA_Zodiac::ea_finish(bool was_stopped)
{
  if (in_scope && ew_timing.residue_lifetime > 0)
  {
    if (do_fades)
    {
      if (fade_value < 0.01f)
        return;
      zode_color.alpha *= fade_value;
    }
    if ((zode_data->zflags & afxZodiacDefs::SHOW_ON_TERRAIN) != 0)
    {
      if (do_altitude_bias && terrain_altitude > zode_data->altitude_falloff)
      {
        F32 alt_bias = calc_terrain_alt_bias();
        if (alt_bias > 0.0f)
        {
          F32 alt_rad = zode_radius;
          if (zode_data->altitude_shrinks)
            alt_rad *= alt_bias;
          ColorF alt_clr(zode_color.red, zode_color.green, zode_color.blue, zode_color.alpha);
          if (zode_data->altitude_fades)
            zode_color.alpha *= alt_bias;
          became_residue = true;
          afxResidueMgr::add_terrain_zodiac(ew_timing.residue_lifetime, ew_timing.residue_fadetime, zode_data, zode_pos, alt_rad, 
                                            alt_clr, zode_angle);
        }
      }
      else
      {
        became_residue = true;
        afxResidueMgr::add_terrain_zodiac(ew_timing.residue_lifetime, ew_timing.residue_fadetime, zode_data, zode_pos, zode_radius, 
                                          zode_color, zode_angle);
      }
    }
    if ((zode_data->zflags & afxZodiacDefs::SHOW_ON_INTERIORS) != 0)
    {
      if (do_altitude_bias && interior_altitude > zode_data->altitude_falloff)
      {
        F32 alt_bias = calc_interior_alt_bias();
        if (alt_bias > 0.0f)
        {
          F32 alt_rad = zode_radius;
          if (zode_data->altitude_shrinks)
            alt_rad *= alt_bias;
          ColorF alt_clr(zode_color.red, zode_color.green, zode_color.blue, zode_color.alpha);
          if (zode_data->altitude_fades)
            zode_color.alpha *= alt_bias;

          afxZodiacData* temp_zode = zode_data;
          if (became_residue)
            temp_zode = new afxZodiacData(*zode_data, true);
          became_residue = true;
          afxResidueMgr::add_interior_zodiac(ew_timing.residue_lifetime, ew_timing.residue_fadetime, temp_zode, zode_pos, alt_rad, 
                                             zode_vrange, alt_clr, zode_angle);
        }

      }
      else
      {
        afxZodiacData* temp_zode = zode_data;
        if (became_residue)
          temp_zode = new afxZodiacData(*zode_data, true);
        became_residue = true;
        afxResidueMgr::add_interior_zodiac(ew_timing.residue_lifetime, ew_timing.residue_fadetime, temp_zode, zode_pos, zode_radius, 
                                           zode_vrange, zode_color, zode_angle);
      }
    }
  }
}

void afxEA_Zodiac::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (zode_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxZodiacData* orig_db = zode_data;
    zode_data = new afxZodiacData(*orig_db, true);
    orig_db->performSubstitutions(zode_data, choreographer, group_index);
  }
}

#undef myOffset
#define myOffset(field) Offset(field, afxEA_Zodiac)

void afxEA_Zodiac::initPersistFields()
{
  addField("liveColor",         TypeColorF,     myOffset(live_color),
    "...");
  addField("liveColorFactor",   TypeF32,        myOffset(live_color_factor),
    "...");

  Parent::initPersistFields();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_ZodiacDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_ZodiacDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }

  virtual afxEffectWrapper* create() const { return new afxEA_Zodiac; }
};

afxEA_ZodiacDesc afxEA_ZodiacDesc::desc;

bool afxEA_ZodiacDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxZodiacData) == typeid(*db));
}

bool afxEA_ZodiacDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//