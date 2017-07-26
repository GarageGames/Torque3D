
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

#include "T3D/physicalZone.h"

#include "afx/ce/afxPhysicalZone.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxPhysicalZoneData

IMPLEMENT_CO_DATABLOCK_V1(afxPhysicalZoneData);

ConsoleDocClass( afxPhysicalZoneData,
   "@brief A datablock that specifies a PhysicalZone effect.\n\n"

   "A Physical Zone is a Torque effect that applies physical forces to Players and other movable objects that enter a specific "
   "region of influence. AFX has enhanced Physical Zones by allowing orientation of vector forces and adding radial forces. "
   "AFX has also optimized Physical Zone networking so that they can be constrained to moving objects for a variety of "
   "effects including repelling and flying."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxPhysicalZoneData::afxPhysicalZoneData()
{
  mVelocityMod = 1.0f;
  mGravityMod = 1.0f;
  mAppliedForce.zero();
  mPolyhedron = ST_NULLSTRING;
  force_type = PhysicalZone::VECTOR;
  orient_force = false;
  exclude_cons_obj = false;
}

afxPhysicalZoneData::afxPhysicalZoneData(const afxPhysicalZoneData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  mVelocityMod = other.mVelocityMod;
  mGravityMod = other.mGravityMod;
  mAppliedForce = other.mAppliedForce;
  mPolyhedron = other.mPolyhedron;
  force_type = other.force_type;
  orient_force = other.orient_force;
  exclude_cons_obj = other.exclude_cons_obj;
}

#define myOffset(field) Offset(field, afxPhysicalZoneData)

void afxPhysicalZoneData::initPersistFields()
{
  addField("velocityMod",               TypeF32,         myOffset(mVelocityMod),
    "A multiplier that biases the velocity of an object every tick it is within the "
    "zone.");
  addField("gravityMod",                TypeF32,         myOffset(mGravityMod),
    "A multiplier that biases the influence of gravity on objects within the zone.");
  addField("appliedForce",              TypePoint3F,     myOffset(mAppliedForce),
    "A three-valued vector representing a directional force applied to objects withing "
    "the zone.");
  addField("polyhedron",                TypeString,      myOffset(mPolyhedron),
    "Floating point values describing the outer bounds of the PhysicalZone's region of "
    "influence.");

  addField("forceType", TYPEID<PhysicalZone::ForceType>(), myOffset(force_type),
    "This enumerated attribute defines the type of force used in the PhysicalZone. "
    "Possible values: vector, sphere, or cylinder.");

  addField("orientForce",               TypeBool,        myOffset(orient_force),
    "Determines if the force can be oriented by the PhysicalZone's transform matrix.");
  addField("excludeConstraintObject",   TypeBool,        myOffset(exclude_cons_obj),
    "When true, an object used as the primary position constraint of a physical-zone "
    "effect will not be influenced by the forces of the zone.");

  Parent::initPersistFields();
}

void afxPhysicalZoneData::packData(BitStream* stream)
{
	Parent::packData(stream);
}

void afxPhysicalZoneData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//