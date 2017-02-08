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

#ifndef _PHYSICSSHAPE_H_
#define _PHYSICSSHAPE_H_

#ifndef _GAMEBASE_H_
   #include "T3D/gameBase/gameBase.h"
#endif
#ifndef __RESOURCE_H__
   #include "core/resource.h"
#endif
#ifndef _TSSHAPE_H_
   #include "ts/tsShape.h"
#endif
#ifndef _T3D_PHYSICSCOMMON_H_
   #include "T3D/physics/physicsCommon.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
   #include "console/dynamicTypes.h"
#endif
#ifndef _SIMOBJECTREF_H_
   #include "console/simObjectRef.h"
#endif

class TSShapeInstance;
class PhysicsBody;
class PhysicsWorld;
class PhysicsDebrisData;
class ExplosionData;


class PhysicsShapeData : public GameBaseData
{
   typedef GameBaseData Parent;

   void _onResourceChanged( const Torque::Path &path );

public:

   PhysicsShapeData();
   virtual ~PhysicsShapeData();

   DECLARE_CONOBJECT(PhysicsShapeData);
   static void initPersistFields();   
   bool onAdd();
   void onRemove();
   
   // GameBaseData
   void packData(BitStream* stream);
   void unpackData(BitStream* stream);   
   bool preload(bool server, String &errorBuffer );

public:

   /// The shape to load.
   StringTableEntry shapeName;

   /// The shape resource.
   Resource<TSShape> shape;

   /// The shared unscaled collision shape.
   PhysicsCollisionRef colShape;

   ///
   F32 mass;

   /// 
   F32 dynamicFriction;

   /// 
   F32 staticFriction;

   ///
   F32 restitution;

   ///
   F32 linearDamping;

   ///
   F32 angularDamping;

   /// 
   F32 linearSleepThreshold;

   ///
   F32 angularSleepThreshold;

   // A scale applied to the normal linear and angular damping
   // when the object enters a water volume.
   F32 waterDampingScale;

   // The density of this object used for water buoyancy effects.
   F32 buoyancyDensity;


   enum SimType
   {
      /// This physics representation only exists on the client
      /// world and the server only does ghosting.
      SimType_ClientOnly,

      /// The physics representation only exists on the server world
      /// and the client gets delta updates for rendering.
      SimType_ServerOnly,

      /// The physics representation exists on the client and the server
      /// worlds with corrections occuring when the client gets out of sync.
      SimType_ClientServer,

      /// The bits used to pack the SimType field.
      SimType_Bits = 3,

   } simType;
   
   SimObjectRef< PhysicsDebrisData > debris;   
   SimObjectRef< ExplosionData > explosion;   
   SimObjectRef< PhysicsShapeData > destroyedShape;
};

typedef PhysicsShapeData::SimType PhysicsSimType;
DefineEnumType( PhysicsSimType );

class TSThread;


/// A simple single body dynamic physics object.
class PhysicsShape : public GameBase
{
   typedef GameBase Parent;

protected:

   /// The abstracted physics actor.
   PhysicsBody *mPhysicsRep;

   ///
   PhysicsWorld *mWorld;

   /// The starting position to place the shape when
   /// the level begins or is reset.
   MatrixF mResetPos;

   //VectorF mBuildScale;
   //F32 mBuildAngDrag;
   //F32 mBuildLinDrag;

   /// The rendered shape.
   TSShapeInstance *mShapeInst;

   /// The current physics state.
   PhysicsState mState;

   /// The previous and current render states.
   PhysicsState mRenderState[2];

   /// True if the PhysicsShape has been destroyed ( gameplay ).
   bool mDestroyed;

   /// Enables automatic playing of the animation named "ambient" (if it exists) 
   /// when the PhysicsShape is loaded.
   bool mPlayAmbient;
   S32 mAmbientSeq;
   TSThread* mAmbientThread;

   /// If a specified to create one in the PhysicsShape data, this is the 
   /// subshape created when this PhysicsShape is destroyed.
   /// Is only assigned (non null) on the serverside PhysicsShape.
   SimObjectPtr< PhysicsShape > mDestroyedShape;

   ///
   enum MaskBits 
   {
      StateMask = Parent::NextFreeMask << 0,
      ResetPosMask = Parent::NextFreeMask << 1,
      DamageMask = Parent::NextFreeMask << 2,

      NextFreeMask = Parent::NextFreeMask << 3
   };

   bool _createShape();

   void _initAmbient();
  
   ///
   void _applyCorrection( const MatrixF &mat );

   void _onPhysicsReset( PhysicsResetEvent reset );

   void _updateContainerForces();

   /// If true then no corrections are sent from the server 
   /// and/or applied from the client.
   ///
   /// This is only ment for debugging.
   ///
   static bool smNoCorrections;

   /// If true then no smoothing is done on the client when
   /// applying server corrections.
   ///
   /// This is only ment for debugging.
   ///
   static bool smNoSmoothing;

public:

   PhysicsShape();
   virtual ~PhysicsShape();

   DECLARE_CONOBJECT( PhysicsShape );

   /// Returns the PhysicsShapeData datablock.
   PhysicsShapeData* getDataBlock() { return static_cast<PhysicsShapeData*>( Parent::getDataBlock() ); }

   // SimObject
   static void consoleInit();
   static void initPersistFields();
   void inspectPostApply();
   bool onAdd();
   void onRemove();   
   
   // SceneObject
   void prepRenderImage( SceneRenderState *state );
   void setTransform( const MatrixF &mat );
   F32 getMass() const;
   Point3F getVelocity() const { return mState.linVelocity; }
   void applyImpulse( const Point3F &pos, const VectorF &vec );
   void applyRadialImpulse( const Point3F &origin, F32 radius, F32 magnitude );
   void applyTorque( const Point3F &torque );
   void applyForce( const Point3F &force );
   void setScale(const VectorF & scale);

   // GameBase
   bool onNewDataBlock( GameBaseData *dptr, bool reload );
   void interpolateTick( F32 delta );
   void processTick( const Move *move );
   void advanceTime( F32 timeDelta );
   U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *conn, BitStream *stream );

   bool isDestroyed() const { return mDestroyed; }
   void destroy();
   void restore();

   /// Save the current transform as where we return to when a physics reset
   /// event occurs. This is automatically set in onAdd but some manipulators
   /// such as Prefab need to make use of this.
   void storeRestorePos();
};

#endif // _PHYSICSSHAPE_H_
