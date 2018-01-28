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

   //Static geometry stuff
   bool                    mStatic;

   OptimizedPolyList       mGeometry;

   MeshRenderSystemInterface() : SystemInterface(), mShapeInstance(nullptr), mTransform(MatrixF::Identity), mScale(Point3F::One), mIsClient(false), mStatic(false)
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
protected:
   /*struct StaticBatchElement
   {
      SimObject* owner;
      OptimizedPolyList geometry;
      String batchName;
   };

   static Vector<StaticBatchElement> mStaticElements;*/

   //We retain the pushed geometry data for rendering here. It's static(unless forced to change through editing or whatnot)
   //so rendering the batches is real fast
   struct BufferMaterials
   {
      // The name of the Material we will use for rendering
      String            mMaterialName;
      // The actual Material instance
      BaseMatInstance*  mMaterialInst;

      BufferMaterials()
      {
         mMaterialName = "";
         mMaterialInst = NULL;
      }
   };

   static Vector<BufferMaterials> mBufferMaterials;

   struct BufferSet
   {
      U32 surfaceMaterialId;

      U32 vertCount;
      U32 primCount;

      Point3F center;

      struct Buffers
      {
         U32 vertStart;
         U32 primStart;
         U32 vertCount;
         U32 primCount;

         Vector<GFXVertexPNTT> vertData;
         Vector<U32> primData;

         GFXVertexBufferHandle< GFXVertexPNTT > vertexBuffer;
         GFXPrimitiveBufferHandle            primitiveBuffer;

         Buffers()
         {
            vertStart = 0;
            primStart = 0;
            vertCount = 0;
            primCount = 0;

            vertexBuffer = NULL;
            primitiveBuffer = NULL;
         }
      };

      Vector<Buffers> buffers;

      BufferSet()
      {
         Buffers newBuffer;
         buffers.push_back(newBuffer);

         surfaceMaterialId = 0;

         vertCount = 0;
         primCount = 0;

         center = Point3F::Zero;
      }
   };

   static Vector<BufferSet> mStaticBuffers;

public:
   /*virtual void prepRenderImage(SceneRenderState *state);

   bool setMeshAsset(const char* assetName);

   virtual TSShape* getShape() { if (mMeshAsset)  return mMeshAsset->getShape(); else return NULL; }
   virtual TSShapeInstance* getShapeInstance() { return mShapeInstance; }

   Resource<TSShape> getShapeResource() { return mMeshAsset->getShapeResource(); }

   void _onResourceChanged(const Torque::Path &path);

   virtual bool castRayRendered(const Point3F &start, const Point3F &end, RayInfo *info);

   void mountObjectToNode(SceneObject* objB, String node, MatrixF txfm);

   virtual void onDynamicModified(const char* slotName, const char* newValue);

   void changeMaterial(U32 slot, MaterialAsset* newMat);
   bool setMatInstField(U32 slot, const char* field, const char* value);

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
   }*/

   /*MeshRenderSystem()
   {

   }
   virtual ~MeshRenderSystem()
   {
      smInterfaceList.clear();
   }

   static MeshComponentInterface* GetNewInterface()
   {
      smInterfaceList.increment();

      return &smInterfaceList.last();
   }

   static void RemoveInterface(T* q)
   {
      smInterfaceList.erase(q);
   }*/

   //Core render function, which does all the real work
   static void render(SceneManager *sceneManager, SceneRenderState* state);

   //Render our particular interface's data
   static void renderInterface(U32 interfaceIndex, SceneRenderState* state);

   //Static Batch rendering
   static void rebuildBuffers();

   static U32 findBufferSetByMaterial(U32 matId);
};