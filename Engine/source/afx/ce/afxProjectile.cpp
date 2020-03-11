
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

#include "T3D/shapeBase.h"

#include "afx/ce/afxProjectile.h"
#include "afx/afxChoreographer.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxProjectileData

IMPLEMENT_CO_DATABLOCK_V1(afxProjectileData);

ConsoleDocClass( afxProjectileData,
   "@brief A datablock that specifies a Projectile effect.\n\n"

   "afxProjectileData inherits from ProjectileData and adds some AFX specific fields."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxProjectileData::afxProjectileData()
{
  networking = GHOSTABLE;
  launch_pos_spec = ST_NULLSTRING;
  launch_dir_bias.zero();
  ignore_src_timeout = false;
  dynamicCollisionMask = 0;
  staticCollisionMask = 0;
  override_collision_masks = false;
  launch_dir_method = TowardPos2Constraint;
}

afxProjectileData::afxProjectileData(const afxProjectileData& other, bool temp_clone) : ProjectileData(other, temp_clone)
{
  networking = other.networking;
  launch_pos_spec = other.launch_pos_spec;
  launch_pos_def = other.launch_pos_def;
  launch_dir_bias = other.launch_dir_bias;
  ignore_src_timeout = other.ignore_src_timeout;
  dynamicCollisionMask = other.dynamicCollisionMask;
  staticCollisionMask = other.staticCollisionMask;
  override_collision_masks = other.override_collision_masks;
  launch_dir_method = other.launch_dir_method;
}

ImplementEnumType( afxProjectile_LaunchDirType, "Possible projectile launch direction types.\n" "@ingroup afxProjectile\n\n" )
   { afxProjectileData::TowardPos2Constraint,  "towardPos2Constraint",  "..." },
   { afxProjectileData::OrientConstraint,      "orientConstraint",      "..." },
   { afxProjectileData::LaunchDirField,        "launchDirField",        "..." },
EndImplementEnumType;

#define myOffset(field) Offset(field, afxProjectileData)

void afxProjectileData::initPersistFields()
{
  addField("networking",              TypeS8,       myOffset(networking),
    "...");
  addField("launchPosSpec",           TypeString,   myOffset(launch_pos_spec),
    "...");
  addField("launchDirBias",           TypePoint3F,  myOffset(launch_dir_bias),
    "...");
  addField("ignoreSourceTimeout",     TypeBool,     myOffset(ignore_src_timeout),
    "...");
  addField("dynamicCollisionMask",    TypeS32,      myOffset(dynamicCollisionMask),
    "...");
  addField("staticCollisionMask",     TypeS32,      myOffset(staticCollisionMask),
    "...");
  addField("overrideCollisionMasks",  TypeBool,     myOffset(override_collision_masks),
    "...");

  addField("launchDirMethod", TYPEID<afxProjectileData::LaunchDirType>(), myOffset(launch_dir_method),
    "Possible values: towardPos2Constraint, orientConstraint, or launchDirField.");

  Parent::initPersistFields();
}

bool afxProjectileData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  bool runs_on_s = ((networking & (SERVER_ONLY | SERVER_AND_CLIENT)) != 0);
  bool runs_on_c = ((networking & (CLIENT_ONLY | SERVER_AND_CLIENT)) != 0);
  launch_pos_def.parseSpec(launch_pos_spec, runs_on_s, runs_on_c);

  return true;
}

void afxProjectileData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->write(networking);
  stream->writeString(launch_pos_spec);
  if (stream->writeFlag(!launch_dir_bias.isZero()))
  {
    stream->write(launch_dir_bias.x);  
    stream->write(launch_dir_bias.y);  
    stream->write(launch_dir_bias.z);  
  }
  stream->writeFlag(ignore_src_timeout);
  if (stream->writeFlag(override_collision_masks))
  {
    stream->write(dynamicCollisionMask);  
    stream->write(staticCollisionMask);  
  }

  stream->writeInt(launch_dir_method, 2);
}

