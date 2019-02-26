
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

#include "afx/ce/afxConsoleMessage.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxConsoleMessageData

IMPLEMENT_CO_DATABLOCK_V1(afxConsoleMessageData);

ConsoleDocClass( afxConsoleMessageData,
   "@brief A datablock that specifies a Console Message effect.\n\n"

   "Console Message effects are useful for debugging purposes when you want to make sure that an effect with a certain kind "
   "of timing is actually getting executed and for evaluating some kinds of field substitutions."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxConsoleMessageData::afxConsoleMessageData()
{
  message_str = ST_NULLSTRING;
}

afxConsoleMessageData::afxConsoleMessageData(const afxConsoleMessageData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  message_str = other.message_str;
}

#define myOffset(field) Offset(field, afxConsoleMessageData)

void afxConsoleMessageData::initPersistFields()
{
  addField("message",    TypeString,     myOffset(message_str),
    "A text message to be displayed when the effect is executed.");

  Parent::initPersistFields();
}

bool afxConsoleMessageData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxConsoleMessageData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->writeString(message_str);
}

void afxConsoleMessageData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  message_str = stream->readSTString();
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
