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
#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "T3D/components/render/meshComponent.h"
#include "core/util/safeDelete.h"
#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "core/stream/bitStream.h"
#include "sim/netConnection.h"
#include "gfx/gfxTransformSaver.h"
#include "console/engineAPI.h"
#include "lighting/lightQuery.h"
#include "scene/sceneManager.h"
#include "gfx/bitmap/ddsFile.h"
#include "gfx/gfxTextureManager.h"
#include "materials/materialFeatureTypes.h"
#include "renderInstance/renderImposterMgr.h"
#include "util/imposterCapture.h"
#include "gfx/sim/debugDraw.h"  
#include "gfx/gfxDrawUtil.h"
#include "materials/materialManager.h"
#include "materials/matInstance.h"
#include "core/strings/findMatch.h"
#include "T3D/components/render/meshComponent_ScriptBinding.h"

ImplementEnumType(BatchingMode,
   "Type of mesh data available in a shape.\n"
   "@ingroup gameObjects")
{
   MeshComponent::Individual, "Individual", "This mesh is rendered indivudally, wthout batching or instancing."
},
   { MeshComponent::StaticBatch, "Static Batching", "Statically batches this mesh together with others to reduce drawcalls." },
   //{ MeshComponent::DynamicBatch, "Dynamic Batching", "Dynamical batches this mesh together with others to reduce drawcalls each frame." },
  // { MeshComponent::Instanced, "Instanced", "This mesh is rendered as an instance, reducing draw overhead with others that share the same mesh and material." },
      EndImplementEnumType;

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////
MeshComponent::MeshComponent() : Component(), mShape(nullptr), mRenderMode(Individual)
{
   mFriendlyName = "Mesh Component";
   mComponentType = "Render";

   mDescription = getDescriptionText("Causes the object to render a non-animating 3d shape using the file provided.");

   mNetworked = true;

   mShapeName = StringTable->EmptyString();
   mShapeAsset = StringTable->EmptyString();

   mMeshAsset = StringTable->EmptyString();
   mMeshAssetId = StringTable->EmptyString();

   mInterfaceData = new MeshRenderSystemInterface();

   mRenderMode = Individual;
}

MeshComponent::~MeshComponent()
{
   if (mInterfaceData)
      SAFE_DELETE(mInterfaceData);
}

IMPLEMENT_CO_NETOBJECT_V1(MeshComponent);

//==========================================================================================
bool MeshComponent::onAdd()
{
   if(! Parent::onAdd())
      return false;

   // Register for the resource change signal.
   ResourceManager::get().getChangedSignal().notify( this, &MeshComponent::_onResourceChanged );

   return true;
}

void MeshComponent::onComponentAdd()
{
   Parent::onComponentAdd();

   if (isClientObject())
      mInterfaceData->mIsClient = true;

  // if (mInterfaceData != nullptr)
  //   mInterfaceData->mIsClient = isClientObject();

   //get the default shape, if any
   updateShape();
}

void MeshComponent::onRemove()
{
   Parent::onRemove();
}

void MeshComponent::onComponentRemove()
{
   if(mOwner)
   {
      Point3F pos = mOwner->getPosition(); //store our center pos
      mOwner->setObjectBox(Box3F(Point3F(-1,-1,-1), Point3F(1,1,1)));
      mOwner->setPosition(pos);
   }  

   Parent::onComponentRemove();  
}

void MeshComponent::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("Rendering");
   addField("BatchingMode", TypeBatchingMode, Offset(mRenderMode, MeshComponent),
      "The mode of batching this shape should be rendered with.");
   endGroup("Rendering");

   //create a hook to our internal variables
   addGroup("Model");
   addProtectedField("MeshAsset", TypeShapeAssetPtr, Offset(mShapeAsset, MeshComponent), &_setMesh, &defaultProtectedGetFn,
      "The asset Id used for the mesh.", AbstractClassRep::FieldFlags::FIELD_ComponentInspectors);
   endGroup("Model");
}

bool MeshComponent::_setMesh(void *object, const char *index, const char *data)
{
   MeshComponent *rbI = static_cast<MeshComponent*>(object);
   
   // Sanity!
   AssertFatal(data != NULL, "Cannot use a NULL asset Id.");

   return rbI->setMeshAsset(data);
}

