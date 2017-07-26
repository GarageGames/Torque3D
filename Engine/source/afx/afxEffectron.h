
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

#ifndef _AFX_COMPOSITE_EFFECT_H_
#define _AFX_COMPOSITE_EFFECT_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "console/typeValidators.h"

#include "afxChoreographer.h"
#include "afxEffectWrapper.h"
#include "afxPhrase.h"

class afxChoreographerData;
class afxEffectWrapperData;

class afxEffectronData : public afxChoreographerData
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
  F32           duration;
  S32           n_loops;

  afxEffectBaseData* dummy_fx_entry;

  afxEffectList fx_list;
  
private:
  void          pack_fx(BitStream* stream, const afxEffectList& fx, bool packed);
  void          unpack_fx(BitStream* stream, afxEffectList& fx);

public:
  /*C*/         afxEffectronData();
  /*C*/         afxEffectronData(const afxEffectronData&, bool = false);

  virtual void  reloadReset();

  virtual bool  onAdd();
  virtual void  packData(BitStream*);
  virtual void  unpackData(BitStream*);

  bool          preload(bool server, String &errorStr);

  void          gatherConstraintDefs(Vector<afxConstraintDef>&); 

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  DECLARE_CONOBJECT(afxEffectronData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectron

class afxEffectron : public afxChoreographer
{
  typedef afxChoreographer Parent;

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

private:
  afxEffectronData*  datablock;
  SimObject*         exeblock;

  bool          constraints_initialized;
  bool          scoping_initialized;

  U8            effect_state;
  F32           effect_elapsed;
  U8            marks_mask;
  afxConstraintID listener_cons_id;
  afxConstraintID camera_cons_id;
  SceneObject*  camera_cons_obj;
  afxPhrase*    active_phrase;
  F32           time_factor;

private:
  void          init();
  bool          state_expired();
  void          init_constraints();
  void          init_scoping();
  void          setup_active_fx();
  bool          cleanup_over();

public:
  /*C*/         afxEffectron();
  /*C*/         afxEffectron(bool not_default);
  /*D*/         ~afxEffectron();

    // STANDARD OVERLOADED METHODS //
  virtual bool  onNewDataBlock(GameBaseData* dptr, bool reload);
  virtual void  processTick(const Move*);
  virtual void  advanceTime(F32 dt);
  virtual bool  onAdd();
  virtual U32   packUpdate(NetConnection*, U32, BitStream*);
  virtual void  unpackUpdate(NetConnection*, BitStream*);

  virtual void  inflictDamage(const char * label, const char* flavor, SimObjectId target,
                              F32 amt, U8 count, F32 ad_amt, F32 rad, Point3F pos, F32 imp);
  virtual void  sync_with_clients();
  void          finish_startup();

  DECLARE_CONOBJECT(afxEffectron);
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
  void          leave_active_state_c();

  void          sync_client(U16 marks, U8 state, F32 elapsed);

public:
  void          postEvent(U8 event);
  void          setTimeFactor(F32 f) { time_factor = (f > 0) ? f : 1.0f; }
  F32           getTimeFactor() { return time_factor; }

  bool          activationCallInit(bool postponed=false);
  void          activate();

public:
  static afxEffectron*  start_effect(afxEffectronData*, SimObject* extra);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
#endif // _AFX_EFFECTRON_H_
