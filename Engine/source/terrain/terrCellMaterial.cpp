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
#include "terrain/terrCellMaterial.h"

#include "terrain/terrData.h"
#include "terrain/terrCell.h"
#include "materials/materialFeatureTypes.h"
#include "materials/materialManager.h"
#include "terrain/terrFeatureTypes.h"
#include "terrain/terrMaterial.h"
#include "renderInstance/renderPrePassMgr.h"
#include "shaderGen/shaderGen.h"
#include "shaderGen/featureMgr.h"
#include "scene/sceneRenderState.h"
#include "materials/sceneData.h"
#include "gfx/util/screenspace.h"
#include "lighting/advanced/advancedLightBinManager.h"

S32 sgMaxTerrainMaterialsPerPass = 3;

AFTER_MODULE_INIT( MaterialManager )
{
   Con::NotifyDelegate callabck( &TerrainCellMaterial::_updateDefaultAnisotropy );
   Con::addVariableNotify( "$pref::Video::defaultAnisotropy", callabck );
}

Vector<TerrainCellMaterial*> TerrainCellMaterial::smAllMaterials;

Vector<String> _initSamplerNames()
{
   Vector<String> samplerNames;
   samplerNames.push_back("$baseTexMap");
   samplerNames.push_back("$layerTex");   
   samplerNames.push_back("$macrolayerTex");   
   samplerNames.push_back("$lightMapTex");
   samplerNames.push_back("$lightInfoBuffer");
   for(int i = 0; i < 3; ++i)
   {
      samplerNames.push_back(avar("$normalMap%d",i));
      samplerNames.push_back(avar("$detailMap%d",i));
      samplerNames.push_back(avar("$macroMap%d",i));
   }   

   return samplerNames;
}


const Vector<String> TerrainCellMaterial::mSamplerNames = _initSamplerNames();

TerrainCellMaterial::TerrainCellMaterial()
   :  mTerrain( NULL ),
      mCurrPass( 0 ),
      mPrePassMat( NULL ),
      mReflectMat( NULL )
{
   smAllMaterials.push_back( this );
}

TerrainCellMaterial::~TerrainCellMaterial()
{
   SAFE_DELETE( mPrePassMat );
   SAFE_DELETE( mReflectMat );   
   smAllMaterials.remove( this );
}

void TerrainCellMaterial::_updateDefaultAnisotropy()
{
   // TODO: We need to split the stateblock initialization
   // from the shader constant lookup and pass setup in a 
   // future version of terrain materials.
   //
   // For now use some custom code in a horrible loop to 
   // change the anisotropy directly and fast.
   //

   const U32 maxAnisotropy = MATMGR->getDefaultAnisotropy();

   Vector<TerrainCellMaterial*>::iterator iter = smAllMaterials.begin();
   for ( ; iter != smAllMaterials.end(); iter++ )
   {
      for ( U32 p=0; p < (*iter)->mPasses.size(); p++ )
      {
         Pass &pass = (*iter)->mPasses[p];

         // Start from the existing state block.
         GFXStateBlockDesc desc = pass.stateBlock->getDesc();

         for ( U32 m=0; m < pass.materials.size(); m++ )
         {
            const MaterialInfo *matInfo = pass.materials[m];

            if ( matInfo->detailTexConst->isValid() )
            {
               const S32 sampler = matInfo->detailTexConst->getSamplerRegister();

               if ( maxAnisotropy > 1 )
               {
                  desc.samplers[sampler].minFilter = GFXTextureFilterAnisotropic;
                  desc.samplers[sampler].maxAnisotropy = maxAnisotropy;
               }
               else
                  desc.samplers[sampler].minFilter = GFXTextureFilterLinear;
            }

            if ( matInfo->macroTexConst->isValid() )
            {
               const S32 sampler = matInfo->macroTexConst->getSamplerRegister();

               if ( maxAnisotropy > 1 )
               {
                  desc.samplers[sampler].minFilter = GFXTextureFilterAnisotropic;
                  desc.samplers[sampler].maxAnisotropy = maxAnisotropy;
               }
               else
                  desc.samplers[sampler].minFilter = GFXTextureFilterLinear;
            }

            if ( matInfo->normalTexConst->isValid() )
            {
               const S32 sampler = matInfo->normalTexConst->getSamplerRegister();

               if ( maxAnisotropy > 1 )
               {
                  desc.samplers[sampler].minFilter = GFXTextureFilterAnisotropic;
                  desc.samplers[sampler].maxAnisotropy = maxAnisotropy;
               }
               else
                  desc.samplers[sampler].minFilter = GFXTextureFilterLinear;
            }

         } // for ( U32 m=0; m < pass.materials.size(); m++ )

         // Set the updated stateblock.
         desc.setCullMode( GFXCullCCW );
         pass.stateBlock = GFX->createStateBlock( desc );

         //reflection
         desc.setCullMode( GFXCullCW );
         pass.reflectionStateBlock = GFX->createStateBlock(desc);

         // Create the wireframe state blocks.
         GFXStateBlockDesc wireframe( desc );
         wireframe.fillMode = GFXFillWireframe;
         wireframe.setCullMode( GFXCullCCW );
         pass.wireframeStateBlock = GFX->createStateBlock( wireframe );

      } // for ( U32 p=0; i < (*iter)->mPasses.size(); p++ )
   }

}

