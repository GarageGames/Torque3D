//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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
//-----------------------------------------------------------------------------

#ifndef _EXPLOSION_H_
#define _EXPLOSION_H_

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif
#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif
#ifndef _LIGHTINFO_H_
#include "lighting/lightInfo.h"
#endif

class ParticleEmitter;
class ParticleEmitterData;
class TSThread;
class SFXTrack;
struct DebrisData;

//--------------------------------------------------------------------------
class ExplosionData : public GameBaseData {
  public:
   typedef GameBaseData Parent;

   enum ExplosionConsts
   {
      EC_NUM_DEBRIS_TYPES = 1,
      EC_NUM_EMITTERS = 4,
      EC_MAX_SUB_EXPLOSIONS = 5,
      EC_NUM_TIME_KEYS = 4,
   };

  public:
   StringTableEntry dtsFileName;

   bool faceViewer;

   S32 particleDensity;
   F32 particleRadius;

   SFXTrack*        soundProfile;
   ParticleEmitterData* particleEmitter;
   S32                  particleEmitterId;

   Point3F              explosionScale;
   F32                  playSpeed;

   Resource<TSShape> explosionShape;
   S32               explosionAnimation;

   ParticleEmitterData*    emitterList[EC_NUM_EMITTERS];
   S32                     emitterIDList[EC_NUM_EMITTERS];

   DebrisData *   debrisList[EC_NUM_DEBRIS_TYPES];
   S32            debrisIDList[EC_NUM_DEBRIS_TYPES];

   F32            debrisThetaMin;
   F32            debrisThetaMax;
   F32            debrisPhiMin;
   F32            debrisPhiMax;
   S32            debrisNum;
   S32            debrisNumVariance;
   F32            debrisVelocity;
   F32            debrisVelocityVariance;

   // sub - explosions
   ExplosionData*    explosionList[EC_MAX_SUB_EXPLOSIONS];
   S32               explosionIDList[EC_MAX_SUB_EXPLOSIONS];

   S32               delayMS;
   S32               delayVariance;
   S32               lifetimeMS;
   S32               lifetimeVariance;

   F32               offset;
   Point3F           sizes[ EC_NUM_TIME_KEYS ];
   F32               times[ EC_NUM_TIME_KEYS ];

   // camera shake data
   bool              shakeCamera;
   VectorF           camShakeFreq;
   VectorF           camShakeAmp;
   F32               camShakeDuration;
   F32               camShakeRadius;
   F32               camShakeFalloff;

   // Dynamic Lighting. The light is smoothly
   // interpolated from start to end time.
   F32               lightStartRadius;
   F32               lightEndRadius;
   ColorF            lightStartColor;
   ColorF            lightEndColor;
   F32               lightStartBrightness;
   F32               lightEndBrightness;
   F32               lightNormalOffset;

   ExplosionData();
   DECLARE_CONOBJECT(ExplosionData);
   bool onAdd();
   bool preload(bool server, String &errorStr);
   static void  initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};


//--------------------------------------------------------------------------
class Explosion : public GameBase, public ISceneLight
{
   typedef GameBase Parent;

  private:
   ExplosionData*   mDataBlock;

   TSShapeInstance* mExplosionInstance;
   TSThread*        mExplosionThread;

   SimObjectPtr<ParticleEmitter> mEmitterList[ ExplosionData::EC_NUM_EMITTERS ];
   SimObjectPtr<ParticleEmitter> mMainEmitter;

   U32               mCurrMS;
   U32               mEndingMS;
   F32               mRandAngle;
   LightInfo*        mLight;

  protected:
   Point3F  mInitialNormal;
   F32      mFade;
   bool     mActive;
   S32      mDelayMS;
   F32      mRandomVal;
   U32      mCollideType;

  protected:
   bool onAdd();
   void onRemove();
   bool explode();

   void processTick(const Move *move);
   void advanceTime(F32 dt);
   void updateEmitters( F32 dt );
   void launchDebris( Point3F &axis );
   void spawnSubExplosions();
   void setCurrentScale();

   // Rendering
  protected:
   void prepRenderImage( SceneRenderState *state );
   void prepBatchRender(SceneRenderState *state);
   void prepModelView(SceneRenderState*);

  public:
   Explosion();
   ~Explosion();
   void setInitialState(const Point3F& point, const Point3F& normal, const F32 fade = 1.0);

   // ISceneLight
   virtual void submitLights( LightManager *lm, bool staticLighting );
   virtual LightInfo* getLight() { return mLight; }

   bool onNewDataBlock( GameBaseData *dptr, bool reload );
   void setCollideType( U32 cType ){ mCollideType = cType; }

   DECLARE_CONOBJECT(Explosion);
   static void initPersistFields();
};

#endif // _H_EXPLOSION

