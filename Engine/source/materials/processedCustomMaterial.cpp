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
#include "materials/processedCustomMaterial.h"

#include "gfx/sim/cubemapData.h"
#include "materials/sceneData.h"
#include "shaderGen/shaderGenVars.h"
#include "scene/sceneRenderState.h"
#include "materials/customMaterialDefinition.h"
#include "materials/shaderData.h"
#include "materials/materialManager.h"
#include "materials/matTextureTarget.h"
#include "materials/materialFeatureTypes.h"
#include "materials/materialParameters.h"
#include "gfx/sim/gfxStateBlockData.h"
#include "core/util/safeDelete.h"
#include "gfx/genericConstBuffer.h"
#include "console/simFieldDictionary.h"
#include "console/propertyParsing.h"
#include "gfx/util/screenspace.h"
#include "scene/reflectionManager.h"


ProcessedCustomMaterial::ProcessedCustomMaterial(Material &mat)
{
   mMaterial = &mat;
   AssertFatal(dynamic_cast<CustomMaterial*>(mMaterial), "Incompatible Material type!");
   mCustomMaterial = static_cast<CustomMaterial*>(mMaterial);
   mHasSetStageData = false;
   mHasGlow = false;
   mHasAccumulation = false;
   mMaxStages = 0;
   mMaxTex = 0;
}

ProcessedCustomMaterial::~ProcessedCustomMaterial()
{   
}

void ProcessedCustomMaterial::_setStageData()
{
   // Only do this once
   if ( mHasSetStageData ) 
      return;
   mHasSetStageData = true;   

   ShaderRenderPassData* rpd = _getRPD(0);   
   mConditionerMacros.clear();

   // Loop through all the possible textures, set the right flags, and load them if needed
   for(U32 i=0; i<CustomMaterial::MAX_TEX_PER_PASS; i++ )
   {
      rpd->mTexType[i] = Material::NoTexture;   // Set none as the default in case none of the cases below catch it.
      String filename = mCustomMaterial->mTexFilename[i];

      if(filename.isEmpty())
         continue;

      if(filename.equal(String("$dynamiclight"), String::NoCase))
      {
         rpd->mTexType[i] = Material::DynamicLight;
         rpd->mSamplerNames[i] = mCustomMaterial->mSamplerNames[i];
         mMaxTex = i+1;
         continue;
      }

      if(filename.equal(String("$dynamicShadowMap"), String::NoCase))
      {
         rpd->mTexType[i] = Material::DynamicShadowMap;
         rpd->mSamplerNames[i] = mCustomMaterial->mSamplerNames[i];
         mMaxTex = i+1;
         continue;
      }

      if(filename.equal(String("$dynamiclightmask"), String::NoCase))
      {
         rpd->mTexType[i] = Material::DynamicLightMask;
         rpd->mSamplerNames[i] = mCustomMaterial->mSamplerNames[i];
         mMaxTex = i+1;
         continue;
      }

      if(filename.equal(String("$lightmap"), String::NoCase))
      {
         rpd->mTexType[i] = Material::Lightmap;
         rpd->mSamplerNames[i] = mCustomMaterial->mSamplerNames[i];
         mMaxTex = i+1;
         continue;
      }

      if(filename.equal(String("$cubemap"), String::NoCase))
      {
         if( mCustomMaterial->mCubemapData )
         {
            rpd->mTexType[i] = Material::Cube;
            rpd->mSamplerNames[i] = mCustomMaterial->mSamplerNames[i];
            mMaxTex = i+1;
         }
         else
         {
            mCustomMaterial->logError( "Could not find CubemapData - %s", mCustomMaterial->mCubemapName.c_str());
         }
         continue;
      }

      if(filename.equal(String("$dynamicCubemap"), String::NoCase))
      {
         rpd->mTexType[i] = Material::SGCube;
         rpd->mSamplerNames[i] = mCustomMaterial->mSamplerNames[i];
         mMaxTex = i+1;
         continue;
      }

      if(filename.equal(String("$backbuff"), String::NoCase))
      {
         rpd->mTexType[i] = Material::BackBuff;
         rpd->mSamplerNames[i] = mCustomMaterial->mSamplerNames[i];
         mMaxTex = i+1;
         continue;
      }

      if(filename.equal(String("$reflectbuff"), String::NoCase))
      {
         rpd->mTexType[i] = Material::ReflectBuff;
         rpd->mSamplerNames[i] = mCustomMaterial->mSamplerNames[i];
         mMaxTex = i+1;
         continue;
      }

      if(filename.equal(String("$miscbuff"), String::NoCase))
      {
         rpd->mTexType[i] = Material::Misc;
         rpd->mSamplerNames[i] = mCustomMaterial->mSamplerNames[i];
         mMaxTex = i+1;
         continue;
      }

      // Check for a RenderTexTargetBin assignment
      if (filename.substr( 0, 1 ).equal("#"))
      {
         String texTargetBufferName = filename.substr(1, filename.length() - 1);
         NamedTexTarget *texTarget = NamedTexTarget::find( texTargetBufferName ); 
         rpd->mTexSlot[i].texTarget = texTarget;

         // Get the conditioner macros.
         if ( texTarget )
            texTarget->getShaderMacros( &mConditionerMacros );

         rpd->mTexType[i] = Material::TexTarget;
         rpd->mSamplerNames[i] = mCustomMaterial->mSamplerNames[i];
         mMaxTex = i+1;
         continue;
      }

      rpd->mTexSlot[i].texObject = _createTexture( filename, &GFXDefaultStaticDiffuseProfile );
      if ( !rpd->mTexSlot[i].texObject )
      {
         mMaterial->logError("Failed to load texture %s", _getTexturePath(filename).c_str());
         continue;
      }
      rpd->mTexType[i] = Material::Standard;
      rpd->mSamplerNames[i] = mCustomMaterial->mSamplerNames[i];
      mMaxTex = i+1;
   }

   // We only get one cubemap
   if( mCustomMaterial->mCubemapData )
   {
      mCustomMaterial->mCubemapData->createMap();      
      rpd->mCubeMap = mMaterial->mCubemapData->mCubemap; // BTRTODO ?
      if ( !rpd->mCubeMap )
         mMaterial->logError("Failed to load cubemap");
   }

   // If this has a output target defined, it may be writing 
   // to a tex target bin with a conditioner, so search for 
   // one and add its macros.
   if ( mCustomMaterial->mOutputTarget.isNotEmpty() )
   {
      NamedTexTarget *texTarget = NamedTexTarget::find( mCustomMaterial->mOutputTarget );
      if ( texTarget )
         texTarget->getShaderMacros( &mConditionerMacros );
   }

   // Copy the glow state over.
   mHasGlow = mCustomMaterial->mGlow[0];
}

