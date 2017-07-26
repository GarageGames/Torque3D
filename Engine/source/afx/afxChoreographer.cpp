
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

#include "console/engineAPI.h"
#include "T3D/gameBase/gameConnection.h"
#include "math/mathIO.h"
#include "console/compiler.h"

#include "afx/afxConstraint.h"
#include "afx/afxChoreographer.h"
#include "afx/afxEffectWrapper.h"
#include "afx/util/afxParticlePool.h"
#include "afx/forces/afxForceSet.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxChoreographerData);

ConsoleDocClass( afxChoreographerData,
   "@brief Datablock base class used by choreographers.\n\n"

   "@ingroup afxChoreographers\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxChoreographerData::afxChoreographerData()
{
  exec_on_new_clients = false;
  echo_packet_usage = 100;
  client_script_file = ST_NULLSTRING;
  client_init_func = ST_NULLSTRING;
}

afxChoreographerData::afxChoreographerData(const afxChoreographerData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  exec_on_new_clients = other.exec_on_new_clients;
  echo_packet_usage = other.echo_packet_usage;
  client_script_file = other.client_script_file;
  client_init_func = other.client_init_func;
}

#define myOffset(field) Offset(field, afxChoreographerData)

void afxChoreographerData::initPersistFields()
{
  addField("execOnNewClients",    TypeBool,       myOffset(exec_on_new_clients),
    "...");
  addField("echoPacketUsage",     TypeS8,         myOffset(echo_packet_usage),
    "...");
  addField("clientScriptFile",    TypeFilename,   myOffset(client_script_file),
    "...");
  addField("clientInitFunction",  TypeString,     myOffset(client_init_func),
    "...");

  Parent::initPersistFields();
}

void afxChoreographerData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->write(exec_on_new_clients);
  stream->write(echo_packet_usage);
  stream->writeString(client_script_file);
  stream->writeString(client_init_func);
}

void afxChoreographerData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&exec_on_new_clients);
  stream->read(&echo_packet_usage);
  client_script_file = stream->readSTString();
  client_init_func = stream->readSTString();
}

bool afxChoreographerData::preload(bool server, String &errorStr)
{
  if (!Parent::preload(server, errorStr))
    return false;

  if (!server && client_script_file != ST_NULLSTRING)
  {
    Compiler::gSyntaxError = false;
    Con::evaluate(avar("exec(\"%s\");", client_script_file), false, 0);
    if (Compiler::gSyntaxError)
    {
      Con::errorf("afxChoreographerData: failed to exec clientScriptFile \"%s\" -- syntax error", client_script_file);
      Compiler::gSyntaxError = false;
    }
  }

  return true;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxChoreographer);

ConsoleDocClass( afxChoreographer,
   "@brief Base class used by choreographers.\n\n"

   "@ingroup afxChoreographers\n"
   "@ingroup AFX\n"
);

afxChoreographer::afxChoreographer()
{
  datablock = 0;  // datablock initializer (union these?)
  exeblock = 0;   // object to use in executef callouts

  // create the constraint manager
  constraint_mgr = new afxConstraintMgr();

  ranking = 0;
  lod = 0;
  exec_conds_mask = 0;
  choreographer_id = 0;
  extra = 0;
  started_with_newop = false;
  postpone_activation = false;
  remapped_cons_sent = false; // CONSTRAINT REMAPPING

  dyn_cons_defs = &dc_defs_a;
  dyn_cons_defs2 = &dc_defs_b;

  proc_after_obj = 0;
  trigger_mask = 0;

  force_set_mgr = 0;

  mOrderGUID = 0x0FFFFFFF;
}

afxChoreographer::~afxChoreographer()
{
  explicit_clients.clear();

  for (S32 i = 0; i < dyn_cons_defs->size(); i++)
  {
    if ((*dyn_cons_defs)[i].cons_type == POINT_CONSTRAINT && (*dyn_cons_defs)[i].cons_obj.point != NULL)
      delete (*dyn_cons_defs)[i].cons_obj.point;
    else if ((*dyn_cons_defs)[i].cons_type == TRANSFORM_CONSTRAINT && (*dyn_cons_defs)[i].cons_obj.xfm != NULL)
      delete (*dyn_cons_defs)[i].cons_obj.xfm;
  }

  constraint_mgr->clearAllScopeableObjs();
  delete constraint_mgr;

  delete force_set_mgr;
}

void afxChoreographer::initPersistFields()
{
  // conditionals
  addField("extra",              TYPEID<SimObject>(), Offset(extra, afxChoreographer),
    "...");
  addField("postponeActivation", TypeBool,            Offset(postpone_activation, afxChoreographer),
    "...");

  Parent::initPersistFields();
}

