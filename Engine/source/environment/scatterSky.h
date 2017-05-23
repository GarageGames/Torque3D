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

#ifndef _SCATTERSKY_H_
#define _SCATTERSKY_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _GFXSTATEBLOCK_H_
#include "gfx/gfxStateBlock.h"
#endif
#ifndef _RENDERPASSMANAGER_H_
#include "renderInstance/renderPassManager.h"
#endif
#ifndef _PRIMBUILDER_H_
#include "gfx/primBuilder.h"
#endif
#ifndef _LIGHTINFO_H_
#include "lighting/lightInfo.h"
#endif
#ifndef _LIGHTFLAREDATA_H_
#include "T3D/lightFlareData.h"
#endif
#ifndef _TRESPONSECURVE_H_
#include "math/util/tResponseCurve.h"
#endif

class LightInfo;
class SphereMesh;
class TimeOfDay;
class CubemapData;
class MatrixSet;

class ScatterSky : public SceneObject, public ISceneLight
{
   typedef SceneObject Parent;

public:

   enum MaskBits
   {
      UpdateMask = Parent::NextFreeMask,
      TimeMask = Parent::NextFreeMask << 1,
      NextFreeMask = Parent::NextFreeMask << 2
   };

   ScatterSky();
   ~ScatterSky();

   // SimObject
   bool onAdd();
   void onRemove();

   // ISceneLight
   virtual void submitLights( LightManager *lm, bool staticLighting );
   virtual LightInfo* getLight() { return mLight; }

   // ConsoleObject
   DECLARE_CONOBJECT(ScatterSky);
   void inspectPostApply();
   static void initPersistFields();

   // Network
   U32  packUpdate  ( NetConnection *conn, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *conn,           BitStream *stream );

   void prepRenderImage( SceneRenderState* state );
  
   ///
   void setAzimuth( F32 azimuth );
   ///
   void setElevation( F32 elevation );

   ///
   F32 getAzimuth() const { return mSunAzimuth; }
   ///
   F32 getElevation() const { return mSunElevation; }

protected:

   void _render( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );
   void _debugRender( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );
   void _renderMoon( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

   void _initVBIB();
   bool _initShader();
   void _initMoon();
   void _initCurves();

   F32 _getRayleighPhase( F32 fCos2 );
   F32 _getMiePhase( F32 fCos, F32 fCos2, F32 g, F32 g2 );
   F32 _vernierScale( F32 fCos );

   void _generateSkyPoints();

   void _getColor( const Point3F &pos, ColorF *outColor );
   void _getFogColor( ColorF *outColor );
   void _getAmbientColor( ColorF *outColor );
   void _getSunColor( ColorF *outColor );
   void _interpolateColors();

   void _conformLights();

   void _updateTimeOfDay( TimeOfDay *timeofDay, F32 time );

   // static protected field set methods
   static bool ptSetElevation( void *object, const char *index, const char *data );
   static bool ptSetAzimuth( void *object, const char *index, const char *data );

   // SimObject.
   virtual void _onSelected();
   virtual void _onUnselected();

protected:

   static const F32 smEarthRadius;
   static const F32 smAtmosphereRadius;
   static const F32 smViewerHeight;

#define CURVE_COUNT 5

   FloatCurve mCurves[CURVE_COUNT];

   U32 mVertCount;
   U32 mPrimCount;

   F32 mRayleighScattering;
   F32 mRayleighScattering4PI;
   F32 mSunSize;
   F32 mMieScattering;
   F32 mMieScattering4PI;

   F32 mSkyBrightness;
   F32 mMiePhaseAssymetry;

   F32 mOuterRadius;
   F32 mScale;
   ColorF mWavelength;
   F32 mWavelength4[3];
   F32 mRayleighScaleDepth;
   F32 mMieScaleDepth;

   F32 mSphereInnerRadius;
   F32 mSphereOuterRadius;

   F32 mExposure;
   F32 mNightInterpolant;
   F32 mZOffset;

   VectorF mLightDir;
   VectorF mSunDir;

   F32 mSunAzimuth;
   F32 mSunElevation;
   F32 mMoonAzimuth;
   F32 mMoonElevation;

   F32 mTimeOfDay;

   F32 mBrightness;

   ColorF mNightColor;
   ColorF mNightFogColor;

   ColorF mAmbientColor;   ///< Not a field
   ColorF mSunColor;       ///< Not a field
   ColorF mFogColor;       ///< Not a field

   ColorF mAmbientScale;
   ColorF mSunScale;
   ColorF mFogScale;

   LightInfo *mLight;

   bool mCastShadows;
   S32 mStaticRefreshFreq;
   S32 mDynamicRefreshFreq;
   bool mDirty;

   LightFlareData *mFlareData;
   LightFlareState mFlareState;
   F32 mFlareScale;

   bool mMoonEnabled;
   String mMoonMatName;
   BaseMatInstance *mMoonMatInst;
   F32 mMoonScale;
   ColorF mMoonTint;
   VectorF mMoonLightDir;
   CubemapData *mNightCubemap;
   String mNightCubemapName;
   bool mUseNightCubemap;
   MatrixSet *mMatrixSet;

   Vector<Point3F> mSkyPoints;

   // Prim buffer, vertex buffer and shader for rendering.
   GFXPrimitiveBufferHandle mPrimBuffer;
   GFXVertexBufferHandle<GFXVertexP> mVB;
   GFXShaderRef mShader;

   GFXStateBlockRef mStateBlock;

   // Shared shader constant blocks
   GFXShaderConstBufferRef mShaderConsts;
   GFXShaderConstHandle *mModelViewProjSC;
   GFXShaderConstHandle *mMiscSC;                     // Camera height, cam height squared, scale and scale over depth.
   GFXShaderConstHandle *mSphereRadiiSC;              // Inner and out radius, and inner and outer radius squared.
   GFXShaderConstHandle *mScatteringCoefficientsSC;   // Rayleigh sun brightness, mie sun brightness and 4 * PI * coefficients.
   GFXShaderConstHandle *mCamPosSC;
   GFXShaderConstHandle *mLightDirSC;
   GFXShaderConstHandle *mSunDirSC;
   GFXShaderConstHandle *mNightColorSC;
   GFXShaderConstHandle *mInverseWavelengthSC;
   GFXShaderConstHandle *mNightInterpolantAndExposureSC;
   GFXShaderConstHandle *mUseCubemapSC;
   F32 mColorizeAmt;
   ColorF mColorize;
   GFXShaderConstHandle *mColorizeSC;

};

#endif // _SCATTERSKY_H_