bool ProcessedCustomMaterial::init( const FeatureSet &features, 
                                    const GFXVertexFormat *vertexFormat,
                                    const MatFeaturesDelegate &featuresDelegate )
{
   // If we don't have a shader data... we have nothing to do.
   if ( !mCustomMaterial->mShaderData )
      return true;

   // Custom materials only do one pass at the moment... so
   // add one for the stage data to fill in.
   ShaderRenderPassData *rpd = new ShaderRenderPassData();
   mPasses.push_back( rpd );

   _setStageData();
   _initPassStateBlocks();   
   mStateHint.clear();

   // Note: We don't use the vertex format in a custom 
   // material at all right now.
   //
   // Maybe we can add some required semantics and
   // validate that the format fits the shader?

   // Build a composite list of shader macros from
   // the conditioner and the user defined lists.
   Vector<GFXShaderMacro> macros;
   macros.merge( mConditionerMacros );
   macros.merge( mUserMacros );

   // Ask the shader data to give us a shader instance.
   rpd->shader = mCustomMaterial->mShaderData->getShader( macros );      
   if ( !rpd->shader )
   {
      delete rpd;
      mPasses.clear();
      return false;
   }

   rpd->shaderHandles.init( rpd->shader, mCustomMaterial );      
   _initMaterialParameters();
   mDefaultParameters = allocMaterialParameters();
   setMaterialParameters( mDefaultParameters, 0 );
   mStateHint.init( this );
   
   for(int i = 0; i < mMaxTex; i++)
   {
      ShaderConstHandles *handles = _getShaderConstHandles( mPasses.size()-1 );
      AssertFatal(handles,"");

      if(rpd->mSamplerNames[i].isEmpty())      
         continue;      

      String samplerName = rpd->mSamplerNames[i].startsWith("$") ? rpd->mSamplerNames[i] : String("$") + rpd->mSamplerNames[i];
      GFXShaderConstHandle *handle = rpd->shader->getShaderConstHandle( samplerName ); 
      AssertFatal(handle,"");
      handles->mTexHandlesSC[i] = handle;
   }
   
   return true;
}

