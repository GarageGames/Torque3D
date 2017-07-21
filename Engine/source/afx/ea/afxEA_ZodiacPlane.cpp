
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
#include "afx/util/afxEase.h"
#include "afx/afxResidueMgr.h"
#include "afx/ce/afxZodiacPlane.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_ZodiacPlane

class afxEA_ZodiacPlane : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxZodiacPlaneData*  zode_data;
  afxZodiacPlane*      pzode;

  F32               zode_angle_offset;
  AngAxisF          aa_rot;

  F32               live_color_factor;
  ColorF            live_color;

  F32               calc_facing_angle();
  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_ZodiacPlane();
  /*D*/             ~afxEA_ZodiacPlane();

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
  virtual void      ea_finish(bool was_stopped);
  virtual void      ea_set_scope_status(bool flag);
  virtual void      onDeleteNotify(SimObject*);
  virtual void      getUpdatedBoxCenter(Point3F& pos);
  virtual void      getBaseColor(ColorF& color) { color = zode_data->color; }
};

F32 afxEA_ZodiacPlane::calc_facing_angle() 
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

afxEA_ZodiacPlane::afxEA_ZodiacPlane()
{
  zode_data = 0;
  pzode = 0;
  zode_angle_offset = 0;
  live_color.set(1,1,1,1);
  live_color_factor = 0.0f;
}

afxEA_ZodiacPlane::~afxEA_ZodiacPlane()
{
  if (pzode)
    pzode->deleteObject();
  if (zode_data && zode_data->isTempClone())
    delete zode_data;
  zode_data = 0;
}

void afxEA_ZodiacPlane::ea_set_datablock(SimDataBlock* db)
{
  zode_data = dynamic_cast<afxZodiacPlaneData*>(db);
}

