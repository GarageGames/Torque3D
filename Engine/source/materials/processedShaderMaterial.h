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

#ifndef _MATERIALS_PROCESSEDSHADERMATERIAL_H_
#define _MATERIALS_PROCESSEDSHADERMATERIAL_H_

#ifndef _MATERIALS_PROCESSEDMATERIAL_H_
#include "processedMaterial.h"
#endif
#ifndef _GFXSHADER_H_
#include "gfx/gfxShader.h"
#endif

class GenericConstBufferLayout;
class ShaderData;
class LightInfo;
class ShaderMaterialParameterHandle;
class ShaderFeatureConstHandles;
class CustomMaterial;


class ShaderConstHandles
{
public:
   GFXShaderConstHandle* mDiffuseColorSC;
   GFXShaderConstHandle* mToneMapTexSC;
   GFXShaderConstHandle* mTexMatSC;
   GFXShaderConstHandle* mSpecularColorSC;
   GFXShaderConstHandle* mSpecularPowerSC;
   GFXShaderConstHandle* mParallaxInfoSC;
   GFXShaderConstHandle* mFogDataSC;
   GFXShaderConstHandle* mFogColorSC;   
   GFXShaderConstHandle* mDetailScaleSC;
   GFXShaderConstHandle* mVisiblitySC;
   GFXShaderConstHandle* mColorMultiplySC;
   GFXShaderConstHandle* mAlphaTestValueSC;
   GFXShaderConstHandle* mModelViewProjSC;
   GFXShaderConstHandle* mWorldViewOnlySC;     
   GFXShaderConstHandle* mWorldToCameraSC;
   GFXShaderConstHandle* mWorldToObjSC;         
   GFXShaderConstHandle* mViewToObjSC;         
   GFXShaderConstHandle* mCubeTransSC;
   GFXShaderConstHandle* mObjTransSC;
   GFXShaderConstHandle* mCubeEyePosSC;
   GFXShaderConstHandle* mEyePosSC;
   GFXShaderConstHandle* mEyePosWorldSC;
   GFXShaderConstHandle* m_vEyeSC;
   GFXShaderConstHandle* mEyeMatSC;
   GFXShaderConstHandle* mOneOverFarplane;
   GFXShaderConstHandle* mAccumTimeSC;
   GFXShaderConstHandle* mMinnaertConstantSC;
   GFXShaderConstHandle* mSubSurfaceParamsSC;
   GFXShaderConstHandle* mDiffuseAtlasParamsSC;
   GFXShaderConstHandle* mBumpAtlasParamsSC;
   GFXShaderConstHandle* mDiffuseAtlasTileSC;
   GFXShaderConstHandle* mBumpAtlasTileSC;
   GFXShaderConstHandle *mRTSizeSC;
   GFXShaderConstHandle *mOneOverRTSizeSC;
   GFXShaderConstHandle* mDetailBumpStrength;
   GFXShaderConstHandle* mViewProjSC;

   GFXShaderConstHandle *mImposterUVs;
   GFXShaderConstHandle *mImposterLimits;

   GFXShaderConstHandle* mTexHandlesSC[Material::MAX_TEX_PER_PASS];
   GFXShaderConstHandle* mRTParamsSC[TEXTURE_STAGE_COUNT];

   void init( GFXShader* shader, CustomMaterial* mat = NULL );
};

class ShaderRenderPassData : public RenderPassData
{
   typedef RenderPassData Parent;

public:

   virtual ~ShaderRenderPassData() { reset(); }

   GFXShaderRef         shader;
   ShaderConstHandles   shaderHandles;
   Vector<ShaderFeatureConstHandles*> featureShaderHandles;

   virtual void reset();
   virtual String describeSelf() const;
};

class ProcessedShaderMaterial : public ProcessedMaterial
{
   typedef ProcessedMaterial Parent;
public:

   ProcessedShaderMaterial();
   ProcessedShaderMaterial(Material &mat);
   ~ProcessedShaderMaterial();

