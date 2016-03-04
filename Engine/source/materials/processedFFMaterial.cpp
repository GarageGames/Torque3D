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
#include "materials/processedFFMaterial.h"

#include "gfx/sim/cubemapData.h"
#include "materials/sceneData.h"
#include "materials/customMaterialDefinition.h"
#include "materials/materialFeatureTypes.h"
#include "gfx/sim/gfxStateBlockData.h"
#include "gfx/gfxDevice.h"
#include "gfx/genericConstBuffer.h"
#include "materials/materialParameters.h"
#include "lighting/lightInfo.h"
#include "scene/sceneRenderState.h"
#include "core/util/safeDelete.h"
#include "math/util/matrixSet.h"

class FFMaterialParameterHandle : public MaterialParameterHandle
{
public:
   virtual ~FFMaterialParameterHandle() {}
   virtual const String& getName() const { return mName; }
   virtual bool isValid() const { return false; }
   virtual S32 getSamplerRegister( U32 pass ) const { return -1; }
private:
   String mName;
};

ProcessedFFMaterial::ProcessedFFMaterial()
{
   VECTOR_SET_ASSOCIATION( mParamDesc );

   _construct();
}

ProcessedFFMaterial::ProcessedFFMaterial(Material &mat, const bool isLightingMaterial)
{
   VECTOR_SET_ASSOCIATION( mParamDesc );

   _construct();
   mMaterial = &mat;
   mIsLightingMaterial = isLightingMaterial;
}

void ProcessedFFMaterial::_construct()
{   
   mHasSetStageData = false;
   mHasGlow = false;
   mHasAccumulation = false;
   mIsLightingMaterial = false;
   mDefaultHandle = new FFMaterialParameterHandle();
   mDefaultParameters = new MaterialParameters();
   mCurrentParams = mDefaultParameters;
}

ProcessedFFMaterial::~ProcessedFFMaterial()
{
   SAFE_DELETE(mDefaultParameters);
   SAFE_DELETE( mDefaultHandle );
}

void ProcessedFFMaterial::_createPasses( U32 stageNum, const FeatureSet &features )
{
   FixedFuncFeatureData featData;
   _determineFeatures(stageNum, featData, features);
   // Just create a simple pass!
   _addPass(0, featData);

   mFeatures.clear();
   if ( featData.features[FixedFuncFeatureData::DiffuseMap] ) 
      mFeatures.addFeature( MFT_DiffuseMap );
   if ( featData.features[FixedFuncFeatureData::LightMap] ) 
      mFeatures.addFeature( MFT_LightMap );
   if ( featData.features[FixedFuncFeatureData::ToneMap] ) 
      mFeatures.addFeature( MFT_ToneMap );

}

void ProcessedFFMaterial::_determineFeatures(   U32 stageNum, 
                                                FixedFuncFeatureData& featData, 
                                                const FeatureSet &features )
{
   if ( mStages[stageNum].getTex( MFT_DiffuseMap ) )
      featData.features[FixedFuncFeatureData::DiffuseMap] = true;

   if ( features.hasFeature( MFT_LightMap ) )
      featData.features[FixedFuncFeatureData::LightMap] = true;
   if ( features.hasFeature( MFT_ToneMap )) 
      featData.features[FixedFuncFeatureData::ToneMap] = true;
}

U32 ProcessedFFMaterial::getNumStages()
{
   // Loops through all stages to determine how many stages we actually use
   U32 numStages = 0;

   U32 i;
   for( i=0; i<Material::MAX_STAGES; i++ )
   {
      // Assume stage is inactive
      bool stageActive = false;

      // Cubemaps only on first stage
      if( i == 0 )
      {
         // If we have a cubemap the stage is active
         if( mMaterial->mCubemapData || mMaterial->mDynamicCubemap )
         {
            numStages++;
            continue;
         }
      }

      // If we have a texture for the a feature the 
      // stage is active.
      if ( mStages[i].hasValidTex() )
         stageActive = true;

      // If this stage has specular lighting, it's active
      if (  mMaterial->mPixelSpecular[i] )
         stageActive = true;

      // If we have a Material that is vertex lit
      // then it may not have a texture
      if( mMaterial->mVertLit[i] )
      {
         stageActive = true;
      }

      // Increment the number of active stages
      numStages += stageActive;
   }


   return numStages;
}

