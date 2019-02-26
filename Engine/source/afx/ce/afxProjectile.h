
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

#ifndef _AFX_PROJECTILE_H_
#define _AFX_PROJECTILE_H_

#include "lighting/lightInfo.h"
#include "T3D/projectile.h"

#include "afx/afxEffectDefs.h"
#include "afx/afxConstraint.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxProjectileData

class afxProjectileData : public ProjectileData, public afxEffectDefs
{
  typedef ProjectileData Parent;

public:
  enum LaunchDirType {
    TowardPos2Constraint,
    OrientConstraint,
    LaunchDirField
  };

public:
  U8                networking;
  StringTableEntry  launch_pos_spec;
  afxConstraintDef  launch_pos_def;
  Point3F           launch_dir_bias;
  bool              ignore_src_timeout;
  U32               dynamicCollisionMask;
  U32               staticCollisionMask;
  bool              override_collision_masks;
  U32               launch_dir_method;

  virtual void      gather_cons_defs(Vector<afxConstraintDef>& defs);

public:
  /*C*/             afxProjectileData();
  /*C*/             afxProjectileData(const afxProjectileData&, bool = false);

  virtual bool      onAdd();
  void              packData(BitStream* stream);
  void              unpackData(BitStream* stream);

  virtual bool      allowSubstitutions() const { return true; }

  static void       initPersistFields();

  DECLARE_CONOBJECT(afxProjectileData);
  DECLARE_CATEGORY("AFX");
};

typedef afxProjectileData::LaunchDirType afxProjectile_LaunchDirType;
DefineEnumType( afxProjectile_LaunchDirType );

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxProjectile

class afxProjectile : public Projectile, public afxEffectDefs
{
  typedef Projectile Parent;

private:
  U32                 chor_id;
  bool                hookup_with_chor;
  StringTableEntry    ghost_cons_name;
  bool                client_only;

public:
  /*C*/               afxProjectile();
  /*C*/               afxProjectile(U32 networking, U32 chor_id, StringTableEntry cons_name);
  /*D*/               ~afxProjectile();

  void                init(Point3F& pos, Point3F& vel, ShapeBase* src_obj);

  virtual bool        onNewDataBlock(GameBaseData* dptr, bool reload);
  virtual void        processTick(const Move *move);
  virtual void        interpolateTick(F32 delta);
  virtual void        advanceTime(F32 dt);
  virtual bool        onAdd();
  virtual void        onRemove();
  virtual U32         packUpdate(NetConnection*, U32, BitStream*);
  virtual void        unpackUpdate(NetConnection*, BitStream*);
  virtual void        explode(const Point3F& p, const Point3F& n, const U32 collideType);

  DECLARE_CONOBJECT(afxProjectile);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_PROJECTILE_H_
