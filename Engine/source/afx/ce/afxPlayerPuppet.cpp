
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
#include "scene/sceneRenderState.h"
#include "math/mathIO.h"

#include "afx/afxChoreographer.h"
#include "afx/ce/afxPlayerPuppet.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxPlayerPuppetData

IMPLEMENT_CO_DATABLOCK_V1(afxPlayerPuppetData);

ConsoleDocClass( afxPlayerPuppetData,
   "@brief A datablock that specifies a Player Puppet effect.\n\n"

   "Player Puppet effects are defined using the afxPlayerPuppetData datablock and are used to control the movement of "
   "Player objects with the AFX constraint system. The Player Puppet effect is similar to the Player Movement effect, but "
   "where movement effects 'steer' the player using the same mechanisms that allow Player control from mouse and keyboard "
   "input, Player Puppet effects totally take over a Player's transformation using the AFX constraint system."
   "\n\n"

   "Player Puppet can be configured to directly move a Player's client ghosts as well as its server instance. When doing this, "
   "it is important to keep the general motion of the Player object and its ghosts somewhat consistent. Otherwise, obvious "
   "discontinuities in the motion will result when the Player Puppet ends and control is restored to the server Player."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxPlayerPuppetData::afxPlayerPuppetData()
{
  obj_spec = ST_NULLSTRING;
  networking = SERVER_ONLY;
}

afxPlayerPuppetData::afxPlayerPuppetData(const afxPlayerPuppetData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  obj_spec = other.obj_spec;
  networking = other.networking;
}

#define myOffset(field) Offset(field, afxPlayerPuppetData)

void afxPlayerPuppetData::initPersistFields()
{
  addField("objectSpec",      TypeString,   myOffset(obj_spec),
    "...");
  addField("networking",      TypeS8,       myOffset(networking),
    "...");

  Parent::initPersistFields();

  // disallow some field substitutions
  disableFieldSubstitutions("objectSpec");
  disableFieldSubstitutions("networking");
}

bool afxPlayerPuppetData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  bool runs_on_s = ((networking & (SERVER_ONLY | SERVER_AND_CLIENT)) != 0);
  bool runs_on_c = ((networking & (CLIENT_ONLY | SERVER_AND_CLIENT)) != 0);
  obj_def.parseSpec(obj_spec, runs_on_s, runs_on_c);

  return true;
}

void afxPlayerPuppetData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->writeString(obj_spec);
  stream->write(networking);
}

void afxPlayerPuppetData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  obj_spec = stream->readSTString();
  stream->read(&networking);
}

void afxPlayerPuppetData::gather_cons_defs(Vector<afxConstraintDef>& defs)
{ 
  if (obj_def.isDefined())
    defs.push_back(obj_def);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//