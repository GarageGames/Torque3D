
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

#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/xm/afxXfmMod.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxXM_MountedImageNodeData : public afxXM_BaseData
{
  typedef afxXM_BaseData Parent;

public:
  StringTableEntry  node_name;      // name of a mounted-image node
  U32               image_slot;     // which mounted-image? 
                                    //   (0 <= image_slot < MaxMountedImages)
public:
  /*C*/         afxXM_MountedImageNodeData();
  /*C*/         afxXM_MountedImageNodeData(const afxXM_MountedImageNodeData&, bool = false);

  void          packData(BitStream* stream);
  void          unpackData(BitStream* stream);
  bool          onAdd();

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  afxXM_Base*   create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_MountedImageNodeData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

class afxXM_MountedImageNode : public afxXM_Base
{
  typedef afxXM_Base Parent;

  StringTableEntry  node_name;
  U32               image_slot;
  S32               node_ID;
  ShapeBase*        shape;
  afxConstraint*    cons;

  afxConstraint*  find_constraint();

public:
  /*C*/         afxXM_MountedImageNode(afxXM_MountedImageNodeData*, afxEffectWrapper*);

  virtual void  start(F32 timestamp);
  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_MountedImageNodeData);

ConsoleDocClass( afxXM_MountedImageNodeData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_MountedImageNodeData::afxXM_MountedImageNodeData()
{ 
  image_slot = 0;
  node_name = ST_NULLSTRING;
}

afxXM_MountedImageNodeData::afxXM_MountedImageNodeData(const afxXM_MountedImageNodeData& other, bool temp_clone) : afxXM_BaseData(other, temp_clone)
{
  image_slot = other.image_slot;
  node_name = other.node_name;
}

void afxXM_MountedImageNodeData::initPersistFields()
{
  addField("imageSlot",   TypeS32,      Offset(image_slot, afxXM_MountedImageNodeData),
    "...");
  addField("nodeName",    TypeString,   Offset(node_name, afxXM_MountedImageNodeData),
    "...");

  Parent::initPersistFields();
}

void afxXM_MountedImageNodeData::packData(BitStream* stream)
{
  Parent::packData(stream);
  stream->write(image_slot);
  stream->writeString(node_name);
}

void afxXM_MountedImageNodeData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&image_slot);
  node_name = stream->readSTString();
}

bool afxXM_MountedImageNodeData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  if (image_slot >= ShapeBase::MaxMountedImages)
  {
    // datablock will not be added if imageSlot is out-of-bounds
    Con::errorf(ConsoleLogEntry::General, 
               "afxXM_MountedImageNodeData(%s): imageSlot (%u) >= MaxMountedImages (%d)",
               getName(), 
               image_slot,
               ShapeBase::MaxMountedImages);
    return false;
  }

  return true;
}

afxXM_Base* afxXM_MountedImageNodeData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_MountedImageNodeData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_MountedImageNodeData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  return new afxXM_MountedImageNode(datablock, fx);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_MountedImageNode::afxXM_MountedImageNode(afxXM_MountedImageNodeData* db, afxEffectWrapper* fxw) 
: afxXM_Base(db, fxw)
{ 
  image_slot = db->image_slot; 
  node_name = db->node_name; 
  cons = 0;
  node_ID = -1;
  shape = 0;
}

// find the first constraint with a shape by checking pos
// then orient constraints in that order.
afxConstraint* afxXM_MountedImageNode::find_constraint()
{
  afxConstraint* cons = fx_wrapper->getOrientConstraint();
  if (cons && dynamic_cast<ShapeBase*>(cons->getSceneObject()))
    return cons;

  cons = fx_wrapper->getPosConstraint();
  if (cons && dynamic_cast<ShapeBase*>(cons->getSceneObject()))
    return cons;

  return 0;
}

void afxXM_MountedImageNode::start(F32 timestamp)
{
  // constraint won't change over the modifier's
  // lifetime so we find it here in start().
  cons = find_constraint();
  if (!cons)
    Con::errorf(ConsoleLogEntry::General, 
                "afxXM_MountedImageNode: failed to find a ShapeBase derived constraint source.");
}

void afxXM_MountedImageNode::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  if (!cons)
    return;

  // validate shape
  //   The shape must be validated in case it gets deleted
  //   of goes out scope. 
  SceneObject* scene_object = cons->getSceneObject();
  if (scene_object != (SceneObject*)shape)
  {
    shape = dynamic_cast<ShapeBase*>(scene_object);
    if (shape && node_name != ST_NULLSTRING)
    {
      node_ID = shape->getNodeIndex(image_slot, node_name);
      if (node_ID < 0)
      {
        Con::errorf(ConsoleLogEntry::General, 
               "afxXM_MountedImageNode: failed to find nodeName, \"%s\".",
               node_name);
      }
    }
    else
      node_ID = -1;
  }

  if (shape)
  {
    shape->getImageTransform(image_slot, node_ID, &params.ori);
    params.pos = params.ori.getPosition();
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//