bool ProcessedFFMaterial::setupPass( SceneRenderState *state, const SceneData &sgData, U32 pass )
{
   PROFILE_SCOPE( ProcessedFFMaterial_SetupPass );

   // Make sure we have a pass
   if(pass >= mPasses.size())
      return false;

   _setRenderState( state, sgData, pass );

   // Bind our textures
   setTextureStages( state, sgData, pass );
   return true;
}

void ProcessedFFMaterial::setTextureStages(SceneRenderState * state, const SceneData& sgData, U32 pass)
{
   // We may need to do some trickery in here for fixed function, this is just copy/paste from MatInstance
#ifdef TORQUE_DEBUG
   AssertFatal( pass<mPasses.size(), "Pass out of bounds" );
#endif
   RenderPassData *rpd = mPasses[pass];
   for( U32 i=0; i<rpd->mNumTex; i++ )
   {      
      U32 currTexFlag = rpd->mTexType[i];
      if (!LIGHTMGR || !LIGHTMGR->setTextureStage(sgData, currTexFlag, i, NULL, NULL))
      {
         switch( currTexFlag )
         {
         case Material::NoTexture:
            if (rpd->mTexSlot[i].texObject)
               GFX->setTexture( i, rpd->mTexSlot[i].texObject );
            break;

         case Material::NormalizeCube:
            GFX->setCubeTexture(i, Material::GetNormalizeCube());
            break;

         case Material::Lightmap:
            GFX->setTexture( i, sgData.lightmap );
            break;

         case Material::Cube:
            // TODO: Is this right?
            GFX->setTexture( i, rpd->mTexSlot[0].texObject );
            break;

         case Material::SGCube:
            // No cubemap support just yet
            //GFX->setCubeTexture( i, sgData.cubemap );
            GFX->setTexture( i, rpd->mTexSlot[0].texObject );
            break;

         case Material::BackBuff:
            GFX->setTexture( i, sgData.backBuffTex );
            break;
         }
      }
   }
}

MaterialParameters* ProcessedFFMaterial::allocMaterialParameters()
{   
   return new MaterialParameters();
}

MaterialParameters* ProcessedFFMaterial::getDefaultMaterialParameters()
{
   return mDefaultParameters;
}

MaterialParameterHandle* ProcessedFFMaterial::getMaterialParameterHandle(const String& name)
{
   return mDefaultHandle;
}

void ProcessedFFMaterial::setTransforms(const MatrixSet &matrixSet, SceneRenderState *state, const U32 pass)
{
   GFX->setWorldMatrix(matrixSet.getObjectToWorld());
   GFX->setViewMatrix(matrixSet.getWorldToCamera());
   GFX->setProjectionMatrix(matrixSet.getCameraToScreen());
}

void ProcessedFFMaterial::setSceneInfo(SceneRenderState * state, const SceneData& sgData, U32 pass)
{
   _setPrimaryLightInfo(*sgData.objTrans, sgData.lights[0], pass);
   _setSecondaryLightInfo(*sgData.objTrans, sgData.lights[1]);   
}

