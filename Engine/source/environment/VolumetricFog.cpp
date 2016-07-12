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

#include "environment/VolumetricFog.h"
#include "windowManager/platformWindowMgr.h"
#include "gfx/gfxTransformSaver.h"
#include "renderInstance/renderPassManager.h"
#include "math/mathIO.h"
#include "materials/shaderData.h"
#include "math/util/matrixSet.h"
#include "core/resourceManager.h"
#include "core/stream/bitStream.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/shapeBase.h"
#include "ts/tsShapeInstance.h"
#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"
#include "VolumetricFogRTManager.h"
#include "lighting/lightInfo.h"
#include "lighting/lightManager.h"

#define COLBOX_SCALE Point3F(1.02f, 1.02f, 1.02f)

IMPLEMENT_CO_NETOBJECT_V1(VolumetricFog);

ConsoleDocClass(VolumetricFog,
"@brief Volumetric Fog Object class. Main class defining the Volumetric\n"
"Fog objects in the scene. Used in conjunction with the VolumetricFogRTManager\n"
"class which is responsible for the required rendertargets.\n\n"
"Methods (exposed to script):\n"
" setFogColorF(color) Changes the overall fog color (color.rgba range 0.0 - 1.0).\n;"
" setFogColor(color) Changes the overall fog color color.rgba range 0 - 255).\n;"
" setFogDensity(density) Changes the overall fog density.\n"
" setFogModulation(strength, speed1, speed2) changes the strength\n"
" and the speeds of the 2 animation layers.\n\n"
"Callbacks:\n"
"onEnterFog triggered whenever the controlobject (Player or Camera) enters the Fog.\n"
" (current Fog object and the controlobject are exposed to script.\n"
"onLeaveFog triggered whenever the controlobject (Player or Camera) left the Fog.\n"
" (current Fog object and the controlobject are exposed to script.\n\n"
"@tsexample\n"
" new VolumetricFog()\n"
" {\n"
" shapeName = \"art/environment/FogRCube.dts\";\n"
" fogColor = \"200 200 200 128\";\n"
" fogDensity = \"0.2\";\n"
" ignoreWater = \"0\";\n"
" MinSize = \"250\";\n"
" FadeSize = \"750\";\n"
" texture = \"art/environment/FogMod_heavy.dds\";\n"
" tiles = \"1.5\";\n"
" modStrength = \"0.2\";\n"
" PrimSpeed = \"-0.01 0.04\";\n"
" SecSpeed = \"0.02 -0.02\";\n"
" position = \"748.644 656.371 65.3506\"; \n"
" rotation = \"0 0 1 20.354\";\n"
" scale = \"40 30 6\";\n"
" };\n"
"@endtsexample\n"
);

IMPLEMENT_CALLBACK(VolumetricFog, onEnterFog, void, (SimObjectId obj), (obj),
"@brief Called when an object enters the volume of the Fog instance.\n\n"

"@param obj the controlobject entering the fog.");

IMPLEMENT_CALLBACK(VolumetricFog, onLeaveFog, void, (SimObjectId obj), (obj),
"@brief Called when an object left the volume of the Fog instance.\n\n"

"@param obj the controlobject leaving the fog.");


VolumetricFog::VolumetricFog()
{
   AssertFatal(VFRTM != NULL, "VolumetricFog Fatal Error: No Manager found");

   if (!VFRTM->IsInitialized())
   VFRTM->Init();

   mNetFlags.set(Ghostable | ScopeAlways);

   mTypeMask |= EnvironmentObjectType | StaticObjectType;

   mPrepassTarget = NULL;
   mDepthBufferTarget = NULL;
   mFrontBufferTarget = NULL;

   z_buf = NULL;
   mTexture = NULL;

   mIsVBDirty = false;
   mIsPBDirty = false;

   mFogColor.set(200, 200, 200, 255);
   mFogDensity = 0.3f;
   mIgnoreWater = false;
   mReflect = false;
   mCamInFog = false;
   mResizing = false;
   mFogReflStrength = 20.0;
   mUseGlow = false;
   mGlowStrength = 0.3f;
   mGlowing = 0;
   mModifLightRays = false;
   mLightRayMod = 1.0f;
   mOldLightRayStrength = 0.1f;

   mShapeName = "";
   mShapeLoaded = false;
   mMinDisplaySize = 10.0f;
   mFadeSize = 0.0f;
   mCurDetailLevel = 0;
   mNumDetailLevels = 0;
   det_size.clear();

   mTextureName = "";
   mIsTextured = false;
   mStrength = 0.5f;
   mTexTiles = 1.0f;
   mSpeed1.set(0.5f, 0.0f);
   mSpeed2.set(0.1f, 0.1f);
}

VolumetricFog::~VolumetricFog()
{
   if (isClientObject())
      return;

   for (S32 i = 0; i < det_size.size(); i++)
   {
      if (det_size[i].indices != NULL)
         delete(det_size[i].indices);
      if (det_size[i].piArray != NULL)
         delete(det_size[i].piArray);
      if (det_size[i].verts != NULL)
         delete(det_size[i].verts);
   }
   det_size.clear();

   if (z_buf.isValid())
      SAFE_DELETE(z_buf);

   if (!mTexture.isNull())
      mTexture.free();
}

