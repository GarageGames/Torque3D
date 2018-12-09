
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

#ifndef _AFX_MAGIC_SPELL_H_
#define _AFX_MAGIC_SPELL_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "core/util/tVector.h"
#include "console/typeValidators.h"

#include "afxChoreographer.h"
#include "afxEffectDefs.h"
#include "afxEffectWrapper.h"
#include "afxMagicMissile.h"

class afxChoreographerData;
class afxMagicMissileData;
class afxEffectWrapperData;
class SceneObject;
class afxMagicSpell;

class afxMagicSpellDefs
{
public:
  enum
  {
    CASTING_PHRASE,
    LAUNCH_PHRASE,
    DELIVERY_PHRASE,
    IMPACT_PHRASE,
    LINGER_PHRASE,
    NUM_PHRASES
  };
};

class afxMagicSpellData : public afxChoreographerData, public afxMagicSpellDefs
{
  typedef afxChoreographerData Parent;

  class ewValidator : public TypeValidator
  {
    U32 id;
  public:
    ewValidator(U32 id) { this->id = id; }
    void validateType(SimObject *object, void *typePtr);
  };

  bool                  mDo_id_convert;

public:
  F32                   mCasting_dur;
  F32                   mDelivery_dur;
  F32                   mLinger_dur;
  //
  S32                   mNum_casting_loops;
  S32                   mNum_delivery_loops;
  S32                   mNum_linger_loops;
  //
  F32                   mExtra_casting_time;
  F32                   mExtra_delivery_time;
  F32                   mExtra_linger_time;
  //
  bool                  mDo_move_interrupts;
  F32                   mMove_interrupt_speed;
  //
  afxMagicMissileData*  mMissile_db;
  bool                  mLaunch_on_server_signal;
  U32                   mPrimary_target_types;
  //
  afxEffectWrapperData* mDummy_fx_entry;

                        // various effects lists
  afxEffectList         mCasting_fx_list;
  afxEffectList         mLaunch_fx_list;
  afxEffectList         mDelivery_fx_list;
  afxEffectList         mImpact_fx_list;
  afxEffectList         mLinger_fx_list;

  void                  pack_fx(BitStream* stream, const afxEffectList& fx, bool packed);
  void                  unpack_fx(BitStream* stream, afxEffectList& fx);

public:
  /*C*/                 afxMagicSpellData();
  /*C*/                 afxMagicSpellData(const afxMagicSpellData&, bool = false);

  virtual void          reloadReset();

  virtual bool          onAdd();
  virtual void          packData(BitStream*);
  virtual void          unpackData(BitStream*);
  virtual bool          writeField(StringTableEntry fieldname, const char* value);

  bool                  preload(bool server, String &errorStr);

  void                  gatherConstraintDefs(Vector<afxConstraintDef>&); 

  virtual bool          allowSubstitutions() const { return true; }

  static void           initPersistFields();

  DECLARE_CONOBJECT(afxMagicSpellData);
  DECLARE_CATEGORY("AFX");

  /// @name Callbacks
  /// @{
  DECLARE_CALLBACK( void, onDamage, (afxMagicSpell* spell, const char* label, const char* flaver, U32 target_id, F32 amount, U8 n, Point3F pos, F32 ad_amount, F32 radius, F32 impulse) );
  DECLARE_CALLBACK( void, onDeactivate, (afxMagicSpell* spell) );
  DECLARE_CALLBACK( void, onInterrupt, (afxMagicSpell* spell, ShapeBase* caster) );
  DECLARE_CALLBACK( void, onLaunch, (afxMagicSpell* spell, ShapeBase* caster, SceneObject* target, afxMagicMissile* missile) );
  DECLARE_CALLBACK( void, onImpact, (afxMagicSpell* spell, ShapeBase* caster, SceneObject* impacted, Point3F pos, Point3F normal) );
  DECLARE_CALLBACK( bool, onPreactivate, (SimObject* param_holder, ShapeBase* caster, SceneObject* target, SimObject* extra) );
  DECLARE_CALLBACK( void, onActivate, (afxMagicSpell* spell, ShapeBase* caster, SceneObject* target) );
  /// @}
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxMagicSpell

class ShapeBase;
class GameConnection;
class afxEffectVector;
class afxConstraint;
class afxConstraintMgr;
class afxMagicMissile;
class afxChoreographer;
class afxPhrase;

class afxMagicSpell : public afxChoreographer, public afxMagicSpellDefs
{
  typedef afxChoreographer Parent;
  friend class afxMagicMissile;

