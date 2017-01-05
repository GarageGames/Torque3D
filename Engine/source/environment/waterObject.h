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

#ifndef _WATEROBJECT_H_
#define _WATEROBJECT_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _GFXSTRUCTS_H_
#include "gfx/gfxStructs.h"
#endif
#ifndef _FOGSTRUCTS_H_
#include "scene/fogStructs.h"
#endif
#ifndef _GFXSTATEBLOCK_H_
#include "gfx/gfxStateBlock.h"
#endif
#ifndef _REFLECTOR_H_
#include "scene/reflector.h"
#endif
#ifndef _ALIGNEDARRAY_H_
#include "core/util/tAlignedArray.h"
#endif
#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif

GFXDeclareVertexFormat( GFXWaterVertex )
{
   Point3F point;
   Point3F normal;
   Point2F undulateData;
   Point4F horizonFactor;
};


class MaterialParameterHandle;
class BaseMatInstance;
class ShaderData;
class MatrixSet;

struct WaterMatParams
{
   MaterialParameterHandle* mRippleMatSC;   
   MaterialParameterHandle* mRippleDirSC;
   MaterialParameterHandle* mRippleTexScaleSC;
   MaterialParameterHandle* mRippleSpeedSC;
   MaterialParameterHandle* mRippleMagnitudeSC;
   MaterialParameterHandle* mFoamDirSC;
   MaterialParameterHandle* mFoamTexScaleSC;
   MaterialParameterHandle* mFoamOpacitySC;
   MaterialParameterHandle* mFoamSpeedSC;
   MaterialParameterHandle* mWaveDirSC;
   MaterialParameterHandle* mWaveDataSC;   
   MaterialParameterHandle* mReflectTexSizeSC;
   MaterialParameterHandle* mBaseColorSC;
   MaterialParameterHandle* mMiscParamsSC;
   MaterialParameterHandle* mReflectParamsSC;
   MaterialParameterHandle* mReflectNormalSC;
   MaterialParameterHandle* mHorizonPositionSC;
   MaterialParameterHandle* mFogParamsSC;   
   MaterialParameterHandle* mMoreFogParamsSC;
   MaterialParameterHandle* mFarPlaneDistSC;
   MaterialParameterHandle* mWetnessParamsSC;
   MaterialParameterHandle* mDistortionParamsSC;
   MaterialParameterHandle* mUndulateMaxDistSC;
   MaterialParameterHandle* mAmbientColorSC;
   MaterialParameterHandle* mLightDirSC;
   MaterialParameterHandle* mFoamParamsSC;   
   MaterialParameterHandle* mGridElementSizeSC;   
   MaterialParameterHandle* mElapsedTimeSC;
   MaterialParameterHandle* mModelMatSC;
   MaterialParameterHandle* mFoamSamplerSC;
   MaterialParameterHandle* mRippleSamplerSC;
   MaterialParameterHandle* mCubemapSamplerSC;
   MaterialParameterHandle* mSpecularParamsSC;   
   MaterialParameterHandle* mDepthGradMaxSC;
   MaterialParameterHandle* mReflectivitySC;
   MaterialParameterHandle* mDepthGradSamplerSC;

   void clear();
   void init(BaseMatInstance* matInst);
};


class GFXOcclusionQuery;
class PostEffect;
class CubemapData;

//-------------------------------------------------------------------------
// WaterObject Class
//-------------------------------------------------------------------------

class WaterObject : public SceneObject
{
   typedef SceneObject Parent;

protected:

   enum MaskBits {      
      UpdateMask     = Parent::NextFreeMask << 0,
      WaveMask       = Parent::NextFreeMask << 1,
      MaterialMask   = Parent::NextFreeMask << 2,
      TextureMask    = Parent::NextFreeMask << 3,
      SoundMask      = Parent::NextFreeMask << 4,
      NextFreeMask   = Parent::NextFreeMask << 5
   };

   enum consts {
      MAX_WAVES = 3,
      MAX_FOAM = 2,
      NUM_ANIM_FRAMES = 32,
   };

   enum MaterialType
   {
      WaterMat = 0,
      UnderWaterMat,      
      BasicWaterMat,
      BasicUnderWaterMat,      
      NumMatTypes
   };

public:

   WaterObject();
   virtual ~WaterObject();

   DECLARE_CONOBJECT( WaterObject );

   // ConsoleObject
   static void consoleInit();
   static void initPersistFields();

   // SimObject
   virtual bool onAdd();
   virtual void onRemove();
   virtual void inspectPostApply();
   virtual bool processArguments(S32 argc, ConsoleValueRef *argv);

   // NetObject
   virtual U32  packUpdate( NetConnection * conn, U32 mask, BitStream *stream );
   virtual void unpackUpdate( NetConnection * conn, BitStream *stream );

