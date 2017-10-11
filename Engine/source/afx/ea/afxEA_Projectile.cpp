
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

#include "lighting/lightInfo.h"
#include "T3D/projectile.h"

#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/ce/afxProjectile.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_Projectile 

class afxEA_Projectile : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  ProjectileData*     projectile_data;
  afxProjectileData*  afx_projectile_data;
  afxProjectile*      projectile;
  bool                launched;
  bool                impacted;
  bool                projectile_done;
  afxConstraint*      launch_cons;
  Point3F             launch_dir_bias;

  void                do_runtime_substitutions();

public:
  /*C*/               afxEA_Projectile();
  /*D*/               ~afxEA_Projectile();

  virtual bool        isDone();

  virtual void        ea_set_datablock(SimDataBlock*);
  virtual bool        ea_start();
  virtual bool        ea_update(F32 dt);
  virtual void        ea_finish(bool was_stopped);

  virtual void        onDeleteNotify(SimObject*);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_Projectile::afxEA_Projectile()
{
  projectile_data = 0;
  afx_projectile_data = 0;
  projectile = 0;
  launched = false;
  impacted = false;
  projectile_done = false;
  launch_cons = 0;
  launch_dir_bias.zero();
}

afxEA_Projectile::~afxEA_Projectile()
{
  if (projectile)
    clearNotify(projectile);
  //if (projectile_data && projectile_data->isTempClone())
  //  delete projectile_data;
  projectile_data = 0;
  afx_projectile_data = 0;
}

bool afxEA_Projectile::isDone()
{
  return (datablock->use_as_cons_obj || datablock->use_ghost_as_cons_obj) ? projectile_done : impacted;
}

void afxEA_Projectile::ea_set_datablock(SimDataBlock* db)
{
  projectile_data = dynamic_cast<ProjectileData*>(db);
  afx_projectile_data =  dynamic_cast<afxProjectileData*>(projectile_data);
}