bool afxEA_ZodiacPlane::ea_start()
{
  if (!zode_data)
  {
    Con::errorf("afxEA_ZodiacPlane::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  if (!zode_data->use_full_xfm)
    zode_angle_offset = calc_facing_angle();

  switch (zode_data->face_dir)
  {
  case afxZodiacPlaneData::FACES_UP:
    aa_rot.set(Point3F(0.0f,0.0f,1.0f),0.0f);
    break;  
  case afxZodiacPlaneData::FACES_DOWN:
    aa_rot.set(Point3F(0.0f,0.0f,-1.0f),0.0f);
    break;  
  case afxZodiacPlaneData::FACES_FORWARD:
    aa_rot.set(Point3F(0.0f,1.0f,0.0f),0.0f);
    break;  
  case afxZodiacPlaneData::FACES_BACK:
    aa_rot.set(Point3F(0.0f,-1.0f,0.0f),0.0f);
    break;  
  case afxZodiacPlaneData::FACES_RIGHT:
    aa_rot.set(Point3F(1.0f,0.0f,0.0f),0.0f);
    break;  
  case afxZodiacPlaneData::FACES_LEFT:
    aa_rot.set(Point3F(-1.0f,0.0f,0.0f),0.0f);
    break;  
  }

  return true;
}

bool afxEA_ZodiacPlane::ea_update(F32 dt)
{
  if (!pzode)
  {
    // create and register effect
    pzode = new afxZodiacPlane();
    pzode->onNewDataBlock(zode_data, false);
    if (!pzode->registerObject())
    {
      delete pzode;
      pzode = 0;
      Con::errorf("afxEA_ZodiacPlane::ea_update() -- effect failed to register.");
      return false;
    }
    deleteNotify(pzode);

    ///pzode->setSequenceRateFactor(datablock->rate_factor/prop_time_factor);
    ///pzode->setSortPriority(datablock->sort_priority);
  }

  if (pzode)
  {
    //ColorF zode_color = zode_data->color;
    ColorF zode_color = updated_color;

    if (live_color_factor > 0.0)
       zode_color.interpolate(zode_color, live_color, live_color_factor);

    if (do_fades)
    {
      if (zode_data->blend_flags == afxZodiacDefs::BLEND_SUBTRACTIVE)
        zode_color *= fade_value*live_fade_factor;
      else
        zode_color.alpha *= fade_value*live_fade_factor;
    }

    // scale and grow zode
    //F32 zode_radius = zode_data->radius_xy*updated_scale.x + life_elapsed*zode_data->growth_rate;
    F32 zode_radius = zode_data->radius_xy + life_elapsed*zode_data->growth_rate;

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

    if (zode_data->respect_ori_cons && !zode_data->use_full_xfm)
    {
      VectorF shape_vec;
      updated_xfm.getColumn(1, &shape_vec);
      shape_vec.normalize();

      F32 ang;

      switch (zode_data->face_dir)
      {
      case afxZodiacPlaneData::FACES_FORWARD:
      case afxZodiacPlaneData::FACES_BACK:
        ang = mAtan2(shape_vec.x, shape_vec.z);
        break;  
      case afxZodiacPlaneData::FACES_RIGHT:
      case afxZodiacPlaneData::FACES_LEFT:
        ang = mAtan2(shape_vec.y, shape_vec.z);
        break;  
      case afxZodiacPlaneData::FACES_UP:
      case afxZodiacPlaneData::FACES_DOWN:
      default:
        ang = mAtan2(shape_vec.x, shape_vec.y);
        break;  
      }

      if (ang < 0.0f)
        ang += M_2PI_F;

      switch (zode_data->face_dir)
      {
      case afxZodiacPlaneData::FACES_DOWN:
      case afxZodiacPlaneData::FACES_BACK:
      case afxZodiacPlaneData::FACES_LEFT:
        ang = -ang;
        break;  
      }

      zode_angle_offset = mRadToDeg(ang); 
    }

    F32 zode_angle = zode_data->calcRotationAngle(life_elapsed, datablock->rate_factor/prop_time_factor);
    zode_angle = mFmod(zode_angle + zode_angle_offset, 360.0f); 
    aa_rot.angle = mDegToRad(zode_angle);
    
    MatrixF spin_xfm; 
    aa_rot.setMatrix(&spin_xfm);

    // set color, radius
    pzode->setColor(zode_color);
    pzode->setRadius(zode_radius);
    if (zode_data->use_full_xfm)
    {
      updated_xfm.mul(spin_xfm);
      pzode->setTransform(updated_xfm);
    }
    else
      pzode->setTransform(spin_xfm);
    pzode->setPosition(updated_pos);
    pzode->setScale(updated_scale);
  }

  return true;
}

void afxEA_ZodiacPlane::ea_finish(bool was_stopped)
{
  if (!pzode)
    return;
  
  pzode->deleteObject();
  pzode = 0;
}

void afxEA_ZodiacPlane::ea_set_scope_status(bool in_scope)
{
  if (pzode)
    pzode->setVisibility(in_scope);
}

void afxEA_ZodiacPlane::onDeleteNotify(SimObject* obj)
{
  if (pzode == dynamic_cast<afxZodiacPlane*>(obj))
    pzode = 0;

  Parent::onDeleteNotify(obj);
}

void afxEA_ZodiacPlane::getUpdatedBoxCenter(Point3F& pos)
{
  if (pzode)
    pos = pzode->getBoxCenter();
}

void afxEA_ZodiacPlane::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (zode_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxZodiacPlaneData* orig_db = zode_data;
    zode_data = new afxZodiacPlaneData(*orig_db, true);
    orig_db->performSubstitutions(zode_data, choreographer, group_index);
  }
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_ZodiacPlaneDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_ZodiacPlaneDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }

  virtual afxEffectWrapper* create() const { return new afxEA_ZodiacPlane; }
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_ZodiacPlaneDesc afxEA_ZodiacPlaneDesc::desc;

bool afxEA_ZodiacPlaneDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxZodiacPlaneData) == typeid(*db));
}

bool afxEA_ZodiacPlaneDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//