void TerrainCellMaterial::setTransformAndEye(   const MatrixF &modelXfm, 
                                                const MatrixF &viewXfm,
                                                const MatrixF &projectXfm,
                                                F32 farPlane )
{
   PROFILE_SCOPE( TerrainCellMaterial_SetTransformAndEye );

   MatrixF modelViewProj = projectXfm * viewXfm * modelXfm;
  
   MatrixF invViewXfm( viewXfm );
   invViewXfm.inverse();
   Point3F eyePos = invViewXfm.getPosition();
   
   MatrixF invModelXfm( modelXfm );
   invModelXfm.inverse();

   Point3F objEyePos = eyePos;
   invModelXfm.mulP( objEyePos );
   
   VectorF vEye = invViewXfm.getForwardVector();
   vEye.normalize( 1.0f / farPlane );

   for ( U32 i=0; i < mPasses.size(); i++ )
   {
      Pass &pass = mPasses[i];

      pass.consts->setSafe( pass.modelViewProjConst, modelViewProj );

      if( pass.viewToObj->isValid() || pass.worldViewOnly->isValid() )
      {
         MatrixF worldViewOnly = viewXfm * modelXfm;

         pass.consts->setSafe( pass.worldViewOnly, worldViewOnly );

         if( pass.viewToObj->isValid() )
         {
            worldViewOnly.affineInverse();
            pass.consts->set( pass.viewToObj, worldViewOnly);
         } 
      }

      pass.consts->setSafe( pass.eyePosWorldConst, eyePos );
      pass.consts->setSafe( pass.eyePosConst, objEyePos );
      pass.consts->setSafe( pass.objTransConst, modelXfm );
      pass.consts->setSafe( pass.worldToObjConst, invModelXfm );
      pass.consts->setSafe( pass.vEyeConst, vEye );
   }
}

TerrainCellMaterial* TerrainCellMaterial::getPrePassMat()
{
   if ( !mPrePassMat )
   {
      mPrePassMat = new TerrainCellMaterial();
      mPrePassMat->init( mTerrain, mMaterials, true, false, mMaterials == 0 );
   }

   return mPrePassMat;
}

TerrainCellMaterial* TerrainCellMaterial::getReflectMat()
{
   if ( !mReflectMat )
   {
      mReflectMat = new TerrainCellMaterial();
      mReflectMat->init( mTerrain, mMaterials, false, true, true );
   }

   return mReflectMat;
}