void VolumetricFog::initPersistFields()
{
   addGroup("VolumetricFogData");
   addField("shapeName", TypeShapeFilename, Offset(mShapeName, VolumetricFog),
      "Path and filename of the model file (.DTS, .DAE) to use for this Volume.");
   addField("FogColor", TypeColorI, Offset(mFogColor, VolumetricFog),
      "Fog color RGBA (Alpha is ignored)");
   addField("FogDensity", TypeF32, Offset(mFogDensity, VolumetricFog), 
      "Overal fog density value (0 disables the fog).");
   addField("IgnoreWater", TypeBool, Offset(mIgnoreWater, VolumetricFog), 
      "Set to true if volumetric fog should continue while submerged.");
   addField("MinSize", TypeF32, Offset(mMinDisplaySize, VolumetricFog), 
      "Min size (in pixels) for fog to be rendered.");
   addField("FadeSize", TypeF32, Offset(mFadeSize, VolumetricFog), 
      "Object size in pixels at which the FX-fading kicks in (0 disables fading).");
   endGroup("VolumetricFogData");

   addGroup("VolumetricFogModulation");
   addField("texture", TypeImageFilename, Offset(mTextureName, VolumetricFog),
      "A texture which contains Fogdensity modulator in the red channel and color with 1-green channel. No texture disables modulation.");
   addField("tiles", TypeF32, Offset(mTexTiles, VolumetricFog), 
      "How many times the texture is mapped to the object.");
   addField("modStrength", TypeF32, Offset(mStrength, VolumetricFog),
      "Overall strength of the density modulation (0 disables modulation).");
   addField("PrimSpeed", TypePoint2F, Offset(mSpeed1, VolumetricFog),
      "Overall primary speed of the density modulation (x-speed(u) y-speed(v))");
   addField("SecSpeed", TypePoint2F, Offset(mSpeed2, VolumetricFog),
      "Overall secundary speed of the density modulation (x-speed(u) y-speed(v))");
   endGroup("VolumetricFogModulation");

   addGroup("Reflections");
   addField("Reflectable", TypeBool, Offset(mReflect, VolumetricFog), 
      "Set to true if volumetric fog should be reflected.");
   addField("ReflectStrength", TypeF32, Offset(mFogReflStrength, VolumetricFog), 
      "Strength of the reflections (0 disables the fog).");
   endGroup("Reflections");

   addGroup("PostFX");
   addField("useGlow", TypeBool, Offset(mUseGlow, VolumetricFog), 
      "Set to true if volumetric fog should use glow PostFX.");
   addField("glowStrength", TypeF32, Offset(mGlowStrength, VolumetricFog),
      "Overall strength of the glow PostFX.");
   addField("modLightRay", TypeBool, Offset(mModifLightRays, VolumetricFog), 
      "Set to true if volumetric fog should modify the brightness of the Lightrays.");
   addField("lightRayMod", TypeF32, Offset(mLightRayMod, VolumetricFog),
      "Modifier for LightRay PostFX when inside Fog.");
   endGroup("PostFX");
   Parent::initPersistFields();
}

void VolumetricFog::inspectPostApply()
{
   Parent::inspectPostApply();
   mSpeed.set(mSpeed1.x, mSpeed1.y, mSpeed2.x, mSpeed2.y);
   setMaskBits(VolumetricFogMask | FogColorMask | FogDensityMask | FogModulationMask | FogPostFXMask | FogShapeMask);
}

bool VolumetricFog::onAdd()
{
   if (!Parent::onAdd())
      return false;

   if (!VFRTM->IsInitialized())
   {
      Con::errorf("No VolumetricFogRTManager present!!");
      return false;
   }

   resetWorldBox();

   mShapeLoaded = LoadShape();

   setRenderTransform(mObjToWorld);

   addToScene();
   ColBox.set(getTransform(), (mObjBox.getExtents() * getScale() * COLBOX_SCALE));
   mObjSize = mWorldBox.getGreatestDiagonalLength();
   mObjScale = getScale();
   mTexTiles = mAbs(mTexTiles);
   mSpeed.set(mSpeed1.x, mSpeed1.y, mSpeed2.x, mSpeed2.y);
   mInvScale = (1.0f / getMax(getMax(mObjScale.x, mObjScale.y), mObjScale.z));

   if (isClientObject())
   {
      conn = GameConnection::getConnectionToServer();
      if (!conn)
      {  
         Con::errorf("VolumetricFog::onAdd - No Serverconnection");
         return false;
      }

      glowFX = static_cast<PostEffect*>(Sim::findObject("VolFogGlowPostFx"));

      mOldLightRayStrength = Con::getFloatVariable("$LightRayPostFX::brightScalar",1.0f);

      GuiCanvas* cv = dynamic_cast<GuiCanvas*>(Sim::findObject("Canvas"));
      if (cv == NULL)
      {
         Con::errorf("VolumetricFog::onAdd - Canvas not found!!");
         return false;
      }
      mPlatformWindow = cv->getPlatformWindow();
      VolumetricFogRTManager::getVolumetricFogRTMResizeSignal().notify(this, &VolumetricFog::handleResize);
      GuiCanvas::getCanvasSizeChangeSignal().notify(this, &VolumetricFog::handleCanvasResize);

      InitTexture();
      return setupRenderer();
   }

   VFRTM->IncFogObjects();
   
   return true;
}

void VolumetricFog::onRemove()
{
   if (isClientObject())
   {
      if (isTicking())
      {
         setProcessTick(false);
         if (mGlowing != 0)
         {
            mGlowing = 0;
            glowFX->disable();
         }
         _leaveFog(static_cast<ShapeBase*>(conn->getControlObject()));
      }
      VolumetricFogRTManager::getVolumetricFogRTMResizeSignal().remove(this, &VolumetricFog::handleResize);
      GuiCanvas::getCanvasSizeChangeSignal().remove(this, &VolumetricFog::handleCanvasResize);
   }
   removeFromScene();
   VFRTM->DecFogObjects();
   Parent::onRemove();
}
void VolumetricFog::handleCanvasResize(GuiCanvas* canvas)
{
   UpdateBuffers(0,true);
}

void VolumetricFog::handleResize(VolumetricFogRTManager *RTM, bool resize)
{
   if (resize)
   {
      mResizing = true;
      RTM->FogAnswered();
   }
   else
      mResizing = false;

   if (mIsTextured)
   {
      F32 width = (F32)mPlatformWindow->getClientExtent().x;
      F32 height = (F32)mPlatformWindow->getClientExtent().y;

      mTexScale.x = 2.0f - ((F32)mTexture.getWidth() / width);
      mTexScale.y = 2.0f - ((F32)mTexture.getHeight() / height);
   }

   UpdateBuffers(0,true);
}

//-----------------------------------------------------------------------------
// Loadshape extracted from TSMesh and TSShapeInstance
//-----------------------------------------------------------------------------

