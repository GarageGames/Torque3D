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

#ifndef _LIGHTSHADOWMAP_H_
#define _LIGHTSHADOWMAP_H_

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _GFXCUBEMAP_H_
#include "gfx/gfxCubemap.h"
#endif
#ifndef _GFXTARGET_H_
#include "gfx/gfxTarget.h"
#endif
#ifndef _LIGHTINFO_H_
#include "lighting/lightInfo.h"
#endif
#ifndef _MATHUTIL_FRUSTUM_H_
#include "math/util/frustum.h"
#endif
#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif
#ifndef _SHADOW_COMMON_H_
#include "lighting/shadowMap/shadowCommon.h"
#endif
#ifndef _GFXSHADER_H_
#include "gfx/gfxShader.h"
#endif

class ShadowMapManager;
class SceneManager;
class SceneRenderState;
class BaseMatInstance;
class MaterialParameters;
class SharedShadowMapObjects;
struct SceneData;
class GFXShaderConstBuffer;
class GFXShaderConstHandle;
class GFXShader;
class GFXOcclusionQuery;
class LightManager;
class RenderPassManager;


// Shader constant handle lookup
// This isn't broken up as much as it could be, we're mixing single light constants
// and pssm constants.
struct LightingShaderConstants
{
   bool mInit;

   GFXShaderRef mShader;

   GFXShaderConstHandle* mLightParamsSC;
   GFXShaderConstHandle* mLightSpotParamsSC;
   
   // NOTE: These are the shader constants used for doing 
   // lighting  during the forward pass.  Do not confuse 
   // these for the prepass lighting constants which are 
   // used from AdvancedLightBinManager.
   GFXShaderConstHandle *mLightPositionSC;
   GFXShaderConstHandle *mLightDiffuseSC;
   GFXShaderConstHandle *mLightAmbientSC;
   GFXShaderConstHandle *mLightInvRadiusSqSC;
   GFXShaderConstHandle *mLightSpotDirSC;
   GFXShaderConstHandle *mLightSpotAngleSC;
   GFXShaderConstHandle *mLightSpotFalloffSC;

   GFXShaderConstHandle* mShadowMapSC;
   GFXShaderConstHandle* mShadowMapSizeSC;

   GFXShaderConstHandle* mCookieMapSC;

   GFXShaderConstHandle* mRandomDirsConst;
   GFXShaderConstHandle* mShadowSoftnessConst;

   GFXShaderConstHandle* mWorldToLightProjSC;
   GFXShaderConstHandle* mViewToLightProjSC;

   GFXShaderConstHandle* mScaleXSC;
   GFXShaderConstHandle* mScaleYSC;
   GFXShaderConstHandle* mOffsetXSC;
   GFXShaderConstHandle* mOffsetYSC;
   GFXShaderConstHandle* mAtlasXOffsetSC;
   GFXShaderConstHandle* mAtlasYOffsetSC;
   GFXShaderConstHandle* mAtlasScaleSC;

   // fadeStartLength.x = Distance in eye space to start fading shadows
   // fadeStartLength.y = 1 / Length of fade
   GFXShaderConstHandle* mFadeStartLength;
   GFXShaderConstHandle* mFarPlaneScalePSSM;
   GFXShaderConstHandle* mOverDarkFactorPSSM;

   GFXShaderConstHandle* mTapRotationTexSC;

   LightingShaderConstants();
   ~LightingShaderConstants();

   void init(GFXShader* buffer);

   void _onShaderReload();
};

typedef Map<GFXShader*, LightingShaderConstants*> LightConstantMap;


/// This represents everything we need to render
/// the shadowmap for one light.
class LightShadowMap
{
public:

   const static GFXFormat ShadowMapFormat;

   /// Used to scale the shadow texture size for performance tweaking.
   static F32 smShadowTexScalar;

   /// Whether to render shadow frustums for lights that have debug
   /// rendering enabled.
   static bool smDebugRenderFrustums;

public:

   LightShadowMap( LightInfo *light );

   virtual ~LightShadowMap();

   void render(   RenderPassManager* renderPass,
                  const SceneRenderState *diffuseState );

   U32 getLastUpdate() const { return mLastUpdate; }

   //U32 getLastVisible() const { return mLastVisible; }

   bool isViewDependent() const { return mIsViewDependent; }

   bool wasOccluded() const { return mWasOccluded; }

   void preLightRender();

   void postLightRender();

   void updatePriority( const SceneRenderState *state, U32 currTimeMs );

   F32 getLastScreenSize() const { return mLastScreenSize; }

   F32 getLastPriority() const { return mLastPriority; }

   virtual bool hasShadowTex() const { return mShadowMapTex.isValid(); }

   virtual bool setTextureStage( U32 currTexFlag, LightingShaderConstants* lsc );

   LightInfo* getLightInfo() { return mLight; }

   virtual void setShaderParameters(GFXShaderConstBuffer* params, LightingShaderConstants* lsc) = 0;

   U32 getTexSize() const { return mTexSize; }

   /// Returns the best texture size based on the user
   /// texture size, the last light screen size, and 
   /// global shadow tweak parameters.
   U32 getBestTexSize( U32 scale = 1 ) const;