bool afxChoreographer::onAdd()
{
  if (!Parent::onAdd()) 
    return(false);

  if (isServerObject())
    choreographer_id = arcaneFX::registerChoreographer(this);
  else
  {
    if (proc_after_obj)
    {
      processAfter(proc_after_obj);
      proc_after_obj = 0;
    }

    force_set_mgr = new afxForceSetMgr();

    if (datablock && datablock->client_init_func != ST_NULLSTRING)
      Con::executef(datablock->client_init_func, getIdString());
  }

  return(true);
}

void afxChoreographer::onRemove()
{
  for (S32 i = 0; i < particle_pools.size(); i++)
    if (particle_pools[i])
      particle_pools[i]->setChoreographer(0);
  particle_pools.clear();

  if (isServerObject())
    arcaneFX::unregisterChoreographer(this);
  else
    arcaneFX::unregisterClientChoreographer(this);

  choreographer_id = 0;
  Parent::onRemove();
}

void afxChoreographer::onDeleteNotify(SimObject* obj)
{
  if (dynamic_cast<SceneObject*>(obj))
  { 
    SceneObject* scn_obj = (SceneObject*)(obj);
    for (S32 i = 0; i < dyn_cons_defs->size(); i++)
      if ((*dyn_cons_defs)[i].cons_type != OBJECT_CONSTRAINT || (*dyn_cons_defs)[i].cons_obj.object != scn_obj)
        dyn_cons_defs2->push_back((*dyn_cons_defs)[i]);

    Vector<dynConstraintDef>* tmp = dyn_cons_defs;
    dyn_cons_defs = dyn_cons_defs2;
    dyn_cons_defs2 = tmp;
    dyn_cons_defs2->clear();

    if (isServerObject() && scn_obj->isScopeable())
        constraint_mgr->removeScopeableObject(scn_obj);
  }
  else if (dynamic_cast<NetConnection*>(obj))
  {
    removeExplicitClient((NetConnection*)obj);
  }

  Parent::onDeleteNotify(obj);
}

bool afxChoreographer::onNewDataBlock(GameBaseData* dptr, bool reload)
{
  datablock = dynamic_cast<afxChoreographerData*>(dptr);
  if (!datablock || !Parent::onNewDataBlock(dptr, reload))
    return false;

  exeblock = datablock;

  return true;
}

void afxChoreographer::pack_constraint_info(NetConnection* conn, BitStream* stream)
{
  stream->write(dyn_cons_defs->size());
  for (S32 i = 0; i < dyn_cons_defs->size(); i++)
  {
    if ((*dyn_cons_defs)[i].cons_type == OBJECT_CONSTRAINT && (*dyn_cons_defs)[i].cons_obj.object != NULL)
    {
      stream->writeString((*dyn_cons_defs)[i].cons_name);
      stream->write((*dyn_cons_defs)[i].cons_type);
      SceneObject* object = (*dyn_cons_defs)[i].cons_obj.object;
      S32 ghost_idx = conn->getGhostIndex(object); 
      if (stream->writeFlag(ghost_idx != -1))
      {
        stream->writeRangedU32(U32(ghost_idx), 0, NetConnection::MaxGhostCount);
      }
      else
      {
        if (stream->writeFlag(object->getScopeId() > 0))
        {
          stream->writeInt(object->getScopeId(), NetObject::SCOPE_ID_BITS);
          stream->writeFlag(dynamic_cast<ShapeBase*>(object) != NULL);
        }
      }
    }
    else if ((*dyn_cons_defs)[i].cons_type == POINT_CONSTRAINT && (*dyn_cons_defs)[i].cons_obj.point != NULL)
    {
      stream->writeString((*dyn_cons_defs)[i].cons_name);
      stream->write((*dyn_cons_defs)[i].cons_type);
      mathWrite(*stream, *(*dyn_cons_defs)[i].cons_obj.point);
    }
    else if ((*dyn_cons_defs)[i].cons_type == TRANSFORM_CONSTRAINT && (*dyn_cons_defs)[i].cons_obj.xfm != NULL)
    {
      stream->writeString((*dyn_cons_defs)[i].cons_name);
      stream->write((*dyn_cons_defs)[i].cons_type);
      mathWrite(*stream, *(*dyn_cons_defs)[i].cons_obj.xfm);
    }
  }
      
  constraint_mgr->packConstraintNames(conn, stream);
}

