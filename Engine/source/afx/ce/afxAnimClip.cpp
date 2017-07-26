
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

#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"

#include "afx/ce/afxAnimClip.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxAnimClipData

IMPLEMENT_CO_DATABLOCK_V1(afxAnimClipData);

ConsoleDocClass( afxAnimClipData,
   "@brief A datablock that specifies an Animation Clip effect.\n\n"

   "An Animation Clip forces a target ShapeBase-derived object, such as Player or AIPlayer, to perform a particular "
   "animation sequence. Animation Clip does not supply any new animation data, but simply selects, by name, a "
   "sequence that is already defined in the target. Animation Clip can also target afxModel effects within the same "
   "choreographer."
   "\n\n"

   "The target of an Animation Clip is the constraint source object specified by the posConstraint field of the enclosing "
   "effect wrapper. The target must be a ShapeBase-derived object, or an afxModel and it must contain an animation "
   "sequence with the same name as the clipName field."
   "\n\n"

   "Animation Clip controls the rate of animation playback and can even play a sequence in reverse. When an Animation "
   "Clip selects a blended animation sequence, it is mixed with the current animation instead of replacing it. Animation "
   "Clips can be used to activate multiple, overlapping blend sequences."
   "\n\n"

   "Normally when an Animation Clip is applied to a user-controlled Player, any interactive user actions will override the "
   "animation selected by the clip, but Animation Clips can be configured to temporarily block out some user actions for "
   "the duration of the clip."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxAnimClipData::afxAnimClipData()
{
  clip_name = ST_NULLSTRING;
  rate = 1.0f;
  pos_offset = 0.0;
  trans = 0.12f;
  flags = 0;

  ignore_disabled = false;
  ignore_enabled = false;
  is_death_anim = false;
  lock_anim = false;
  ignore_first_person = false;
  ignore_third_person = false;
}

afxAnimClipData::afxAnimClipData(const afxAnimClipData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  clip_name = other.clip_name;
  rate = other.rate;
  pos_offset = other.pos_offset;
  trans = other.trans;
  flags = other.flags;

  expand_flags();
}

void afxAnimClipData::onStaticModified(const char* slot, const char* newValue)
{
  Parent::onStaticModified(slot, newValue);
  merge_flags();
}

#define myOffset(field) Offset(field, afxAnimClipData)

void afxAnimClipData::initPersistFields()
{
  addField("clipName",          TYPEID< StringTableEntry >(),  myOffset(clip_name),
    "The name of an animation sequence to be played by a ShapeBase-derived object to which this effect is "
    "constrained. Also works on afxModel effects.\n"
    "default: \"\"\n");
  addField("rate",              TYPEID< F32 >(),               myOffset(rate),                  
    "The desired playback speed for the sequence. A value of 1.0 indicates forward playback at a normal rate. Negative "
    "values cause the sequence to play backwards.\n"
    "default: 1.0\n");
  addField("posOffset",         TYPEID< F32 >(),               myOffset(pos_offset),
    "Sets a starting offset for the selected animation clip. It directly specifies an animation thread position in the 0.0 to "
    "1.0 range as a fraction of the clip's duration.\n"
    "default: 1.0\n");
  addField("transitionTime",    TYPEID< F32 >(),               myOffset(trans),
    "The duration in which the active animation overlaps and blends into the sequence selected by the animation clip.\n"
    "default: 0.12\n");
  addField("ignoreCorpse",      TYPEID< bool >(),              myOffset(ignore_disabled),
    "Specifies if the animation clip should not be applied to corpses or anything else with a disabled damage state.\n"
    "default: false\n");
  addField("ignoreLiving",      TYPEID< bool >(),              myOffset(ignore_enabled),
    "Specifies if the animation clip should not be applied to living objects or anything else with an enabled damage "
    "state.\n"
    "default: false\n");
  addField("treatAsDeathAnim",  TYPEID< bool >(),              myOffset(is_death_anim),
    "Indicates if the animation clip is a death animation. If the target object dies during the effect, this will prevent "
    "the object from playing another standard death animation after this clip finishes.\n"
    "default: false\n");
  addField("lockAnimation",     TYPEID< bool >(),              myOffset(lock_anim),
    "Indicates if user control of a Player should be temporarily blocked during the clip. (See afxAnimLockData.)\n"
    "default: false\n");
  addField("ignoreFirstPerson", TYPEID< bool >(),              myOffset(ignore_first_person),   
    "If true, the clip will not be played on targets that are the control object and the camera is in first person mode.\n"
    "default: false\n");
  addField("ignoreThirdPerson", TYPEID< bool >(),              myOffset(ignore_third_person),   
    "If true, the clip will not be played on targets that are the control object and the camera is in third person mode.\n"
    "default: false\n");

  // synonyms
  addField("ignoreDisabled",    TYPEID< bool >(),              myOffset(ignore_disabled),
    "A synonym for ignoreLiving.");
  addField("ignoreEnabled",     TYPEID< bool >(),              myOffset(ignore_enabled),
    "A synonym for ignoreCorpse.");

  Parent::initPersistFields();
}

bool afxAnimClipData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxAnimClipData::packData(BitStream* stream)
{
	Parent::packData(stream);

  merge_flags();

  stream->writeString(clip_name);
  stream->write(rate);
  stream->write(pos_offset);
  stream->write(trans);
  stream->write(flags);
}

void afxAnimClipData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  clip_name = stream->readSTString();
  stream->read(&rate);
  stream->read(&pos_offset);
  stream->read(&trans);
  stream->read(&flags);

  expand_flags();
}

bool afxAnimClipData::writeField(StringTableEntry fieldname, const char* value)
{
   if (!Parent::writeField(fieldname, value))
      return false;

   // don't write the synonyms
   if( fieldname == StringTable->insert("ignoreDisabled") )
      return false;
   if( fieldname == StringTable->insert("ignoreEnabled") )
      return false;

   return true;
}

void afxAnimClipData::expand_flags()
{
  ignore_disabled = ((flags & IGNORE_DISABLED) != 0);
  ignore_enabled = ((flags & IGNORE_ENABLED) != 0);
  lock_anim = ((flags & BLOCK_USER_CONTROL) != 0);
  is_death_anim = ((flags & IS_DEATH_ANIM) != 0);
  ignore_first_person = ((flags & IGNORE_FIRST_PERSON) != 0);
  ignore_third_person = ((flags & IGNORE_THIRD_PERSON) != 0);
}

void afxAnimClipData::merge_flags()
{
  flags = (((ignore_disabled) ? IGNORE_DISABLED : 0) | 
           ((ignore_enabled) ? IGNORE_ENABLED : 0) | 
           ((lock_anim) ? BLOCK_USER_CONTROL : 0) | 
           ((ignore_first_person) ? IGNORE_FIRST_PERSON : 0) |
           ((ignore_third_person) ? IGNORE_THIRD_PERSON : 0) |
           ((is_death_anim) ? IS_DEATH_ANIM : 0));
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
