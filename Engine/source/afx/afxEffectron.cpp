
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
#include "math/mathIO.h"

#include "afx/afxChoreographer.h"
#include "afx/afxEffectron.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectronData::ewValidator
//
// When an effect is added using "addEffect", this validator intercepts the value
// and adds it to the dynamic effects list. 
//
void afxEffectronData::ewValidator::validateType(SimObject* object, void* typePtr)
{
  afxEffectronData* eff_data = dynamic_cast<afxEffectronData*>(object);
  afxEffectBaseData** ew = (afxEffectBaseData**)(typePtr);

  if (eff_data && ew)
  {
    eff_data->fx_list.push_back(*ew);
    *ew = 0;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class EffectronFinishStartupEvent : public SimEvent
{
public:
  void process(SimObject* obj) 
  { 
     afxEffectron* eff = dynamic_cast<afxEffectron*>(obj);
     if (eff)
       eff->finish_startup();
  }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectronData

IMPLEMENT_CO_DATABLOCK_V1(afxEffectronData);

ConsoleDocClass( afxEffectronData,
   "@brief Defines the properties of an afxEffectron.\n\n"

   "@ingroup afxChoreographers\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxEffectronData::afxEffectronData()
{
  duration = 0.0f;
  n_loops = 1;

  // dummy entry holds effect-wrapper pointer while a special validator
  // grabs it and adds it to an appropriate effects list
  dummy_fx_entry = NULL;

  // marked true if datablock ids need to
  // be converted into pointers
  do_id_convert = false;
}

afxEffectronData::afxEffectronData(const afxEffectronData& other, bool temp_clone) : afxChoreographerData(other, temp_clone)
{
  duration = other.duration;
  n_loops = other.n_loops;
  dummy_fx_entry = other.dummy_fx_entry;
  do_id_convert = other.do_id_convert;
  fx_list = other.fx_list;
}

void afxEffectronData::reloadReset()
{
  fx_list.clear();
}

#define myOffset(field) Offset(field, afxEffectronData)

void afxEffectronData::initPersistFields()
{
  addField("duration",    TypeF32,      myOffset(duration),
    "...");
  addField("numLoops",    TypeS32,      myOffset(n_loops),
    "...");
  // effect lists
  // for each of these, dummy_fx_entry is set and then a validator adds it to the appropriate effects list 
  static ewValidator emptyValidator(0);
  
  addFieldV("addEffect",  TYPEID<afxEffectBaseData>(),  myOffset(dummy_fx_entry), &emptyValidator,
    "...");

  Parent::initPersistFields();

  // disallow some field substitutions
  disableFieldSubstitutions("addEffect");
}

bool afxEffectronData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxEffectronData::pack_fx(BitStream* stream, const afxEffectList& fx, bool packed)
{
  stream->writeInt(fx.size(), EFFECTS_PER_PHRASE_BITS);
  for (int i = 0; i < fx.size(); i++)
    writeDatablockID(stream, fx[i], packed);
}

void afxEffectronData::unpack_fx(BitStream* stream, afxEffectList& fx)
{
  fx.clear();
  S32 n_fx = stream->readInt(EFFECTS_PER_PHRASE_BITS);
  for (int i = 0; i < n_fx; i++)
    fx.push_back((afxEffectWrapperData*)readDatablockID(stream));
}

void afxEffectronData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->write(duration);
  stream->write(n_loops);

  pack_fx(stream, fx_list, packed);
}

void afxEffectronData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&duration);
  stream->read(&n_loops);

  do_id_convert = true;
  unpack_fx(stream, fx_list);
}

bool afxEffectronData::preload(bool server, String &errorStr)
{
  if (!Parent::preload(server, errorStr))
    return false;

  // Resolve objects transmitted from server
  if (!server) 
  {
    if (do_id_convert)
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
              "afxEffectronData::preload() -- bad datablockId: 0x%x", 
              db_id);
          }
        }
      }
      do_id_convert = false;
    }
  }

  return true;
}

