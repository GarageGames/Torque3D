
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

#include "afx/ce/afxGuiText.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxGuiTextData

IMPLEMENT_CO_DATABLOCK_V1(afxGuiTextData);

ConsoleDocClass( afxGuiTextData,
   "@brief A datablock that specifies a Gui Text effect.\n\n"

   "A Gui Text effect, with the help of an existing afxGuiTextHud, can be used to display 2D text effects on the Gui Canvas. "
   "Essentially, using Gui Text effects with an afxGuiTextHud is like using the stock GuiShapeNameHud, but with the ability "
   "to make additional text elements come and go as effects constrained to the projection of 3D positions onto the 2D screen."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxGuiTextData::afxGuiTextData()
{
  text_str = ST_NULLSTRING;
  text_clr.set(1,1,1,1);
}

afxGuiTextData::afxGuiTextData(const afxGuiTextData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  text_str = other.text_str;
  text_clr = other.text_clr;
}

#define myOffset(field) Offset(field, afxGuiTextData)

void afxGuiTextData::initPersistFields()
{
  addField("text",    TypeString,     myOffset(text_str),
    "The literal text to display on the afxGuiTextHud. The center of the text will be "
    "placed at the projection of the 3D constraint position into 2D screen space.\n"
    "If the text field is set to the special string, '#shapeName', the shape name of the "
    "primary position constraint object will be used. (This is only meaningful if the "
    "constraint source is a ShapeBase-derived object.)");
  addField("color",   TypeColorF,     myOffset(text_clr),
    "A color value for the text label.");

  Parent::initPersistFields();
}

bool afxGuiTextData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxGuiTextData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->writeString(text_str);
  stream->write(text_clr);
}

void afxGuiTextData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  text_str = stream->readSTString();
  stream->read(&text_clr);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
