
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

#include "arcaneFX.h"

#include "T3D/aiPlayer.h"
#include "T3D/tsStatic.h"
#include "sim/netConnection.h"
#include "ts/tsShapeInstance.h"

#include "afxConstraint.h"
#include "afxChoreographer.h"
#include "afxEffectWrapper.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxConstraintDef

// static
StringTableEntry  afxConstraintDef::SCENE_CONS_KEY;
StringTableEntry  afxConstraintDef::EFFECT_CONS_KEY;
StringTableEntry  afxConstraintDef::GHOST_CONS_KEY;

afxConstraintDef::afxConstraintDef()
{
  if (SCENE_CONS_KEY == 0)
  {
    SCENE_CONS_KEY = StringTable->insert("#scene");
    EFFECT_CONS_KEY = StringTable->insert("#effect");
    GHOST_CONS_KEY = StringTable->insert("#ghost");
  }

  reset();
}

bool afxConstraintDef::isDefined() 
{
  return (def_type != CONS_UNDEFINED); 
}

bool afxConstraintDef::isArbitraryObject() 
{ 
  return ((cons_src_name != ST_NULLSTRING) && (def_type == CONS_SCENE)); 
}

void afxConstraintDef::reset()
{
  cons_src_name = ST_NULLSTRING;
  cons_node_name = ST_NULLSTRING;
  def_type = CONS_UNDEFINED;
  history_time = 0;
  sample_rate = 30;
  runs_on_server = false;
  runs_on_client = false;
  pos_at_box_center = false;
  treat_as_camera = false;
}

