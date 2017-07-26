
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
#include "afx/ce/afxPlayerMovement.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxPlayerMovementData

IMPLEMENT_CO_DATABLOCK_V1(afxPlayerMovementData);

ConsoleDocClass( afxPlayerMovementData,
   "@brief A datablock that specifies a Player Movement effect.\n\n"

   "Player Movement effects are used to directly alter the speed and/or movement direction of Player objects. The Player "
   "Movement effect is similar to the Player Puppet effect, but where puppet effects totally take over a Player's transformation "
   "using the AFX constraint system, Player Movement effects 'steer' the player using the same mechanisms that allow "
   "Player control from mouse and keyboard input. Another difference is that Player Movement effects only influence the "
   "server instance of a Player. Puppet effects can influence both the Player's server instance and its client ghosts."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

static Point3F default_movement(F32_MAX, F32_MAX, F32_MAX);

afxPlayerMovementData::afxPlayerMovementData()
{
  speed_bias = 1.0f;
  movement = default_movement;
  movement_op = OP_MULTIPLY;
}

afxPlayerMovementData::afxPlayerMovementData(const afxPlayerMovementData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  speed_bias = other.speed_bias;
  movement = other.movement;
  movement_op = other.movement_op;
}

ImplementEnumType( afxPlayerMovement_OpType, "Possible player movement operation types.\n" "@ingroup afxPlayerMovemen\n\n" )  
  { afxPlayerMovementData::OP_ADD,      "add",        "..." },
  { afxPlayerMovementData::OP_MULTIPLY, "multiply",   "..." },
  { afxPlayerMovementData::OP_REPLACE,  "replace",    "..." },
  { afxPlayerMovementData::OP_MULTIPLY, "mult",       "..." },
EndImplementEnumType;

#define myOffset(field) Offset(field, afxPlayerMovementData)

void afxPlayerMovementData::initPersistFields()
{
  addField("speedBias",     TypeF32,        myOffset(speed_bias),
    "A floating-point multiplier that scales the constraint Player's movement speed.");
  addField("movement",      TypePoint3F,    myOffset(movement),
    "");
  addField("movementOp", TYPEID<afxPlayerMovementData::OpType>(), myOffset(movement_op),
    "Possible values: add, multiply, or replace.");

  Parent::initPersistFields();
}

bool afxPlayerMovementData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxPlayerMovementData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->write(speed_bias);
  if (stream->writeFlag(movement != default_movement))
  {
    stream->write(movement.x);
    stream->write(movement.y);
    stream->write(movement.z);
    stream->writeInt(movement_op, OP_BITS);
  }
}

void afxPlayerMovementData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&speed_bias);
  if (stream->readFlag())
  {
    stream->read(&movement.x);
    stream->read(&movement.y);
    stream->read(&movement.z);
    movement_op = stream->readInt(OP_BITS);
  }
  else
  {
    movement = default_movement;
    movement_op = OP_MULTIPLY;
  }
}

bool afxPlayerMovementData::hasMovementOverride()
{
  return (movement != default_movement);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//