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
#include "materials/processedShaderMaterial.h"

#include "core/util/safeDelete.h"
#include "gfx/sim/cubemapData.h"
#include "gfx/gfxShader.h"
#include "gfx/genericConstBuffer.h"
#include "gfx/gfxPrimitiveBuffer.h"
#include "scene/sceneRenderState.h"
#include "shaderGen/shaderFeature.h"
#include "shaderGen/shaderGenVars.h"
#include "shaderGen/featureMgr.h"
#include "shaderGen/shaderGen.h"
#include "materials/sceneData.h"
#include "materials/materialFeatureTypes.h"
#include "materials/materialManager.h"
#include "materials/shaderMaterialParameters.h"
#include "materials/matTextureTarget.h"
#include "gfx/util/screenspace.h"
#include "math/util/matrixSet.h"

// We need to include customMaterialDefinition for ShaderConstHandles::init
#include "materials/customMaterialDefinition.h"

///
/// ShaderConstHandles
///
void ShaderConstHandles::init( GFXShader *shader, CustomMaterial* mat /*=NULL*/ )
{
   mDiffuseColorSC = shader->getShaderConstHandle("$diffuseMaterialColor");
   mTexMatSC = shader->getShaderConstHandle(ShaderGenVars::texMat);
   mToneMapTexSC = shader->getShaderConstHandle(ShaderGenVars::toneMap);
   mSpecularColorSC = shader->getShaderConstHandle(ShaderGenVars::specularColor);
   mSpecularPowerSC = shader->getShaderConstHandle(ShaderGenVars::specularPower);
   mSpecularStrengthSC = shader->getShaderConstHandle(ShaderGenVars::specularStrength);
   mAccuScaleSC = shader->getShaderConstHandle("$accuScale");
   mAccuDirectionSC = shader->getShaderConstHandle("$accuDirection");
   mAccuStrengthSC = shader->getShaderConstHandle("$accuStrength");
   mAccuCoverageSC = shader->getShaderConstHandle("$accuCoverage");
   mAccuSpecularSC = shader->getShaderConstHandle("$accuSpecular");
   mParallaxInfoSC = shader->getShaderConstHandle("$parallaxInfo");
   mFogDataSC = shader->getShaderConstHandle(ShaderGenVars::fogData);
   mFogColorSC = shader->getShaderConstHandle(ShaderGenVars::fogColor);
   mDetailScaleSC = shader->getShaderConstHandle(ShaderGenVars::detailScale);
   mVisiblitySC = shader->getShaderConstHandle(ShaderGenVars::visibility);
   mColorMultiplySC = shader->getShaderConstHandle(ShaderGenVars::colorMultiply);
   mAlphaTestValueSC = shader->getShaderConstHandle(ShaderGenVars::alphaTestValue);
   mModelViewProjSC = shader->getShaderConstHandle(ShaderGenVars::modelview);
   mWorldViewOnlySC = shader->getShaderConstHandle(ShaderGenVars::worldViewOnly);
   mWorldToCameraSC = shader->getShaderConstHandle(ShaderGenVars::worldToCamera);
   mWorldToObjSC = shader->getShaderConstHandle(ShaderGenVars::worldToObj);
   mViewToObjSC = shader->getShaderConstHandle(ShaderGenVars::viewToObj);
   mCubeTransSC = shader->getShaderConstHandle(ShaderGenVars::cubeTrans);
   mObjTransSC = shader->getShaderConstHandle(ShaderGenVars::objTrans);
   mCubeEyePosSC = shader->getShaderConstHandle(ShaderGenVars::cubeEyePos);
   mEyePosSC = shader->getShaderConstHandle(ShaderGenVars::eyePos);
   mEyePosWorldSC = shader->getShaderConstHandle(ShaderGenVars::eyePosWorld);
   m_vEyeSC = shader->getShaderConstHandle(ShaderGenVars::vEye);
   mEyeMatSC = shader->getShaderConstHandle(ShaderGenVars::eyeMat);
   mOneOverFarplane = shader->getShaderConstHandle(ShaderGenVars::oneOverFarplane);
   mAccumTimeSC = shader->getShaderConstHandle(ShaderGenVars::accumTime);
   mMinnaertConstantSC = shader->getShaderConstHandle(ShaderGenVars::minnaertConstant);
   mSubSurfaceParamsSC = shader->getShaderConstHandle(ShaderGenVars::subSurfaceParams);
   mDiffuseAtlasParamsSC = shader->getShaderConstHandle(ShaderGenVars::diffuseAtlasParams);
   mDiffuseAtlasTileSC = shader->getShaderConstHandle(ShaderGenVars::diffuseAtlasTileParams);
   mBumpAtlasParamsSC = shader->getShaderConstHandle(ShaderGenVars::bumpAtlasParams);
   mBumpAtlasTileSC = shader->getShaderConstHandle(ShaderGenVars::bumpAtlasTileParams);
   mRTSizeSC = shader->getShaderConstHandle( "$targetSize" );
   mOneOverRTSizeSC = shader->getShaderConstHandle( "$oneOverTargetSize" );
   mDetailBumpStrength = shader->getShaderConstHandle( "$detailBumpStrength" );
   mViewProjSC = shader->getShaderConstHandle( "$viewProj" );

   // MFT_ImposterVert
   mImposterUVs = shader->getShaderConstHandle( "$imposterUVs" );
   mImposterLimits = shader->getShaderConstHandle( "$imposterLimits" );

   for (S32 i = 0; i < TEXTURE_STAGE_COUNT; ++i)
      mRTParamsSC[i] = shader->getShaderConstHandle( String::ToString( "$rtParams%d", i ) );

   // Clear any existing texture handles.
   dMemset( mTexHandlesSC, 0, sizeof( mTexHandlesSC ) );
   if(mat)
   {
      for (S32 i = 0; i < Material::MAX_TEX_PER_PASS; ++i)
         mTexHandlesSC[i] = shader->getShaderConstHandle(mat->mSamplerNames[i]);
   }

   // Deferred Shading
   mMatInfoFlagsSC = shader->getShaderConstHandle(ShaderGenVars::matInfoFlags);
}

///
/// ShaderRenderPassData
///
void ShaderRenderPassData::reset()
{
   Parent::reset();

   shader = NULL;

   for ( U32 i=0; i < featureShaderHandles.size(); i++ )
      delete featureShaderHandles[i];

   featureShaderHandles.clear();
}

String ShaderRenderPassData::describeSelf() const
{
   // First write the shader identification.
   String desc = String::ToString( "%s\n", shader->describeSelf().c_str() );

   // Let the parent get the rest.
   desc += Parent::describeSelf();

   return desc;
}

