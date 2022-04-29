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
#ifndef SHAPE_COLLISION_COMPONENT_H
#define SHAPE_COLLISION_COMPONENT_H

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
#ifndef ENTITY_H
#include "T3D/entity.h"
#endif
#ifndef CORE_INTERFACES_H
#include "T3D/components/coreInterfaces.h"
#endif
#ifndef COLLISION_COMPONENT_H
#include "T3D/components/collision/collisionComponent.h"
#endif
#ifndef RENDER_COMPONENT_INTERFACE_H
#include "T3D/components/render/renderComponentInterface.h"
#endif

class TSShapeInstance;
class SceneRenderState;
class ShapeCollisionComponent;
class PhysicsBody;
class PhysicsWorld;

class ShapeCollisionComponent : public CollisionComponent
{
   typedef Component Parent;
public:
   enum MeshType
   {
      None = 0,            ///< No mesh
      Bounds = 1,          ///< Bounding box of the shape
      CollisionMesh = 2,   ///< Specifically designated collision meshes
      VisibleMesh = 3      ///< Rendered mesh polygons
   };

protected:
   MeshType mCollisionType;
   MeshType mDecalType;
   MeshType mLOSType;

   Vector<S32> mCollisionDetails;
   Vector<S32> mLOSDetails;

   StringTableEntry colisionMeshPrefix;

   RenderComponentInterface* mOwnerRenderInterface;

   PhysicsComponent* mOwnerPhysicsComp;

   //only really relevent for the collision mesh type
   //if we note an animation component is added, we flag as being animated.
   //This way, if we're using collision meshes, we can set it up to update their transforms
   //as needed
   bool mAnimated;

   enum
   {
      ColliderMask = Parent::NextFreeMask,
   };

public:
   ShapeCollisionComponent();
   virtual ~ShapeCollisionComponent();
   DECLARE_CONOBJECT(ShapeCollisionComponent);

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   virtual void componentAddedToOwner(Component *comp);
   virtual void componentRemovedFromOwner(Component *comp);
   virtual void ownerTransformSet(MatrixF *mat);
   void targetShapeChanged(RenderComponentInterface* instanceInterface);

   virtual void onComponentRemove();
   virtual void onComponentAdd();

   virtual void checkDependencies();

   static void initPersistFields();

   void inspectPostApply();

   //Setup
   virtual void prepCollision();

   //Updates
   virtual void processTick();
   
   PhysicsCollision* buildColShapes();

   void updatePhysics();

   virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);

   virtual bool buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF &sphere){ return false; }

   virtual PhysicsCollision* getCollisionData();

};

typedef ShapeCollisionComponent::MeshType CollisionMeshMeshType;
DefineEnumType(CollisionMeshMeshType);

#endif // COLLISION_COMPONENT_H