void afxProjectileData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&networking);
  launch_pos_spec = stream->readSTString();
  if (stream->readFlag())
  {
    stream->read(&launch_dir_bias.x);  
    stream->read(&launch_dir_bias.y);  
    stream->read(&launch_dir_bias.z);  
  }
  else
    launch_dir_bias.zero();
  ignore_src_timeout = stream->readFlag();
  if ((override_collision_masks = stream->readFlag()) == true)
  {
    stream->read(&dynamicCollisionMask);  
    stream->read(&staticCollisionMask);  
  }
  else
  {
    dynamicCollisionMask = 0;
    staticCollisionMask = 0;
  }

  launch_dir_method = (U32) stream->readInt(2);
}

void afxProjectileData::gather_cons_defs(Vector<afxConstraintDef>& defs)
{ 
  if (launch_pos_def.isDefined())
    defs.push_back(launch_pos_def);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxProjectile

IMPLEMENT_CO_NETOBJECT_V1(afxProjectile);

ConsoleDocClass( afxProjectile,
   "@brief A Projectile effect as defined by an afxProjectileData datablock.\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
);

afxProjectile::afxProjectile()
{
  chor_id = 0;
  hookup_with_chor = false;
  ghost_cons_name = ST_NULLSTRING;
  client_only = false;
}

afxProjectile::afxProjectile(U32 networking, U32 chor_id, StringTableEntry cons_name)
{
  if (networking & SCOPE_ALWAYS)
  {
    mNetFlags.clear();
    mNetFlags.set(Ghostable | ScopeAlways);
    client_only = false;
  }
  else if (networking & GHOSTABLE)
  {
    mNetFlags.clear();
    mNetFlags.set(Ghostable);
    client_only = false;
  }
  else if (networking & SERVER_ONLY)
  {
    mNetFlags.clear();
    client_only = false;
  }
  else // if (networking & CLIENT_ONLY)
  {
    mNetFlags.clear();
    mNetFlags.set(IsGhost);
    client_only = true;
  }

  this->chor_id = chor_id;
  hookup_with_chor = false;
  this->ghost_cons_name = cons_name;
}

afxProjectile::~afxProjectile()
{
}

void afxProjectile::init(Point3F& pos, Point3F& vel, ShapeBase* src_obj)
{
  mCurrPosition = pos;
  mCurrVelocity = vel;
  if (src_obj)
  {
    mSourceObject = src_obj;
    mSourceObjectId = src_obj->getId();
    mSourceObjectSlot = 0;
  }

  setPosition(mCurrPosition);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

bool afxProjectile::onNewDataBlock(GameBaseData* dptr, bool reload)
{
  return Parent::onNewDataBlock(dptr, reload);
}

void afxProjectile::processTick(const Move* move)
{
  // note: this deletion test must occur before calling to the parent's
  // processTick() because if this is a server projectile, the parent
  // might decide to delete it and then client_only will no longer be
  // valid after the return.
  bool do_delete = (client_only && mCurrTick >= mDataBlock->lifetime);

  Parent::processTick(move);

  if (do_delete)
    deleteObject();
}

void afxProjectile::interpolateTick(F32 delta)
{
  if (client_only)
    return;

  Parent::interpolateTick(delta);
}

void afxProjectile::advanceTime(F32 dt)
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

bool afxProjectile::onAdd()
{
  if(!Parent::onAdd())
    return false;

  if (isClientObject())
  {
    // add to client side mission cleanup
    SimGroup *cleanup = dynamic_cast<SimGroup *>( Sim::findObject( "ClientMissionCleanup") );
    if( cleanup != NULL )
    {
      cleanup->addObject( this );
    }
    else
    {
      AssertFatal( false, "Error, could not find ClientMissionCleanup group" );
      return false;
    }
  }

  return true;
}

void afxProjectile::onRemove()
{  
  Parent::onRemove();
}

//~~~~~~~~~~~~~~~~~~~~

U32 afxProjectile::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
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

void afxProjectile::unpackUpdate(NetConnection * conn, BitStream * stream)
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

class afxProjectileDeleteEvent : public SimEvent
{
public:
   void process(SimObject *object)
   {
      object->deleteObject();
   }
};

void afxProjectile::explode(const Point3F& p, const Point3F& n, const U32 collideType)
{
  // Make sure we don't explode twice...
  if ( isHidden() )
    return;

  Parent::explode(p, n, collideType);

  if (isClientObject() && client_only) 
    Sim::postEvent(this, new afxProjectileDeleteEvent, Sim::getCurrentTime() + DeleteWaitTime);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//