///
/// ProcessedShaderMaterial
///
ProcessedShaderMaterial::ProcessedShaderMaterial()
   :  mDefaultParameters( NULL ),
      mInstancingState( NULL )
{
   VECTOR_SET_ASSOCIATION( mShaderConstDesc );
   VECTOR_SET_ASSOCIATION( mParameterHandles );
}

ProcessedShaderMaterial::ProcessedShaderMaterial(Material &mat)
   :  mDefaultParameters( NULL ),
      mInstancingState( NULL )
{
   VECTOR_SET_ASSOCIATION( mShaderConstDesc );
   VECTOR_SET_ASSOCIATION( mParameterHandles );
   mMaterial = &mat;
}

ProcessedShaderMaterial::~ProcessedShaderMaterial()
{
   SAFE_DELETE(mInstancingState);
   SAFE_DELETE(mDefaultParameters);
   for (U32 i = 0; i < mParameterHandles.size(); i++)
      SAFE_DELETE(mParameterHandles[i]);
}

//
// Material init
//
bool ProcessedShaderMaterial::init( const FeatureSet &features, 
                                    const GFXVertexFormat *vertexFormat,
                                    const MatFeaturesDelegate &featuresDelegate )
{
   // Load our textures
   _setStageData();

   // Determine how many stages we use
   mMaxStages = getNumStages(); 
   mVertexFormat = vertexFormat;
   mFeatures.clear();
   mStateHint.clear();
   SAFE_DELETE(mInstancingState);

   for( U32 i=0; i<mMaxStages; i++ )
   {
      MaterialFeatureData fd;

      // Determine the features of this stage
      _determineFeatures( i, fd, features );
   
      // Let the delegate poke at the features.
      if ( featuresDelegate )
         featuresDelegate( this, i, fd, features );

      // Create the passes for this stage
      if ( fd.features.isNotEmpty() )
         if( !_createPasses( fd, i, features ) )
            return false;
   }

   _initRenderPassDataStateBlocks();
   _initMaterialParameters();
   mDefaultParameters =  allocMaterialParameters();
   setMaterialParameters( mDefaultParameters, 0 );
   mStateHint.init( this );   

   // Enable instancing if we have it.
   if ( mFeatures.hasFeature( MFT_UseInstancing ) )
   {
      mInstancingState = new InstancingState();
      mInstancingState->setFormat( _getRPD( 0 )->shader->getInstancingFormat(), mVertexFormat );
   }
   return true;
}

U32 ProcessedShaderMaterial::getNumStages()
{
   // Loops through all stages to determine how many 
   // stages we actually use.  
   // 
   // The first stage is always active else we shouldn't be
   // creating the material to begin with.
   U32 numStages = 1;

   U32 i;
   for( i=1; i<Material::MAX_STAGES; i++ )
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
      if ( mMaterial->mPixelSpecular[i] )
         stageActive = true;

      // If this stage has diffuse color, it's active
      if (  mMaterial->mDiffuse[i].alpha > 0 &&
            mMaterial->mDiffuse[i] != ColorF::WHITE )
         stageActive = true;

      // If we have a Material that is vertex lit
      // then it may not have a texture
      if( mMaterial->mVertLit[i] )
         stageActive = true;

      // Increment the number of active stages
      numStages += stageActive;
   }

   return numStages;
}

