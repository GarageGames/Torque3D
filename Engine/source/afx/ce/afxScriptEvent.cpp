
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

#include "afx/ce/afxScriptEvent.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxScriptEventData

IMPLEMENT_CO_DATABLOCK_V1(afxScriptEventData);

ConsoleDocClass( afxScriptEventData,
   "@brief A datablock that specifies a Script Event effect.\n\n"

   "Arbitrary script functions can be called as an AFX effect using afxScriptEventData. They are useful for implementing "
   "high-level scripted side-effects such as character resurrection or teleportation."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxScriptEventData::afxScriptEventData()
{
  method_name = ST_NULLSTRING;
  script_data = ST_NULLSTRING;
}

afxScriptEventData::afxScriptEventData(const afxScriptEventData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  method_name = other.method_name;
  script_data = other.script_data;
}

#define myOffset(field) Offset(field, afxScriptEventData)

void afxScriptEventData::initPersistFields()
{
  addField("methodName",  TypeString,   myOffset(method_name),
    "The name of a script method defined for the instance class of an effects "
    "choreographer. The arguments used to call this method are determined by the type "
    "of choreographer.");
  addField("scriptData",  TypeString,   myOffset(script_data),
    "An arbitrary blind data value which is passed in as an argument of the script event "
    "method. The value of scriptData can be used to differentiate uses when handling "
    "different script event effects with a single method.");

  Parent::initPersistFields();
}

void afxScriptEventData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->writeString(method_name);
  stream->writeString(script_data);
}

void afxScriptEventData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  method_name = stream->readSTString();
  script_data = stream->readSTString();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
