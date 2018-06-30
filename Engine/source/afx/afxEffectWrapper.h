
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

#ifndef _AFX_EFFECT_WRAPPER_H_
#define _AFX_EFFECT_WRAPPER_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"
#include "afxEffectDefs.h"
#include "afxConstraint.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

struct afxEffectTimingData
{
  F32     delay;
  F32     lifetime;
  F32     fade_in_time;
  F32     fade_out_time;
  F32     residue_lifetime;   
  F32     residue_fadetime;
  F32     life_bias;
  Point2F fadein_ease;
  Point2F fadeout_ease;

  afxEffectTimingData()
  {
    delay = 0.0f;
    lifetime = 0.0f;
    fade_in_time = 0.0f;
    fade_out_time = 0.0f;
    residue_lifetime = 0.0f;
    residue_fadetime = 0.0f;
    life_bias = 1.0f;
    fadein_ease.set(0.0f, 1.0f);
    fadeout_ease.set(0.0f, 1.0f);
  }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEffectWrapperData;
class afxAnimCurve;

class afxEffectAdapterDesc
{
private:
  static Vector<afxEffectAdapterDesc*>* adapters;

public:
  /*C*/         afxEffectAdapterDesc();

  virtual bool  testEffectType(const SimDataBlock*) const=0;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const=0;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const=0;
  virtual bool  runsOnClient(const afxEffectWrapperData*) const=0;
  virtual bool  isPositional(const afxEffectWrapperData*) const { return true; }
  virtual void  prepEffect(afxEffectWrapperData*) const { }

  virtual afxEffectWrapper* create() const=0;

  static bool   identifyEffect(afxEffectWrapperData*);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxXM_BaseData;

class afxEffectBaseData : public GameBaseData, public afxEffectDefs
{
  typedef GameBaseData  Parent;

public:
  /*C*/           afxEffectBaseData() { }
  /*C*/           afxEffectBaseData(const afxEffectBaseData& other, bool temp=false)  : GameBaseData(other, temp){ }

  virtual void    gather_cons_defs(Vector<afxConstraintDef>& defs) { };

  DECLARE_CONOBJECT(afxEffectBaseData);
  DECLARE_CATEGORY("AFX");
};

//class afxEffectWrapperData : public GameBaseData, public afxEffectDefs
class afxEffectWrapperData : public afxEffectBaseData
{
  //typedef GameBaseData  Parent;
  typedef afxEffectBaseData  Parent;

  bool                  do_id_convert;

public:
  enum  { MAX_CONDITION_STATES = 4 };

public:
  StringTableEntry      effect_name; 
  bool                  use_as_cons_obj;    
  bool                  use_ghost_as_cons_obj;    

  StringTableEntry      cons_spec; 
  StringTableEntry      pos_cons_spec;
  StringTableEntry      orient_cons_spec;
  StringTableEntry      aim_cons_spec;
  StringTableEntry      life_cons_spec;
  //
  afxConstraintDef      cons_def;
  afxConstraintDef      pos_cons_def;
  afxConstraintDef      orient_cons_def;
  afxConstraintDef      aim_cons_def;
  afxConstraintDef      life_cons_def;

  afxEffectTimingData   ewd_timing;
  U32                   inherit_timing;
  
  F32                   scale_factor;       // scale size if applicable
  F32                   rate_factor;        // scale rate if applicable
  F32                   user_fade_out_time;

  bool                  is_looping;
  U32                   n_loops;
  F32                   loop_gap_time;

  bool                  ignore_time_factor;
  bool                  propagate_time_factor;

  ByteRange             ranking_range;
  ByteRange             lod_range;
  S32                   life_conds;
  bool                  effect_enabled;
  U32                   exec_cond_on_bits[MAX_CONDITION_STATES];
  U32                   exec_cond_off_bits[MAX_CONDITION_STATES];
  U32                   exec_cond_bitmasks[MAX_CONDITION_STATES];

  S32                   data_ID;

  afxXM_BaseData*       xfm_modifiers[MAX_XFM_MODIFIERS];

  Box3F                 forced_bbox;
  bool                  update_forced_bbox;

  S8                    sort_priority;
  Point3F               direction;
  F32                   speed;
  F32                   mass;