void afxChoreographer::unpack_constraint_info(NetConnection* conn, BitStream* stream)
{
  S32 n_defs;
  stream->read(&n_defs);
  dyn_cons_defs->clear();
  for (S32 i = 0; i < n_defs; i++)
  {
    StringTableEntry cons_name = stream->readSTString();
    U8 cons_type; stream->read(&cons_type);
    if (cons_type == OBJECT_CONSTRAINT)
    {
      SceneObject* scn_obj = NULL;
      if (stream->readFlag())
      {
        S32 ghost_idx = stream->readRangedU32(0, NetConnection::MaxGhostCount);
        scn_obj = dynamic_cast<SceneObject*>(conn->resolveGhost(ghost_idx));
        if (scn_obj)
        {
          addObjectConstraint(scn_obj, cons_name); 
        }
        else
          Con::errorf("CANNOT RESOLVE GHOST %d %s", ghost_idx, cons_name);
      }
      else
      {
        if (stream->readFlag())
        {
          U16 scope_id = stream->readInt(NetObject::SCOPE_ID_BITS);
          bool is_shape = stream->readFlag();
          addObjectConstraint(scope_id, cons_name, is_shape);                                                                   
        }
      }
    }
    else if (cons_type == POINT_CONSTRAINT)
    {
      Point3F point;
      mathRead(*stream, &point);
      addPointConstraint(point, cons_name);
    }    
    else if (cons_type == TRANSFORM_CONSTRAINT)
    {
      MatrixF xfm;
      mathRead(*stream, &xfm);
      addTransformConstraint(xfm, cons_name);
    }    
  }
  
  constraint_mgr->unpackConstraintNames(stream);
}

void afxChoreographer::setup_dynamic_constraints()
{
  for (S32 i = 0; i < dyn_cons_defs->size(); i++)
  {
    switch ((*dyn_cons_defs)[i].cons_type)
    {
    case OBJECT_CONSTRAINT:
      constraint_mgr->setReferenceObject((*dyn_cons_defs)[i].cons_name, (*dyn_cons_defs)[i].cons_obj.object);
      break;
    case POINT_CONSTRAINT:
      constraint_mgr->setReferencePoint((*dyn_cons_defs)[i].cons_name, *(*dyn_cons_defs)[i].cons_obj.point);
      break;
    case TRANSFORM_CONSTRAINT:
      constraint_mgr->setReferenceTransform((*dyn_cons_defs)[i].cons_name, *(*dyn_cons_defs)[i].cons_obj.xfm);
      break;
    case OBJECT_CONSTRAINT_SANS_OBJ:
      constraint_mgr->setReferenceObjectByScopeId((*dyn_cons_defs)[i].cons_name, (*dyn_cons_defs)[i].cons_obj.scope_id, false);
      break;
    case OBJECT_CONSTRAINT_SANS_SHAPE:
      constraint_mgr->setReferenceObjectByScopeId((*dyn_cons_defs)[i].cons_name, (*dyn_cons_defs)[i].cons_obj.scope_id, true);
      break;
    }
  }
}

afxChoreographer::dynConstraintDef* afxChoreographer::find_cons_def_by_name(const char* cons_name)
{
  StringTableEntry cons_name_ste = StringTable->insert(cons_name);

  for (S32 i = 0; i < dyn_cons_defs->size(); i++)
  {
    if ((*dyn_cons_defs)[i].cons_name == cons_name_ste)
      return &((*dyn_cons_defs)[i]);
  }

  return 0;
}

void afxChoreographer::check_packet_usage(NetConnection* conn, BitStream* stream, S32 mark_stream_pos, const char* msg_tag)
{

  S32 packed_size = stream->getCurPos() - mark_stream_pos;
  S32 current_headroom = stream->getMaxWriteBitNum()-(stream->getStreamSize()<<3);
  S32 max_headroom = stream->getMaxWriteBitNum()-(conn->getMaxRatePacketSize()<<3);

  if (packed_size > max_headroom)
  {
    Con::errorf("%s [%s] WARNING -- packed-bits (%d) > limit (%d) for max PacketSize settings. [%s]",
                msg_tag, datablock->getName(), packed_size, max_headroom, "PACKET OVERRUN POSSIBLE");
    Con::errorf("getMaxRatePacketSize()=%d getCurRatePacketSize()=%d", conn->getMaxRatePacketSize(), conn->getCurRatePacketSize());
  }
  // JTF Note: this current_headroom > 0 check is odd and occurs when object is created real early
  // in the startup, such as when the dead orc gets fatal damage and we try to post a text effect.
  else if (packed_size > current_headroom && current_headroom > 0)
  {
    Con::errorf("%s [%s] WARNING -- packed-bits (%d) > limit (%d) for current PacketSize settings. [%s]",
                msg_tag, datablock->getName(), packed_size, current_headroom, "PACKET OVERRUN POSSIBLE");
  }
  else
  {
    F32 percentage = 100.0f*((F32)packed_size/(F32)max_headroom);
    if (percentage >= datablock->echo_packet_usage)
    {
        Con::warnf("%s [%s] -- packed-bits (%d) < limit (%d). [%.1f%% full]", msg_tag, datablock->getName(),
                  packed_size, max_headroom, percentage);
    }
  }
}

