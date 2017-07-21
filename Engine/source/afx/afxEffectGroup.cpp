
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

#include "afx/afxEffectGroup.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectGroupData::egValidator
//
// When an effect is added using "addEffect", this validator intercepts the value
// and adds it to the dynamic effects list. 
//
void afxEffectGroupData::egValidator::validateType(SimObject* object, void* typePtr)
{
  afxEffectGroupData* eff_data = dynamic_cast<afxEffectGroupData*>(object);
  afxEffectBaseData** ew = (afxEffectBaseData**)(typePtr);

  if (eff_data && ew)
  {
    eff_data->fx_list.push_back(*ew);
    *ew = 0;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectGroupData

IMPLEMENT_CO_DATABLOCK_V1(afxEffectGroupData);

ConsoleDocClass( afxEffectGroupData,
   "@brief A datablock that describes an Effect Group.\n\n"

   "afxEffectGroupData provides a way for adding several effects to a choreographer as a "
   "group and can be used wherever an afxEffectWrapperData is used. Basically, an "
   "effect-group is a simple list of effect-wrappers. When an effect-group is added to a "
   "choreographer, the end result is almost the same as adding all of the group's "
   "effect-wrappers directly to the choreographer. The main difference is that the "
   "grouped effects can be turned on and off collectively and created in multiples. "
   "Effect-groups can also contain other effect-groups, forming a hierarchy of effects.\n\n"

   "A great strength of effect-groups is that they have a count setting that multiplies "
   "the number of times the effects in the group are added to the owning choreographer "
   "and this doesn't happen until the choreographer instance is created and launched. "
   "This makes a big difference for certain kinds of effects, such as fireworks, that "
   "tend to consist of small groupings of effects that are repeated many times with "
   "slight variations. With groups, an effect like this has a very compact representation "
   "for transmitting from server to clients, that only expands when actually used.\n\n"

   "Effect-groups with a count greater than one are extremely useful when some of the "
   "effects use field substitutions. When an effect-group is expanded, it essentially runs "
   "through a for-loop from 0 to count-1 and creates a new set of effect instances each "
   "time through the loop. For each new set of effects, their group-index is set to the "
   "index of this for-loop, which in turn replaces the ## token used in any field "
   "substitutions in the child effects. In essence, the for-loop index becomes a parameter "
   "of the child effects which can be used to vary the effects created in each loop.\n\n"

   "@see afxEffectBaseData\n\n"
   "@see afxEffectWrapperData\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxEffectGroupData::afxEffectGroupData()
{
  group_enabled = true;
  group_count = 1;
  idx_offset = 0;
  assign_idx = false;

  // dummy entry holds effect-wrapper pointer while a special validator
  // grabs it and adds it to an appropriate effects list
  dummy_fx_entry = NULL;

  // marked true if datablock ids need to
  // be converted into pointers
  do_id_convert = false;
}

afxEffectGroupData::afxEffectGroupData(const afxEffectGroupData& other, bool temp_clone) : afxEffectBaseData(other, temp_clone)
{
  group_enabled = other.group_enabled;
  group_count = other.group_count;
  idx_offset = other.idx_offset;
  assign_idx = other.assign_idx;
  timing = other.timing;
  dummy_fx_entry = other.dummy_fx_entry;
  do_id_convert = other.do_id_convert; // --
  fx_list = other.fx_list; // -- 
}

void afxEffectGroupData::reloadReset()
{
  fx_list.clear();
}

void afxEffectGroupData::pack_fx(BitStream* stream, const afxEffectList& fx, bool packed)
{
  stream->writeInt(fx.size(), EFFECTS_PER_PHRASE_BITS);
  for (int i = 0; i < fx.size(); i++)
    writeDatablockID(stream, fx[i], packed);
}

void afxEffectGroupData::unpack_fx(BitStream* stream, afxEffectList& fx)
{
  fx.clear();
  S32 n_fx = stream->readInt(EFFECTS_PER_PHRASE_BITS);
  for (int i = 0; i < n_fx; i++)
    fx.push_back((afxEffectWrapperData*)readDatablockID(stream));
}

#define myOffset(field) Offset(field, afxEffectGroupData)

void afxEffectGroupData::initPersistFields()
{
  addField("groupEnabled",   TypeBool,    myOffset(group_enabled),
    "...");
  addField("count",          TypeS32,     myOffset(group_count),
    "...");
  addField("indexOffset",    TypeS8,      myOffset(idx_offset),
    "...");
  addField("assignIndices",  TypeBool,    myOffset(assign_idx),
    "...");

  addField("delay",          TypeF32,     myOffset(timing.delay),
    "...");
  addField("lifetime",       TypeF32,     myOffset(timing.lifetime),
    "...");
  addField("fadeInTime",     TypeF32,     myOffset(timing.fade_in_time),
    "...");
  addField("fadeOutTime",    TypeF32,     myOffset(timing.fade_out_time),
    "...");

  // effect lists
  // for each of these, dummy_fx_entry is set and then a validator adds it to the appropriate effects list 
  static egValidator emptyValidator(0);
  
  addFieldV("addEffect",  TYPEID<afxEffectBaseData>(),  myOffset(dummy_fx_entry), &emptyValidator,
    "...");

  Parent::initPersistFields();

  // disallow some field substitutions
  disableFieldSubstitutions("addEffect");
}

void afxEffectGroupData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->writeFlag(group_enabled);
  stream->write(group_count);
  stream->write(idx_offset);
  stream->writeFlag(assign_idx);
  stream->write(timing.delay);
  stream->write(timing.lifetime);
  stream->write(timing.fade_in_time);
  stream->write(timing.fade_out_time);

  pack_fx(stream, fx_list, packed);
}

void afxEffectGroupData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  group_enabled = stream->readFlag();
  stream->read(&group_count);
  stream->read(&idx_offset);
  assign_idx = stream->readFlag();
  stream->read(&timing.delay);
  stream->read(&timing.lifetime);
  stream->read(&timing.fade_in_time);
  stream->read(&timing.fade_out_time);

  do_id_convert = true;
  unpack_fx(stream, fx_list);
}

bool afxEffectGroupData::preload(bool server, String &errorStr)
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
              "afxEffectGroupData::preload() -- bad datablockId: 0x%x", 
              db_id);
          }
        }
      }
      do_id_convert = false;
    }
  }

  return true;
}

void afxEffectGroupData::gather_cons_defs(Vector<afxConstraintDef>& defs)
{
  for (S32 i = 0; i < fx_list.size(); i++)
  {
    if (fx_list[i])
      fx_list[i]->gather_cons_defs(defs);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

DefineEngineMethod(afxEffectGroupData, reset, void, (),,
                   "Resets an effect-group datablock during reload.\n\n"
                   "@ingroup AFX")
{
  object->reloadReset();
}

DefineEngineMethod(afxEffectGroupData, addEffect, void, (afxEffectBaseData* effect),,
                   "Adds an effect (wrapper or group) to an effect-group.\n\n"
                   "@ingroup AFX")
{
  if (!effect) 
  {
    Con::errorf("afxEffectGroupData::addEffect() -- missing afxEffectWrapperData.");
    return;
  }
  
  object->fx_list.push_back(effect);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