  enum MaskBits 
  {
    MagicMissileMask      = Parent::NextFreeMask << 0,
    StateEventMask        = Parent::NextFreeMask << 1,
    LaunchEventMask       = Parent::NextFreeMask << 2,
    ImpactEventMask       = Parent::NextFreeMask << 3,
    SyncEventMask         = Parent::NextFreeMask << 4,
    RemapConstraintMask   = Parent::NextFreeMask << 5, // CONSTRAINT REMAPPING
    NextFreeMask          = Parent::NextFreeMask << 6
  };

public:
  enum
  {
    NULL_EVENT,
    ACTIVATE_EVENT,
    LAUNCH_EVENT,
    IMPACT_EVENT,
    SHUTDOWN_EVENT,
    DEACTIVATE_EVENT,
    INTERRUPT_PHASE_EVENT,
    INTERRUPT_SPELL_EVENT
  };

  enum
  {
    INACTIVE_STATE,
    CASTING_STATE,
    DELIVERY_STATE,
    LINGER_STATE,
    CLEANUP_STATE,
    DONE_STATE,
    LATE_STATE
  };

  enum {
    MARK_ACTIVATE           = BIT(0),
    MARK_LAUNCH             = BIT(1),
    MARK_IMPACT             = BIT(2),
    MARK_SHUTDOWN           = BIT(3),
    MARK_DEACTIVATE         = BIT(4),
    MARK_END_CASTING        = BIT(5),
    MARK_END_DELIVERY       = BIT(6),
    MARK_END_LINGER         = BIT(7),
    MARK_INTERRUPT_CASTING  = BIT(8),
    MARK_INTERRUPT_DELIVERY = BIT(9),
    MARK_INTERRUPT_LINGER   = BIT(10),
    MARK_INTERRUPT_CLEANUP  = BIT(11),
    //
    MARK_ENDINGS = MARK_END_CASTING | MARK_END_DELIVERY | MARK_END_LINGER,
    MARK_INTERRUPTS = MARK_INTERRUPT_CASTING | MARK_INTERRUPT_DELIVERY | MARK_INTERRUPT_LINGER | MARK_INTERRUPT_CLEANUP
  };

  class ObjectDeleteEvent : public SimEvent
  {
  public:
    void process(SimObject *obj) { if (obj) obj->deleteObject(); }
  };

private:
  static StringTableEntry  CASTER_CONS;
  static StringTableEntry  TARGET_CONS;
  static StringTableEntry  MISSILE_CONS;
  static StringTableEntry  CAMERA_CONS;
  static StringTableEntry  LISTENER_CONS;
  static StringTableEntry  IMPACT_POINT_CONS;
  static StringTableEntry  IMPACTED_OBJECT_CONS;

private:
  afxMagicSpellData*   mDatablock;
  SimObject*           mExeblock;
  afxMagicMissileData* mMissile_db;

  ShapeBase*    mCaster;
  SceneObject*  mTarget;
  SimObject*    mCaster_field;
  SimObject*    mTarget_field;

  U16           mCaster_scope_id;
  U16           mTarget_scope_id;
  bool          mTarget_is_shape;

  bool          mConstraints_initialized;
  bool          mScoping_initialized;

  U8            mSpell_state;
  F32           mSpell_elapsed;

  afxConstraintID mListener_cons_id;
  afxConstraintID mCaster_cons_id;
  afxConstraintID mTarget_cons_id;
  afxConstraintID mImpacted_cons_id;
  afxConstraintID mCamera_cons_id;
  SceneObject*  mCamera_cons_obj;

  afxPhrase*    mPhrases[NUM_PHRASES];
  F32           mTfactors[NUM_PHRASES];

  bool          mNotify_castbar;
  F32           mOverall_time_factor;

