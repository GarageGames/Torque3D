
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

#include "afx/ce/afxAreaDamage.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxAreaDamageData

IMPLEMENT_CO_DATABLOCK_V1(afxAreaDamageData);

ConsoleDocClass( afxAreaDamageData,
   "@brief A datablock that specifies an Area Damage effect.\n\n"

   "An Area Damage effect is useful for assigning area damage with unusual timing that must be synchronized with other "
   "effects. Negative damage amounts can be used for healing effects."
   "\n\n"

   "The primary difference between afxAreaDamageData and afxDamageData, which is also capable of inflicting area damage, "
   "is that afxAreaDamageData effects calculate the area damage in C++ code rather than calling out to the script function "
   "radiusDamage(). In cases where area damage needs to be inflicted repeatedly or in areas crowded with many targets, "
   "afxAreaDamageData is likely to get better performance."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxAreaDamageData::afxAreaDamageData()
{
  flavor = ST_NULLSTRING;
  amount = 0;
  radius = 0;
  impulse = 0;
  notify_damage_src = false;
  exclude_cons_obj = false;
}

afxAreaDamageData::afxAreaDamageData(const afxAreaDamageData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  flavor = other.flavor;
  amount = other.amount;
  radius = other.radius;
  impulse = other.impulse;
  notify_damage_src = other.notify_damage_src;
  exclude_cons_obj = other.exclude_cons_obj;
}

#define myOffset(field) Offset(field, afxAreaDamageData)

void afxAreaDamageData::initPersistFields()
{
  addField("flavor",                    TypeString,     myOffset(flavor),
    "An arbitrary string which is passed as an argument to a spell's onDamage() script "
    "method. It is used to classify a type of damage such as 'melee', 'magical', or "
    "'fire'.");
  addField("damage",                    TypeF32,        myOffset(amount),
    "An amount of area damage to inflict on a target. Objects within half the radius "
    "receive full damage which then diminishes out to the full distance of the specified "
    "radius.");
  addField("radius",                    TypeF32,        myOffset(radius),
    "Radius centered at the effect position in which damage will be applied.");
  addField("impulse",                   TypeF32,        myOffset(impulse),
    "Specifies an amount of force to apply to damaged objects. Objects within half the "
    "radius receive full impulse which then diminishes out to the full distance of the "
    "specified radius.");
  addField("notifyDamageSource",        TypeBool,       myOffset(notify_damage_src),
    "When true, the onInflictedAreaDamage() method of the damaged object will be called "
    "to notify it of the damage. This is useful for starting some effects or action that "
    "responds to the damage.");
  addField("excludeConstraintObject",   TypeBool,       myOffset(exclude_cons_obj),
    "When true, the object specified as the effect's primary position constraint will not "
    "receive any damage.");

  Parent::initPersistFields();
}

bool afxAreaDamageData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxAreaDamageData::packData(BitStream* stream)
{
	Parent::packData(stream);
}

void afxAreaDamageData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