   // ProcessedMaterial
   virtual bool init(   const FeatureSet &features, 
                        const GFXVertexFormat *vertexFormat,
                        const MatFeaturesDelegate &featuresDelegate );
   virtual bool setupPass(SceneRenderState *, const SceneData& sgData, U32 pass);
   virtual void setTextureStages(SceneRenderState *, const SceneData &sgData, U32 pass );
   virtual void setTransforms(const MatrixSet &matrixSet, SceneRenderState *state, const U32 pass);
   virtual void setSceneInfo(SceneRenderState *, const SceneData& sgData, U32 pass);
   virtual void setBuffers(GFXVertexBufferHandleBase* vertBuffer, GFXPrimitiveBufferHandle* primBuffer); 
   virtual bool stepInstance();
   virtual void dumpMaterialInfo();
   virtual MaterialParameters* allocMaterialParameters();    
   virtual MaterialParameters* getDefaultMaterialParameters() { return mDefaultParameters; }   
   virtual MaterialParameterHandle* getMaterialParameterHandle(const String& name);
   virtual U32 getNumStages();

protected:

   Vector<GFXShaderConstDesc> mShaderConstDesc;
   MaterialParameters* mDefaultParameters;
   Vector<ShaderMaterialParameterHandle*> mParameterHandles;

   /// Hold the instancing state data for the material.   
   class InstancingState
   {
      const static U32 COUNT = 200;

   public:

      InstancingState()
         :  mInstFormat( NULL ),
            mBuffer( NULL ),
            mCount( -1 )
      {
      }

      ~InstancingState()
      {
         delete [] mBuffer;
      }

      void setFormat( const GFXVertexFormat *instFormat, const GFXVertexFormat *vertexFormat )
      {
         mInstFormat = instFormat;
         mDeclFormat.copy( *vertexFormat );
         mDeclFormat.append( *mInstFormat, 1 );
         mDeclFormat.getDecl();

         delete [] mBuffer;
         mBuffer = new U8[ mInstFormat->getSizeInBytes() * COUNT ];
         mCount = -1;
      }

      bool step( U8 **outPtr )
      {
         // Are we starting a new draw call?
         if ( mCount < 0 )
         {
            *outPtr = mBuffer;
            mCount = 0;
         }
         else
         {
            // Increment to the next instance.
            *outPtr += mInstFormat->getSizeInBytes();
            mCount++;
         }

         return mCount < COUNT;
      }

      void resetStep() { mCount = -1; }

      U8* getBuffer() const { return mBuffer; }

      S32 getCount() const { return mCount; }

      const GFXVertexFormat* getFormat() const { return mInstFormat; }

      const GFXVertexFormat* getDeclFormat() const { return &mDeclFormat; }

   protected:

      GFXVertexFormat mDeclFormat;
      const GFXVertexFormat *mInstFormat;  
      U8 *mBuffer;
      S32 mCount;

   };

   /// The instancing state if this material
   /// supports instancing.
   InstancingState *mInstancingState;
   
   /// @name Internal functions
   ///
   /// @{

   /// Adds a pass for the given stage.
   virtual bool _addPass( ShaderRenderPassData &rpd, 
      U32 &texIndex, 
      MaterialFeatureData &fd,
      U32 stageNum,
      const FeatureSet &features);

   /// Chooses a blend op for the given pass
   virtual void _setPassBlendOp( ShaderFeature *sf,
      ShaderRenderPassData &passData,
      U32 &texIndex,
      MaterialFeatureData &stageFeatures,
      U32 stageNum,
      const FeatureSet &features);

   /// Creates passes for the given stage
   virtual bool _createPasses( MaterialFeatureData &fd, U32 stageNum, const FeatureSet &features );

   /// Fills in the MaterialFeatureData for the given stage
   virtual void _determineFeatures( U32 stageNum, 
                                    MaterialFeatureData &fd, 
                                    const FeatureSet &features );

   /// Do we have a cubemap on pass?
   virtual bool _hasCubemap(U32 pass);

   /// Used by setTextureTransforms
   F32 _getWaveOffset( U32 stage );

   /// Sets texture transformation matrices for texture animations such as scale and wave
   virtual void _setTextureTransforms(const U32 pass);

   /// Sets all of the necessary shader constants for the given pass
   virtual void _setShaderConstants(SceneRenderState *, const SceneData &sgData, U32 pass);

   /// @}

   void _setPrimaryLightConst(const LightInfo* light, const MatrixF& objTrans, const U32 stageNum);

   /// This is here to deal with the differences between ProcessedCustomMaterials and ProcessedShaderMaterials.
   virtual GFXShaderConstBuffer* _getShaderConstBuffer(const U32 pass);
   virtual ShaderConstHandles* _getShaderConstHandles(const U32 pass);

   ///
   virtual void _initMaterialParameters();

   ShaderRenderPassData* _getRPD(const U32 pass) { return static_cast<ShaderRenderPassData*>(mPasses[pass]); }
};

#endif // _MATERIALS_PROCESSEDSHADERMATERIAL_H_