void ProcessedCustomMaterial::_initPassStateBlock( RenderPassData *rpd, GFXStateBlockDesc &result )
{
   Parent::_initPassStateBlock( rpd, result );

   if (mCustomMaterial->getStateBlockData())
      result.addDesc(mCustomMaterial->getStateBlockData()->getState());
}

void ProcessedCustomMaterial::_initPassStateBlocks()
{
   AssertFatal(mHasSetStageData, "State data must be set before initializing state block!");
   ShaderRenderPassData* rpd = _getRPD(0);
   _initRenderStateStateBlocks( rpd );
}

bool ProcessedCustomMaterial::_hasCubemap(U32 pass)
{
   // If the material doesn't have a cubemap, we don't
   if( mMaterial->mCubemapData ) return true;
   else return false;
}

bool ProcessedCustomMaterial::setupPass( SceneRenderState *state, const SceneData& sgData, U32 pass )
{
   PROFILE_SCOPE( ProcessedCustomMaterial_SetupPass );

   // Make sure we have a pass.
   if ( pass >= mPasses.size() )
      return false;

   ShaderRenderPassData* rpd = _getRPD( pass );
   U32 currState = _getRenderStateIndex( state, sgData );
   GFX->setStateBlock(rpd->mRenderStates[currState]);      

   // activate shader
   if ( rpd->shader )
      GFX->setShader( rpd->shader );
   else
      GFX->setupGenericShaders();

   // Set our textures   
   setTextureStages( state, sgData, pass );   
   
   GFXShaderConstBuffer* shaderConsts = _getShaderConstBuffer(pass);
   GFX->setShaderConstBuffer(shaderConsts);
   
   // Set our shader constants.
   _setTextureTransforms(pass);
   _setShaderConstants(state, sgData, pass);

   LightManager* lm = state ? LIGHTMGR : NULL;
   if (lm)
      lm->setLightInfo(this, NULL, sgData, state, pass, shaderConsts);

   shaderConsts->setSafe(rpd->shaderHandles.mAccumTimeSC, MATMGR->getTotalTime());   

   return true;
}

void ProcessedCustomMaterial::setTextureStages( SceneRenderState *state, const SceneData &sgData, U32 pass )
{      
   LightManager* lm = state ? LIGHTMGR : NULL;   
   ShaderRenderPassData* rpd = _getRPD(pass);
   ShaderConstHandles* handles = _getShaderConstHandles(pass);
   GFXShaderConstBuffer* shaderConsts = _getShaderConstBuffer(pass);

   const NamedTexTarget *texTarget;
   GFXTextureObject *texObject; 
   
   for( U32 i=0; i<mMaxTex; i++ )
   {            
      U32 currTexFlag = rpd->mTexType[i];
      if ( !lm || !lm->setTextureStage(sgData, currTexFlag, i, shaderConsts, handles ) )
      {
      	GFXShaderConstHandle* handle = handles->mTexHandlesSC[i];
         if ( !handle->isValid() )
         	continue;

         S32 samplerRegister = handle->getSamplerRegister();
         
         switch( currTexFlag )
         {
         case 0:
         default:
            break;

         case Material::Mask:
         case Material::Standard:
         case Material::Bump:
         case Material::Detail:
            {
               GFX->setTexture( samplerRegister, rpd->mTexSlot[i].texObject );
               break;
            }

         case Material::Lightmap:
            {
               GFX->setTexture( samplerRegister, sgData.lightmap );
               break;
            }
         case Material::Cube:
            {
               GFX->setCubeTexture( samplerRegister, rpd->mCubeMap );
               break;
            }
         case Material::SGCube:
            {
               GFX->setCubeTexture( samplerRegister, sgData.cubemap );
               break;
            }
         case Material::BackBuff:
            {
               GFX->setTexture( samplerRegister, sgData.backBuffTex );
               //if ( sgData.reflectTex )
               //   GFX->setTexture( samplerRegister, sgData.reflectTex );
               //else
               //{
               //    GFXTextureObject *refractTex = REFLECTMGR->getRefractTex( true );
               //    GFX->setTexture( samplerRegister, refractTex );
               //}
               break;
            }
         case Material::ReflectBuff:
            {
               GFX->setTexture( samplerRegister, sgData.reflectTex );
               break;
            }
         case Material::Misc:
            {
               GFX->setTexture( samplerRegister, sgData.miscTex );
               break;
            }
         case Material::TexTarget:
            {
               texTarget = rpd->mTexSlot[i].texTarget;
               if ( !texTarget )
               {
                  GFX->setTexture( samplerRegister, NULL );
                  break;
               }
               
               texObject = texTarget->getTexture();

               // If no texture is available then map the default 2x2
               // black texture to it.  This at least will ensure that
               // we get consistant behavior across GPUs and platforms.
               if ( !texObject )
                  texObject = GFXTexHandle::ZERO;

               if ( handles->mRTParamsSC[samplerRegister]->isValid() && texObject )
               {
                  const Point3I &targetSz = texObject->getSize();
                  const RectI &targetVp = texTarget->getViewport();
                  Point4F rtParams;

                  ScreenSpace::RenderTargetParameters(targetSz, targetVp, rtParams);
                  shaderConsts->set(handles->mRTParamsSC[samplerRegister], rtParams);
               }
              
               GFX->setTexture( samplerRegister, texObject );
               break;
            }
         }
      }
   }
}