  bool                  borrow_altitudes;
  StringTableEntry      vis_keys_spec;
  afxAnimCurve*         vis_keys;

  SimDataBlock*         effect_data;
  afxEffectAdapterDesc* effect_desc;

  S32                   group_index;

  void                  parse_cons_specs();
  void                  parse_vis_keys();
  void                  gather_cons_defs(Vector<afxConstraintDef>& defs);
  void                  pack_mods(BitStream*, afxXM_BaseData* mods[], bool packed);
  void                  unpack_mods(BitStream*, afxXM_BaseData* mods[]);

public:
  /*C*/             afxEffectWrapperData();
  /*C*/             afxEffectWrapperData(const afxEffectWrapperData&, bool = false);
  /*D*/             ~afxEffectWrapperData();

  virtual bool      onAdd();
  virtual void      packData(BitStream*);
  virtual void      unpackData(BitStream*);

  bool              preload(bool server, String &errorStr);

  virtual void      onPerformSubstitutions();

  bool              requiresStop(const afxEffectTimingData& timing) { return effect_desc->requiresStop(this, timing); }
  bool              runsOnServer() { return effect_desc->runsOnServer(this); }
  bool              runsOnClient() { return effect_desc->runsOnClient(this); }
  bool              runsHere(bool server_here) { return (server_here) ? runsOnServer() : runsOnClient(); }
  bool              isPositional() { return effect_desc->isPositional(this); }
  bool              testExecConditions(U32 conditions);

  F32               afterStopTime() { return ewd_timing.fade_out_time; }

  virtual bool      allowSubstitutions() const { return true; }

  static void       initPersistFields();

  DECLARE_CONOBJECT(afxEffectWrapperData);
  DECLARE_CATEGORY("AFX");
};

inline bool afxEffectWrapperData::testExecConditions(U32 conditions)
{
  if (exec_cond_bitmasks[0] == 0)
    return true;

  if ((exec_cond_bitmasks[0] & conditions) == exec_cond_on_bits[0])
    return true;

  for (S32 i = 1; i < MAX_CONDITION_STATES; i++)
  {
    if (exec_cond_bitmasks[i] == 0)
      return false;
    if ((exec_cond_bitmasks[i] & conditions) == exec_cond_on_bits[i])
      return true;
  }
  return false;
}

typedef Vector<afxEffectBaseData*> afxEffectList;

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectWrapper 
//
//  NOTE -- this not a subclass of GameBase... it is only meant to exist on
//    the client-side.

class ShapeBase;
class GameBase;
class TSShape;
class TSShapeInstance;
class SceneObject;
class afxConstraint;
class afxConstraintMgr;
class afxChoreographer;
class afxXM_Base;

class afxEffectWrapper : public SimObject,  public afxEffectDefs
{
  typedef SimObject Parent;
  friend class afxEffectVector;

private:
  bool              test_life_conds();

protected:
  afxEffectWrapperData* mDatablock;

  afxEffectTimingData   mEW_timing;

  F32               mFade_in_end;
  F32               mFade_out_start;
  F32               mFull_lifetime;

  F32               mTime_factor;
  F32               mProp_time_factor;

  afxChoreographer* mChoreographer;
  afxConstraintMgr* mCons_mgr;

  afxConstraintID   mPos_cons_id;
  afxConstraintID   mOrient_cons_id;
  afxConstraintID   mAim_cons_id;
  afxConstraintID   mLife_cons_id;

  afxConstraintID   mEffect_cons_id;

  F32               mElapsed;
  F32               mLife_elapsed;
  F32               mLife_end;
  bool              mStopped;
  bool              mCond_alive;

  U32               mNum_updates;

  MatrixF           mUpdated_xfm;
  Point3F           mUpdated_pos;
  Point3F           mUpdated_aim;
  Point3F           mUpdated_scale;
  LinearColorF      mUpdated_color;

  F32               mFade_value;
  F32               mLast_fade_value;

  bool              mDo_fade_inout;
  bool              mDo_fades;
  bool              mIn_scope;
  bool              mIs_aborted;

  U8                mEffect_flags;

  afxXM_Base*       mXfm_modifiers[MAX_XFM_MODIFIERS];

