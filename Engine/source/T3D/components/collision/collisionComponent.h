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
#pragma once

#ifndef COLLISION_COMPONENT_H
#define COLLISION_COMPONENT_H

#ifndef COMPONENT_H
#include "T3D/components/component.h"
#endif

#ifndef _CONVEX_H_
#include "collision/convex.h"
#endif
#ifndef _COLLISION_H_
#include "collision/collision.h"
#endif
#ifndef _EARLYOUTPOLYLIST_H_
#include "collision/earlyOutPolyList.h"
#endif
#ifndef _SIM_H_
#include "console/sim.h"
#endif
#ifndef _SCENECONTAINER_H_
#include "scene/sceneContainer.h"
#endif
#ifndef _T3D_PHYSICSCOMMON_H_
#include "T3D/physics/physicsCommon.h"
#endif
#ifndef PHYSICS_COMPONENT_H
#include "T3D/components/physics/physicsComponent.h"
#endif
#ifndef _T3D_PHYSICS_PHYSICSWORLD_H_
#include "T3D/physics/physicsWorld.h"
#endif

struct CollisionContactInfo
{
   bool contacted, move;
   SceneObject *contactObject;
   VectorF  idealContactNormal;
   VectorF  contactNormal;
   Point3F  contactPoint;
   F32	   contactTime;
   S32	   contactTimer;
   BaseMatInstance *contactMaterial;

   Vector<SceneObject*> overlapObjects;

   void clear()
   {
      contacted=move=false; 
      contactObject = NULL; 
      contactNormal.set(0,0,0);
      contactTime = 0.f;
      contactTimer = 0;
      idealContactNormal.set(0, 0, 1);
      contactMaterial = NULL;
      overlapObjects.clear();
   }

   CollisionContactInfo() { clear(); }

};

class CollisionComponent : public Component
{
   typedef Component Parent;

public:
	// CollisionTimeout
	// This struct lets us track our collisions and estimate when they've have timed out and we'll need to act on it.
	struct CollisionTimeout 
   {
      CollisionTimeout* next;
      SceneObject* object;
      U32 objectNumber;
      SimTime expireTime;
      VectorF vector;
   };

   Signal< void( SceneObject* ) > onCollisionSignal;
   Signal< void( SceneObject* ) > onContactSignal;

protected:
   PhysicsWorld* mPhysicsWorld;
   PhysicsBody* mPhysicsRep;

   CollisionTimeout* mTimeoutList;
   static CollisionTimeout* sFreeTimeoutList;

   CollisionList mCollisionList;
   Vector<CollisionComponent*> mCollisionNotifyList;

   CollisionContactInfo mContactInfo;

   U32 CollisionMoveMask;

   bool mBlockColliding;

   bool mCollisionInited;

   void handleCollisionNotifyList();

   void queueCollision( SceneObject *obj, const VectorF &vec);

	/// checkEarlyOut
	/// This function lets you trying and early out of any expensive collision checks by using simple extruded poly boxes representing our objects
	/// If it returns true, we know we won't hit with the given parameters and can successfully early out. If it returns false, our test case collided
	/// and we should do the full collision sim.
	bool checkEarlyOut(Point3F start, VectorF velocity, F32 time, Box3F objectBox, Point3F objectScale, 
														Box3F collisionBox, U32 collisionMask, CollisionWorkingList &colWorkingList);

public:
   CollisionComponent();
   virtual ~CollisionComponent();

   DECLARE_CONOBJECT(CollisionComponent);

   //Setup
   virtual void prepCollision() {};

   /// checkCollisions
   // This is our main function for checking if a collision is happening based on the start point, velocity and time
   // We do the bulk of the collision checking in here
   //virtual bool checkCollisions( const F32 travelTime, Point3F *velocity, Point3F start )=0;

   CollisionList *getCollisionList() { return &mCollisionList; }

   void clearCollisionList() { mCollisionList.clear(); }

   void clearCollisionNotifyList() { mCollisionNotifyList.clear(); }

   Collision *getCollision(S32 col);

   CollisionContactInfo* getContactInfo() { return &mContactInfo; }

	enum PublicConstants { 
      CollisionTimeoutValue = 250
   };

   bool doesBlockColliding() { return mBlockColliding; }

   /// handleCollisionList
   /// This basically takes in a CollisionList and calls handleCollision for each.
   void handleCollisionList(CollisionList &collisionList, VectorF velocity);

   /// handleCollision
   /// This will take a collision and queue the collision info for the object so that in knows about the collision.
   void handleCollision(Collision &col, VectorF velocity);

   virtual bool checkCollisions(const F32 travelTime, Point3F *velocity, Point3F start);
   virtual bool updateCollisions(F32 time, VectorF vector, VectorF velocity);
   virtual void updateWorkingCollisionSet(const U32 mask);

   //
   bool buildConvexOpcode(TSShapeInstance* sI, S32 dl, const Box3F &bounds, Convex *c, Convex *list);
   bool buildMeshOpcode(TSMesh *mesh, const MatrixF &meshToObjectMat, const Box3F &bounds, Convex *convex, Convex *list);

   bool castRayOpcode(S32 dl, const Point3F & startPos, const Point3F & endPos, RayInfo *info);
   bool castRayMeshOpcode(TSMesh *mesh, const Point3F &s, const Point3F &e, RayInfo *info, TSMaterialList *materials);

   virtual PhysicsCollision* getCollisionData() {
      return nullptr;
   }

   virtual PhysicsBody *getPhysicsRep()
   {
      return mPhysicsRep;
   }

   void buildConvex(const Box3F& box, Convex* convex) {}
   bool buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF &sphere) { return false; }

   //
   Point3F getContactNormal();
   bool hasContact();
   S32 getCollisionCount();
   Point3F getCollisionNormal(S32 collisionIndex);
   F32 getCollisionAngle(S32 collisionIndex, Point3F upVector);
   S32 getBestCollision(Point3F upVector);
   F32 getBestCollisionAngle(VectorF upVector);

   Signal< void(PhysicsCollision* collision) > onCollisionChanged;
};

#endif