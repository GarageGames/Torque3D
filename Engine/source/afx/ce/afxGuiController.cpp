
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

#include "core/stream/bitStream.h"

#include "afx/ce/afxGuiController.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxGuiControllerData

IMPLEMENT_CO_DATABLOCK_V1(afxGuiControllerData);

ConsoleDocClass( afxGuiControllerData,
   "@brief A datablock that specifies a Gui Controller effect.\n\n"

   "A Gui Controller enables effect manipulation of pre-existing gui controls. With a Gui Controller effect, a regular gui control "
   "is located by name, made visible during the lifetime of the effect, and potentially repositioned by projecting 3D constraint "
   "positions into 2D screen space. In addition, when used with a progress-bar control, (GuiProgressCtrl, afxSpellCastBar, "
   "afxStatusBar), the progress-bar will continuously reflect the elapsed progress of the effect over its lifetime."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxGuiControllerData::afxGuiControllerData()
{
  control_name = ST_NULLSTRING;
  preserve_pos = false;
  ctrl_client_only = false;
}

afxGuiControllerData::afxGuiControllerData(const afxGuiControllerData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  control_name = other.control_name;
  preserve_pos = other.preserve_pos;
  ctrl_client_only = other.ctrl_client_only;
}

#define myOffset(field) Offset(field, afxGuiControllerData)

void afxGuiControllerData::initPersistFields()
{
  addField("controlName",           TypeString,     myOffset(control_name),
    "Specifies the name of an existing gui-control.");
  addField("preservePosition",      TypeBool,       myOffset(preserve_pos),
    "When true, the gui-control will retain its initial position, otherwise the "
    "gui-control position will be continuously updated using a projection of the "
    "3D constraint position into 2D screen coordinates.");
  addField("controllingClientOnly", TypeBool,       myOffset(ctrl_client_only),
    "If true, the effect will only be applied to a gui-control on the client that "
    "matches the controlling-client of the primary position constraint object.");

  Parent::initPersistFields();
}

bool afxGuiControllerData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxGuiControllerData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->writeString(control_name);
  stream->writeFlag(preserve_pos);
  stream->writeFlag(ctrl_client_only);
}

void afxGuiControllerData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  control_name = stream->readSTString();
  preserve_pos = stream->readFlag();
  ctrl_client_only = stream->readFlag();
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