bool MeshComponent::_setShape( void *object, const char *index, const char *data )
{
   MeshComponent *rbI = static_cast<MeshComponent*>(object);
   rbI->mShapeName = StringTable->insert(data);
   rbI->updateShape(); //make sure we force the update to resize the owner bounds
   rbI->setMaskBits(ShapeMask);

   return true;
}

bool MeshComponent::setMeshAsset(const char* assetName)
{
   // Fetch the asset Id.
   if (mInterfaceData == nullptr)
      return false;

   mMeshAssetId = StringTable->insert(assetName);

   mMeshAsset = mMeshAssetId;

   if (mMeshAsset.isNull())
   {
      Con::errorf("[MeshComponent] Failed to load mesh asset.");
      return false;
   }

   mShapeName = mMeshAssetId;
   mShapeAsset = mShapeName;
   updateShape(); //make sure we force the update to resize the owner bounds
   setMaskBits(ShapeMask);

   return true;
}

void MeshComponent::updateShape()
{
   if (mInterfaceData == nullptr)
      return;

   //if ((mShapeName && mShapeName[0] != '\0') || (mShapeAsset && mShapeAsset[0] != '\0'))
   if ((mShapeName && mShapeName[0] != '\0') || (mMeshAssetId && mMeshAssetId[0] != '\0'))

   {
      if (mMeshAsset == NULL)
         return;

      mShape = mMeshAsset->getShape();

      if (!mMeshAsset->getShape())
         return;

      setupShape();

      //Do this on both the server and client
      S32 materialCount = mMeshAsset->getShape()->materialList->getMaterialNameList().size();

      if (isServerObject())
      {
         //we need to update the editor
         for (U32 i = 0; i < mFields.size(); i++)
         {
            //find any with the materialslot title and clear them out
            if (FindMatch::isMatch("MaterialSlot*", mFields[i].mFieldName, false))
            {
               setDataField(mFields[i].mFieldName, NULL, "");
               mFields.erase(i);
               continue;
            }
         }

         //next, get a listing of our materials in the shape, and build our field list for them
         char matFieldName[128];

         if (materialCount > 0)
            mComponentGroup = StringTable->insert("Materials");

         for (U32 i = 0; i < materialCount; i++)
         {
            String materialname = mMeshAsset->getShape()->materialList->getMaterialName(i);
            if (materialname == String("ShapeBounds"))
               continue;

            dSprintf(matFieldName, 128, "MaterialSlot%d", i);

            addComponentField(matFieldName, "A material used in the shape file", "Material", materialname, "");
         }

         if (materialCount > 0)
            mComponentGroup = "";
      }

      if (mOwner != NULL)
      {
         Point3F min, max, pos;
         pos = mOwner->getPosition();

         mOwner->getWorldToObj().mulP(pos);

         min = mMeshAsset->getShape()->mBounds.minExtents;
         max = mMeshAsset->getShape()->mBounds.maxExtents;

         if (mInterfaceData)
         {
            mInterfaceData->mBounds.set(min, max);
            mInterfaceData->mScale = mOwner->getScale();
            mInterfaceData->mTransform = mOwner->getRenderTransform();
         }

         mOwner->setObjectBox(Box3F(min, max));

         mOwner->resetWorldBox();

         if (mOwner->getSceneManager() != NULL)
            mOwner->getSceneManager()->notifyObjectDirty(mOwner);
      }

      if (isClientObject() && mInterfaceData)
      {
         if (mRenderMode == StaticBatch)
         {
            mInterfaceData->mStatic = true;

            OptimizedPolyList geom;
            MatrixF transform = mInterfaceData->mTransform;
            mInterfaceData->mGeometry.setTransform(&transform, mInterfaceData->mScale);
            mInterfaceData->mGeometry.setObject(mOwner);

            mInterfaceData->mShapeInstance->buildPolyList(&mInterfaceData->mGeometry, 0);
         }
         else
         {
            mInterfaceData->mStatic = false;
         }

         MeshRenderSystem::rebuildBuffers();
      }

      //finally, notify that our shape was changed
      onShapeInstanceChanged.trigger(this);
   }
}

void MeshComponent::setupShape()
{
   mInterfaceData->mShapeInstance = new TSShapeInstance(mMeshAsset->getShape(), true);
}

