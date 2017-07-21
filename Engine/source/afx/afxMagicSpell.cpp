
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
#include "T3D/containerQuery.h"

#include "afx/afxChoreographer.h"
#include "afx/afxPhrase.h"
#include "afx/afxMagicSpell.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxMagicSpellData::ewValidator
//
// When any of the effect list fields (addCastingEffect, etc.) are set, this validator
// intercepts the value and adds it to the appropriate effects list. One validator is
// created for each effect list and an id is used to identify which list to add the effect
// to.
//
void afxMagicSpellData::ewValidator::validateType(SimObject* object, void* typePtr)
{
  afxMagicSpellData* spelldata = dynamic_cast<afxMagicSpellData*>(object);
  afxEffectBaseData** ew = (afxEffectBaseData**)(typePtr);

  if (spelldata && ew)
  {
    switch (id)
    {
    case CASTING_PHRASE:
      spelldata->casting_fx_list.push_back(*ew);
      break;
    case LAUNCH_PHRASE:
      spelldata->launch_fx_list.push_back(*ew);
      break;
    case DELIVERY_PHRASE:
      spelldata->delivery_fx_list.push_back(*ew);
      break;
    case IMPACT_PHRASE:
      spelldata->impact_fx_list.push_back(*ew);
      break;
    case LINGER_PHRASE:
      spelldata->linger_fx_list.push_back(*ew);
      break;
    }
    *ew = 0;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class SpellFinishStartupEvent : public SimEvent
{
public:
  void process(SimObject* obj)
  {
     afxMagicSpell* spell = dynamic_cast<afxMagicSpell*>(obj);
     if (spell)
       spell->finish_startup();
  }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxMagicSpellData

IMPLEMENT_CO_DATABLOCK_V1(afxMagicSpellData);

ConsoleDocClass( afxMagicSpellData,
   "@brief Defines the properties of an afxMagicSpell.\n\n"

   "@ingroup afxChoreographers\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

IMPLEMENT_CALLBACK( afxMagicSpellData, onDamage, void,
   (afxMagicSpell* spell, const char* label, const char* flaver, U32 target_id, F32 amount, U8 n, Point3F pos, F32 ad_amount, F32 radius, F32 impulse),
   (spell, label, flaver, target_id, amount, n, pos, ad_amount, radius, impulse),
   "Called when the spell deals damage.\n"
   "@param spell the spell object\n" );

IMPLEMENT_CALLBACK( afxMagicSpellData, onDeactivate, void, (afxMagicSpell* spell), (spell),
   "Called when the spell ends naturally.\n"
   "@param spell the spell object\n" );

IMPLEMENT_CALLBACK( afxMagicSpellData, onInterrupt, void, (afxMagicSpell* spell, ShapeBase* caster), (spell, caster),
   "Called when the spell ends unnaturally due to an interruption.\n"
   "@param spell the spell object\n" );

IMPLEMENT_CALLBACK( afxMagicSpellData, onLaunch, void,
   (afxMagicSpell* spell, ShapeBase* caster, SceneObject* target, afxMagicMissile* missile),
   (spell, caster, target, missile),
   "Called when the spell's casting stage ends and the delivery stage begins.\n"
   "@param spell the spell object\n" );

IMPLEMENT_CALLBACK( afxMagicSpellData, onImpact, void,
   (afxMagicSpell* spell, ShapeBase* caster, SceneObject* impacted, Point3F pos, Point3F normal),
   (spell, caster, impacted, pos, normal),
   "Called at the spell's missile impact marking the end of the deliver stage and the start of the linger stage.\n"
   "@param spell the spell object\n" );

IMPLEMENT_CALLBACK( afxMagicSpellData, onPreactivate, bool,
   (SimObject* param_holder, ShapeBase* caster, SceneObject* target, SimObject* extra),
   (param_holder, caster, target, extra),
   "Called during spell casting before spell instance is fully created.\n");

IMPLEMENT_CALLBACK( afxMagicSpellData, onActivate, void,
   (afxMagicSpell* spell, ShapeBase* caster, SceneObject* target),
   (spell, caster, target),
   "Called when the spell starts.\n"
   "@param spell the spell object\n" );

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

afxMagicSpellData::afxMagicSpellData()
{
  casting_dur = 0.0f;
  delivery_dur = 0.0f;
  linger_dur = 0.0f;

  n_casting_loops = 1;
  n_delivery_loops = 1;
  n_linger_loops = 1;

  extra_casting_time = 0.0f;
  extra_delivery_time = 0.0f;
  extra_linger_time = 0.0f;

  // interrupt flags
  do_move_interrupts = true;
  move_interrupt_speed = 2.0f;

  // delivers projectile spells
  missile_db = 0;
  launch_on_server_signal = false;
  primary_target_types = PlayerObjectType;

  // dummy entry holds effect-wrapper pointer while a special validator
  // grabs it and adds it to an appropriate effects list
  dummy_fx_entry = NULL;

  // marked true if datablock ids need to
  // be converted into pointers
  do_id_convert = false;
}

afxMagicSpellData::afxMagicSpellData(const afxMagicSpellData& other, bool temp_clone) : afxChoreographerData(other, temp_clone)
{
  casting_dur = other.casting_dur;
  delivery_dur = other.delivery_dur;
  linger_dur = other.linger_dur;
  n_casting_loops = other.n_casting_loops;
  n_delivery_loops = other.n_delivery_loops;
  n_linger_loops = other.n_linger_loops;
  extra_casting_time = other.extra_casting_time;
  extra_delivery_time = other.extra_delivery_time;
  extra_linger_time = other.extra_linger_time;
  do_move_interrupts = other.do_move_interrupts;
  move_interrupt_speed = other.move_interrupt_speed;
  missile_db = other.missile_db;
  launch_on_server_signal = other.launch_on_server_signal;
  primary_target_types = other.primary_target_types;

  dummy_fx_entry = other.dummy_fx_entry;
  do_id_convert = other.do_id_convert;

  casting_fx_list = other.casting_fx_list;
  launch_fx_list = other.launch_fx_list;
  delivery_fx_list = other.delivery_fx_list;
  impact_fx_list = other.impact_fx_list;
  linger_fx_list = other.linger_fx_list;
}

void afxMagicSpellData::reloadReset()
{
  casting_fx_list.clear();
  launch_fx_list.clear();
  delivery_fx_list.clear();
  impact_fx_list.clear();
  linger_fx_list.clear();
}

#define myOffset(field) Offset(field, afxMagicSpellData)

void afxMagicSpellData::initPersistFields()
{
  static ewValidator _castingPhrase(CASTING_PHRASE);
  static ewValidator _launchPhrase(LAUNCH_PHRASE);
  static ewValidator _deliveryPhrase(DELIVERY_PHRASE);
  static ewValidator _impactPhrase(IMPACT_PHRASE);
  static ewValidator _lingerPhrase(LINGER_PHRASE);

  // for each effect list, dummy_fx_entry is set and then a validator adds it to the appropriate effects list

  addGroup("Casting Stage");
  addField("castingDur",            TypeF32,        myOffset(casting_dur),
    "...");
  addField("numCastingLoops",       TypeS32,        myOffset(n_casting_loops),
    "...");
  addField("extraCastingTime",      TypeF32,        myOffset(extra_casting_time),
    "...");
  addFieldV("addCastingEffect", TYPEID<afxEffectBaseData>(), Offset(dummy_fx_entry, afxMagicSpellData), &_castingPhrase,
    "...");
  endGroup("Casting Stage");

  addGroup("Delivery Stage");
  addField("deliveryDur",           TypeF32,        myOffset(delivery_dur),
    "...");
  addField("numDeliveryLoops",      TypeS32,        myOffset(n_delivery_loops),
    "...");
  addField("extraDeliveryTime",     TypeF32,        myOffset(extra_delivery_time),
    "...");
  addFieldV("addLaunchEffect", TYPEID<afxEffectBaseData>(), Offset(dummy_fx_entry, afxMagicSpellData), &_launchPhrase,
    "...");
  addFieldV("addDeliveryEffect", TYPEID<afxEffectBaseData>(), Offset(dummy_fx_entry, afxMagicSpellData), &_deliveryPhrase,
    "...");
  endGroup("Delivery Stage");

  addGroup("Linger Stage");
  addField("lingerDur",             TypeF32,        myOffset(linger_dur),
    "...");
  addField("numLingerLoops",        TypeS32,        myOffset(n_linger_loops),
    "...");
  addField("extraLingerTime",       TypeF32,        myOffset(extra_linger_time),
    "...");
  addFieldV("addImpactEffect", TYPEID<afxEffectBaseData>(), Offset(dummy_fx_entry, afxMagicSpellData), &_impactPhrase,
    "...");
  addFieldV("addLingerEffect", TYPEID<afxEffectBaseData>(), Offset(dummy_fx_entry, afxMagicSpellData), &_lingerPhrase,
    "...");
  endGroup("Linger Stage");

  // interrupt flags
  addField("allowMovementInterrupts", TypeBool,     myOffset(do_move_interrupts),
    "...");
  addField("movementInterruptSpeed",  TypeF32,      myOffset(move_interrupt_speed),
    "...");

  // delivers projectile spells
  addField("missile",               TYPEID<afxMagicMissileData>(), myOffset(missile_db),
    "...");
  addField("launchOnServerSignal",  TypeBool,                   myOffset(launch_on_server_signal),
    "...");
  addField("primaryTargetTypes",    TypeS32,                    myOffset(primary_target_types),
    "...");


  Parent::initPersistFields();

  // disallow some field substitutions
  onlyKeepClearSubstitutions("missile"); // subs resolving to "~~", or "~0" are OK
  disableFieldSubstitutions("addCastingEffect");
  disableFieldSubstitutions("addLaunchEffect");
  disableFieldSubstitutions("addDeliveryEffect");
  disableFieldSubstitutions("addImpactEffect");
  disableFieldSubstitutions("addLingerEffect");
}

bool afxMagicSpellData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  if (missile_db != NULL && delivery_dur == 0.0)
    delivery_dur = -1;

  return true;
}

void afxMagicSpellData::pack_fx(BitStream* stream, const afxEffectList& fx, bool packed)
{
  stream->writeInt(fx.size(), EFFECTS_PER_PHRASE_BITS);
  for (int i = 0; i < fx.size(); i++)
    writeDatablockID(stream, fx[i], packed);
}

void afxMagicSpellData::unpack_fx(BitStream* stream, afxEffectList& fx)
{
  fx.clear();
  S32 n_fx = stream->readInt(EFFECTS_PER_PHRASE_BITS);
  for (int i = 0; i < n_fx; i++)
    fx.push_back((afxEffectWrapperData*)readDatablockID(stream));
}

void afxMagicSpellData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->write(casting_dur);
  stream->write(delivery_dur);
  stream->write(linger_dur);
  //
  stream->write(n_casting_loops);
  stream->write(n_delivery_loops);
  stream->write(n_linger_loops);
  //
  stream->write(extra_casting_time);
  stream->write(extra_delivery_time);
  stream->write(extra_linger_time);

  stream->writeFlag(do_move_interrupts);
  stream->write(move_interrupt_speed);

  writeDatablockID(stream, missile_db, packed);
  stream->write(launch_on_server_signal);
  stream->write(primary_target_types);

  pack_fx(stream, casting_fx_list, packed);
  pack_fx(stream, launch_fx_list, packed);
  pack_fx(stream, delivery_fx_list, packed);
  pack_fx(stream, impact_fx_list, packed);
  pack_fx(stream, linger_fx_list, packed);
}

void afxMagicSpellData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&casting_dur);
  stream->read(&delivery_dur);
  stream->read(&linger_dur);
  //
  stream->read(&n_casting_loops);
  stream->read(&n_delivery_loops);
  stream->read(&n_linger_loops);
  //
  stream->read(&extra_casting_time);
  stream->read(&extra_delivery_time);
  stream->read(&extra_linger_time);

  do_move_interrupts = stream->readFlag();
  stream->read(&move_interrupt_speed);

  missile_db = (afxMagicMissileData*) readDatablockID(stream);
  stream->read(&launch_on_server_signal);
  stream->read(&primary_target_types);

  do_id_convert = true;
  unpack_fx(stream, casting_fx_list);
  unpack_fx(stream, launch_fx_list);
  unpack_fx(stream, delivery_fx_list);
  unpack_fx(stream, impact_fx_list);
  unpack_fx(stream, linger_fx_list);
}