void afxEffectronData::gatherConstraintDefs(Vector<afxConstraintDef>& defs)
{
  afxConstraintDef::gather_cons_defs(defs, fx_list);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

DefineEngineMethod(afxEffectronData, reset, void, (),,
                   "Resets an effectron datablock during reload.\n\n"
                   "@ingroup AFX")
{
  object->reloadReset();
}

DefineEngineMethod(afxEffectronData, addEffect, void, (afxEffectBaseData* effect),,
                   "Adds an effect (wrapper or group) to an effectron's phase.\n\n"
                   "@ingroup AFX")
{
  if (!effect) 
  {
    Con::errorf("afxEffectronData::addEffect() -- missing afxEffectWrapperData.");
    return;
  }
  
  object->fx_list.push_back(effect);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectron

IMPLEMENT_CO_NETOBJECT_V1(afxEffectron);

ConsoleDocClass( afxEffectron,
   "@brief A basic effects choreographer.\n\n"

   "@ingroup afxChoreographers\n"
   "@ingroup AFX\n"
);

StringTableEntry  afxEffectron::CAMERA_CONS;
StringTableEntry  afxEffectron::LISTENER_CONS;

void afxEffectron::init()
{
  // setup static predefined constraint names
  if (CAMERA_CONS == 0)
  {
    CAMERA_CONS = StringTable->insert("camera");
    LISTENER_CONS = StringTable->insert("listener");
  }

  // afxEffectron is always in scope, however the effects used 
  // do their own scoping in that they will shut off if their 
  // position constraint leaves scope.
  //
  //   note -- ghosting is delayed until constraint 
  //           initialization is done.
  //
  //mNetFlags.set(Ghostable | ScopeAlways);
  mNetFlags.clear(Ghostable | ScopeAlways);

  datablock = NULL;
  exeblock = NULL;

  constraints_initialized = false;
  scoping_initialized = false;

  effect_state = (U8) INACTIVE_STATE;
  effect_elapsed = 0;

  // define named constraints
  constraint_mgr->defineConstraint(CAMERA_CONSTRAINT, CAMERA_CONS);
  constraint_mgr->defineConstraint(POINT_CONSTRAINT,  LISTENER_CONS);

  active_phrase = NULL;
  time_factor = 1.0f;
  camera_cons_obj = 0;

  marks_mask = 0;
}

afxEffectron::afxEffectron()
{
  started_with_newop = true;
  init();
}

afxEffectron::afxEffectron(bool not_default)
{
  started_with_newop = false;
  init();
}

afxEffectron::~afxEffectron()
{
  delete active_phrase;

  if (datablock && datablock->isTempClone())
  {
    delete datablock;
    datablock = 0;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

// STANDARD OVERLOADED METHODS //

bool afxEffectron::onNewDataBlock(GameBaseData* dptr, bool reload)
{
  datablock = dynamic_cast<afxEffectronData*>(dptr);
  if (!datablock || !Parent::onNewDataBlock(dptr, reload))
    return false;

  if (isServerObject() && started_with_newop)
  {
    // copy dynamic fields from the datablock but
    // don't replace fields with a value
    assignDynamicFieldsFrom(dptr, arcaneFX::sParameterFieldPrefix, true);
  }

  exeblock = datablock;

  if (isClientObject())
  {
    // make a temp datablock clone if there are substitutions
    if (datablock->getSubstitutionCount() > 0)
    {
      afxEffectronData* orig_db = datablock;
      datablock = new afxEffectronData(*orig_db, true);
      exeblock = orig_db;
      // Don't perform substitutions yet, the effectrons's dynamic fields haven't
      // arrived yet and the substitutions may refer to them. Hold off and do
      // in in the onAdd() method.
    }  
  }
  else if (started_with_newop)
  {
    // make a temp datablock clone if there are substitutions
    if (datablock->getSubstitutionCount() > 0)
    {
      afxEffectronData* orig_db = datablock;
      datablock = new afxEffectronData(*orig_db, true);
      exeblock = orig_db;
      orig_db->performSubstitutions(datablock, this, ranking);
    }
  }

  return true;
}

void afxEffectron::processTick(const Move* m)
{
	Parent::processTick(m);

  // don't process moves or client ticks
  if (m != 0 || isClientObject())
    return;

  process_server();
}

void afxEffectron::advanceTime(F32 dt)
{
  Parent::advanceTime(dt);

  process_client(dt);
}

bool afxEffectron::onAdd()
{
  if (!Parent::onAdd()) 
    return false;

  if (isClientObject())
  {
    if (datablock->isTempClone())
    {
      afxEffectronData* orig_db = (afxEffectronData*)exeblock;
      orig_db->performSubstitutions(datablock, this, ranking);
    }  
  }
  else if (started_with_newop && !postpone_activation)
  {
    if (!activationCallInit())
      return false;
    activate();
  }

  return true;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

U32 afxEffectron::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
{
  S32 mark_stream_pos = stream->getCurPos();

  U32 retMask = Parent::packUpdate(conn, mask, stream);
  
  // InitialUpdate
  if (stream->writeFlag(mask & InitialUpdateMask))
  {
    // pack extra object's ghost index or scope id if not yet ghosted
    if (stream->writeFlag(dynamic_cast<NetObject*>(extra) != 0))
    {
      NetObject* net_extra = (NetObject*)extra;
      S32 ghost_idx = conn->getGhostIndex(net_extra);
      if (stream->writeFlag(ghost_idx != -1))
         stream->writeRangedU32(U32(ghost_idx), 0, NetConnection::MaxGhostCount);
      else
      {
        if (stream->writeFlag(net_extra->getScopeId() > 0))
        {
          stream->writeInt(net_extra->getScopeId(), NetObject::SCOPE_ID_BITS);
        }
      }
    }

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

  check_packet_usage(conn, stream, mark_stream_pos, "afxEffectron:");
  AssertISV(stream->isValid(), "afxEffectron::packUpdate(): write failure occurred, possibly caused by packet-size overrun."); 
  
  return retMask;
}

//~~~~~~~~~~~~~~~~~~~~//

void afxEffectron::unpackUpdate(NetConnection * conn, BitStream * stream)
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

    // extra sent
    if (stream->readFlag())
    {
      // cleanup?
      if (stream->readFlag()) // is ghost_idx
      {
        S32 ghost_idx = stream->readRangedU32(0, NetConnection::MaxGhostCount);
        extra = dynamic_cast<SimObject*>(conn->resolveGhost(ghost_idx));
      }
      else
      {
        if (stream->readFlag()) // has scope_id
        {
          // JTF NOTE: U16 extra_scope_id = stream->readInt(NetObject::SCOPE_ID_BITS);
          stream->readInt(NetObject::SCOPE_ID_BITS);
        }
      }
    }

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

void afxEffectron::sync_with_clients()
{
  setMaskBits(SyncEventMask);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// private

bool afxEffectron::state_expired()
{
  afxPhrase* phrase = (effect_state == ACTIVE_STATE) ? active_phrase : NULL;

  if (phrase)
  {
    if (phrase->expired(effect_elapsed))
      return (!phrase->recycle(effect_elapsed));
    return false;
  }

  return true;
}

void afxEffectron::init_constraints()
{
  if (constraints_initialized)
  {
    //Con::printf("CONSTRAINTS ALREADY INITIALIZED");
    return;
  }

  Vector<afxConstraintDef> defs;
  datablock->gatherConstraintDefs(defs);

  constraint_mgr->initConstraintDefs(defs, isServerObject());

  if (isServerObject())
  {
    // find local camera
    camera_cons_obj = get_camera();
    if (camera_cons_obj)
      camera_cons_id = constraint_mgr->setReferenceObject(CAMERA_CONS, camera_cons_obj);
  }
  else // if (isClientObject())
  {
    // find local camera
    camera_cons_obj = get_camera();
    if (camera_cons_obj)
      camera_cons_id = constraint_mgr->setReferenceObject(CAMERA_CONS, camera_cons_obj);

    // find local listener
    Point3F listener_pos;
    listener_pos = SFX->getListener().getTransform().getPosition();
    listener_cons_id = constraint_mgr->setReferencePoint(LISTENER_CONS, listener_pos);
  }

  constraint_mgr->adjustProcessOrdering(this);

  constraints_initialized = true;
}

void afxEffectron::init_scoping()
{
  if (scoping_initialized)
  {
    //Con::printf("SCOPING ALREADY INITIALIZED");
    return;
  }

  if (isServerObject())
  {
    if (explicit_clients.size() > 0)
    {
      for (U32 i = 0; i < explicit_clients.size(); i++)
        explicit_clients[i]->objectLocalScopeAlways(this);
    }
    else
    {
      mNetFlags.set(Ghostable);
      setScopeAlways();
    }
    scoping_initialized = true;
  }
}

void afxEffectron::setup_active_fx()
{
  active_phrase = new afxPhrase(isServerObject(), /*willStop=*/true);

  if (active_phrase)
  {
    active_phrase->init(datablock->fx_list, datablock->duration, this, time_factor, datablock->n_loops);
  }
}

bool afxEffectron::cleanup_over()
{
  if (active_phrase && !active_phrase->isEmpty())
    return false;

  return true;
}

void afxEffectron::inflictDamage(const char * label, const char* flavor, SimObjectId target_id,
                                   F32 amount, U8 n, F32 ad_amount, F32 radius, Point3F pos, F32 impulse)
{ 
  char *posArg = Con::getArgBuffer(64);
  dSprintf(posArg, 64, "%f %f %f", pos.x, pos.y, pos.z);
  Con::executef(exeblock, "onDamage", 
                              getIdString(), 
                              label,
                              flavor,
                              Con::getIntArg(target_id),
                              Con::getFloatArg(amount), 
                              Con::getIntArg(n),
                              posArg,
                              Con::getFloatArg(ad_amount), 
                              Con::getFloatArg(radius),
                              Con::getFloatArg(impulse));
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// private

void afxEffectron::process_server()
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

  if (active_phrase)
    active_phrase->update(TickSec, effect_elapsed);
}

void afxEffectron::change_state_s(U8 pending_state)
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

void afxEffectron::enter_done_state_s()
{ 
  postEvent(DEACTIVATE_EVENT);

  F32 done_time = effect_elapsed;

  if (active_phrase)
  {
    F32 phrase_done;
    if (active_phrase->willStop() && active_phrase->isInfinite())
      phrase_done = effect_elapsed + active_phrase->calcAfterLife();
    else
      phrase_done = active_phrase->calcDoneTime();
    if (phrase_done > done_time)
      done_time = phrase_done;
  }

  F32 time_left = done_time - effect_elapsed;
  if (time_left < 0)
    time_left = 0;

  Sim::postEvent(this, new ObjectDeleteEvent, Sim::getCurrentTime() + time_left*1000 + 500);

  // CALL SCRIPT afxEffectronData::onDeactivate(%eff)
  Con::executef(exeblock, "onDeactivate", getIdString());
}

void afxEffectron::enter_active_state_s()
{ 
  // stamp constraint-mgr starting time
  constraint_mgr->setStartTime(Platform::getVirtualMilliseconds());
  effect_elapsed = 0;

  setup_dynamic_constraints();

  // start casting effects
  setup_active_fx();
  if (active_phrase)
    active_phrase->start(effect_elapsed, effect_elapsed);
}

void afxEffectron::leave_active_state_s()
{ 
  if (active_phrase)
    active_phrase->stop(effect_elapsed);
}

void afxEffectron::enter_cleanup_state_s()
{ 
  postEvent(SHUTDOWN_EVENT);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// private

void afxEffectron::process_client(F32 dt)
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
  if (active_phrase)
    active_phrase->update(dt, effect_elapsed);
}

void afxEffectron::change_state_c(U8 pending_state)
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
    break;
  case DONE_STATE:
    break;
  }
}

void afxEffectron::enter_active_state_c(F32 starttime)
{ 
  // stamp constraint-mgr starting time
  constraint_mgr->setStartTime(Platform::getVirtualMilliseconds() - (U32)(effect_elapsed*1000));
  ///effect_elapsed = 0;

  setup_dynamic_constraints();

  setup_active_fx();
  if (active_phrase)
    active_phrase->start(starttime, effect_elapsed);
}

void afxEffectron::leave_active_state_c()
{ 
  if (active_phrase)
    active_phrase->stop(effect_elapsed);
}

void afxEffectron::sync_client(U16 marks, U8 state, F32 elapsed)
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

void afxEffectron::postEvent(U8 event) 
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

void afxEffectron::finish_startup()
{
  init_constraints();
  init_scoping();
  postEvent(afxEffectron::ACTIVATE_EVENT);
}

// static
afxEffectron* 
afxEffectron::start_effect(afxEffectronData* datablock, SimObject* extra) 
{
  AssertFatal(datablock != NULL, "Datablock is missing.");

  afxEffectronData* exeblock = datablock;

  SimObject* param_holder = new SimObject();
  if (!param_holder->registerObject())
  {
    Con::errorf("afxEffectron: failed to register parameter object.");
    delete param_holder;
    return 0;
  }

  param_holder->assignDynamicFieldsFrom(datablock, arcaneFX::sParameterFieldPrefix);
  if (extra)
  {
    // copy dynamic fields from the extra object to the param holder
    param_holder->assignDynamicFieldsFrom(extra, arcaneFX::sParameterFieldPrefix);
  }

  // CALL SCRIPT afxEffectronData::onPreactivate(%params, %extra)
  const char* result = Con::executef(datablock, "onPreactivate", 
                                     Con::getIntArg(param_holder->getId()), 
                                     (extra) ? Con::getIntArg(extra->getId()) : "");
  if (result && result[0] != '\0' && !dAtob(result))
  {
#if defined(TORQUE_DEBUG)
    Con::warnf("afxEffectron: onPreactivate() returned false, effect aborted.");
#endif
    Sim::postEvent(param_holder, new ObjectDeleteEvent, Sim::getCurrentTime());
    return 0;
  }

  // make a temp datablock clone if there are substitutions
  if (datablock->getSubstitutionCount() > 0)
  {
    datablock = new afxEffectronData(*exeblock, true);
    exeblock->performSubstitutions(datablock, param_holder);
  }

  // create a new effectron instance
  afxEffectron* eff = new afxEffectron(true);
  eff->setDataBlock(datablock);
  eff->exeblock = exeblock;
  eff->setExtra(extra);

  // copy dynamic fields from the param holder to the effectron
  eff->assignDynamicFieldsFrom(param_holder, arcaneFX::sParameterFieldPrefix);
  Sim::postEvent(param_holder, new ObjectDeleteEvent, Sim::getCurrentTime());

  // register
  if (!eff->registerObject())
  {
    Con::errorf("afxEffectron: failed to register effectron instance.");
    Sim::postEvent(eff, new ObjectDeleteEvent, Sim::getCurrentTime());
    return 0;
  }
  registerForCleanup(eff);

  eff->activate();

  return eff;
}

bool afxEffectron::activationCallInit(bool postponed) 
{
  if (postponed && (!started_with_newop || !postpone_activation))
  {
    Con::errorf("afxEffectron::activate() -- activate() is only required when creating an effectron with the \"new\" operator "
                "and the postponeActivation field is set to \"true\".");
    return false;
  }

  return true;
}

void afxEffectron::activate() 
{
  // separating the final part of startup allows the calling script
  // to make certain types of calls on the returned effectron that  
  // need to happen prior to constraint initialization.
  Sim::postEvent(this, new EffectronFinishStartupEvent, Sim::getCurrentTime());

  // CALL SCRIPT afxEffectronData::onActivate(%eff)
  Con::executef(exeblock, "onActivate", getIdString());
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// console functions

DefineEngineMethod(afxEffectron, setTimeFactor, void, (float factor),,
                   "Sets the time-factor for the effectron.\n\n"
                   "@ingroup AFX") 
{
  object->setTimeFactor(factor);
}

DefineEngineMethod(afxEffectron, interrupt, void, (),,
                   "Interrupts and deletes a running effectron.\n\n"
                   "@ingroup AFX") 
{
  object->postEvent(afxEffectron::INTERRUPT_EVENT);
}

DefineEngineMethod(afxEffectron, activate, void, (),,
                   "Activates an effectron that was started with postponeActivation=true.\n\n"
                   "@ingroup AFX") 
{
  if (object->activationCallInit(true))
    object->activate();
}

DefineEngineFunction(startEffectron, S32, (afxEffectronData* datablock, const char* constraintSource, const char* constraintName, SimObject* extra),
                     (NULL, NULL, NULL, NULL),
                     "Instantiates the effectron defined by datablock.\n\n"
                     "@ingroup AFX")
{
  if (!datablock)
  {
    Con::errorf("startEffectron() -- missing valid datablock.");
    return 0;
  }

  //
  // Start the Effectron
  //
  afxEffectron* eff = afxEffectron::start_effect(datablock, extra);

  //
  // Create a constraint from arguments (if specified).
  //
  if (eff)
  {
    if (constraintSource && constraintName)
    {
      if (!eff->addConstraint(constraintSource, constraintName))
        Con::errorf("startEffectron() -- failed to find constraint object [%s].", constraintSource);
    }
  }

  //
  // Return the ID (or 0 if start failed).
  //
  return (eff) ? eff->getId() : 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