bool VolumetricFog::LoadShape()
{
   GFXPrimitiveType GFXdrawTypes[] = { GFXTriangleList, GFXTriangleStrip };
   if (!mShapeName || mShapeName[0] == '\0')
   {
      Con::errorf("VolumetricFog::LoadShape() - No shape name! Volumetric Fog will not be rendered!");
      return false;
   }

   // Load shape, server side only reads bounds and radius

   Resource<TSShape> mShape;
   mShape = ResourceManager::get().load(mShapeName);
   if (bool(mShape) == false)
   {
      Con::errorf("VolumetricFog::LoadShape() - Unable to load shape: %s", mShapeName);
      return false;
   }

   mObjBox = mShape->bounds;
   mRadius = mShape->radius;
   resetWorldBox();

   if (!isClientObject())
      return false;

   TSShapeInstance *mShapeInstance = new TSShapeInstance(mShape, false);
   meshes mesh_detail;

   for (S32 i = 0; i < det_size.size(); i++)
   {
      if (det_size[i].indices != NULL)
         delete(det_size[i].indices);
      if (det_size[i].piArray != NULL)
         delete(det_size[i].piArray);
      if (det_size[i].verts != NULL)
         delete(det_size[i].verts);
   }
   det_size.clear();

   // browsing model for detail levels

   for (U32 i = 0; i < mShape->details.size(); i++)
   {
      const TSDetail *detail = &mShape->details[i];
      mesh_detail.det_size = detail->size;
      mesh_detail.sub_shape = detail->subShapeNum;
      mesh_detail.obj_det = detail->objectDetailNum;
      mesh_detail.verts = NULL;
      mesh_detail.piArray = NULL;
      mesh_detail.indices = NULL;
      if (detail->size >= 0.0f && detail->subShapeNum >= 0)
         det_size.push_back(mesh_detail);
   }

   for (U32 i = 0; i < det_size.size(); i++)
   {
      const S32 ss = det_size[i].sub_shape;
      if (ss >= 0)
      {
         const S32 start = mShape->subShapeFirstObject[ss];
         const S32 end = start + mShape->subShapeNumObjects[ss];
         for (S32 j = start; j < end; j++)
         {
            // Loading shape, only the first mesh for each detail will be used!
            TSShapeInstance::MeshObjectInstance *meshObj = &mShapeInstance->mMeshObjects[j];
            if (!meshObj)
               continue;
            TSMesh *mesh = meshObj->getMesh(det_size[i].obj_det);
            if (mesh != NULL)
            {
               const U32 numNrms = mesh->mNumVerts;
               GFXVertexPNTT *tmpVerts = NULL;
               tmpVerts = new GFXVertexPNTT[numNrms];
               mIsVBDirty = true;
               for (U32 k = 0; k < numNrms; k++)
                  {
                     Point3F norm = mesh->mVertexData[k].normal();
                     Point3F vert = mesh->mVertexData[k].vert();
                     Point2F uv = mesh->mVertexData[k].tvert();
                     tmpVerts[k].point = vert;
                     tmpVerts[k].texCoord = uv;
                     tmpVerts[k].normal = norm;
                  }
               det_size[i].verts = tmpVerts;
               det_size[i].num_verts = numNrms;

               det_size[i].piArray = new Vector<GFXPrimitive>();
               GFXPrimitive pInfo;

               det_size[i].indices = new Vector<U32>();

               for (U32 k = 0; k < mesh->indices.size(); k++)
                  det_size[i].indices->push_back(mesh->indices[k]);

               U32 primitivesSize = mesh->primitives.size();
               for (U32 k = 0; k < primitivesSize; k++)
               {
                  const TSDrawPrimitive & draw = mesh->primitives[k];
                  GFXPrimitiveType drawType = GFXdrawTypes[draw.matIndex >> 30];
                  switch (drawType)
                  {
                     case GFXTriangleList:
                        pInfo.type = drawType;
                        pInfo.numPrimitives = draw.numElements / 3;
                        pInfo.startIndex = draw.start;
                        // Use the first index to determine which 16-bit address space we are operating in
                        pInfo.startVertex = mesh->indices[draw.start] & 0xFFFF0000;
                        pInfo.minIndex = pInfo.startVertex;
                        pInfo.numVertices = getMin((U32)0x10000, mesh->mNumVerts - pInfo.startVertex);
                        break;
                     case GFXTriangleStrip:
                        pInfo.type = drawType;
                        pInfo.numPrimitives = draw.numElements - 2;
                        pInfo.startIndex = draw.start;
                        // Use the first index to determine which 16-bit address space we are operating in
                        pInfo.startVertex = mesh->indices[draw.start] & 0xFFFF0000;
                        pInfo.minIndex = pInfo.startVertex;
                        pInfo.numVertices = getMin((U32)0x10000, mesh->mNumVerts - pInfo.startVertex);
                        break;
                     default:
                        Con::errorf("VolumetricFog::LoadShape Unknown drawtype!?!");
                        return false;
                        break;
                  }
               det_size[i].piArray->push_back(pInfo);
               j = end;
            }
         }
         else
         {
            Con::errorf("VolumetricFog::LoadShape Error loading mesh from shape!");
            delete mShapeInstance;
            return false;
         }
         mIsVBDirty = true;
         mIsPBDirty = true;
         }
      }
   }

   mNumDetailLevels = det_size.size();
   mCurDetailLevel = 0;
   UpdateBuffers(mCurDetailLevel);
   delete mShapeInstance;

   return true;
}

//-----------------------------------------------------------------------------
// UpdateBuffers called whenever detaillevel changes (LOD)
//-----------------------------------------------------------------------------


