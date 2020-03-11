
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

#ifndef _AFX_SELECTION_EFFECT_H_
#define _AFX_SELECTION_EFFECT_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "console/typeValidators.h"

#include "afxChoreographer.h"
#include "afxEffectWrapper.h"
#include "afxPhrase.h"

class afxChoreographerData;
class afxEffectBaseData;

class  afxSelectronDefs
{
public:
  enum {
    MAIN_PHRASE,
    SELECT_PHRASE,
    DESELECT_PHRASE,
    NUM_PHRASES
  };
};

class afxSelectronData : public afxChoreographerData, public afxSelectronDefs
{
  typedef afxChoreographerData Parent;

  class ewValidator : public TypeValidator
  {
    U32 id;
  public:
    ewValidator(U32 id) { this->id = id; }
    void validateType(SimObject *object, void *typePtr);
  };

  bool          do_id_convert;

public:
  F32           main_dur;
  F32           select_dur;
  F32           deselect_dur;

  S32           n_main_loops;
  S32           n_select_loops;
  S32           n_deselect_loops;

  bool          registered;
  U8            obj_type_style;
  U32           obj_type_mask;

  afxEffectBaseData* dummy_fx_entry;

  afxEffectList main_fx_list;
  afxEffectList select_fx_list;
  afxEffectList deselect_fx_list;
  
private:
  void          pack_fx(BitStream* stream, const afxEffectList& fx, bool packed);
  void          unpack_fx(BitStream* stream, afxEffectList& fx);

public:
  /*C*/         afxSelectronData();
  /*C*/         afxSelectronData(const afxSelectronData&, bool = false);
  /*D*/         ~afxSelectronData();

  virtual void  reloadReset();

  virtual bool  onAdd();
  virtual void  packData(BitStream*);
  virtual void  unpackData(BitStream*);

  bool          preload(bool server, String &errorStr);

  bool          matches(U32 mask, U8 style);
  void          gatherConstraintDefs(Vector<afxConstraintDef>&); 

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  DECLARE_CONOBJECT(afxSelectronData);
  DECLARE_CATEGORY("AFX");
};

inline bool afxSelectronData::matches(U32 mask, U8 style)
{
  if (obj_type_style != style)
    return false;

  if (obj_type_mask == 0 && mask == 0)
    return true;

  return ((obj_type_mask & mask) != 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxSelectron

class afxSelectron : public afxChoreographer, public afxSelectronDefs
{
  typedef afxChoreographer Parent;
  friend class arcaneFX;

public:
  enum MaskBits 
  {
    StateEventMask    = Parent::NextFreeMask << 0,
    SyncEventMask     = Parent::NextFreeMask << 1,
    NextFreeMask      = Parent::NextFreeMask << 2
  };

  enum
  {
    NULL_EVENT,
    ACTIVATE_EVENT,
    SHUTDOWN_EVENT,
    DEACTIVATE_EVENT,
    INTERRUPT_EVENT
  };

  enum
  {
    INACTIVE_STATE,
    ACTIVE_STATE,
    CLEANUP_STATE,
    DONE_STATE,
    LATE_STATE
  };

  enum {
    MARK_ACTIVATE   = BIT(0),
    MARK_SHUTDOWN   = BIT(1),
    MARK_DEACTIVATE = BIT(2),
    MARK_INTERRUPT  = BIT(3),
  };

  class ObjectDeleteEvent : public SimEvent
  {
  public:
    void process(SimObject *obj) { if (obj) obj->deleteObject(); }
  };

private:
  static StringTableEntry  CAMERA_CONS;
  static StringTableEntry  LISTENER_CONS;
  static StringTableEntry  FREE_TARGET_CONS;

private:
  afxSelectronData*  datablock;
  SimObject*         exeblock;

  bool          constraints_initialized;
  bool          client_only;

  U8            effect_state;
  F32           effect_elapsed;

  afxConstraintID listener_cons_id;
  afxConstraintID free_target_cons_id;
  afxConstraintID camera_cons_id;
  SceneObject*  camera_cons_obj;

  afxPhrase*    phrases[NUM_PHRASES];

  F32           time_factor;
  U8            marks_mask;

private:
  void          init();
  bool          state_expired();
  void          init_constraints();
  void          setup_main_fx();
  void          setup_select_fx();
  void          setup_deselect_fx();
  bool          cleanup_over();

public:
  /*C*/         afxSelectron();
  /*C*/         afxSelectron(bool not_default);
  /*D*/         ~afxSelectron();

    // STANDARD OVERLOADED METHODS //
  virtual bool  onNewDataBlock(GameBaseData* dptr, bool reload);
  virtual void  processTick(const Move*);
  virtual void  advanceTime(F32 dt);
  virtual bool  onAdd();
  virtual void  onRemove();
  virtual U32   packUpdate(NetConnection*, U32, BitStream*);
  virtual void  unpackUpdate(NetConnection*, BitStream*);

  virtual void  sync_with_clients();
  void          finish_startup();

  DECLARE_CONOBJECT(afxSelectron);
  DECLARE_CATEGORY("AFX");

private:
  void          process_server();
  //
  void          change_state_s(U8 pending_state);
  //
  void          enter_active_state_s();
  void          leave_active_state_s();
  void          enter_cleanup_state_s();
  void          enter_done_state_s();

private:
  void          process_client(F32 dt);
  //
  void          change_state_c(U8 pending_state);
  //
  void          enter_active_state_c(F32 starttime);
  void          enter_cleanup_state_c();
  void          enter_done_state_c();
  void          leave_active_state_c();

  void          sync_client(U16 marks, U8 state, F32 elapsed);

public:
  void          postEvent(U8 event);
  void          setTimeFactor(F32 f) { time_factor = (f > 0) ? f : 1.0f; }
  F32           getTimeFactor() { return time_factor; }

  void          activate();

public:
  static afxSelectron*  start_selectron(SceneObject* picked, U8 subcode, SimObject* extra);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
#endif // _AFX_SELECTION_EFFECT_H_