void ProcessedShaderMaterial::_determineFeatures(  U32 stageNum, 
                                                   MaterialFeatureData &fd, 
                                                   const FeatureSet &features )
{
   PROFILE_SCOPE( ProcessedShaderMaterial_DetermineFeatures );

   const F32 shaderVersion = GFX->getPixelShaderVersion();
   AssertFatal(shaderVersion > 0.0 , "Cannot create a shader material if we don't support shaders");

   bool lastStage = stageNum == (mMaxStages-1);

   // First we add all the features which the 
   // material has defined.

   if ( mMaterial->isTranslucent() )
   {
      // Note: This is for decal blending into the prepass
      // for AL... it probably needs to be made clearer.
      if (  mMaterial->mTranslucentBlendOp == Material::LerpAlpha &&
            mMaterial->mTranslucentZWrite )
         fd.features.addFeature( MFT_IsTranslucentZWrite );
      else
      {
         fd.features.addFeature( MFT_IsTranslucent );
         fd.features.addFeature( MFT_ForwardShading );
      }
   }

   // TODO: This sort of sucks... BL should somehow force this
   // feature on from the outside and not this way.
   if ( dStrcmp( LIGHTMGR->getId(), "BLM" ) == 0 )
      fd.features.addFeature( MFT_ForwardShading );

   // Disabling the InterlacedPrePass feature for now. It is not ready for prime-time
   // and it should not be triggered off of the DoubleSided parameter. [2/5/2010 Pat]
   /*if ( mMaterial->isDoubleSided() )
   {
      fd.features.addFeature( MFT_InterlacedPrePass );
   }*/

   // Allow instancing if it was requested and the card supports
   // SM 3.0 or above.
   //
   // We also disable instancing for non-single pass materials
   // and glowing materials because its untested/unimplemented.
   //
   if (  features.hasFeature( MFT_UseInstancing ) &&
         mMaxStages == 1 &&
         !mMaterial->mGlow[0] &&
         !mMaterial->mDynamicCubemap &&
         shaderVersion >= 3.0f )
      fd.features.addFeature( MFT_UseInstancing );

   if ( mMaterial->mAlphaTest )
      fd.features.addFeature( MFT_AlphaTest );

   if ( mMaterial->mEmissive[stageNum] )
      fd.features.addFeature( MFT_IsEmissive );
   else
      fd.features.addFeature( MFT_RTLighting );

   if ( mMaterial->mAnimFlags[stageNum] )
      fd.features.addFeature( MFT_TexAnim );  

   if ( mMaterial->mVertLit[stageNum] )
      fd.features.addFeature( MFT_VertLit );
   
   // cubemaps only available on stage 0 for now - bramage   
   if ( stageNum < 1 && mMaterial->isTranslucent() &&
         (  (  mMaterial->mCubemapData && mMaterial->mCubemapData->mCubemap ) ||
               mMaterial->mDynamicCubemap ) )
   {
       fd.features.addFeature( MFT_CubeMap );
   }

   if (features.hasFeature(MFT_SkyBox))
   {
      fd.features.addFeature(MFT_CubeMap);
      fd.features.addFeature(MFT_SkyBox);
   }
   fd.features.addFeature( MFT_Visibility );

   if (  lastStage && 
         (  !gClientSceneGraph->usePostEffectFog() ||
            fd.features.hasFeature( MFT_IsTranslucent ) ||
            fd.features.hasFeature( MFT_ForwardShading )) )
      fd.features.addFeature( MFT_Fog );

   if ( mMaterial->mMinnaertConstant[stageNum] > 0.0f )
      fd.features.addFeature( MFT_MinnaertShading );

   if ( mMaterial->mSubSurface[stageNum] )
      fd.features.addFeature( MFT_SubSurface );

   if ( !mMaterial->mCellLayout[stageNum].isZero() )
   {
      fd.features.addFeature( MFT_DiffuseMapAtlas );

      if ( mMaterial->mNormalMapAtlas )
         fd.features.addFeature( MFT_NormalMapAtlas );
   }

   // Grab other features like normal maps, base texture, etc.
   FeatureSet mergeFeatures;
   mStages[stageNum].getFeatureSet( &mergeFeatures );
   fd.features.merge( mergeFeatures );
   
   if ( fd.features[ MFT_NormalMap ] )   
   {   
      if (  mStages[stageNum].getTex( MFT_NormalMap )->mFormat == GFXFormatDXT5 &&   
           !mStages[stageNum].getTex( MFT_NormalMap )->mHasTransparency )   
         fd.features.addFeature( MFT_IsDXTnm );   
   }

   // Now for some more advanced features that we 
   // cannot do on SM 2.0 and below.
   if ( shaderVersion > 2.0f )
   {

      if (  mMaterial->mParallaxScale[stageNum] > 0.0f &&
         fd.features[ MFT_NormalMap ] )
         fd.features.addFeature( MFT_Parallax );

      // If not parallax then allow per-pixel specular if
      // we have real time lighting enabled.
      else if (   fd.features[MFT_RTLighting] && 
                  mMaterial->mPixelSpecular[stageNum] )
         fd.features.addFeature( MFT_PixSpecular );
   }

   // Without realtime lighting and on lower end 
   // shader models disable the specular map.
   if (  !fd.features[ MFT_RTLighting ] || shaderVersion == 2.0 )
      fd.features.removeFeature( MFT_SpecularMap );

   // If we have a specular map then make sure we
   // have per-pixel specular enabled.
   if( fd.features[ MFT_SpecularMap ] )
   {
      fd.features.addFeature( MFT_PixSpecular );

      // Check for an alpha channel on the specular map. If it has one (and it
      // has values less than 255) than the artist has put the gloss map into
      // the alpha channel.
      if( mStages[stageNum].getTex( MFT_SpecularMap )->mHasTransparency )
         fd.features.addFeature( MFT_GlossMap );
   }

   if ( mMaterial->mAccuEnabled[stageNum] )
   {
      mHasAccumulation = true;
   }

   // we need both diffuse and normal maps + sm3 to have an accu map
   if(   fd.features[ MFT_AccuMap ] && 
       ( !fd.features[ MFT_DiffuseMap ] || 
         !fd.features[ MFT_NormalMap ] ||
         GFX->getPixelShaderVersion() < 3.0f ) ) {
      AssertWarn(false, "SAHARA: Using an Accu Map requires SM 3.0 and a normal map.");
      fd.features.removeFeature( MFT_AccuMap );
      mHasAccumulation = false;
   }
   
   // Without a base texture use the diffuse color
   // feature to ensure some sort of output.
   if (!fd.features[MFT_DiffuseMap])
   {
      fd.features.addFeature( MFT_DiffuseColor );

      // No texture coords... no overlay.
      fd.features.removeFeature( MFT_OverlayMap );
   }

   // If we have a diffuse map and the alpha on the diffuse isn't
   // zero and the color isn't pure white then multiply the color.
   else if (   mMaterial->mDiffuse[stageNum].alpha > 0.0f && 
               mMaterial->mDiffuse[stageNum] != ColorF::WHITE )
      fd.features.addFeature( MFT_DiffuseColor );

   // If lightmaps or tonemaps are enabled or we 
   // don't have a second UV set then we cannot 
   // use the overlay texture.
   if (  fd.features[MFT_LightMap] || 
         fd.features[MFT_ToneMap] || 
         mVertexFormat->getTexCoordCount() < 2 )
      fd.features.removeFeature( MFT_OverlayMap );

   // If tonemaps are enabled don't use lightmap
   if ( fd.features[MFT_ToneMap] || mVertexFormat->getTexCoordCount() < 2 )
      fd.features.removeFeature( MFT_LightMap );

   // Don't allow tonemaps if we don't have a second UV set
   if ( mVertexFormat->getTexCoordCount() < 2 )
      fd.features.removeFeature( MFT_ToneMap );

   // Always add the HDR output feature.  
   //
   // It will be filtered out if it was disabled 
   // for this material creation below.
   //
   // Also the shader code will evaluate to a nop
   // if HDR is not enabled in the scene.
   //
   fd.features.addFeature( MFT_HDROut );

   // If vertex color is enabled on the material's stage and
   // color is present in vertex format, add diffuse vertex
   // color feature.
   
   if (  mMaterial->mVertColor[ stageNum ] &&
         mVertexFormat->hasColor() )
      fd.features.addFeature( MFT_DiffuseVertColor );

   // Allow features to add themselves.
   for ( U32 i = 0; i < FEATUREMGR->getFeatureCount(); i++ )
   {
      const FeatureInfo &info = FEATUREMGR->getAt( i );
      info.feature->determineFeature(  mMaterial, 
                                       mVertexFormat, 
                                       stageNum, 
                                       *info.type, 
                                       features, 
                                       &fd );
   }

   // Now disable any features that were 
   // not part of the input feature handle.
   fd.features.filter( features );
}