bool VolumetricFog::UpdateBuffers(U32 dl, bool force)
{
   if (mVB.isNull() || mIsVBDirty || dl != mCurDetailLevel || force)
   {
      mVB.set(GFX, det_size[dl].num_verts, GFXBufferTypeDynamic);
      mIsVBDirty = false;
   }
   GFXVertexPNTT *vertPtr = mVB.lock();
   if (!vertPtr)
   {
      mVB.unlock();
      return false;
   }
   dMemcpy(vertPtr, det_size[dl].verts, sizeof (GFXVertexPNTT)* det_size[dl].num_verts);
   mVB.unlock();

   if (mIsPBDirty || mPB.isNull() || dl != mCurDetailLevel || force)
   {
      #ifdef TORQUE_DEBUG
      mPB.set(GFX, det_size[dl].indices->size(), det_size[dl].piArray->size(), GFXBufferTypeDynamic, avar("%s() - VolFogPrimBuffer (line %d)", __FUNCTION__, __LINE__));
      #else
      mPB.set(GFX, det_size[dl].indices->size(), det_size[dl].piArray->size(), GFXBufferTypeDynamic);
      #endif
      U16 *ibIndices = NULL;
      GFXPrimitive *piInput = NULL;
      mPB.lock(&ibIndices, &piInput);
      dCopyArray(ibIndices, det_size[dl].indices->address(), det_size[dl].indices->size());
      dMemcpy(piInput, det_size[dl].piArray->address(), det_size[dl].piArray->size() * sizeof(GFXPrimitive));
      mPB.unlock();
      mIsPBDirty = false;
   }
   mCurDetailLevel = dl;
   return true;
}

U32 VolumetricFog::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);
   if (stream->writeFlag(mask & FogColorMask))
      stream->write(mFogColor);
   if (stream->writeFlag(mask & FogDensityMask))
      stream->write(mFogDensity);
   if (stream->writeFlag(mask & FogModulationMask))
   {
      stream->write(mTextureName);
      mTexTiles = mFabs(mTexTiles);
      stream->write(mTexTiles);
      stream->write(mStrength);
      mathWrite(*stream, mSpeed);
   }
   if (stream->writeFlag(mask & FogPostFXMask))
   {
      stream->writeFlag(mUseGlow);
      stream->write(mGlowStrength);
      stream->writeFlag(mModifLightRays);
      stream->write(mLightRayMod);
   }
   if (stream->writeFlag(mask & VolumetricFogMask))
   {
      stream->writeFlag(mIgnoreWater);
      stream->writeFlag(mReflect);
      stream->write(mFogReflStrength);
      stream->writeFlag(mResizing);
      stream->write(mMinDisplaySize);
      stream->write(mFadeSize);
   }
   if (stream->writeFlag(mask & FogShapeMask))
   {
      stream->writeString(mShapeName);
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
      if (!mShapeName || mShapeName[0] == '\0')
         return retMask;
      Resource<TSShape> mShape;
      mShape = ResourceManager::get().load(mShapeName);
      if (bool(mShape) == false)
         return retMask;
      mObjBox = mShape->bounds;
      mRadius = mShape->radius;
      resetWorldBox();
      mObjSize = mWorldBox.getGreatestDiagonalLength();
      mObjScale = getScale();
      mInvScale = (1.0f / getMax(getMax(mObjScale.x, mObjScale.y), mObjScale.z));
   }
   return retMask;
}

void VolumetricFog::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);
   MatrixF mat;
   VectorF scale;
   VectorF mOldScale = getScale();
   String oldTextureName = mTextureName;
   StringTableEntry oldShape = mShapeName;

   if (stream->readFlag())// Fog color
      stream->read(&mFogColor);
   if (stream->readFlag())// Fog Density
   {
      stream->read(&mFogDensity);
      if (isTicking())
      {
         char buf[20];
         dSprintf(buf, sizeof(buf), "%3.7f", mFogDensity);
         Con::setVariable("$VolumetricFog::density", buf);
      }
   }
   if (stream->readFlag())// Fog Modulation
   {
      stream->read(&mTextureName);
      stream->read(&mTexTiles);
      mTexTiles = mFabs(mTexTiles);
      stream->read(&mStrength);
      mathRead(*stream, &mSpeed);
      mSpeed1.set(mSpeed.x, mSpeed.y);
      mSpeed2.set(mSpeed.z, mSpeed.w);

      if (isProperlyAdded())
      {
         if (oldTextureName != mTextureName)
            InitTexture();
         if (oldTextureName.isNotEmpty() && mTextureName.isEmpty())
         {
            mIsTextured = false;
            mTexture.free();
         }
      }
   }
   if (stream->readFlag())//Fog PostFX
   {
      mUseGlow = stream->readFlag();
      stream->read(&mGlowStrength);
      mModifLightRays = stream->readFlag();
      stream->read(&mLightRayMod);
      if (isTicking())
      {
         char buf[20];
         dSprintf(buf, sizeof(buf), "%3.7f", mGlowStrength);
         Con::setVariable("$VolFogGlowPostFx::glowStrength", buf);
         if (mUseGlow && !glowFX->isEnabled())
            glowFX->enable();
         if (!mUseGlow && glowFX->isEnabled())
            glowFX->disable();
         if (mModifLightRays)
         {
            char buf[20];
            dSprintf(buf, sizeof(buf), "%3.7f", mOldLightRayStrength * mLightRayMod);
            Con::setVariable("$LightRayPostFX::brightScalar", buf);
         }
         if (!mModifLightRays)
         {
            char buf[20];
            dSprintf(buf, sizeof(buf), "%3.7f", mOldLightRayStrength);
            Con::setVariable("$LightRayPostFX::brightScalar", buf);
         }
      }
   }
   if (stream->readFlag())//Volumetric Fog
   {
      mIgnoreWater = stream->readFlag();
      mReflect = stream->readFlag();
      stream->read(&mFogReflStrength);
      mResizing = stream->readFlag();
      stream->read(&mMinDisplaySize);
      stream->read(&mFadeSize);
   }
   if (stream->readFlag())//Fog shape
   {
      mShapeName = stream->readSTString();
      mathRead(*stream, &mat);
      mathRead(*stream, &scale);
      if (strcmp(oldShape, mShapeName) != 0)
      {
         mIsVBDirty = true;
         mShapeLoaded = LoadShape();
      }
      setScale(scale);
      setTransform(mat);
      ColBox.set(getTransform(), (mObjBox.getExtents() * getScale() * COLBOX_SCALE));
      mObjSize = mWorldBox.getGreatestDiagonalLength();
      mObjScale = getScale();
      mInvScale = (1.0f / getMax(getMax(mObjScale.x, mObjScale.y), mObjScale.z));
   }
}

