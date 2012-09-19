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

#ifndef _DEBRIS_H_
#define _DEBRIS_H_

#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif
#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif

class ParticleEmitterData;
class ParticleEmitter;
class ExplosionData;
class TSPartInstance;
class TSShapeInstance;
class TSShape;

//**************************************************************************
// Debris Data
//**************************************************************************
struct DebrisData : public GameBaseData
{
   typedef GameBaseData Parent;

   //-----------------------------------------------------------------------
   // Data Decs
   //-----------------------------------------------------------------------
   enum DebrisDataConst
   {
      DDC_NUM_EMITTERS = 2,
   };


   //-----------------------------------------------------------------------
   // Debris datablock
   //-----------------------------------------------------------------------
   F32      velocity;
   F32      velocityVariance;
   F32      friction;
   F32      elasticity;
   F32      lifetime;
   F32      lifetimeVariance;
   U32      numBounces;
   U32      bounceVariance;
   F32      minSpinSpeed;
   F32      maxSpinSpeed;
   bool     explodeOnMaxBounce;  // explodes after it has bounced max times
   bool     staticOnMaxBounce;   // becomes static after bounced max times
   bool     snapOnMaxBounce;     // snap into a "resting" position on last bounce
   bool     fade;
   bool     useRadiusMass;       // use mass calculations based on radius
   F32      baseRadius;          // radius at which the standard elasticity and friction apply
   F32      gravModifier;        // how much gravity affects debris
   F32      terminalVelocity;    // max velocity magnitude
   bool     ignoreWater;

   const char* shapeName;
   Resource<TSShape> shape;

   StringTableEntry  textureName;


   S32                     explosionId;
   ExplosionData *         explosion;
   ParticleEmitterData*    emitterList[DDC_NUM_EMITTERS];
   S32                     emitterIDList[DDC_NUM_EMITTERS];

   DebrisData();

   bool        onAdd();
   bool        preload( bool server, String &errorStr );
   static void initPersistFields();
   void        packData(BitStream* stream);
   void        unpackData(BitStream* stream);

   DECLARE_CONOBJECT(DebrisData);

};

//**************************************************************************
// Debris
//**************************************************************************
class Debris : public GameBase
{
   typedef GameBase Parent;

private:
   S32               mNumBounces;
   F32               mSize;
   Point3F           mLastPos;
   Point3F           mVelocity;
   F32               mLifetime;
   DebrisData *      mDataBlock;
   F32               mElapsedTime;
   TSShapeInstance * mShape;
   TSPartInstance *  mPart;
   MatrixF           mInitialTrans;
   F32               mXRotSpeed;
   F32               mZRotSpeed;
   Point3F           mRotAngles;
   F32               mRadius;
   bool              mStatic;
   F32               mElasticity;
   F32               mFriction;

   SimObjectPtr<ParticleEmitter> mEmitterList[ DebrisData::DDC_NUM_EMITTERS ];

   /// Bounce the debris - returns true if debris bounces.
   bool bounce( const Point3F &nextPos, F32 dt );
   
   /// Compute state of debris as if it hasn't collided with anything.
   void computeNewState( Point3F &newPos, Point3F &newVel, F32 dt );

   void  explode();
   void  rotate( F32 dt );

protected:
   virtual void   processTick(const Move* move);
   virtual void   advanceTime( F32 dt );
   void prepRenderImage(SceneRenderState *state);
   void prepBatchRender(SceneRenderState *state);


   bool           onAdd();
   void           onRemove();
   void           updateEmitters( Point3F &pos, Point3F &vel, U32 ms );

public:

   Debris();
   ~Debris();

   static void    initPersistFields();

   bool   onNewDataBlock( GameBaseData *dptr, bool reload );

   void  init( const Point3F &position, const Point3F &velocity );
   void  setLifetime( F32 lifetime ){ mLifetime = lifetime; }
   void  setPartInstance( TSPartInstance *part ){ mPart = part; }
   void  setSize( F32 size );
   void  setVelocity( const Point3F &vel ){ mVelocity = vel; }
   void  setRotAngles( const Point3F &angles ){ mRotAngles = angles; }

   DECLARE_CONOBJECT(Debris);

};




#endif
