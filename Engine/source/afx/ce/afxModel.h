
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

#ifndef _AFX_MODEL_H_
#define _AFX_MODEL_H_

#include "renderInstance/renderPassManager.h"

class ParticleEmitterData;
class ParticleEmitter;
class ExplosionData;
class TSPartInstance;
class TSShapeInstance;
class TSShape;

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxModel Data

struct afxModelData : public GameBaseData
{
  typedef GameBaseData Parent;

  StringTableEntry      shapeName;
  StringTableEntry      sequence;
  F32                   seq_rate;
  F32                   seq_offset;
  F32                   alpha_mult;
  bool                  use_vertex_alpha;
  U32                   force_on_material_flags;
  U32                   force_off_material_flags;
  bool                  texture_filtering;
  F32                   fog_mult;

  struct TextureTagRemapping
  {
     char* old_tag;
     char* new_tag;
  };
  char*                 remap_buffer;
  Vector<TextureTagRemapping> txr_tag_remappings;

  StringTableEntry      remap_txr_tags;

  Resource<TSShape>     shape;

  bool                  overrideLightingOptions;
  bool                  receiveSunLight;
  bool                  receiveLMLighting;
  bool                  useAdaptiveSelfIllumination;
  bool                  useCustomAmbientLighting;
  bool                  customAmbientForSelfIllumination;
  ColorF                customAmbientLighting;
  bool                  shadowEnable;

  U32                   shadowSize;
  F32                   shadowMaxVisibleDistance;
  F32                   shadowProjectionDistance;
  F32                   shadowSphereAdjust;

public:
  /*C*/                 afxModelData();
  /*C*/                 afxModelData(const afxModelData&, bool = false);
  /*D*/                 ~afxModelData();

  bool                  preload(bool server, String &errorStr);
  void                  packData(BitStream* stream);
  void                  unpackData(BitStream* stream);

  virtual void          onPerformSubstitutions();
  virtual bool          allowSubstitutions() const { return true; }

  static void           initPersistFields();

  DECLARE_CONOBJECT(afxModelData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxModel

class afxModel : public GameBase
{
  typedef GameBase Parent;

private:
  afxModelData*         mDataBlock;
  TSShapeInstance*      shape_inst;
  TSThread*             main_seq_thread;
  S32                   main_seq_id;
  F32                   seq_rate_factor;
  bool                  seq_animates_vis;
  U32                   last_anim_tag;
  F32                   fade_amt;
  bool                  is_visible;
  S8                    sort_priority;

  struct BlendThread
  {
    TSThread* thread;
    U32       tag;
  };
  Vector<BlendThread>   blend_clips;
  static U32            unique_anim_tag_counter;

protected:
  Vector<S32>           mCollisionDetails;
  Vector<S32>           mLOSDetails;
  bool                  castRay(const Point3F &start, const Point3F &end, RayInfo* info);

  virtual void          advanceTime(F32 dt);

  virtual void          prepRenderImage(SceneRenderState*);

  void                  renderObject(SceneRenderState*);

  virtual bool          onAdd();
  virtual void          onRemove();

public:
  /*C*/                 afxModel();
  /*D*/                 ~afxModel();

  virtual bool          onNewDataBlock(GameBaseData* dptr, bool reload);

  void                  setFadeAmount(F32 amt) { fade_amt = amt; }
  void                  setSequenceRateFactor(F32 factor);
  void                  setSortPriority(S8 priority) { sort_priority = priority; }

  const char*           getShapeFileName() const { return mDataBlock->shapeName; }
  void                  setVisibility(bool flag) { is_visible = flag; }
  TSShape*              getTSShape() { return mDataBlock->shape; }
  TSShapeInstance*      getTSShapeInstance() { return shape_inst; }

  U32                   setAnimClip(const char* clip, F32 pos, F32 rate, F32 trans);
  void                  resetAnimation(U32 tag);
  F32                   getAnimClipDuration(const char* clip);

  DECLARE_CONOBJECT(afxModel);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_MODEL_H_