bool afxMagicSpellData::writeField(StringTableEntry fieldname, const char* value)
{
   if (!Parent::writeField(fieldname, value))
      return false;

   // don't write the dynamic array fields
   if( fieldname == StringTable->insert("addCastingEffect") )
      return false;
   if( fieldname == StringTable->insert("addLaunchEffect") )
      return false;
   if( fieldname == StringTable->insert("addDeliveryEffect") )
      return false;
   if( fieldname == StringTable->insert("addImpactEffect") )
      return false;
   if( fieldname == StringTable->insert("addLingerEffect") )
      return false;

   return true;
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
          "afxMagicSpellData::preload() -- bad datablockId: 0x%x (%s)",
          db_id, tag);
      }
    }
  }
}

bool afxMagicSpellData::preload(bool server, String &errorStr)
{
  if (!Parent::preload(server, errorStr))
    return false;

  // Resolve objects transmitted from server
  if (!server)
  {
    if (do_id_convert)
    {
      SimObjectId missile_id = SimObjectId((uintptr_t)missile_db);
      if (missile_id != 0)
      {
        // try to convert id to pointer
        if (!Sim::findObject(missile_id, missile_db))
        {
          Con::errorf(ConsoleLogEntry::General,
            "afxMagicSpellData::preload() -- bad datablockId: 0x%x (missile)",
            missile_id);
        }
      }
      expand_fx_list(casting_fx_list, "casting");
      expand_fx_list(launch_fx_list, "launch");
      expand_fx_list(delivery_fx_list, "delivery");
      expand_fx_list(impact_fx_list, "impact");
      expand_fx_list(linger_fx_list, "linger");
      do_id_convert = false;
    }
  }

  return true;
}