   // SceneObject
   virtual void prepRenderImage( SceneRenderState *state );
   virtual void renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );
   virtual SFXAmbience* getSoundAmbience() const { return mSoundAmbience; }
   virtual bool containsPoint( const Point3F& point ) { return isUnderwater( point ); }

   // WaterObject
   virtual F32 getViscosity() const { return mViscosity; }
   virtual F32 getDensity() const { return mDensity; }
   virtual F32 getSurfaceHeight( const Point2F &pos ) const { return 0.0f; };  
   virtual const char* getLiquidType() const { return mLiquidType; }
   virtual F32 getWaterCoverage( const Box3F &worldBox ) const { return 0.0f; }
   virtual VectorF getFlow( const Point3F &pos ) const { return Point3F::Zero; }
   virtual void updateUnderwaterEffect( SceneRenderState *state );
   virtual bool isUnderwater( const Point3F &pnt ) const { return false; }

protected:
      
   virtual void setShaderXForms( BaseMatInstance *mat ) {};
   virtual void setupVBIB() {};
   virtual void innerRender( SceneRenderState *state ) {};
   virtual void setCustomTextures( S32 matIdx, U32 pass, const WaterMatParams &paramHandles );
   void drawUnderwaterFilter( SceneRenderState *state );

   virtual void setShaderParams( SceneRenderState *state, BaseMatInstance *mat, const WaterMatParams &paramHandles );
   PostEffect* getUnderwaterEffect();

   bool initMaterial( S32 idx );   
   void cleanupMaterials();
   S32 getMaterialIndex( const Point3F &camPos );

   void initTextures();

   virtual void _getWaterPlane( const Point3F &camPos, PlaneF &outPlane, Point3F &outPos ) {}

   /// Callback used internally when smDisableTrueReflections changes.
   void _onDisableTrueRelfections();

protected:

   static bool _setFullReflect( void *object, const char *index, const char *data );
   static bool _checkDensity(void *object, const char *index, const char *data);

   // WaterObject
   F32 mViscosity;
   F32 mDensity;
   String mLiquidType;   
   F32 mFresnelBias;
   F32 mFresnelPower;
   F32 mSpecularPower;
   ColorF mSpecularColor;
   bool mEmissive;

   // Reflection
   bool mFullReflect;
   PlaneReflector mPlaneReflector;
   ReflectorDesc mReflectorDesc;
   PlaneF mWaterPlane;
   Point3F mWaterPos;
   bool mReflectNormalUp;
   F32 mReflectivity;

   // Water Fogging
   WaterFogData mWaterFogData;   

   // Distortion
   F32 mDistortStartDist;
   F32 mDistortEndDist;
   F32 mDistortFullDepth;   

   // Ripples
   Point2F  mRippleDir[ MAX_WAVES ];
   F32      mRippleSpeed[ MAX_WAVES ];
   Point2F  mRippleTexScale[ MAX_WAVES ];
   F32      mRippleMagnitude[ MAX_WAVES ];

   F32 mOverallRippleMagnitude;

   // Waves   
   Point2F  mWaveDir[ MAX_WAVES ];
   F32      mWaveSpeed[ MAX_WAVES ];   
   F32      mWaveMagnitude[ MAX_WAVES ];  
   
   F32 mOverallWaveMagnitude;

   // Foam
   Point2F  mFoamDir[ MAX_WAVES ];
   F32      mFoamSpeed[ MAX_WAVES ];
   Point2F  mFoamTexScale[ MAX_WAVES ];
   F32      mFoamOpacity[ MAX_WAVES ];
   
   F32 mOverallFoamOpacity;
   F32 mFoamMaxDepth;
   F32 mFoamAmbientLerp;
   F32 mFoamRippleInfluence;

   // Basic Lighting
   F32 mClarity;
   ColorI mUnderwaterColor;

   // Misc
   F32 mDepthGradientMax;

   // Other textures
   String mRippleTexName;
   String mFoamTexName;
   String mCubemapName;
   String mDepthGradientTexName;

   // Sound
   SFXAmbience* mSoundAmbience;

   // Not fields...

   /// Defined here and sent to the shader in WaterCommon::setShaderParams
   /// but needs to be initialized in child classes prior to that call.
   F32 mUndulateMaxDist;

   /// Defined in WaterCommon but set and used by child classes.
   /// If true will refuse to render a reflection even if called from 
   /// the ReflectionManager, is set true if occlusion query is enabled and 
   /// it determines it is occluded.
   //bool mSkipReflectUpdate;

   /// Derived classes can set this value prior to calling Parent::setShaderConst
   /// to pass it into the shader miscParam.w
   F32 mMiscParamW;   

   SimObjectPtr<PostEffect> mUnderwaterPostFx;

   /// A global for enabling wireframe rendering
   /// on all water objects.
   static bool smWireframe;

   /// Force all water objects to use static cubemap reflections
   static bool smDisableTrueReflections;

   // Rendering   
   bool mBasicLighting;
   //U32 mRenderUpdateCount;
   //U32 mReflectUpdateCount;
   bool mGenerateVB;
   String mSurfMatName[NumMatTypes];
   BaseMatInstance* mMatInstances[NumMatTypes];
   WaterMatParams mMatParamHandles[NumMatTypes];   
   bool mUnderwater;
   GFXStateBlockRef mUnderwaterSB;
   GFXTexHandle mRippleTex;
   GFXTexHandle mDepthGradientTex;
   GFXTexHandle mFoamTex;   
   CubemapData *mCubemap;
   MatrixSet *mMatrixSet;
   NamedTexTarget mNamedDepthGradTex;
};

#endif // _WATEROBJECT_H_
