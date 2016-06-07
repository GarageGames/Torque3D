//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSICSBEHAVIOR_H_
#define _PHYSICSBEHAVIOR_H_
#include "T3D/components/component.h"

#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif
#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif
#ifndef _SCENERENDERSTATE_H_
#include "scene/sceneRenderState.h"
#endif
#ifndef _MBOX_H_
#include "math/mBox.h"
#endif
#ifndef _ENTITY_H_
#include "T3D/entity.h"
#endif
#ifndef _CONVEX_H_
#include "collision/convex.h"
#endif
#ifndef _BOXCONVEX_H_
#include "collision/boxConvex.h"
#endif
#ifndef _RIGID_H_
#include "T3D/rigid.h"
#endif
#ifndef _T3D_PHYSICS_PHYSICSBODY_H_
#include "T3D/physics/physicsBody.h"
#endif

#ifndef _RENDER_COMPONENT_INTERFACE_H_
#include "T3D/components/render/renderComponentInterface.h"
#endif

class TSShapeInstance;
class SceneRenderState;
class PhysicsBody;
class PhysicsBehaviorInstance;
//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class PhysicsComponent : public Component
{
   typedef Component Parent;

protected:
   bool mStatic;
   bool mAtRest;
   S32  mAtRestCounter;

   VectorF mGravity;
   VectorF mVelocity;
   F32     mDrag;
   F32		mMass;

   F32		mGravityMod;

   S32 csmAtRestTimer;
   F32 sAtRestVelocity;      // Min speed after collisio

public:
   enum MaskBits {
      PositionMask = Parent::NextFreeMask << 0,
      FreezeMask = Parent::NextFreeMask << 1,
      ForceMoveMask = Parent::NextFreeMask << 2,
      VelocityMask = Parent::NextFreeMask << 3,
      NextFreeMask = Parent::NextFreeMask << 4
   };

   struct StateDelta
   {
      Move move;                    ///< Last move from server
      F32 dt;                       ///< Last interpolation time
      // Interpolation data
      Point3F pos;
      Point3F posVec;
      QuatF rot[2];
      // Warp data
      S32 warpTicks;                ///< Number of ticks to warp
      S32 warpCount;                ///< Current pos in warp
      Point3F warpOffset;
      QuatF warpRot[2];
   };

   StateDelta mDelta;
   S32 mPredictionCount;            ///< Number of ticks to predict

public:
   PhysicsComponent();
   virtual ~PhysicsComponent();
   DECLARE_CONOBJECT(PhysicsComponent);

   static void initPersistFields();

   virtual void interpolateTick(F32 dt);
   virtual void updatePos(const U32 /*mask*/, const F32 dt){}
   virtual void _updatePhysics();
   virtual PhysicsBody *getPhysicsRep();

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   virtual void onComponentAdd();

   void updateContainer();

   virtual void updateVelocity(const F32 dt);
   virtual Point3F getVelocity() { return mVelocity; }
   virtual void getOriginVector(const Point3F &p, Point3F* r);
   virtual void getVelocity(const Point3F& r, Point3F* v);
   virtual void setVelocity(const VectorF& vel);
   virtual void setTransform(const MatrixF& mat);
   virtual void setPosition(const Point3F& pos);
   void setRenderPosition(const Point3F& pos, F32 dt);

   virtual void applyImpulse(const Point3F&, const VectorF& vec);
   virtual F32 getZeroImpulse(const Point3F& r, const Point3F& normal);
   virtual void accumulateForce(F32 dt, Point3F force);

   //Rigid Body Collision Conveinence Hooks
   virtual bool updateCollision(F32 dt, Rigid& ns, CollisionList &cList) { return false; }
   virtual bool resolveContacts(Rigid& ns, CollisionList& cList, F32 dt) { return false; }
   //virtual bool resolveCollision(Rigid&  ns, CollisionList& cList) { return false; }
   virtual bool resolveCollision(const Point3F& p, const Point3F &normal) { return false; }
};

#endif // _COMPONENT_H_