SceneObject* afxChoreographer::get_camera(Point3F* cam_pos) const
{
  GameConnection* conn = GameConnection::getConnectionToServer();
  if (!conn)
  {
    if (cam_pos)
      cam_pos->zero();
    return 0;
  }

  SceneObject* cam_obj = conn->getCameraObject();
  if (cam_pos)
  {
    if (cam_obj)
    {
      MatrixF cam_mtx;
      conn->getControlCameraTransform(0, &cam_mtx);
      cam_mtx.getColumn(3, cam_pos);
    }
    else
      cam_pos->zero();
  }

  return cam_obj;
}

U32 afxChoreographer::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
{
  U32 retMask = Parent::packUpdate(conn, mask, stream);
  
  if (stream->writeFlag(mask & InitialUpdateMask))      //-- INITIAL UPDATE ?
  {
    stream->write(ranking);
    stream->write(lod);
    stream->write(exec_conds_mask);
    stream->write(choreographer_id);

    // write dynamic fields beginning with arcaneFX::sParameterFieldPrefix
    SimFieldDictionaryIterator itr(getFieldDictionary());
    SimFieldDictionary::Entry* entry;
    U32 prefix_len = dStrlen(arcaneFX::sParameterFieldPrefix);
    if (prefix_len > 0)
    {
      while ((entry = *itr) != NULL)
      {
        if (dStrncmp(entry->slotName, arcaneFX::sParameterFieldPrefix, prefix_len) == 0)
        {
          stream->writeFlag(true);
          stream->writeString(entry->slotName);
          stream->writeLongString(1023, entry->value);
        }
        ++itr;
      }
    }
    stream->writeFlag(false);
  }

  if (stream->writeFlag(mask & TriggerMask))
  {
    stream->write(trigger_mask);
  }

  // CONSTRAINT REMAPPING <<
  if (stream->writeFlag((mask & RemapConstraintMask) && !remapped_cons_defs.empty()))
  {
    remapped_cons_sent = true;
    //Con::errorf("PACKING CONS REMAP %d conn:%d", remapped_cons_defs.size(), (U32) conn);

    stream->write(remapped_cons_defs.size());
    for (S32 i = 0; i < remapped_cons_defs.size(); i++)
    {
      if (remapped_cons_defs[i]->cons_type == OBJECT_CONSTRAINT && remapped_cons_defs[i]->cons_obj.object != NULL)
      {
        //Con::errorf("PACKING CONS REMAP: name %s", remapped_cons_defs[i]->cons_name);
        stream->writeString(remapped_cons_defs[i]->cons_name);
        //Con::errorf("PACKING CONS REMAP: type %d", remapped_cons_defs[i]->cons_type);
        stream->write(remapped_cons_defs[i]->cons_type);
        SceneObject* object = remapped_cons_defs[i]->cons_obj.object;
        S32 ghost_idx = conn->getGhostIndex(object);
        //Con::errorf("PACKING CONS REMAP: ghost %d", ghost_idx);
        if (stream->writeFlag(ghost_idx != -1))
        {
          stream->writeRangedU32(U32(ghost_idx), 0, NetConnection::MaxGhostCount);
        }
        else
        {
          if (stream->writeFlag(object->getScopeId() > 0))
          {
            stream->writeInt(object->getScopeId(), NetObject::SCOPE_ID_BITS);
            stream->writeFlag(dynamic_cast<ShapeBase*>(object) != NULL);
          }
        }
      }
      else if (remapped_cons_defs[i]->cons_type == POINT_CONSTRAINT && remapped_cons_defs[i]->cons_obj.point != NULL)
      {
        stream->writeString(remapped_cons_defs[i]->cons_name);
        stream->write(remapped_cons_defs[i]->cons_type);
        mathWrite(*stream, *remapped_cons_defs[i]->cons_obj.point);
      }
      else if (remapped_cons_defs[i]->cons_type == TRANSFORM_CONSTRAINT && remapped_cons_defs[i]->cons_obj.xfm != NULL)
      {
        stream->writeString(remapped_cons_defs[i]->cons_name);
        stream->write(remapped_cons_defs[i]->cons_type);
        mathWrite(*stream, *remapped_cons_defs[i]->cons_obj.xfm);
      }
    }
  }
  // CONSTRAINT REMAPPING >>

  AssertISV(stream->isValid(), "afxChoreographer::packUpdate(): write failure occurred, possibly caused by packet-size overrun."); 

  return retMask;
}