void TerrainCellMaterial::init(  TerrainBlock *block,
                                 U64 activeMaterials, 
                                 bool prePassMat,
                                 bool reflectMat,
                                 bool baseOnly )
{
   // This isn't allowed for now.
   AssertFatal( !( prePassMat && reflectMat ), "TerrainCellMaterial::init - We shouldn't get prepass and reflection in the same material!" );

   mTerrain = block;
   mMaterials = activeMaterials;

   Vector<MaterialInfo*> materials;

   for ( U32 i = 0; i < 64; i++ )
   {
      if ( !( mMaterials & ((U64)1 << i ) ) )
         continue;

      TerrainMaterial *mat = block->getMaterial( i );

      MaterialInfo *info = new MaterialInfo();
      info->layerId = i;
      info->mat = mat;
      materials.push_back( info );
   }

   mCurrPass = 0;
   mPasses.clear();

   // Ok... loop till we successfully generate all 
   // the shader passes for the materials.
   while ( materials.size() > 0 || baseOnly )
   {
      mPasses.increment();

      if ( !_createPass(   &materials, 
                           &mPasses.last(), 
                           mPasses.size() == 1, 
                           prePassMat,
                           reflectMat,
                           baseOnly ) )
      {
         Con::errorf( "TerrainCellMaterial::init - Failed to create pass!" );

         // The pass failed to be generated... give up.
         mPasses.last().materials.clear();
         mPasses.clear();
         for_each( materials.begin(), materials.end(), delete_pointer() );
         return;
      }

      if ( baseOnly )
         break;
   }

   // Cleanup any remaining matinfo.
   for_each( materials.begin(), materials.end(), delete_pointer() );

   // If we have attached mats then update them too.
   if ( mPrePassMat )
      mPrePassMat->init( mTerrain, mMaterials, true, false, baseOnly );
   if ( mReflectMat )
      mReflectMat->init( mTerrain, mMaterials, false, true, baseOnly );
}

