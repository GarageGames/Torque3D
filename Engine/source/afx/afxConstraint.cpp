
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
  return (mDef_type != CONS_UNDEFINED); 
}

bool afxConstraintDef::isArbitraryObject() 
{ 
  return ((mCons_src_name != ST_NULLSTRING) && (mDef_type == CONS_SCENE)); 
}

void afxConstraintDef::reset()
{
  mCons_src_name = ST_NULLSTRING;
  mCons_node_name = ST_NULLSTRING;
  mDef_type = CONS_UNDEFINED;
  mHistory_time = 0;
  mSample_rate = 30;
  mRuns_on_server = false;
  mRuns_on_client = false;
  mPos_at_box_center = false;
  mTreat_as_camera = false;
}

bool afxConstraintDef::parseSpec(const char* spec, bool runs_on_server, 
                                 bool runs_on_client)
{
  reset();

  if (spec == 0 || spec[0] == '\0')
    return false;

  mHistory_time = 0.0f;
  mSample_rate = 30;

  mRuns_on_server = runs_on_server;
  mRuns_on_client = runs_on_client;

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
      mPos_at_box_center = true;
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
          mHistory_time = hist_age;
        if (args > 1)
          mSample_rate = hist_rate;
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

    mCons_src_name = cons_name_key;
    mDef_type = CONS_PREDEFINED;
    dFree(buffer);
    return true;
  }

  // "#scene.NAME" or "#scene.NAME.NODE""
  if (cons_name_key == SCENE_CONS_KEY)
  {
    mCons_src_name = StringTable->insert(words2[1]);
    if (n_words2 > 2)
      mCons_node_name = StringTable->insert(words2[2]);
    mDef_type = CONS_SCENE;
    dFree(buffer);
    return true;
  }

  // "#effect.NAME" or "#effect.NAME.NODE"
  if (cons_name_key == EFFECT_CONS_KEY)
  {
    mCons_src_name = StringTable->insert(words2[1]);
    if (n_words2 > 2)
      mCons_node_name = StringTable->insert(words2[2]);
    mDef_type = CONS_EFFECT;
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

    mCons_src_name = StringTable->insert(words2[1]);
    if (n_words2 > 2)
      mCons_node_name = StringTable->insert(words2[2]);
    mDef_type = CONS_GHOST;
    dFree(buffer);
    return true;
  }

  // "CONSTRAINT_NAME.NODE"
  if (n_words2 == 2)
  {
    mCons_src_name = cons_name_key;
    mCons_node_name = StringTable->insert(words2[1]);
    mDef_type = CONS_PREDEFINED;
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
  mMgr = mgr;
  mIs_defined = false;
  mIs_valid = false;
  mLast_pos.zero();
  mLast_xfm.identity();
  mHistory_time = 0.0f;
  mIs_alive = true;
  mGone_missing = false;
  mChange_code = 0;
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

#define CONS_BY_ID(id) ((*mConstraints_v[(id).index])[(id).sub_index])
#define CONS_BY_IJ(i,j) ((*mConstraints_v[(i)])[(j)])

afxConstraintMgr::afxConstraintMgr()
{
  mStartTime = 0;
  mOn_server = false;
  mInitialized = false;
  mScoping_dist_sq = 1000.0f*1000.0f;
  missing_objs = &missing_objs_a;
  missing_objs2 = &missing_objs_b;
}

afxConstraintMgr::~afxConstraintMgr()
{
  for (S32 i = 0; i < mConstraints_v.size(); i++)
  {
    for (S32 j = 0; j < (*mConstraints_v[i]).size(); j++)
      delete CONS_BY_IJ(i,j);
    delete mConstraints_v[i];
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

S32 afxConstraintMgr::find_cons_idx_from_name(StringTableEntry which)
{
  for (S32 i = 0; i < mConstraints_v.size(); i++)
  {
    afxConstraint* cons = CONS_BY_IJ(i,0);
    if (cons && afxConstraintDef::CONS_EFFECT != cons->mCons_def.mDef_type && 
        which == cons->mCons_def.mCons_src_name)
    {
        return i;
    }
  }

  return -1;
}

S32 afxConstraintMgr::find_effect_cons_idx_from_name(StringTableEntry which)
{
  for (S32 i = 0; i < mConstraints_v.size(); i++)
  {
    afxConstraint* cons = CONS_BY_IJ(i,0);
    if (cons && afxConstraintDef::CONS_EFFECT == cons->mCons_def.mDef_type &&
        which == cons->mCons_def.mCons_src_name)
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
  mPredefs.push_back(predef);
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
  cons->mCons_def.mDef_type = afxConstraintDef::CONS_EFFECT;
  cons->mCons_def.mCons_src_name = which;
  afxConstraintList* list = new afxConstraintList();
  list->push_back(cons);
  mConstraints_v.push_back(list);

  return setReferenceEffect(which, ew);
}

void afxConstraintMgr::setReferencePoint(afxConstraintID id, Point3F point, Point3F vector)
{
  afxPointConstraint* pt_cons = dynamic_cast<afxPointConstraint*>(CONS_BY_ID(id));

  // need to change type
  if (!pt_cons)
  {
    afxConstraint* cons = CONS_BY_ID(id);
    pt_cons = newPointCons(this, cons->mCons_def.mHistory_time > 0.0f);
    pt_cons->mCons_def = cons->mCons_def;
    CONS_BY_ID(id) = pt_cons;
    delete cons;
  }

  pt_cons->set(point, vector);

  // nullify all subnodes
  for (S32 j = 1; j < (*mConstraints_v[id.index]).size(); j++)
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
    xfm_cons = newTransformCons(this, cons->mCons_def.mHistory_time > 0.0f);
    xfm_cons->mCons_def = cons->mCons_def;
    CONS_BY_ID(id) = xfm_cons;
    delete cons;
  }

  xfm_cons->set(xfm);

  // nullify all subnodes
  for (S32 j = 1; j < (*mConstraints_v[id.index]).size(); j++)
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
    shape_cons = newShapeCons(this, cons->mCons_def.mHistory_time > 0.0f);
    shape_cons->mCons_def = cons->mCons_def;
    CONS_BY_ID(id) = shape_cons;
    delete cons;
  }

  // set new shape on root 
  shape_cons->set(shape);

  // update all subnodes
  for (S32 j = 1; j < (*mConstraints_v[id.index]).size(); j++)
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
    shape_cons = newShapeCons(this, cons->mCons_def.mHistory_time > 0.0f);
    shape_cons->mCons_def = cons->mCons_def;
    CONS_BY_ID(id) = shape_cons;
    delete cons;
  }

  // set new shape on root 
  shape_cons->set_scope_id(scope_id);

  // update all subnodes
  for (S32 j = 1; j < (*mConstraints_v[id.index]).size(); j++)
  {
    afxConstraint* cons = CONS_BY_IJ(id.index,j);
    if (cons)
      cons->set_scope_id(scope_id);
  }
}

// Assigns an existing scene-object to the constraint matching the given constraint-id.
void afxConstraintMgr::setReferenceObject(afxConstraintID id, SceneObject* obj)
{
  if (!mInitialized)
    Con::errorf("afxConstraintMgr::setReferenceObject() -- constraint manager not initialized");

  if (!CONS_BY_ID(id)->mCons_def.mTreat_as_camera)
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
    obj_cons = newObjectCons(this, cons->mCons_def.mHistory_time > 0.0f);
    obj_cons->mCons_def = cons->mCons_def;
    CONS_BY_ID(id) = obj_cons;
    delete cons;
  }

  obj_cons->set(obj);

  // update all subnodes
  for (S32 j = 1; j < (*mConstraints_v[id.index]).size(); j++)
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
  if (!mInitialized)
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
    obj_cons = newObjectCons(this, cons->mCons_def.mHistory_time > 0.0f);
    obj_cons->mCons_def = cons->mCons_def;
    CONS_BY_ID(id) = obj_cons;
    delete cons;
  }

  obj_cons->set_scope_id(scope_id);

  // update all subnodes
  for (S32 j = 1; j < (*mConstraints_v[id.index]).size(); j++)
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
  for (S32 j = 1; j < (*mConstraints_v[id.index]).size(); j++)
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
    cons->mIs_valid = false;
}