void afxChoreographer::unpackUpdate(NetConnection * conn, BitStream * stream)
{
  Parent::unpackUpdate(conn, stream);
  
  // InitialUpdate Only
  if (stream->readFlag())
  {
    stream->read(&ranking);
    stream->read(&lod);
    stream->read(&exec_conds_mask);
    stream->read(&choreographer_id);

    // read dynamic fields
    while(stream->readFlag())
    {
      char slotName[256];
      char value[1024];
      stream->readString(slotName);
      stream->readLongString(1023, value);
      setDataField(StringTable->insert(slotName), 0, value);
    }

    arcaneFX::registerClientChoreographer(this);
  }

  if (stream->readFlag()) // TriggerMask
  {
    stream->read(&trigger_mask);
  }

  // CONSTRAINT REMAPPING <<
  if (stream->readFlag()) // RemapConstraintMask
  {
    S32 n_defs;
    stream->read(&n_defs);
    for (S32 i = 0; i < n_defs; i++)
    {
      StringTableEntry cons_name = stream->readSTString();
      U8 cons_type; stream->read(&cons_type);
      if (cons_type == OBJECT_CONSTRAINT)
      {
        SceneObject* scn_obj = NULL;
        if (stream->readFlag())
        {
          S32 ghost_idx = stream->readRangedU32(0, NetConnection::MaxGhostCount);
          scn_obj = dynamic_cast<SceneObject*>(conn->resolveGhost(ghost_idx));
          if (scn_obj)
          {
            remapObjectConstraint(scn_obj, cons_name); 
          }
          else
            Con::errorf("CANNOT RESOLVE GHOST %d %s", ghost_idx, cons_name);
        }
        else
        {
          if (stream->readFlag())
          {
            U16 scope_id = stream->readInt(NetObject::SCOPE_ID_BITS);
            bool is_shape = stream->readFlag();
            remapObjectConstraint(scope_id, cons_name, is_shape);
          }
        }
      }
      else if (cons_type == POINT_CONSTRAINT)
      {
        Point3F point;
        mathRead(*stream, &point);
        remapPointConstraint(point, cons_name);
      }    
      else if (cons_type == TRANSFORM_CONSTRAINT)
      {
        MatrixF xfm;
        mathRead(*stream, &xfm);
        remapTransformConstraint(xfm, cons_name);
      }    
    }
  }
  // CONSTRAINT REMAPPING >>
}

void afxChoreographer::executeScriptEvent(const char* method, afxConstraint* cons, 
                                          const MatrixF& xfm, const char* data)
{
  SceneObject* cons_obj = (cons) ? cons->getSceneObject() : NULL;

  char *arg_buf = Con::getArgBuffer(256);
  Point3F pos;
  xfm.getColumn(3,&pos);
  AngAxisF aa(xfm);
  dSprintf(arg_buf,256,"%g %g %g %g %g %g %g",
           pos.x, pos.y, pos.z,
           aa.axis.x, aa.axis.y, aa.axis.z, aa.angle);

  // CALL SCRIPT afxChoreographerData::method(%choreographer, %constraint, %transform, %data)
  Con::executef(exeblock, method, 
                getIdString(),
                (cons_obj) ? cons_obj->getIdString() : "",
                arg_buf,
                data);
}

void afxChoreographer::addObjectConstraint(SceneObject* object, const char* cons_name)
{
  if (!object || !cons_name)
    return;

  dynConstraintDef dyn_def;
  dyn_def.cons_name = StringTable->insert(cons_name);
  dyn_def.cons_type = OBJECT_CONSTRAINT;
  dyn_def.cons_obj.object = object;
  dyn_cons_defs->push_back(dyn_def);

#if defined(AFX_CAP_SCOPE_TRACKING)
  if (isServerObject() && object->isScopeable())
    constraint_mgr->addScopeableObject(object);
#endif

  constraint_mgr->defineConstraint(OBJECT_CONSTRAINT, dyn_def.cons_name);
  deleteNotify(object);
}

