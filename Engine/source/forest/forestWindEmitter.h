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

#ifndef _FORESTWINDEMITTER_H_
#define _FORESTWINDEMITTER_H_


#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MSPHERE_H_
#include "math/mSphere.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif


class ForestWindEmitter;
class ForestWindAccumulator;
class BaseMatInstance;


class ForestWind
{

protected:

   F32 mStrength;

   VectorF mDirection;

   F32 mLastGustTime;
   F32 mLastYawTime;

   F32 mTargetYawAngle;

   F32 mCurrentInterp;
   Point2F mCurrentTarget;

   ForestWindEmitter *mParent;

   MRandom mRandom;

   bool mIsDirty;

public:

   ForestWind( ForestWindEmitter *emitter );
   virtual ~ForestWind();

   void processTick();

   void setStrengthAndDirection( F32 strength, const VectorF &direction );

   void setStrength( F32 strength );

   void setDirection( const VectorF &direction );

   void setIsDirty( bool isDirty ) { mIsDirty = isDirty; }

   F32 getStrength() const { return mStrength; }

   const VectorF& getDirection() const { return mDirection; }
   VectorF getTarget() const { return VectorF( mCurrentTarget.x, mCurrentTarget.y, 0 ); }
};


/// A vector of WindEmitter pointers.
typedef Vector<ForestWindEmitter*> ForestWindEmitterList;

class ForestWindEmitter : public SceneObject
{
   typedef SceneObject Parent;
   friend class ForestWind;
   friend class ForestWindAccumulator;

protected:

   enum
   {
	     EnabledMask    = Parent::NextFreeMask << 0,
	     WindMask       = Parent::NextFreeMask << 1,
		    NextFreeMask   = Parent::NextFreeMask << 2
   };

   bool mAddedToScene;

   bool mEnabled;

   ForestWind *mWind;

   F32 mWindStrength;

   VectorF mWindDirection;

   /// Controls how often the wind gust peaks per second.
   F32 mWindGustFrequency;

   /// The maximum distance in meters that the peak wind 
   /// gust will displace an element.
   F32 mWindGustStrength;

   /// The range that the wind direction
   /// will yaw between (-val to +val).
   F32 mWindGustYawAngle;

   /// The frequency, in seconds, between
   /// animations in the gust yaw angle.
   F32 mWindGustYawFrequency;

   /// The maximum amount of random wobble
   /// that will be applied to the gusting
   /// as well as the yaw rotation.
   F32 mWindGustWobbleStrength;

   /// Controls the overally rapidity of the wind turbulence.
   F32 mWindTurbulenceFrequency;

   /// The maximum distance in meters that the turbulence can
   /// displace a ground cover element.
   F32 mWindTurbulenceStrength;

   /// If the radius is greater than zero then this is
   /// a localized radial wind emitter.
   F32 mWindRadius;

   /// Explicitly denotes whether this is a
   /// global directional wind emitter or a
   /// localized radial wind emitter.
   bool mRadialEmitter;

   bool mHasMount;
   bool mIsMounted;

   /// An object that the emitter can
   /// get position updates from.
   SimObjectPtr<SceneObject> mMountObject;

   static ForestWindEmitterList smEmitters;

   void _initWind( U32 mask = 0xFFFFFFFF );

   void _onMountObjectGhostReceived( SceneObject *object );

public:

   ForestWindEmitter( bool makeClientObject = false );

   virtual ~ForestWindEmitter();

   bool isEnabled() const { return mEnabled; }

   ForestWind* getWind() { return mWind; }

   bool isLocalWind() const { return mWindRadius > 0.0f; }
   bool isRadialEmitter() const { return mRadialEmitter; }

   F32 getWindRadius() const { return mWindRadius; }

   F32 getWindRadiusSquared() const { return mWindRadius * mWindRadius; }

   void setWindRadius( F32 radius ) { mWindRadius = radius; } 

   F32 getStrength() const;
   void setStrength( F32 strength );

   void _renderEmitterInfo( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

   void attachToObject( SceneObject *obj );
   void updateMountPosition();

   // SceneObject
   virtual void setTransform( const MatrixF &mat );
   void prepRenderImage( SceneRenderState *state );

   // SimObject
   bool onAdd();
   void onRemove();
   void inspectPostApply();
   void onEditorEnable();
   void onEditorDisable();
   void onDeleteNotify(SimObject *object);

   // NetObject
   U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *conn, BitStream *stream );

   // ConObject.
   static void initPersistFields();
   DECLARE_CONOBJECT(ForestWindEmitter);
};

#endif // _FORESTWINDEMITTER_H_