template <typename T>
void ProcessedCustomMaterial::setMaterialParameter(MaterialParameters* param, 
                                                   MaterialParameterHandle* handle,
                                                   const String& value)
{
   T typedValue;
   if (PropertyInfo::default_scan(value, typedValue))
   {      
      param->set(handle, typedValue);
   } else {
      Con::errorf("Error setting %s, parse error: %s", handle->getName().c_str(), value.c_str());
   }
}

void ProcessedCustomMaterial::setMatrixParameter(MaterialParameters* param, 
                                                 MaterialParameterHandle* handle,
                                                 const String& value, GFXShaderConstType matrixType)
{
   MatrixF result(true);
   F32* m = result;
   switch (matrixType)
   {
   case GFXSCT_Float2x2 :
      dSscanf(value.c_str(),"%g %g %g %g", 
         m[result.idx(0,0)], m[result.idx(0,1)], 
         m[result.idx(1,0)], m[result.idx(1,1)]);
      break;
   case GFXSCT_Float3x3 :
      dSscanf(value.c_str(),"%g %g %g %g %g %g %g %g %g", 
         m[result.idx(0,0)], m[result.idx(0,1)], m[result.idx(0,2)], 
         m[result.idx(1,0)], m[result.idx(1,1)], m[result.idx(1,2)], 
         m[result.idx(2,0)], m[result.idx(2,1)], m[result.idx(2,2)]);         
      break;
   default:
      AssertFatal(false, "Invalid type!");
      break;
   }
}

// BTRTODO: Support arrays!?
MaterialParameters* ProcessedCustomMaterial::allocMaterialParameters()
{
   MaterialParameters* ret = Parent::allocMaterialParameters();
   // See if any of the dynamic fields match up with shader constants we have.
   SimFieldDictionary* fields = mMaterial->getFieldDictionary();
   if (!fields || fields->getNumFields() == 0)
      return ret;

   const Vector<GFXShaderConstDesc>& consts = ret->getShaderConstDesc();
   for (U32 i = 0; i < consts.size(); i++)
   {
      // strip the dollar sign from the front.
      String stripped(consts[i].name);
      stripped.erase(0, 1);      

      SimFieldDictionary::Entry* field = fields->findDynamicField(stripped);      
      if (field)
      {
         MaterialParameterHandle* handle = getMaterialParameterHandle(consts[i].name);
         switch (consts[i].constType)
         {
         case GFXSCT_Float :
            setMaterialParameter<F32>(ret, handle, field->value);
            break;
         case GFXSCT_Float2: 
            setMaterialParameter<Point2F>(ret, handle, field->value);
            break;            
         case GFXSCT_Float3: 
            setMaterialParameter<Point3F>(ret, handle, field->value);
            break;            
         case GFXSCT_Float4: 
            setMaterialParameter<Point4F>(ret, handle, field->value);
            break;            
         case GFXSCT_Float2x2:                         
         case GFXSCT_Float3x3: 
            setMatrixParameter(ret, handle, field->value, consts[i].constType);
            break;
         case GFXSCT_Float4x4: 
            setMaterialParameter<MatrixF>(ret, handle, field->value);
            break;            
         case GFXSCT_Int: 
            setMaterialParameter<S32>(ret, handle, field->value);
            break;
         case GFXSCT_Int2: 
            setMaterialParameter<Point2I>(ret, handle, field->value);
            break;
         case GFXSCT_Int3: 
            setMaterialParameter<Point3I>(ret, handle, field->value);
            break;
         case GFXSCT_Int4: 
            setMaterialParameter<Point4I>(ret, handle, field->value);
            break;
         // Do we want to ignore these?
         case GFXSCT_Sampler:
         case GFXSCT_SamplerCube:
         default:
            break;
         }
      }
   }
   return ret;
}