bool afxEA_Projectile::ea_start()
{
  if (!projectile_data)
  {
    Con::errorf("afxEA_Projectile::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  if (!afx_projectile_data)
  {
    projectile = new afxProjectile();
  }
  else
  {
    if (datablock->use_ghost_as_cons_obj && datablock->effect_name != ST_NULLSTRING)
      projectile = new afxProjectile(afx_projectile_data->networking, choreographer->getChoreographerId(), datablock->effect_name);
    else
      projectile = new afxProjectile(afx_projectile_data->networking, 0, ST_NULLSTRING);
    projectile->ignoreSourceTimeout = afx_projectile_data->ignore_src_timeout;
    if (afx_projectile_data->override_collision_masks)
    {
      projectile->dynamicCollisionMask = afx_projectile_data->dynamicCollisionMask;
      projectile->staticCollisionMask = afx_projectile_data->staticCollisionMask;
    }
    afxConstraintID launch_pos_id = cons_mgr->getConstraintId(afx_projectile_data->launch_pos_def);
    launch_cons = cons_mgr->getConstraint(launch_pos_id);
    launch_dir_bias = afx_projectile_data->launch_dir_bias;
  }

  projectile->onNewDataBlock(projectile_data, false);

  return true;
}

bool afxEA_Projectile::ea_update(F32 dt)
{
  if (!launched && projectile)
  {
    if (in_scope)
    {
      afxConstraint* pos_cons = getPosConstraint();
      ShapeBase* src_obj = (pos_cons) ? (dynamic_cast<ShapeBase*>(pos_cons->getSceneObject())) : 0;

      F32 muzzle_vel = projectile_data->muzzleVelocity;

      Point3F dir_vec;
      if (afx_projectile_data)
      {
        switch (afx_projectile_data->launch_dir_method)
        {
        case afxProjectileData::OrientConstraint:
          dir_vec.set(0,0,1); 
          updated_xfm.mulV(dir_vec);
          break;
        case afxProjectileData::LaunchDirField:
          dir_vec.set(0,0,1); 
          break;
        case afxProjectileData::TowardPos2Constraint:
        default:
          dir_vec = updated_aim - updated_pos;
          break;
        }
      }
      else
        dir_vec = updated_aim - updated_pos;

      dir_vec.normalizeSafe();
      if (!launch_dir_bias.isZero())
      {
        dir_vec += launch_dir_bias;
        dir_vec.normalizeSafe();
      }
      dir_vec *= muzzle_vel;

      Point3F launch_pos;
      if (launch_cons && launch_cons->getPosition(launch_pos))
      {
        ShapeBase* launch_obj = (launch_cons) ? (dynamic_cast<ShapeBase*>(launch_cons->getSceneObject())) : 0;
        projectile->init(launch_pos, dir_vec, (launch_obj) ? launch_obj : src_obj);
      }
      else
        projectile->init(updated_pos, dir_vec, src_obj);

      if (!projectile->registerObject())
      {
        delete projectile;
        projectile = 0;
        Con::errorf("afxEA_Projectile::ea_update() -- effect failed to register.");
        return false;
      }

      deleteNotify(projectile);

      if (projectile)
        projectile->setDataField(StringTable->insert("afxOwner"), 0, choreographer->getIdString());

    }
    launched = true;
  }

  if (launched && projectile)
  {
    if (in_scope)
    {
      updated_xfm = projectile->getRenderTransform();
      updated_xfm.getColumn(3, &updated_pos);
    }
  }

  return true;
}

void afxEA_Projectile::ea_finish(bool was_stopped)
{
  if (projectile)
  {
    clearNotify(projectile);
    projectile = 0;
  }
  launched = false;
  impacted = false;
}

void afxEA_Projectile::onDeleteNotify(SimObject* obj)
{
  // projectile deleted?
  Projectile* del_projectile = dynamic_cast<Projectile*>(obj);
  if (del_projectile == projectile)
  {
    projectile = NULL;
    projectile_done = true;
  }
}

void afxEA_Projectile::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (projectile_data->getSubstitutionCount() > 0)
  {
    if (typeid(afxProjectileData) == typeid(*projectile_data))
    {
      afxProjectileData* orig_db = (afxProjectileData*)projectile_data;
      afx_projectile_data = new afxProjectileData(*orig_db, true);
      projectile_data = afx_projectile_data;
      orig_db->performSubstitutions(projectile_data, choreographer, group_index);
    }
    else
    {
      // clone the datablock and perform substitutions
      ProjectileData* orig_db = projectile_data;
      afx_projectile_data = 0;
      projectile_data = new ProjectileData(*orig_db, true);
      orig_db->performSubstitutions(projectile_data, choreographer, group_index);
    }
  }
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_ProjectileDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_ProjectileDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const;
  virtual bool  runsOnClient(const afxEffectWrapperData*) const;

  virtual afxEffectWrapper* create() const { return new afxEA_Projectile; }
};

afxEA_ProjectileDesc afxEA_ProjectileDesc::desc;

bool afxEA_ProjectileDesc::testEffectType(const SimDataBlock* db) const
{
  if (typeid(ProjectileData) == typeid(*db))
    return true;
  if (typeid(afxProjectileData) == typeid(*db))
    return true;
  return false;
}

bool afxEA_ProjectileDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return ((ew->use_as_cons_obj || ew->use_ghost_as_cons_obj) && timing.lifetime < 0);
}

bool afxEA_ProjectileDesc::runsOnServer(const afxEffectWrapperData* ew) const
{
  afxProjectileData* afx_projectile_data = dynamic_cast<afxProjectileData*>(ew->effect_data);
  if (!afx_projectile_data)
    return true;

  U8 networking = ((const afxProjectileData*)ew->effect_data)->networking;
  return ((networking & CLIENT_ONLY) == 0);
}

bool afxEA_ProjectileDesc::runsOnClient(const afxEffectWrapperData* ew) const
{ 
  afxProjectileData* afx_projectile_data = dynamic_cast<afxProjectileData*>(ew->effect_data);
  if (!afx_projectile_data)
    return false;

  U8 networking = ((const afxProjectileData*)ew->effect_data)->networking;
  return ((networking & CLIENT_ONLY) != 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//