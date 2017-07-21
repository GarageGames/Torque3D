
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

#include "afx/afxEffectWrapper.h"
#include "afx/ce/afxPhraseEffect.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxPhraseEffectData::ewValidator
//
// When an effect is added using "addEffect", this validator intercepts the value
// and adds it to the dynamic effects list. 
//
void afxPhraseEffectData::ewValidator::validateType(SimObject* object, void* typePtr)
{
  afxPhraseEffectData* eff_data = dynamic_cast<afxPhraseEffectData*>(object);
  afxEffectBaseData** ew = (afxEffectBaseData**)(typePtr);

  if (eff_data && ew)
  {
    eff_data->fx_list.push_back(*ew);
    *ew = 0;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxPhraseEffectData

IMPLEMENT_CO_DATABLOCK_V1(afxPhraseEffectData);

ConsoleDocClass( afxPhraseEffectData,
   "@brief A datablock that specifies a Phrase Effect, a grouping of other effects.\n\n"

   "A Phrase Effect is a grouping or phrase of effects that do nothing until certain trigger events occur. It's like having a whole "
   "Effectron organized as an individual effect."
   "\n\n"

   "Phrase effects can respond to a number of different kinds of triggers:\n"
   "  -- Player triggers such as footsteps, jumps, landings, and idle triggers.\n"
   "  -- Arbitrary animation triggers on dts-based scene objects.\n"
   "  -- Arbitrary trigger bits assigned to active choreographer objects."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxPhraseEffectData::afxPhraseEffectData()
{
  duration = 0.0f;
  n_loops = 1;

  // dummy entry holds effect-wrapper pointer while a special validator
  // grabs it and adds it to an appropriate effects list
  dummy_fx_entry = NULL;

  // marked true if datablock ids need to
  // be converted into pointers
  do_id_convert = false;

  trigger_mask = 0;
  match_type = MATCH_ANY;
  match_state = STATE_ON;
  phrase_type = PHRASE_TRIGGERED;

  no_choreographer_trigs = false;
  no_cons_trigs = false;
  no_player_trigs = false;

  on_trig_cmd = ST_NULLSTRING;
}

afxPhraseEffectData::afxPhraseEffectData(const afxPhraseEffectData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  duration = other.duration;
  n_loops = other.n_loops;
  dummy_fx_entry = other.dummy_fx_entry;
  do_id_convert = other.do_id_convert; // --
  trigger_mask = other.trigger_mask;
  match_type = other.match_type;
  match_state = other.match_state;
  phrase_type = other.phrase_type;
  no_choreographer_trigs = other.no_choreographer_trigs;
  no_cons_trigs = other.no_cons_trigs;
  no_player_trigs = other.no_player_trigs;
  on_trig_cmd = other.on_trig_cmd;

  // fx_list; // -- ??
}

void afxPhraseEffectData::reloadReset()
{
  fx_list.clear();
}

ImplementEnumType( afxPhraseEffect_MatchType, "Possible phrase effect match types.\n" "@ingroup afxPhraseEffect\n\n" )
   { afxPhraseEffectData::MATCH_ANY,   "any",      "..." },
   { afxPhraseEffectData::MATCH_ALL,   "all",      "..." },
EndImplementEnumType;

ImplementEnumType( afxPhraseEffect_StateType, "Possible phrase effect state types.\n" "@ingroup afxPhraseEffect\n\n" )
   { afxPhraseEffectData::STATE_ON,           "on",      "..." },
   { afxPhraseEffectData::STATE_OFF,          "off",     "..." },
   { afxPhraseEffectData::STATE_ON_AND_OFF,   "both",    "..." },
EndImplementEnumType;

ImplementEnumType( afxPhraseEffect_PhraseType, "Possible phrase effect types.\n" "@ingroup afxPhraseEffect\n\n" )
   { afxPhraseEffectData::PHRASE_TRIGGERED,   "triggered",     "..." },
   { afxPhraseEffectData::PHRASE_CONTINUOUS,  "continuous",    "..." },
EndImplementEnumType;

#define myOffset(field) Offset(field, afxPhraseEffectData)

void afxPhraseEffectData::initPersistFields()
{
  addField("duration",    TypeF32,      myOffset(duration),
    "Specifies a duration for the phrase-effect. If set to infinity, the phrase-effect "
    "needs to have a phraseType of “continuous. Set infinite duration using "
    "$AFX::INFINITE_TIME.");
  addField("numLoops",    TypeS32,      myOffset(n_loops),
    "Specifies the number of times the phrase-effect should loop. If set to infinity, "
    "the phrase-effect needs to have a phraseType of “continuous”. Set infinite looping "
    "using $AFX::INFINITE_REPEATS.");
  addField("triggerMask", TypeS32,      myOffset(trigger_mask),
    "Sets which bits to consider in the current trigger-state which consists of 32 "
    "trigger-bits combined from (possibly overlapping) player trigger bits, constraint "
    "trigger bits, and choreographer trigger bits.");

  addField("matchType", TYPEID<afxPhraseEffectData::MatchType>(), myOffset(match_type),
    "Selects what combination of bits in triggerMask lead to a trigger. When set to "
    "'any', any bit in triggerMask matching the current trigger-state will cause a "
    "trigger. If set to 'all', every bit in triggerMask must match the trigger-state. "
    "Possible values: any or all.");
  addField("matchState", TYPEID<afxPhraseEffectData::StateType>(), myOffset(match_state),
    "Selects which bit-state(s) of bits in the triggerMask to consider when comparing to "
    "the current trigger-state. Possible values: on, off, or both.");
  addField("phraseType", TYPEID<afxPhraseEffectData::PhraseType>(), myOffset(phrase_type),
    "Selects between triggered and continuous types of phrases. When set to 'triggered', "
    "the phrase-effect is triggered when the relevant trigger-bits change state. When set "
    "to 'continuous', the phrase-effect will stay active as long as the trigger-bits "
    "remain in a matching state. Possible values: triggered or continuous.");

  addField("ignoreChoreographerTriggers",   TypeBool,  myOffset(no_choreographer_trigs),
    "When true, trigger-bits on the choreographer will be ignored.");
  addField("ignoreConstraintTriggers",      TypeBool,  myOffset(no_cons_trigs),
    "When true, animation triggers from dts-based constraint source objects will be "
    "ignored.");
  addField("ignorePlayerTriggers",          TypeBool,  myOffset(no_player_trigs),
    "When true, Player-specific triggers from Player-derived constraint source objects "
    "will be ignored.");

  addField("onTriggerCommand",    TypeString,   myOffset(on_trig_cmd),
    "Like a field substitution statement without the leading '$$' token, this eval "
    "statement will be executed when a trigger occurs. Any '%%' and '##'  tokens will be "
    "substituted.");

  // effect lists
  // for each of these, dummy_fx_entry is set and then a validator adds it to the appropriate effects list 
  static ewValidator emptyValidator(0);  
  addFieldV("addEffect",  TYPEID< afxEffectBaseData >(),  myOffset(dummy_fx_entry), &emptyValidator,
    "A field macro which adds an effect wrapper datablock to a list of effects associated "
    "with the phrase-effect's single phrase. Unlike other fields, addEffect follows an "
    "unusual syntax. Order is important since the effects will resolve in the order they "
    "are added to each list.");

  Parent::initPersistFields();

  // disallow some field substitutions
  disableFieldSubstitutions("addEffect");
}

bool afxPhraseEffectData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxPhraseEffectData::pack_fx(BitStream* stream, const afxEffectList& fx, bool packed)
{
  stream->writeInt(fx.size(), EFFECTS_PER_PHRASE_BITS);
  for (int i = 0; i < fx.size(); i++)
    writeDatablockID(stream, fx[i], packed);
}

void afxPhraseEffectData::unpack_fx(BitStream* stream, afxEffectList& fx)
{
  fx.clear();
  S32 n_fx = stream->readInt(EFFECTS_PER_PHRASE_BITS);
  for (int i = 0; i < n_fx; i++)
    fx.push_back((afxEffectWrapperData*)readDatablockID(stream));
}

void afxPhraseEffectData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->write(duration);
  stream->write(n_loops);
  stream->write(trigger_mask);
  stream->writeInt(match_type, 1);
  stream->writeInt(match_state, 2);
  stream->writeInt(phrase_type, 1);

  stream->writeFlag(no_choreographer_trigs);
  stream->writeFlag(no_cons_trigs);
  stream->writeFlag(no_player_trigs);

  stream->writeString(on_trig_cmd);

  pack_fx(stream, fx_list, packed);
}

void afxPhraseEffectData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&duration);
  stream->read(&n_loops);
  stream->read(&trigger_mask);
  match_type = stream->readInt(1);
  match_state = stream->readInt(2);
  phrase_type = stream->readInt(1);

  no_choreographer_trigs = stream->readFlag();
  no_cons_trigs = stream->readFlag();
  no_player_trigs = stream->readFlag();

  on_trig_cmd = stream->readSTString();

  do_id_convert = true;
  unpack_fx(stream, fx_list);
}

bool afxPhraseEffectData::preload(bool server, String &errorStr)
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
              "afxPhraseEffectData::preload() -- bad datablockId: 0x%x", 
              db_id);
          }
        }
      }
      do_id_convert = false;
    }
  }

  return true;
}

void afxPhraseEffectData::gather_cons_defs(Vector<afxConstraintDef>& defs)
{
  afxConstraintDef::gather_cons_defs(defs, fx_list);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

DefineEngineMethod( afxPhraseEffectData, addEffect, void, ( afxEffectBaseData* effectData ),,
   "Add a child effect to a phrase effect datablock. Argument can be an afxEffectWrappperData or an afxEffectGroupData.\n" )
{
  if (!effectData) 
  {
    Con::errorf("afxPhraseEffectData::addEffect() -- failed to resolve effect datablock.");
    return;
  }

  object->fx_list.push_back(effectData);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