void MeshComponent::_onResourceChanged( const Torque::Path &path )
{
   if (mInterfaceData == nullptr)
      return;

   String filePath;
   if (mMeshAsset)
      filePath = Torque::Path(mMeshAsset->getShapeFilename());

   if (!mMeshAsset || path != Torque::Path(mMeshAsset->getShapeFilename()) )
      return;

   updateShape();
   setMaskBits(ShapeMask);
}

void MeshComponent::inspectPostApply()
{
   Parent::inspectPostApply();
}

U32 MeshComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (!mOwner || con->getGhostIndex(mOwner) == -1)
   {
      stream->writeFlag(false);
      stream->writeFlag(false);

      if (mask & ShapeMask)
         retMask |= ShapeMask;
      if (mask & MaterialMask)
         retMask |= MaterialMask;
      return retMask;
   }

   if (stream->writeFlag(mask & ShapeMask))
   {
      stream->writeString(mShapeName);

      stream->writeInt(mRenderMode, 8);
   }

   if (stream->writeFlag( mask & MaterialMask ))
   {
      stream->writeInt(mChangingMaterials.size(), 16);

      for(U32 i=0; i < mChangingMaterials.size(); i++)
      {
         stream->writeInt(mChangingMaterials[i].slot, 16);

         NetStringHandle matNameStr = mChangingMaterials[i].assetId.c_str();
         con->packNetStringHandleU(stream, matNameStr);
      }

      mChangingMaterials.clear();
    }

   return retMask;
}

void MeshComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   if(stream->readFlag())
   {
      mShapeName = stream->readSTString();

      mRenderMode = (RenderMode)stream->readInt(8);
      setMeshAsset(mShapeName);
      updateShape();
   }

   if(stream->readFlag())
   {
      mChangingMaterials.clear();
      U32 materialCount = stream->readInt(16);

      for(U32 i=0; i < materialCount; i++)
      {
         matMap newMatMap;
         newMatMap.slot = stream->readInt(16);
         newMatMap.assetId = String(con->unpackNetStringHandleU(stream).getString());

         //do the lookup, now
         newMatMap.matAsset = AssetDatabase.acquireAsset<MaterialAsset>(newMatMap.assetId);

         mChangingMaterials.push_back(newMatMap);
      }

      updateMaterials();
   }
}

void MeshComponent::prepRenderImage( SceneRenderState *state )
{
   /*if (!mEnabled || !mOwner || !mShapeInstance)
      return;

   Point3F cameraOffset;
   mOwner->getRenderTransform().getColumn(3, &cameraOffset);
   cameraOffset -= state->getDiffuseCameraPosition();
   F32 dist = cameraOffset.len();
   if (dist < 0.01f)
      dist = 0.01f;

   Point3F objScale = getOwner()->getScale();
   F32 invScale = (1.0f / getMax(getMax(objScale.x, objScale.y), objScale.z));

   mShapeInstance->setDetailFromDistance(state, dist * invScale);

   if (mShapeInstance->getCurrentDetail() < 0)
      return;

   GFXTransformSaver saver;

   // Set up our TS render state.
   TSRenderState rdata;
   rdata.setSceneState(state);
   rdata.setFadeOverride(1.0f);
   rdata.setOriginSort(false);

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init(mOwner->getWorldSphere());
   rdata.setLightQuery(&query);

   MatrixF mat = mOwner->getRenderTransform();

   if (mOwner->isMounted())
   {
      MatrixF wrldPos = mOwner->getWorldTransform();
      Point3F wrldPosPos = wrldPos.getPosition();

      Point3F mntPs = mat.getPosition();
      EulerF mntRt = RotationF(mat).asEulerF();

      bool tr = true;
   }

   mat.scale(objScale);
   GFX->setWorldMatrix(mat);

   mShapeInstance->render(rdata);*/
}

void MeshComponent::updateMaterials()
{
   if (mChangingMaterials.empty() || !mMeshAsset->getShape())
      return;

   TSMaterialList* pMatList = mInterfaceData->mShapeInstance->getMaterialList();
   pMatList->setTextureLookupPath(getShapeResource().getPath().getPath());

   const Vector<String> &materialNames = pMatList->getMaterialNameList();
   for ( S32 i = 0; i < materialNames.size(); i++ )
   {
      for(U32 m=0; m < mChangingMaterials.size(); m++)
      {
         if(mChangingMaterials[m].slot == i)
         {
            //Fetch the actual material asset
            pMatList->renameMaterial( i, mChangingMaterials[m].matAsset->getMaterialDefinitionName());
         }
      }

      mChangingMaterials.clear();
   }

   // Initialize the material instances
   mInterfaceData->mShapeInstance->initMaterialList();
}