bool ProcessedShaderMaterial::_createPasses( MaterialFeatureData &stageFeatures, U32 stageNum, const FeatureSet &features )
{
   // Creates passes for the given stage
   ShaderRenderPassData passData;
   U32 texIndex = 0;

   for( U32 i=0; i < FEATUREMGR->getFeatureCount(); i++ )
   {
      const FeatureInfo &info = FEATUREMGR->getAt( i );
      if ( !stageFeatures.features.hasFeature( *info.type ) ) 
         continue;

      U32 numTexReg = info.feature->getResources( stageFeatures ).numTexReg;

      // adds pass if blend op changes for feature
      _setPassBlendOp( info.feature, passData, texIndex, stageFeatures, stageNum, features );

      // Add pass if num tex reg is going to be too high
      if( passData.mNumTexReg + numTexReg > GFX->getNumSamplers() )
      {
         if( !_addPass( passData, texIndex, stageFeatures, stageNum, features ) )
            return false;
         _setPassBlendOp( info.feature, passData, texIndex, stageFeatures, stageNum, features );
      }

      passData.mNumTexReg += numTexReg;
      passData.mFeatureData.features.addFeature( *info.type );

#if defined(TORQUE_DEBUG) && defined( TORQUE_OPENGL)
      U32 oldTexNumber = texIndex;
#endif

      info.feature->setTexData( mStages[stageNum], stageFeatures, passData, texIndex );

#if defined(TORQUE_DEBUG) && defined( TORQUE_OPENGL)
      if(oldTexNumber != texIndex)
      {
         for(int i = oldTexNumber; i < texIndex; i++)
         {
            AssertFatal(passData.mSamplerNames[ oldTexNumber ].isNotEmpty(), avar( "ERROR: ShaderGen feature %s don't set used sampler name", info.feature->getName().c_str()) );
         }
      }
#endif

      // Add pass if tex units are maxed out
      if( texIndex > GFX->getNumSamplers() )
      {
         if( !_addPass( passData, texIndex, stageFeatures, stageNum, features ) )
            return false;
         _setPassBlendOp( info.feature, passData, texIndex, stageFeatures, stageNum, features );
      }
   }

#if defined(TORQUE_DEBUG) && defined( TORQUE_OPENGL)
   for(int i = 0; i < texIndex; i++)
   {
      AssertFatal(passData.mSamplerNames[ i ].isNotEmpty(),"");
   }
#endif

   const FeatureSet &passFeatures = passData.mFeatureData.codify();
   if ( passFeatures.isNotEmpty() )
   {
      mFeatures.merge( passFeatures );
      if(  !_addPass( passData, texIndex, stageFeatures, stageNum, features ) )
      {
         mFeatures.clear();
         return false;
      }
   }

   return true;
} 

void ProcessedShaderMaterial::_initMaterialParameters()
{   
   // Cleanup anything left first.
   SAFE_DELETE( mDefaultParameters );
   for ( U32 i = 0; i < mParameterHandles.size(); i++ )
      SAFE_DELETE( mParameterHandles[i] );

   // Gather the shaders as they all need to be 
   // passed to the ShaderMaterialParameterHandles.
   Vector<GFXShader*> shaders;
   shaders.setSize( mPasses.size() );
   for ( U32 i = 0; i < mPasses.size(); i++ )
      shaders[i] = _getRPD(i)->shader;

   // Run through each shader and prepare its constants.
   for ( U32 i = 0; i < mPasses.size(); i++ )
   {
      const Vector<GFXShaderConstDesc>& desc = shaders[i]->getShaderConstDesc();

      Vector<GFXShaderConstDesc>::const_iterator p = desc.begin();
      for ( ; p != desc.end(); p++ )
      {
         // Add this to our list of shader constants
         GFXShaderConstDesc d(*p);
         mShaderConstDesc.push_back(d);

         ShaderMaterialParameterHandle* smph = new ShaderMaterialParameterHandle(d.name, shaders);
         mParameterHandles.push_back(smph);
      }
   }
}

bool ProcessedShaderMaterial::_addPass( ShaderRenderPassData &rpd, 
                                       U32 &texIndex, 
                                       MaterialFeatureData &fd,
                                       U32 stageNum,
                                       const FeatureSet &features )
{
   // Set number of textures, stage, glow, etc.
   rpd.mNumTex = texIndex;
   rpd.mStageNum = stageNum;
   rpd.mGlow |= mMaterial->mGlow[stageNum];

   // Copy over features
   rpd.mFeatureData.materialFeatures = fd.features;

   Vector<String> samplers;
   samplers.setSize(Material::MAX_TEX_PER_PASS);
   for(int i = 0; i < Material::MAX_TEX_PER_PASS; ++i)
   {
      samplers[i] = (rpd.mSamplerNames[i].isEmpty() || rpd.mSamplerNames[i][0] == '$') ? rpd.mSamplerNames[i] : "$" + rpd.mSamplerNames[i];
   }

   // Generate shader
   GFXShader::setLogging( true, true );
   rpd.shader = SHADERGEN->getShader( rpd.mFeatureData, mVertexFormat, &mUserMacros, samplers );
   if( !rpd.shader )
      return false;
   rpd.shaderHandles.init( rpd.shader );   

   // If a pass glows, we glow
   if( rpd.mGlow )
      mHasGlow = true;
 
   ShaderRenderPassData *newPass = new ShaderRenderPassData( rpd );
   mPasses.push_back( newPass );

   //initSamplerHandles
   ShaderConstHandles *handles = _getShaderConstHandles( mPasses.size()-1 );
   AssertFatal(handles,"");
   for(int i = 0; i < rpd.mNumTex; i++)
   { 
      if(rpd.mSamplerNames[i].isEmpty())
      {
         handles->mTexHandlesSC[i] = newPass->shader->getShaderConstHandle( String::EmptyString );
         handles->mRTParamsSC[i] = newPass->shader->getShaderConstHandle( String::EmptyString );
         continue;
      }

      String samplerName = rpd.mSamplerNames[i];
      if( !samplerName.startsWith("$"))
         samplerName.insert(0, "$");

      GFXShaderConstHandle *handle = newPass->shader->getShaderConstHandle( samplerName ); 

      handles->mTexHandlesSC[i] = handle;
      handles->mRTParamsSC[i] = newPass->shader->getShaderConstHandle( String::ToString( "$rtParams%s", samplerName.c_str()+1 ) ); 
      
      AssertFatal( handle,"");
   }

   // Give each active feature a chance to create specialized shader consts.
   for( U32 i=0; i < FEATUREMGR->getFeatureCount(); i++ )
   {
      const FeatureInfo &info = FEATUREMGR->getAt( i );
      if ( !fd.features.hasFeature( *info.type ) ) 
         continue;

      ShaderFeatureConstHandles *fh = info.feature->createConstHandles( rpd.shader, mUserObject );
      if ( fh )
         newPass->featureShaderHandles.push_back( fh );
   }

   rpd.reset();
   texIndex = 0;
   
   return true;
}

void ProcessedShaderMaterial::_setPassBlendOp( ShaderFeature *sf,
                                              ShaderRenderPassData &passData,
                                              U32 &texIndex,
                                              MaterialFeatureData &stageFeatures,
                                              U32 stageNum,
                                              const FeatureSet &features )
{
   if( sf->getBlendOp() == Material::None )
   {
      return;
   }

   // set up the current blend operation for multi-pass materials
   if( mPasses.size() > 0)
   {
      // If passData.numTexReg is 0, this is a brand new pass, so set the
      // blend operation to the first feature.
      if( passData.mNumTexReg == 0 )
      {
         passData.mBlendOp = sf->getBlendOp();
      }
      else
      {
         // numTegReg is more than zero, if this feature
         // doesn't have the same blend operation, then
         // we need to create yet another pass 
         if( sf->getBlendOp() != passData.mBlendOp && mPasses[mPasses.size()-1]->mStageNum == stageNum)
         {
            _addPass( passData, texIndex, stageFeatures, stageNum, features );
            passData.mBlendOp = sf->getBlendOp();
         }
      }
   }
} 