void VolumetricFog::processTick(const Move* move)
{
   Parent::processTick(move);
   mCounter++;
   if ( mGlowing==1 && mCurGlow < mGlowStrength )
   {
      mCurGlow += (mGlowStrength / 10.0);
      char buf[20];
      dSprintf(buf, sizeof(buf), "%3.7f", mCurGlow);
      Con::setVariable("$VolFogGlowPostFx::glowStrength", buf);
   }
   else if ( mGlowing == 2 && mCurGlow > 0.0f )
   {
      mCurGlow -= (mGlowStrength / 5.0f);
      if (mCurGlow <= 0.0f)
      {
         glowFX->disable();
         mGlowing = 0;
         setProcessTick(false);
         return;
      }
      else
      {
         char buf[20];
         dSprintf(buf, sizeof(buf), "%3.7f", mCurGlow);
         Con::setVariable("$VolFogGlowPostFx::glowStrength", buf);
      }
   }
   if (mCounter == 3)
   {
      ShapeBase* control = static_cast<ShapeBase*>(conn->getControlObject());
      MatrixF xfm;
      control->getRenderEyeTransform(&xfm);
      Point3F pos = xfm.getPosition();
      if (!ColBox.isContained(pos))
         _leaveFog(control);
      mCounter = 0;
   }
}

void VolumetricFog::_enterFog(ShapeBase *control)
{
   if (mUseGlow)
   {
      if (glowFX)
      {
         mCurGlow = 0.0f;
         Con::setVariable("$VolFogGlowPostFx::glowStrength", "0.0");
         glowFX->enable();
         mGlowing = 1;
      }
   }
   if (mModifLightRays)
   {
      char buf[20];
      dSprintf(buf, sizeof(buf), "%3.7f", mOldLightRayStrength * mLightRayMod);
      Con::setVariable("$LightRayPostFX::brightScalar", buf);
   }
   mCounter = 0;
   char buf[20];
   dSprintf(buf, sizeof(buf), "%3.7f", mFogDensity);
   Con::setVariable("$VolumetricFog::density", buf);
   setProcessTick(true);
   if (control)
      onEnterFog_callback(control->getId());
}

void VolumetricFog::_leaveFog(ShapeBase *control)
{
   mCamInFog = false;
   Con::setVariable("$VolumetricFog::density", "0.0");
   if (mModifLightRays)
   {
      char buf[20];
      dSprintf(buf, sizeof(buf), "%3.7f", mOldLightRayStrength);
      Con::setVariable("$LightRayPostFX::brightScalar", buf);
   }
   if (mUseGlow)
   {
      if (glowFX && mGlowing != 2)
      {
         mCurGlow = mGlowStrength;
         mGlowing = 2;
         if (control)
            onLeaveFog_callback(control->getId());
      }
   }
   else
   {
      setProcessTick(false);
      if (control)
         onLeaveFog_callback(control->getId());
   }
}

//-----------------------------------------------------------------------------
// Setting up the renderers
//-----------------------------------------------------------------------------

