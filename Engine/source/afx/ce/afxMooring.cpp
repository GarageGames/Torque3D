
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

#include "math/mathIO.h"

#include "afx/afxChoreographer.h"
#include "afx/ce/afxMooring.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxMooringData

IMPLEMENT_CO_DATABLOCK_V1(afxMooringData);

ConsoleDocClass( afxMooringData,
   "@brief A datablock that specifies a Mooring effect.\n\n"

   "A Mooring is an invisible effect object which can be positioned and oriented within a scene like other objects. Its main "
   "purpose is to serve as a common mount point for other effects within the same choreographer. Typically one uses AFX "
   "animation features to create movement for a Mooring and then other effects are bound to it using effect-to-effect "
   "constraints (#effect)."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxMooringData::afxMooringData()
{
  track_pos_only = false;
  networking = SCOPE_ALWAYS;
  display_axis_marker = false;
}

afxMooringData::afxMooringData(const afxMooringData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  track_pos_only = other.track_pos_only;
  networking = other.networking;
  display_axis_marker = other.display_axis_marker;
}

#define myOffset(field) Offset(field, afxMooringData)

void afxMooringData::initPersistFields()
{
  addField("displayAxisMarker",   TypeBool,     myOffset(display_axis_marker),
    "Specifies whether to display an axis to help visualize the position and orientation "
    "of the mooring.");
  addField("trackPosOnly",        TypeBool,     myOffset(track_pos_only),
    "This field is only meaningful for networking settings of SCOPE_ALWAYS and GHOSTABLE. "
    "In these cases, client moorings are ghosting a mooring on the server, and "
    "trackPosOnly determines if the client moorings need to be updated with the server "
    "mooring's complete transform or just its position. If only the position needs to be "
    "tracked, setting trackPosOnly to true will reduce the network traffic.");
  addField("networking",          TypeS8,       myOffset(networking),
    "Specifies the networking model used for the mooring and should be one of: "
    "$AFX::SCOPE_ALWAYS, $AFX::GHOSTABLE, $AFX::SERVER_ONLY, or $AFX::CLIENT_ONLY");

  Parent::initPersistFields();
}

bool afxMooringData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxMooringData::packData(BitStream* stream)
{
	Parent::packData(stream);
  stream->write(display_axis_marker);
  stream->write(track_pos_only);
  stream->write(networking);
}

void afxMooringData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
  stream->read(&display_axis_marker);
  stream->read(&track_pos_only);
  stream->read(&networking);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxMooring

IMPLEMENT_CO_NETOBJECT_V1(afxMooring);

ConsoleDocClass( afxMooring,
   "@brief A Mooring effect as defined by an afxMooringData datablock.\n\n"

   "A Mooring is an invisible effect object which can be positioned and oriented within "
   "a scene like other objects. Its main purpose is to serve as a common mount point for "
   "other effects within the same choreographer. Typically one uses AFX animation "
   "features to create movement for a Mooring and then other effects are bound to it "
   "using effect-to-effect constraints (#effect).\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
);

afxMooring::afxMooring()
{
  mNetFlags.set(Ghostable | ScopeAlways);

  chor_id = 0;
  hookup_with_chor = false;
  ghost_cons_name = ST_NULLSTRING;
}

afxMooring::afxMooring(U32 networking, U32 chor_id, StringTableEntry cons_name)
{
  if (networking & SCOPE_ALWAYS)
  {
    mNetFlags.clear();
    mNetFlags.set(Ghostable | ScopeAlways);
  }
  else if (networking & GHOSTABLE)
  {
    mNetFlags.clear();
    mNetFlags.set(Ghostable);
  }
  else if (networking & SERVER_ONLY)
  {
    mNetFlags.clear();
  }
  else // if (networking & CLIENT_ONLY)
  {
    mNetFlags.clear();
    mNetFlags.set(IsGhost);
  }

  this->chor_id = chor_id;
  hookup_with_chor = false;
  this->ghost_cons_name = cons_name;
}

afxMooring::~afxMooring()
{
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

bool afxMooring::onNewDataBlock(GameBaseData* dptr, bool reload)
{
  mDataBlock = dynamic_cast<afxMooringData*>(dptr);
  if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
    return false;

  return true;
}

void afxMooring::advanceTime(F32 dt)
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

  Point3F pos = getRenderPosition();
}

U32 afxMooring::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
{
  U32 retMask = Parent::packUpdate(conn, mask, stream);
  
  // InitialUpdate
  if (stream->writeFlag(mask & InitialUpdateMask)) 
  {
    stream->write(chor_id);
    stream->writeString(ghost_cons_name);
  }
  
  if (stream->writeFlag(mask & PositionMask)) 
  {
    if (mDataBlock->track_pos_only)
      mathWrite(*stream, mObjToWorld.getPosition());
    else
      stream->writeAffineTransform(mObjToWorld);
  } 
  
  return retMask;
}

//~~~~~~~~~~~~~~~~~~~~//

void afxMooring::unpackUpdate(NetConnection * conn, BitStream * stream)
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
  
  if (stream->readFlag()) 
  {
    if (mDataBlock->track_pos_only)
    {
      Point3F pos;
      mathRead(*stream, &pos);
      setPosition(pos);
    }
    else
    {
      MatrixF mat;
      stream->readAffineTransform(&mat);
      setTransform(mat);
      setRenderTransform(mat);
    }
  }
}

void afxMooring::setTransform(const MatrixF& mat)
{
   Parent::setTransform(mat);
   setMaskBits(PositionMask);
}

bool afxMooring::onAdd()
{
  if(!Parent::onAdd())
    return false;

  mObjBox = Box3F(Point3F(-0.5, -0.5, -0.5), Point3F(0.5, 0.5, 0.5));
  
  addToScene();
  
  return true;
}

void afxMooring::onRemove()
{
  removeFromScene();
  
  Parent::onRemove();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//