//
// Runtime / rendering
//
bool ProcessedShaderMaterial::setupPass( SceneRenderState *state, const SceneData &sgData, U32 pass )
{
   PROFILE_SCOPE( ProcessedShaderMaterial_SetupPass );

   // Make sure we have the pass
   if(pass >= mPasses.size())
   {
      // If we were rendering instanced data tell
      // the device to reset that vb stream.
      if ( mInstancingState )
         GFX->setVertexBuffer( NULL, 1 );

      return false;
   }

   _setRenderState( state, sgData, pass );

   // Set shaders
   ShaderRenderPassData* rpd = _getRPD(pass);
   if( rpd->shader )
   {
      GFX->setShader( rpd->shader );
      GFX->setShaderConstBuffer(_getShaderConstBuffer(pass));      
      _setShaderConstants(state, sgData, pass);      

      // If we're instancing then do the initial step to get
      // set the vb pointer to the const buffer.
      if ( mInstancingState )
         stepInstance();
   }
   else
   {
      GFX->setupGenericShaders();
      GFX->setShaderConstBuffer(NULL);
   } 

   // Set our textures
   setTextureStages( state, sgData, pass );
   _setTextureTransforms(pass);

   return true;
}

void ProcessedShaderMaterial::setTextureStages( SceneRenderState *state, const SceneData &sgData, U32 pass )
{
   PROFILE_SCOPE( ProcessedShaderMaterial_SetTextureStages );

   ShaderConstHandles *handles = _getShaderConstHandles(pass);
   AssertFatal(handles,"");

   // Set all of the textures we need to render the give pass.
#ifdef TORQUE_DEBUG
   AssertFatal( pass<mPasses.size(), "Pass out of bounds" );
#endif

   RenderPassData *rpd = mPasses[pass];
   GFXShaderConstBuffer* shaderConsts = _getShaderConstBuffer(pass);
   NamedTexTarget *texTarget;
   GFXTextureObject *texObject; 

   for( U32 i=0; i<rpd->mNumTex; i++ )
   {
      U32 currTexFlag = rpd->mTexType[i];
      if (!LIGHTMGR || !LIGHTMGR->setTextureStage(sgData, currTexFlag, i, shaderConsts, handles))
      {
         switch( currTexFlag )
         {
         // If the flag is unset then assume its just
         // a regular texture to set... nothing special.
         case 0:
         default:
            GFX->setTexture(i, rpd->mTexSlot[i].texObject);
            break;

         case Material::NormalizeCube:
            GFX->setCubeTexture(i, Material::GetNormalizeCube());
            break;

         case Material::Lightmap:
            GFX->setTexture( i, sgData.lightmap );
            break;

         case Material::ToneMapTex:
            shaderConsts->setSafe(handles->mToneMapTexSC, (S32)i);
            GFX->setTexture(i, rpd->mTexSlot[i].texObject);
            break;

         case Material::Cube:
            GFX->setCubeTexture( i, rpd->mCubeMap );
            break;

         case Material::SGCube:
            GFX->setCubeTexture( i, sgData.cubemap );
            break;

         case Material::BackBuff:
            GFX->setTexture( i, sgData.backBuffTex );
            break;

         case Material::AccuMap:
            if ( sgData.accuTex )
               GFX->setTexture( i, sgData.accuTex );
            else
               GFX->setTexture( i, GFXTexHandle::ZERO );
            break;
            
         case Material::TexTarget:
            {
               texTarget = rpd->mTexSlot[i].texTarget;
               if ( !texTarget )
               {
                  GFX->setTexture( i, NULL );
                  break;
               }
            
               texObject = texTarget->getTexture();

               // If no texture is available then map the default 2x2
               // black texture to it.  This at least will ensure that
               // we get consistant behavior across GPUs and platforms.
               if ( !texObject )
                  texObject = GFXTexHandle::ZERO;

               if ( handles->mRTParamsSC[i]->isValid() && texObject )
               {
                  const Point3I &targetSz = texObject->getSize();
                  const RectI &targetVp = texTarget->getViewport();
                  Point4F rtParams;

                  ScreenSpace::RenderTargetParameters(targetSz, targetVp, rtParams);

                  shaderConsts->set(handles->mRTParamsSC[i], rtParams);
               }

               GFX->setTexture( i, texObject );
               break;
            }
         }
      }
   }
}

void ProcessedShaderMaterial::_setTextureTransforms(const U32 pass)
{
   PROFILE_SCOPE( ProcessedShaderMaterial_SetTextureTransforms );

   ShaderConstHandles* handles = _getShaderConstHandles(pass);
   if (handles->mTexMatSC->isValid())
   {   
      MatrixF texMat( true );

      mMaterial->updateTimeBasedParams();
      F32 waveOffset = _getWaveOffset( pass ); // offset is between 0.0 and 1.0

      // handle scroll anim type
      if(  mMaterial->mAnimFlags[pass] & Material::Scroll )
      {
         if( mMaterial->mAnimFlags[pass] & Material::Wave )
         {
            Point3F scrollOffset;
            scrollOffset.x = mMaterial->mScrollDir[pass].x * waveOffset;
            scrollOffset.y = mMaterial->mScrollDir[pass].y * waveOffset;
            scrollOffset.z = 1.0;

            texMat.setColumn( 3, scrollOffset );
         }
         else
         {
            Point3F offset( mMaterial->mScrollOffset[pass].x, 
               mMaterial->mScrollOffset[pass].y, 
               1.0 );

            texMat.setColumn( 3, offset );
         }

      }

      // handle rotation
      if( mMaterial->mAnimFlags[pass] & Material::Rotate )
      {
         if( mMaterial->mAnimFlags[pass] & Material::Wave )
         {
            F32 rotPos = waveOffset * M_2PI;
            texMat.set( EulerF( 0.0, 0.0, rotPos ) );
            texMat.setColumn( 3, Point3F( 0.5, 0.5, 0.0 ) );

            MatrixF test( true );
            test.setColumn( 3, Point3F( mMaterial->mRotPivotOffset[pass].x, 
               mMaterial->mRotPivotOffset[pass].y,
               0.0 ) );
            texMat.mul( test );
         }
         else
         {
            texMat.set( EulerF( 0.0, 0.0, mMaterial->mRotPos[pass] ) );

            texMat.setColumn( 3, Point3F( 0.5, 0.5, 0.0 ) );

            MatrixF test( true );
            test.setColumn( 3, Point3F( mMaterial->mRotPivotOffset[pass].x, 
               mMaterial->mRotPivotOffset[pass].y,
               0.0 ) );
            texMat.mul( test );
         }
      }

      // Handle scale + wave offset
      if(  mMaterial->mAnimFlags[pass] & Material::Scale &&
         mMaterial->mAnimFlags[pass] & Material::Wave )
      {
         F32 wOffset = fabs( waveOffset );

         texMat.setColumn( 3, Point3F( 0.5, 0.5, 0.0 ) );

         MatrixF temp( true );
         temp.setRow( 0, Point3F( wOffset,  0.0,  0.0 ) );
         temp.setRow( 1, Point3F( 0.0,  wOffset,  0.0 ) );
         temp.setRow( 2, Point3F( 0.0,  0.0,  wOffset ) );
         temp.setColumn( 3, Point3F( -wOffset * 0.5, -wOffset * 0.5, 0.0 ) );
         texMat.mul( temp );
      }

      // handle sequence
      if( mMaterial->mAnimFlags[pass] & Material::Sequence )
      {
         U32 frameNum = (U32)(MATMGR->getTotalTime() * mMaterial->mSeqFramePerSec[pass]);
         F32 offset = frameNum * mMaterial->mSeqSegSize[pass];

         if ( mMaterial->mAnimFlags[pass] & Material::Scale )
            texMat.scale( Point3F( mMaterial->mSeqSegSize[pass], 1.0f, 1.0f ) );

         Point3F texOffset = texMat.getPosition();
         texOffset.x += offset;
         texMat.setPosition( texOffset );
      }

      GFXShaderConstBuffer* shaderConsts = _getShaderConstBuffer(pass);
      shaderConsts->setSafe(handles->mTexMatSC, texMat);
   }
}