bool VolumetricFog::setupRenderer()
{
   // Search for the prepass rendertarget and shadermacros.
   mPrepassTarget = NamedTexTarget::find("prepass");
   if (!mPrepassTarget.isValid())
   {
      Con::errorf("VolumetricFog::setupRenderer - could not find PrepassTarget");
      return false;
   }

   Vector<GFXShaderMacro> macros;
   if (mPrepassTarget)
      mPrepassTarget->getShaderMacros(&macros);

   // Search the depth and frontbuffers which are created by the VolumetricFogRTManager

   mDepthBufferTarget = NamedTexTarget::find("volfogdepth");
   if (!mDepthBufferTarget.isValid())
   {
      Con::errorf("VolumetricFog::setupRenderer - could not find depthbuffer");
      return false;
   }

   mFrontBufferTarget = NamedTexTarget::find("volfogfront");
   if (!mFrontBufferTarget.isValid())
   {
      Con::errorf("VolumetricFog::setupRenderer - could not find frontbuffer");
      return false;
   }

   // Find and setup the prepass Shader

   ShaderData *shaderData;
   mPrePassShader = Sim::findObject("VolumetricFogPrePassShader", shaderData) ?
   shaderData->getShader() : NULL;
   if (!mPrePassShader)
   {
      Con::errorf("VolumetricFog::setupRenderer - could not find VolumetricFogPrePassShader");
      return false;
   }

   // Create ShaderConstBuffer and Handles

   mPPShaderConsts = mPrePassShader->allocConstBuffer();
   if (mPPShaderConsts.isNull())
   {
      Con::errorf("VolumetricFog::setupRenderer - could not allocate ShaderConstants 1.");
      return false;
   }

   mPPModelViewProjSC = mPrePassShader->getShaderConstHandle("$modelView");

   // Find and setup the VolumetricFog Shader

   shaderData = NULL;
   mShader = Sim::findObject("VolumetricFogShader", shaderData) ?
   shaderData->getShader(macros) : NULL;
   if (!mShader)
   {
      Con::errorf("VolumetricFog::setupRenderer - could not find VolumetricFogShader");
      return false;
   }

   // Create ShaderConstBuffer and Handles

   mShaderConsts = mShader->allocConstBuffer();
   if (mShaderConsts.isNull())
   {
      Con::errorf("VolumetricFog::setupRenderer - could not allocate ShaderConstants 2.");
      return false;
   }

   mModelViewProjSC = mShader->getShaderConstHandle("$modelView");
   mFadeSizeSC = mShader->getShaderConstHandle("$fadesize");
   mFogColorSC = mShader->getShaderConstHandle("$fogColor");
   mFogDensitySC = mShader->getShaderConstHandle("$fogDensity");
   mPreBias = mShader->getShaderConstHandle("$preBias");
   mAccumTime = mShader->getShaderConstHandle("$accumTime");
   mIsTexturedSC = mShader->getShaderConstHandle("$textured");
   mTexTilesSC = mShader->getShaderConstHandle("$numtiles");
   mModStrengthSC = mShader->getShaderConstHandle("$modstrength");
   mModSpeedSC = mShader->getShaderConstHandle("$modspeed");
   mViewPointSC = mShader->getShaderConstHandle("$viewpoint");
   mTexScaleSC = mShader->getShaderConstHandle("$texscale");
   mAmbientColorSC = mShader->getShaderConstHandle("$ambientColor");

   // Find and setup the reflection Shader

   shaderData = NULL;
   mReflectionShader = Sim::findObject("VolumetricFogReflectionShader", shaderData) ?
   shaderData->getShader() : NULL;
   if (!mReflectionShader)
   {
      Con::errorf("VolumetricFog::setupRenderer - could not find VolumetricFogReflectionShader");
      return false;
   }

   mReflShaderConsts = mReflectionShader->allocConstBuffer();
   if (mReflShaderConsts.isNull())
   {
      Con::errorf("VolumetricFog::setupRenderer - could not allocate ShaderConstants for VolumetricFogReflectionShader.");
      return false;
   }

   mReflModelViewProjSC = mReflectionShader->getShaderConstHandle("$modelView");
   mReflFogColorSC = mReflectionShader->getShaderConstHandle("$fogColor");
   mReflFogDensitySC = mReflectionShader->getShaderConstHandle("$fogDensity");
   mReflFogStrengthSC = mReflectionShader->getShaderConstHandle("$reflStrength");

   // Create the prepass StateBlock

   desc_preD.setCullMode(GFXCullCW);
   desc_preD.setBlend(true);
   desc_preD.setZReadWrite(false, false);
   desc_preD.stencilEnable = false;
   desc_preF.setCullMode(GFXCullCCW);
   desc_preF.setBlend(true);
   desc_preF.setZReadWrite(true, false);
   desc_preF.stencilEnable = false;
   
   // Create the VolumetricFog StateBlock

   descD.setCullMode(GFXCullCW);
   descD.setBlend(true);
   descD.setZReadWrite(false, false);// desc.setZReadWrite(true, false);

   // prepassBuffer sampler
   descD.samplersDefined = true;
   descD.samplers[0].addressModeU = GFXAddressClamp;
   descD.samplers[0].addressModeV = GFXAddressClamp;
   descD.samplers[0].addressModeW = GFXAddressClamp;
   descD.samplers[0].magFilter = GFXTextureFilterLinear;
   descD.samplers[0].minFilter = GFXTextureFilterLinear;
   descD.samplers[0].mipFilter = GFXTextureFilterLinear;
   descD.samplers[0].textureColorOp = GFXTOPDisable;

   // DepthBuffer sampler
   descD.samplers[1].addressModeU = GFXAddressClamp;
   descD.samplers[1].addressModeV = GFXAddressClamp;
   descD.samplers[1].addressModeW = GFXAddressClamp;
   descD.samplers[1].magFilter = GFXTextureFilterLinear;
   descD.samplers[1].minFilter = GFXTextureFilterLinear;
   descD.samplers[1].mipFilter = GFXTextureFilterLinear;
   descD.samplers[1].textureColorOp = GFXTOPModulate;

   // FrontBuffer sampler
   descD.samplers[2].addressModeU = GFXAddressClamp;
   descD.samplers[2].addressModeV = GFXAddressClamp;
   descD.samplers[2].addressModeW = GFXAddressClamp;
   descD.samplers[2].magFilter = GFXTextureFilterLinear;
   descD.samplers[2].minFilter = GFXTextureFilterLinear;
   descD.samplers[2].mipFilter = GFXTextureFilterLinear;
   descD.samplers[2].textureColorOp = GFXTOPModulate;

   // animated density modifier map sampler
   descD.samplers[3].addressModeU = GFXAddressWrap;
   descD.samplers[3].addressModeV = GFXAddressWrap;
   descD.samplers[3].addressModeW = GFXAddressWrap;
   descD.samplers[3].magFilter = GFXTextureFilterLinear;
   descD.samplers[3].minFilter = GFXTextureFilterLinear;
   descD.samplers[3].mipFilter = GFXTextureFilterLinear;
   descD.samplers[3].textureColorOp = GFXTOPModulate;

   dMemcpy(&descF, &descD, sizeof(GFXStateBlockDesc));
   descF.setCullMode(GFXCullCCW);
   descF.setBlend(true);
   descF.setZReadWrite(true, false);

   desc_refl.setCullMode(GFXCullCCW);
   desc_refl.setBlend(true);
   desc_refl.setZReadWrite(true, false);

   mStateblock_preD = GFX->createStateBlock(desc_preD);
   mStateblock_preF = GFX->createStateBlock(desc_preF);
   mStateblockD = GFX->createStateBlock(descD);
   mStateblockF = GFX->createStateBlock(descF);
   mStateblock_refl = GFX->createStateBlock(desc_refl);

   // Create Rendertarget

   z_buf = GFX->allocRenderToTextureTarget();
   if (z_buf == NULL)
   {
      Con::errorf("VolumetricFog::setupRenderer - Could not create Render Target");
      return false;
   }

   return true;
}