void afxChoreographer::addObjectConstraint(U16 scope_id, const char* cons_name, bool is_shape)
{
  if (!cons_name)
    return;

  dynConstraintDef dyn_def;
  dyn_def.cons_name = StringTable->insert(cons_name);
  dyn_def.cons_type = (is_shape) ? OBJECT_CONSTRAINT_SANS_SHAPE : OBJECT_CONSTRAINT_SANS_OBJ;
  dyn_def.cons_obj.scope_id = scope_id;
  dyn_cons_defs->push_back(dyn_def);

  constraint_mgr->defineConstraint(OBJECT_CONSTRAINT, dyn_def.cons_name);
}

void afxChoreographer::addPointConstraint(Point3F& point, const char* cons_name)
{
  dynConstraintDef dyn_def;
  dyn_def.cons_name = StringTable->insert(cons_name);
  dyn_def.cons_type = POINT_CONSTRAINT;
  dyn_def.cons_obj.point = new Point3F(point);
  dyn_cons_defs->push_back(dyn_def);

  constraint_mgr->defineConstraint(POINT_CONSTRAINT, dyn_def.cons_name);
}

void afxChoreographer::addTransformConstraint(MatrixF& xfm, const char* cons_name)
{
  dynConstraintDef dyn_def;
  dyn_def.cons_name = StringTable->insert(cons_name);
  dyn_def.cons_type = TRANSFORM_CONSTRAINT;
  dyn_def.cons_obj.xfm = new MatrixF(xfm);
  dyn_cons_defs->push_back(dyn_def);

  constraint_mgr->defineConstraint(TRANSFORM_CONSTRAINT, dyn_def.cons_name);
}

// CONSTRAINT REMAPPING <<
static inline U32 resolve_cons_spec(const char* source_spec, Point3F& pos, MatrixF& xfm, SceneObject** scn_obj)
{
  AngAxisF aa;

  S32 args_n = dSscanf(source_spec, "%g %g %g %g %g %g %g", 
                       &pos.x, &pos.y, &pos.z,
                       &aa.axis.x, &aa.axis.y, &aa.axis.z, &aa.angle);

  // TRANSFORM CONSTRAINT SRC
  if (args_n == 7)
  {
     aa.setMatrix(&xfm);
     xfm.setColumn(3,pos);
     return afxEffectDefs::TRANSFORM_CONSTRAINT;
  }

  // POINT CONSTRAINT SRC 
  if (args_n == 3)
  {
    return afxEffectDefs::POINT_CONSTRAINT;
  }

  SimObject* cons_sim_obj = Sim::findObject(source_spec);
  *scn_obj = dynamic_cast<SceneObject*>(cons_sim_obj);
  if (*scn_obj)
  {
    return afxEffectDefs::OBJECT_CONSTRAINT;
  }

  return afxEffectDefs::UNDEFINED_CONSTRAINT_TYPE;
}
// CONSTRAINT REMAPPING >>

bool afxChoreographer::addConstraint(const char* source_spec, const char* cons_name)
{
  VectorF pos;
  MatrixF xfm;
  SceneObject* scn_obj;

  switch (resolve_cons_spec(source_spec, pos, xfm, &scn_obj))
  {
  case TRANSFORM_CONSTRAINT:
    addTransformConstraint(xfm, cons_name);
    return true;
  case POINT_CONSTRAINT:
    addPointConstraint(pos, cons_name);
    return true;
  case OBJECT_CONSTRAINT:
    addObjectConstraint(scn_obj, cons_name);
    return true;
  }

  return false;
}

void afxChoreographer::addNamedEffect(afxEffectWrapper* ew)
{
  named_effects.addObject(ew);
}

void afxChoreographer::removeNamedEffect(afxEffectWrapper* ew)
{
  named_effects.removeObject(ew);
}

afxEffectWrapper* afxChoreographer::findNamedEffect(StringTableEntry name)
{
  return (afxEffectWrapper*) named_effects.findObject(name);
}

void afxChoreographer::setGhostConstraintObject(SceneObject* obj, StringTableEntry cons_name)
{
  if (constraint_mgr)
    constraint_mgr->setReferenceObject(cons_name, obj);
}

void afxChoreographer::restoreScopedObject(SceneObject* obj)
{
  constraint_mgr->restoreScopedObject(obj, this);
  constraint_mgr->adjustProcessOrdering(this);
}

void afxChoreographer::addExplicitClient(NetConnection* conn) 
{ 
  if (!conn)
    return;

  for (S32 i = 0; i < explicit_clients.size(); i++)
  {
    if (explicit_clients[i] == conn)
      return;
  }

  explicit_clients.push_back(conn);
  deleteNotify(conn);
}

void afxChoreographer::removeExplicitClient(NetConnection* conn) 
{ 
  if (!conn)
    return;

  for (S32 i = 0; i < explicit_clients.size(); i++)
  {
    if (explicit_clients[i] == conn)
    {
      clearNotify(conn);
      explicit_clients.erase_fast(i);
      return;
    }
  }
}

