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

#ifndef STATIC_MESH_COMPONENT_H
#define STATIC_MESH_COMPONENT_H

#ifndef COMPONENT_H
#include "T3D/components/component.h"
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
#ifndef ENTITY_H
#include "T3D/entity.h"
#endif
#ifndef _NETSTRINGTABLE_H_
   #include "sim/netStringTable.h"
#endif
#ifndef CORE_INTERFACES_H
#include "T3D/components/coreInterfaces.h"
#endif
#ifndef RENDER_COMPONENT_INTERFACE_H
#include "T3D/components/render/renderComponentInterface.h"
#endif
#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif 
#ifndef _SHAPE_ASSET_H_
#include "T3D/assets/ShapeAsset.h"
#endif 
#ifndef _GFXVERTEXFORMAT_H_
#include "gfx/gfxVertexFormat.h"
#endif

class TSShapeInstance;
class SceneRenderState;
//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class MeshComponent : public Component,
   public RenderComponentInterface,
   public CastRayRenderedInterface,
   public EditorInspectInterface
{
   typedef Component Parent;

protected:
   enum
   {
      ShapeMask = Parent::NextFreeMask,
      MaterialMask = Parent::NextFreeMask << 1,
      NextFreeMask = Parent::NextFreeMask << 2,
   };

   StringTableEntry		mShapeName;
   StringTableEntry		mShapeAsset;
   TSShape*		         mShape;
   Box3F						mShapeBounds;
   Point3F					mCenterOffset;

   struct matMap
   {
      String matName;
      U32 slot;
   };

   Vector<matMap>  mChangingMaterials;
   Vector<matMap>  mMaterials;

   class boneObject : public SimGroup
   {
      MeshComponent *mOwner;
   public:
      boneObject(MeshComponent *owner){ mOwner = owner; }

      StringTableEntry mBoneName;
      S32 mItemID;

      virtual void addObject(SimObject *obj);
   };

   Vector<boneObject*> mNodesList;

public:
   StringTableEntry       mMeshAssetId;
   AssetPtr<ShapeAsset>   mMeshAsset;

   TSShapeInstance*       mShapeInstance;

public:
   MeshComponent();
   virtual ~MeshComponent();
   DECLARE_CONOBJECT(MeshComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void inspectPostApply();

   virtual void prepRenderImage(SceneRenderState *state);

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   Box3F getShapeBounds() { return mShapeBounds; }

   virtual MatrixF getNodeTransform(S32 nodeIdx);
   S32 getNodeByName(String nodeName);

   void setupShape();
   void updateShape();
   void updateMaterials();

   virtual void onComponentRemove();
   virtual void onComponentAdd();

   static bool _setMesh(void *object, const char *index, const char *data);
   static bool _setShape(void *object, const char *index, const char *data);
   const char* _getShape(void *object, const char *data);

   bool setMeshAsset(const char* assetName);

   virtual TSShape* getShape() { if (mMeshAsset)  return mMeshAsset->getShape(); else return NULL; }
   virtual TSShapeInstance* getShapeInstance() { return mShapeInstance; }

   Resource<TSShape> getShapeResource() { return mMeshAsset->getShapeResource(); }

   void _onResourceChanged(const Torque::Path &path);

   virtual bool castRayRendered(const Point3F &start, const Point3F &end, RayInfo *info);

   void mountObjectToNode(SceneObject* objB, String node, MatrixF txfm);

   virtual void onDynamicModified(const char* slotName, const char* newValue);

   void changeMaterial(U32 slot, const char* newMat);

   virtual void onInspect();
   virtual void onEndInspect();

   virtual Vector<MatrixF> getNodeTransforms()
   {
      Vector<MatrixF> bob;
      return bob;
   }

   virtual void setNodeTransforms(Vector<MatrixF> transforms)
   {
      return;
   }
};

#endif