bool TerrainCellMaterial::_createPass( Vector<MaterialInfo*> *materials, 
                                       Pass *pass, 
                                       bool firstPass,
                                       bool prePassMat,
                                       bool reflectMat,
                                       bool baseOnly )
{
   if ( GFX->getPixelShaderVersion() < 3.0f )
      baseOnly = true;

   // NOTE: At maximum we only try to combine sgMaxTerrainMaterialsPerPass materials 
   // into a single pass.  This is sub-optimal for the simplest
   // cases, but the most common case results in much fewer
   // shader generation failures and permutations leading to
   // faster load time and less hiccups during gameplay.
   U32 matCount = getMin( sgMaxTerrainMaterialsPerPass, materials->size() );

   Vector<GFXTexHandle> normalMaps;

   // See if we're currently running under the
   // basic lighting manager.
   //
   // TODO: This seems ugly... we should trigger
   // features like this differently in the future.
   //
   bool useBLM = dStrcmp( LIGHTMGR->getId(), "BLM" ) == 0;

   // Do we need to disable normal mapping?
   const bool disableNormalMaps = MATMGR->getExclusionFeatures().hasFeature( MFT_NormalMap ) || useBLM;

   // How about parallax?
   const bool disableParallaxMaps = GFX->getPixelShaderVersion() < 3.0f || 
                                    MATMGR->getExclusionFeatures().hasFeature( MFT_Parallax );

   // Has advanced lightmap support been enabled for prepass.
   bool advancedLightmapSupport = false;
   if ( prePassMat )
   {
      // This sucks... but it works.
      AdvancedLightBinManager *lightBin;
      if ( Sim::findObject( "AL_LightBinMgr", lightBin ) )
         advancedLightmapSupport = lightBin->MRTLightmapsDuringPrePass();
   }

   // Loop till we create a valid shader!
   while( true )
   {
      FeatureSet features;
      features.addFeature( MFT_VertTransform );

      if ( prePassMat )
      {
         features.addFeature( MFT_EyeSpaceDepthOut );
         features.addFeature( MFT_PrePassConditioner );
         features.addFeature( MFT_DeferredTerrainBaseMap );
         features.addFeature(MFT_isDeferred);

         if ( advancedLightmapSupport )
            features.addFeature( MFT_RenderTarget3_Zero );
      }
      else
      {
         features.addFeature( MFT_TerrainBaseMap );
         features.addFeature( MFT_RTLighting );

         // The HDR feature is always added... it will compile out
         // if HDR is not enabled in the engine.
         features.addFeature( MFT_HDROut );
      }
      features.addFeature(MFT_DeferredTerrainBlankInfoMap);

      // Enable lightmaps and fogging if we're in BL.
      if ( reflectMat || useBLM )
      {
         features.addFeature( MFT_Fog );
         features.addFeature( MFT_ForwardShading );
      }
      if ( useBLM )
         features.addFeature( MFT_TerrainLightMap );

      // The additional passes need to be lerp blended into the
      // target to maintain the results of the previous passes.
      if ( !firstPass )
         features.addFeature( MFT_TerrainAdditive );

      normalMaps.clear();
      pass->materials.clear();

      // Now add all the material layer features.

      for ( U32 i=0; i < matCount && !baseOnly; i++ )
      {
         TerrainMaterial *mat = (*materials)[i]->mat;

         if ( mat == NULL )
            continue;

         // We only include materials that 
         // have more than a base texture.
         if (  mat->getDetailSize() <= 0 ||
               mat->getDetailDistance() <= 0 ||
               mat->getDetailMap().isEmpty() )
            continue;         

         S32 featureIndex = pass->materials.size();

		 // check for macro detail texture
         if (  !(mat->getMacroSize() <= 0 || mat->getMacroDistance() <= 0 || mat->getMacroMap().isEmpty() ) )
         {
            if(prePassMat)
               features.addFeature( MFT_DeferredTerrainMacroMap, featureIndex );
            else
	         features.addFeature( MFT_TerrainMacroMap, featureIndex );
         }

         if(prePassMat)
             features.addFeature( MFT_DeferredTerrainDetailMap, featureIndex );
         else
         features.addFeature( MFT_TerrainDetailMap, featureIndex );

         pass->materials.push_back( (*materials)[i] );
         normalMaps.increment();

         // Skip normal maps if we need to.
         if ( !disableNormalMaps && mat->getNormalMap().isNotEmpty() )
         {
            features.addFeature( MFT_TerrainNormalMap, featureIndex );

            normalMaps.last().set( mat->getNormalMap(), 
               &GFXDefaultStaticNormalMapProfile, "TerrainCellMaterial::_createPass() - NormalMap" );

            if ( normalMaps.last().getFormat() == GFXFormatDXT5 )
               features.addFeature( MFT_IsDXTnm, featureIndex );

            // Do we need and can we do parallax mapping?
            if (  !disableParallaxMaps &&
                  mat->getParallaxScale() > 0.0f &&
                  !mat->useSideProjection() )
               features.addFeature( MFT_TerrainParallaxMap, featureIndex );
         }

         // Is this layer got side projection?
         if ( mat->useSideProjection() )
            features.addFeature( MFT_TerrainSideProject, featureIndex );
      }

      MaterialFeatureData featureData;
      featureData.features = features;
      featureData.materialFeatures = features;

      // Check to see how many vertex shader output 
      // registers we're gonna need.
      U32 numTex = 0;
      U32 numTexReg = 0;
      for ( U32 i=0; i < features.getCount(); i++ )
      {
         S32 index;
         const FeatureType &type = features.getAt( i, &index );
         ShaderFeature* sf = FEATUREMGR->getByType( type );
         if ( !sf ) 
            continue;

         sf->setProcessIndex( index );
         ShaderFeature::Resources res = sf->getResources( featureData );

         numTex += res.numTex;
         numTexReg += res.numTexReg;
      }

      // Can we build the shader?
      //
      // NOTE: The 10 is sort of an abitrary SM 3.0 
      // limit.  Its really supposed to be 11, but that
      // always fails to compile so far.
      //
      if (  numTex < GFX->getNumSamplers() &&
            numTexReg <= 10 )
      {         
         // NOTE: We really shouldn't be getting errors building the shaders,
         // but we can generate more instructions than the ps_2_x will allow.
         //
         // There is no way to deal with this case that i know of other than
         // letting the compile fail then recovering by trying to build it
         // with fewer materials.
         //
         // We normally disable the shader error logging so that the user 
         // isn't fooled into thinking there is a real bug.  That is until
         // we get down to a single material.  If a single material case
         // fails it means it cannot generate any passes at all!
         const bool logErrors = matCount == 1;
         GFXShader::setLogging( logErrors, true );

         pass->shader = SHADERGEN->getShader( featureData, getGFXVertexFormat<TerrVertex>(), NULL, mSamplerNames );
      }

      // If we got a shader then we can continue.
      if ( pass->shader )
         break;

      // If the material count is already 1 then this
      // is a real shader error... give up!
      if ( matCount <= 1 )
         return false;

      // If we failed we next try half the input materials
      // so that we can more quickly arrive at a valid shader.
      matCount -= matCount / 2;
   }

   // Setup the constant buffer.
   pass->consts = pass->shader->allocConstBuffer();

   // Prepare the basic constants.
   pass->modelViewProjConst = pass->shader->getShaderConstHandle( "$modelview" );
   pass->worldViewOnly = pass->shader->getShaderConstHandle( "$worldViewOnly" );
   pass->viewToObj = pass->shader->getShaderConstHandle( "$viewToObj" );
   pass->eyePosWorldConst = pass->shader->getShaderConstHandle( "$eyePosWorld" );
   pass->eyePosConst = pass->shader->getShaderConstHandle( "$eyePos" );
   pass->vEyeConst = pass->shader->getShaderConstHandle( "$vEye" );
   pass->layerSizeConst = pass->shader->getShaderConstHandle( "$layerSize" );
   pass->objTransConst = pass->shader->getShaderConstHandle( "$objTrans" );
   pass->worldToObjConst = pass->shader->getShaderConstHandle( "$worldToObj" );  
   pass->lightInfoBufferConst = pass->shader->getShaderConstHandle( "$lightInfoBuffer" );   
   pass->baseTexMapConst = pass->shader->getShaderConstHandle( "$baseTexMap" );
   pass->layerTexConst = pass->shader->getShaderConstHandle( "$layerTex" );
   pass->fogDataConst = pass->shader->getShaderConstHandle( "$fogData" );
   pass->fogColorConst = pass->shader->getShaderConstHandle( "$fogColor" );
   pass->lightMapTexConst = pass->shader->getShaderConstHandle( "$lightMapTex" );
   pass->oneOverTerrainSize = pass->shader->getShaderConstHandle( "$oneOverTerrainSize" );
   pass->squareSize = pass->shader->getShaderConstHandle( "$squareSize" );

   pass->lightParamsConst = pass->shader->getShaderConstHandle( "$rtParamslightInfoBuffer" );

   // Now prepare the basic stateblock.
   GFXStateBlockDesc desc;
   if ( !firstPass )
   {
      desc.setBlend( true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha );

      // If this is the prepass then we don't want to
      // write to the last two color channels (where
      // depth is usually encoded).
      //
      // This trick works in combination with the 
      // MFT_TerrainAdditive feature to lerp the
      // output normal with the previous pass.
      //
      if ( prePassMat )
         desc.setColorWrites( true, true, true, false );
   }

   // We write to the zbuffer if this is a prepass
   // material or if the prepass is disabled.
   desc.setZReadWrite( true,  !MATMGR->getPrePassEnabled() || 
                              prePassMat ||
                              reflectMat );

   desc.samplersDefined = true;
   if ( pass->baseTexMapConst->isValid() )
      desc.samplers[pass->baseTexMapConst->getSamplerRegister()] = GFXSamplerStateDesc::getWrapLinear();

   if ( pass->layerTexConst->isValid() )
      desc.samplers[pass->layerTexConst->getSamplerRegister()] = GFXSamplerStateDesc::getClampPoint();

   if ( pass->lightInfoBufferConst->isValid() )
      desc.samplers[pass->lightInfoBufferConst->getSamplerRegister()] = GFXSamplerStateDesc::getClampPoint();

   if ( pass->lightMapTexConst->isValid() )
      desc.samplers[pass->lightMapTexConst->getSamplerRegister()] = GFXSamplerStateDesc::getWrapLinear();

   const U32 maxAnisotropy = MATMGR->getDefaultAnisotropy();

   // Finally setup the material specific shader 
   // constants and stateblock state.
   //
   // NOTE: If this changes be sure to check TerrainCellMaterial::_updateDefaultAnisotropy
   // to see if it needs the same changes.
   //
   for ( U32 i=0; i < pass->materials.size(); i++ )
   {
      MaterialInfo *matInfo = pass->materials[i];

      matInfo->detailInfoVConst = pass->shader->getShaderConstHandle( avar( "$detailScaleAndFade%d", i ) );
      matInfo->detailInfoPConst = pass->shader->getShaderConstHandle( avar( "$detailIdStrengthParallax%d", i ) );

      matInfo->detailTexConst = pass->shader->getShaderConstHandle( avar( "$detailMap%d", i ) );
      if ( matInfo->detailTexConst->isValid() )
      {
         const S32 sampler = matInfo->detailTexConst->getSamplerRegister();

         desc.samplers[sampler] = GFXSamplerStateDesc::getWrapLinear();
         desc.samplers[sampler].magFilter = GFXTextureFilterLinear;
         desc.samplers[sampler].mipFilter = GFXTextureFilterLinear;

         if ( maxAnisotropy > 1 )
         {
            desc.samplers[sampler].minFilter = GFXTextureFilterAnisotropic;
            desc.samplers[sampler].maxAnisotropy = maxAnisotropy;
         }
         else
            desc.samplers[sampler].minFilter = GFXTextureFilterLinear;

         matInfo->detailTex.set( matInfo->mat->getDetailMap(), 
            &GFXDefaultStaticDiffuseProfile, "TerrainCellMaterial::_createPass() - DetailMap" );
      }

      matInfo->macroInfoVConst = pass->shader->getShaderConstHandle( avar( "$macroScaleAndFade%d", i ) );
      matInfo->macroInfoPConst = pass->shader->getShaderConstHandle( avar( "$macroIdStrengthParallax%d", i ) );

      matInfo->macroTexConst = pass->shader->getShaderConstHandle( avar( "$macroMap%d", i ) );
      if ( matInfo->macroTexConst->isValid() )
      {
         const S32 sampler = matInfo->macroTexConst->getSamplerRegister();

         desc.samplers[sampler] = GFXSamplerStateDesc::getWrapLinear();
         desc.samplers[sampler].magFilter = GFXTextureFilterLinear;
         desc.samplers[sampler].mipFilter = GFXTextureFilterLinear;

         if ( maxAnisotropy > 1 )
         {
            desc.samplers[sampler].minFilter = GFXTextureFilterAnisotropic;
            desc.samplers[sampler].maxAnisotropy = maxAnisotropy;
         }
         else
            desc.samplers[sampler].minFilter = GFXTextureFilterLinear;

         matInfo->macroTex.set( matInfo->mat->getMacroMap(), 
            &GFXDefaultStaticDiffuseProfile, "TerrainCellMaterial::_createPass() - MacroMap" );
      }
	  //end macro texture

      matInfo->normalTexConst = pass->shader->getShaderConstHandle( avar( "$normalMap%d", i ) );
      if ( matInfo->normalTexConst->isValid() )
      {
         const S32 sampler = matInfo->normalTexConst->getSamplerRegister();

         desc.samplers[sampler] = GFXSamplerStateDesc::getWrapLinear();
         desc.samplers[sampler].magFilter = GFXTextureFilterLinear;
         desc.samplers[sampler].mipFilter = GFXTextureFilterLinear;

         if ( maxAnisotropy > 1 )
         {
            desc.samplers[sampler].minFilter = GFXTextureFilterAnisotropic;
            desc.samplers[sampler].maxAnisotropy = maxAnisotropy;
         }
         else
            desc.samplers[sampler].minFilter = GFXTextureFilterLinear;

         matInfo->normalTex = normalMaps[i];
      }
   }

   // Remove the materials we processed and leave the
   // ones that remain for the next pass.
   for ( U32 i=0; i < matCount; i++ )
   {
      MaterialInfo *matInfo = materials->first();
      if ( baseOnly || pass->materials.find_next( matInfo ) == -1 )
         delete matInfo;     
      materials->pop_front();
   }

   // If we're doing prepass it requires some 
   // special stencil settings for it to work.
   if ( prePassMat )
      desc.addDesc( RenderPrePassMgr::getOpaqueStenciWriteDesc( false ) );

   desc.setCullMode( GFXCullCCW );
   pass->stateBlock = GFX->createStateBlock(desc);

   //reflection stateblock
   desc.setCullMode( GFXCullCW );
   pass->reflectionStateBlock = GFX->createStateBlock(desc);

   // Create the wireframe state blocks.
   GFXStateBlockDesc wireframe( desc );
   wireframe.fillMode = GFXFillWireframe;
   wireframe.setCullMode( GFXCullCCW );
   pass->wireframeStateBlock = GFX->createStateBlock( wireframe );

   return true;
}