  U16           mMarks_mask;

private:
  void          init();
  bool          state_expired();
  F32           state_elapsed();
  void          init_constraints();
  void          init_scoping();
  void          setup_casting_fx();
  void          setup_launch_fx();
  void          setup_delivery_fx();
  void          setup_impact_fx();
  void          setup_linger_fx();
  bool          cleanup_over();
  bool          is_caster_moving();
  bool          is_caster_client(ShapeBase* caster, GameConnection* conn);
  bool          is_impact_in_water(SceneObject* obj, const Point3F& p);

protected:
  virtual bool  remap_builtin_constraint(SceneObject*, const char* cons_name); // CONSTRAINT REMAPPING
  virtual void  pack_constraint_info(NetConnection* conn, BitStream* stream);
  virtual void  unpack_constraint_info(NetConnection* conn, BitStream* stream);

private:
  afxMagicMissile*  mMissile;
  bool              mMissile_is_armed;
  SceneObject*      mImpacted_obj;
  Point3F           mImpact_pos;
  Point3F           mImpact_norm;
  U16               mImpacted_scope_id;
  bool              mImpacted_is_shape;

  void          init_missile_s(afxMagicMissileData* mm);
  void          launch_missile_s();

  void          init_missile_c(afxMagicMissileData* mm);
  void          launch_missile_c();

public:
  virtual void  impactNotify(const Point3F& p, const Point3F& n, SceneObject*);
  virtual void  executeScriptEvent(const char* method, afxConstraint*, 
                                   const MatrixF& pos, const char* data);
  virtual void  inflictDamage(const char * label, const char* flavor, SimObjectId target,
                              F32 amt, U8 count, F32 ad_amt, F32 rad, Point3F pos, F32 imp);

public:
  /*C*/         afxMagicSpell();
  /*C*/         afxMagicSpell(ShapeBase* caster, SceneObject* target);
  /*D*/         ~afxMagicSpell();

    // STANDARD OVERLOADED METHODS //
  virtual bool  onNewDataBlock(GameBaseData* dptr, bool reload);
  virtual void  processTick(const Move*);
  virtual void  advanceTime(F32 dt);
  virtual bool  onAdd();
  virtual void  onRemove();
  virtual void  onDeleteNotify(SimObject*);
  virtual U32   packUpdate(NetConnection*, U32, BitStream*);
  virtual void  unpackUpdate(NetConnection*, BitStream*);

  virtual void  sync_with_clients();
  void          finish_startup();

  static void   initPersistFields();

  DECLARE_CONOBJECT(afxMagicSpell);
  DECLARE_CATEGORY("AFX");

private:
  void          process_server();
  //
  void          change_state_s(U8 pending_state);
  //
  void          enter_casting_state_s();
  void          leave_casting_state_s();
  void          enter_delivery_state_s();
  void          leave_delivery_state_s();
  void          enter_linger_state_s();
  void          leave_linger_state_s();
  void          enter_done_state_s();

private:
  void          process_client(F32 dt);
  //
  void          change_state_c(U8 pending_state);
  //
  void          enter_casting_state_c(F32 starttime);
  void          leave_casting_state_c();
  void          enter_delivery_state_c(F32 starttime);
  void          leave_delivery_state_c();
  void          enter_linger_state_c(F32 starttime);
  void          leave_linger_state_c();
  //
  void          sync_client(U16 marks, U8 state, F32 state_elapsed, F32 spell_elapsed);

public:
  void          postSpellEvent(U8 event);
  void          resolveTimeFactors();

  void          setTimeFactor(F32 f) { mOverall_time_factor = (f > 0) ? f : 1.0f; }
  F32           getTimeFactor() { return mOverall_time_factor; }
  void          setTimeFactor(U8 phase, F32 f) { mTfactors[phase] = (f > 0) ? f : 1.0f; }
  F32           getTimeFactor(U8 phase) { return mTfactors[phase]; }

  ShapeBase*        getCaster() const { return mCaster; }
  SceneObject*      getTarget() const { return mTarget; }
  afxMagicMissile*  getMissile() const { return mMissile; }
  SceneObject*      getImpactedObject() const { return mImpacted_obj; }

  virtual void      restoreObject(SceneObject*);

  bool              activationCallInit(bool postponed=false);
  void              activate();

public:
  static afxMagicSpell* cast_spell(afxMagicSpellData*, ShapeBase* caster, SceneObject* target, SimObject* extra);
  
  static void     displayScreenMessage(ShapeBase* caster, const char* msg);
  static Point3F  getShapeImpactPos(SceneObject*);
};

inline bool afxMagicSpell::is_caster_moving()
{
  return (mCaster) ? (mCaster->getVelocity().len() > mDatablock->mMove_interrupt_speed) : false;
}

inline bool afxMagicSpell::is_caster_client(ShapeBase* caster, GameConnection* conn)
{
  return (caster) ? (caster->getControllingClient() == conn) : false;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_MAGIC_SPELL_H_
