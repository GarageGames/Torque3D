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

#ifndef _ADVANCEDLIGHTMANAGER_H_
#define _ADVANCEDLIGHTMANAGER_H_

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif 
#ifndef _LIGHTMANAGER_H_
#include "lighting/lightManager.h"
#endif 
#ifndef _LIGHTINFO_H_
#include "lighting/lightInfo.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _GFXTARGET_H_
#include "gfx/gfxTarget.h"
#endif
#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif
#ifndef _LIGHTSHADOWMAP_H_
#include "lighting/shadowMap/lightShadowMap.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif


class AvailableSLInterfaces;
class AdvancedLightBinManager;
class RenderPrePassMgr;
class BaseMatInstance;
class MaterialParameters;
class MaterialParameterHandle;
class GFXShader;
class GFXShaderConstHandle;
class ShadowMapManager;


class AdvancedLightManager : public LightManager
{
   typedef LightManager Parent;

public:

   /// Return the lightBinManager for this light manager.
   AdvancedLightBinManager* getLightBinManager() { return mLightBinManager; }

   // LightManager
   virtual bool isCompatible() const;
   virtual void activate( SceneManager *sceneManager );
   virtual void deactivate();
   virtual void registerGlobalLight(LightInfo *light, SimObject *obj);
   virtual void unregisterAllLights();
   virtual void setLightInfo( ProcessedMaterial *pmat, 
                              const Material *mat, 
                              const SceneData &sgData, 
                              const SceneRenderState *state,
                              U32 pass, 
                              GFXShaderConstBuffer *shaderConsts );
   virtual bool setTextureStage( const SceneData &sgData, 
                                 const U32 currTexFlag, 
                                 const U32 textureSlot, 
                                 GFXShaderConstBuffer *shaderConsts, 
                                 ShaderConstHandles *handles );

   typedef GFXVertexPC LightVertex;

   GFXVertexBufferHandle<LightVertex> getSphereMesh(U32 &outNumPrimitives, GFXPrimitiveBuffer *&outPrimitives );
   GFXVertexBufferHandle<LightVertex> getConeMesh(U32 &outNumPrimitives, GFXPrimitiveBuffer *&outPrimitives );

   LightShadowMap* findShadowMapForObject( SimObject *object );

protected:   

   // LightManager
   virtual void _addLightInfoEx( LightInfo *lightInfo );
   virtual void _initLightFields();

   /// A simple protected singleton.  Use LightManager::findByName()
   /// to access this light manager.
   /// @see LightManager::findByName()
   static AdvancedLightManager smSingleton;

   // These are protected because we're a singleton and
   // no one else should be creating us!
   AdvancedLightManager();
   virtual ~AdvancedLightManager();

   SimObjectPtr<AdvancedLightBinManager> mLightBinManager;

   SimObjectPtr<RenderPrePassMgr> mPrePassRenderBin;

   LightConstantMap mConstantLookup;

   GFXShaderRef mLastShader;

   LightingShaderConstants* mLastConstants;

   // Convex geometry for lights
   GFXVertexBufferHandle<LightVertex> mSphereGeometry;

   GFXPrimitiveBufferHandle mSphereIndices;

   U32 mSpherePrimitiveCount;

   GFXVertexBufferHandle<LightVertex> mConeGeometry;

   GFXPrimitiveBufferHandle mConeIndices;

   U32 mConePrimitiveCount;

   LightingShaderConstants* getLightingShaderConstants(GFXShaderConstBuffer* shader);
   
};

#endif // _ADVANCEDLIGHTMANAGER_H_