void TerrainCellMaterial::_updateMaterialConsts( Pass *pass )
{
   PROFILE_SCOPE( TerrainCellMaterial_UpdateMaterialConsts );

   for ( U32 j=0; j < pass->materials.size(); j++ )
   {
      MaterialInfo *matInfo = pass->materials[j];

      F32 detailSize = matInfo->mat->getDetailSize();
      F32 detailScale = 1.0f;
      if ( !mIsZero( detailSize ) )
         detailScale = mTerrain->getWorldBlockSize() / detailSize;

      // Scale the distance by the global scalar.
      const F32 distance = mTerrain->smDetailScale * matInfo->mat->getDetailDistance();

      // NOTE: The negation of the y scale is to make up for 
      // my mistake early in development and passing the wrong
      // y texture coord into the system.
      //
      // This negation fixes detail, normal, and parallax mapping
      // without harming the layer id blending code.
      //
      // Eventually we should rework this to correct this little
      // mistake, but there isn't really a hurry to.
      //
      Point4F detailScaleAndFade(   detailScale,
                                    -detailScale,
                                    distance, 
                                    0 );

      if ( !mIsZero( distance ) )
         detailScaleAndFade.w = 1.0f / distance;

      Point3F detailIdStrengthParallax( matInfo->layerId,
                                        matInfo->mat->getDetailStrength(),
                                        matInfo->mat->getParallaxScale() );

      pass->consts->setSafe( matInfo->detailInfoVConst, detailScaleAndFade );
      pass->consts->setSafe( matInfo->detailInfoPConst, detailIdStrengthParallax );

	// macro texture info

      F32 macroSize = matInfo->mat->getMacroSize();
      F32 macroScale = 1.0f;
      if ( !mIsZero( macroSize ) )
         macroScale = mTerrain->getWorldBlockSize() / macroSize;

      // Scale the distance by the global scalar.
      const F32 macroDistance = mTerrain->smDetailScale * matInfo->mat->getMacroDistance();

      Point4F macroScaleAndFade(   macroScale,
                                    -macroScale,
                                    macroDistance, 
                                    0 );

      if ( !mIsZero( macroDistance ) )
         macroScaleAndFade.w = 1.0f / macroDistance;

      Point3F macroIdStrengthParallax( matInfo->layerId,
                                        matInfo->mat->getMacroStrength(),
                                        0 );

      pass->consts->setSafe( matInfo->macroInfoVConst, macroScaleAndFade );
      pass->consts->setSafe( matInfo->macroInfoPConst, macroIdStrengthParallax );
   }
}

