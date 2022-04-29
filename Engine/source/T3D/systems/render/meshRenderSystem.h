#pragma once
#include "scene/sceneRenderState.h"
#include "T3D/systems/componentSystem.h"
#include "ts/tsShape.h"
#include "ts/tsShapeInstance.h"
#include "T3D/assets/ShapeAsset.h"
#include "T3D/assets/MaterialAsset.h"

#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _OPTIMIZEDPOLYLIST_H_
#include "collision/optimizedPolyList.h"
#endif

class MeshRenderSystemInterface : public SystemInterface<MeshRenderSystemInterface>
{
public:
   TSShapeInstance * mShapeInstance;

   MatrixF                 mTransform;
   Point3F                 mScale;
   Box3F						   mBounds;
   SphereF                 mSphere;

   bool                    mIsClient;

   struct matMap
   {
      //MaterialAsset* matAsset;
      String assetId;
      U32 slot;
   };

   Vector<matMap>  mChangingMaterials;
   Vector<matMap>  mMaterials;

   MeshRenderSystemInterface() : SystemInterface(), mShapeInstance(nullptr), mTransform(MatrixF::Identity), mScale(Point3F::One), mIsClient(false)
   {
      mBounds = Box3F(1);
      mSphere = SphereF();
   }

   ~MeshRenderSystemInterface()
   {
      //SAFE_DELETE(mShape);
      SAFE_DELETE(mShapeInstance);
   }
};

class MeshRenderSystem
{
public:
   //Core render function, which does all the real work
   static void render(SceneManager *sceneManager, SceneRenderState* state);

   //Render our particular interface's data
   static void renderInterface(U32 interfaceIndex, SceneRenderState* state);
};