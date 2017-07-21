
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

#include "console/engineAPI.h"
#include "T3D/gameBase/gameConnection.h"
#include "sfx/sfxSystem.h"

#include "afx/afxChoreographer.h"
#include "afx/afxSelectron.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxSelectronData::ewValidator
//
// When an effect is added using "addEffect", this validator intercepts the value
// and adds it to the dynamic effects list.
//
void afxSelectronData::ewValidator::validateType(SimObject* object, void* typePtr)
{
  afxSelectronData* sele_data = dynamic_cast<afxSelectronData*>(object);
  afxEffectBaseData** ew = (afxEffectBaseData**)(typePtr);

  if (sele_data && ew)
  {
    switch (id)
    {
    case MAIN_PHRASE:
      sele_data->main_fx_list.push_back(*ew);
      break;
    case SELECT_PHRASE:
      sele_data->select_fx_list.push_back(*ew);
      break;
    case DESELECT_PHRASE:
      sele_data->deselect_fx_list.push_back(*ew);
      break;
    }
    *ew = 0;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class SelectronFinishStartupEvent : public SimEvent
{
public:
  void process(SimObject* obj)
  {
     afxSelectron* selectron = dynamic_cast<afxSelectron*>(obj);
     if (selectron)
       selectron->finish_startup();
  }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxSelectronData

IMPLEMENT_CO_DATABLOCK_V1(afxSelectronData);

ConsoleDocClass( afxSelectronData,
   "@brief Defines the properties of an afxSelectronData.\n\n"

   "@ingroup afxChoreographers\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxSelectronData::afxSelectronData()
{
  main_dur = 0.0f;
  select_dur = 0.0f;
  deselect_dur = 0.0f;

  n_main_loops = 1;
  n_select_loops = 1;
  n_deselect_loops = 1;

  registered = false;

  obj_type_style = 0;
  obj_type_mask = 0;

  // dummy entry holds effect-wrapper pointer while a special validator
  // grabs it and adds it to an appropriate effects list
  dummy_fx_entry = NULL;

  // marked true if datablock ids need to
  // be converted into pointers
  do_id_convert = false;
}

afxSelectronData::afxSelectronData(const afxSelectronData& other, bool temp_clone) : afxChoreographerData(other, temp_clone)
{
  main_dur = other.main_dur;
  select_dur = other.select_dur;
  deselect_dur = other.deselect_dur;
  n_main_loops = other.n_main_loops;
  n_select_loops = other.n_select_loops;
  n_deselect_loops = other.n_deselect_loops;
  registered = false;
  obj_type_style = other.obj_type_style;
  obj_type_mask = other.obj_type_mask;
  dummy_fx_entry = other.dummy_fx_entry;
  do_id_convert = other.do_id_convert;

  main_fx_list = other.main_fx_list;
  select_fx_list = other.select_fx_list;
  deselect_fx_list = other.deselect_fx_list;
}

afxSelectronData::~afxSelectronData()
{
  if (registered && !isTempClone())
    arcaneFX::unregisterSelectronData(this);
}

void afxSelectronData::reloadReset()
{
  main_fx_list.clear();
  select_fx_list.clear();
  deselect_fx_list.clear();
}

#define myOffset(field) Offset(field, afxSelectronData)

void afxSelectronData::initPersistFields()
{
   static ewValidator _mainPhrase(MAIN_PHRASE);
   static ewValidator _selectPhrase(SELECT_PHRASE);
   static ewValidator _deselectPhrase(DESELECT_PHRASE);

  addField("mainDur",                   TypeF32,    myOffset(main_dur),
    "...");
  addField("selectDur",                 TypeF32,    myOffset(select_dur),
    "...");
  addField("deselectDur",               TypeF32,    myOffset(deselect_dur),
    "...");
  addField("mainRepeats",               TypeS32,    myOffset(n_main_loops),
    "...");
  addField("selectRepeats",             TypeS32,    myOffset(n_select_loops),
    "...");
  addField("deselectRepeats",           TypeS32,    myOffset(n_deselect_loops),
    "...");
  addField("selectionTypeMask",         TypeS32,    myOffset(obj_type_mask),
    "...");
  addField("selectionTypeStyle",        TypeS8,     myOffset(obj_type_style),
    "...");

  // effect lists
  // for each of these, dummy_fx_entry is set and then a validator adds it to the appropriate effects list
  addFieldV("addMainEffect",      TYPEID<afxEffectBaseData>(),  myOffset(dummy_fx_entry),  &_mainPhrase,
    "...");
  addFieldV("addSelectEffect",    TYPEID<afxEffectBaseData>(),  myOffset(dummy_fx_entry),  &_selectPhrase,
    "...");
  addFieldV("addDeselectEffect",  TYPEID<afxEffectBaseData>(),  myOffset(dummy_fx_entry),  &_deselectPhrase,
    "...");

  // deprecated
  addField("numMainLoops",      TypeS32,      myOffset(n_main_loops),
    "...");
  addField("numSelectLoops",    TypeS32,      myOffset(n_select_loops),
    "...");
  addField("numDeselectLoops",  TypeS32,      myOffset(n_deselect_loops),
    "...");

  Parent::initPersistFields();

  // disallow some field substitutions
  disableFieldSubstitutions("addMainEffect");
  disableFieldSubstitutions("addSelectEffect");
  disableFieldSubstitutions("addDeselectEffect");
}

bool afxSelectronData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxSelectronData::pack_fx(BitStream* stream, const afxEffectList& fx, bool packed)
{
  stream->writeInt(fx.size(), EFFECTS_PER_PHRASE_BITS);
  for (int i = 0; i < fx.size(); i++)
    writeDatablockID(stream, fx[i], packed);
}

void afxSelectronData::unpack_fx(BitStream* stream, afxEffectList& fx)
{
  fx.clear();
  S32 n_fx = stream->readInt(EFFECTS_PER_PHRASE_BITS);
  for (int i = 0; i < n_fx; i++)
    fx.push_back((afxEffectWrapperData*)readDatablockID(stream));
}

void afxSelectronData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->write(main_dur);
  stream->write(select_dur);
  stream->write(deselect_dur);
  stream->write(n_main_loops);
  stream->write(n_select_loops);
  stream->write(n_deselect_loops);
  stream->write(obj_type_style);
  stream->write(obj_type_mask);

  pack_fx(stream, main_fx_list, packed);
  pack_fx(stream, select_fx_list, packed);
  pack_fx(stream, deselect_fx_list, packed);
}

void afxSelectronData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&main_dur);
  stream->read(&select_dur);
  stream->read(&deselect_dur);
  stream->read(&n_main_loops);
  stream->read(&n_select_loops);
  stream->read(&n_deselect_loops);
  stream->read(&obj_type_style);
  stream->read(&obj_type_mask);

  do_id_convert = true;
  unpack_fx(stream, main_fx_list);
  unpack_fx(stream, select_fx_list);
  unpack_fx(stream, deselect_fx_list);
}

inline void expand_fx_list(afxEffectList& fx_list, const char* tag)
{
  for (S32 i = 0; i < fx_list.size(); i++)
  {
    SimObjectId db_id = SimObjectId((uintptr_t)fx_list[i]);
    if (db_id != 0)
    {
      // try to convert id to pointer
      if (!Sim::findObject(db_id, fx_list[i]))
      {
        Con::errorf(ConsoleLogEntry::General,
          "afxSelectronData::preload() -- bad datablockId: 0x%x (%s)",
          db_id, tag);
      }
    }
  }
}

bool afxSelectronData::preload(bool server, String &errorStr)
{
  if (!Parent::preload(server, errorStr))
    return false;

  // Resolve objects transmitted from server
  if (!server)
  {
    if (do_id_convert)
    {
      expand_fx_list(main_fx_list, "main");
      expand_fx_list(select_fx_list, "select");
      expand_fx_list(deselect_fx_list, "deselect");
      do_id_convert = false;
    }

    // this is where a selectron registers itself with the rest of AFX
    if (!registered)
    {
      arcaneFX::registerSelectronData(this);
      registered = true;
    }
  }

  return true;
}

void afxSelectronData::gatherConstraintDefs(Vector<afxConstraintDef>& defs)
{
  afxConstraintDef::gather_cons_defs(defs, main_fx_list);
  afxConstraintDef::gather_cons_defs(defs, select_fx_list);
  afxConstraintDef::gather_cons_defs(defs, deselect_fx_list);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

DefineEngineMethod(afxSelectronData, reset, void, (),,
                   "Resets a selectron datablock during reload.\n\n"
                   "@ingroup AFX")
{
  object->reloadReset();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxSelectron

IMPLEMENT_CO_NETOBJECT_V1(afxSelectron);

ConsoleDocClass( afxSelectron,
   "@brief A choreographer for selection effects.\n\n"

   "@ingroup afxChoreographers\n"
   "@ingroup AFX\n"
);

StringTableEntry  afxSelectron::CAMERA_CONS;
StringTableEntry  afxSelectron::LISTENER_CONS;
StringTableEntry  afxSelectron::FREE_TARGET_CONS;

void afxSelectron::init()
{
  client_only = true;
  mNetFlags.clear(Ghostable | ScopeAlways);
  mNetFlags.set(IsGhost);

  // setup static predefined constraint names
  if (CAMERA_CONS == 0)
  {
    CAMERA_CONS = StringTable->insert("camera");
    LISTENER_CONS = StringTable->insert("listener");
    FREE_TARGET_CONS = StringTable->insert("freeTarget");
  }

  datablock = NULL;
  exeblock = NULL;

  constraints_initialized = false;

  effect_state = (U8) INACTIVE_STATE;
  effect_elapsed = 0;

  // define named constraints
  constraint_mgr->defineConstraint(CAMERA_CONSTRAINT, CAMERA_CONS);
  constraint_mgr->defineConstraint(POINT_CONSTRAINT,  LISTENER_CONS);
  constraint_mgr->defineConstraint(POINT_CONSTRAINT,  FREE_TARGET_CONS);

  for (S32 i = 0; i < NUM_PHRASES; i++)
    phrases[i] = NULL;

  time_factor = 1.0f;
  camera_cons_obj = 0;

  marks_mask = 0;
}

afxSelectron::afxSelectron()
{
  started_with_newop = true;
  init();
}

afxSelectron::afxSelectron(bool not_default)
{
  started_with_newop = false;
  init();
}

afxSelectron::~afxSelectron()
{
  for (S32 i = 0; i < NUM_PHRASES; i++)
  {
    if (phrases[i])
    {
      phrases[i]->interrupt(effect_elapsed);
      delete phrases[i];
    }
  }

  if (datablock && datablock->isTempClone())
  {
    delete datablock;
    datablock = 0;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

// STANDARD OVERLOADED METHODS //

bool afxSelectron::onNewDataBlock(GameBaseData* dptr, bool reload)
{
  datablock = dynamic_cast<afxSelectronData*>(dptr);
  if (!datablock || !Parent::onNewDataBlock(dptr, reload))
    return false;

  exeblock = datablock;

  return true;
}

void afxSelectron::processTick(const Move* m)
{
	Parent::processTick(m);

  // don't process moves or client ticks
  if (m != 0 || isClientObject())
    return;

  process_server();
}

void afxSelectron::advanceTime(F32 dt)
{
  Parent::advanceTime(dt);

  process_client(dt);
}

bool afxSelectron::onAdd()
{
  if (started_with_newop)
  {
    Con::errorf("afxSelectron::onAdd() -- selectrons cannot be created with the \"new\" operator. Use startSelectron() instead.");
    return false;
  }

  NetConnection* conn = NetConnection::getConnectionToServer();
  if (!conn || !Parent::onAdd())
    return false;

  conn->addObject(this);

  return true;
}

void afxSelectron::onRemove()
{
  getContainer()->removeObject(this);
  Parent::onRemove();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

U32 afxSelectron::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
{
  U32 retMask = Parent::packUpdate(conn, mask, stream);

  // InitialUpdate
  if (stream->writeFlag(mask & InitialUpdateMask))
  {
    stream->write(time_factor);

    GameConnection* gconn = dynamic_cast<GameConnection*>(conn);
    bool zoned_in = (gconn) ? gconn->isZonedIn() : false;
    if (stream->writeFlag(zoned_in))
      pack_constraint_info(conn, stream);
  }

  // StateEvent or SyncEvent
  if (stream->writeFlag((mask & StateEventMask) || (mask & SyncEventMask)))
  {
    stream->write(marks_mask);
    stream->write(effect_state);
    stream->write(effect_elapsed);
  }

  // SyncEvent
  bool do_sync_event = ((mask & SyncEventMask) && !(mask & InitialUpdateMask));
  if (stream->writeFlag(do_sync_event))
  {
    pack_constraint_info(conn, stream);
  }

  return retMask;
}

//~~~~~~~~~~~~~~~~~~~~//

void afxSelectron::unpackUpdate(NetConnection * conn, BitStream * stream)
{
  Parent::unpackUpdate(conn, stream);

  bool initial_update = false;
  bool zoned_in = true;
  bool do_sync_event = false;
  U8 new_marks_mask = 0;
  U8 new_state = INACTIVE_STATE;
  F32 new_elapsed = 0;

  // InitialUpdate Only
  if (stream->readFlag())
  {
    initial_update = true;

    stream->read(&time_factor);

    // if client is marked as fully zoned in
    if ((zoned_in = stream->readFlag()) == true)
    {
      unpack_constraint_info(conn, stream);
      init_constraints();
    }
  }

  // StateEvent or SyncEvent
  // this state data is sent for both state-events and
  // sync-events
  if (stream->readFlag())
  {
    stream->read(&new_marks_mask);
    stream->read(&new_state);
    stream->read(&new_elapsed);

    marks_mask = new_marks_mask;
  }

  // SyncEvent
  do_sync_event = stream->readFlag();
  if (do_sync_event)
  {
    unpack_constraint_info(conn, stream);
    init_constraints();
  }

  //~~~~~~~~~~~~~~~~~~~~//

  if (!zoned_in)
    effect_state = LATE_STATE;

  // need to adjust state info to get all synced up with spell on server
  if (do_sync_event && !initial_update)
    sync_client(new_marks_mask, new_state, new_elapsed);
}

void afxSelectron::sync_with_clients()
{
  setMaskBits(SyncEventMask);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// private

bool afxSelectron::state_expired()
{
  afxPhrase* phrase = (effect_state == ACTIVE_STATE) ? phrases[MAIN_PHRASE] : NULL;

  if (phrase)
  {
    if (phrase->expired(effect_elapsed))
      return (!phrase->recycle(effect_elapsed));
    return false;
  }

  return true;
}

void afxSelectron::init_constraints()
{
  if (constraints_initialized)
  {
    //Con::printf("CONSTRAINTS ALREADY INITIALIZED");
    return;
  }

  Vector<afxConstraintDef> defs;
  datablock->gatherConstraintDefs(defs);

  constraint_mgr->initConstraintDefs(defs, isServerObject());

  if (isClientObject())
  {
    // find local camera
    camera_cons_obj = get_camera();
    if (camera_cons_obj)
      camera_cons_id = constraint_mgr->setReferenceObject(CAMERA_CONS, camera_cons_obj);

    // find local listener
    Point3F listener_pos;
    listener_pos = SFX->getListener().getTransform().getPosition();
    listener_cons_id = constraint_mgr->setReferencePoint(LISTENER_CONS, listener_pos);

    // find free target
    free_target_cons_id = constraint_mgr->setReferencePoint(FREE_TARGET_CONS, arcaneFX::sFreeTargetPos);
  }

  constraint_mgr->adjustProcessOrdering(this);

  constraints_initialized = true;
}

void afxSelectron::setup_main_fx()
{
  phrases[MAIN_PHRASE] = new afxPhrase(isServerObject(), true);

  if (phrases[MAIN_PHRASE])
    phrases[MAIN_PHRASE]->init(datablock->main_fx_list, datablock->main_dur, this, time_factor,
                               datablock->n_main_loops);
}

void afxSelectron::setup_select_fx()
{
  phrases[SELECT_PHRASE] = new afxPhrase(isServerObject(), true);

  if (phrases[SELECT_PHRASE])
    phrases[SELECT_PHRASE]->init(datablock->select_fx_list, -1, this, time_factor, 1);
}

void afxSelectron::setup_deselect_fx()
{
  phrases[DESELECT_PHRASE] = new afxPhrase(isServerObject(), true);

  if (phrases[DESELECT_PHRASE])
    phrases[DESELECT_PHRASE]->init(datablock->deselect_fx_list, -1, this, time_factor, 1);
}

bool afxSelectron::cleanup_over()
{
  for (S32 i = 0; i < NUM_PHRASES; i++)
    if (phrases[i] && !phrases[i]->isEmpty())
      return false;

  return true;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// private

void afxSelectron::process_server()
{
  if (effect_state != INACTIVE_STATE)
    effect_elapsed += TickSec;

  U8 pending_state = effect_state;

  // check for state changes
  switch (effect_state)
  {
  case INACTIVE_STATE:
    if (marks_mask & MARK_ACTIVATE)
      pending_state = ACTIVE_STATE;
    break;
  case ACTIVE_STATE:
    if (marks_mask & MARK_INTERRUPT)
      pending_state = CLEANUP_STATE;
    else if (marks_mask & MARK_SHUTDOWN)
      pending_state = CLEANUP_STATE;
    else if (state_expired())
      pending_state = CLEANUP_STATE;
    break;
  case CLEANUP_STATE:
    if (cleanup_over())
      pending_state = DONE_STATE;
    break;
  }

  if (effect_state != pending_state)
    change_state_s(pending_state);

  if (effect_state == INACTIVE_STATE)
    return;

  //--------------------------//

  // sample the constraints
  constraint_mgr->sample(TickSec, Platform::getVirtualMilliseconds());

  for (S32 i = 0; i < NUM_PHRASES; i++)
    if (phrases[i])
      phrases[i]->update(TickSec, effect_elapsed);
}

void afxSelectron::change_state_s(U8 pending_state)
{
  if (effect_state == pending_state)
    return;

  switch (effect_state)
  {
  case INACTIVE_STATE:
    break;
  case ACTIVE_STATE:
    leave_active_state_s();
    break;
  case CLEANUP_STATE:
    break;
  case DONE_STATE:
    break;
  }

  effect_state = pending_state;

  switch (pending_state)
  {
  case INACTIVE_STATE:
    break;
  case ACTIVE_STATE:
    enter_active_state_s();
    break;
  case CLEANUP_STATE:
    enter_cleanup_state_s();
    break;
  case DONE_STATE:
    enter_done_state_s();
    break;
  }
}

void afxSelectron::enter_done_state_s()
{
  postEvent(DEACTIVATE_EVENT);

  F32 done_time = effect_elapsed;

  for (S32 i = 0; i < NUM_PHRASES; i++)
  {
    if (phrases[i])
    {
      F32 phrase_done;
      if (phrases[i]->willStop() && phrases[i]->isInfinite())
        phrase_done = effect_elapsed + phrases[i]->calcAfterLife();
      else
        phrase_done = phrases[i]->calcDoneTime();
      if (phrase_done > done_time)
        done_time = phrase_done;
    }
  }

  F32 time_left = done_time - effect_elapsed;
  if (time_left < 0)
    time_left = 0;

  Sim::postEvent(this, new ObjectDeleteEvent, Sim::getCurrentTime() + time_left*1000 + 500);

  // CALL SCRIPT afxSelectronData::onDeactivate(%sele)
  Con::executef(datablock, "onDeactivate", getIdString());
}

void afxSelectron::enter_active_state_s()
{
  // stamp constraint-mgr starting time
  constraint_mgr->setStartTime(Platform::getVirtualMilliseconds());
  effect_elapsed = 0;

  setup_dynamic_constraints();

  // start casting effects
  setup_main_fx();
  if (phrases[MAIN_PHRASE])
    phrases[MAIN_PHRASE]->start(effect_elapsed, effect_elapsed);

  setup_select_fx();
  if (phrases[SELECT_PHRASE])
    phrases[SELECT_PHRASE]->start(effect_elapsed, effect_elapsed);
}

void afxSelectron::leave_active_state_s()
{
  if (phrases[MAIN_PHRASE])
    phrases[MAIN_PHRASE]->stop(effect_elapsed);
}

void afxSelectron::enter_cleanup_state_s()
{
  // start deselect effects
  setup_deselect_fx();
  if (phrases[SELECT_PHRASE])
    phrases[SELECT_PHRASE]->interrupt(effect_elapsed);
  if (phrases[DESELECT_PHRASE])
    phrases[DESELECT_PHRASE]->start(effect_elapsed, effect_elapsed);

  postEvent(SHUTDOWN_EVENT);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// private

void afxSelectron::process_client(F32 dt)
{
  effect_elapsed += dt;

  U8 pending_state = effect_state;

  // check for state changes
  switch (effect_state)
  {
  case INACTIVE_STATE:
    if (marks_mask & MARK_ACTIVATE)
      pending_state = ACTIVE_STATE;
    break;
  case ACTIVE_STATE:
    if (marks_mask & MARK_INTERRUPT)
      pending_state = CLEANUP_STATE;
    else if (marks_mask & MARK_SHUTDOWN)
      pending_state = CLEANUP_STATE;
    else if (state_expired())
      pending_state = CLEANUP_STATE;
    break;
  case CLEANUP_STATE:
    if (cleanup_over())
      pending_state = DONE_STATE;
    break;
  }

  if (effect_state != pending_state)
    change_state_c(pending_state);

  if (effect_state == INACTIVE_STATE)
    return;

  //--------------------------//

  // update the listener constraint position
  if (!listener_cons_id.undefined())
  {
    Point3F listener_pos;
    listener_pos = SFX->getListener().getTransform().getPosition();
    constraint_mgr->setReferencePoint(listener_cons_id, listener_pos);
  }

  // update the free target constraint position
  if (!free_target_cons_id.undefined())
  {
    if (!arcaneFX::sFreeTargetPosValid)
      constraint_mgr->invalidateReference(free_target_cons_id);
    else
      constraint_mgr->setReferencePoint(free_target_cons_id, arcaneFX::sFreeTargetPos);
  }

  // find local camera position
  Point3F cam_pos;
  SceneObject* current_cam = get_camera(&cam_pos);

  // detect camera changes
  if (!camera_cons_id.undefined() && current_cam != camera_cons_obj)
  {
    constraint_mgr->setReferenceObject(camera_cons_id, current_cam);
    camera_cons_obj = current_cam;
  }

  // sample the constraints
  constraint_mgr->sample(dt, Platform::getVirtualMilliseconds(), (current_cam) ? &cam_pos : 0);

  // update active effects lists
  for (S32 i = 0; i < NUM_PHRASES; i++)
    if (phrases[i])
      phrases[i]->update(dt, effect_elapsed);

}

void afxSelectron::change_state_c(U8 pending_state)
{
  if (effect_state == pending_state)
    return;

  switch (effect_state)
  {
  case INACTIVE_STATE:
    break;
  case ACTIVE_STATE:
    leave_active_state_c();
    break;
  case CLEANUP_STATE:
    break;
  case DONE_STATE:
    break;
  }

  effect_state = pending_state;

  switch (pending_state)
  {
  case INACTIVE_STATE:
    break;
  case ACTIVE_STATE:
    enter_active_state_c(effect_elapsed);
    break;
  case CLEANUP_STATE:
    enter_cleanup_state_c();
    break;
  case DONE_STATE:
    enter_done_state_c();
    break;
  }
}

void afxSelectron::enter_active_state_c(F32 starttime)
{
  // stamp constraint-mgr starting time
  constraint_mgr->setStartTime(Platform::getVirtualMilliseconds() - (U32)(effect_elapsed*1000));
  ///effect_elapsed = 0;

  setup_dynamic_constraints();

  setup_main_fx();
  if (phrases[MAIN_PHRASE])
    phrases[MAIN_PHRASE]->start(starttime, effect_elapsed);

  setup_select_fx();
  if (phrases[SELECT_PHRASE])
    phrases[SELECT_PHRASE]->start(starttime, effect_elapsed);
}

void afxSelectron::leave_active_state_c()
{
  if (phrases[MAIN_PHRASE])
  {
    //if (marks_mask & MARK_INTERRUPT)
    //  active_phrase->interrupt(effect_elapsed);
    //else
      phrases[MAIN_PHRASE]->stop(effect_elapsed);
  }
}

void afxSelectron::enter_cleanup_state_c()
{
  if (!client_only)
    return;

  // start deselect effects
  setup_deselect_fx();
  if (phrases[DESELECT_PHRASE])
    phrases[DESELECT_PHRASE]->start(effect_elapsed, effect_elapsed);

  postEvent(SHUTDOWN_EVENT);
}

void afxSelectron::enter_done_state_c()
{
  if (!client_only)
    return;

  postEvent(DEACTIVATE_EVENT);

  F32 done_time = effect_elapsed;

  for (S32 i = 0; i < NUM_PHRASES; i++)
  {
    if (phrases[i])
    {
      F32 phrase_done;
      if (phrases[i]->willStop() && phrases[i]->isInfinite())
        phrase_done = effect_elapsed + phrases[i]->calcAfterLife();
      else
        phrase_done = phrases[i]->calcDoneTime();
      if (phrase_done > done_time)
        done_time = phrase_done;
    }
  }

  F32 time_left = done_time - effect_elapsed;
  if (time_left < 0)
    time_left = 0;

  Sim::postEvent(this, new ObjectDeleteEvent, Sim::getCurrentTime() + time_left*1000 + 500);

  // CALL SCRIPT afxSelectronData::onDeactivate(%selectron)
  Con::executef(datablock, "onDeactivate", getIdString());
}

void afxSelectron::sync_client(U16 marks, U8 state, F32 elapsed)
{
  //Con::printf("SYNC marks=%d old_state=%d state=%d elapsed=%g",
  //            marks, effect_state, state, elapsed);

  if (effect_state != LATE_STATE)
    return;

  marks_mask = marks;

  // don't want to be started on late zoning clients
  if (!datablock->exec_on_new_clients)
  {
    effect_state = DONE_STATE;
  }

  // it looks like we're ghosting pretty late and
  // should just return to the inactive state.
  else if (marks & (MARK_INTERRUPT | MARK_DEACTIVATE | MARK_SHUTDOWN))
  {
    effect_state = DONE_STATE;
  }

  // it looks like we should be in the active state.
  else if (marks & MARK_ACTIVATE)
  {
    effect_state = ACTIVE_STATE;
    effect_elapsed = elapsed;
    enter_active_state_c(0.0);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// public:

void afxSelectron::postEvent(U8 event)
{
  setMaskBits(StateEventMask);

  switch (event)
  {
  case ACTIVATE_EVENT:
    marks_mask |= MARK_ACTIVATE;
    break;
  case SHUTDOWN_EVENT:
    marks_mask |= MARK_SHUTDOWN;
    break;
  case DEACTIVATE_EVENT:
    marks_mask |= MARK_DEACTIVATE;
    break;
  case INTERRUPT_EVENT:
    marks_mask |= MARK_INTERRUPT;
    break;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxSelectron::finish_startup()
{
  init_constraints();
  postEvent(afxSelectron::ACTIVATE_EVENT);
}

// static
afxSelectron*
afxSelectron::start_selectron(SceneObject* picked, U8 subcode, SimObject* extra)
{
  U32 picked_type = (picked) ? picked->getTypeMask() : 0;

  afxSelectronData* datablock = arcaneFX::findSelectronData(picked_type, subcode);
  if (!datablock)
  {
    Con::errorf("startSelectron() -- failed to match object-type (%x/%d) to a selection-effect.",
                picked_type, subcode);
    return 0;
  }

  afxSelectronData* exeblock = datablock;

  SimObject* param_holder = new SimObject();
  if (!param_holder->registerObject())
  {
    Con::errorf("afxSelectron: failed to register parameter object.");
    delete param_holder;
    return 0;
  }

  param_holder->assignDynamicFieldsFrom(datablock, arcaneFX::sParameterFieldPrefix);
  if (extra)
  {
    // copy dynamic fields from the extra object to the param holder
    param_holder->assignDynamicFieldsFrom(extra, arcaneFX::sParameterFieldPrefix);
  }

  // CALL SCRIPT afxSelectronData::onPreactivate(%params, %extra)
  const char* result = Con::executef(datablock, "onPreactivate",
                                     Con::getIntArg(param_holder->getId()),
                                     (extra) ? Con::getIntArg(extra->getId()) : "");
  if (result && result[0] != '\0' && !dAtob(result))
  {
#if defined(TORQUE_DEBUG)
    Con::warnf("afxSelectron: onPreactivate() returned false, effect aborted.");
#endif
    Sim::postEvent(param_holder, new ObjectDeleteEvent, Sim::getCurrentTime());
    return 0;
  }

  // make a temp datablock clone if there are substitutions
  if (datablock->getSubstitutionCount() > 0)
  {
    datablock = new afxSelectronData(*exeblock, true);
    exeblock->performSubstitutions(datablock, param_holder);
  }

  // create a new selectron instance
  afxSelectron* selectron = new afxSelectron(true);
  selectron->setDataBlock(datablock);
  selectron->exeblock = exeblock;
  selectron->setExtra(extra);

  // copy dynamic fields from the param holder to the selectron
  selectron->assignDynamicFieldsFrom(param_holder, arcaneFX::sParameterFieldPrefix);
  Sim::postEvent(param_holder, new ObjectDeleteEvent, Sim::getCurrentTime());

  // register
  if (!selectron->registerObject())
  {
    Con::errorf("afxSelectron: failed to register selectron instance.");
    Sim::postEvent(selectron, new ObjectDeleteEvent, Sim::getCurrentTime());
    return 0;
  }

  selectron->activate();

  return selectron;
}

void afxSelectron::activate()
{
  // separating the final part of startup allows the calling script
  // to make certain types of calls on the returned effectron that
  // need to happen prior to constraint initialization.
  Sim::postEvent(this, new SelectronFinishStartupEvent, Sim::getCurrentTime());

  // CALL SCRIPT afxEffectronData::onActivate(%eff)
  Con::executef(exeblock, "onActivate", getIdString());
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// console functions

DefineEngineMethod(afxSelectron, setTimeFactor, void, (float factor), (1.0f),
                   "Sets the time factor of the selectron.\n\n"
                   "@ingroup AFX")
{
  object->setTimeFactor(factor);
}

DefineEngineMethod(afxSelectron, interrupt, void, (),,
                   "Interrupts and deletes a running selectron.\n\n"
                   "@ingroup AFX")
{
  object->postEvent(afxSelectron::INTERRUPT_EVENT);
}

DefineEngineMethod(afxSelectron, stopSelectron, void, (),,
                   "Stops and deletes a running selectron.\n\n"
                   "@ingroup AFX")
{
  object->postEvent(afxSelectron::INTERRUPT_EVENT);
}

DefineEngineFunction(startSelectron, S32, (SceneObject* selectedObj, unsigned int subcode, SimObject* extra),
                     (NULL, 0, NULL),
                     "Instantiates a selectron.\n\n"
                     "@ingroup AFX")
{
  //
  // Start the Selectron
  //
  afxSelectron* selectron = afxSelectron::start_selectron(selectedObj, (U8)subcode, extra);

  //
  // Return the ID (or 0 if start failed).
  //
  return (selectron) ? selectron->getId() : 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//









