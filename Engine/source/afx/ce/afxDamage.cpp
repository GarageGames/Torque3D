
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

#include "afx/ce/afxDamage.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxDamageData

IMPLEMENT_CO_DATABLOCK_V1(afxDamageData);

ConsoleDocClass( afxDamageData,
   "@brief A datablock that specifies a Damage effect.\n\n"

   "A Damage effect is useful for assigning damage with unusual timing that must be synchronized with other effects. They " 
   "can be used to deal direct damage, radius damage, and damage over time. Negative damage amounts can be used for "
   "healing effects."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxDamageData::afxDamageData()
{
  label = ST_NULLSTRING;
  flavor = ST_NULLSTRING;
  amount = 0;
  repeats = 1;
  ad_amount = 0;
  radius = 0;
  impulse = 0;
}

afxDamageData::afxDamageData(const afxDamageData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  label = other.label;
  flavor = other.flavor;
  amount = other.amount;
  repeats = other.repeats;
  ad_amount = other.ad_amount;
  radius = other.radius;
  impulse = other.impulse;
}

#define myOffset(field) Offset(field, afxDamageData)

void afxDamageData::initPersistFields()
{
  addField("label",               TypeString,     myOffset(label),
    "An arbitrary string which is passed as an argument to a spell's onDamage() script "
    "method. It can be used to identify which damage effect the damage came from in "
    "cases where more than one damage effect is used in a single spell.");
  addField("flavor",              TypeString,     myOffset(flavor),
    "An arbitrary string which is passed as an argument to a spell's onDamage() script "
    "method. It is used to classify a type of damage such as 'melee', 'magical', or "
    "'fire'.");
  addField("directDamage",        TypeF32,        myOffset(amount),
    "An amount of direct damage to inflict on a target.");
  addField("directDamageRepeats", TypeS8,         myOffset(repeats),
    "The number of times to inflict the damage specified by directDamage. Values "
    "greater than 1 inflict damage over time, with the amount of directDamage "
    "repeatedly dealt at evenly spaced intervals over the lifetime of the effect.");
  addField("areaDamage",          TypeF32,        myOffset(ad_amount),
    "An amount of area damage to inflict on a target. Objects within half the radius "
    "receive full damage which then diminishes out to the full distance of "
    "areaDamageRadius.");
  addField("areaDamageRadius",    TypeF32,        myOffset(radius),
    "Radius centered at the effect position in which damage will be applied.");
  addField("areaDamageImpulse",   TypeF32,        myOffset(impulse),
    "Specifies an amount of force to apply to damaged objects. Objects within half the "
    "radius receive full impulse which then diminishes out to the full distance of "
    "areaDamageRadius.");

  Parent::initPersistFields();
}

bool afxDamageData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxDamageData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->writeString(label);
  stream->writeString(flavor);
  stream->write(amount);
  stream->write(repeats);
  stream->write(ad_amount);
  stream->write(radius);
  stream->write(impulse);
}

void afxDamageData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  label = stream->readSTString();
  flavor = stream->readSTString();
  stream->read(&amount);
  stream->read(&repeats);
  stream->read(&ad_amount);
  stream->read(&radius);
  stream->read(&impulse);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