  F32               mLive_scale_factor;
  F32               mLive_fade_factor;
  F32               mTerrain_altitude;
  F32               mInterior_altitude;

  S32               mGroup_index;

public:
  /*C*/             afxEffectWrapper();
  virtual           ~afxEffectWrapper();

  void              ew_init(afxChoreographer*, afxEffectWrapperData*, afxConstraintMgr*, 
                            F32 time_factor);

  F32               getFullLifetime() { return mEW_timing.lifetime + mEW_timing.fade_out_time; }
  F32               getTimeFactor() { return mTime_factor; }
  afxConstraint*    getPosConstraint() { return mCons_mgr->getConstraint(mPos_cons_id); }
  afxConstraint*    getOrientConstraint() { return mCons_mgr->getConstraint(mOrient_cons_id); }
  afxConstraint*    getAimConstraint() { return mCons_mgr->getConstraint(mAim_cons_id); }
  afxConstraint*    getLifeConstraint() { return mCons_mgr->getConstraint(mLife_cons_id); }
  afxChoreographer* getChoreographer() { return mChoreographer; }

  virtual bool      isDone();
  virtual bool      deleteWhenStopped() { return false; }
  F32               afterStopTime() { return mEW_timing.fade_out_time; }
  bool              isAborted() const { return mIs_aborted; }

  void              prestart();
  bool              start(F32 timestamp);
  bool              update(F32 dt);
  void              stop();
  void              cleanup(bool was_stopped=false);
  void              setScopeStatus(bool flag);

  virtual void      ea_set_datablock(SimDataBlock*) { }
  virtual bool      ea_start() { return true; }
  virtual bool      ea_update(F32 dt) { return true; }
  virtual void      ea_finish(bool was_stopped) { }
  virtual void      ea_set_scope_status(bool flag) { }
  virtual bool      ea_is_enabled() { return true; }
  virtual SceneObject* ea_get_scene_object() const { return 0; }
  U32               ea_get_triggers() const { return 0; }

  void              getUpdatedPosition(Point3F& pos) { pos = mUpdated_pos;}
  void              getUpdatedTransform(MatrixF& xfm) { xfm = mUpdated_xfm; }
  void              getUpdatedScale(Point3F& scale) { scale = mUpdated_scale; }
  void              getUpdatedColor(LinearColorF& color) { color = mUpdated_color; }
  virtual void      getUpdatedBoxCenter(Point3F& pos) { pos = mUpdated_pos;}

  virtual void      getUnconstrainedPosition(Point3F& pos) { pos.zero();}
  virtual void      getUnconstrainedTransform(MatrixF& xfm) { xfm.identity(); }
  virtual void      getBaseColor(LinearColorF& color) { color.set(1.0f, 1.0f, 1.0f, 1.0f); }

  SceneObject*      getSceneObject() const { return ea_get_scene_object(); }
  U32               getTriggers() const { return ea_get_triggers(); }

  F32               getMass() { return mDatablock->mass; }
  Point3F           getDirection() { return mDatablock->direction; }
  F32               getSpeed() { return mDatablock->speed; }

  virtual TSShape*          getTSShape() { return 0; }
  virtual TSShapeInstance*  getTSShapeInstance() { return 0; }

  virtual U32       setAnimClip(const char* clip, F32 pos, F32 rate, F32 trans) { return 0; }
  virtual void      resetAnimation(U32 tag) { }
  virtual F32       getAnimClipDuration(const char* clip) { return 0.0f; }

  void              setTerrainAltitude(F32 alt) { mTerrain_altitude = alt; }
  void              setInteriorAltitude(F32 alt) { mInterior_altitude = alt; }
  void              getAltitudes(F32& terr_alt, F32& inter_alt) const { terr_alt = mTerrain_altitude; inter_alt = mInterior_altitude; }

  void              setGroupIndex(S32 idx) { mGroup_index = idx; }
  S32               getGroupIndex() const { return mGroup_index; }

  bool              inScope() const { return mIn_scope; }

public:
  static void       initPersistFields();

  static afxEffectWrapper* ew_create(afxChoreographer*, afxEffectWrapperData*, afxConstraintMgr*, F32 time_factor, S32 group_index=0);

  DECLARE_CONOBJECT(afxEffectWrapper);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_EFFECT_WRAPPER_H_