bool TerrainCellMaterial::setupPass(   const SceneRenderState *state, 
                                       const SceneData &sceneData )
{
   PROFILE_SCOPE( TerrainCellMaterial_SetupPass );

   if ( mCurrPass >= mPasses.size() )
   {
      mCurrPass = 0;
      return false;
   }

   Pass &pass = mPasses[mCurrPass];

   _updateMaterialConsts( &pass );

   if ( pass.baseTexMapConst->isValid() )
      GFX->setTexture( pass.baseTexMapConst->getSamplerRegister(), mTerrain->mBaseTex.getPointer() );

   if ( pass.layerTexConst->isValid() )
      GFX->setTexture( pass.layerTexConst->getSamplerRegister(), mTerrain->mLayerTex.getPointer() );

   if ( pass.lightMapTexConst->isValid() )
      GFX->setTexture( pass.lightMapTexConst->getSamplerRegister(), mTerrain->getLightMapTex() );

   if ( sceneData.wireframe )
      GFX->setStateBlock( pass.wireframeStateBlock );
   else if ( state->isReflectPass( ))
      GFX->setStateBlock( pass.reflectionStateBlock );
   else
      GFX->setStateBlock( pass.stateBlock );

   GFX->setShader( pass.shader );
   GFX->setShaderConstBuffer( pass.consts );

   // Let the light manager prepare any light stuff it needs.
   LIGHTMGR->setLightInfo( NULL,
                           NULL,
                           sceneData,
                           state,
                           mCurrPass,
                           pass.consts );

   for ( U32 i=0; i < pass.materials.size(); i++ )
   {
      MaterialInfo *matInfo = pass.materials[i];

      if ( matInfo->detailTexConst->isValid() )
         GFX->setTexture( matInfo->detailTexConst->getSamplerRegister(), matInfo->detailTex );
      if ( matInfo->macroTexConst->isValid() )
         GFX->setTexture( matInfo->macroTexConst->getSamplerRegister(), matInfo->macroTex );
      if ( matInfo->normalTexConst->isValid() )
         GFX->setTexture( matInfo->normalTexConst->getSamplerRegister(), matInfo->normalTex );
   }

   pass.consts->setSafe( pass.layerSizeConst, (F32)mTerrain->mLayerTex.getWidth() );

   if ( pass.oneOverTerrainSize->isValid() )
   {
      F32 oneOverTerrainSize = 1.0f / mTerrain->getWorldBlockSize();
      pass.consts->set( pass.oneOverTerrainSize, oneOverTerrainSize );
   }

   pass.consts->setSafe( pass.squareSize, mTerrain->getSquareSize() );

   if ( pass.fogDataConst->isValid() )
   {
      Point3F fogData;
      fogData.x = sceneData.fogDensity;
      fogData.y = sceneData.fogDensityOffset;
      fogData.z = sceneData.fogHeightFalloff;     
      pass.consts->set( pass.fogDataConst, fogData );
   }

   pass.consts->setSafe( pass.fogColorConst, sceneData.fogColor );

   if (  pass.lightInfoBufferConst->isValid() &&
         pass.lightParamsConst->isValid() )
   {
      if ( !mLightInfoTarget )
         mLightInfoTarget = NamedTexTarget::find( "lightinfo" );

      GFXTextureObject *texObject = mLightInfoTarget->getTexture();
      
      // TODO: Sometimes during reset of the light manager we get a
      // NULL texture here.  This is corrected on the next frame, but
      // we should still investigate why that happens.
      
      if ( texObject )
      {
         GFX->setTexture( pass.lightInfoBufferConst->getSamplerRegister(), texObject );

         const Point3I &targetSz = texObject->getSize();
         const RectI &targetVp = mLightInfoTarget->getViewport();
         Point4F rtParams;
         ScreenSpace::RenderTargetParameters(targetSz, targetVp, rtParams);
         pass.consts->setSafe( pass.lightParamsConst, rtParams );
      }
   }

   ++mCurrPass;
   return true;
}

BaseMatInstance* TerrainCellMaterial::getShadowMat()
{
   // Find our material which has some settings
   // defined on it in script.
   Material *mat = MATMGR->getMaterialDefinitionByName( "AL_DefaultShadowMaterial" );

   // Create the material instance adding the feature which
   // handles rendering terrain cut outs.
   FeatureSet features = MATMGR->getDefaultFeatures();
   BaseMatInstance *matInst = mat->createMatInstance();
   if ( !matInst->init( features, getGFXVertexFormat<TerrVertex>() ) )
   {
      delete matInst;
      matInst = NULL;
   }

   return matInst;
}

