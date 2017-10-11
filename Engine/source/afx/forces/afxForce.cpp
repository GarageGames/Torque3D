
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
#include "afx/forces/afxForce.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxForceData

afxForceData::afxForceData()
{
  force_set_name = ST_NULLSTRING;
  force_desc = 0;
}

afxForceData::afxForceData(const afxForceData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  force_set_name = other.force_set_name;
  force_desc = other.force_desc;
}

#define myOffset(field) Offset(field, afxForceData)

void afxForceData::initPersistFields()
{
  addField("forceSetName",    TypeString,   myOffset(force_set_name),
    "...");

  Parent::initPersistFields();
}

bool afxForceData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxForceData::packData(BitStream* stream)
{
	Parent::packData(stream);
  stream->writeString(force_set_name);
}

void afxForceData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  force_set_name = stream->readSTString();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxForce

afxForce::afxForce()
{
  datablock = 0;
  fade_amt = 1.0f;
}

afxForce::~afxForce()
{
}

bool afxForce::onNewDataBlock(afxForceData* dptr, bool reload)
{
  datablock = dptr;
  return true;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

Vector<afxForceDesc*>* afxForceDesc::forces = 0;

afxForceDesc::afxForceDesc() 
{ 
  if (!forces)
    forces = new Vector<afxForceDesc*>;

  forces->push_back(this);
}

bool afxForceDesc::identifyForce(afxForceData* force_data)
{
  if (!force_data)
  {
    Con::errorf("afxForceDesc::identifyForce() -- force datablock was not specified.");
    return false;
  }

  if (!forces)
  {
    Con::errorf("afxForceDesc::identifyForce() -- force registration list has not been allocated.");
    return false;
  }

  if (forces->size() == 0)
  {
    Con::errorf("afxForceDesc::identifyForce() -- no forces have been registered.");
    return false;
  }

  for (S32 i = 0; i < forces->size(); i++)
  {
    if ((*forces)[i]->testForceType(force_data))
    {
      force_data->force_desc = (*forces)[i];
      return true;
    }
  }

  Con::errorf("afxForceDesc::identifyForce() -- force %s has an undefined type. -- %d", 
    force_data, forces->size());
  return false;
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//