void VolumetricFog::prepRenderImage(SceneRenderState *state)
{
   if (!mShapeLoaded || mFogDensity <= 0.0f || mResizing)
      return;

   if (!state->isDiffusePass())
   {
      if (!state->isReflectPass())
      return;
   }
   
   PROFILE_SCOPE(VolumetricFog_prepRenderImage);

   // Time critical therefore static_cast
   ShapeBase* control = static_cast<ShapeBase*>(conn->getControlObject());
   if (control->getWaterCoverage() >= 0.9f && !mIgnoreWater)
      return;

   camPos = state->getCameraPosition();
   F32 dist = (camPos - getBoxCenter()).len();
   F32 scaleFactor = dist * mInvScale;
   if (scaleFactor <= 0.0f)
   {
      if (mCurDetailLevel != 0)
         UpdateBuffers(0);
   }
   const F32 pixelScale = state->getViewport().extent.y / 300.0f;

   mPixelSize = (mRadius / scaleFactor) * state->getWorldToScreenScale().y * pixelScale;
   if (mPixelSize < mMinDisplaySize)
      return;
   if (mNumDetailLevels > 1)
   {
      if ((det_size[mCurDetailLevel].det_size > mPixelSize) && (mCurDetailLevel < mNumDetailLevels - 1))
         UpdateBuffers(mCurDetailLevel + 1);
      else if (mCurDetailLevel > 0)
      {
         if (mPixelSize >= det_size[mCurDetailLevel - 1].det_size)
            UpdateBuffers(mCurDetailLevel - 1);
      }
   }

   if (state->isReflectPass() && mReflect)
   {
      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind(this, &VolumetricFog::reflect_render);
      ri->type = RenderPassManager::RIT_VolumetricFog;
      ri->translucentSort = true;
      ri->sortDistSq = getRenderWorldBox().getSqDistanceToPoint(camPos);
      if (dist < 1.0f)
         ri->defaultKey = 1;
      else
         ri->defaultKey = U32(dist);
      state->getRenderPass()->addInst(ri);
      return;
   }
   else if (state->isDiffusePass())
   {
      viewDist = state->getFarPlane();
      mFOV = state->getCameraFrustum().getFov() / M_PI_F;
      Point3F mEyeVec = state->getVectorEye() * viewDist;

      mViewPoint.x = ((mAtan2(mEyeVec.x, mEyeVec.y) / M_PI_F) + 1.0f) * mTexTiles;
      mViewPoint.y = (0.5f - (mAsin(mEyeVec.z) / M_PI_F)) * mTexTiles;

      bool isInside = ColBox.isContained(camPos);
      if (isInside && !mCamInFog)
      {
         mCamInFog = true;
         _enterFog(control);
      }
      else if (!isInside && mCamInFog)
         mCamInFog = false;

      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind(this, &VolumetricFog::render);
      ri->type = RenderPassManager::RIT_VolumetricFog;
      ri->translucentSort = true;
      ri->sortDistSq = getRenderWorldBox().getSqDistanceToPoint(camPos);
      if (dist < 1.0f)
         ri->defaultKey = 1;
      else
         ri->defaultKey = U32(dist);
      state->getRenderPass()->addInst(ri);
      return;
   }
   return;
}

void VolumetricFog::render(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat)
{
   if (overrideMat || !mShapeLoaded || !isClientObject() || mResizing)
      return;

   PROFILE_SCOPE(VolumetricFog_Render);

   GFXTransformSaver saver;
   GFX->setVertexBuffer(mVB);
   GFX->setPrimitiveBuffer(mPB);

   MatrixF mat = getRenderTransform();
   mat.scale(mObjScale);
   GFX->multWorld(mat);

   GFX->setShader(mPrePassShader);
   GFX->setShaderConstBuffer(mPPShaderConsts);
   GFX->setStateBlock(mStateblock_preD);

   // Set all the shader consts...

   MatrixF xform(GFX->getProjectionMatrix());
   xform *= GFX->getViewMatrix();
   xform *= GFX->getWorldMatrix();

   mPPShaderConsts->setSafe(mPPModelViewProjSC, xform);

   const ColorF &sunlight = state->getAmbientLightColor();

   Point3F ambientColor(sunlight.red, sunlight.green, sunlight.blue);
   mShaderConsts->setSafe(mAmbientColorSC, ambientColor);

   GFXTextureObject *mDepthBuffer = mDepthBufferTarget ? mDepthBufferTarget->getTexture(0) : NULL;
   GFXTextureObject *mFrontBuffer = mFrontBufferTarget ? mFrontBufferTarget->getTexture(0) : NULL;

   GFX->pushActiveRenderTarget();

   //render backside to target mDepthBuffer
   z_buf->attachTexture(GFXTextureTarget::DepthStencil, GFXTextureTarget::sDefaultDepthStencil);
   z_buf->attachTexture(GFXTextureTarget::Color0, mDepthBuffer);

   GFX->setActiveRenderTarget(z_buf);
   GFX->clear(GFXClearStencil | GFXClearTarget , ColorI(0,0,0,0), 1.0f, 0);

   GFX->drawPrimitive(0);
   z_buf->resolve();

   //render frontside to target mFrontBuffer
   z_buf->attachTexture(GFXTextureTarget::DepthStencil, GFXTextureTarget::sDefaultDepthStencil);
   z_buf->attachTexture(GFXTextureTarget::Color0, mFrontBuffer);
   GFX->clear(GFXClearStencil | GFXClearTarget, ColorI(0, 0, 0, 0), 1.0f, 0);

   GFX->setStateBlock(mStateblock_preF);

   GFX->drawPrimitive(0);
   z_buf->resolve();

   GFX->popActiveRenderTarget();
   z_buf->attachTexture(GFXTextureTarget::Color0, NULL);

   //render Volumetric Fog
   GFX->setShader(mShader);
   GFX->setShaderConstBuffer(mShaderConsts);

   mShaderConsts->setSafe(mModelViewProjSC, xform);
   if (mFadeSize > 0.0f)
      mShaderConsts->setSafe(mFadeSizeSC, mClampF(mPixelSize / mFadeSize, 0.0f, 1.0f));
   else
      mShaderConsts->setSafe(mFadeSizeSC, 1.0f);
   mShaderConsts->setSafe(mFogColorSC, mFogColor);
   mShaderConsts->setSafe(mFogDensitySC, mFogDensity);
   mShaderConsts->setSafe(mPreBias, viewDist);
   mShaderConsts->setSafe(mAccumTime, (F32)Sim::getCurrentTime() / 1000.0f);
   mShaderConsts->setSafe(mModStrengthSC, mStrength);
   mShaderConsts->setSafe(mModSpeedSC, mSpeed);
   mShaderConsts->setSafe(mViewPointSC, mViewPoint);
   mShaderConsts->setSafe(mTexScaleSC, mTexScale * mFOV);
   mShaderConsts->setSafe(mTexTilesSC, mTexTiles);

   GFXTextureObject *prepasstex = mPrepassTarget ? mPrepassTarget->getTexture(0) : NULL;

   GFX->setTexture(0, prepasstex);
   GFX->setTexture(1, mDepthBuffer);
   GFX->setTexture(2, mFrontBuffer);

   if (mIsTextured && mStrength > 0.0f)
   {
      GFX->setTexture(3, mTexture);
      mShaderConsts->setSafe(mIsTexturedSC, 1.0f);
   }
   else
      mShaderConsts->setSafe(mIsTexturedSC, 0.0f);

   if (mCamInFog)
   {
      /*GFXLockedRect *rect=mDepthBuffer->lock();
      U32 pixoffset = 0;// 1572864 + (512 * 4);
      U8 red = rect->bits[pixoffset];
      U8 green = rect->bits[pixoffset+1];
      U8 blue = rect->bits[pixoffset+2];
      U8 alpha = rect->bits[pixoffset+3];
      mDepthBuffer->unlock();
      S32 lval = ((alpha << 24) + (blue << 16) + (green << 8) + (red));
      F32 fval = ((F32)lval / S32_MAX);
      Con::printf("Color %d %d %d %d %d %f", red, green, blue, alpha, lval, fval);*/
      GFX->setStateBlock(mStateblockD);
   }
   else
      GFX->setStateBlock(mStateblockF);

   GFX->drawPrimitive(0);

   // Ensure these two textures are bound to the pixel shader input on the second run as they are used as pixel shader outputs (render targets).
   GFX->setTexture(1, NULL); //mDepthBuffer
   GFX->setTexture(2, NULL); //mFrontBuffer
   GFX->updateStates(); //update the dirty texture state we set above
}