//--------------------------------------------------------------------------
// Get wave offset for texture animations using a wave transform
//--------------------------------------------------------------------------
F32 ProcessedShaderMaterial::_getWaveOffset( U32 stage )
{
   switch( mMaterial->mWaveType[stage] )
   {
   case Material::Sin:
      {
         return mMaterial->mWaveAmp[stage] * mSin( M_2PI * mMaterial->mWavePos[stage] );
         break;
      }

   case Material::Triangle:
      {
         F32 frac = mMaterial->mWavePos[stage] - mFloor( mMaterial->mWavePos[stage] );
         if( frac > 0.0 && frac <= 0.25 )
         {
            return mMaterial->mWaveAmp[stage] * frac * 4.0;
         }

         if( frac > 0.25 && frac <= 0.5 )
         {
            return mMaterial->mWaveAmp[stage] * ( 1.0 - ((frac-0.25)*4.0) );
         }

         if( frac > 0.5 && frac <= 0.75 )
         {
            return mMaterial->mWaveAmp[stage] * (frac-0.5) * -4.0;
         }

         if( frac > 0.75 && frac <= 1.0 )
         {
            return -mMaterial->mWaveAmp[stage] * ( 1.0 - ((frac-0.75)*4.0) );
         }

         break;
      }

   case Material::Square:
      {
         F32 frac = mMaterial->mWavePos[stage] - mFloor( mMaterial->mWavePos[stage] );
         if( frac > 0.0 && frac <= 0.5 )
         {
            return 0.0;
         }
         else
         {
            return mMaterial->mWaveAmp[stage];
         }
         break;
      }

   }

   return 0.0;
}