MatrixF MeshComponent::getNodeTransform(S32 nodeIdx)
{
   if (mInterfaceData != nullptr && mMeshAsset->getShape())
   {
      S32 nodeCount = getShape()->nodes.size();

      if(nodeIdx >= 0 && nodeIdx < nodeCount)
      {
         //animate();
         MatrixF nodeTransform = mInterfaceData->mShapeInstance->mNodeTransforms[nodeIdx];
         const Point3F& scale = mOwner->getScale();

         // The position of the node needs to be scaled.
         Point3F position = nodeTransform.getPosition();
         position.convolve(scale);
         nodeTransform.setPosition(position);

         MatrixF finalTransform = MatrixF::Identity;

         finalTransform.mul(mOwner->getRenderTransform(), nodeTransform);

         return finalTransform;
      }
   }

   return MatrixF::Identity;
}

S32 MeshComponent::getNodeByName(String nodeName)
{
   if (mMeshAsset->getShape())
   {
      S32 nodeIdx = getShape()->findNode(nodeName);

      return nodeIdx;
   }

   return -1;
}

bool MeshComponent::castRayRendered(const Point3F &start, const Point3F &end, RayInfo *info)
{
   return false;
}

void MeshComponent::mountObjectToNode(SceneObject* objB, String node, MatrixF txfm)
{
   const char* test;
   test = node.c_str();
   if(dIsdigit(test[0]))
   {
      getOwner()->mountObject(objB, dAtoi(node), txfm);
   }
   else
   {
      if(TSShape* shape = getShape())
      {
         S32 idx = shape->findNode(node);
         getOwner()->mountObject(objB, idx, txfm);
      }
   }
}

void MeshComponent::onDynamicModified(const char* slotName, const char* newValue)
{
   if(FindMatch::isMatch( "materialslot*", slotName, false ))
   {
      if(!getShape())
         return;

      S32 slot = -1;
      String outStr( String::GetTrailingNumber( slotName, slot ) );

      if(slot == -1)
         return;

      //Safe to assume the inbound value for the material will be a MaterialAsset, so lets do a lookup on the name
      MaterialAsset* matAsset = AssetDatabase.acquireAsset<MaterialAsset>(newValue);
      if (!matAsset)
         return;

      bool found = false;
      for(U32 i=0; i < mChangingMaterials.size(); i++)
      {
         if(mChangingMaterials[i].slot == slot)
         {
            mChangingMaterials[i].matAsset = matAsset;
            mChangingMaterials[i].assetId = newValue;
            found = true;
         }
      }

      if(!found)
      {
         matMap newMatMap;
         newMatMap.slot = slot;
         newMatMap.matAsset = matAsset;
         newMatMap.assetId = newValue;

         mChangingMaterials.push_back(newMatMap);
      }

      setMaskBits(MaterialMask);
   }

   Parent::onDynamicModified(slotName, newValue);
}

void MeshComponent::changeMaterial(U32 slot, MaterialAsset* newMat)
{
   
   char fieldName[512];

   //update our respective field
   dSprintf(fieldName, 512, "materialSlot%d", slot);
   setDataField(fieldName, NULL, newMat->getAssetId());
}

bool MeshComponent::setMatInstField(U32 slot, const char* field, const char* value)
{
   TSMaterialList* pMatList = mInterfaceData->mShapeInstance->getMaterialList();
   pMatList->setTextureLookupPath(getShapeResource().getPath().getPath());

   MaterialParameters* params = pMatList->getMaterialInst(slot)->getMaterialParameters();

   if (pMatList->getMaterialInst(slot)->getFeatures().hasFeature(MFT_DiffuseColor))
   {
      MaterialParameterHandle* handle = pMatList->getMaterialInst(slot)->getMaterialParameterHandle("DiffuseColor");

      params->set(handle, LinearColorF(0, 0, 0));
   }

   return true;
}

void MeshComponent::onInspect()
{
}

void MeshComponent::onEndInspect()
{
}

void MeshComponent::ownerTransformSet(MatrixF *mat)
{
   if (mInterfaceData != nullptr)
   {
      MatrixF newTransform = *mat;
      mInterfaceData->mTransform = newTransform;
   }
}