void VolumetricFog::reflect_render(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat)
{
   if (overrideMat || !mShapeLoaded || !isClientObject() || mResizing || (mFogReflStrength==0.0f))
      return;

   GFXTransformSaver saver;
   GFX->setVertexBuffer(mVB);
   GFX->setPrimitiveBuffer(mPB);

   MatrixF mat = getRenderTransform();
   mat.scale(mObjScale);
   GFX->multWorld(mat);

   GFX->setShader(mReflectionShader);
   GFX->setShaderConstBuffer(mReflShaderConsts);
   GFX->setStateBlock(mStateblock_refl);

   // Set all the shader consts...
   MatrixF xform(GFX->getProjectionMatrix());
   xform *= GFX->getViewMatrix();
   xform *= GFX->getWorldMatrix();

   mReflShaderConsts->setSafe(mReflModelViewProjSC, xform);
   mReflShaderConsts->setSafe(mReflFogColorSC, mFogColor);
   mReflShaderConsts->setSafe(mReflFogDensitySC, mFogDensity);
   mReflShaderConsts->setSafe(mReflFogStrengthSC, mFogReflStrength);

   GFX->drawPrimitive(0);
}

//-----------------------------------------------------------------------------
// InitTexture is called whenever a modulation texture is added to the object
//-----------------------------------------------------------------------------

void VolumetricFog::InitTexture()
{
   mIsTextured = false;

   if (mTextureName.isNotEmpty())
      mTexture.set(mTextureName, &GFXDefaultStaticDiffuseProfile, "VolumetricFogMod");

   if (!mTexture.isNull())
   {
      mIsTextured = true;

      F32 width = (F32)mPlatformWindow->getClientExtent().x;
      F32 height = (F32)mPlatformWindow->getClientExtent().y;

      mTexScale.x = 2.0f - ((F32)mTexture.getWidth() / width);
      mTexScale.y = 2.0f - ((F32)mTexture.getHeight() / height);
   }
}

void VolumetricFog::setFogColor(ColorF color)
{
   mFogColor.set(255 * color.red,255 * color.green,255 * color.blue);
   setMaskBits(FogColorMask);
}

void VolumetricFog::setFogColor(ColorI color)
{
   mFogColor = color;
   setMaskBits(FogColorMask);
}

void VolumetricFog::setFogDensity(F32 density)
{
   if (density < 0.0f)
      density = 0.0f;
   mFogDensity = density;
   setMaskBits(FogDensityMask);
}

void VolumetricFog::setFogModulation(F32 strength,Point2F speed1,Point2F speed2)
{
   mStrength = strength;
   mSpeed1 = speed1;
   mSpeed2 = speed2;
   mSpeed.set(speed1.x, speed1.y, speed2.x, speed2.y);
   setMaskBits(FogModulationMask);
}

void VolumetricFog::setFogGlow(bool on_off, F32 strength)
{
   mUseGlow = on_off;
   mGlowStrength = strength;
   setMaskBits(FogPostFXMask);
}

void VolumetricFog::setFogLightray(bool on_off, F32 strength)
{
   mModifLightRays = on_off;
   mLightRayMod = strength;
   setMaskBits(FogPostFXMask);
}

bool VolumetricFog::isInsideFog()
{
   return mCamInFog;
}

DefineEngineMethod(VolumetricFog, SetFogColorF, void, (ColorF new_color), ,
"@brief Changes the color of the fog\n\n."
"@params new_color the new fog color (rgb 0.0 - 1.0, a is ignored.")
{
   object->setFogColor(new_color);
}

DefineEngineMethod(VolumetricFog, SetFogColor, void, (ColorI new_color), ,
"@brief Changes the color of the fog\n\n."
"@params new_color the new fog color (rgb 0-255, a is ignored.")
{
   object->setFogColor(new_color);
}

DefineEngineMethod(VolumetricFog, SetFogDensity, void, (F32 new_density), ,
"@brief Changes the density of the fog\n\n."
"@params new_density the new fog density.")
{
   object->setFogDensity(new_density);
}

DefineEngineMethod(VolumetricFog, SetFogModulation, void, (F32 new_strenght, Point2F new_speed1, Point2F new_speed2), ,
"@brief Changes the modulation of the fog\n\n."
"@params new_strenght the new strength of the modulation.\n"
"@params new_speed1 the new speed (x y) of the modulation layer 1.\n"
"@params new_speed2 the new speed (x y) of the modulation layer 2.\n")
{
   object->setFogModulation(new_strenght, new_speed1, new_speed2);
}

DefineEngineMethod(VolumetricFog, SetFogGlow, void, (bool on_off,F32 strength), ,
"@brief Changes the glow postfx when inside the fog\n\n."
"@params on_off set to true to enable glow.\n"
"@params strength glow strength.\n")
{
   object->setFogGlow(on_off, strength);
}

DefineEngineMethod(VolumetricFog, SetFogLightray, void, (bool on_off, F32 strength), ,
"@brief Changes the lightrays postfx when inside the fog\n\n."
"@params on_off set to true to modification of the lightray postfx.\n"
"@params strength lightray strength.\n")
{
   object->setFogLightray(on_off, strength);
}

DefineEngineMethod(VolumetricFog, isInsideFog, bool, (), ,
"@brief returns true if control object is inside the fog\n\n.")
{
   return object->isInsideFog();
}