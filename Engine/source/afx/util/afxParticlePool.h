
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

#ifndef _AFX_PARTICLE_POOL_H_
#define _AFX_PARTICLE_POOL_H_

class afxParticlePoolData : public GameBaseData
{
private:
  typedef GameBaseData  Parent;

public:
  enum PoolType 
  {
    POOL_NORMAL,
    POOL_TWOPASS
  };

  U32             pool_type;
  ColorF          base_color;
  F32             blend_weight;

public:
  /*C*/           afxParticlePoolData();
  /*C*/           afxParticlePoolData(const afxParticlePoolData&, bool = false);
  /*D*/           ~afxParticlePoolData();

  virtual void    packData(BitStream*);
  virtual void    unpackData(BitStream*);

  virtual bool    allowSubstitutions() const { return true; }

  static void     initPersistFields();

	DECLARE_CONOBJECT(afxParticlePoolData);
  DECLARE_CATEGORY("AFX");
};

typedef afxParticlePoolData::PoolType afxParticlePool_PoolType;
DefineEnumType( afxParticlePool_PoolType );

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

struct Particle;
class ParticleEmitter;
class afxChoreographer;

typedef Vector<ParticleEmitter*> ParticleEmitterList;

class afxParticlePool : public GameBase
{
  typedef GameBase Parent;

  class ObjectDeleteEvent : public SimEvent
  {
  public:
    void process(SimObject *obj) { if (obj) obj->deleteObject(); }
  };

  struct SortParticlePool
  {
    Particle*         p;
    F32               k;  // interpreted differently depending on rendering method
    ParticleEmitter*  emitter;
  };

private:
  afxParticlePoolData*  mDataBlock;
  afxParticlePoolData*  key_block;
  U32                   key_index;
  ParticleEmitterList   emitters;
  afxChoreographer*     choreographer;
  S8                    sort_priority;

  static Vector<SortParticlePool> orderedVector;
  static int QSORT_CALLBACK cmpSortParticlePool(const void* p1, const void* p2);

  S32 mCurBuffSize;
  GFXVertexBufferHandle<GFXVertexPCT> mVertBuff;
  S32 mCurBuffSize2;
  GFXVertexBufferHandle<GFXVertexPCT> mVertBuff2;

protected:
  virtual void    prepRenderImage(SceneRenderState*);

  void            pool_prepBatchRender(RenderPassManager*, const Point3F &camPos, const ColorF &ambientColor);
  void            pool_renderObject_Normal(RenderPassManager*, const Point3F &camPos, const ColorF &ambientColor);
  void            pool_renderObject_TwoPass(RenderPassManager*, const Point3F &camPos, const ColorF &ambientColor);

  virtual bool    onAdd();
  virtual void    onRemove();

  void            renderBillboardParticle_blend(Particle&, const Point3F* basePnts, const MatrixF& camView, const F32 spinFactor,
                                                const F32 blend_factor, ParticleEmitter*);
  void            renderBillboardParticle_color(Particle&, const Point3F* basePnts, const MatrixF& camView, const F32 spinFactor,
                                                const ColorF& color, ParticleEmitter*);

public:
  /*C*/           afxParticlePool();
  /*D*/           ~afxParticlePool();

  virtual bool    onNewDataBlock(GameBaseData* dptr, bool reload);

  void            addParticleEmitter(ParticleEmitter*);
  void            removeParticleEmitter(ParticleEmitter*);

  void            setChoreographer(afxChoreographer* ch) { choreographer = ch; }
  void            setKeyBlock(afxParticlePoolData* db, U32 idx) { key_block = db; key_index = idx; }
  bool            hasMatchingKeyBlock(const afxParticlePoolData* db, U32 idx) const { return (db == key_block && idx == key_index); }

  void            updatePoolBBox(ParticleEmitter*);

  void            setSortPriority(S8 priority);

  DECLARE_CONOBJECT(afxParticlePool);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_PARTICLE_POOL_H_
