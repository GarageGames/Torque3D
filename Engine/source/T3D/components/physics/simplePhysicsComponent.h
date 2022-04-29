//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#ifndef SIMPLE_PHYSICS_COMPONENT_H
#define SIMPLE_PHYSICS_COMPONENT_H

#ifndef PHYSICS_COMPONENT_H
#include "T3D/components/physics/physicsComponent.h"
#endif

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
#include "T3D/Entity.h"
#endif
#ifndef _CONVEX_H_
#include "collision/convex.h"
#endif
#ifndef _BOXCONVEX_H_
#include "collision/boxConvex.h"
#endif

class SceneRenderState;
class PhysicsBody;
//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class SimplePhysicsComponent : public PhysicsComponent
{
   typedef PhysicsComponent Parent;

protected:
   F32 mBuoyancy;
   F32 mFriction;
   F32 mElasticity;
   F32 mMaxVelocity;
   bool mSticky;

   U32 mIntegrationCount;

   Point3F moveSpeed;

   Point3F mStickyCollisionPos;
   Point3F mStickyCollisionNormal;

public:
   SimplePhysicsComponent();
   virtual ~SimplePhysicsComponent();
   DECLARE_CONOBJECT(SimplePhysicsComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   virtual void processTick();
   virtual void interpolateTick(F32 dt);
   virtual void updatePos(const F32 dt);
   void updateForces();

   void updateMove(const Move* move);
   Point3F _move(const F32 travelTime);

   //virtual void onComponentRemove();

   virtual VectorF getVelocity() { return mVelocity; }
   virtual void setVelocity(const VectorF& vel);

   //
   DECLARE_CALLBACK(void, updateMove, (SimplePhysicsComponent* obj));
};

#endif // _COMPONENT_H_
