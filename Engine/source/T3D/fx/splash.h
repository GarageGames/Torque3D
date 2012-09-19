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

#ifndef _SPLASH_H_
#define _SPLASH_H_

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif

#ifndef _TORQUE_LIST_
#include "core/util/tList.h"
#endif

#include "gfx/gfxTextureHandle.h"

class ParticleEmitter;
class ParticleEmitterData;
class AudioProfile;
class ExplosionData;


//--------------------------------------------------------------------------
// Ring Point
//--------------------------------------------------------------------------
struct SplashRingPoint
{
   Point3F  position;
   Point3F  velocity;
};

//--------------------------------------------------------------------------
// Splash Ring
//--------------------------------------------------------------------------
struct SplashRing
{
   Vector <SplashRingPoint> points;
   ColorF   color;
   F32      lifetime;
   F32      elapsedTime;
   F32      v;

   SplashRing()
   {
      color.set( 0.0, 0.0, 0.0, 1.0 );
      lifetime = 0.0;
      elapsedTime = 0.0;
      v = 0.0;
   }

   bool isActive()
   {
      return elapsedTime < lifetime;
   }
};

//--------------------------------------------------------------------------
// Splash Data
//--------------------------------------------------------------------------
class SplashData : public GameBaseData 
{
  public:
   typedef GameBaseData Parent;

   enum Constants
   {
      NUM_EMITTERS = 3,
      NUM_TIME_KEYS = 4,
      NUM_TEX = 2,
   };

public:
   AudioProfile*           soundProfile;
   S32                     soundProfileId;

   ParticleEmitterData*    emitterList[NUM_EMITTERS];
   S32                     emitterIDList[NUM_EMITTERS];

   S32               delayMS;
   S32               delayVariance;
   S32               lifetimeMS;
   S32               lifetimeVariance;
   Point3F           scale;
   F32               width;
   F32               height;
   U32               numSegments;
   F32               velocity;
   F32               acceleration;
   F32               texWrap;
   F32               texFactor;

   F32               ejectionFreq;
   F32               ejectionAngle;
   F32               ringLifetime;
   F32               startRadius;

   F32               times[ NUM_TIME_KEYS ];
   ColorF            colors[ NUM_TIME_KEYS ];

   StringTableEntry  textureName[NUM_TEX];
   GFXTexHandle      textureHandle[NUM_TEX];

   ExplosionData*    explosion;
   S32               explosionId;

   SplashData();
   DECLARE_CONOBJECT(SplashData);
   bool onAdd();
   bool preload(bool server, String &errorStr);
   static void  initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};

//--------------------------------------------------------------------------
// Splash
//--------------------------------------------------------------------------
class Splash : public GameBase
{
   typedef GameBase Parent;

private:
   SplashData*    mDataBlock;

   SimObjectPtr<ParticleEmitter> mEmitterList[ SplashData::NUM_EMITTERS ];

   typedef Torque::List<SplashRing> SplashRingList;
   SplashRingList ringList;

   U32         mCurrMS;
   U32         mEndingMS;
   F32         mRandAngle;
   F32         mRadius;
   F32         mVelocity;
   F32         mHeight;
   ColorF      mColor;
   F32         mTimeSinceLastRing;
   bool        mDead;
   F32         mElapsedTime;

protected:
   Point3F     mInitialPosition;
   Point3F     mInitialNormal;
   F32         mFade;
   F32         mFog;
   bool        mActive;
   S32         mDelayMS;

protected:
   bool        onAdd();
   void        onRemove();
   void        processTick(const Move *move);
   void        advanceTime(F32 dt);
   void        updateEmitters( F32 dt );
   void        updateWave( F32 dt );
   void        updateColor();
   SplashRing  createRing();
   void        updateRings( F32 dt );
   void        updateRing( SplashRing& ring, F32 dt );
   void        emitRings( F32 dt );
   void        spawnExplosion();

public:
   Splash();
   ~Splash();
   void setInitialState(const Point3F& point, const Point3F& normal, const F32 fade = 1.0);

   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection *conn,           BitStream* stream);

   bool onNewDataBlock( GameBaseData *dptr, bool reload );
   DECLARE_CONOBJECT(Splash);
};


#endif // _H_SPLASH