   const MatrixF& getWorldToLightProj() const { return mWorldToLightProj; }

   static GFXTextureObject* _getDepthTarget( U32 width, U32 height );

   virtual ShadowType getShadowType() const = 0;

   // Cleanup texture resources
   virtual void releaseTextures();

   ///
   GFXTextureObject* getTexture() const { return mShadowMapTex; }

   ///
   void setDebugTarget( const String &name );

   static void releaseAllTextures();

   /// Releases any shadow maps that have not been culled
   /// in a while and returns the count of the remaing 
   /// shadow maps in use.
   static U32 releaseUnusedTextures();

   ///
   static S32 QSORT_CALLBACK cmpPriority( LightShadowMap *const *lsm1, LightShadowMap *const *lsm2 );

   /// Returns the correct shadow material this type of light
   /// or NULL if no shadow material is possible.
   BaseMatInstance* getShadowMaterial( BaseMatInstance *inMat ) const;

protected:

   /// All the shadow maps in the system.
   static Vector<LightShadowMap*> smShadowMaps;

   /// All the shadow maps that have been recently rendered to.
   static Vector<LightShadowMap*> smUsedShadowMaps;
   
   virtual void _render(   RenderPassManager* renderPass,
                           const SceneRenderState *diffuseState ) = 0;

   /// If there is a LightDebugInfo attached to the light that owns this map,
   /// then update its information from the given render state.
   ///
   /// @note This method only does something in debug builds.
   void _debugRender( SceneRenderState* shadowRenderState );

   /// Helper for rendering shadow map for debugging.
   NamedTexTarget mDebugTarget;

   /// If true the shadow is view dependent and cannot
   /// be skipped if visible and within active range.
   bool mIsViewDependent;

   /// The time this shadow was last updated.
   U32 mLastUpdate;

   /// The time this shadow was last culled and prioritized.
   U32 mLastCull;

   /// The shadow occlusion query used when the light is
   /// rendered to determine if any pixel of it is visible.
   GFXOcclusionQuery *mVizQuery;

   /// If true the light was occluded by geometry the
   /// last frame it was updated.
   //the last frame.
   bool mWasOccluded;

   F32 mLastScreenSize;

   F32 mLastPriority;

   MatrixF mWorldToLightProj;

   GFXTextureTargetRef mTarget;
   U32 mTexSize;
   GFXTexHandle mShadowMapTex;

   // The light we are rendering.
   LightInfo *mLight;   

   // Used for blur
   GFXShader* mLastShader;
   GFXShaderConstHandle* mBlurBoundaries;

   // Calculate view matrices and set proper projection with GFX
   void calcLightMatrices( MatrixF& outLightMatrix, const Frustum &viewFrustum );

   /// The callback used to get texture events.
   /// @see GFXTextureManager::addEventDelegate
   void _onTextureEvent( GFXTexCallbackCode code );  
};

GFX_DeclareTextureProfile( ShadowMapProfile );
GFX_DeclareTextureProfile( ShadowMapZProfile );


class ShadowMapParams : public LightInfoEx
{
public:

   ShadowMapParams( LightInfo *light );
   virtual ~ShadowMapParams();

   /// The LightInfoEx hook type.
   static LightInfoExType Type;

   // LightInfoEx
   virtual void set( const LightInfoEx *ex );
   virtual const LightInfoExType& getType() const { return Type; }
   virtual void packUpdate( BitStream *stream ) const;
   virtual void unpackUpdate( BitStream *stream );

   LightShadowMap* getShadowMap() const { return mShadowMap; }

   LightShadowMap* getOrCreateShadowMap();

   bool hasCookieTex() const { return cookie.isNotEmpty(); }

   GFXTextureObject* getCookieTex();

   GFXCubemap* getCookieCubeTex();

   // Validates the parameters after a field is changed.
   void _validate();

protected:

   void _initShadowMap();

   ///
   LightShadowMap *mShadowMap;

   LightInfo *mLight;

   GFXTexHandle mCookieTex;

   GFXCubemapHandle mCookieCubeTex;

public:

   // We're leaving these public for easy access 
   // for console protected fields.

   /// @name Shadow Map
   /// @{
   
   ///
   U32 texSize;

   /// 
   FileName cookie;

   /// @}

   Point3F attenuationRatio;

   /// @name Point Lights
   /// @{

   ///
   ShadowType shadowType;

   /// @}

   /// @name Exponential Shadow Map Parameters
   /// @{
   Point4F overDarkFactor;
   /// @}   

   /// @name Parallel Split Shadow Map
   /// @{

   ///
   F32 shadowDistance;  

   ///
   F32 shadowSoftness;

   /// The number of splits in the shadow map.
   U32 numSplits;

   /// 
   F32 logWeight;

   /// At what distance do we start fading the shadows out completely.
   F32 fadeStartDist;

   /// This toggles only terrain being visible in the last
   /// split of a PSSM shadow map.
   bool lastSplitTerrainOnly;

   /// @}
};

#endif // _LIGHTSHADOWMAP_H_
