
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

#include "afx/ce/afxAnimLock.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxAnimLockData

IMPLEMENT_CO_DATABLOCK_V1(afxAnimLockData);

ConsoleDocClass( afxAnimLockData,
   "@brief A datablock that specifies an Animation Lock effect.\n\n"

   "Animation Lock is used to temporarily lock out user-controlled Player actions, usually while an Animation Clip is "
   "concurrently playing. Animation Clips can already do this, but must lock out user actions for the entire clip length. "
   "Sometimes you only want to block user actions for a short section of a longer playing animation, such as the part where "
   "the Player is thrown into the air from an impact. With Animation Lock, you can set a specific timespan for when user "
   "actions are blocked, independent of any Animation Clip timing."
   "\n\n"

   "The target of an Animation Lock is the constraint source object specified by the posConstraint field of the enclosing effect "
   "wrapper. The target must be a Player, a subclass of Player, or an afxModel."
   "\n\n"

   "The timing of the Animation Lock is determined by the timing fields of the enclosing effect wrapper."
   "\n\n"

   "Locking behavior timing is set by fields of the enclosing effect wrapper, so afxAnimLockData does not require any fields. "
   "However, TorqueScript syntax disallows the declaration of an empty datablock. Therefore, it is recommended that you set "
   "a dynamic field named 'priority' to zero in the body of the datablock as a workaround to this limitation."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxAnimLockData::afxAnimLockData()
{
}

#define myOffset(field) Offset(field, afxAnimLockData)

void afxAnimLockData::initPersistFields()
{
  Parent::initPersistFields();
}

bool afxAnimLockData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxAnimLockData::packData(BitStream* stream)
{
	Parent::packData(stream);
}

void afxAnimLockData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