void afxConstraintMgr::create_constraint(const afxConstraintDef& def)
{
  if (def.mDef_type == afxConstraintDef::CONS_UNDEFINED)
    return;

  //Con::printf("CON - %s [%s] [%s] h=%g", def.cons_type_name, def.cons_src_name, def.cons_node_name, def.history_time);

  bool want_history = (def.mHistory_time > 0.0f);

  // constraint is an arbitrary named scene object
  //
  if (def.mDef_type == afxConstraintDef::CONS_SCENE)
  {
    if (def.mCons_src_name == ST_NULLSTRING)
      return;

    // find the arbitrary object by name
    SceneObject* arb_obj;
    if (mOn_server)
    {
      arb_obj = dynamic_cast<SceneObject*>(Sim::findObject(def.mCons_src_name));
      if (!arb_obj)
         Con::errorf("afxConstraintMgr -- failed to find scene constraint source, \"%s\" on server.", 
                     def.mCons_src_name);
    }
    else
    {
      arb_obj = find_object_from_name(def.mCons_src_name);
      if (!arb_obj)
         Con::errorf("afxConstraintMgr -- failed to find scene constraint source, \"%s\" on client.", 
                     def.mCons_src_name);
    }

    // if it's a shapeBase object, create a Shape or ShapeNode constraint
    if (dynamic_cast<ShapeBase*>(arb_obj))
    {
      if (def.mCons_node_name == ST_NULLSTRING && !def.mPos_at_box_center)
      {
        afxShapeConstraint* cons = newShapeCons(this, def.mCons_src_name, want_history);
        cons->mCons_def = def;
        cons->set((ShapeBase*)arb_obj); 
        afxConstraintList* list = new afxConstraintList();
        list->push_back(cons);
		mConstraints_v.push_back(list);
      }
      else if (def.mPos_at_box_center)
      {
        afxShapeConstraint* cons = newShapeCons(this, def.mCons_src_name, want_history);
        cons->mCons_def = def;
        cons->set((ShapeBase*)arb_obj); 
        afxConstraintList* list = mConstraints_v[mConstraints_v.size()-1]; // SHAPE-NODE CONS-LIST (#scene)(#center)
        if (list && (*list)[0])
          list->push_back(cons);
      }
      else
      {
        afxShapeNodeConstraint* sub = newShapeNodeCons(this, def.mCons_src_name, def.mCons_node_name, want_history);
        sub->mCons_def = def;
        sub->set((ShapeBase*)arb_obj); 
        afxConstraintList* list = mConstraints_v[mConstraints_v.size()-1];
        if (list && (*list)[0])
          list->push_back(sub);
      }
    }
    // if it's not a shapeBase object, create an Object constraint
    else if (arb_obj)
    {
      if (!def.mPos_at_box_center)
      {
        afxObjectConstraint* cons = newObjectCons(this, def.mCons_src_name, want_history);
        cons->mCons_def = def;
        cons->set(arb_obj);
        afxConstraintList* list = new afxConstraintList(); // OBJECT CONS-LIST (#scene)
        list->push_back(cons);
		mConstraints_v.push_back(list);
      }
      else // if (def.pos_at_box_center)
      {
        afxObjectConstraint* cons = newObjectCons(this, def.mCons_src_name, want_history);
        cons->mCons_def = def;
        cons->set(arb_obj); 
        afxConstraintList* list = mConstraints_v[mConstraints_v.size()-1]; // OBJECT CONS-LIST (#scene)(#center)
        if (list && (*list)[0])
          list->push_back(cons);
      }
    }
  }

  // constraint is an arbitrary named effect
  //
  else if (def.mDef_type == afxConstraintDef::CONS_EFFECT)
  {
    if (def.mCons_src_name == ST_NULLSTRING)
      return;

    // create an Effect constraint
    if (def.mCons_node_name == ST_NULLSTRING && !def.mPos_at_box_center)
    {
      afxEffectConstraint* cons = new afxEffectConstraint(this, def.mCons_src_name);
      cons->mCons_def = def;
      afxConstraintList* list = new afxConstraintList();
      list->push_back(cons);
	  mConstraints_v.push_back(list);
    }
    // create an Effect #center constraint
    else if (def.mPos_at_box_center)
    {
      afxEffectConstraint* cons = new afxEffectConstraint(this, def.mCons_src_name);
      cons->mCons_def = def;
      afxConstraintList* list = mConstraints_v[mConstraints_v.size()-1]; // EFFECT-NODE CONS-LIST (#effect)
      if (list && (*list)[0])
        list->push_back(cons);
    }
    // create an EffectNode constraint
    else
    {
      afxEffectNodeConstraint* sub = new afxEffectNodeConstraint(this, def.mCons_src_name, def.mCons_node_name);
      sub->mCons_def = def;
      afxConstraintList* list = mConstraints_v[mConstraints_v.size()-1];
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

    if (def.mDef_type == afxConstraintDef::CONS_GHOST)
    {
      for (S32 i = 0; i < mPredefs.size(); i++)
      {
        if (mPredefs[i].name == def.mCons_src_name)
        {
          if (def.mCons_node_name == ST_NULLSTRING && !def.mPos_at_box_center)
          {
            cons = newShapeCons(this, want_history);
            cons->mCons_def = def;
          }
          else if (def.mPos_at_box_center)
          {
            cons_ctr = newShapeCons(this, want_history);
            cons_ctr->mCons_def = def;
          }
          else
          {
            sub = newShapeNodeCons(this, ST_NULLSTRING, def.mCons_node_name, want_history);
            sub->mCons_def = def;
          }
          break;
        }
      }
    }
    else
    {
      for (S32 i = 0; i < mPredefs.size(); i++)
      {
        if (mPredefs[i].name == def.mCons_src_name)
        {
          switch (mPredefs[i].type)
          {
          case POINT_CONSTRAINT:
            cons = newPointCons(this, want_history);
            cons->mCons_def = def;
            break;
          case TRANSFORM_CONSTRAINT:
            cons = newTransformCons(this, want_history);
            cons->mCons_def = def;
            break;
          case OBJECT_CONSTRAINT:
            if (def.mCons_node_name == ST_NULLSTRING && !def.mPos_at_box_center)
            {
              cons = newShapeCons(this, want_history);
              cons->mCons_def = def;
            }
            else if (def.mPos_at_box_center)
            {
              cons_ctr = newShapeCons(this, want_history);
              cons_ctr->mCons_def = def;
            }
            else
            {
              sub = newShapeNodeCons(this, ST_NULLSTRING, def.mCons_node_name, want_history);
              sub->mCons_def = def;
            }
            break;
          case CAMERA_CONSTRAINT:
            cons = newObjectCons(this, want_history);
            cons->mCons_def = def;
            cons->mCons_def.mTreat_as_camera = true;
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
	  mConstraints_v.push_back(list);
    }
    else if (cons_ctr && mConstraints_v.size() > 0)
    {
      afxConstraintList* list = mConstraints_v[mConstraints_v.size()-1]; // PREDEF-NODE CONS-LIST
      if (list && (*list)[0])
        list->push_back(cons_ctr);
    }
    else if (sub && mConstraints_v.size() > 0)
    {
      afxConstraintList* list = mConstraints_v[mConstraints_v.size()-1];
      if (list && (*list)[0])
        list->push_back(sub);
    }
    else
      Con::printf("predef not found %s", def.mCons_src_name);
  }
}

afxConstraintID afxConstraintMgr::getConstraintId(const afxConstraintDef& def)
{
  if (def.mDef_type == afxConstraintDef::CONS_UNDEFINED)
    return afxConstraintID();

  if (def.mCons_src_name != ST_NULLSTRING)
  {
    for (S32 i = 0; i < mConstraints_v.size(); i++)
    {
      afxConstraintList* list = mConstraints_v[i];
      afxConstraint* cons = (*list)[0];
      if (def.mCons_src_name == cons->mCons_def.mCons_src_name)
      {
        for (S32 j = 0; j < list->size(); j++)
        {
          afxConstraint* sub = (*list)[j];
          if (def.mCons_node_name == sub->mCons_def.mCons_node_name &&
              def.mPos_at_box_center == sub->mCons_def.mPos_at_box_center &&
              def.mCons_src_name == sub->mCons_def.mCons_src_name)
          {
            return afxConstraintID(i, j);
          }
        }

        // if we're here, it means the root object name matched but the node name
        // did not.
        if (def.mDef_type == afxConstraintDef::CONS_PREDEFINED && !def.mPos_at_box_center)
        {
          afxShapeConstraint* shape_cons = dynamic_cast<afxShapeConstraint*>(cons);
          if (shape_cons)
          {
             //Con::errorf("Append a Node constraint [%s.%s] [%d,%d]", def.cons_src_name, def.cons_node_name, i, list->size());
             bool want_history = (def.mHistory_time > 0.0f);
             afxConstraint* sub = newShapeNodeCons(this, ST_NULLSTRING, def.mCons_node_name, want_history);
             sub->mCons_def = def;
             ((afxShapeConstraint*)sub)->set(shape_cons->mShape);
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
  U32 elapsed = now - mStartTime;

  for (S32 i = 0; i < mConstraints_v.size(); i++)
  {
    afxConstraintList* list = mConstraints_v[i];
    for (S32 j = 0; j < list->size(); j++)
      (*list)[j]->sample(dt, elapsed, cam_pos);
  }
}

S32 QSORT_CALLBACK cmp_cons_defs(const void* a, const void* b)
{
  afxConstraintDef* def_a = (afxConstraintDef*) a;
  afxConstraintDef* def_b = (afxConstraintDef*) b;

  if (def_a->mDef_type == def_b->mDef_type)
  {
    if (def_a->mCons_src_name == def_b->mCons_src_name)
    {
      if (def_a->mPos_at_box_center == def_b->mPos_at_box_center)
        return (def_a->mCons_node_name - def_b->mCons_node_name);
      else
        return (def_a->mPos_at_box_center) ? 1 : -1;
    }
    return (def_a->mCons_src_name - def_b->mCons_src_name);
  }

  return (def_a->mDef_type - def_b->mDef_type);
}

void afxConstraintMgr::initConstraintDefs(Vector<afxConstraintDef>& all_defs, bool on_server, F32 scoping_dist)
{
  mInitialized = true;
  mOn_server = on_server;

  if (scoping_dist > 0.0)
    mScoping_dist_sq = scoping_dist*scoping_dist;
  else
  {
    SceneManager* sg = (on_server) ? gServerSceneGraph : gClientSceneGraph;
    F32 vis_dist = (sg) ? sg->getVisibleDistance() : 1000.0f;
	mScoping_dist_sq = vis_dist*vis_dist;
  }

  if (all_defs.size() < 1)
    return;

  // find effect ghost constraints
  if (!on_server)
  {
    Vector<afxConstraintDef> ghost_defs;

    for (S32 i = 0; i < all_defs.size(); i++)
      if (all_defs[i].mDef_type == afxConstraintDef::CONS_GHOST && all_defs[i].mCons_src_name != ST_NULLSTRING)
        ghost_defs.push_back(all_defs[i]);
    
    if (ghost_defs.size() > 0)
    {
      // sort the defs
      if (ghost_defs.size() > 1)
        dQsort(ghost_defs.address(), ghost_defs.size(), sizeof(afxConstraintDef), cmp_cons_defs);
      
      S32 last = 0;
      defineConstraint(OBJECT_CONSTRAINT, ghost_defs[0].mCons_src_name);

      for (S32 i = 1; i < ghost_defs.size(); i++)
      {
        if (ghost_defs[last].mCons_src_name != ghost_defs[i].mCons_src_name)
        {
          defineConstraint(OBJECT_CONSTRAINT, ghost_defs[i].mCons_src_name);
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
      if (all_defs[i].mRuns_on_server)
        defs.push_back(all_defs[i]);
  }
  else
  {
    for (S32 i = 0; i < all_defs.size(); i++)
      if (all_defs[i].mRuns_on_client)
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
    if (defs[0].mCons_node_name != ST_NULLSTRING)
    {
      afxConstraintDef root_def = defs[0];
      root_def.mCons_node_name = ST_NULLSTRING;
      unique_defs.push_back(root_def);
      last++;
    }
    else if (defs[0].mPos_at_box_center)
    {
      afxConstraintDef root_def = defs[0];
      root_def.mPos_at_box_center = false;
      unique_defs.push_back(root_def);
      last++;
    }

    unique_defs.push_back(defs[0]);
    
    for (S32 i = 1; i < defs.size(); i++)
    {
      if (unique_defs[last].mCons_node_name != defs[i].mCons_node_name ||
          unique_defs[last].mCons_src_name != defs[i].mCons_src_name ||
          unique_defs[last].mPos_at_box_center != defs[i].mPos_at_box_center ||
          unique_defs[last].mDef_type != defs[i].mDef_type)
      {
        // manufacture root-object def if absent
        if (defs[i].mCons_src_name != ST_NULLSTRING && unique_defs[last].mCons_src_name != defs[i].mCons_src_name)
        {
          if (defs[i].mCons_node_name != ST_NULLSTRING || defs[i].mPos_at_box_center)
          {
            afxConstraintDef root_def = defs[i];
            root_def.mCons_node_name = ST_NULLSTRING;
            root_def.mPos_at_box_center = false;
            unique_defs.push_back(root_def);
            last++;
          }
        }
        unique_defs.push_back(defs[i]);
        last++;
      }
      else
      {
        if (defs[i].mHistory_time > unique_defs[last].mHistory_time)
          unique_defs[last].mHistory_time = defs[i].mHistory_time;
        if (defs[i].mSample_rate > unique_defs[last].mSample_rate)
          unique_defs[last].mSample_rate = defs[i].mSample_rate;
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
    mNames_on_server.clear();
    defs.clear();

    for (S32 i = 0; i < all_defs.size(); i++)
      if (all_defs[i].mRuns_on_client && all_defs[i].isArbitraryObject())
        defs.push_back(all_defs[i]);

    if (defs.size() < 1)
      return;

    // sort the defs
    if (defs.size() > 1)
      dQsort(defs.address(), defs.size(), sizeof(afxConstraintDef), cmp_cons_defs);

    S32 last = 0;
    mNames_on_server.push_back(defs[0].mCons_src_name);

    for (S32 i = 1; i < defs.size(); i++)
    {
      if (mNames_on_server[last] != defs[i].mCons_src_name)
      {
        mNames_on_server.push_back(defs[i].mCons_src_name);
        last++;
      }
    }
  }
}

void afxConstraintMgr::packConstraintNames(NetConnection* conn, BitStream* stream)
{
  // pack any named constraint names and ghost indices
  if (stream->writeFlag(mNames_on_server.size() > 0)) //-- ANY NAMED CONS_BY_ID?
  {
    stream->write(mNames_on_server.size());
    for (S32 i = 0; i < mNames_on_server.size(); i++)
    {
      stream->writeString(mNames_on_server[i]);
      NetObject* obj = dynamic_cast<NetObject*>(Sim::findObject(mNames_on_server[i]));
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
    mNames_on_server.clear();
    S32 sz; stream->read(&sz);
    for (S32 i = 0; i < sz; i++)
    {
      mNames_on_server.push_back(stream->readSTString());
      S32 ghost_id; stream->read(&ghost_id);
      mGhost_ids.push_back(ghost_id);
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

SceneObject* afxConstraintMgr::find_object_from_name(StringTableEntry name)
{
  if (mNames_on_server.size() > 0)
  {
    for (S32 i = 0; i < mNames_on_server.size(); i++)
      if (mNames_on_server[i] == name)
      {
        if (mGhost_ids[i] == -1)
          return 0;
        NetConnection* conn = NetConnection::getConnectionToServer();
        if (!conn)
          return 0;
        return dynamic_cast<SceneObject*>(conn->resolveGhost(mGhost_ids[i]));
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
  if (cons->mGone_missing)
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

  cons->mGone_missing = true;
  missing_objs->push_back(cons);
}

void afxConstraintMgr::restoreScopedObject(SceneObject* obj, afxChoreographer* ch)
{
  for (S32 i = 0; i < missing_objs->size(); i++)
  {
    if ((*missing_objs)[i]->getScopeId() == obj->getScopeId())
    {
      (*missing_objs)[i]->mGone_missing = false;
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
  for (S32 i = 0; i < mConstraints_v.size(); i++)
  {
    afxConstraintList* list = mConstraints_v[i];
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
  mPoint.zero();
  mVector.set(0,0,1);
}

afxPointConstraint::~afxPointConstraint()
{
}

void afxPointConstraint::set(Point3F point, Point3F vector)
{
  mPoint = point;
  mVector = vector;
  mIs_defined = true;
  mIs_valid = true;
  mChange_code++;
  sample(0.0f, 0, 0);
}

void afxPointConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  if (cam_pos)
  {
    Point3F dir = (*cam_pos) - mPoint;
    F32 dist_sq = dir.lenSquared();
    if (dist_sq > mMgr->getScopingDistanceSquared())
    {
      mIs_valid = false;
      return;
    }
    mIs_valid = true;
  }

  mLast_pos = mPoint;
  mLast_xfm.identity();
  mLast_xfm.setPosition(mPoint);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxTransformConstraint

afxTransformConstraint::afxTransformConstraint(afxConstraintMgr* mgr) 
  : afxConstraint(mgr)
{
   mXfm.identity();
}

afxTransformConstraint::~afxTransformConstraint()
{
}

void afxTransformConstraint::set(const MatrixF& xfm)
{
  mXfm = xfm;
  mIs_defined = true;
  mIs_valid = true;
  mChange_code++;
  sample(0.0f, 0, 0);
}

void afxTransformConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  if (cam_pos)
  {
    Point3F dir = (*cam_pos) - mXfm.getPosition();
    F32 dist_sq = dir.lenSquared();
    if (dist_sq > mMgr->getScopingDistanceSquared())
    {
      mIs_valid = false;
      return;
    }
    mIs_valid = true;
  }

  mLast_xfm = mXfm;
  mLast_pos = mXfm.getPosition();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxShapeConstraint

afxShapeConstraint::afxShapeConstraint(afxConstraintMgr* mgr) 
  : afxConstraint(mgr)
{
  mArb_name = ST_NULLSTRING;
  mShape = 0;
  mScope_id = 0;
  mClip_tag = 0;
  mLock_tag = 0;
}

afxShapeConstraint::afxShapeConstraint(afxConstraintMgr* mgr, StringTableEntry arb_name) 
  : afxConstraint(mgr)
{
  mArb_name = arb_name;
  mShape = 0;
  mScope_id = 0;
  mClip_tag = 0;
  mLock_tag = 0;
}

afxShapeConstraint::~afxShapeConstraint()
{
  if (mShape)
    clearNotify(mShape);
}

void afxShapeConstraint::set(ShapeBase* shape)
{
  if (mShape)
  {
    mScope_id = 0;
    clearNotify(mShape);
    if (mClip_tag > 0)
      remapAnimation(mClip_tag, shape);
    if (mLock_tag > 0)
      unlockAnimation(mLock_tag);
  }

  mShape = shape;

  if (mShape)
  {
    deleteNotify(mShape);
    mScope_id = mShape->getScopeId();
  }

  if (mShape != NULL)
  {
    mIs_defined = true;
    mIs_valid = true;
    mChange_code++;
    sample(0.0f, 0, 0);
  }
  else
    mIs_valid = false;
}

void afxShapeConstraint::set_scope_id(U16 scope_id)
{
  if (mShape)
    clearNotify(mShape);

  mShape = 0;
  mScope_id = scope_id;

  mIs_defined = (mScope_id > 0);
  mIs_valid = false;
  mMgr->postMissingConstraintObject(this);
}

void afxShapeConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  if (mGone_missing)
    return;

  if (mShape)
  {
    mLast_xfm = mShape->getRenderTransform();
    if (mCons_def.mPos_at_box_center)
      mLast_pos = mShape->getBoxCenter();
    else
      mLast_pos = mShape->getRenderPosition();
  }
}

void afxShapeConstraint::restoreObject(SceneObject* obj) 
{ 
  if (mShape)
  {
    mScope_id = 0;
    clearNotify(mShape);
  }

  mShape = (ShapeBase* )obj;

  if (mShape)
  {
    deleteNotify(mShape);
    mScope_id = mShape->getScopeId();
  }

  mIs_valid = (mShape != NULL);
}

void afxShapeConstraint::onDeleteNotify(SimObject* obj)
{
  if (mShape == dynamic_cast<ShapeBase*>(obj))
  {
    mShape = 0;
    mIs_valid = false;
    if (mScope_id > 0)
      mMgr->postMissingConstraintObject(this, true);
  }

  Parent::onDeleteNotify(obj);
}

U32 afxShapeConstraint::setAnimClip(const char* clip, F32 pos, F32 rate, F32 trans, bool is_death_anim)
{
  if (!mShape)
    return 0;

  if (mShape->isServerObject())
  {
    AIPlayer* ai_player = dynamic_cast<AIPlayer*>(mShape);
    if (ai_player && !ai_player->isBlendAnimation(clip))
    {
      ai_player->saveMoveState();
      ai_player->stopMove();
    }
  }

  mClip_tag = mShape->playAnimation(clip, pos, rate, trans, false/*hold*/, true/*wait*/, is_death_anim);
  return mClip_tag;
}

void afxShapeConstraint::remapAnimation(U32 tag, ShapeBase* other_shape)
{
  if (mClip_tag == 0)
    return;

  if (!mShape)
    return;

  if (!other_shape)
  {
    resetAnimation(tag);
    return;
  }

  Con::errorf("remapAnimation -- Clip name, %s.", mShape->getLastClipName(tag));

  if (mShape->isClientObject())
  {
    mShape->restoreAnimation(tag);
  }
  else
  {
    AIPlayer* ai_player = dynamic_cast<AIPlayer*>(mShape);
    if (ai_player)
      ai_player->restartMove(tag);
    else
      mShape->restoreAnimation(tag);
  }

  mClip_tag = 0;
}

void afxShapeConstraint::resetAnimation(U32 tag)
{
  if (mClip_tag == 0)
    return;

  if (!mShape)
    return;
  
  if (mShape->isClientObject())
  {
    mShape->restoreAnimation(tag);
  }
  else
  {
    AIPlayer* ai_player = dynamic_cast<AIPlayer*>(mShape);
    if (ai_player)
      ai_player->restartMove(tag);
    else
      mShape->restoreAnimation(tag);
  }

  if ((tag & 0x80000000) == 0 && tag == mClip_tag)
    mClip_tag = 0;
}

U32 afxShapeConstraint::lockAnimation()
{
  if (!mShape)
    return 0;

  mLock_tag = mShape->lockAnimation();
  return mLock_tag;
}

void afxShapeConstraint::unlockAnimation(U32 tag)
{
  if (mLock_tag == 0)
    return;

  if (!mShape)
    return;
  
  mShape->unlockAnimation(tag);
  mLock_tag = 0;
}

F32 afxShapeConstraint::getAnimClipDuration(const char* clip)
{
  return (mShape) ? mShape->getAnimationDuration(clip) : 0.0f;
}

S32 afxShapeConstraint::getDamageState()
{
  return (mShape) ? mShape->getDamageState() : -1;
}

U32 afxShapeConstraint::getTriggers()
{
  return (mShape) ? mShape->getShapeInstance()->getTriggerStateMask() : 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxShapeNodeConstraint

afxShapeNodeConstraint::afxShapeNodeConstraint(afxConstraintMgr* mgr)  
  : afxShapeConstraint(mgr)
{
  mArb_node = ST_NULLSTRING;
  mShape_node_ID = -1;
}

afxShapeNodeConstraint::afxShapeNodeConstraint(afxConstraintMgr* mgr, StringTableEntry arb_name, StringTableEntry arb_node)
  : afxShapeConstraint(mgr, arb_name)
{
  mArb_node = arb_node;
  mShape_node_ID = -1;
}

void afxShapeNodeConstraint::set(ShapeBase* shape)
{
  if (shape)
  {
    mShape_node_ID = shape->getShape()->findNode(mArb_node);
    if (mShape_node_ID == -1)
      Con::errorf("Failed to find node [%s]", mArb_node);
  }
  else
	  mShape_node_ID = -1;

  Parent::set(shape);
}

void afxShapeNodeConstraint::set_scope_id(U16 scope_id)
{
	mShape_node_ID = -1;
  Parent::set_scope_id(scope_id);
}

void afxShapeNodeConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  if (mShape && mShape_node_ID != -1)
  {
    mLast_xfm = mShape->getRenderTransform();
    mLast_xfm.scale(mShape->getScale());
    mLast_xfm.mul(mShape->getShapeInstance()->mNodeTransforms[mShape_node_ID]);
    mLast_pos = mLast_xfm.getPosition();
  }
}

void afxShapeNodeConstraint::restoreObject(SceneObject* obj) 
{ 
  ShapeBase* shape = dynamic_cast<ShapeBase*>(obj);
  if (shape)
  {
    mShape_node_ID = shape->getShape()->findNode(mArb_node);
    if (mShape_node_ID == -1)
      Con::errorf("Failed to find node [%s]", mArb_node);
  }
  else
    mShape_node_ID = -1;
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
  mArb_name = ST_NULLSTRING;
  mObj = 0;
  mScope_id = 0;
  mIs_camera = false;
}

afxObjectConstraint::afxObjectConstraint(afxConstraintMgr* mgr, StringTableEntry arb_name) 
  : afxConstraint(mgr)
{
  mArb_name = arb_name;
  mObj = 0;
  mScope_id = 0;
  mIs_camera = false;
}

afxObjectConstraint::~afxObjectConstraint()
{
  if (mObj)
    clearNotify(mObj);
}

void afxObjectConstraint::set(SceneObject* obj)
{
  if (mObj)
  {
    mScope_id = 0;
    clearNotify(mObj);
  }

  mObj = obj;

  if (mObj)
  {
    deleteNotify(mObj);
	mScope_id = mObj->getScopeId();
  }

  if (mObj != NULL)
  {
    mIs_camera = mObj->isCamera();

    mIs_defined = true;
    mIs_valid = true;
    mChange_code++;
    sample(0.0f, 0, 0);
  }
  else
    mIs_valid = false;
}

void afxObjectConstraint::set_scope_id(U16 scope_id)
{
  if (mObj)
    clearNotify(mObj);

  mObj = 0;
  mScope_id = scope_id;

  mIs_defined = (scope_id > 0);
  mIs_valid = false;
  mMgr->postMissingConstraintObject(this);
}

void afxObjectConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  if (mGone_missing)
    return;

  if (mObj)
  {
    if (!mIs_camera && mCons_def.mTreat_as_camera && dynamic_cast<ShapeBase*>(mObj))
    {
      ShapeBase* cam_obj = (ShapeBase*) mObj;
      F32 pov = 1.0f;
      cam_obj->getCameraTransform(&pov, &mLast_xfm);
      mLast_xfm.getColumn(3, &mLast_pos);
    }
    else
    {
      mLast_xfm = mObj->getRenderTransform();
      if (mCons_def.mPos_at_box_center)
        mLast_pos = mObj->getBoxCenter();
      else
        mLast_pos = mObj->getRenderPosition();
    }
  }
}

void afxObjectConstraint::restoreObject(SceneObject* obj)
{
  if (mObj)
  {
    mScope_id = 0;
    clearNotify(mObj);
  }

  mObj = obj;

  if (mObj)
  {
    deleteNotify(mObj);
    mScope_id = mObj->getScopeId();
  }

  mIs_valid = (mObj != NULL);
}

void afxObjectConstraint::onDeleteNotify(SimObject* obj)
{
  if (mObj == dynamic_cast<SceneObject*>(obj))
  {
    mObj = 0;
    mIs_valid = false;
    if (mScope_id > 0)
      mMgr->postMissingConstraintObject(this, true);
  }

  Parent::onDeleteNotify(obj);
}

U32 afxObjectConstraint::getTriggers()
{
  TSStatic* ts_static = dynamic_cast<TSStatic*>(mObj);
  if (ts_static)
  {
    TSShapeInstance* obj_inst = ts_static->getShapeInstance();
    return (obj_inst) ? obj_inst->getTriggerStateMask() : 0;
  }

  ShapeBase* shape_base = dynamic_cast<ShapeBase*>(mObj);
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
  mEffect_name = ST_NULLSTRING;
  mEffect = 0;
}

afxEffectConstraint::afxEffectConstraint(afxConstraintMgr* mgr, StringTableEntry effect_name) 
  : afxConstraint(mgr)
{
  mEffect_name = effect_name;
  mEffect = 0;
}

afxEffectConstraint::~afxEffectConstraint()
{
}

bool afxEffectConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  if (!mEffect || !mEffect->inScope())
    return false;
 
  if (mCons_def.mPos_at_box_center)
    mEffect->getUpdatedBoxCenter(pos);
  else
    mEffect->getUpdatedPosition(pos);
  
  return true;
}

bool afxEffectConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  if (!mEffect || !mEffect->inScope())
    return false;
  
  mEffect->getUpdatedTransform(xfm);
  return true;
}

bool afxEffectConstraint::getAltitudes(F32& terrain_alt, F32& interior_alt) 
{ 
  if (!mEffect)
    return false;
  
  mEffect->getAltitudes(terrain_alt, interior_alt);
  return true;
}

void afxEffectConstraint::set(afxEffectWrapper* effect)
{
  mEffect = effect;

  if (mEffect != NULL)
  {
    mIs_defined = true;
    mIs_valid = true;
    mChange_code++;
  }
  else
    mIs_valid = false;
}

U32 afxEffectConstraint::setAnimClip(const char* clip, F32 pos, F32 rate, F32 trans, bool is_death_anim)
{
  return (mEffect) ? mEffect->setAnimClip(clip, pos, rate, trans) : 0;
}

void afxEffectConstraint::resetAnimation(U32 tag)
{
  if (mEffect)
    mEffect->resetAnimation(tag);
}

F32 afxEffectConstraint::getAnimClipDuration(const char* clip)
{
  return (mEffect) ? getAnimClipDuration(clip) : 0;
}

U32 afxEffectConstraint::getTriggers()
{
  return (mEffect) ? mEffect->getTriggers() : 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectNodeConstraint

afxEffectNodeConstraint::afxEffectNodeConstraint(afxConstraintMgr* mgr) 
  : afxEffectConstraint(mgr)
{
  mEffect_node = ST_NULLSTRING;
  mEffect_node_ID = -1;
}

afxEffectNodeConstraint::afxEffectNodeConstraint(afxConstraintMgr* mgr, StringTableEntry name, StringTableEntry node)
: afxEffectConstraint(mgr, name)
{
  mEffect_node = node;
  mEffect_node_ID = -1;
}



bool afxEffectNodeConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  if (!mEffect || !mEffect->inScope())
    return false;
  
  TSShapeInstance* ts_shape_inst = mEffect->getTSShapeInstance();
  if (!ts_shape_inst)
    return false;

  if (mEffect_node_ID == -1)
  {
    TSShape* ts_shape = mEffect->getTSShape();
	mEffect_node_ID = (ts_shape) ? ts_shape->findNode(mEffect_node) : -1;
  }

  if (mEffect_node_ID == -1)
    return false;

  mEffect->getUpdatedTransform(mLast_xfm);

  Point3F scale;
  mEffect->getUpdatedScale(scale);

  MatrixF gag = ts_shape_inst->mNodeTransforms[mEffect_node_ID];
  gag.setPosition( gag.getPosition()*scale );

  MatrixF xfm;
  xfm.mul(mLast_xfm, gag);
  //
  pos = xfm.getPosition();

  return true;
}

bool afxEffectNodeConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  if (!mEffect || !mEffect->inScope())
    return false;
  
  TSShapeInstance* ts_shape_inst = mEffect->getTSShapeInstance();
  if (!ts_shape_inst)
    return false;

  if (mEffect_node_ID == -1)
  {
    TSShape* ts_shape = mEffect->getTSShape();
	mEffect_node_ID = (ts_shape) ? ts_shape->findNode(mEffect_node) : -1;
  }

  if (mEffect_node_ID == -1)
    return false;

  mEffect->getUpdatedTransform(mLast_xfm);

  Point3F scale;
  mEffect->getUpdatedScale(scale);

  MatrixF gag = ts_shape_inst->mNodeTransforms[mEffect_node_ID];
  gag.setPosition( gag.getPosition()*scale );

  xfm.mul(mLast_xfm, gag);

  return true;
}

void afxEffectNodeConstraint::set(afxEffectWrapper* effect)
{
  if (effect)
  {
    TSShape* ts_shape = effect->getTSShape();
	mEffect_node_ID = (ts_shape) ? ts_shape->findNode(mEffect_node) : -1;
  }
  else
    mEffect_node_ID = -1;

  Parent::set(effect);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxSampleBuffer

afxSampleBuffer::afxSampleBuffer()
{
  mBuffer_sz = 0;
  mBuffer_ms = 0;
  mMS_per_sample = 33;
  mElapsed_ms = 0;
  mLast_sample_ms = 0;
  mNext_sample_num = 0;
  mNum_samples = 0;
}

afxSampleBuffer::~afxSampleBuffer()
{
}

void afxSampleBuffer::configHistory(F32 hist_len, U8 sample_rate)
{
  mBuffer_sz = mCeil(hist_len*sample_rate) + 1;
  mMS_per_sample = mCeil(1000.0f/sample_rate);
  mBuffer_ms = mBuffer_sz*mMS_per_sample;
}

void afxSampleBuffer::recordSample(F32 dt, U32 elapsed_ms, void* data)
{
  mElapsed_ms = elapsed_ms;

  if (!data)
    return;

  U32 now_sample_num = elapsed_ms/mMS_per_sample;
  if (mNext_sample_num <= now_sample_num)
  {
    mLast_sample_ms = elapsed_ms;
    while (mNext_sample_num <= now_sample_num)
    {
      recSample(mNext_sample_num % mBuffer_sz, data);
      mNext_sample_num++;
      mNum_samples++;
    }
  }
}

inline bool afxSampleBuffer::compute_idx_from_lag(F32 lag, U32& idx) 
{ 
  bool in_bounds = true;

  U32 lag_ms = lag*1000.0f;
  U32 rec_ms = (mElapsed_ms < mBuffer_ms) ? mElapsed_ms : mBuffer_ms;
  if (lag_ms > rec_ms)
  {
    // hasn't produced enough history
    lag_ms = rec_ms;
    in_bounds = false;
  }

  U32 latest_sample_num = mLast_sample_ms/mMS_per_sample;
  U32 then_sample_num = (mElapsed_ms - lag_ms)/mMS_per_sample;

  if (then_sample_num > latest_sample_num)
  {
    // latest sample is older than lag
    then_sample_num = latest_sample_num;
    in_bounds = false;
  }

  idx = then_sample_num % mBuffer_sz;
  return in_bounds;
}

inline bool afxSampleBuffer::compute_idx_from_lag(F32 lag, U32& idx1, U32& idx2, F32& t) 
{ 
  bool in_bounds = true;

  F32 lag_ms = lag*1000.0f;
  F32 rec_ms = (mElapsed_ms < mBuffer_ms) ? mElapsed_ms : mBuffer_ms;
  if (lag_ms > rec_ms)
  {
    // hasn't produced enough history
    lag_ms = rec_ms;
    in_bounds = false;
  }

  F32 per_samp = mMS_per_sample;
  F32 latest_sample_num = mLast_sample_ms/per_samp;
  F32 then_sample_num = (mElapsed_ms - lag_ms)/per_samp;

  U32 latest_sample_num_i = latest_sample_num;
  U32 then_sample_num_i = then_sample_num;

  if (then_sample_num_i >= latest_sample_num_i)
  {
    if (latest_sample_num_i < then_sample_num_i)
      in_bounds = false;
    t = 0.0;
    idx1 = then_sample_num_i % mBuffer_sz;
    idx2 = idx1;
  }
  else
  {
    t = then_sample_num - then_sample_num_i;
    idx1 = then_sample_num_i % mBuffer_sz;
    idx2 = (then_sample_num_i+1) % mBuffer_sz;
  }

  return in_bounds;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxSampleXfmBuffer

afxSampleXfmBuffer::afxSampleXfmBuffer()
{
  mXfm_buffer = 0;
}

afxSampleXfmBuffer::~afxSampleXfmBuffer()
{
  delete [] mXfm_buffer;
}

void afxSampleXfmBuffer::configHistory(F32 hist_len, U8 sample_rate)
{
  if (!mXfm_buffer)
  {
    afxSampleBuffer::configHistory(hist_len, sample_rate);
    if (mBuffer_sz > 0)
      mXfm_buffer = new MatrixF[mBuffer_sz];
  }  
}

void afxSampleXfmBuffer::recSample(U32 idx, void* data)
{
  mXfm_buffer[idx] = *((MatrixF*)data);
}

void afxSampleXfmBuffer::getSample(F32 lag, void* data, bool& in_bounds) 
{ 
  U32 idx1, idx2;
  F32 t;
  in_bounds = compute_idx_from_lag(lag, idx1, idx2, t);

  if (idx1 == idx2)
  {
    MatrixF* m1 = &mXfm_buffer[idx1];
    *((MatrixF*)data) = *m1;
  }
  else
  {
    MatrixF* m1 = &mXfm_buffer[idx1];
    MatrixF* m2 = &mXfm_buffer[idx2];

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
  mSamples = 0;
}

afxPointHistConstraint::~afxPointHistConstraint()
{
  delete mSamples;
}

void afxPointHistConstraint::set(Point3F point, Point3F vector)
{
  if (!mSamples)
  {
    mSamples = new afxSampleXfmBuffer;
	mSamples->configHistory(mCons_def.mHistory_time, mCons_def.mSample_rate);
  }
  
  Parent::set(point, vector);
}

void afxPointHistConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  Parent::sample(dt, elapsed_ms, cam_pos);

  if (isDefined())
  {
    if (isValid())
      mSamples->recordSample(dt, elapsed_ms, &mLast_xfm);
    else
      mSamples->recordSample(dt, elapsed_ms, 0);
  }
}

bool afxPointHistConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  bool in_bounds;

  MatrixF xfm;
  mSamples->getSample(hist, &xfm, in_bounds);

  pos = xfm.getPosition();

  return in_bounds;
}

bool afxPointHistConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  bool in_bounds;

  mSamples->getSample(hist, &xfm, in_bounds);

  return in_bounds; 
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxPointHistConstraint

afxTransformHistConstraint::afxTransformHistConstraint(afxConstraintMgr* mgr)
  : afxTransformConstraint(mgr)
{
  mSamples = 0;
}

afxTransformHistConstraint::~afxTransformHistConstraint()
{
  delete mSamples;
}

void afxTransformHistConstraint::set(const MatrixF& xfm)
{
  if (!mSamples)
  {
    mSamples = new afxSampleXfmBuffer;
	mSamples->configHistory(mCons_def.mHistory_time, mCons_def.mSample_rate);
  }
  
  Parent::set(xfm);
}

void afxTransformHistConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  Parent::sample(dt, elapsed_ms, cam_pos);

  if (isDefined())
  {
    if (isValid())
      mSamples->recordSample(dt, elapsed_ms, &mLast_xfm);
    else
      mSamples->recordSample(dt, elapsed_ms, 0);
  }
}

bool afxTransformHistConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  bool in_bounds;

  MatrixF xfm;
  mSamples->getSample(hist, &xfm, in_bounds);

  pos = xfm.getPosition();

  return in_bounds;
}

bool afxTransformHistConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  bool in_bounds;

  mSamples->getSample(hist, &xfm, in_bounds);

  return in_bounds; 
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxShapeHistConstraint

afxShapeHistConstraint::afxShapeHistConstraint(afxConstraintMgr* mgr)
  : afxShapeConstraint(mgr)
{
  mSamples = 0;
}

afxShapeHistConstraint::afxShapeHistConstraint(afxConstraintMgr* mgr, StringTableEntry arb_name)
  : afxShapeConstraint(mgr, arb_name)
{
  mSamples = 0;
}

afxShapeHistConstraint::~afxShapeHistConstraint()
{
  delete mSamples;
}

void afxShapeHistConstraint::set(ShapeBase* shape)
{
  if (shape && !mSamples)
  {
    mSamples = new afxSampleXfmBuffer;
	mSamples->configHistory(mCons_def.mHistory_time, mCons_def.mSample_rate);
  }
  
  Parent::set(shape);
}

void afxShapeHistConstraint::set_scope_id(U16 scope_id)
{
  if (scope_id > 0 && !mSamples)
  {
    mSamples = new afxSampleXfmBuffer;
	mSamples->configHistory(mCons_def.mHistory_time, mCons_def.mSample_rate);
  }
  
  Parent::set_scope_id(scope_id);
}

void afxShapeHistConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  Parent::sample(dt, elapsed_ms, cam_pos);

  if (isDefined())
  {
    if (isValid())
      mSamples->recordSample(dt, elapsed_ms, &mLast_xfm);
    else
      mSamples->recordSample(dt, elapsed_ms, 0);
  }
}

bool afxShapeHistConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  bool in_bounds;

  MatrixF xfm;
  mSamples->getSample(hist, &xfm, in_bounds);

  pos = xfm.getPosition();

  return in_bounds;
}

bool afxShapeHistConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  bool in_bounds;

  mSamples->getSample(hist, &xfm, in_bounds);

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
  mSamples = 0;
}

afxShapeNodeHistConstraint::afxShapeNodeHistConstraint(afxConstraintMgr* mgr, StringTableEntry arb_name,
                                                       StringTableEntry arb_node)
  : afxShapeNodeConstraint(mgr, arb_name, arb_node)
{
  mSamples = 0;
}

afxShapeNodeHistConstraint::~afxShapeNodeHistConstraint()
{
  delete mSamples;
}

void afxShapeNodeHistConstraint::set(ShapeBase* shape)
{
  if (shape && !mSamples)
  {
    mSamples = new afxSampleXfmBuffer;
	mSamples->configHistory(mCons_def.mHistory_time, mCons_def.mSample_rate);
  }
  
  Parent::set(shape);
}

void afxShapeNodeHistConstraint::set_scope_id(U16 scope_id)
{
  if (scope_id > 0 && !mSamples)
  {
    mSamples = new afxSampleXfmBuffer;
	mSamples->configHistory(mCons_def.mHistory_time, mCons_def.mSample_rate);
  }
  
  Parent::set_scope_id(scope_id);
}

void afxShapeNodeHistConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  Parent::sample(dt, elapsed_ms, cam_pos);

  if (isDefined())
  {
    if (isValid())
      mSamples->recordSample(dt, elapsed_ms, &mLast_xfm);
    else
      mSamples->recordSample(dt, elapsed_ms, 0);
  }
}

bool afxShapeNodeHistConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  bool in_bounds;

  MatrixF xfm;
  mSamples->getSample(hist, &xfm, in_bounds);

  pos = xfm.getPosition();

  return in_bounds;
}

bool afxShapeNodeHistConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  bool in_bounds;

  mSamples->getSample(hist, &xfm, in_bounds);

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
  mSamples = 0;
}

afxObjectHistConstraint::afxObjectHistConstraint(afxConstraintMgr* mgr, StringTableEntry arb_name)
  : afxObjectConstraint(mgr, arb_name)
{
  mSamples = 0;
}

afxObjectHistConstraint::~afxObjectHistConstraint()
{
  delete mSamples;
}

void afxObjectHistConstraint::set(SceneObject* obj)
{
  if (obj && !mSamples)
  {
    mSamples = new afxSampleXfmBuffer;
	mSamples->configHistory(mCons_def.mHistory_time, mCons_def.mSample_rate);
  }
  
  Parent::set(obj);
}

void afxObjectHistConstraint::set_scope_id(U16 scope_id)
{
  if (scope_id > 0 && !mSamples)
  {
    mSamples = new afxSampleXfmBuffer;
    mSamples->configHistory(mCons_def.mHistory_time, mCons_def.mSample_rate);
  }
  
  Parent::set_scope_id(scope_id);
}

void afxObjectHistConstraint::sample(F32 dt, U32 elapsed_ms, const Point3F* cam_pos)
{
  Parent::sample(dt, elapsed_ms, cam_pos);

  if (isDefined())
  {
    if (isValid())
      mSamples->recordSample(dt, elapsed_ms, &mLast_xfm);
    else
      mSamples->recordSample(dt, elapsed_ms, 0);
  }
}

bool afxObjectHistConstraint::getPosition(Point3F& pos, F32 hist) 
{ 
  bool in_bounds;

  MatrixF xfm;
  mSamples->getSample(hist, &xfm, in_bounds);

  pos = xfm.getPosition();

  return in_bounds;
}

bool afxObjectHistConstraint::getTransform(MatrixF& xfm, F32 hist) 
{ 
  bool in_bounds;

  mSamples->getSample(hist, &xfm, in_bounds);

  return in_bounds; 
}

void afxObjectHistConstraint::onDeleteNotify(SimObject* obj)
{
  Parent::onDeleteNotify(obj);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

