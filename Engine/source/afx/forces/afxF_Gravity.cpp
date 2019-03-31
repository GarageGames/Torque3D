
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

class afxF_GravityData : public afxForceData
{
  typedef afxForceData Parent;

public:
  F32             gravity;

public:
  /*C*/           afxF_GravityData();
  /*C*/           afxF_GravityData(const afxF_GravityData&, bool = false);
  
  virtual void    packData(BitStream* stream);
  virtual void    unpackData(BitStream* stream);
  virtual afxForceData* cloneAndPerformSubstitutions(const SimObject*, S32 index=0);

  static void     initPersistFields();

  DECLARE_CONOBJECT(afxF_GravityData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxF_Gravity : public afxForce
{
  typedef afxForce Parent;

private:
  afxF_GravityData* datablock;

public:
  /*C*/             afxF_Gravity();

  virtual bool      onNewDataBlock(afxForceData* dptr, bool reload);

  virtual Point3F   evaluate(Point3F pos, Point3F v, F32 mass);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxForceData

IMPLEMENT_CO_DATABLOCK_V1(afxF_GravityData);

ConsoleDocClass( afxF_GravityData,
   "@brief A datablock for specifiying AFX gravity forces.\n\n"

   "@ingroup afxExperimental\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxF_GravityData::afxF_GravityData()
{
  gravity = 9.807f;
}

afxF_GravityData::afxF_GravityData(const afxF_GravityData& other, bool temp_clone) : afxForceData(other, temp_clone)
{
  gravity = other.gravity;
}

#define myOffset(field) Offset(field, afxF_GravityData)

void afxF_GravityData::initPersistFields()
{
  addField("gravity",   TypeF32,     myOffset(gravity),
    "...");

  Parent::initPersistFields();
}

void afxF_GravityData::packData(BitStream* stream)
{
	Parent::packData(stream);
  stream->write(gravity);
}

void afxF_GravityData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&gravity);
}

afxForceData* afxF_GravityData::cloneAndPerformSubstitutions(const SimObject* owner, S32 index)
{
  afxF_GravityData* grav_data = this;

  // only clone the datablock if there are substitutions
  if (this->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxF_GravityData* orig_db = this;
    grav_data = new afxF_GravityData(*orig_db, true);
    orig_db->performSubstitutions(grav_data, owner, index);
  }

  return grav_data;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

afxF_Gravity::afxF_Gravity() : afxForce()
{
}

bool afxF_Gravity::onNewDataBlock(afxForceData* dptr, bool reload)
{
  datablock = dynamic_cast<afxF_GravityData*>(dptr);
  if (!datablock || !Parent::onNewDataBlock(dptr, reload))
    return false;

  return true;
}

Point3F afxF_Gravity::evaluate(Point3F pos, Point3F v, F32 mass)
{
  return Point3F(0,0,-datablock->gravity)*mass;
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxF_GravityDesc : public afxForceDesc
{
  static afxF_GravityDesc desc;

public:
  virtual bool testForceType(const SimDataBlock*) const;
  virtual afxForce* create() const { return new afxF_Gravity; }
};

afxF_GravityDesc afxF_GravityDesc::desc;

bool afxF_GravityDesc::testForceType(const SimDataBlock* db) const
{
  return (typeid(afxF_GravityData) == typeid(*db));
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