void afxMagicSpellData::gatherConstraintDefs(Vector<afxConstraintDef>& defs)
{
  afxConstraintDef::gather_cons_defs(defs, casting_fx_list);
  afxConstraintDef::gather_cons_defs(defs, launch_fx_list);
  afxConstraintDef::gather_cons_defs(defs, delivery_fx_list);
  afxConstraintDef::gather_cons_defs(defs, impact_fx_list);
  afxConstraintDef::gather_cons_defs(defs, linger_fx_list);

  if (missile_db)
    missile_db->gather_cons_defs(defs);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

DefineEngineMethod(afxMagicSpellData, reset, void, (),,
                   "Resets a spell datablock during reload.\n\n"
                   "@ingroup AFX")
{
  object->reloadReset();
}

DefineEngineMethod(afxMagicSpellData, addCastingEffect, void, (afxEffectBaseData* effect),,
                   "Adds an effect (wrapper or group) to a spell's casting phase.\n\n"
                   "@ingroup AFX")
{
  if (!effect)
  {
    Con::errorf(ConsoleLogEntry::General,
                "afxMagicSpellData::addCastingEffect() -- "
                "missing afxEffectWrapperData.");
    return;
  }

  object->casting_fx_list.push_back(effect);
}

DefineEngineMethod(afxMagicSpellData, addLaunchEffect, void, (afxEffectBaseData* effect),,
                   "Adds an effect (wrapper or group) to a spell's launch phase.\n\n"
                   "@ingroup AFX")

{
  if (!effect)
  {
    Con::errorf(ConsoleLogEntry::General,
                "afxMagicSpellData::addLaunchEffect() -- "
                "failed to find afxEffectWrapperData.");
    return;
  }

  object->launch_fx_list.push_back(effect);
}

DefineEngineMethod(afxMagicSpellData, addDeliveryEffect, void, (afxEffectBaseData* effect),,
                   "Adds an effect (wrapper or group) to a spell's delivery phase.\n\n"
                   "@ingroup AFX")

{
  if (!effect)
  {
    Con::errorf(ConsoleLogEntry::General,
                "afxMagicSpellData::addDeliveryEffect() -- "
                "missing afxEffectWrapperData.");
    return;
  }

  object->delivery_fx_list.push_back(effect);
}

DefineEngineMethod(afxMagicSpellData, addImpactEffect, void, (afxEffectBaseData* effect),,
                   "Adds an effect (wrapper or group) to a spell's impact phase.\n\n"
                   "@ingroup AFX")

{
  if (!effect)
  {
    Con::errorf(ConsoleLogEntry::General,
                "afxMagicSpellData::addImpactEffect() -- "
                "missing afxEffectWrapperData.");
    return;
  }

  object->impact_fx_list.push_back(effect);
}

DefineEngineMethod(afxMagicSpellData, addLingerEffect, void, (afxEffectBaseData* effect),,
                   "Adds an effect (wrapper or group) to a spell's linger phase.\n\n"
                   "@ingroup AFX")

{
  if (!effect)
  {
    Con::errorf(ConsoleLogEntry::General,
                "afxMagicSpellData::addLingerEffect() -- "
                "missing afxEffectWrapperData.");
    return;
  }

  object->linger_fx_list.push_back(effect);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxMagicSpell

IMPLEMENT_GLOBAL_CALLBACK( onCastingStart, void, (), (),
   "A callout called on clients by spells when the casting stage begins.\n"
   "@ingroup AFX\n" );

IMPLEMENT_GLOBAL_CALLBACK( onCastingProgressUpdate, void, (F32 frac), (frac),
   "A callout called periodically on clients by spells to indicate casting progress.\n"
   "@ingroup AFX\n" );

IMPLEMENT_GLOBAL_CALLBACK( onCastingEnd, void, (), (),
   "A callout called on clients by spells when the casting stage ends.\n"
   "@ingroup AFX\n" );

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// CastingPhrase_C
//    Subclass of afxPhrase for the client casting phrase.
//    This subclass adds handling of the casting progress
//    bar in cases where the caster is the client's control
//    object.
//

class CastingPhrase_C : public afxPhrase
{
  typedef afxPhrase Parent;
  ShapeBase*    caster;
  bool          notify_castbar;
  F32           castbar_progress;
public:
  /*C*/         CastingPhrase_C(ShapeBase* caster, bool notify_castbar);
  virtual void  start(F32 startstamp, F32 timestamp);
  virtual void  update(F32 dt, F32 timestamp);
  virtual void  stop(F32 timestamp);
  virtual void  interrupt(F32 timestamp);
};

CastingPhrase_C::CastingPhrase_C(ShapeBase* c, bool notify)
  : afxPhrase(false, true)
{
  caster = c;
  notify_castbar = notify;
  castbar_progress = 0.0f;
}

void CastingPhrase_C::start(F32 startstamp, F32 timestamp)
{
  Parent::start(startstamp, timestamp); //START
  if (notify_castbar)
  {
    castbar_progress = 0.0f;
    onCastingStart_callback();
  }
}

void CastingPhrase_C::update(F32 dt, F32 timestamp)
{
  Parent::update(dt, timestamp);

  if (!notify_castbar)
    return;

  if (dur > 0 && n_loops > 0)
  {
    F32 nfrac = (timestamp - starttime)/(dur*n_loops);
    if (nfrac - castbar_progress > 1.0f/200.0f)
    {
      castbar_progress = (nfrac < 1.0f) ? nfrac : 1.0f;
      onCastingProgressUpdate_callback(castbar_progress);
    }
  }
}

void CastingPhrase_C::stop(F32 timestamp)
{
  Parent::stop(timestamp);
  if (notify_castbar)
  {
    onCastingEnd_callback();
    notify_castbar = false;
  }
}

void CastingPhrase_C::interrupt(F32 timestamp)
{
  Parent::interrupt(timestamp);
  if (notify_castbar)
  {
    onCastingEnd_callback();
    notify_castbar = false;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// some enum to name converters for debugging purposes

#ifdef USE_FOR_DEBUG_MESSAGES
static char* name_from_state(U8 s)
{
  switch (s)
  {
  case afxMagicSpell::INACTIVE_STATE:
    return "inactive";
  case afxMagicSpell::CASTING_STATE:
    return "casting";
  case afxMagicSpell::DELIVERY_STATE:
    return "delivery";
  case afxMagicSpell::LINGER_STATE:
    return "linger";
  case afxMagicSpell::CLEANUP_STATE:
    return "cleanup";
  case afxMagicSpell::DONE_STATE:
    return "done";
  }

  return "unknown";
}

static char* name_from_event(U8 e)
{
  switch (e)
  {
  case afxMagicSpell::NULL_EVENT:
    return "null";
  case afxMagicSpell::ACTIVATE_EVENT:
    return "activate";
  case afxMagicSpell::LAUNCH_EVENT:
    return "launch";
  case afxMagicSpell::IMPACT_EVENT:
    return "impact";
  case afxMagicSpell::SHUTDOWN_EVENT:
    return "shutdown";
  case afxMagicSpell::DEACTIVATE_EVENT:
    return "deactivate";
  case afxMagicSpell::INTERRUPT_PHASE_EVENT:
    return "interrupt_phase";
  case afxMagicSpell::INTERRUPT_SPELL_EVENT:
    return "interrupt_spell";
  }

  return "unknown";
}
#endif

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxMagicSpell

IMPLEMENT_CO_NETOBJECT_V1(afxMagicSpell);

ConsoleDocClass( afxMagicSpell,
   "@brief A magic spell effects choreographer.\n\n"

   "@ingroup afxChoreographers\n"
   "@ingroup AFX\n"
);

// static
StringTableEntry  afxMagicSpell::CASTER_CONS;
StringTableEntry  afxMagicSpell::TARGET_CONS;
StringTableEntry  afxMagicSpell::MISSILE_CONS;
StringTableEntry  afxMagicSpell::CAMERA_CONS;
StringTableEntry  afxMagicSpell::LISTENER_CONS;
StringTableEntry  afxMagicSpell::IMPACT_POINT_CONS;
StringTableEntry  afxMagicSpell::IMPACTED_OBJECT_CONS;

void afxMagicSpell::init()
{
  // setup static predefined constraint names
  if (CASTER_CONS == 0)
  {
    CASTER_CONS = StringTable->insert("caster");
    TARGET_CONS = StringTable->insert("target");
    MISSILE_CONS = StringTable->insert("missile");
    CAMERA_CONS = StringTable->insert("camera");
    LISTENER_CONS = StringTable->insert("listener");
    IMPACT_POINT_CONS = StringTable->insert("impactPoint");
    IMPACTED_OBJECT_CONS = StringTable->insert("impactedObject");
  }

  // afxMagicSpell is always in scope, however effects
  // do their own scoping in that they will shut off if
  // their position constraint leaves scope.
  //
  //   note -- ghosting is delayed until constraint
  //           initialization is done.
  //
  //mNetFlags.set(Ghostable | ScopeAlways);
  mNetFlags.clear(Ghostable | ScopeAlways);

  datablock = NULL;
  exeblock = NULL;
  missile_db = NULL;

  caster = NULL;
  target = NULL;

  caster_field = NULL;
  target_field = NULL;

  caster_scope_id = 0;
  target_scope_id = 0;
  target_is_shape = false;

  constraints_initialized = false;
  scoping_initialized = false;

  spell_state = (U8) INACTIVE_STATE;
  spell_elapsed = 0;

  // define named constraints
  constraint_mgr->defineConstraint(OBJECT_CONSTRAINT,  CASTER_CONS);
  constraint_mgr->defineConstraint(OBJECT_CONSTRAINT,  TARGET_CONS);
  constraint_mgr->defineConstraint(OBJECT_CONSTRAINT, MISSILE_CONS);
  constraint_mgr->defineConstraint(CAMERA_CONSTRAINT, CAMERA_CONS);
  constraint_mgr->defineConstraint(POINT_CONSTRAINT,  LISTENER_CONS);
  constraint_mgr->defineConstraint(POINT_CONSTRAINT,  IMPACT_POINT_CONS);
  constraint_mgr->defineConstraint(OBJECT_CONSTRAINT,  IMPACTED_OBJECT_CONS);

  for (S32 i = 0; i < NUM_PHRASES; i++)
  {
    phrases[i] = NULL;
    tfactors[i] = 1.0f;
  }

  notify_castbar = false;
  overall_time_factor = 1.0f;

  camera_cons_obj = 0;

  marks_mask = 0;

  missile = NULL;
  missile_is_armed = false;
  impacted_obj = NULL;
  impact_pos.zero();
  impact_norm.set(0,0,1);
  impacted_scope_id = 0;
  impacted_is_shape = false;
}

afxMagicSpell::afxMagicSpell()
{
  started_with_newop = true;
  init();
}

afxMagicSpell::afxMagicSpell(ShapeBase* caster, SceneObject* target)
{
  started_with_newop = false;
  init();

  this->caster = caster;
  if (caster)
  {
    caster_field = caster;
    deleteNotify(caster);
    processAfter(caster);
  }

  this->target = target;
  if (target)
  {
    target_field = target;
    deleteNotify(target);
  }
}

afxMagicSpell::~afxMagicSpell()
{
  for (S32 i = 0; i < NUM_PHRASES; i++)
  {
    if (phrases[i])
    {
      phrases[i]->interrupt(spell_elapsed);
      delete phrases[i];
    }
  }

  if (missile)
    missile->deleteObject();

  if (missile_db && missile_db->isTempClone())
  {
    delete missile_db;
    missile_db = 0;
  }

   if (datablock && datablock->isTempClone())
   {
     delete datablock;
     datablock = 0;
   }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

// STANDARD OVERLOADED METHODS //

bool afxMagicSpell::onNewDataBlock(GameBaseData* dptr, bool reload)
{
  datablock = dynamic_cast<afxMagicSpellData*>(dptr);
  if (!datablock || !Parent::onNewDataBlock(dptr, reload))
    return false;

  if (isServerObject() && started_with_newop)
  {
    // copy dynamic fields from the datablock but
    // don't replace fields with a value
    assignDynamicFieldsFrom(dptr, arcaneFX::sParameterFieldPrefix, true);
  }

  exeblock = datablock;
  missile_db = datablock->missile_db;

  if (isClientObject())
  {
    // make a temp datablock clone if there are substitutions
    if (datablock->getSubstitutionCount() > 0)
    {
      afxMagicSpellData* orig_db = datablock;
      datablock = new afxMagicSpellData(*orig_db, true);
      exeblock = orig_db;
      missile_db = datablock->missile_db;
      // Don't perform substitutions yet, the spell's dynamic fields haven't
      // arrived yet and the substitutions may refer to them. Hold off and do
      // in in the onAdd() method.
    }
  }
  else if (started_with_newop)
  {
    // make a temp datablock clone if there are substitutions
    if (datablock->getSubstitutionCount() > 0)
    {
      afxMagicSpellData* orig_db = datablock;
      datablock = new afxMagicSpellData(*orig_db, true);
      exeblock = orig_db;
      orig_db->performSubstitutions(datablock, this, ranking);
      missile_db = datablock->missile_db;
    }
  }

  return true;
}

void afxMagicSpell::processTick(const Move* m)
{
  Parent::processTick(m);

  // don't process moves or client ticks
  if (m != 0 || isClientObject())
    return;

  process_server();
}

void afxMagicSpell::advanceTime(F32 dt)
{
  Parent::advanceTime(dt);

  process_client(dt);
}

bool afxMagicSpell::onAdd()
{
  if (!Parent::onAdd())
    return false ;

  if (isClientObject())
  {
    if (datablock->isTempClone())
    {
      afxMagicSpellData* orig_db = (afxMagicSpellData*)exeblock;
      orig_db->performSubstitutions(datablock, this, ranking);
      missile_db = datablock->missile_db;
      notify_castbar = (notify_castbar && (datablock->casting_dur > 0.0f));
    }
  }
  else if (started_with_newop && !postpone_activation)
  {
    if (!activationCallInit())
      return false;
    activate();
  }

  return true ;
}

void afxMagicSpell::onRemove()
{
  Parent::onRemove();
}

void afxMagicSpell::onDeleteNotify(SimObject* obj)
{
  // caster deleted?
  ShapeBase* shape = dynamic_cast<ShapeBase*>(obj);
  if (shape == caster)
  {
    clearProcessAfter();
    caster = NULL;
    caster_field = NULL;
    caster_scope_id = 0;
  }

  // target deleted?
  SceneObject* scene_obj = dynamic_cast<SceneObject*>(obj);
  if (scene_obj == target)
  {
    target = NULL;
    target_field = NULL;
    target_scope_id = 0;
    target_is_shape = false;
  }

  // impacted_obj deleted?
  if (scene_obj == impacted_obj)
  {
    impacted_obj = NULL;
    impacted_scope_id = 0;
    impacted_is_shape = false;
  }

  // missile deleted?
  afxMagicMissile* missile = dynamic_cast<afxMagicMissile*>(obj);
  if (missile != NULL && missile == this->missile)
  {
    this->missile = NULL;
  }

  // something else
  Parent::onDeleteNotify(obj);
}

// static
void afxMagicSpell::initPersistFields()
{
  addField("caster", TYPEID<SimObject>(), Offset(caster_field, afxMagicSpell),
    "...");
  addField("target", TYPEID<SimObject>(), Offset(target_field, afxMagicSpell),
    "...");

  Parent::initPersistFields();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxMagicSpell::pack_constraint_info(NetConnection* conn, BitStream* stream)
{
  // pack caster's ghost index or scope id if not yet ghosted
  if (stream->writeFlag(caster != NULL))
  {
    S32 ghost_idx = conn->getGhostIndex(caster);
    if (stream->writeFlag(ghost_idx != -1))
      stream->writeRangedU32(U32(ghost_idx), 0, NetConnection::MaxGhostCount);
    else
    {
      bool bit = (caster) ? (caster->getScopeId() > 0) : false;
      if (stream->writeFlag(bit))
        stream->writeInt(caster->getScopeId(), NetObject::SCOPE_ID_BITS);
    }
  }

  // pack target's ghost index or scope id if not yet ghosted
  if (stream->writeFlag(target != NULL))
  {
    S32 ghost_idx = conn->getGhostIndex(target);
    if (stream->writeFlag(ghost_idx != -1))
      stream->writeRangedU32(U32(ghost_idx), 0, NetConnection::MaxGhostCount);
    else
    {
      if (stream->writeFlag(target->getScopeId() > 0))
      {
        stream->writeInt(target->getScopeId(), NetObject::SCOPE_ID_BITS);
        stream->writeFlag(dynamic_cast<ShapeBase*>(target) != NULL); // is shape?
      }
    }
  }

  Parent::pack_constraint_info(conn, stream);
}

void afxMagicSpell::unpack_constraint_info(NetConnection* conn, BitStream* stream)
{
  caster = NULL;
  caster_field = NULL;
  caster_scope_id = 0;
  if (stream->readFlag()) // has caster
  {
    if (stream->readFlag()) // has ghost_idx
    {
      S32 ghost_idx = stream->readRangedU32(0, NetConnection::MaxGhostCount);
      caster = dynamic_cast<ShapeBase*>(conn->resolveGhost(ghost_idx));
      if (caster)
      {
        caster_field = caster;
        deleteNotify(caster);
        processAfter(caster);
      }
    }
    else
    {
      if (stream->readFlag()) // has scope_id (is always a shape)
        caster_scope_id = stream->readInt(NetObject::SCOPE_ID_BITS);
    }
  }

  target = NULL;
  target_field = NULL;
  target_scope_id = 0;
  target_is_shape = false;
  if (stream->readFlag()) // has target
  {
    if (stream->readFlag()) // has ghost_idx
    {
      S32 ghost_idx = stream->readRangedU32(0, NetConnection::MaxGhostCount);
      target = dynamic_cast<SceneObject*>(conn->resolveGhost(ghost_idx));
      if (target)
      {
        target_field = target;
        deleteNotify(target);
      }
    }
    else
    {
      if (stream->readFlag()) // has scope_id
      {
        target_scope_id = stream->readInt(NetObject::SCOPE_ID_BITS);
        target_is_shape = stream->readFlag(); // is shape?
      }
    }
  }

  Parent::unpack_constraint_info(conn, stream);
}

U32 afxMagicSpell::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
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

    // pack initial exec conditions
    stream->write(exec_conds_mask);

    // flag if this client owns the spellcaster
    bool client_owns_caster = is_caster_client(caster, dynamic_cast<GameConnection*>(conn));
    stream->writeFlag(client_owns_caster);

    // pack per-phrase time-factor values
    for (S32 i = 0; i < NUM_PHRASES; i++)
      stream->write(tfactors[i]);

    // flag if this conn is zoned-in yet
    bool zoned_in = client_owns_caster;
    if (!zoned_in)
    {
      GameConnection* gconn = dynamic_cast<GameConnection*>(conn);
      zoned_in = (gconn) ? gconn->isZonedIn() : false;
    }
    if (stream->writeFlag(zoned_in))
      pack_constraint_info(conn, stream);
  }

  // StateEvent or SyncEvent
  if (stream->writeFlag((mask & StateEventMask) || (mask & SyncEventMask)))
  {
    stream->write(marks_mask);
    stream->write(spell_state);
    stream->write(state_elapsed());
    stream->write(spell_elapsed);
  }

  // SyncEvent
  if (stream->writeFlag((mask & SyncEventMask) && !(mask & InitialUpdateMask)))
  {
    pack_constraint_info(conn, stream);
  }

  // LaunchEvent
  if (stream->writeFlag((mask & LaunchEventMask) && (marks_mask & MARK_LAUNCH) && missile))
  {
    F32 vel; Point3F vel_vec;
    missile->getStartingVelocityValues(vel, vel_vec);
    // pack launch vector and velocity
    stream->write(vel);
    mathWrite(*stream, vel_vec);
  }

  // ImpactEvent
  if (stream->writeFlag(((mask & ImpactEventMask) || (mask & SyncEventMask)) && (marks_mask & MARK_IMPACT)))
  {
    // pack impact objects's ghost index or scope id if not yet ghosted
    if (stream->writeFlag(impacted_obj != NULL))
    {
      S32 ghost_idx = conn->getGhostIndex(impacted_obj);
      if (stream->writeFlag(ghost_idx != -1))
        stream->writeRangedU32(U32(ghost_idx), 0, NetConnection::MaxGhostCount);
      else
      {
        if (stream->writeFlag(impacted_obj->getScopeId() > 0))
        {
          stream->writeInt(impacted_obj->getScopeId(), NetObject::SCOPE_ID_BITS);
          stream->writeFlag(dynamic_cast<ShapeBase*>(impacted_obj) != NULL);
        }
      }
    }

    // pack impact position and normal
    mathWrite(*stream, impact_pos);
    mathWrite(*stream, impact_norm);
    stream->write(exec_conds_mask);

    ShapeBase* temp_shape;
    stream->writeFlag(caster != 0 && caster->getDamageState() == ShapeBase::Enabled);
    temp_shape = dynamic_cast<ShapeBase*>(target);
    stream->writeFlag(temp_shape != 0 && temp_shape->getDamageState() == ShapeBase::Enabled);
    temp_shape = dynamic_cast<ShapeBase*>(impacted_obj);
    stream->writeFlag(temp_shape != 0 && temp_shape->getDamageState() == ShapeBase::Enabled);
  }

  check_packet_usage(conn, stream, mark_stream_pos, "afxMagicSpell:");

  AssertISV(stream->isValid(), "afxMagicSpell::packUpdate(): write failure occurred, possibly caused by packet-size overrun.");

  return retMask;
}

//~~~~~~~~~~~~~~~~~~~~//

 // CONSTRAINT REMAPPING <<
bool afxMagicSpell::remap_builtin_constraint(SceneObject* obj, const char* cons_name)
{
  StringTableEntry cons_name_ste = StringTable->insert(cons_name);

  if (cons_name_ste == CASTER_CONS)
    return true;
  if (cons_name_ste == TARGET_CONS)
  {
    if (obj && target && obj != target && !target_cons_id.undefined())
    {
      target = obj;
      constraint_mgr->setReferenceObject(target_cons_id, target);
      if (isServerObject())
      {
        if (target->isScopeable())
          constraint_mgr->addScopeableObject(target);
      }
    }
    return true;
  }
  if (cons_name_ste == MISSILE_CONS)
    return true;
  if (cons_name_ste == CAMERA_CONS)
    return true;
  if (cons_name_ste == LISTENER_CONS)
    return true;
  if (cons_name_ste == IMPACT_POINT_CONS)
    return true;
  if (cons_name_ste == IMPACTED_OBJECT_CONS)
    return true;

  return false;
}
 // CONSTRAINT REMAPPING >>

void afxMagicSpell::unpackUpdate(NetConnection * conn, BitStream * stream)
{
  Parent::unpackUpdate(conn, stream);

  bool initial_update = false;
  bool zoned_in = true;
  bool do_sync_event = false;
  U16 new_marks_mask = 0;
  U8 new_spell_state = INACTIVE_STATE;
  F32 new_state_elapsed = 0;
  F32 new_spell_elapsed = 0;;

  // InitialUpdate
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

    // unpack initial exec conditions
    stream->read(&exec_conds_mask);

    // if this is controlling client for the caster,
    // enable castbar updates
    bool client_owns_caster = stream->readFlag();
    if (client_owns_caster)
      notify_castbar = Con::isFunction("onCastingStart");

    // unpack per-phrase time-factor values
    for (S32 i = 0; i < NUM_PHRASES; i++)
      stream->read(&tfactors[i]);

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
    stream->read(&new_spell_state);
    stream->read(&new_state_elapsed);
    stream->read(&new_spell_elapsed);
    marks_mask = new_marks_mask;
  }

  // SyncEvent
  if ((do_sync_event = stream->readFlag()) == true)
  {
    unpack_constraint_info(conn, stream);
    init_constraints();
  }

  // LaunchEvent
  if (stream->readFlag())
  {
    F32 vel; Point3F vel_vec;
    stream->read(&vel);
    mathRead(*stream, &vel_vec);
    if (missile)
    {
      missile->setStartingVelocity(vel);
      missile->setStartingVelocityVector(vel_vec);
    }
  }

  // ImpactEvent
  if (stream->readFlag())
  {
    if (impacted_obj)
      clearNotify(impacted_obj);
    impacted_obj = NULL;
    impacted_scope_id = 0;
    impacted_is_shape = false;
    if (stream->readFlag()) // is impacted_obj
    {
      if (stream->readFlag()) // is ghost_idx
      {
        S32 ghost_idx = stream->readRangedU32(0, NetConnection::MaxGhostCount);
        impacted_obj = dynamic_cast<SceneObject*>(conn->resolveGhost(ghost_idx));
        if (impacted_obj)
          deleteNotify(impacted_obj);
      }
      else
      {
        if (stream->readFlag()) // has scope_id
        {
          impacted_scope_id = stream->readInt(NetObject::SCOPE_ID_BITS);
          impacted_is_shape = stream->readFlag(); // is shape?
        }
      }
    }

    mathRead(*stream, &impact_pos);
    mathRead(*stream, &impact_norm);
    stream->read(&exec_conds_mask);

    bool caster_alive = stream->readFlag();
    bool target_alive = stream->readFlag();
    bool impacted_alive = stream->readFlag();

    afxConstraint* cons;
    if ((cons = constraint_mgr->getConstraint(caster_cons_id)) != 0)
      cons->setLivingState(caster_alive);
    if ((cons = constraint_mgr->getConstraint(target_cons_id)) != 0)
      cons->setLivingState(target_alive);
    if ((cons = constraint_mgr->getConstraint(impacted_cons_id)) != 0)
      cons->setLivingState(impacted_alive);
  }

  //~~~~~~~~~~~~~~~~~~~~//

  if (!zoned_in)
    spell_state = LATE_STATE;

  // need to adjust state info to get all synced up with spell on server
  if (do_sync_event && !initial_update)
    sync_client(new_marks_mask, new_spell_state, new_state_elapsed, new_spell_elapsed);
}

void afxMagicSpell::sync_with_clients()
{
  setMaskBits(SyncEventMask);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// private

bool afxMagicSpell::state_expired()
{
  afxPhrase* phrase = NULL;

  switch (spell_state)
  {
  case CASTING_STATE:
    phrase = phrases[CASTING_PHRASE];
    break;
  case DELIVERY_STATE:
    phrase = phrases[DELIVERY_PHRASE];
    break;
  case LINGER_STATE:
    phrase = phrases[LINGER_PHRASE];
    break;
  }

  if (phrase)
  {
    if (phrase->expired(spell_elapsed))
      return (!phrase->recycle(spell_elapsed));
    return false;
  }

  return true;
}

F32 afxMagicSpell::state_elapsed()
{
  afxPhrase* phrase = NULL;

  switch (spell_state)
  {
  case CASTING_STATE:
    phrase = phrases[CASTING_PHRASE];
    break;
  case DELIVERY_STATE:
    phrase = phrases[DELIVERY_PHRASE];
    break;
  case LINGER_STATE:
    phrase = phrases[LINGER_PHRASE];
    break;
  }

  return (phrase) ? phrase->elapsed(spell_elapsed) : 0.0f;
}

void afxMagicSpell::init_constraints()
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
    caster_cons_id = constraint_mgr->setReferenceObject(CASTER_CONS, caster);
    target_cons_id = constraint_mgr->setReferenceObject(TARGET_CONS, target);
#if defined(AFX_CAP_SCOPE_TRACKING)
    if (caster && caster->isScopeable())
      constraint_mgr->addScopeableObject(caster);

    if (target && target->isScopeable())
      constraint_mgr->addScopeableObject(target);
#endif

    // find local camera
    camera_cons_obj = get_camera();
    if (camera_cons_obj)
      camera_cons_id = constraint_mgr->setReferenceObject(CAMERA_CONS, camera_cons_obj);
  }
  else // if (isClientObject())
  {
    if (caster)
      caster_cons_id = constraint_mgr->setReferenceObject(CASTER_CONS, caster);
    else if (caster_scope_id > 0)
      caster_cons_id = constraint_mgr->setReferenceObjectByScopeId(CASTER_CONS, caster_scope_id, true);

    if (target)
      target_cons_id = constraint_mgr->setReferenceObject(TARGET_CONS, target);
    else if (target_scope_id > 0)
      target_cons_id = constraint_mgr->setReferenceObjectByScopeId(TARGET_CONS, target_scope_id, target_is_shape);

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

void afxMagicSpell::init_scoping()
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

void afxMagicSpell::setup_casting_fx()
{
  if (isServerObject())
    phrases[CASTING_PHRASE] = new afxPhrase(isServerObject(), true);
  else
    phrases[CASTING_PHRASE] = new CastingPhrase_C(caster, notify_castbar);

  if (phrases[CASTING_PHRASE])
    phrases[CASTING_PHRASE]->init(datablock->casting_fx_list, datablock->casting_dur, this,
                                  tfactors[CASTING_PHRASE], datablock->n_casting_loops, 0,
                                  datablock->extra_casting_time);
}

void afxMagicSpell::setup_launch_fx()
{
  phrases[LAUNCH_PHRASE] = new afxPhrase(isServerObject(), false);
  if (phrases[LAUNCH_PHRASE])
    phrases[LAUNCH_PHRASE]->init(datablock->launch_fx_list, -1, this,
                                 tfactors[LAUNCH_PHRASE], 1);
}

void afxMagicSpell::setup_delivery_fx()
{
  phrases[DELIVERY_PHRASE] = new afxPhrase(isServerObject(), true);
  if (phrases[DELIVERY_PHRASE])
  {
    phrases[DELIVERY_PHRASE]->init(datablock->delivery_fx_list, datablock->delivery_dur, this,
                                   tfactors[DELIVERY_PHRASE], datablock->n_delivery_loops, 0,
                                   datablock->extra_delivery_time);
  }
}

void afxMagicSpell::setup_impact_fx()
{
  phrases[IMPACT_PHRASE] = new afxPhrase(isServerObject(), false);
  if (phrases[IMPACT_PHRASE])
  {
    phrases[IMPACT_PHRASE]->init(datablock->impact_fx_list, -1, this,
                                 tfactors[IMPACT_PHRASE], 1);
  }
}

void afxMagicSpell::setup_linger_fx()
{
  phrases[LINGER_PHRASE] = new afxPhrase(isServerObject(), true);
  if (phrases[LINGER_PHRASE])
    phrases[LINGER_PHRASE]->init(datablock->linger_fx_list, datablock->linger_dur, this,
                                 tfactors[LINGER_PHRASE], datablock->n_linger_loops, 0,
                                 datablock->extra_linger_time);
}

bool afxMagicSpell::cleanup_over()
{
  for (S32 i = 0; i < NUM_PHRASES; i++)
    if (phrases[i] && !phrases[i]->isEmpty())
      return false;

  return true;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// private
//
// MISSILE STUFF
//

void afxMagicSpell::init_missile_s(afxMagicMissileData* mm_db)
{
  if (missile)
    clearNotify(missile);

  // create the missile
  missile = new afxMagicMissile(true, false);
  missile->setSubstitutionData(this, ranking);
  missile->setDataBlock(mm_db);
  missile->setChoreographer(this);
  if (!missile->registerObject())
  {
    Con::errorf("afxMagicSpell: failed to register missile instance.");
    delete missile;
    missile = NULL;
  }

  if (missile)
  {
    deleteNotify(missile);
    registerForCleanup(missile);
  }
}

void afxMagicSpell::launch_missile_s()
{
  if (missile)
  {
    missile->launch();
    constraint_mgr->setReferenceObject(MISSILE_CONS, missile);
  }
}

void afxMagicSpell::init_missile_c(afxMagicMissileData* mm_db)
{
  if (missile)
    clearNotify(missile);

  // create the missile
  missile = new afxMagicMissile(false, true);
  missile->setSubstitutionData(this, ranking);
  missile->setDataBlock(mm_db);
  missile->setChoreographer(this);
  if (!missile->registerObject())
  {
    Con::errorf("afxMagicSpell: failed to register missile instance.");
    delete missile;
    missile = NULL;
  }

  if (missile)
  {
    deleteNotify(missile);
    registerForCleanup(missile);
  }
}

void afxMagicSpell::launch_missile_c()
{
  if (missile)
  {
    missile->launch();
    constraint_mgr->setReferenceObject(MISSILE_CONS, missile);
  }
}

bool afxMagicSpell::is_impact_in_water(SceneObject* obj, const Point3F& p)
{
  // AFX_T3D_BROKEN -- water impact detection is disabled. Look at projectile.
  return false;
}

void afxMagicSpell::impactNotify(const Point3F& p, const Point3F& n, SceneObject* obj)
{
  if (isClientObject())
    return;

  ///impact_time_ms = spell_elapsed_ms;
  if (impacted_obj)
      clearNotify(impacted_obj);
  impacted_obj = obj;
  impact_pos = p;
  impact_norm = n;

  if (impacted_obj != NULL)
  {
    deleteNotify(impacted_obj);
    exec_conds_mask |= IMPACTED_SOMETHING;
    if (impacted_obj == target)
      exec_conds_mask |= IMPACTED_TARGET;
    if (impacted_obj->getTypeMask() & datablock->primary_target_types)
      exec_conds_mask |= IMPACTED_PRIMARY;
  }

  if (is_impact_in_water(obj, p))
    exec_conds_mask |= IMPACT_IN_WATER;

  postSpellEvent(IMPACT_EVENT);

  if (missile)
    clearNotify(missile);
  missile = NULL;
}

void afxMagicSpell::executeScriptEvent(const char* method, afxConstraint* cons,
                                        const MatrixF& xfm, const char* data)
{
  SceneObject* cons_obj = (cons) ? cons->getSceneObject() : NULL;

  char *arg_buf = Con::getArgBuffer(256);
  Point3F pos;
  xfm.getColumn(3,&pos);
  AngAxisF aa(xfm);
  dSprintf(arg_buf,256,"%g %g %g %g %g %g %g",
           pos.x, pos.y, pos.z,
           aa.axis.x, aa.axis.y, aa.axis.z, aa.angle);

  // CALL SCRIPT afxChoreographerData::method(%spell, %caster, %constraint, %transform, %data)
  Con::executef(exeblock, method,
                getIdString(),
                (caster) ? caster->getIdString() : "",
                (cons_obj) ? cons_obj->getIdString() : "",
                arg_buf,
                data);
}

void afxMagicSpell::inflictDamage(const char * label, const char* flavor, SimObjectId target_id,
                                   F32 amount, U8 n, F32 ad_amount, F32 radius, Point3F pos, F32 impulse)
{
 // Con::printf("INFLICT-DAMAGE label=%s flav=%s id=%d amt=%g n=%d rad=%g pos=(%g %g %g) imp=%g",
 //             label, flavor, target_id, amount, n, radius, pos.x, pos.y, pos.z, impulse);

  // CALL SCRIPT afxMagicSpellData::onDamage()
  //    onDamage(%spell, %label, %type, %damaged_obj, %amount, %count, %pos, %ad_amount,
  //             %radius, %impulse)
  datablock->onDamage_callback(this, label, flavor, target_id, amount, n, pos, ad_amount, radius, impulse);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// private

void afxMagicSpell::process_server()
{
  if (spell_state != INACTIVE_STATE)
    spell_elapsed += TickSec;

  U8 pending_state = spell_state;

  // check for state changes
  switch (spell_state)
  {

  case INACTIVE_STATE:
    if (marks_mask & MARK_ACTIVATE)
      pending_state = CASTING_STATE;
    break;

  case CASTING_STATE:
    if (datablock->casting_dur > 0.0f && datablock->do_move_interrupts && is_caster_moving())
    {
      displayScreenMessage(caster, "SPELL INTERRUPTED.");
      postSpellEvent(INTERRUPT_SPELL_EVENT);
    }
    if (marks_mask & MARK_INTERRUPT_CASTING)
      pending_state = CLEANUP_STATE;
    else if (marks_mask & MARK_END_CASTING)
      pending_state = DELIVERY_STATE;
    else if (marks_mask & MARK_LAUNCH)
      pending_state = DELIVERY_STATE;
    else if (state_expired())
      pending_state = DELIVERY_STATE;
    break;

  case DELIVERY_STATE:
    if (marks_mask & MARK_INTERRUPT_DELIVERY)
      pending_state = CLEANUP_STATE;
    else if (marks_mask & MARK_END_DELIVERY)
      pending_state = LINGER_STATE;
    else if (marks_mask & MARK_IMPACT)
      pending_state = LINGER_STATE;
    else if (state_expired())
      pending_state = LINGER_STATE;
    break;

  case LINGER_STATE:
    if (marks_mask & MARK_INTERRUPT_LINGER)
      pending_state = CLEANUP_STATE;
    else if (marks_mask & MARK_END_LINGER)
      pending_state = CLEANUP_STATE;
    else if (marks_mask & MARK_SHUTDOWN)
      pending_state = CLEANUP_STATE;
    else if (state_expired())
      pending_state = CLEANUP_STATE;
    break;

  case CLEANUP_STATE:
    if ((marks_mask & MARK_INTERRUPT_CLEANUP) || cleanup_over())
      pending_state = DONE_STATE;
    break;
  }

  if (spell_state != pending_state)
    change_state_s(pending_state);

  if (spell_state == INACTIVE_STATE)
    return;

  //--------------------------//

  // sample the constraints
  constraint_mgr->sample(TickSec, Platform::getVirtualMilliseconds());

  for (S32 i = 0; i < NUM_PHRASES; i++)
    if (phrases[i])
      phrases[i]->update(TickSec, spell_elapsed);

  if (missile_is_armed)
  {
    launch_missile_s();
    missile_is_armed = false;
  }
}

void afxMagicSpell::change_state_s(U8 pending_state)
{
  if (spell_state == pending_state)
    return;

  // LEAVING THIS STATE
  switch (spell_state)
  {
  case INACTIVE_STATE:
    break;
  case CASTING_STATE:
    leave_casting_state_s();
    break;
  case DELIVERY_STATE:
    leave_delivery_state_s();
    break;
  case LINGER_STATE:
    leave_linger_state_s();
    break;
  case CLEANUP_STATE:
    break;
  case DONE_STATE:
    break;
  }

  spell_state = pending_state;

  // ENTERING THIS STATE
  switch (pending_state)
  {
  case INACTIVE_STATE:
    break;
  case CASTING_STATE:
    enter_casting_state_s();
    break;
  case DELIVERY_STATE:
    enter_delivery_state_s();
    break;
  case LINGER_STATE:
    enter_linger_state_s();
    break;
  case CLEANUP_STATE:
    break;
  case DONE_STATE:
    enter_done_state_s();
    break;
  }
}

void afxMagicSpell::enter_done_state_s()
{
  postSpellEvent(DEACTIVATE_EVENT);

  if (marks_mask & MARK_INTERRUPTS)
  {
    Sim::postEvent(this, new ObjectDeleteEvent, Sim::getCurrentTime() + 500);
  }
  else
  {
    F32 done_time = spell_elapsed;

    for (S32 i = 0; i < NUM_PHRASES; i++)
    {
      if (phrases[i])
      {
        F32 phrase_done;
        if (phrases[i]->willStop() && phrases[i]->isInfinite())
          phrase_done = spell_elapsed + phrases[i]->calcAfterLife();
        else
          phrase_done = phrases[i]->calcDoneTime();
        if (phrase_done > done_time)
          done_time = phrase_done;
      }
    }

    F32 time_left = done_time - spell_elapsed;
    if (time_left < 0)
      time_left = 0;

    Sim::postEvent(this, new ObjectDeleteEvent, Sim::getCurrentTime() + time_left*1000 + 500);
  }

  // CALL SCRIPT afxMagicSpellData::onDeactivate(%spell)
  datablock->onDeactivate_callback(this);
}

void afxMagicSpell::enter_casting_state_s()
{
  // note - onActivate() is called in cast_spell() instead of here to make sure any
  // new time-factor settings resolve before they are sent off to the clients.

  // stamp constraint-mgr starting time and reset spell timer
  constraint_mgr->setStartTime(Platform::getVirtualMilliseconds());
  spell_elapsed = 0;

  setup_dynamic_constraints();

  // start casting effects
  setup_casting_fx();
  if (phrases[CASTING_PHRASE])
    phrases[CASTING_PHRASE]->start(spell_elapsed, spell_elapsed);

  // initialize missile
  if (missile_db)
  {
    missile_db = missile_db->cloneAndPerformSubstitutions(this, ranking);
    init_missile_s(missile_db);
  }
}

void afxMagicSpell::leave_casting_state_s()
{
  if (phrases[CASTING_PHRASE])
  {
    if (marks_mask & MARK_INTERRUPT_CASTING)
    {
      //Con::printf("INTERRUPT CASTING (S)");
      phrases[CASTING_PHRASE]->interrupt(spell_elapsed);
    }
    else
    {
      //Con::printf("LEAVING CASTING (S)");
      phrases[CASTING_PHRASE]->stop(spell_elapsed);
    }
  }

  if (marks_mask & MARK_INTERRUPT_CASTING)
  {
    // CALL SCRIPT afxMagicSpellData::onInterrupt(%spell, %caster)
    datablock->onInterrupt_callback(this, caster);
  }
}

void afxMagicSpell::enter_delivery_state_s()
{
  // CALL SCRIPT afxMagicSpellData::onLaunch(%spell, %caster, %target, %missile)
  datablock->onLaunch_callback(this, caster, target, missile);

  if (datablock->launch_on_server_signal)
    postSpellEvent(LAUNCH_EVENT);

  missile_is_armed = true;

  // start launch effects
  setup_launch_fx();
  if (phrases[LAUNCH_PHRASE])
    phrases[LAUNCH_PHRASE]->start(spell_elapsed, spell_elapsed); //START

  // start delivery effects
  setup_delivery_fx();
  if (phrases[DELIVERY_PHRASE])
    phrases[DELIVERY_PHRASE]->start(spell_elapsed, spell_elapsed); //START
}

void afxMagicSpell::leave_delivery_state_s()
{
  if (phrases[DELIVERY_PHRASE])
  {
    if (marks_mask & MARK_INTERRUPT_DELIVERY)
    {
      //Con::printf("INTERRUPT DELIVERY (S)");
      phrases[DELIVERY_PHRASE]->interrupt(spell_elapsed);
    }
    else
    {
      //Con::printf("LEAVING DELIVERY (S)");
      phrases[DELIVERY_PHRASE]->stop(spell_elapsed);
    }
  }

  if (!missile && !(marks_mask & MARK_IMPACT))
  {
    if (target)
    {
      Point3F p = afxMagicSpell::getShapeImpactPos(target);
      Point3F n = Point3F(0,0,1);
      impactNotify(p, n, target);
    }
    else
    {
      Point3F p = Point3F(0,0,0);
      Point3F n = Point3F(0,0,1);
      impactNotify(p, n, 0);
    }
  }
}

void afxMagicSpell::enter_linger_state_s()
{
  if (impacted_obj)
  {
    impacted_cons_id = constraint_mgr->setReferenceObject(IMPACTED_OBJECT_CONS, impacted_obj);
#if defined(AFX_CAP_SCOPE_TRACKING)
    if (impacted_obj->isScopeable())
      constraint_mgr->addScopeableObject(impacted_obj);
#endif
  }
  else
    constraint_mgr->setReferencePoint(IMPACTED_OBJECT_CONS, impact_pos, impact_norm);
  constraint_mgr->setReferencePoint(IMPACT_POINT_CONS, impact_pos, impact_norm);
  constraint_mgr->setReferenceObject(MISSILE_CONS, 0);

  // start impact effects
  setup_impact_fx();
  if (phrases[IMPACT_PHRASE])
    phrases[IMPACT_PHRASE]->start(spell_elapsed, spell_elapsed); //START

  // start linger effects
  setup_linger_fx();
  if (phrases[LINGER_PHRASE])
    phrases[LINGER_PHRASE]->start(spell_elapsed, spell_elapsed); //START

#if 0 // code temporarily replaced with old callback technique in order to avoid engine bug.
  // CALL SCRIPT afxMagicSpellData::onImpact(%spell, %caster, %impactedObj, %impactedPos, %impactedNorm)
  datablock->onImpact_callback(this, caster, impacted_obj, impact_pos, impact_norm);
#else
  char pos_buf[128];
  dSprintf(pos_buf, sizeof(pos_buf), "%g %g %g", impact_pos.x, impact_pos.y, impact_pos.z);
  char norm_buf[128];
  dSprintf(norm_buf, sizeof(norm_buf), "%g %g %g", impact_norm.x, impact_norm.y, impact_norm.z);
  Con::executef(exeblock, "onImpact", getIdString(),
     (caster) ? caster->getIdString(): "",
     (impacted_obj) ? impacted_obj->getIdString(): "",
     pos_buf, norm_buf);
#endif
}

void afxMagicSpell::leave_linger_state_s()
{
  if (phrases[LINGER_PHRASE])
  {
    if (marks_mask & MARK_INTERRUPT_LINGER)
    {
      //Con::printf("INTERRUPT LINGER (S)");
      phrases[LINGER_PHRASE]->interrupt(spell_elapsed);
    }
    else
    {
      //Con::printf("LEAVING LINGER (S)");
      phrases[LINGER_PHRASE]->stop(spell_elapsed);
    }
  }
}



//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// private

void afxMagicSpell::process_client(F32 dt)
{
  spell_elapsed += dt; //SPELL_ELAPSED

  U8 pending_state = spell_state;

  // check for state changes
  switch (spell_state)
  {
  case INACTIVE_STATE:
    if (marks_mask & MARK_ACTIVATE)
      pending_state = CASTING_STATE;
    break;
  case CASTING_STATE:
    if (marks_mask & MARK_INTERRUPT_CASTING)
      pending_state = CLEANUP_STATE;
    else if (marks_mask & MARK_END_CASTING)
      pending_state = DELIVERY_STATE;
    else if (datablock->launch_on_server_signal)
    {
      if (marks_mask & MARK_LAUNCH)
        pending_state = DELIVERY_STATE;
    }
    else
    {
      if (state_expired())
        pending_state = DELIVERY_STATE;
    }
    break;
  case DELIVERY_STATE:
    if (marks_mask & MARK_INTERRUPT_DELIVERY)
      pending_state = CLEANUP_STATE;
    else if (marks_mask & MARK_END_DELIVERY)
      pending_state = LINGER_STATE;
    else if (marks_mask & MARK_IMPACT)
      pending_state = LINGER_STATE;
    else
      state_expired();
    break;
  case LINGER_STATE:
    if (marks_mask & MARK_INTERRUPT_LINGER)
      pending_state = CLEANUP_STATE;
    else if (marks_mask & MARK_END_LINGER)
      pending_state = CLEANUP_STATE;
    else if (marks_mask & MARK_SHUTDOWN)
      pending_state = CLEANUP_STATE;
    else if (state_expired())
      pending_state = CLEANUP_STATE;
    break;
  case CLEANUP_STATE:
    if ((marks_mask & MARK_INTERRUPT_CLEANUP) || cleanup_over())
      pending_state = DONE_STATE;
    break;
  }

  if (spell_state != pending_state)
    change_state_c(pending_state);

  if (spell_state == INACTIVE_STATE)
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
  for (S32 i = 0; i < NUM_PHRASES; i++)
    if (phrases[i])
      phrases[i]->update(dt, spell_elapsed);

  if (missile_is_armed)
  {
    launch_missile_c();
    missile_is_armed = false;
  }
}

void afxMagicSpell::change_state_c(U8 pending_state)
{
  if (spell_state == pending_state)
    return;

  // LEAVING THIS STATE
  switch (spell_state)
  {
  case INACTIVE_STATE:
    break;
  case CASTING_STATE:
    leave_casting_state_c();
    break;
  case DELIVERY_STATE:
    leave_delivery_state_c();
    break;
  case LINGER_STATE:
    leave_linger_state_c();
    break;
  case CLEANUP_STATE:
    break;
  case DONE_STATE:
    break;
  }

  spell_state = pending_state;

  // ENTERING THIS STATE
  switch (pending_state)
  {
  case INACTIVE_STATE:
    break;
  case CASTING_STATE:
    enter_casting_state_c(spell_elapsed);
    break;
  case DELIVERY_STATE:
    enter_delivery_state_c(spell_elapsed);
    break;
  case LINGER_STATE:
    enter_linger_state_c(spell_elapsed);
    break;
  case CLEANUP_STATE:
    break;
  case DONE_STATE:
    break;
  }
}

void afxMagicSpell::enter_casting_state_c(F32 starttime)
{
  // stamp constraint-mgr starting time
  constraint_mgr->setStartTime(Platform::getVirtualMilliseconds() - (U32)(spell_elapsed*1000));
  //spell_elapsed = 0; //SPELL_ELAPSED

  setup_dynamic_constraints();

  // start casting effects and castbar
  setup_casting_fx();
  if (phrases[CASTING_PHRASE])
    phrases[CASTING_PHRASE]->start(starttime, spell_elapsed); //START

  // initialize missile
  if (missile_db)
  {
    missile_db = missile_db->cloneAndPerformSubstitutions(this, ranking);
    init_missile_c(missile_db);
  }
}

void afxMagicSpell::leave_casting_state_c()
{
  if (phrases[CASTING_PHRASE])
  {
    if (marks_mask & MARK_INTERRUPT_CASTING)
    {
      //Con::printf("INTERRUPT CASTING (C)");
      phrases[CASTING_PHRASE]->interrupt(spell_elapsed);
    }
    else
    {
      //Con::printf("LEAVING CASTING (C)");
      phrases[CASTING_PHRASE]->stop(spell_elapsed);
    }
  }
}

void afxMagicSpell::enter_delivery_state_c(F32 starttime)
{
  missile_is_armed = true;

  setup_launch_fx();
  if (phrases[LAUNCH_PHRASE])
    phrases[LAUNCH_PHRASE]->start(starttime, spell_elapsed); //START

  setup_delivery_fx();
  if (phrases[DELIVERY_PHRASE])
    phrases[DELIVERY_PHRASE]->start(starttime, spell_elapsed); //START
}

void afxMagicSpell::leave_delivery_state_c()
{
  if (missile)
  {
    clearNotify(missile);
    missile->deleteObject();
    missile = NULL;
  }

  if (phrases[DELIVERY_PHRASE])
  {
    if (marks_mask & MARK_INTERRUPT_DELIVERY)
    {
      //Con::printf("INTERRUPT DELIVERY (C)");
      phrases[DELIVERY_PHRASE]->interrupt(spell_elapsed);
    }
    else
    {
      //Con::printf("LEAVING DELIVERY (C)");
      phrases[DELIVERY_PHRASE]->stop(spell_elapsed);
    }
  }
}

void afxMagicSpell::enter_linger_state_c(F32 starttime)
{
  if (impacted_obj)
    impacted_cons_id = constraint_mgr->setReferenceObject(IMPACTED_OBJECT_CONS, impacted_obj);
  else if (impacted_scope_id > 0)
    impacted_cons_id = constraint_mgr->setReferenceObjectByScopeId(IMPACTED_OBJECT_CONS, impacted_scope_id, impacted_is_shape);
  else
    constraint_mgr->setReferencePoint(IMPACTED_OBJECT_CONS, impact_pos, impact_norm);
  constraint_mgr->setReferencePoint(IMPACT_POINT_CONS, impact_pos, impact_norm);
  constraint_mgr->setReferenceObject(MISSILE_CONS, 0);

  setup_impact_fx();
  if (phrases[IMPACT_PHRASE])
    phrases[IMPACT_PHRASE]->start(starttime, spell_elapsed); //START

  setup_linger_fx();
  if (phrases[LINGER_PHRASE])
  {
    phrases[LINGER_PHRASE]->start(starttime, spell_elapsed); //START
  }
}

void afxMagicSpell::leave_linger_state_c()
{
  if (phrases[LINGER_PHRASE])
  {
    if (marks_mask & MARK_INTERRUPT_LINGER)
    {
      //Con::printf("INTERRUPT LINGER (C)");
      phrases[LINGER_PHRASE]->interrupt(spell_elapsed);
    }
    else
    {
      //Con::printf("LEAVING LINGER (C)");
      phrases[LINGER_PHRASE]->stop(spell_elapsed);
    }
  }
}

void afxMagicSpell::sync_client(U16 marks, U8 state, F32 elapsed, F32 spell_elapsed)
{
  //Con::printf("SYNC marks=%d old_state=%s state=%s elapsed=%g spell_elapsed=%g",
  //            marks, name_from_state(spell_state), name_from_state(state), elapsed,
  //            spell_elapsed);

  if (spell_state != LATE_STATE)
    return;

  marks_mask = marks;

  // don't want to be started on late zoning clients
  if (!datablock->exec_on_new_clients)
  {
    spell_state = DONE_STATE;
  }

  // it looks like we're ghosting pretty late and
  // should just return to the inactive state.
  else if ((marks & (MARK_INTERRUPTS | MARK_DEACTIVATE | MARK_SHUTDOWN)) ||
           (((marks & MARK_IMPACT) || (marks & MARK_END_DELIVERY)) && (marks & MARK_END_LINGER)))
  {
    spell_state = DONE_STATE;
  }

  // it looks like we should be in the linger state.
  else if ((marks & MARK_IMPACT) ||
           (((marks & MARK_LAUNCH) || (marks & MARK_END_CASTING)) && (marks & MARK_END_DELIVERY)))
  {
    spell_state = LINGER_STATE;
    this->spell_elapsed = spell_elapsed;
    enter_linger_state_c(spell_elapsed-elapsed);
  }

  // it looks like we should be in the delivery state.
  else if ((marks & MARK_LAUNCH) || (marks & MARK_END_CASTING))
  {
    spell_state = DELIVERY_STATE;
    this->spell_elapsed = spell_elapsed;
    enter_delivery_state_c(spell_elapsed-elapsed);
  }

  // it looks like we should be in the casting state.
  else if (marks & MARK_ACTIVATE)
  {
    spell_state = CASTING_STATE; //SPELL_STATE
    this->spell_elapsed = spell_elapsed;
    enter_casting_state_c(spell_elapsed-elapsed);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// public:

void afxMagicSpell::postSpellEvent(U8 event)
{
  setMaskBits(StateEventMask);

  switch (event)
  {
  case ACTIVATE_EVENT:
    marks_mask |= MARK_ACTIVATE;
    break;
  case LAUNCH_EVENT:
    marks_mask |= MARK_LAUNCH;
    setMaskBits(LaunchEventMask);
    break;
  case IMPACT_EVENT:
    marks_mask |= MARK_IMPACT;
    setMaskBits(ImpactEventMask);
    break;
  case SHUTDOWN_EVENT:
    marks_mask |= MARK_SHUTDOWN;
    break;
  case DEACTIVATE_EVENT:
    marks_mask |= MARK_DEACTIVATE;
    break;
  case INTERRUPT_PHASE_EVENT:
    if (spell_state == CASTING_STATE)
      marks_mask |= MARK_END_CASTING;
    else if (spell_state == DELIVERY_STATE)
      marks_mask |= MARK_END_DELIVERY;
    else if (spell_state == LINGER_STATE)
      marks_mask |= MARK_END_LINGER;
    break;
  case INTERRUPT_SPELL_EVENT:
    if (spell_state == CASTING_STATE)
      marks_mask |= MARK_INTERRUPT_CASTING;
    else if (spell_state == DELIVERY_STATE)
      marks_mask |= MARK_INTERRUPT_DELIVERY;
    else if (spell_state == LINGER_STATE)
      marks_mask |= MARK_INTERRUPT_LINGER;
    else if (spell_state == CLEANUP_STATE)
      marks_mask |= MARK_INTERRUPT_CLEANUP;
    break;
  }
}

void afxMagicSpell::resolveTimeFactors()
{
  for (S32 i = 0; i < NUM_PHRASES; i++)
    tfactors[i] *= overall_time_factor;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxMagicSpell::finish_startup()
{
#if !defined(BROKEN_POINT_IN_WATER)
  // test if caster is in water
  if (caster)
  {
    Point3F pos = caster->getPosition();
    if (caster->pointInWater(pos))
      exec_conds_mask |= CASTER_IN_WATER;
  }
#endif

  resolveTimeFactors();

  init_constraints();
  init_scoping();

  postSpellEvent(afxMagicSpell::ACTIVATE_EVENT);
}

// static
afxMagicSpell*
afxMagicSpell::cast_spell(afxMagicSpellData* datablock, ShapeBase* caster, SceneObject* target, SimObject* extra)
{
  AssertFatal(datablock != NULL, "Datablock is missing.");
  AssertFatal(caster != NULL, "Caster is missing.");

  afxMagicSpellData* exeblock = datablock;

  SimObject* param_holder = new SimObject();
  if (!param_holder->registerObject())
  {
    Con::errorf("afxMagicSpell: failed to register parameter object.");
    delete param_holder;
    return 0;
  }

  param_holder->assignDynamicFieldsFrom(datablock, arcaneFX::sParameterFieldPrefix);
  if (extra)
  {
    // copy dynamic fields from the extra object to the param holder
    param_holder->assignDynamicFieldsFrom(extra, arcaneFX::sParameterFieldPrefix);
  }

  if (datablock->isMethod("onPreactivate"))
  {
     // CALL SCRIPT afxMagicSpellData::onPreactivate(%params, %caster, %target, %extra)
     bool result = datablock->onPreactivate_callback(param_holder, caster, target, extra);
     if (!result)
     {
   #if defined(TORQUE_DEBUG)
       Con::warnf("afxMagicSpell: onPreactivate() returned false, spell aborted.");
   #endif
       Sim::postEvent(param_holder, new ObjectDeleteEvent, Sim::getCurrentTime());
       return 0;
     }
  }

  // make a temp datablock clone if there are substitutions
  if (datablock->getSubstitutionCount() > 0)
  {
    datablock = new afxMagicSpellData(*exeblock, true);
    exeblock->performSubstitutions(datablock, param_holder);
  }

  // create a new spell instance
  afxMagicSpell* spell = new afxMagicSpell(caster, target);
  spell->setDataBlock(datablock);
  spell->exeblock = exeblock;
  spell->setExtra(extra);

  // copy dynamic fields from the param holder to the spell
  spell->assignDynamicFieldsFrom(param_holder, arcaneFX::sParameterFieldPrefix);
  Sim::postEvent(param_holder, new ObjectDeleteEvent, Sim::getCurrentTime());

  // register
  if (!spell->registerObject())
  {
    Con::errorf("afxMagicSpell: failed to register spell instance.");
    Sim::postEvent(spell, new ObjectDeleteEvent, Sim::getCurrentTime());
    return 0;
  }
  registerForCleanup(spell);

  spell->activate();

  return spell;
}

IMPLEMENT_GLOBAL_CALLBACK( DisplayScreenMessage, void, (GameConnection* client, const char* message), (client, message),
   "Called to display a screen message.\n"
   "@ingroup AFX\n" );

void afxMagicSpell::displayScreenMessage(ShapeBase* caster, const char* msg)
{
  if (!caster)
    return;

  GameConnection* client = caster->getControllingClient();
  if (client)
    DisplayScreenMessage_callback(client, msg);
}

Point3F afxMagicSpell::getShapeImpactPos(SceneObject* obj)
{
  Point3F pos = obj->getRenderPosition();
  if (obj->getTypeMask() & CorpseObjectType)
    pos.z += 0.5f;
  else
    pos.z += (obj->getObjBox().len_z()/2);
  return pos;
}

void afxMagicSpell::restoreObject(SceneObject* obj)
{
  if (obj->getScopeId() == caster_scope_id && dynamic_cast<ShapeBase*>(obj) != NULL)
  {
    caster_scope_id = 0;
    caster = (ShapeBase*)obj;
    caster_field = caster;
    deleteNotify(caster);
    processAfter(caster);
  }

  if (obj->getScopeId() == target_scope_id)
  {
    target_scope_id = 0;
    target = obj;
    target_field = target;
    deleteNotify(target);
  }

  if (obj->getScopeId() == impacted_scope_id)
  {
    impacted_scope_id = 0;
    impacted_obj = obj;
    deleteNotify(impacted_obj);
  }
}

bool afxMagicSpell::activationCallInit(bool postponed)
{
  if (postponed && (!started_with_newop || !postpone_activation))
  {
    Con::errorf("afxMagicSpell::activate() -- activate() is only required when creating a spell with the \"new\" operator "
                "and the postponeActivation field is set to \"true\".");
    return false;
  }

  if (!caster_field)
  {
    Con::errorf("afxMagicSpell::activate() -- no spellcaster specified.");
    return false;
  }

  caster = dynamic_cast<ShapeBase*>(caster_field);
  if (!caster)
  {
    Con::errorf("afxMagicSpell::activate() -- spellcaster is not a ShapeBase derived object.");
    return false;
  }

  if (target_field)
  {
    target = dynamic_cast<SceneObject*>(target_field);
    if (!target)
      Con::warnf("afxMagicSpell::activate() -- target is not a SceneObject derived object.");
  }

  return true;
}

void afxMagicSpell::activate()
{
  // separating the final part of startup allows the calling script
  // to make certain types of calls on the returned spell that need
  // to happen prior to object registration.
  Sim::postEvent(this, new SpellFinishStartupEvent, Sim::getCurrentTime());

  caster_field = caster;
  target_field = target;

  // CALL SCRIPT afxMagicSpellData::onActivate(%spell, %caster, %target)
  datablock->onActivate_callback(this, caster, target);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// console methods/functions

DefineEngineMethod(afxMagicSpell, getCaster, S32, (),,
                   "Returns ID of the spell's caster object.\n\n"
                   "@ingroup AFX")
{
  ShapeBase* caster = object->getCaster();
  return (caster) ? caster->getId() : -1;
}

DefineEngineMethod(afxMagicSpell, getTarget, S32, (),,
                   "Returns ID of the spell's target object.\n\n"
                   "@ingroup AFX")
{
  SceneObject* target = object->getTarget();
  return (target) ? target->getId() : -1;
}

DefineEngineMethod(afxMagicSpell, getMissile, S32, (),,
                   "Returns ID of the spell's magic-missile object.\n\n"
                   "@ingroup AFX")
{
  afxMagicMissile* missile = object->getMissile();
  return (missile) ? missile->getId() : -1;
}

DefineEngineMethod(afxMagicSpell, getImpactedObject, S32, (),,
                   "Returns ID of impacted-object for the spell.\n\n"
                   "@ingroup AFX")
{
  SceneObject* imp_obj = object->getImpactedObject();
  return (imp_obj) ? imp_obj->getId() : -1;
}

ConsoleMethod(afxMagicSpell, setTimeFactor, void, 3, 4, "(F32 factor) or (string phase, F32 factor)"
              "Sets the time-factor for the spell, either overall or for a specific phrase.\n\n"
              "@ingroup AFX")
{
  if (argc == 3)
    object->setTimeFactor(dAtof(argv[2]));
  else
  {
    if (dStricmp(argv[2], "overall") == 0)
      object->setTimeFactor(dAtof(argv[3]));
    else if (dStricmp(argv[2], "casting") == 0)
      object->setTimeFactor(afxMagicSpell::CASTING_PHRASE, dAtof(argv[3]));
    else if (dStricmp(argv[2], "launch") == 0)
      object->setTimeFactor(afxMagicSpell::LAUNCH_PHRASE, dAtof(argv[3]));
    else if (dStricmp(argv[2], "delivery") == 0)
      object->setTimeFactor(afxMagicSpell::DELIVERY_PHRASE, dAtof(argv[3]));
    else if (dStricmp(argv[2], "impact") == 0)
      object->setTimeFactor(afxMagicSpell::IMPACT_PHRASE, dAtof(argv[3]));
    else if (dStricmp(argv[2], "linger") == 0)
      object->setTimeFactor(afxMagicSpell::LINGER_PHRASE, dAtof(argv[3]));
    else
      Con::errorf("afxMagicSpell::setTimeFactor() -- unknown spell phrase [%s].", argv[2].getStringValue());
  }
}

DefineEngineMethod(afxMagicSpell, interruptStage, void, (),,
                   "Interrupts the current stage of a magic spell causing it to move onto the next one.\n\n"
                   "@ingroup AFX")
{
  object->postSpellEvent(afxMagicSpell::INTERRUPT_PHASE_EVENT);
}

DefineEngineMethod(afxMagicSpell, interrupt, void, (),,
                   "Interrupts and deletes a running magic spell.\n\n"
                   "@ingroup AFX")
{
  object->postSpellEvent(afxMagicSpell::INTERRUPT_SPELL_EVENT);
}

DefineEngineMethod(afxMagicSpell, activate, void, (),,
                   "Activates a magic spell that was started with postponeActivation=true.\n\n"
                   "@ingroup AFX")
{
  if (object->activationCallInit(true))
    object->activate();
}

DefineEngineFunction(castSpell, S32, (afxMagicSpellData* datablock, ShapeBase* caster, SceneObject* target, SimObject* extra),
                     (NULL, NULL),
                     "Instantiates the magic spell defined by datablock and cast by caster.\n\n"
                     "@ingroup AFX")
{
  if (!datablock)
  {
    Con::errorf("castSpell() -- missing valid spell datablock.");
    return 0;
  }

  if (!caster)
  {
    Con::errorf("castSpell() -- missing valid spellcaster.");
    return 0;
  }

  // target is optional (depends on spell)

  // note -- we must examine all arguments prior to calling cast_spell because
  // it calls Con::executef() which will overwrite the argument array.
  afxMagicSpell* spell = afxMagicSpell::cast_spell(datablock, caster, target, extra);

  return (spell) ? spell->getId() : 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