void ProcessedShaderMaterial::_setShaderConstants(SceneRenderState * state, const SceneData &sgData, U32 pass)
{
   PROFILE_SCOPE( ProcessedShaderMaterial_SetShaderConstants );

   GFXShaderConstBuffer* shaderConsts = _getShaderConstBuffer(pass);
   ShaderConstHandles* handles = _getShaderConstHandles(pass);
   U32 stageNum = getStageFromPass(pass);

   // First we do all the constants which are not
   // controlled via the material... we have to
   // set these all the time as they could change.

   if ( handles->mFogDataSC->isValid() )
   {
      Point3F fogData;
      fogData.x = sgData.fogDensity;
      fogData.y = sgData.fogDensityOffset;
      fogData.z = sgData.fogHeightFalloff;     
      shaderConsts->set( handles->mFogDataSC, fogData );
   }

   shaderConsts->setSafe(handles->mFogColorSC, sgData.fogColor);

   if( handles->mOneOverFarplane->isValid() )
   {
      const F32 &invfp = 1.0f / state->getFarPlane();
      Point4F oneOverFP(invfp, invfp, invfp, invfp);
      shaderConsts->set( handles->mOneOverFarplane, oneOverFP );
   }

   shaderConsts->setSafe( handles->mAccumTimeSC, MATMGR->getTotalTime() );

   // If the shader constants have not been lost then
   // they contain the content from a previous render pass.
   //
   // In this case we can skip updating the material constants
   // which do not change frame to frame.
   //
   // NOTE: This assumes we're not animating material parameters
   // in a way that doesn't cause a shader reload... this isn't
   // being done now, but it could change in the future.
   // 
   if ( !shaderConsts->wasLost() )
      return;

   shaderConsts->setSafe(handles->mSpecularColorSC, mMaterial->mSpecular[stageNum]);   
   shaderConsts->setSafe(handles->mSpecularPowerSC, mMaterial->mSpecularPower[stageNum]);
   shaderConsts->setSafe(handles->mSpecularStrengthSC, mMaterial->mSpecularStrength[stageNum]);

   shaderConsts->setSafe(handles->mParallaxInfoSC, mMaterial->mParallaxScale[stageNum]);   
   shaderConsts->setSafe(handles->mMinnaertConstantSC, mMaterial->mMinnaertConstant[stageNum]);

   if ( handles->mSubSurfaceParamsSC->isValid() )
   {
      Point4F subSurfParams;
      dMemcpy( &subSurfParams, &mMaterial->mSubSurfaceColor[stageNum], sizeof(ColorF) );
      subSurfParams.w = mMaterial->mSubSurfaceRolloff[stageNum];
      shaderConsts->set(handles->mSubSurfaceParamsSC, subSurfParams);
   }

   if ( handles->mRTSizeSC->isValid() )
   {
      const Point2I &resolution = GFX->getActiveRenderTarget()->getSize();
      Point2F pixelShaderConstantData;

      pixelShaderConstantData.x = resolution.x;
      pixelShaderConstantData.y = resolution.y;

      shaderConsts->set( handles->mRTSizeSC, pixelShaderConstantData );
   }

   if ( handles->mOneOverRTSizeSC->isValid() )
   {
      const Point2I &resolution = GFX->getActiveRenderTarget()->getSize();
      Point2F oneOverTargetSize( 1.0f / (F32)resolution.x, 1.0f / (F32)resolution.y );

      shaderConsts->set( handles->mOneOverRTSizeSC, oneOverTargetSize );
   }

   // set detail scale
   shaderConsts->setSafe(handles->mDetailScaleSC, mMaterial->mDetailScale[stageNum]);
   shaderConsts->setSafe(handles->mDetailBumpStrength, mMaterial->mDetailNormalMapStrength[stageNum]);

   // MFT_ImposterVert
   if ( handles->mImposterUVs->isValid() )
   {
      U32 uvCount = getMin( mMaterial->mImposterUVs.size(), 64 ); // See imposter.hlsl   
      AlignedArray<Point4F> imposterUVs( uvCount, sizeof( Point4F ), (U8*)mMaterial->mImposterUVs.address(), false );
      shaderConsts->set( handles->mImposterUVs, imposterUVs );
   }
   shaderConsts->setSafe( handles->mImposterLimits, mMaterial->mImposterLimits );

   // Diffuse
   shaderConsts->setSafe(handles->mDiffuseColorSC, mMaterial->mDiffuse[stageNum]);

   shaderConsts->setSafe( handles->mAlphaTestValueSC, mClampF( (F32)mMaterial->mAlphaRef / 255.0f, 0.0f, 1.0f ) );      

   if(handles->mDiffuseAtlasParamsSC)
   {
      Point4F atlasParams(1.0f / mMaterial->mCellLayout[stageNum].x, // 1 / num_horizontal
         1.0f / mMaterial->mCellLayout[stageNum].y, // 1 / num_vertical
         mMaterial->mCellSize[stageNum],            // tile size in pixels
         getBinLog2(mMaterial->mCellSize[stageNum]) );    // pow of 2 of tile size in pixels 2^9 = 512, 2^10=1024 etc
      shaderConsts->setSafe(handles->mDiffuseAtlasParamsSC, atlasParams);
   }

   if(handles->mBumpAtlasParamsSC)
   {
      Point4F atlasParams(1.0f / mMaterial->mCellLayout[stageNum].x, // 1 / num_horizontal
         1.0f / mMaterial->mCellLayout[stageNum].y, // 1 / num_vertical
         mMaterial->mCellSize[stageNum],            // tile size in pixels
         getBinLog2(mMaterial->mCellSize[stageNum]) );    // pow of 2 of tile size in pixels 2^9 = 512, 2^10=1024 etc
      shaderConsts->setSafe(handles->mBumpAtlasParamsSC, atlasParams);
   }

   if(handles->mDiffuseAtlasTileSC)
   {
      // Sanity check the wrap flags
      //AssertWarn(mMaterial->mTextureAddressModeU == mMaterial->mTextureAddressModeV, "Addresing mode mismatch, texture atlasing will be confused");
      Point4F atlasTileParams( mMaterial->mCellIndex[stageNum].x, // Tile co-ordinate, ie: [0, 3]
         mMaterial->mCellIndex[stageNum].y, 
         0.0f, 0.0f ); // TODO: Wrap mode flags?
      shaderConsts->setSafe(handles->mDiffuseAtlasTileSC, atlasTileParams);
   }

   if(handles->mBumpAtlasTileSC)
   {
      // Sanity check the wrap flags
      //AssertWarn(mMaterial->mTextureAddressModeU == mMaterial->mTextureAddressModeV, "Addresing mode mismatch, texture atlasing will be confused");
      Point4F atlasTileParams( mMaterial->mCellIndex[stageNum].x, // Tile co-ordinate, ie: [0, 3]
         mMaterial->mCellIndex[stageNum].y, 
         0.0f, 0.0f ); // TODO: Wrap mode flags?
      shaderConsts->setSafe(handles->mBumpAtlasTileSC, atlasTileParams);
   }

   // Deferred Shading: Determine Material Info Flags
   S32 matInfoFlags = 
            (mMaterial->mEmissive[stageNum] ? 1 : 0);
   mMaterial->mMatInfoFlags[stageNum] = matInfoFlags / 255.0f;
   shaderConsts->setSafe(handles->mMatInfoFlagsSC, mMaterial->mMatInfoFlags[stageNum]);   
   if( handles->mAccuScaleSC->isValid() )
      shaderConsts->set( handles->mAccuScaleSC, mMaterial->mAccuScale[stageNum] );
   if( handles->mAccuDirectionSC->isValid() )
      shaderConsts->set( handles->mAccuDirectionSC, mMaterial->mAccuDirection[stageNum] );
   if( handles->mAccuStrengthSC->isValid() )
      shaderConsts->set( handles->mAccuStrengthSC, mMaterial->mAccuStrength[stageNum] );
   if( handles->mAccuCoverageSC->isValid() )
      shaderConsts->set( handles->mAccuCoverageSC, mMaterial->mAccuCoverage[stageNum] );
   if( handles->mAccuSpecularSC->isValid() )
      shaderConsts->set( handles->mAccuSpecularSC, mMaterial->mAccuSpecular[stageNum] );
}

bool ProcessedShaderMaterial::_hasCubemap(U32 pass)
{
   // Only support cubemap on the first stage
   if( mPasses[pass]->mStageNum > 0 )
      return false;

   if( mPasses[pass]->mCubeMap )
      return true;

   return false;
}

void ProcessedShaderMaterial::setTransforms(const MatrixSet &matrixSet, SceneRenderState *state, const U32 pass)
{
   PROFILE_SCOPE( ProcessedShaderMaterial_setTransforms );

   GFXShaderConstBuffer* shaderConsts = _getShaderConstBuffer(pass);
   ShaderConstHandles* handles = _getShaderConstHandles(pass);

   // The MatrixSet will lazily generate a matrix under the
   // various 'get' methods, so inline the test for a valid
   // shader constant handle to avoid that work when we can.
   if ( handles->mModelViewProjSC->isValid() )
      shaderConsts->set( handles->mModelViewProjSC, matrixSet.getWorldViewProjection() );
   if ( handles->mObjTransSC->isValid() )
      shaderConsts->set( handles->mObjTransSC, matrixSet.getObjectToWorld() );      
   if ( handles->mWorldToObjSC->isValid() )
      shaderConsts->set( handles->mWorldToObjSC, matrixSet.getWorldToObject() );
   if ( handles->mWorldToCameraSC->isValid() )
      shaderConsts->set( handles->mWorldToCameraSC, matrixSet.getWorldToCamera() );
   if ( handles->mWorldViewOnlySC->isValid() )
      shaderConsts->set( handles->mWorldViewOnlySC, matrixSet.getObjectToCamera() );
   if ( handles->mViewToObjSC->isValid() )
      shaderConsts->set( handles->mViewToObjSC, matrixSet.getCameraToObject() );
   if ( handles->mViewProjSC->isValid() )
      shaderConsts->set( handles->mViewProjSC, matrixSet.getWorldToScreen() );

   if (  handles->mCubeTransSC->isValid() &&
         ( _hasCubemap(pass) || mMaterial->mDynamicCubemap ) )
   {
      // TODO: Could we not remove this constant?  Use mObjTransSC and cast to float3x3 instead?
      shaderConsts->set(handles->mCubeTransSC, matrixSet.getObjectToWorld(), GFXSCT_Float3x3);
   }

   if ( handles->m_vEyeSC->isValid() )
      shaderConsts->set( handles->m_vEyeSC, state->getVectorEye() );
}

