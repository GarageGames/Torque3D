
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

#include <typeinfo>
#include "afx/arcaneFX.h"

#include "afx/forces/afxForce.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxF_DragData : public afxForceData
{
  typedef afxForceData Parent;

public:
  F32             drag_coefficient;
  F32             air_density;
  F32             cross_sectional_area;

public:
  /*C*/           afxF_DragData();
  /*C*/           afxF_DragData(const afxF_DragData&, bool = false);
  
  virtual void    packData(BitStream* stream);
  virtual void    unpackData(BitStream* stream);
  virtual afxForceData* cloneAndPerformSubstitutions(const SimObject*, S32 index=0);

  static void     initPersistFields();

  DECLARE_CONOBJECT(afxF_DragData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxF_Drag : public afxForce
{
  typedef afxForce Parent;

private:
  afxF_DragData*  datablock;
  F32             air_friction_constant;

public:
  /*C*/           afxF_Drag();

  virtual bool    onNewDataBlock(afxForceData* dptr, bool reload);

  virtual void    start();
  virtual Point3F evaluate(Point3F pos, Point3F v, F32 mass);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxDragData

IMPLEMENT_CO_DATABLOCK_V1(afxF_DragData);

ConsoleDocClass( afxF_DragData,
   "@brief A datablock for specifiying AFX drag forces.\n\n"

   "@ingroup afxExperimental\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxF_DragData::afxF_DragData()
{
  air_density = 1.2250f; 
  cross_sectional_area = 0.75f;  // this variable isn't exposed to the user to keep things simple
  drag_coefficient = 1.0f;
}

afxF_DragData::afxF_DragData(const afxF_DragData& other, bool temp_clone) : afxForceData(other, temp_clone)
{
  air_density = other.air_density;
  cross_sectional_area = other.cross_sectional_area;
  drag_coefficient = other.drag_coefficient;
}

#define myOffset(field) Offset(field, afxF_DragData)

void afxF_DragData::initPersistFields()
{
  addField("drag",                TypeF32,      myOffset(drag_coefficient),
    "...");
  addField("airDensity",          TypeF32,      myOffset(air_density),
    "...");
  addField("crossSectionalArea",  TypeF32,      myOffset(cross_sectional_area),
    "...");

  Parent::initPersistFields();
}

void afxF_DragData::packData(BitStream* stream)
{
	Parent::packData(stream);
  stream->write(drag_coefficient);
  stream->write(air_density);
  stream->write(cross_sectional_area);
}

void afxF_DragData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&drag_coefficient);
  stream->read(&air_density);
  stream->read(&cross_sectional_area);
}

afxForceData* afxF_DragData::cloneAndPerformSubstitutions(const SimObject* owner, S32 index)
{
  afxF_DragData* drag_data = this;

  // only clone the datablock if there are substitutions
  if (this->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxF_DragData* orig_db = this;
    drag_data = new afxF_DragData(*orig_db, true);
    orig_db->performSubstitutions(drag_data, owner, index);
  }

  return drag_data;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

afxF_Drag::afxF_Drag() : afxForce()
{
  air_friction_constant = 1.0f;
}

bool afxF_Drag::onNewDataBlock(afxForceData* dptr, bool reload)
{
  datablock = dynamic_cast<afxF_DragData*>(dptr);
  if (!datablock || !Parent::onNewDataBlock(dptr, reload))
    return false;

  return true;
}

void afxF_Drag::start()
{
  air_friction_constant = 0.5f * datablock->drag_coefficient 
                               * datablock->air_density 
                               * datablock->cross_sectional_area;
  //Con::printf("Air Friction: %f", air_friction_constant);
}

Point3F afxF_Drag::evaluate(Point3F pos, Point3F velocity, F32 mass)
{
  // This implements the standard drag equation for object's at high speeds.
  // F-drag = 1/2pACv^2
  //   p = medium (air) density
  //   A = cross-sectional area of moving object (plane perpendicular to direction of motion)
  //   C = coefficient of drag

  // -- Velocity here should actually be relative to the velocity of the fluid... (relative speed)
  //      (is it already?)
  F32 drag = air_friction_constant*velocity.magnitudeSafe();

  // Here, velocity is normalized just to get a direction vector.
  //  Drag is in the direction opposite velocity.  Is this right?
  velocity.normalizeSafe();

  return (velocity*-drag*mass);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxF_DragDesc : public afxForceDesc
{
  static afxF_DragDesc desc;

public:
  virtual bool testForceType(const SimDataBlock*) const;
  virtual afxForce* create() const { return new afxF_Drag; }
};

afxF_DragDesc afxF_DragDesc::desc;

bool afxF_DragDesc::testForceType(const SimDataBlock* db) const
{
  return (typeid(afxF_DragData) == typeid(*db));
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
