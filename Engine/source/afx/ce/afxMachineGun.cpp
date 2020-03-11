
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
#include "lighting/lightInfo.h"
#include "T3D/projectile.h"

#include "afx/ce/afxMachineGun.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxMachineGunData

IMPLEMENT_CO_DATABLOCK_V1(afxMachineGunData);

ConsoleDocClass( afxMachineGunData,
   "@brief A datablock that specifies a Machine Gun effect.\n\n"

   "Machine Gun is a simple but useful effect for rapidly shooting standard Torque Projectile objects. For performance "
   "reasons, keep in mind that each bullet is a separate Projectile object, which is not a very lightweight object."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxMachineGunData::afxMachineGunData()
{
  projectile_data = 0;
  rounds_per_minute = 60;
}

afxMachineGunData::afxMachineGunData(const afxMachineGunData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  projectile_data = other.projectile_data;
  rounds_per_minute = other.rounds_per_minute;
}

#define myOffset(field) Offset(field, afxMachineGunData)

void afxMachineGunData::initPersistFields()
{
  addField("projectile", TYPEID<ProjectileData>(), myOffset(projectile_data),
    "A ProjectileData datablock describing the projectile to be launched.");
  addField("roundsPerMinute", TypeS32, myOffset(rounds_per_minute),
    "Specifies the number of projectiles fired over a minute of time. A value of 1200 "
    "will create 20 projectiles per second.\n"
    "Sample values for real machine guns:\n"
    "    AK-47 = 600, M16 = 750-900, UZI = 600");

  Parent::initPersistFields();

  // disallow some field substitutions
  disableFieldSubstitutions("projectile");
}

bool afxMachineGunData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  if (projectile_data)
  { 
    if (getId() >= DataBlockObjectIdFirst && getId() <= DataBlockObjectIdLast)
    {
      SimObjectId pid = projectile_data->getId();
      if (pid < DataBlockObjectIdFirst || pid > DataBlockObjectIdLast)
      {
        Con::errorf(ConsoleLogEntry::General,"afxMachineGunData: bad ProjectileData datablock.");
        return false;
      }
    }
  }

  return true;
}

void afxMachineGunData::packData(BitStream* stream)
{
	Parent::packData(stream);

  if (stream->writeFlag(projectile_data))
    stream->writeRangedU32(projectile_data->getId(), DataBlockObjectIdFirst, DataBlockObjectIdLast);

  stream->write(rounds_per_minute);
}

void afxMachineGunData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  if (stream->readFlag()) 
  {
    SimObjectId id = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
    Sim::findObject(id, projectile_data);
  }
  
  stream->read(&rounds_per_minute);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
