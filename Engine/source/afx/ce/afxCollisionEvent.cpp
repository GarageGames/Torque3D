
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

#include "afx/ce/afxCollisionEvent.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxCollisionEventData

IMPLEMENT_CO_DATABLOCK_V1(afxCollisionEventData);

ConsoleDocClass( afxCollisionEventData,
   "@brief A datablock that specifies a Collision Event effect.\n\n"

   "MORE NEEDED HERE.\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxCollisionEventData::afxCollisionEventData()
{
  method_name = ST_NULLSTRING;
  script_data = ST_NULLSTRING;
  gen_trigger = false;
  trigger_bit = 0;
}

afxCollisionEventData::afxCollisionEventData(const afxCollisionEventData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  method_name = other.method_name;
  script_data = other.script_data;
  gen_trigger = other.gen_trigger;
  trigger_bit = other.trigger_bit;
}

#define myOffset(field) Offset(field, afxCollisionEventData)

void afxCollisionEventData::initPersistFields()
{
  addField("methodName",        TypeString,   myOffset(method_name),
    "...");
  addField("scriptData",        TypeString,   myOffset(script_data),
    "...");
  addField("generateTrigger",   TypeBool,     myOffset(gen_trigger),
    "...");
  addField("triggerBit",        TypeS8,       myOffset(trigger_bit),
    "...");

  Parent::initPersistFields();
}

void afxCollisionEventData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->writeString(method_name);
  stream->writeString(script_data);
  if (stream->writeFlag(gen_trigger))
    stream->write(trigger_bit);
}

void afxCollisionEventData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  method_name = stream->readSTString();
  script_data = stream->readSTString();
  gen_trigger = stream->readFlag();
  if (gen_trigger)
    stream->read(&trigger_bit);
  else
    trigger_bit = 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