void afxChoreographer::postProcessAfterObject(GameBase* obj)
{
  proc_after_obj = obj;
}

void afxChoreographer::setTriggerMask(U32 mask)
{
  if (mask != trigger_mask)
  {
    trigger_mask = mask;
    setMaskBits(TriggerMask);
  }
}

afxParticlePool* afxChoreographer::findParticlePool(afxParticlePoolData* key_block, U32 key_index) 
{ 
  for (S32 i = 0; i < particle_pools.size(); i++)
    if (particle_pools[i] && particle_pools[i]->hasMatchingKeyBlock(key_block, key_index))
      return particle_pools[i];

  return 0; 
}

void afxChoreographer::registerParticlePool(afxParticlePool* pool) 
{ 
  particle_pools.push_back(pool);
}

void afxChoreographer::unregisterParticlePool(afxParticlePool* pool) 
{ 
  for (S32 i = 0; i < particle_pools.size(); i++)
    if (particle_pools[i] == pool)
    {
      particle_pools.erase_fast(i);
      return;
    }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

DefineEngineMethod( afxChoreographer, setRanking, void, ( unsigned int ranking ),,
   "Set a ranking value (0-255) for the choreographer.\n" )
{
  object->setRanking((U8)ranking);
}

DefineEngineMethod( afxChoreographer, setLevelOfDetail, void, ( unsigned int lod ),,
   "Set a level-of-detail value (0-255) for the choreographer.\n" )
{
  object->setLevelOfDetail((U8)lod);
}

DefineEngineMethod( afxChoreographer, setExecConditions, void, ( U32 mask ),,
   "Set a bitmask to specifiy the state of exec-conditions.\n" )
{
  object->setExecConditions(afxChoreographer::USER_EXEC_CONDS_MASK & mask);
}

DefineEngineMethod( afxChoreographer, addConstraint, void, ( const char* source, const char* name),,
   "Add a dynamic constraint consistiing of a source and name. The source can be a SceneObject, a 3-valued position, or a 7-valued transform.\n" )
{
  if (!object->addConstraint(source, name))
    Con::errorf("afxChoreographer::addConstraint() -- failed to resolve constraint source [%s].", source);
}

DefineEngineMethod( afxChoreographer, addExplicitClient, void, ( NetConnection* client ),,
   "Add an explicit client.\n" )
{
   if (!client)
   {
      Con::errorf(ConsoleLogEntry::General, "afxChoreographer::addExplicitClient: Failed to resolve client connection");
      return;
   }

   object->addExplicitClient(client);
}

DefineEngineMethod( afxChoreographer, setTriggerBit, void, ( U32 bit_num ),,
   "Set a bit of the trigger-mask.\n" )
{
  U32 set_bit = 1 << bit_num;
  object->setTriggerMask(set_bit | object->getTriggerMask());
}

DefineEngineMethod( afxChoreographer, clearTriggerBit, void, ( U32 bit_num ),,
   "Unset a bit of the trigger-mask.\n" )
{
  U32 clear_bit = 1 << bit_num;
  object->setTriggerMask(~clear_bit & object->getTriggerMask());
}

DefineEngineMethod( afxChoreographer, testTriggerBit, bool, ( U32 bit_num ),,
   "Test state of a trigger-mask bit.\n" )
{
  U32 test_bit = 1 << bit_num;
  return ((test_bit & object->getTriggerMask()) != 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

// CONSTRAINT REMAPPING <<

void afxChoreographer::remapObjectConstraint(SceneObject* object, const char* cons_name)
{
  if (!object || !cons_name)
    return;

  // must be a constraint-def with a matching name in the list
  dynConstraintDef* dyn_def = find_cons_def_by_name(cons_name);
  if (!dyn_def)
  {
    Con::errorf("afxChoreographer::remapObjectConstraint() -- failed to find constraint name [%s].", cons_name);
    return;
  }

  // constraint-def must have matching name constraint-type
  if (dyn_def->cons_type != OBJECT_CONSTRAINT)
  {
    Con::errorf("afxChoreographer::remapObjectConstraint() -- remapped contraint type does not match existing constraint type.");
    return;
  }

  // nothing to do if new object is same as old 
  if (dyn_def->cons_obj.object == object)
  {
#ifdef TORQUE_DEBUG
    Con::warnf("afxChoreographer::remapObjectConstraint() -- remapped contraint object is same as existing object.");
#endif
    return;
  }

  dyn_def->cons_obj.object = object;
  deleteNotify(object);

  constraint_mgr->setReferenceObject(StringTable->insert(cons_name), object);

#if defined(AFX_CAP_SCOPE_TRACKING)
  if (isServerObject() && object->isScopeable())
    constraint_mgr->addScopeableObject(object);
#endif

  if (isServerObject())
  {
    if (remapped_cons_sent)
    {
      remapped_cons_defs.clear();
      remapped_cons_sent = false;
    }
    remapped_cons_defs.push_back(dyn_def);
    setMaskBits(RemapConstraintMask);
  }
}

void afxChoreographer::remapObjectConstraint(U16 scope_id, const char* cons_name, bool is_shape)
{
  if (!cons_name)
    return;

  // must be a constraint-def with a matching name in the list
  dynConstraintDef* dyn_def = find_cons_def_by_name(cons_name);
  if (!dyn_def)
  {
    Con::errorf("afxChoreographer::remapObjectConstraint() -- failed to find constraint name [%s].", cons_name);
    return;
  }

  // constraint-def must have matching name constraint-type
  if (dyn_def->cons_type != OBJECT_CONSTRAINT)
  {
    Con::errorf("afxChoreographer::remapObjectConstraint() -- remapped contraint type does not match existing constraint type.");
    return;
  }

  constraint_mgr->setReferenceObjectByScopeId(StringTable->insert(cons_name), scope_id, is_shape);
}

void afxChoreographer::remapPointConstraint(Point3F& point, const char* cons_name)
{
  // must be a constraint-def with a matching name in the list
  dynConstraintDef* dyn_def = find_cons_def_by_name(cons_name);
  if (!dyn_def)
  {
    Con::errorf("afxChoreographer::remapPointConstraint() -- failed to find constraint name [%s].", cons_name);
    return;
  }

  // constraint-def must have matching name constraint-type
  if (dyn_def->cons_type != POINT_CONSTRAINT)
  {
    Con::errorf("afxChoreographer::remapPointConstraint() -- remapped contraint type does not match existing constraint type.");
    return;
  }

  *dyn_def->cons_obj.point = point;

  constraint_mgr->setReferencePoint(StringTable->insert(cons_name), point);

  if (isServerObject())
  {
    if (remapped_cons_sent)
    {
      remapped_cons_defs.clear();
      remapped_cons_sent = false;
    }
    remapped_cons_defs.push_back(dyn_def);
    setMaskBits(RemapConstraintMask);
  }
}

void afxChoreographer::remapTransformConstraint(MatrixF& xfm, const char* cons_name)
{
  // must be a constraint-def with a matching name in the list
  dynConstraintDef* dyn_def = find_cons_def_by_name(cons_name);
  if (!dyn_def)
  {
    Con::errorf("afxChoreographer::remapTransformConstraint() -- failed to find constraint name [%s].", cons_name);
    return;
  }

  // constraint-def must have matching name constraint-type
  if (dyn_def->cons_type != POINT_CONSTRAINT)
  {
    Con::errorf("afxChoreographer::remapTransformConstraint() -- remapped contraint type does not match existing constraint type.");
    return;
  }

  *dyn_def->cons_obj.xfm = xfm;

  constraint_mgr->setReferenceTransform(StringTable->insert(cons_name), xfm);

  if (isServerObject())
  {
    if (remapped_cons_sent)
    {
      remapped_cons_defs.clear();
      remapped_cons_sent = false;
    }
    remapped_cons_defs.push_back(dyn_def);
    setMaskBits(RemapConstraintMask);
  }
}

bool afxChoreographer::remapConstraint(const char* source_spec, const char* cons_name)
{
  SceneObject* scn_obj;
  Point3F pos;
  MatrixF xfm;

  switch (resolve_cons_spec(source_spec, pos, xfm, &scn_obj))
  {
  case TRANSFORM_CONSTRAINT:
    //addTransformConstraint(xfm, cons_name);
    return true;
  case POINT_CONSTRAINT:
    //addPointConstraint(pos, cons_name);
    return true;
  case OBJECT_CONSTRAINT:
    remapObjectConstraint(scn_obj, cons_name);
    return true;
  }

  return false;
}

DefineEngineMethod( afxChoreographer, remapConstraint, void, ( const char* source, const char* name),,
   "Remap a dynamic constraint to use a new source. The source can be a SceneObject, a 3-valued position, or a 7-valued transform. but must match type of existing source.\n" )
{
  if (!object->remapConstraint(source, name))
    Con::errorf("afxChoreographer::remapConstraint() -- failed to resolve constraint source [%s].", source);
}

// CONSTRAINT REMAPPING >>

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