bool afxConstraintDef::parseSpec(const char* spec, bool runs_on_server, 
                                 bool runs_on_client)
{
  reset();

  if (spec == 0 || spec[0] == '\0')
    return false;

  history_time = 0.0f;
  sample_rate = 30;

  this->runs_on_server = runs_on_server;
  this->runs_on_client = runs_on_client;

  // spec should be in one of these forms:
  //    CONSTRAINT_NAME (only)
  //    CONSTRAINT_NAME.NODE (shapeBase objects only)
  //    CONSTRAINT_NAME.#center
  //    object.OBJECT_NAME
  //    object.OBJECT_NAME.NODE (shapeBase objects only)
  //    object.OBJECT_NAME.#center
  //    effect.EFFECT_NAME
  //    effect.EFFECT_NAME.NODE
  //    effect.EFFECT_NAME.#center
  //    #ghost.EFFECT_NAME
  //    #ghost.EFFECT_NAME.NODE
  //    #ghost.EFFECT_NAME.#center
  //

  // create scratch buffer by duplicating spec.
  char special = '\b';
  char* buffer = dStrdup(spec);

  // substitute a dots not inside parens with special character
  S32 n_nested = 0;
  for (char* b = buffer; (*b) != '\0'; b++)
  {
    if ((*b) == '(')
      n_nested++;
    else if ((*b) == ')')
      n_nested--;
    else if ((*b) == '.' && n_nested == 0)
      (*b) = special;
  }

  // divide name into '.' separated tokens (up to 8)
  char* words[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  char* dot = buffer;
  int wdx = 0;
  while (wdx < 8)
  {
    words[wdx] = dot;
    dot = dStrchr(words[wdx++], special);
    if (!dot)
      break;
    *(dot++) = '\0';
    if ((*dot) == '\0')
      break;
  }

  int n_words = wdx;

  // at this point the spec has been split into words. 
  // n_words indicates how many words we have.

  // no words found (must have been all whitespace)
  if (n_words < 1)
  {
    dFree(buffer);
    return false;
  }

  char* hist_spec = 0;
  char* words2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  int n_words2 = 0;

  // move words to words2 while extracting #center and #history
  for (S32 i = 0; i < n_words; i++)
  {
    if (dStrcmp(words[i], "#center") == 0)
      pos_at_box_center = true;
    else if (dStrncmp(words[i], "#history(", 9) == 0)
      hist_spec = words[i];
    else
      words2[n_words2++] = words[i];
  }

  // words2[] now contains just the constraint part

  // no words found (must have been all #center and #history)
  if (n_words2 < 1)
  {
    dFree(buffer);
    return false;
  }

  if (hist_spec)
  {
    char* open_paren = dStrchr(hist_spec, '(');
    if (open_paren)
    {
      hist_spec = open_paren+1;
      if ((*hist_spec) != '\0')
      {
        char* close_paren = dStrchr(hist_spec, ')');
        if (close_paren)
          (*close_paren) = '\0';
        char* slash = dStrchr(hist_spec, '/');
        if (slash)
          (*slash) = ' ';

        F32 hist_age = 0.0;
        U32 hist_rate = 30;
        S32 args = dSscanf(hist_spec,"%g %d", &hist_age, &hist_rate);

        if (args > 0)
          history_time = hist_age;
        if (args > 1)
          sample_rate = hist_rate;
      }
    }
  }

  StringTableEntry cons_name_key = StringTable->insert(words2[0]);

  // must be in CONSTRAINT_NAME (only) form
  if (n_words2 == 1)
  {
    // arbitrary object/effect constraints must have a name
    if (cons_name_key == SCENE_CONS_KEY || cons_name_key == EFFECT_CONS_KEY)
    {
      dFree(buffer);
      return false;
    }

    cons_src_name = cons_name_key;
    def_type = CONS_PREDEFINED;
    dFree(buffer);
    return true;
  }

  // "#scene.NAME" or "#scene.NAME.NODE""
  if (cons_name_key == SCENE_CONS_KEY)
  {
    cons_src_name = StringTable->insert(words2[1]);
    if (n_words2 > 2)
      cons_node_name = StringTable->insert(words2[2]);
    def_type = CONS_SCENE;
    dFree(buffer);
    return true;
  }

  // "#effect.NAME" or "#effect.NAME.NODE"
  if (cons_name_key == EFFECT_CONS_KEY)
  {
    cons_src_name = StringTable->insert(words2[1]);
    if (n_words2 > 2)
      cons_node_name = StringTable->insert(words2[2]);
    def_type = CONS_EFFECT;
    dFree(buffer);
    return true;
  }

  // "#ghost.NAME" or "#ghost.NAME.NODE"
  if (cons_name_key == GHOST_CONS_KEY)
  {
    if (runs_on_server)
    {
      dFree(buffer);
      return false;
    }

    cons_src_name = StringTable->insert(words2[1]);
    if (n_words2 > 2)
      cons_node_name = StringTable->insert(words2[2]);
    def_type = CONS_GHOST;
    dFree(buffer);
    return true;
  }

  // "CONSTRAINT_NAME.NODE"
  if (n_words2 == 2)
  {
    cons_src_name = cons_name_key;
    cons_node_name = StringTable->insert(words2[1]);
    def_type = CONS_PREDEFINED;
    dFree(buffer);
    return true;
  }

  // must be in unsupported form
  dFree(buffer); 
  return false;
}

void afxConstraintDef::gather_cons_defs(Vector<afxConstraintDef>& defs, afxEffectList& fx)
{
  for (S32 i = 0; i <  fx.size(); i++)
  {
    if (fx[i])
      fx[i]->gather_cons_defs(defs);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxConstraint

afxConstraint::afxConstraint(afxConstraintMgr* mgr)
{
  this->mgr = mgr;
  is_defined = false;
  is_valid = false;
  last_pos.zero();
  last_xfm.identity();
  history_time = 0.0f;
  is_alive = true;
  gone_missing = false;
  change_code = 0;
}

afxConstraint::~afxConstraint()
{
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

inline afxPointConstraint* newPointCons(afxConstraintMgr* mgr, bool hist)
{
  return (hist) ? new afxPointHistConstraint(mgr) : new afxPointConstraint(mgr);
}

inline afxTransformConstraint* newTransformCons(afxConstraintMgr* mgr, bool hist)
{
  return (hist) ? new afxTransformHistConstraint(mgr) : new afxTransformConstraint(mgr);
}

inline afxShapeConstraint* newShapeCons(afxConstraintMgr* mgr, bool hist)
{
  return (hist) ? new afxShapeHistConstraint(mgr) : new afxShapeConstraint(mgr);
}

inline afxShapeConstraint* newShapeCons(afxConstraintMgr* mgr, StringTableEntry name, bool hist)
{
  return (hist) ? new afxShapeHistConstraint(mgr, name) : new afxShapeConstraint(mgr, name);
}

inline afxShapeNodeConstraint* newShapeNodeCons(afxConstraintMgr* mgr, StringTableEntry name, StringTableEntry node, bool hist)
{
  return (hist) ? new afxShapeNodeHistConstraint(mgr, name, node) : new afxShapeNodeConstraint(mgr, name, node);
}

inline afxObjectConstraint* newObjectCons(afxConstraintMgr* mgr, bool hist)
{
  return (hist) ? new afxObjectHistConstraint(mgr) : new afxObjectConstraint(mgr);
}

inline afxObjectConstraint* newObjectCons(afxConstraintMgr* mgr, StringTableEntry name, bool hist)
{
  return (hist) ? new afxObjectHistConstraint(mgr, name) : new afxObjectConstraint(mgr, name);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxConstraintMgr

#define CONS_BY_ID(id) ((*constraints_v[(id).index])[(id).sub_index])
#define CONS_BY_IJ(i,j) ((*constraints_v[(i)])[(j)])

afxConstraintMgr::afxConstraintMgr()
{
  starttime = 0;
  on_server = false;
  initialized = false;
  scoping_dist_sq = 1000.0f*1000.0f;
  missing_objs = &missing_objs_a;
  missing_objs2 = &missing_objs_b;
}

afxConstraintMgr::~afxConstraintMgr()
{
  for (S32 i = 0; i < constraints_v.size(); i++)
  {
    for (S32 j = 0; j < (*constraints_v[i]).size(); j++)
      delete CONS_BY_IJ(i,j);
    delete constraints_v[i];
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

S32 afxConstraintMgr::find_cons_idx_from_name(StringTableEntry which)
{
  for (S32 i = 0; i < constraints_v.size(); i++)
  {
    afxConstraint* cons = CONS_BY_IJ(i,0);
    if (cons && afxConstraintDef::CONS_EFFECT != cons->cons_def.def_type && 
        which == cons->cons_def.cons_src_name)
    {
        return i;
    }
  }

  return -1;
}

S32 afxConstraintMgr::find_effect_cons_idx_from_name(StringTableEntry which)
{
  for (S32 i = 0; i < constraints_v.size(); i++)
  {
    afxConstraint* cons = CONS_BY_IJ(i,0);
    if (cons && afxConstraintDef::CONS_EFFECT == cons->cons_def.def_type && 
        which == cons->cons_def.cons_src_name)
    {
      return i;
    }
  }

  return -1;
}

// Defines a predefined constraint with given name and type
void afxConstraintMgr::defineConstraint(U32 type, StringTableEntry name)
{
  preDef predef = { name, type };
  predefs.push_back(predef);
}

afxConstraintID afxConstraintMgr::setReferencePoint(StringTableEntry which, Point3F point, 
                                                    Point3F vector)
{
  S32 idx = find_cons_idx_from_name(which);
  if (idx < 0)
    return afxConstraintID();

  afxConstraintID id = afxConstraintID(idx);
  setReferencePoint(id, point, vector); 

  return id;
}

afxConstraintID afxConstraintMgr::setReferenceTransform(StringTableEntry which, MatrixF& xfm)
{
  S32 idx = find_cons_idx_from_name(which);
  if (idx < 0)
    return afxConstraintID();

  afxConstraintID id = afxConstraintID(idx);
  setReferenceTransform(id, xfm); 

  return id;
}

// Assigns an existing scene-object to the named constraint
afxConstraintID afxConstraintMgr::setReferenceObject(StringTableEntry which, SceneObject* obj)
{
  S32 idx = find_cons_idx_from_name(which);
  if (idx < 0)
    return afxConstraintID();

  afxConstraintID id = afxConstraintID(idx);
  setReferenceObject(id, obj); 

  return id;
}

// Assigns an un-scoped scene-object by scope_id to the named constraint
afxConstraintID afxConstraintMgr::setReferenceObjectByScopeId(StringTableEntry which, U16 scope_id, bool is_shape)
{
  S32 idx = find_cons_idx_from_name(which);
  if (idx < 0)
    return afxConstraintID();

  afxConstraintID id = afxConstraintID(idx);
  setReferenceObjectByScopeId(id, scope_id, is_shape); 

  return id;
}

afxConstraintID afxConstraintMgr::setReferenceEffect(StringTableEntry which, afxEffectWrapper* ew)
{
  S32 idx = find_effect_cons_idx_from_name(which);

  if (idx < 0)
    return afxConstraintID();

  afxConstraintID id = afxConstraintID(idx);
  setReferenceEffect(id, ew); 

  return id;
}

afxConstraintID afxConstraintMgr::createReferenceEffect(StringTableEntry which, afxEffectWrapper* ew)
{
  afxEffectConstraint* cons = new afxEffectConstraint(this, which);
  //cons->cons_def = def;
  cons->cons_def.def_type = afxConstraintDef::CONS_EFFECT;
  cons->cons_def.cons_src_name = which;
  afxConstraintList* list = new afxConstraintList();
  list->push_back(cons);
  constraints_v.push_back(list);

  return setReferenceEffect(which, ew);
}

void afxConstraintMgr::setReferencePoint(afxConstraintID id, Point3F point, Point3F vector)
{
  afxPointConstraint* pt_cons = dynamic_cast<afxPointConstraint*>(CONS_BY_ID(id));

  // need to change type
  if (!pt_cons)
  {
    afxConstraint* cons = CONS_BY_ID(id);
    pt_cons = newPointCons(this, cons->cons_def.history_time > 0.0f);
    pt_cons->cons_def = cons->cons_def;
    CONS_BY_ID(id) = pt_cons;
    delete cons;
  }

  pt_cons->set(point, vector);

  // nullify all subnodes
  for (S32 j = 1; j < (*constraints_v[id.index]).size(); j++)
  {
    afxConstraint* cons = CONS_BY_IJ(id.index,j);
    if (cons)
      cons->unset();
  }
}

void afxConstraintMgr::setReferenceTransform(afxConstraintID id, MatrixF& xfm)
{
  afxTransformConstraint* xfm_cons = dynamic_cast<afxTransformConstraint*>(CONS_BY_ID(id));

  // need to change type
  if (!xfm_cons)
  {
    afxConstraint* cons = CONS_BY_ID(id);
    xfm_cons = newTransformCons(this, cons->cons_def.history_time > 0.0f);
    xfm_cons->cons_def = cons->cons_def;
    CONS_BY_ID(id) = xfm_cons;
    delete cons;
  }

  xfm_cons->set(xfm);

  // nullify all subnodes
  for (S32 j = 1; j < (*constraints_v[id.index]).size(); j++)
  {
    afxConstraint* cons = CONS_BY_IJ(id.index,j);
    if (cons)
      cons->unset();
  }
}

void afxConstraintMgr::set_ref_shape(afxConstraintID id, ShapeBase* shape)
{
  id.sub_index = 0;

  afxShapeConstraint* shape_cons = dynamic_cast<afxShapeConstraint*>(CONS_BY_ID(id));

  // need to change type
  if (!shape_cons)
  {
    afxConstraint* cons = CONS_BY_ID(id);
    shape_cons = newShapeCons(this, cons->cons_def.history_time > 0.0f);
    shape_cons->cons_def = cons->cons_def;
    CONS_BY_ID(id) = shape_cons;
    delete cons;
  }

  // set new shape on root 
  shape_cons->set(shape);

  // update all subnodes
  for (S32 j = 1; j < (*constraints_v[id.index]).size(); j++)
  {
    afxConstraint* cons = CONS_BY_IJ(id.index,j);
    if (cons)
    {
      if (dynamic_cast<afxShapeNodeConstraint*>(cons))
        ((afxShapeNodeConstraint*)cons)->set(shape);
      else if (dynamic_cast<afxShapeConstraint*>(cons))
        ((afxShapeConstraint*)cons)->set(shape);
      else if (dynamic_cast<afxObjectConstraint*>(cons))
        ((afxObjectConstraint*)cons)->set(shape);
      else
        cons->unset();
    }
  }
}

void afxConstraintMgr::set_ref_shape(afxConstraintID id, U16 scope_id)
{
  id.sub_index = 0;

  afxShapeConstraint* shape_cons = dynamic_cast<afxShapeConstraint*>(CONS_BY_ID(id));

  // need to change type
  if (!shape_cons)
  {
    afxConstraint* cons = CONS_BY_ID(id);
    shape_cons = newShapeCons(this, cons->cons_def.history_time > 0.0f);
    shape_cons->cons_def = cons->cons_def;
    CONS_BY_ID(id) = shape_cons;
    delete cons;
  }

  // set new shape on root 
  shape_cons->set_scope_id(scope_id);

  // update all subnodes
  for (S32 j = 1; j < (*constraints_v[id.index]).size(); j++)
  {
    afxConstraint* cons = CONS_BY_IJ(id.index,j);
    if (cons)
      cons->set_scope_id(scope_id);
  }
}

// Assigns an existing scene-object to the constraint matching the given constraint-id.
void afxConstraintMgr::setReferenceObject(afxConstraintID id, SceneObject* obj)
{
  if (!initialized)
    Con::errorf("afxConstraintMgr::setReferenceObject() -- constraint manager not initialized");

  if (!CONS_BY_ID(id)->cons_def.treat_as_camera)
  {
    ShapeBase* shape = dynamic_cast<ShapeBase*>(obj);
    if (shape)
    {
      set_ref_shape(id, shape);
      return;
    }
  }

  afxObjectConstraint* obj_cons = dynamic_cast<afxObjectConstraint*>(CONS_BY_ID(id));

  // need to change type
  if (!obj_cons)
  {
    afxConstraint* cons = CONS_BY_ID(id);
    obj_cons = newObjectCons(this, cons->cons_def.history_time > 0.0f);
    obj_cons->cons_def = cons->cons_def;
    CONS_BY_ID(id) = obj_cons;
    delete cons;
  }

  obj_cons->set(obj);

  // update all subnodes
  for (S32 j = 1; j < (*constraints_v[id.index]).size(); j++)
  {
    afxConstraint* cons = CONS_BY_IJ(id.index,j);
    if (cons)
    {
      if (dynamic_cast<afxObjectConstraint*>(cons))
          ((afxObjectConstraint*)cons)->set(obj);
      else
        cons->unset();
    }
  }
}

// Assigns an un-scoped scene-object by scope_id to the constraint matching the 
// given constraint-id.
void afxConstraintMgr::setReferenceObjectByScopeId(afxConstraintID id, U16 scope_id, bool is_shape)
{
  if (!initialized)
    Con::errorf("afxConstraintMgr::setReferenceObject() -- constraint manager not initialized");

  if (is_shape)
  {
    set_ref_shape(id, scope_id);
    return;
  }

  afxObjectConstraint* obj_cons = dynamic_cast<afxObjectConstraint*>(CONS_BY_ID(id));

  // need to change type
  if (!obj_cons)
  {
    afxConstraint* cons = CONS_BY_ID(id);
    obj_cons = newObjectCons(this, cons->cons_def.history_time > 0.0f);
    obj_cons->cons_def = cons->cons_def;
    CONS_BY_ID(id) = obj_cons;
    delete cons;
  }

  obj_cons->set_scope_id(scope_id);

  // update all subnodes
  for (S32 j = 1; j < (*constraints_v[id.index]).size(); j++)
  {
    afxConstraint* cons = CONS_BY_IJ(id.index,j);
    if (cons)
      cons->set_scope_id(scope_id);
  }
}

void afxConstraintMgr::setReferenceEffect(afxConstraintID id, afxEffectWrapper* ew)
{
  afxEffectConstraint* eff_cons = dynamic_cast<afxEffectConstraint*>(CONS_BY_ID(id));
  if (!eff_cons)
    return;

  eff_cons->set(ew);

  // update all subnodes
  for (S32 j = 1; j < (*constraints_v[id.index]).size(); j++)
  {
    afxConstraint* cons = CONS_BY_IJ(id.index,j);
    if (cons)
    {
      if (dynamic_cast<afxEffectNodeConstraint*>(cons))
        ((afxEffectNodeConstraint*)cons)->set(ew);
      else if (dynamic_cast<afxEffectConstraint*>(cons))
        ((afxEffectConstraint*)cons)->set(ew);
      else
        cons->unset();
    }
  }
}

void afxConstraintMgr::invalidateReference(afxConstraintID id)
{
  afxConstraint* cons = CONS_BY_ID(id);
  if (cons)
    cons->is_valid = false;
}

void afxConstraintMgr::create_constraint(const afxConstraintDef& def)
{
  if (def.def_type == afxConstraintDef::CONS_UNDEFINED)
    return;

  //Con::printf("CON - %s [%s] [%s] h=%g", def.cons_type_name, def.cons_src_name, def.cons_node_name, def.history_time);

  bool want_history = (def.history_time > 0.0f);

  // constraint is an arbitrary named scene object
  //
  if (def.def_type == afxConstraintDef::CONS_SCENE)
  {
    if (def.cons_src_name == ST_NULLSTRING)
      return;

    // find the arbitrary object by name
    SceneObject* arb_obj;
    if (on_server)
    {
      arb_obj = dynamic_cast<SceneObject*>(Sim::findObject(def.cons_src_name));
      if (!arb_obj)
         Con::errorf("afxConstraintMgr -- failed to find scene constraint source, \"%s\" on server.", 
                     def.cons_src_name);
    }
    else
    {
      arb_obj = find_object_from_name(def.cons_src_name);
      if (!arb_obj)
         Con::errorf("afxConstraintMgr -- failed to find scene constraint source, \"%s\" on client.", 
                     def.cons_src_name);
    }

    // if it's a shapeBase object, create a Shape or ShapeNode constraint
    if (dynamic_cast<ShapeBase*>(arb_obj))
    {
      if (def.cons_node_name == ST_NULLSTRING && !def.pos_at_box_center)
      {
        afxShapeConstraint* cons = newShapeCons(this, def.cons_src_name, want_history);
        cons->cons_def = def;
        cons->set((ShapeBase*)arb_obj); 
        afxConstraintList* list = new afxConstraintList();
        list->push_back(cons);
        constraints_v.push_back(list);
      }
      else if (def.pos_at_box_center)
      {
        afxShapeConstraint* cons = newShapeCons(this, def.cons_src_name, want_history);
        cons->cons_def = def;
        cons->set((ShapeBase*)arb_obj); 
        afxConstraintList* list = constraints_v[constraints_v.size()-1]; // SHAPE-NODE CONS-LIST (#scene)(#center)
        if (list && (*list)[0])
          list->push_back(cons);
      }
      else
      {
        afxShapeNodeConstraint* sub = newShapeNodeCons(this, def.cons_src_name, def.cons_node_name, want_history);
        sub->cons_def = def;
        sub->set((ShapeBase*)arb_obj); 
        afxConstraintList* list = constraints_v[constraints_v.size()-1];
        if (list && (*list)[0])
          list->push_back(sub);
      }
    }
    // if it's not a shapeBase object, create an Object constraint
    else if (arb_obj)
    {
      if (!def.pos_at_box_center)
      {
        afxObjectConstraint* cons = newObjectCons(this, def.cons_src_name, want_history);
        cons->cons_def = def;
        cons->set(arb_obj);
        afxConstraintList* list = new afxConstraintList(); // OBJECT CONS-LIST (#scene)
        list->push_back(cons);
        constraints_v.push_back(list);
      }
      else // if (def.pos_at_box_center)
      {
        afxObjectConstraint* cons = newObjectCons(this, def.cons_src_name, want_history);
        cons->cons_def = def;
        cons->set(arb_obj); 
        afxConstraintList* list = constraints_v[constraints_v.size()-1]; // OBJECT CONS-LIST (#scene)(#center)
        if (list && (*list)[0])
          list->push_back(cons);
      }
    }
  }

  // constraint is an arbitrary named effect
  //
  else if (def.def_type == afxConstraintDef::CONS_EFFECT)
  {
    if (def.cons_src_name == ST_NULLSTRING)
      return;

    // create an Effect constraint
    if (def.cons_node_name == ST_NULLSTRING && !def.pos_at_box_center)
    {
      afxEffectConstraint* cons = new afxEffectConstraint(this, def.cons_src_name);
      cons->cons_def = def;
      afxConstraintList* list = new afxConstraintList();
      list->push_back(cons);
      constraints_v.push_back(list);
    }
    // create an Effect #center constraint
    else if (def.pos_at_box_center)
    {
      afxEffectConstraint* cons = new afxEffectConstraint(this, def.cons_src_name);
      cons->cons_def = def;
      afxConstraintList* list = constraints_v[constraints_v.size()-1]; // EFFECT-NODE CONS-LIST (#effect)
      if (list && (*list)[0])
        list->push_back(cons);
    }
    // create an EffectNode constraint
    else
    {
      afxEffectNodeConstraint* sub = new afxEffectNodeConstraint(this, def.cons_src_name, def.cons_node_name);
      sub->cons_def = def;
      afxConstraintList* list = constraints_v[constraints_v.size()-1];
      if (list && (*list)[0])
        list->push_back(sub);
    }
  }

  // constraint is a predefined constraint
  //
  else
  {
    afxConstraint* cons = 0;
    afxConstraint* cons_ctr = 0;
    afxConstraint* sub = 0;

    if (def.def_type == afxConstraintDef::CONS_GHOST)
    {
      for (S32 i = 0; i < predefs.size(); i++)
      {
        if (predefs[i].name == def.cons_src_name)
        {
          if (def.cons_node_name == ST_NULLSTRING && !def.pos_at_box_center)
          {
            cons = newShapeCons(this, want_history);
            cons->cons_def = def;
          }
          else if (def.pos_at_box_center)
          {
            cons_ctr = newShapeCons(this, want_history);
            cons_ctr->cons_def = def;
          }
          else
          {
            sub = newShapeNodeCons(this, ST_NULLSTRING, def.cons_node_name, want_history);
            sub->cons_def = def;
          }
          break;
        }
      }
    }
    else
    {
      for (S32 i = 0; i < predefs.size(); i++)
      {
        if (predefs[i].name == def.cons_src_name)
        {
          switch (predefs[i].type)
          {
          case POINT_CONSTRAINT:
            cons = newPointCons(this, want_history);
            cons->cons_def = def;
            break;
          case TRANSFORM_CONSTRAINT:
            cons = newTransformCons(this, want_history);
            cons->cons_def = def;
            break;
          case OBJECT_CONSTRAINT:
            if (def.cons_node_name == ST_NULLSTRING && !def.pos_at_box_center)
            {
              cons = newShapeCons(this, want_history);
              cons->cons_def = def;
            }
            else if (def.pos_at_box_center)
            {
              cons_ctr = newShapeCons(this, want_history);
              cons_ctr->cons_def = def;
            }
            else
            {
              sub = newShapeNodeCons(this, ST_NULLSTRING, def.cons_node_name, want_history);
              sub->cons_def = def;
            }
            break;
          case CAMERA_CONSTRAINT:
            cons = newObjectCons(this, want_history);
            cons->cons_def = def;
            cons->cons_def.treat_as_camera = true;
            break;
          }
          break;
        }
      }
    }

    if (cons)
    {
      afxConstraintList* list = new afxConstraintList();
      list->push_back(cons);
      constraints_v.push_back(list);
    }
    else if (cons_ctr && constraints_v.size() > 0)
    {
      afxConstraintList* list = constraints_v[constraints_v.size()-1]; // PREDEF-NODE CONS-LIST
      if (list && (*list)[0])
        list->push_back(cons_ctr);
    }
    else if (sub && constraints_v.size() > 0)
    {
      afxConstraintList* list = constraints_v[constraints_v.size()-1];
      if (list && (*list)[0])
        list->push_back(sub);
    }
    else
      Con::printf("predef not found %s", def.cons_src_name);
  }
}

afxConstraintID afxConstraintMgr::getConstraintId(const afxConstraintDef& def)
{
  if (def.def_type == afxConstraintDef::CONS_UNDEFINED)
    return afxConstraintID();

  if (def.cons_src_name != ST_NULLSTRING)
  {
    for (S32 i = 0; i < constraints_v.size(); i++)
    {
      afxConstraintList* list = constraints_v[i];
      afxConstraint* cons = (*list)[0];
      if (def.cons_src_name == cons->cons_def.cons_src_name)
      {
        for (S32 j = 0; j < list->size(); j++)
        {
          afxConstraint* sub = (*list)[j];
          if (def.cons_node_name == sub->cons_def.cons_node_name && 
              def.pos_at_box_center == sub->cons_def.pos_at_box_center && 
              def.cons_src_name == sub->cons_def.cons_src_name)
          {
            return afxConstraintID(i, j);
          }
        }

        // if we're here, it means the root object name matched but the node name
        // did not.
        if (def.def_type == afxConstraintDef::CONS_PREDEFINED && !def.pos_at_box_center)
        {
          afxShapeConstraint* shape_cons = dynamic_cast<afxShapeConstraint*>(cons);
          if (shape_cons)
          {
             //Con::errorf("Append a Node constraint [%s.%s] [%d,%d]", def.cons_src_name, def.cons_node_name, i, list->size());
             bool want_history = (def.history_time > 0.0f);
             afxConstraint* sub = newShapeNodeCons(this, ST_NULLSTRING, def.cons_node_name, want_history);
             sub->cons_def = def;
             ((afxShapeConstraint*)sub)->set(shape_cons->shape);
             list->push_back(sub);

             return afxConstraintID(i, list->size()-1);
          }
        }

        break;
      }
    }
  }

  return afxConstraintID();
}

afxConstraint* afxConstraintMgr::getConstraint(afxConstraintID id)
{
  if (id.undefined())
    return 0;

  afxConstraint* cons = CONS_BY_IJ(id.index,id.sub_index);
  if (cons && !cons->isDefined())
    return NULL;

  return cons;
}

void afxConstraintMgr::sample(F32 dt, U32 now, const Point3F* cam_pos)
{
  U32 elapsed = now - starttime;

  for (S32 i = 0; i < constraints_v.size(); i++)
  {
    afxConstraintList* list = constraints_v[i];
    for (S32 j = 0; j < list->size(); j++)
      (*list)[j]->sample(dt, elapsed, cam_pos);
  }
}

S32 QSORT_CALLBACK cmp_cons_defs(const void* a, const void* b)
{
  afxConstraintDef* def_a = (afxConstraintDef*) a;
  afxConstraintDef* def_b = (afxConstraintDef*) b;

  if (def_a->def_type == def_b->def_type)
  {
    if (def_a->cons_src_name == def_b->cons_src_name)
    {
      if (def_a->pos_at_box_center == def_b->pos_at_box_center)
        return (def_a->cons_node_name - def_b->cons_node_name);
      else
        return (def_a->pos_at_box_center) ? 1 : -1;
    }
    return (def_a->cons_src_name - def_b->cons_src_name);
  }

  return (def_a->def_type - def_b->def_type);
}

void afxConstraintMgr::initConstraintDefs(Vector<afxConstraintDef>& all_defs, bool on_server, F32 scoping_dist)
{
  initialized = true;
  this->on_server = on_server;

  if (scoping_dist > 0.0)
    scoping_dist_sq = scoping_dist*scoping_dist;
  else
  {
    SceneManager* sg = (on_server) ? gServerSceneGraph : gClientSceneGraph;
    F32 vis_dist = (sg) ? sg->getVisibleDistance() : 1000.0f;
    scoping_dist_sq = vis_dist*vis_dist;
  }

  if (all_defs.size() < 1)
    return;

  // find effect ghost constraints
  if (!on_server)
  {
    Vector<afxConstraintDef> ghost_defs;

    for (S32 i = 0; i < all_defs.size(); i++)
      if (all_defs[i].def_type == afxConstraintDef::CONS_GHOST && all_defs[i].cons_src_name != ST_NULLSTRING)
        ghost_defs.push_back(all_defs[i]);
    
    if (ghost_defs.size() > 0)
    {
      // sort the defs
      if (ghost_defs.size() > 1)
        dQsort(ghost_defs.address(), ghost_defs.size(), sizeof(afxConstraintDef), cmp_cons_defs);
      
      S32 last = 0;
      defineConstraint(OBJECT_CONSTRAINT, ghost_defs[0].cons_src_name);

      for (S32 i = 1; i < ghost_defs.size(); i++)
      {
        if (ghost_defs[last].cons_src_name != ghost_defs[i].cons_src_name)
        {
          defineConstraint(OBJECT_CONSTRAINT, ghost_defs[i].cons_src_name);
          last++;
        }
      }
    }
  }

  Vector<afxConstraintDef> defs;

  // collect defs that run here (server or client)
  if (on_server)
  {
    for (S32 i = 0; i < all_defs.size(); i++)
      if (all_defs[i].runs_on_server)
        defs.push_back(all_defs[i]);
  }
  else
  {
    for (S32 i = 0; i < all_defs.size(); i++)
      if (all_defs[i].runs_on_client)
        defs.push_back(all_defs[i]);
  }

  // create unique set of constraints.
  //
  if (defs.size() > 0)
  {
    // sort the defs
    if (defs.size() > 1)
      dQsort(defs.address(), defs.size(), sizeof(afxConstraintDef), cmp_cons_defs);
    
    Vector<afxConstraintDef> unique_defs;
    S32 last = 0;
    
    // manufacture root-object def if absent
    if (defs[0].cons_node_name != ST_NULLSTRING)
    {
      afxConstraintDef root_def = defs[0];
      root_def.cons_node_name = ST_NULLSTRING;
      unique_defs.push_back(root_def);
      last++;
    }
    else if (defs[0].pos_at_box_center)
    {
      afxConstraintDef root_def = defs[0];
      root_def.pos_at_box_center = false;
      unique_defs.push_back(root_def);
      last++;
    }

    unique_defs.push_back(defs[0]);
    
    for (S32 i = 1; i < defs.size(); i++)
    {
      if (unique_defs[last].cons_node_name != defs[i].cons_node_name ||
          unique_defs[last].cons_src_name != defs[i].cons_src_name || 
          unique_defs[last].pos_at_box_center != defs[i].pos_at_box_center || 
          unique_defs[last].def_type != defs[i].def_type)
      {
        // manufacture root-object def if absent
        if (defs[i].cons_src_name != ST_NULLSTRING && unique_defs[last].cons_src_name != defs[i].cons_src_name)
        {
          if (defs[i].cons_node_name != ST_NULLSTRING || defs[i].pos_at_box_center)
          {
            afxConstraintDef root_def = defs[i];
            root_def.cons_node_name = ST_NULLSTRING;
            root_def.pos_at_box_center = false;
            unique_defs.push_back(root_def);
            last++;
          }
        }
        unique_defs.push_back(defs[i]);
        last++;
      }
      else
      {
        if (defs[i].history_time > unique_defs[last].history_time)
          unique_defs[last].history_time = defs[i].history_time;
        if (defs[i].sample_rate > unique_defs[last].sample_rate)
          unique_defs[last].sample_rate = defs[i].sample_rate;
      }
    }
    
    //Con::printf("\nConstraints on %s", (on_server) ? "server" : "client");
    for (S32 i = 0; i < unique_defs.size(); i++)
      create_constraint(unique_defs[i]);
  }

  // collect the names of all the arbitrary object constraints
  // that run on clients and store in names_on_server array.
  //
  if (on_server)
  {
    names_on_server.clear();
    defs.clear();

    for (S32 i = 0; i < all_defs.size(); i++)
      if (all_defs[i].runs_on_client && all_defs[i].isArbitraryObject())
        defs.push_back(all_defs[i]);

    if (defs.size() < 1)
      return;

    // sort the defs
    if (defs.size() > 1)
      dQsort(defs.address(), defs.size(), sizeof(afxConstraintDef), cmp_cons_defs);

    S32 last = 0;
    names_on_server.push_back(defs[0].cons_src_name);

    for (S32 i = 1; i < defs.size(); i++)
    {
      if (names_on_server[last] != defs[i].cons_src_name)
      {
        names_on_server.push_back(defs[i].cons_src_name);
        last++;
      }
    }
  }
}

void afxConstraintMgr::packConstraintNames(NetConnection* conn, BitStream* stream)
{
  // pack any named constraint names and ghost indices
  if (stream->writeFlag(names_on_server.size() > 0)) //-- ANY NAMED CONS_BY_ID?
  {
    stream->write(names_on_server.size());
    for (S32 i = 0; i < names_on_server.size(); i++)
    {
      stream->writeString(names_on_server[i]);
      NetObject* obj = dynamic_cast<NetObject*>(Sim::findObject(names_on_server[i]));
      if (!obj)
      {
        //Con::printf("CONSTRAINT-OBJECT %s does not exist.", names_on_server[i]);
        stream->write((S32)-1);
      }
      else
      {
        S32 ghost_id = conn->getGhostIndex(obj);
        /*
        if (ghost_id == -1)
          Con::printf("CONSTRAINT-OBJECT %s does not have a ghost.", names_on_server[i]);
        else
          Con::printf("CONSTRAINT-OBJECT %s name to server.", names_on_server[i]);
         */
        stream->write(ghost_id);
      }
    }
  }
}

void afxConstraintMgr::unpackConstraintNames(BitStream* stream)
{
  if (stream->readFlag())                                         //-- ANY NAMED CONS_BY_ID?
  {
    names_on_server.clear();
    S32 sz; stream->read(&sz);
    for (S32 i = 0; i < sz; i++)
    {
      names_on_server.push_back(stream->readSTString());
      S32 ghost_id; stream->read(&ghost_id);
      ghost_ids.push_back(ghost_id);
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

SceneObject* afxConstraintMgr::find_object_from_name(StringTableEntry name)
{
  if (names_on_server.size() > 0)
  {
    for (S32 i = 0; i < names_on_server.size(); i++)
      if (names_on_server[i] == name)
      {
        if (ghost_ids[i] == -1)
          return 0;
        NetConnection* conn = NetConnection::getConnectionToServer();
        if (!conn)
          return 0;
        return dynamic_cast<SceneObject*>(conn->resolveGhost(ghost_ids[i]));
      }
  }

  return dynamic_cast<SceneObject*>(Sim::findObject(name));
}

void afxConstraintMgr::addScopeableObject(SceneObject* object)
{
  for (S32 i = 0; i < scopeable_objs.size(); i++)
  {
    if (scopeable_objs[i] == object)
      return;
  }

  object->addScopeRef();
  scopeable_objs.push_back(object);
}

void afxConstraintMgr::removeScopeableObject(SceneObject* object)
{
  for (S32 i = 0; i < scopeable_objs.size(); i++)
    if (scopeable_objs[i] == object)
    {
      object->removeScopeRef();
      scopeable_objs.erase_fast(i);
      return;
    }
}

void afxConstraintMgr::clearAllScopeableObjs()
{
  for (S32 i = 0; i < scopeable_objs.size(); i++)
    scopeable_objs[i]->removeScopeRef();
  scopeable_objs.clear();
}

void afxConstraintMgr::postMissingConstraintObject(afxConstraint* cons, bool is_deleting)
{
  if (cons->gone_missing)
    return;

  if (!is_deleting)
  {
    SceneObject* obj = arcaneFX::findScopedObject(cons->getScopeId());
    if (obj)
    {
      cons->restoreObject(obj);
      return;
    }
  }

  cons->gone_missing = true;
  missing_objs->push_back(cons);
}

void afxConstraintMgr::restoreScopedObject(SceneObject* obj, afxChoreographer* ch)
{
  for (S32 i = 0; i < missing_objs->size(); i++)
  {
    if ((*missing_objs)[i]->getScopeId() == obj->getScopeId())
    {
      (*missing_objs)[i]->gone_missing = false;
      (*missing_objs)[i]->restoreObject(obj);
      if (ch)
        ch->restoreObject(obj);
    }
    else
      missing_objs2->push_back((*missing_objs)[i]);
  }

  Vector<afxConstraint*>* tmp = missing_objs;
  missing_objs = missing_objs2;
  missing_objs2 = tmp;
  missing_objs2->clear();
}

void afxConstraintMgr::adjustProcessOrdering(afxChoreographer* ch)
{
  Vector<ProcessObject*> cons_sources;

  // add choreographer to the list
  cons_sources.push_back(ch);

  // collect all the ProcessObject related constraint sources
  for (S32 i = 0; i < constraints_v.size(); i++)
  {
    afxConstraintList* list = constraints_v[i];
    afxConstraint* cons = (*list)[0];
    if (cons)
    {
      ProcessObject* pobj = dynamic_cast<ProcessObject*>(cons->getSceneObject());
      if (pobj)
        cons_sources.push_back(pobj);
    }
  }

  ProcessList* proc_list;
  if (ch->isServerObject())
    proc_list = ServerProcessList::get();
  else
    proc_list = ClientProcessList::get();
  if (!proc_list)
    return;

  GameBase* nearest_to_end = dynamic_cast<GameBase*>(proc_list->findNearestToEnd(cons_sources));
  GameBase* chor = (GameBase*) ch;

  // choreographer needs to be processed after the latest process object
  if (chor != nearest_to_end && nearest_to_end != 0)
  {
    //Con::printf("Choreographer processing BEFORE some of its constraints... fixing. [%s] -- %s",
    //   (ch->isServerObject()) ? "S" : "C", nearest_to_end->getClassName());
    if (chor->isProperlyAdded())
      ch->processAfter(nearest_to_end);
    else
      ch->postProcessAfterObject(nearest_to_end);
  }
  else
  {
    //Con::printf("Choreographer processing AFTER its constraints... fine. [%s]",
    //   (ch->isServerObject()) ? "S" : "C");
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxPointConstraint

afxPointConstraint::afxPointConstraint(afxConstraintMgr* mgr) 
  : afxConstraint(mgr)
{
  point.zero();
  vector.set(0,0,1);
}

afxPointConstraint::~afxPointConstraint()
{
}

void afxPointConstraint::set(Point3F point, Point3F vector)
{
  this->point = point;
  this->vector = vector;
  is_defined = true;
  is_valid = true;
  change_code++;
  sample(0.0f, 0, 0);
}

void afxPointConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  if (cam_pos)
  {
    Point3F dir = (*cam_pos) - point;
    F32 dist_sq = dir.lenSquared();
    if (dist_sq > mgr->getScopingDistanceSquared())
    {
      is_valid = false;
      return;
    }
    is_valid = true;
  }

  last_pos = point;
  last_xfm.identity();
  last_xfm.setPosition(point);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxTransformConstraint

afxTransformConstraint::afxTransformConstraint(afxConstraintMgr* mgr) 
  : afxConstraint(mgr)
{
  xfm.identity();
}

afxTransformConstraint::~afxTransformConstraint()
{
}

void afxTransformConstraint::set(const MatrixF& xfm)
{
  this->xfm = xfm;
  is_defined = true;
  is_valid = true;
  change_code++;
  sample(0.0f, 0, 0);
}

void afxTransformConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  if (cam_pos)
  {
    Point3F dir = (*cam_pos) - xfm.getPosition();
    F32 dist_sq = dir.lenSquared();
    if (dist_sq > mgr->getScopingDistanceSquared())
    {
      is_valid = false;
      return;
    }
    is_valid = true;
  }

  last_xfm = xfm;
  last_pos = xfm.getPosition();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxShapeConstraint

afxShapeConstraint::afxShapeConstraint(afxConstraintMgr* mgr) 
  : afxConstraint(mgr)
{
  arb_name = ST_NULLSTRING;
  shape = 0;
  scope_id = 0;
  clip_tag = 0;
  lock_tag = 0;
}

afxShapeConstraint::afxShapeConstraint(afxConstraintMgr* mgr, StringTableEntry arb_name) 
  : afxConstraint(mgr)
{
  this->arb_name = arb_name;
  shape = 0;
  scope_id = 0;
  clip_tag = 0;
  lock_tag = 0;
}

afxShapeConstraint::~afxShapeConstraint()
{
  if (shape)
    clearNotify(shape);
}

void afxShapeConstraint::set(ShapeBase* shape)
{
  if (this->shape)
  {
    scope_id = 0;
    clearNotify(this->shape);
    if (clip_tag > 0)
      remapAnimation(clip_tag, shape);
    if (lock_tag > 0)
      unlockAnimation(lock_tag);
  }

  this->shape = shape;

  if (this->shape)
  {
    deleteNotify(this->shape);
    scope_id = this->shape->getScopeId();
  }

  if (this->shape != NULL)
  {
    is_defined = true;
    is_valid = true;
    change_code++;
    sample(0.0f, 0, 0);
  }
  else
    is_valid = false;
}

void afxShapeConstraint::set_scope_id(U16 scope_id)
{
  if (shape)
    clearNotify(shape);

  shape = 0;
  this->scope_id = scope_id;

  is_defined = (this->scope_id > 0);
  is_valid = false;
  mgr->postMissingConstraintObject(this);
}

void afxShapeConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  if (gone_missing)
    return;

  if (shape)
  {
    last_xfm = shape->getRenderTransform();
    if (cons_def.pos_at_box_center)
      last_pos = shape->getBoxCenter();
    else
      last_pos = shape->getRenderPosition();
  }
}

void afxShapeConstraint::restoreObject(SceneObject* obj) 
{ 
  if (this->shape)
  {
    scope_id = 0;
    clearNotify(this->shape);
  }

  this->shape = (ShapeBase* )obj;

  if (this->shape)
  {
    deleteNotify(this->shape);
    scope_id = this->shape->getScopeId();
  }

  is_valid = (this->shape != NULL);
}

void afxShapeConstraint::onDeleteNotify(SimObject* obj)
{
  if (shape == dynamic_cast<ShapeBase*>(obj))
  {
    shape = 0;
    is_valid = false;
    if (scope_id > 0)
      mgr->postMissingConstraintObject(this, true);
  }

  Parent::onDeleteNotify(obj);
}

U32 afxShapeConstraint::setAnimClip(const char* clip, F32 pos, F32 rate, F32 trans, bool is_death_anim)
{
  if (!shape)
    return 0;

  if (shape->isServerObject())
  {
    AIPlayer* ai_player = dynamic_cast<AIPlayer*>(shape);
    if (ai_player && !ai_player->isBlendAnimation(clip))
    {
      ai_player->saveMoveState();
      ai_player->stopMove();
    }
  }

  clip_tag = shape->playAnimation(clip, pos, rate, trans, false/*hold*/, true/*wait*/, is_death_anim);
  return clip_tag;
}

void afxShapeConstraint::remapAnimation(U32 tag, ShapeBase* other_shape)
{
  if (clip_tag == 0)
    return;

  if (!shape)
    return;

  if (!other_shape)
  {
    resetAnimation(tag);
    return;
  }

  Con::errorf("remapAnimation -- Clip name, %s.", shape->getLastClipName(tag));

  if (shape->isClientObject())
  {
    shape->restoreAnimation(tag);
  }
  else
  {
    AIPlayer* ai_player = dynamic_cast<AIPlayer*>(shape);
    if (ai_player)
      ai_player->restartMove(tag);
    else
      shape->restoreAnimation(tag);
  }

  clip_tag = 0;
}

void afxShapeConstraint::resetAnimation(U32 tag)
{
  if (clip_tag == 0)
    return;

  if (!shape)
    return;
  
  if (shape->isClientObject())
  {
    shape->restoreAnimation(tag);
  }
  else
  {
    AIPlayer* ai_player = dynamic_cast<AIPlayer*>(shape);
    if (ai_player)
      ai_player->restartMove(tag);
    else
      shape->restoreAnimation(tag);
  }

  if ((tag & 0x80000000) == 0 && tag == clip_tag)
    clip_tag = 0;
}

U32 afxShapeConstraint::lockAnimation()
{
  if (!shape)
    return 0;

  lock_tag = shape->lockAnimation();
  return lock_tag;
}

void afxShapeConstraint::unlockAnimation(U32 tag)
{
  if (lock_tag == 0)
    return;

  if (!shape)
    return;
  
  shape->unlockAnimation(tag);
  lock_tag = 0;
}

F32 afxShapeConstraint::getAnimClipDuration(const char* clip)
{
  return (shape) ? shape->getAnimationDuration(clip) : 0.0f;
}

S32 afxShapeConstraint::getDamageState()
{
  return (shape) ? shape->getDamageState() : -1;
}

U32 afxShapeConstraint::getTriggers()
{
  return (shape) ? shape->getShapeInstance()->getTriggerStateMask() : 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxShapeNodeConstraint

afxShapeNodeConstraint::afxShapeNodeConstraint(afxConstraintMgr* mgr)  
  : afxShapeConstraint(mgr)
{
  arb_node = ST_NULLSTRING;
  shape_node_ID = -1;
}

afxShapeNodeConstraint::afxShapeNodeConstraint(afxConstraintMgr* mgr, StringTableEntry arb_name, StringTableEntry arb_node)
  : afxShapeConstraint(mgr, arb_name)
{
  this->arb_node = arb_node;
  shape_node_ID = -1;
}

void afxShapeNodeConstraint::set(ShapeBase* shape)
{
  if (shape)
  {
    shape_node_ID = shape->getShape()->findNode(arb_node);
    if (shape_node_ID == -1)
      Con::errorf("Failed to find node [%s]", arb_node);
  }
  else
    shape_node_ID = -1;

  Parent::set(shape);
}

void afxShapeNodeConstraint::set_scope_id(U16 scope_id)
{
  shape_node_ID = -1;
  Parent::set_scope_id(scope_id);
}

void afxShapeNodeConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  if (shape && shape_node_ID != -1)
  {
    last_xfm = shape->getRenderTransform();
    last_xfm.scale(shape->getScale());
    last_xfm.mul(shape->getShapeInstance()->mNodeTransforms[shape_node_ID]);
    last_pos = last_xfm.getPosition();
  }
}

void afxShapeNodeConstraint::restoreObject(SceneObject* obj) 
{ 
  ShapeBase* shape = dynamic_cast<ShapeBase*>(obj);
  if (shape)
  {
    shape_node_ID = shape->getShape()->findNode(arb_node);
    if (shape_node_ID == -1)
      Con::errorf("Failed to find node [%s]", arb_node);
  }
  else
    shape_node_ID = -1;
  Parent::restoreObject(obj);
}

void afxShapeNodeConstraint::onDeleteNotify(SimObject* obj)
{
  Parent::onDeleteNotify(obj);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxObjectConstraint

afxObjectConstraint::afxObjectConstraint(afxConstraintMgr* mgr) 
  : afxConstraint(mgr)
{
  arb_name = ST_NULLSTRING;
  obj = 0;
  scope_id = 0;
  is_camera = false;
}

afxObjectConstraint::afxObjectConstraint(afxConstraintMgr* mgr, StringTableEntry arb_name) 
  : afxConstraint(mgr)
{
  this->arb_name = arb_name;
  obj = 0;
  scope_id = 0;
  is_camera = false;
}

afxObjectConstraint::~afxObjectConstraint()
{
  if (obj)
    clearNotify(obj);
}

void afxObjectConstraint::set(SceneObject* obj)
{
  if (this->obj)
  {
    scope_id = 0;
    clearNotify(this->obj);
  }

  this->obj = obj;

  if (this->obj)
  {
    deleteNotify(this->obj);
    scope_id = this->obj->getScopeId();
  }

  if (this->obj != NULL)
  {
    is_camera = this->obj->isCamera();

    is_defined = true;
    is_valid = true;
    change_code++;
    sample(0.0f, 0, 0);
  }
  else
    is_valid = false;
}

void afxObjectConstraint::set_scope_id(U16 scope_id)
{
  if (obj)
    clearNotify(obj);

  obj = 0;
  this->scope_id = scope_id;

  is_defined = (scope_id > 0);
  is_valid = false;
  mgr->postMissingConstraintObject(this);
}

void afxObjectConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  if (gone_missing)
    return;

  if (obj)
  {
    if (!is_camera && cons_def.treat_as_camera && dynamic_cast<ShapeBase*>(obj))
    {
      ShapeBase* cam_obj = (ShapeBase*) obj;
      F32 pov = 1.0f;
      cam_obj->getCameraTransform(&pov, &last_xfm);
      last_xfm.getColumn(3, &last_pos);
    }
    else
    {
      last_xfm = obj->getRenderTransform();
      if (cons_def.pos_at_box_center)
        last_pos = obj->getBoxCenter();
      else
        last_pos = obj->getRenderPosition();
    }
  }
}

void afxObjectConstraint::restoreObject(SceneObject* obj)
{
  if (this->obj)
  {
    scope_id = 0;
    clearNotify(this->obj);
  }

  this->obj = obj;

  if (this->obj)
  {
    deleteNotify(this->obj);
    scope_id = this->obj->getScopeId();
  }

  is_valid = (this->obj != NULL);
}

void afxObjectConstraint::onDeleteNotify(SimObject* obj)
{
  if (this->obj == dynamic_cast<SceneObject*>(obj))
  {
    this->obj = 0;
    is_valid = false;
    if (scope_id > 0)
      mgr->postMissingConstraintObject(this, true);
  }

  Parent::onDeleteNotify(obj);
}

U32 afxObjectConstraint::getTriggers()
{
  TSStatic* ts_static = dynamic_cast<TSStatic*>(obj);
  if (ts_static)
  {
    TSShapeInstance* obj_inst = ts_static->getShapeInstance();
    return (obj_inst) ? obj_inst->getTriggerStateMask() : 0;
  }

  ShapeBase* shape_base = dynamic_cast<ShapeBase*>(obj);
  if (shape_base)
  {
    TSShapeInstance* obj_inst = shape_base->getShapeInstance();
    return (obj_inst) ? obj_inst->getTriggerStateMask() : 0;
  }

  return 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectConstraint

afxEffectConstraint::afxEffectConstraint(afxConstraintMgr* mgr) 
  : afxConstraint(mgr)
{
  effect_name = ST_NULLSTRING;
  effect = 0;
}

afxEffectConstraint::afxEffectConstraint(afxConstraintMgr* mgr, StringTableEntry effect_name) 
  : afxConstraint(mgr)
{
  this->effect_name = effect_name;
  effect = 0;
}

afxEffectConstraint::~afxEffectConstraint()
{
}

bool afxEffectConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  if (!effect || !effect->inScope())
    return false;
 
  if (cons_def.pos_at_box_center)
    effect->getUpdatedBoxCenter(pos);
  else
    effect->getUpdatedPosition(pos);
  
  return true;
}

bool afxEffectConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  if (!effect || !effect->inScope())
    return false;
  
  effect->getUpdatedTransform(xfm);
  return true;
}

bool afxEffectConstraint::getAltitudes(F32& terrain_alt, F32& interior_alt) 
{ 
  if (!effect)
    return false;
  
  effect->getAltitudes(terrain_alt, interior_alt);
  return true;
}

void afxEffectConstraint::set(afxEffectWrapper* effect)
{
  this->effect = effect;

  if (this->effect != NULL)
  {
    is_defined = true;
    is_valid = true;
    change_code++;
  }
  else
    is_valid = false;
}

U32 afxEffectConstraint::setAnimClip(const char* clip, F32 pos, F32 rate, F32 trans, bool is_death_anim)
{
  return (effect) ? effect->setAnimClip(clip, pos, rate, trans) : 0;
}

void afxEffectConstraint::resetAnimation(U32 tag)
{
  if (effect)
    effect->resetAnimation(tag);
}

F32 afxEffectConstraint::getAnimClipDuration(const char* clip)
{
  return (effect) ? getAnimClipDuration(clip) : 0;
}

U32 afxEffectConstraint::getTriggers()
{
  return (effect) ? effect->getTriggers() : 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectNodeConstraint

afxEffectNodeConstraint::afxEffectNodeConstraint(afxConstraintMgr* mgr) 
  : afxEffectConstraint(mgr)
{
  effect_node = ST_NULLSTRING;
  effect_node_ID = -1;
}

afxEffectNodeConstraint::afxEffectNodeConstraint(afxConstraintMgr* mgr, StringTableEntry name, StringTableEntry node)
: afxEffectConstraint(mgr, name)
{
  this->effect_node = node;
  effect_node_ID = -1;
}



bool afxEffectNodeConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  if (!effect || !effect->inScope())
    return false;
  
  TSShapeInstance* ts_shape_inst = effect->getTSShapeInstance();
  if (!ts_shape_inst)
    return false;

  if (effect_node_ID == -1)
  {
    TSShape* ts_shape = effect->getTSShape();
    effect_node_ID = (ts_shape) ? ts_shape->findNode(effect_node) : -1;
  }

  if (effect_node_ID == -1)
    return false;

  effect->getUpdatedTransform(last_xfm);

  Point3F scale;
  effect->getUpdatedScale(scale);

  MatrixF gag = ts_shape_inst->mNodeTransforms[effect_node_ID];
  gag.setPosition( gag.getPosition()*scale );

  MatrixF xfm;
  xfm.mul(last_xfm, gag);
  //
  pos = xfm.getPosition();

  return true;
}

bool afxEffectNodeConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  if (!effect || !effect->inScope())
    return false;
  
  TSShapeInstance* ts_shape_inst = effect->getTSShapeInstance();
  if (!ts_shape_inst)
    return false;

  if (effect_node_ID == -1)
  {
    TSShape* ts_shape = effect->getTSShape();
    effect_node_ID = (ts_shape) ? ts_shape->findNode(effect_node) : -1;
  }

  if (effect_node_ID == -1)
    return false;

  effect->getUpdatedTransform(last_xfm);

  Point3F scale;
  effect->getUpdatedScale(scale);

  MatrixF gag = ts_shape_inst->mNodeTransforms[effect_node_ID];
  gag.setPosition( gag.getPosition()*scale );

  xfm.mul(last_xfm, gag);

  return true;
}

void afxEffectNodeConstraint::set(afxEffectWrapper* effect)
{
  if (effect)
  {
    TSShape* ts_shape = effect->getTSShape();
    effect_node_ID = (ts_shape) ? ts_shape->findNode(effect_node) : -1;
  }
  else
    effect_node_ID = -1;

  Parent::set(effect);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxSampleBuffer

afxSampleBuffer::afxSampleBuffer()
{
  buffer_sz = 0;
  buffer_ms = 0;
  ms_per_sample = 33;
  elapsed_ms = 0;
  last_sample_ms = 0;
  next_sample_num = 0;
  n_samples = 0;
}

afxSampleBuffer::~afxSampleBuffer()
{
}

void afxSampleBuffer::configHistory(F32 hist_len, U8 sample_rate)
{
  buffer_sz = mCeil(hist_len*sample_rate) + 1;
  ms_per_sample = mCeil(1000.0f/sample_rate);
  buffer_ms = buffer_sz*ms_per_sample;
}

void afxSampleBuffer::recordSample(F32 dt, U32 elapsed_ms, void* data)
{
  this->elapsed_ms = elapsed_ms;

  if (!data)
    return;

  U32 now_sample_num = elapsed_ms/ms_per_sample;
  if (next_sample_num <= now_sample_num)
  {
    last_sample_ms = elapsed_ms;
    while (next_sample_num <= now_sample_num)
    {
      recSample(next_sample_num % buffer_sz, data);
      next_sample_num++;
      n_samples++;
    }
  }
}

inline bool afxSampleBuffer::compute_idx_from_lag(F32 lag, U32& idx) 
{ 
  bool in_bounds = true;

  U32 lag_ms = lag*1000.0f;
  U32 rec_ms = (elapsed_ms < buffer_ms) ? elapsed_ms : buffer_ms;
  if (lag_ms > rec_ms)
  {
    // hasn't produced enough history
    lag_ms = rec_ms;
    in_bounds = false;
  }

  U32 latest_sample_num = last_sample_ms/ms_per_sample;
  U32 then_sample_num = (elapsed_ms - lag_ms)/ms_per_sample;

  if (then_sample_num > latest_sample_num)
  {
    // latest sample is older than lag
    then_sample_num = latest_sample_num;
    in_bounds = false;
  }

  idx = then_sample_num % buffer_sz;
  return in_bounds;
}

inline bool afxSampleBuffer::compute_idx_from_lag(F32 lag, U32& idx1, U32& idx2, F32& t) 
{ 
  bool in_bounds = true;

  F32 lag_ms = lag*1000.0f;
  F32 rec_ms = (elapsed_ms < buffer_ms) ? elapsed_ms : buffer_ms;
  if (lag_ms > rec_ms)
  {
    // hasn't produced enough history
    lag_ms = rec_ms;
    in_bounds = false;
  }

  F32 per_samp = ms_per_sample;
  F32 latest_sample_num = last_sample_ms/per_samp;
  F32 then_sample_num = (elapsed_ms - lag_ms)/per_samp;

  U32 latest_sample_num_i = latest_sample_num;
  U32 then_sample_num_i = then_sample_num;

  if (then_sample_num_i >= latest_sample_num_i)
  {
    if (latest_sample_num_i < then_sample_num_i)
      in_bounds = false;
    t = 0.0;
    idx1 = then_sample_num_i % buffer_sz;
    idx2 = idx1;
  }
  else
  {
    t = then_sample_num - then_sample_num_i;
    idx1 = then_sample_num_i % buffer_sz;
    idx2 = (then_sample_num_i+1) % buffer_sz;
  }

  return in_bounds;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxSampleXfmBuffer

afxSampleXfmBuffer::afxSampleXfmBuffer()
{
  xfm_buffer = 0;
}

afxSampleXfmBuffer::~afxSampleXfmBuffer()
{
  delete [] xfm_buffer;
}

void afxSampleXfmBuffer::configHistory(F32 hist_len, U8 sample_rate)
{
  if (!xfm_buffer)
  {
    afxSampleBuffer::configHistory(hist_len, sample_rate);
    if (buffer_sz > 0)
      xfm_buffer = new MatrixF[buffer_sz];
  }  
}

void afxSampleXfmBuffer::recSample(U32 idx, void* data)
{
  xfm_buffer[idx] = *((MatrixF*)data);
}

void afxSampleXfmBuffer::getSample(F32 lag, void* data, bool& in_bounds) 
{ 
  U32 idx1, idx2;
  F32 t;
  in_bounds = compute_idx_from_lag(lag, idx1, idx2, t);

  if (idx1 == idx2)
  {
    MatrixF* m1 = &xfm_buffer[idx1];
    *((MatrixF*)data) = *m1;
  }
  else
  {
    MatrixF* m1 = &xfm_buffer[idx1];
    MatrixF* m2 = &xfm_buffer[idx2];

    Point3F p1 = m1->getPosition();
    Point3F p2 = m2->getPosition();
    Point3F p; p.interpolate(p1, p2, t);

    if (t < 0.5f)
      *((MatrixF*)data) = *m1;
    else
      *((MatrixF*)data) = *m2;

    ((MatrixF*)data)->setPosition(p);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxPointHistConstraint

afxPointHistConstraint::afxPointHistConstraint(afxConstraintMgr* mgr)
  : afxPointConstraint(mgr)
{
  samples = 0;
}

afxPointHistConstraint::~afxPointHistConstraint()
{
  delete samples;
}

void afxPointHistConstraint::set(Point3F point, Point3F vector)
{
  if (!samples)
  {
    samples = new afxSampleXfmBuffer;
    samples->configHistory(cons_def.history_time, cons_def.sample_rate);
  }
  
  Parent::set(point, vector);
}

void afxPointHistConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  Parent::sample(dt, elapsed_ms, cam_pos);

  if (isDefined())
  {
    if (isValid())
      samples->recordSample(dt, elapsed_ms, &last_xfm);
    else
      samples->recordSample(dt, elapsed_ms, 0);
  }
}

bool afxPointHistConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  bool in_bounds;

  MatrixF xfm;
  samples->getSample(hist, &xfm, in_bounds);

  pos = xfm.getPosition();

  return in_bounds;
}

bool afxPointHistConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  bool in_bounds;

  samples->getSample(hist, &xfm, in_bounds);

  return in_bounds; 
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxPointHistConstraint

afxTransformHistConstraint::afxTransformHistConstraint(afxConstraintMgr* mgr)
  : afxTransformConstraint(mgr)
{
  samples = 0;
}

afxTransformHistConstraint::~afxTransformHistConstraint()
{
  delete samples;
}

void afxTransformHistConstraint::set(const MatrixF& xfm)
{
  if (!samples)
  {
    samples = new afxSampleXfmBuffer;
    samples->configHistory(cons_def.history_time, cons_def.sample_rate);
  }
  
  Parent::set(xfm);
}

void afxTransformHistConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  Parent::sample(dt, elapsed_ms, cam_pos);

  if (isDefined())
  {
    if (isValid())
      samples->recordSample(dt, elapsed_ms, &last_xfm);
    else
      samples->recordSample(dt, elapsed_ms, 0);
  }
}

bool afxTransformHistConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  bool in_bounds;

  MatrixF xfm;
  samples->getSample(hist, &xfm, in_bounds);

  pos = xfm.getPosition();

  return in_bounds;
}

bool afxTransformHistConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  bool in_bounds;

  samples->getSample(hist, &xfm, in_bounds);

  return in_bounds; 
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxShapeHistConstraint

afxShapeHistConstraint::afxShapeHistConstraint(afxConstraintMgr* mgr)
  : afxShapeConstraint(mgr)
{
  samples = 0;
}

afxShapeHistConstraint::afxShapeHistConstraint(afxConstraintMgr* mgr, StringTableEntry arb_name)
  : afxShapeConstraint(mgr, arb_name)
{
  samples = 0;
}

afxShapeHistConstraint::~afxShapeHistConstraint()
{
  delete samples;
}

void afxShapeHistConstraint::set(ShapeBase* shape)
{
  if (shape && !samples)
  {
    samples = new afxSampleXfmBuffer;
    samples->configHistory(cons_def.history_time, cons_def.sample_rate);
  }
  
  Parent::set(shape);
}

void afxShapeHistConstraint::set_scope_id(U16 scope_id)
{
  if (scope_id > 0 && !samples)
  {
    samples = new afxSampleXfmBuffer;
    samples->configHistory(cons_def.history_time, cons_def.sample_rate);
  }
  
  Parent::set_scope_id(scope_id);
}

void afxShapeHistConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  Parent::sample(dt, elapsed_ms, cam_pos);

  if (isDefined())
  {
    if (isValid())
      samples->recordSample(dt, elapsed_ms, &last_xfm);
    else
      samples->recordSample(dt, elapsed_ms, 0);
  }
}

bool afxShapeHistConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  bool in_bounds;

  MatrixF xfm;
  samples->getSample(hist, &xfm, in_bounds);

  pos = xfm.getPosition();

  return in_bounds;
}

bool afxShapeHistConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  bool in_bounds;

  samples->getSample(hist, &xfm, in_bounds);

  return in_bounds; 
}

void afxShapeHistConstraint::onDeleteNotify(SimObject* obj)
{
  Parent::onDeleteNotify(obj);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxShapeNodeHistConstraint

afxShapeNodeHistConstraint::afxShapeNodeHistConstraint(afxConstraintMgr* mgr)
  : afxShapeNodeConstraint(mgr)
{
  samples = 0;
}

afxShapeNodeHistConstraint::afxShapeNodeHistConstraint(afxConstraintMgr* mgr, StringTableEntry arb_name,
                                                       StringTableEntry arb_node)
  : afxShapeNodeConstraint(mgr, arb_name, arb_node)
{
  samples = 0;
}

afxShapeNodeHistConstraint::~afxShapeNodeHistConstraint()
{
  delete samples;
}

void afxShapeNodeHistConstraint::set(ShapeBase* shape)
{
  if (shape && !samples)
  {
    samples = new afxSampleXfmBuffer;
    samples->configHistory(cons_def.history_time, cons_def.sample_rate);
  }
  
  Parent::set(shape);
}

void afxShapeNodeHistConstraint::set_scope_id(U16 scope_id)
{
  if (scope_id > 0 && !samples)
  {
    samples = new afxSampleXfmBuffer;
    samples->configHistory(cons_def.history_time, cons_def.sample_rate);
  }
  
  Parent::set_scope_id(scope_id);
}

void afxShapeNodeHistConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  Parent::sample(dt, elapsed_ms, cam_pos);

  if (isDefined())
  {
    if (isValid())
      samples->recordSample(dt, elapsed_ms, &last_xfm);
    else
      samples->recordSample(dt, elapsed_ms, 0);
  }
}

bool afxShapeNodeHistConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  bool in_bounds;

  MatrixF xfm;
  samples->getSample(hist, &xfm, in_bounds);

  pos = xfm.getPosition();

  return in_bounds;
}

bool afxShapeNodeHistConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  bool in_bounds;

  samples->getSample(hist, &xfm, in_bounds);

  return in_bounds; 
}

void afxShapeNodeHistConstraint::onDeleteNotify(SimObject* obj)
{
  Parent::onDeleteNotify(obj);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxObjectHistConstraint

afxObjectHistConstraint::afxObjectHistConstraint(afxConstraintMgr* mgr)
  : afxObjectConstraint(mgr)
{
  samples = 0;
}

afxObjectHistConstraint::afxObjectHistConstraint(afxConstraintMgr* mgr, StringTableEntry arb_name)
  : afxObjectConstraint(mgr, arb_name)
{
  samples = 0;
}

afxObjectHistConstraint::~afxObjectHistConstraint()
{
  delete samples;
}

void afxObjectHistConstraint::set(SceneObject* obj)
{
  if (obj && !samples)
  {
    samples = new afxSampleXfmBuffer;
    samples->configHistory(cons_def.history_time, cons_def.sample_rate);
  }
  
  Parent::set(obj);
}

void afxObjectHistConstraint::set_scope_id(U16 scope_id)
{
  if (scope_id > 0 && !samples)
  {
    samples = new afxSampleXfmBuffer;
    samples->configHistory(cons_def.history_time, cons_def.sample_rate);
  }
  
  Parent::set_scope_id(scope_id);
}

void afxObjectHistConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  Parent::sample(dt, elapsed_ms, cam_pos);

  if (isDefined())
  {
    if (isValid())
      samples->recordSample(dt, elapsed_ms, &last_xfm);
    else
      samples->recordSample(dt, elapsed_ms, 0);
  }
}

bool afxObjectHistConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  bool in_bounds;

  MatrixF xfm;
  samples->getSample(hist, &xfm, in_bounds);

  pos = xfm.getPosition();

  return in_bounds;
}

bool afxObjectHistConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  bool in_bounds;

  samples->getSample(hist, &xfm, in_bounds);

  return in_bounds; 
}

void afxObjectHistConstraint::onDeleteNotify(SimObject* obj)
{
  Parent::onDeleteNotify(obj);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

