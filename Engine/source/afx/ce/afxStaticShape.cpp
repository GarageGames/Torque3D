
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

#include "ts/tsShapeInstance.h"

#include "afx/afxChoreographer.h"
#include "afx/ce/afxStaticShape.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxStaticShapeData

IMPLEMENT_CO_DATABLOCK_V1(afxStaticShapeData);

ConsoleDocClass( afxStaticShapeData,
   "@brief A datablock that specifies a StaticShape effect.\n\n"

   "afxStaticShapeData inherits from StaticShapeData and adds some AFX specific fields. StaticShape effects should be "
   "specified using afxStaticShapeData rather than StaticShapeData datablocks."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxStaticShapeData::afxStaticShapeData()
{
  sequence = ST_NULLSTRING;
  ignore_scene_amb = false; 
  use_custom_scene_amb = false;
  custom_scene_amb.set(0.5f, 0.5f, 0.5f);
  do_spawn = false;
}

afxStaticShapeData::afxStaticShapeData(const afxStaticShapeData& other, bool temp_clone) : StaticShapeData(other, temp_clone)
{
  sequence = other.sequence;
  ignore_scene_amb = other.ignore_scene_amb;
  use_custom_scene_amb = other.use_custom_scene_amb;
  custom_scene_amb = other.custom_scene_amb;
  do_spawn = other.do_spawn;
}

#define myOffset(field) Offset(field, afxStaticShapeData)

void afxStaticShapeData::initPersistFields()
{
  addField("sequence",              TypeFilename, myOffset(sequence),
    "An animation sequence in the StaticShape to play.");
  addField("ignoreSceneAmbient",    TypeBool,     myOffset(ignore_scene_amb),
    "...");
  addField("useCustomSceneAmbient", TypeBool,     myOffset(use_custom_scene_amb),
    "...");
  addField("customSceneAmbient",    TypeColorF,   myOffset(custom_scene_amb),
    "...");
  addField("doSpawn",               TypeBool,     myOffset(do_spawn),
    "When true, the StaticShape effect will leave behind the StaticShape object as a "
    "permanent part of the scene.");

  Parent::initPersistFields();
}

void afxStaticShapeData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->writeString(sequence);
  stream->writeFlag(ignore_scene_amb);
  if (stream->writeFlag(use_custom_scene_amb))
    stream->write(custom_scene_amb);
  stream->writeFlag(do_spawn);
}

void afxStaticShapeData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  sequence = stream->readSTString();
  ignore_scene_amb = stream->readFlag();
  if ((use_custom_scene_amb = stream->readFlag()) == true)
    stream->read(&custom_scene_amb);
  do_spawn = stream->readFlag();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxStaticShape

IMPLEMENT_CO_NETOBJECT_V1(afxStaticShape);

ConsoleDocClass( afxStaticShape,
   "@brief A StaticShape effect as defined by an afxStaticShapeData datablock.\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
);

afxStaticShape::afxStaticShape()
{
  afx_data = 0;
  is_visible = true;
  chor_id = 0;
  hookup_with_chor = false;
  ghost_cons_name = ST_NULLSTRING;
}

afxStaticShape::~afxStaticShape()
{
}

void afxStaticShape::init(U32 chor_id, StringTableEntry cons_name)
{
  this->chor_id = chor_id;
  ghost_cons_name = cons_name;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

bool afxStaticShape::onNewDataBlock(GameBaseData* dptr, bool reload)
{
  mDataBlock = dynamic_cast<StaticShapeData*>(dptr);
  if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
    return false;

  afx_data = dynamic_cast<afxStaticShapeData*>(mDataBlock);

  if (!mShapeInstance)
    return true;

  const char* seq_name = 0;

  // if datablock is afxStaticShapeData we get the sequence setting 
  // directly from the datablock on the client-side only
  if (afx_data)
  {
    if (isClientObject())
      seq_name = afx_data->sequence;
  }
  // otherwise datablock is stock StaticShapeData and we look for
  // a sequence name on a dynamic field on the server.
  else
  {
    if (isServerObject())
      seq_name = mDataBlock->getDataField(StringTable->insert("sequence"),0);
  } 

  // if we have a sequence name, attempt to start a thread
  if (seq_name)
  {
    TSShape* shape = mShapeInstance->getShape();
    if (shape) 
    {
      S32 seq = shape->findSequence(seq_name);
      if (seq != -1)
        setThreadSequence(0,seq);
    }
  }

  return true;
}

void afxStaticShape::advanceTime(F32 dt)
{
  Parent::advanceTime(dt);

  if (hookup_with_chor)
  {
    afxChoreographer* chor = arcaneFX::findClientChoreographer(chor_id);
    if (chor)
    {
      chor->setGhostConstraintObject(this, ghost_cons_name);
      hookup_with_chor = false;
    }
  }
}

U32 afxStaticShape::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
{
  U32 retMask = Parent::packUpdate(conn, mask, stream);

  // InitialUpdate
  if (stream->writeFlag(mask & InitialUpdateMask)) 
  {
    stream->write(chor_id);
    stream->writeString(ghost_cons_name);
  }

  return retMask;
}

//~~~~~~~~~~~~~~~~~~~~//

void afxStaticShape::unpackUpdate(NetConnection * conn, BitStream * stream)
{
  Parent::unpackUpdate(conn, stream);
  
  // InitialUpdate
  if (stream->readFlag())
  {
    stream->read(&chor_id);
    ghost_cons_name = stream->readSTString();

    if (chor_id != 0 && ghost_cons_name != ST_NULLSTRING)
      hookup_with_chor = true;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

void afxStaticShape::prepRenderImage(SceneRenderState* state)
{
  if (is_visible) 
     Parent::prepRenderImage(state);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//