void ProcessedShaderMaterial::setSceneInfo(SceneRenderState * state, const SceneData& sgData, U32 pass)
{
   PROFILE_SCOPE( ProcessedShaderMaterial_setSceneInfo );

   GFXShaderConstBuffer* shaderConsts = _getShaderConstBuffer(pass);
   ShaderConstHandles* handles = _getShaderConstHandles(pass);

   // Set cubemap stuff here (it's convenient!)
   const Point3F &eyePosWorld = state->getCameraPosition();
   if ( handles->mCubeEyePosSC->isValid() )
   {
      if(_hasCubemap(pass) || mMaterial->mDynamicCubemap)
      {
         Point3F cubeEyePos = eyePosWorld - sgData.objTrans->getPosition();
         shaderConsts->set(handles->mCubeEyePosSC, cubeEyePos);      
      }
   }

   shaderConsts->setSafe(handles->mVisiblitySC, sgData.visibility);

   shaderConsts->setSafe(handles->mEyePosWorldSC, eyePosWorld);   

   if ( handles->mEyePosSC->isValid() )
   {
      MatrixF tempMat( *sgData.objTrans );
      tempMat.inverse();
      Point3F eyepos;
      tempMat.mulP( eyePosWorld, &eyepos );
      shaderConsts->set(handles->mEyePosSC, eyepos);   
   }

   shaderConsts->setSafe(handles->mEyeMatSC, state->getCameraTransform());   

   ShaderRenderPassData *rpd = _getRPD( pass );
   for ( U32 i=0; i < rpd->featureShaderHandles.size(); i++ )
      rpd->featureShaderHandles[i]->setConsts( state, sgData, shaderConsts );

   LIGHTMGR->setLightInfo( this, mMaterial, sgData, state, pass, shaderConsts );
}

void ProcessedShaderMaterial::setBuffers( GFXVertexBufferHandleBase *vertBuffer, GFXPrimitiveBufferHandle *primBuffer )
{
   PROFILE_SCOPE(ProcessedShaderMaterial_setBuffers);

   // If we're not instanced then just call the parent.
   if ( !mInstancingState )
   {
      Parent::setBuffers( vertBuffer, primBuffer );
      return;
   }

   PROFILE_SCOPE(ProcessedShaderMaterial_setBuffers_instancing);

   const S32 instCount = mInstancingState->getCount();
   AssertFatal( instCount > 0,
      "ProcessedShaderMaterial::setBuffers - No instances rendered!" );

   // Nothing special here.
   GFX->setPrimitiveBuffer( *primBuffer );

   // Set the first stream the the normal VB and set the
   // correct frequency for the number of instances to render.
   GFX->setVertexBuffer( *vertBuffer, 0, instCount );

   // Get a volatile VB and fill it with the vertex data.
   const GFXVertexFormat *instFormat = mInstancingState->getFormat();
   GFXVertexBufferDataHandle instVB;
   instVB.set( GFX, instFormat->getSizeInBytes(), instFormat, instCount, GFXBufferTypeVolatile );
   U8 *dest = instVB.lock();
   if(!dest) return;
   dMemcpy( dest, mInstancingState->getBuffer(), instFormat->getSizeInBytes() * instCount );
   instVB.unlock();

   // Set the instance vb for streaming.
   GFX->setVertexBuffer( instVB, 1, 1 );

   // Finally set the vertex format which defines
   // both of the streams.
   GFX->setVertexFormat( mInstancingState->getDeclFormat() );

   // Done... reset the count.
   mInstancingState->resetStep();
}

bool ProcessedShaderMaterial::stepInstance()
{
   PROFILE_SCOPE(ProcessedShaderMaterial_stepInstance);
   AssertFatal( mInstancingState, "ProcessedShaderMaterial::stepInstance - This material isn't instanced!" );  
   return mInstancingState->step( &_getShaderConstBuffer( 0 )->mInstPtr );
}

MaterialParameters* ProcessedShaderMaterial::allocMaterialParameters()
{
   ShaderMaterialParameters* smp = new ShaderMaterialParameters();
   Vector<GFXShaderConstBufferRef> buffers( __FILE__, __LINE__ );
   buffers.setSize(mPasses.size());
   for (U32 i = 0; i < mPasses.size(); i++)
      buffers[i] = _getRPD(i)->shader->allocConstBuffer();
   // smp now owns these buffers.
   smp->setBuffers(mShaderConstDesc, buffers);
   return smp;   
}

MaterialParameterHandle* ProcessedShaderMaterial::getMaterialParameterHandle(const String& name)
{
   // Search our list
   for (U32 i = 0; i < mParameterHandles.size(); i++)
   {
      if (mParameterHandles[i]->getName().equal(name))
         return mParameterHandles[i];
   }
   
   // If we didn't find it, we have to add it to support shader reloading.

   Vector<GFXShader*> shaders;
   shaders.setSize(mPasses.size());
   for (U32 i = 0; i < mPasses.size(); i++)
      shaders[i] = _getRPD(i)->shader;

   ShaderMaterialParameterHandle* smph = new ShaderMaterialParameterHandle( name, shaders );
   mParameterHandles.push_back(smph);

   return smph;
}

/// This is here to deal with the differences between ProcessedCustomMaterials and ProcessedShaderMaterials.
GFXShaderConstBuffer* ProcessedShaderMaterial::_getShaderConstBuffer( const U32 pass )
{   
   if (mCurrentParams && pass < mPasses.size())
   {
      return static_cast<ShaderMaterialParameters*>(mCurrentParams)->getBuffer(pass);
   }
   return NULL;
}

ShaderConstHandles* ProcessedShaderMaterial::_getShaderConstHandles(const U32 pass)
{
   if (pass < mPasses.size())
   {
      return &_getRPD(pass)->shaderHandles;
   }
   return NULL;
}

void ProcessedShaderMaterial::dumpMaterialInfo()
{
   for ( U32 i = 0; i < getNumPasses(); i++ )
   {
      const ShaderRenderPassData *passData = _getRPD( i );

      if ( passData == NULL )
         continue;

      const GFXShader      *shader = passData->shader;

      if ( shader == NULL )
         Con::printf( "  [%i] [NULL shader]", i );
      else
         Con::printf( "  [%i] %s", i, shader->describeSelf().c_str() );
   }
}
