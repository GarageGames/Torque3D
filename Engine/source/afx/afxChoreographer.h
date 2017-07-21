
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

#ifndef _AFX_CHOREOGRAPHER_H_
#define _AFX_CHOREOGRAPHER_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afxEffectDefs.h"
#include "afxEffectWrapper.h"
#include "afxMagicMissile.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxChoreographerData

class afxChoreographerData : public GameBaseData, public afxEffectDefs
{
  typedef GameBaseData  Parent;

public:
  bool              exec_on_new_clients;
  U8                echo_packet_usage;
  StringTableEntry  client_script_file;
  StringTableEntry  client_init_func;

public:
  /*C*/         afxChoreographerData();
  /*C*/         afxChoreographerData(const afxChoreographerData&, bool = false);

  virtual void  packData(BitStream*);
  virtual void  unpackData(BitStream*);

  bool          preload(bool server, String &errorStr);

  static void   initPersistFields();

  DECLARE_CONOBJECT(afxChoreographerData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxChoreographer

class afxConstraint;
class afxConstraintMgr;
class afxEffectWrapper;
class afxParticlePool;
class afxParticlePoolData;
class SimSet;
class afxForceSetMgr;

class afxChoreographer : public GameBase, public afxEffectDefs, public afxMagicMissileCallback
{
  typedef GameBase  Parent;

public:
  enum MaskBits 
  {
    TriggerMask         = Parent::NextFreeMask << 0,
    RemapConstraintMask = Parent::NextFreeMask << 1, // CONSTRAINT REMAPPING
    NextFreeMask        = Parent::NextFreeMask << 2
  };

  enum 
  { 
    USER_EXEC_CONDS_MASK = 0x00ffffff
  };

protected:
  struct dynConstraintDef
  {
    StringTableEntry  cons_name;
    U8                cons_type;
    union
    {
      SceneObject* object;
      Point3F*     point;
      MatrixF*     xfm;
      U16          scope_id;
    } cons_obj;
  };

private:
  afxChoreographerData*    datablock;
  SimSet                   named_effects;
  SimObject*               exeblock;
  afxForceSetMgr*          force_set_mgr;
  Vector<afxParticlePool*> particle_pools;
  Vector<dynConstraintDef> dc_defs_a;
  Vector<dynConstraintDef> dc_defs_b;
  GameBase*                proc_after_obj;
  U32                      trigger_mask;

protected:
  Vector<dynConstraintDef>* dyn_cons_defs;
  Vector<dynConstraintDef>* dyn_cons_defs2;
  afxConstraintMgr* constraint_mgr;
  U32               choreographer_id;
  U8                ranking;
  U8                lod;
  U32               exec_conds_mask;
  SimObject*        extra;
  Vector<NetConnection*> explicit_clients;
  bool              started_with_newop;
  bool              postpone_activation;

  virtual void      pack_constraint_info(NetConnection* conn, BitStream* stream);
  virtual void      unpack_constraint_info(NetConnection* conn, BitStream* stream);
  void              setup_dynamic_constraints();
  void              check_packet_usage(NetConnection*, BitStream*, S32 mark_stream_pos, const char* msg_tag);
  SceneObject*      get_camera(Point3F* cam_pos=0) const;

public:
  /*C*/             afxChoreographer();
  virtual           ~afxChoreographer();

  static void       initPersistFields();

  virtual bool      onAdd();
  virtual void      onRemove();
  virtual void      onDeleteNotify(SimObject*);
  virtual bool      onNewDataBlock(GameBaseData* dptr, bool reload);
  virtual U32       packUpdate(NetConnection*, U32, BitStream*);
  virtual void      unpackUpdate(NetConnection*, BitStream*);

  virtual void      sync_with_clients() { }
  
  afxConstraintMgr* getConstraintMgr() { return constraint_mgr; }
  afxForceSetMgr*   getForceSetMgr() { return force_set_mgr; }

  afxParticlePool*  findParticlePool(afxParticlePoolData* key_block, U32 key_index);
  void              registerParticlePool(afxParticlePool*);
  void              unregisterParticlePool(afxParticlePool*);

  void              setRanking(U8 value) { ranking = value; }
  U8                getRanking() const { return ranking; }
  bool              testRanking(U8 low, U8 high) { return (ranking <= high && ranking >= low); }
  void              setLevelOfDetail(U8 value) { lod = value; }
  U8                getLevelOfDetail() const { return lod; }
  bool              testLevelOfDetail(U8 low, U8 high) { return (lod <= high && lod >= low); }
  void              setExecConditions(U32 mask) { exec_conds_mask = mask; }
  U32               getExecConditions() const { return exec_conds_mask; }

  virtual void      executeScriptEvent(const char* method, afxConstraint*, 
                                       const MatrixF& xfm, const char* data);

  virtual void      inflictDamage(const char * label, const char* flavor, SimObjectId target,
                                  F32 amt, U8 count, F32 ad_amt, F32 rad, Point3F pos, F32 imp) { }

  void              addObjectConstraint(SceneObject*, const char* cons_name);
  void              addObjectConstraint(U16 scope_id, const char* cons_name, bool is_shape);
  void              addPointConstraint(Point3F&, const char* cons_name);
  void              addTransformConstraint(MatrixF&, const char* cons_name);
  bool              addConstraint(const char* source_spec, const char* cons_name);

  void              addNamedEffect(afxEffectWrapper*);
  void              removeNamedEffect(afxEffectWrapper*);
  afxEffectWrapper* findNamedEffect(StringTableEntry);

  void              clearChoreographerId() { choreographer_id = 0; }
  U32               getChoreographerId() { return choreographer_id; }
  void              setGhostConstraintObject(SceneObject*, StringTableEntry cons_name);
  void              setExtra(SimObject* extra) { this->extra = extra; }
  void              addExplicitClient(NetConnection* conn);
  void              removeExplicitClient(NetConnection* conn);
  U32               getExplicitClientCount() { return explicit_clients.size(); }

  void              restoreScopedObject(SceneObject* obj);
  virtual void      restoreObject(SceneObject*) { };

  void              postProcessAfterObject(GameBase* obj);
  U32               getTriggerMask() const { return trigger_mask; }
  void              setTriggerMask(U32 trigger_mask);

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//
// missile watcher callbacks
public:
  virtual void  impactNotify(const Point3F& p, const Point3F& n, SceneObject*) { }

  DECLARE_CONOBJECT(afxChoreographer);
  DECLARE_CATEGORY("AFX");
  
  // CONSTRAINT REMAPPING <<
protected:
  Vector<dynConstraintDef*> remapped_cons_defs;
  bool              remapped_cons_sent;
  virtual bool      remap_builtin_constraint(SceneObject*, const char* cons_name) { return false; }     
  dynConstraintDef* find_cons_def_by_name(const char* cons_name);
public:
  void              remapObjectConstraint(SceneObject*, const char* cons_name);
  void              remapObjectConstraint(U16 scope_id, const char* cons_name, bool is_shape);
  void              remapPointConstraint(Point3F&, const char* cons_name);
  void              remapTransformConstraint(MatrixF&, const char* cons_name);
  bool              remapConstraint(const char* source_spec, const char* cons_name);
  // CONSTRAINT REMAPPING >> 
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_CHOREOGRAPHER_H_