void ProcessedFFMaterial::_setPrimaryLightInfo(const MatrixF &_objTrans, LightInfo* light, U32 pass)
{
   // Just in case
   GFX->setGlobalAmbientColor(ColorF(0.0f, 0.0f, 0.0f, 1.0f));
   if ( light->getType() == LightInfo::Ambient )
   {
      // Ambient light
      GFX->setGlobalAmbientColor( light->getAmbient() );
      return;
   }

   GFX->setLight(0, NULL);
   GFX->setLight(1, NULL);
   // This is a quick hack that lets us use FF lights
   GFXLightMaterial lightMat;
   lightMat.ambient = ColorF(1.0f, 1.0f, 1.0f, 1.0f);
   lightMat.diffuse = ColorF(1.0f, 1.0f, 1.0f, 1.0f);
   lightMat.emissive = ColorF(0.0f, 0.0f, 0.0f, 0.0f);
   lightMat.specular = ColorF(0.0f, 0.0f, 0.0f, 0.0f);
   lightMat.shininess = 128.0f;
   GFX->setLightMaterial(lightMat);   

   // set object transform
   MatrixF objTrans = _objTrans;
   objTrans.inverse();

   // fill in primary light
   //-------------------------
   GFXLightInfo xlatedLight;
   light->setGFXLight(&xlatedLight);
   Point3F lightPos = light->getPosition();
   Point3F lightDir = light->getDirection();
   objTrans.mulP(lightPos);
   objTrans.mulV(lightDir);

   xlatedLight.mPos = lightPos;
   xlatedLight.mDirection = lightDir;

   GFX->setLight(0, &xlatedLight);
}

void ProcessedFFMaterial::_setSecondaryLightInfo(const MatrixF &_objTrans, LightInfo* light)
{
   // set object transform
   MatrixF objTrans = _objTrans;
   objTrans.inverse();

   // fill in secondary light
   //-------------------------
   GFXLightInfo xlatedLight;
   light->setGFXLight(&xlatedLight);

   Point3F lightPos = light->getPosition();
   Point3F lightDir = light->getDirection();
   objTrans.mulP(lightPos);
   objTrans.mulV(lightDir);

   xlatedLight.mPos = lightPos;
   xlatedLight.mDirection = lightDir;

   GFX->setLight(1, &xlatedLight);
}

bool ProcessedFFMaterial::init(  const FeatureSet &features, 
                                 const GFXVertexFormat *vertexFormat,
                                 const MatFeaturesDelegate &featuresDelegate )
{
   TORQUE_UNUSED( vertexFormat );
   TORQUE_UNUSED( featuresDelegate );

   _setStageData();

   // Just create a simple pass
   _createPasses(0, features);
   _initRenderPassDataStateBlocks();
   mStateHint.init( this );

   return true;
}

void ProcessedFFMaterial::_addPass(U32 stageNum, FixedFuncFeatureData& featData)
{
   U32 numTex = 0;

   // Just creates a simple pass, but it can still glow!
   RenderPassData rpd;

   // Base texture, texunit 0
   if(featData.features[FixedFuncFeatureData::DiffuseMap])
   {
      rpd.mTexSlot[0].texObject = mStages[stageNum].getTex( MFT_DiffuseMap );
      rpd.mTexType[0] = Material::NoTexture;
      numTex++;
   }

   // lightmap, texunit 1
   if(featData.features[FixedFuncFeatureData::LightMap])
   {
      rpd.mTexType[1] = Material::Lightmap;
      numTex++;
   }

   rpd.mNumTex = numTex;
   rpd.mStageNum = stageNum;
   rpd.mGlow = false;

   mPasses.push_back( new RenderPassData(rpd) );
}

void ProcessedFFMaterial::_setPassBlendOp()
{

}

void ProcessedFFMaterial::_initPassStateBlock( RenderPassData *rpd, GFXStateBlockDesc &result )
{
   Parent::_initPassStateBlock( rpd, result );

   if ( mIsLightingMaterial )
   {
      result.ffLighting = true;
      result.blendDefined = true;
      result.blendEnable = true;
      result.blendSrc = GFXBlendOne;
      result.blendDest = GFXBlendZero;
   }

   // This is here for generic FF shader fallbacks.
   CustomMaterial* custmat = dynamic_cast<CustomMaterial*>(mMaterial);
   if (custmat && custmat->getStateBlockData() )
      result.addDesc(custmat->getStateBlockData()->getState());
}
