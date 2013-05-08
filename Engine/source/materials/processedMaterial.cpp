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
#include "materials/processedMaterial.h"

#include "materials/sceneData.h"
#include "materials/materialParameters.h"
#include "materials/matTextureTarget.h"
#include "materials/materialFeatureTypes.h"
#include "materials/materialManager.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxPrimitiveBuffer.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/sim/cubemapData.h"

RenderPassData::RenderPassData()
{
   reset();
}

void RenderPassData::reset()
{
   for( U32 i = 0; i < Material::MAX_TEX_PER_PASS; ++ i )
      destructInPlace( &mTexSlot[ i ] );

   dMemset( &mTexSlot, 0, sizeof(mTexSlot) );
   dMemset( &mTexType, 0, sizeof(mTexType) );

   mCubeMap = NULL;
   mNumTex = mNumTexReg = mStageNum = 0;
   mGlow = false;
   mBlendOp = Material::None;

   mFeatureData.clear();

   for (U32 i = 0; i < STATE_MAX; i++)
      mRenderStates[i] = NULL;
}

String RenderPassData::describeSelf() const
{
   String desc;

   // Now write all the textures.
   String texName;
   for ( U32 i=0; i < Material::MAX_TEX_PER_PASS; i++ )
   {
      if ( mTexType[i] == Material::TexTarget )
         texName = ( mTexSlot[i].texTarget ) ? mTexSlot[i].texTarget->getName() : "null_texTarget";
      else if ( mTexType[i] == Material::Cube && mCubeMap )
         texName = mCubeMap->getPath();
      else if ( mTexSlot[i].texObject )
         texName = mTexSlot[i].texObject->getPath();
      else
         continue;

      desc += String::ToString( "TexSlot %d: %d, %s\n", i, mTexType[i], texName.c_str() );
   }

   // Write out the first render state which is the
   // basis for all the other states and shoud be
   // enough to define the pass uniquely.
   desc += mRenderStates[0]->getDesc().describeSelf();

   return desc;
}

ProcessedMaterial::ProcessedMaterial()
:  mMaterial( NULL ),
   mCurrentParams( NULL ),
   mHasSetStageData( false ),
   mHasGlow( false ),   
   mMaxStages( 0 ),
   mVertexFormat( NULL ),
   mUserObject( NULL )
{
   VECTOR_SET_ASSOCIATION( mPasses );
}

ProcessedMaterial::~ProcessedMaterial()
{
   for_each( mPasses.begin(), mPasses.end(), delete_pointer() );
}

void ProcessedMaterial::_setBlendState(Material::BlendOp blendOp, GFXStateBlockDesc& desc )
{
   switch( blendOp )
   {
   case Material::Add:
      {
         desc.blendSrc = GFXBlendOne;
         desc.blendDest = GFXBlendOne;
         break;
      }
   case Material::AddAlpha:
      {
         desc.blendSrc = GFXBlendSrcAlpha;
         desc.blendDest = GFXBlendOne;
         break;
      }
   case Material::Mul:
      {
         desc.blendSrc = GFXBlendDestColor;
         desc.blendDest = GFXBlendZero;
         break;
      }
   case Material::LerpAlpha:
      {
         desc.blendSrc = GFXBlendSrcAlpha;
         desc.blendDest = GFXBlendInvSrcAlpha;
         break;
      }

   default:
      {
         // default to LerpAlpha
         desc.blendSrc = GFXBlendSrcAlpha;
         desc.blendDest = GFXBlendInvSrcAlpha;
         break;
      }
   }
}

void ProcessedMaterial::setBuffers(GFXVertexBufferHandleBase* vertBuffer, GFXPrimitiveBufferHandle* primBuffer)
{
   GFX->setVertexBuffer( *vertBuffer );
   GFX->setPrimitiveBuffer( *primBuffer );
}

bool ProcessedMaterial::stepInstance()
{
   AssertFatal( false, "ProcessedMaterial::stepInstance() - This type of material doesn't support instancing!" );
   return false;
}

String ProcessedMaterial::_getTexturePath(const String& filename)
{
   // if '/', then path is specified, use it.
   if( filename.find('/') != String::NPos )
   {
      return filename;
   }

   // otherwise, construct path
   return mMaterial->getPath() + filename;
}

GFXTexHandle ProcessedMaterial::_createTexture( const char* filename, GFXTextureProfile *profile)
{
   return GFXTexHandle( _getTexturePath(filename), profile, avar("%s() - NA (line %d)", __FUNCTION__, __LINE__) );
}

void ProcessedMaterial::addStateBlockDesc(const GFXStateBlockDesc& sb)
{
   mUserDefined = sb;
}

void ProcessedMaterial::_initStateBlockTemplates(GFXStateBlockDesc& stateTranslucent, GFXStateBlockDesc& stateGlow, GFXStateBlockDesc& stateReflect)
{
   // Translucency   
   stateTranslucent.blendDefined = true;
   stateTranslucent.blendEnable = mMaterial->mTranslucentBlendOp != Material::None;
   _setBlendState(mMaterial->mTranslucentBlendOp, stateTranslucent);
   stateTranslucent.zDefined = true;
   stateTranslucent.zWriteEnable = mMaterial->mTranslucentZWrite;   
   stateTranslucent.alphaDefined = true;
   stateTranslucent.alphaTestEnable = mMaterial->mAlphaTest;
   stateTranslucent.alphaTestRef = mMaterial->mAlphaRef;
   stateTranslucent.alphaTestFunc = GFXCmpGreaterEqual;
   stateTranslucent.samplersDefined = true;
   stateTranslucent.samplers[0].textureColorOp = GFXTOPModulate;
   stateTranslucent.samplers[0].alphaOp = GFXTOPModulate;   
   stateTranslucent.samplers[0].alphaArg1 = GFXTATexture;
   stateTranslucent.samplers[0].alphaArg2 = GFXTADiffuse;   

   // Glow   
   stateGlow.zDefined = true;
   stateGlow.zWriteEnable = false;

   // Reflect   
   stateReflect.cullDefined = true;
   stateReflect.cullMode = mMaterial->mDoubleSided ? GFXCullNone : GFXCullCW;
}

void ProcessedMaterial::_initRenderPassDataStateBlocks()
{
   for (U32 pass = 0; pass < mPasses.size(); pass++)
      _initRenderStateStateBlocks( mPasses[pass] );
}

void ProcessedMaterial::_initPassStateBlock( RenderPassData *rpd, GFXStateBlockDesc &result )
{
   if ( rpd->mBlendOp != Material::None )
   {
      result.blendDefined = true;
      result.blendEnable = true;
      _setBlendState( rpd->mBlendOp, result );
   }

   if (mMaterial->isDoubleSided())
   {
      result.cullDefined = true;
      result.cullMode = GFXCullNone;         
   }

   if(mMaterial->mAlphaTest)
   {
      result.alphaDefined = true;
      result.alphaTestEnable = mMaterial->mAlphaTest;
      result.alphaTestRef = mMaterial->mAlphaRef;
      result.alphaTestFunc = GFXCmpGreaterEqual;
   }

   result.samplersDefined = true;
   NamedTexTarget *texTarget;

   U32 maxAnisotropy = 1;
   if ( mMaterial->mUseAnisotropic[ rpd->mStageNum ] )
      maxAnisotropy = MATMGR->getDefaultAnisotropy();

   for( U32 i=0; i < rpd->mNumTex; i++ )
   {      
      U32 currTexFlag = rpd->mTexType[i];

      switch( currTexFlag )
      {
         default:
         {
            result.samplers[i].textureColorOp = GFXTOPModulate;
            result.samplers[i].addressModeU = GFXAddressWrap;
            result.samplers[i].addressModeV = GFXAddressWrap;

            if ( maxAnisotropy > 1 )
            {
               result.samplers[i].minFilter = GFXTextureFilterAnisotropic;
               result.samplers[i].magFilter = GFXTextureFilterAnisotropic;
               result.samplers[i].maxAnisotropy = maxAnisotropy;
            }
            else
            {
               result.samplers[i].minFilter = GFXTextureFilterLinear;
               result.samplers[i].magFilter = GFXTextureFilterLinear;
            }
            break;
         }

         case Material::Cube:
         case Material::SGCube:
         case Material::NormalizeCube:
         {
            result.samplers[i].addressModeU = GFXAddressClamp;
            result.samplers[i].addressModeV = GFXAddressClamp;
            result.samplers[i].addressModeW = GFXAddressClamp;
            break;
         }

         case Material::TexTarget:
         {
            texTarget = mPasses[0]->mTexSlot[i].texTarget;
            if ( texTarget )
               texTarget->setupSamplerState( &result.samplers[i] );
            break;
         }
      }
   }

   // The prepass will take care of writing to the 
   // zbuffer, so we don't have to by default.
   // The prepass can't write to the backbuffer's zbuffer in OpenGL.
   if (  MATMGR->getPrePassEnabled() && 
         !GFX->getAdapterType() == OpenGL && 
         !mFeatures.hasFeature(MFT_ForwardShading))
      result.setZReadWrite( result.zEnable, false );

   result.addDesc(mUserDefined);
}

/// Creates the default state blocks for a list of render states
void ProcessedMaterial::_initRenderStateStateBlocks( RenderPassData *rpd )
{
   GFXStateBlockDesc stateTranslucent;
   GFXStateBlockDesc stateGlow;
   GFXStateBlockDesc stateReflect;
   GFXStateBlockDesc statePass;

   _initStateBlockTemplates( stateTranslucent, stateGlow, stateReflect );
   _initPassStateBlock( rpd, statePass );

   // Ok, we've got our templates set up, let's combine them together based on state and
   // create our state blocks.
   for (U32 i = 0; i < RenderPassData::STATE_MAX; i++)
   {
      GFXStateBlockDesc stateFinal;

      if (i & RenderPassData::STATE_REFLECT)
         stateFinal.addDesc(stateReflect);
      if (i & RenderPassData::STATE_TRANSLUCENT)
         stateFinal.addDesc(stateTranslucent);
      if (i & RenderPassData::STATE_GLOW)
         stateFinal.addDesc(stateGlow);

      stateFinal.addDesc(statePass);

      if (i & RenderPassData::STATE_WIREFRAME)
         stateFinal.fillMode = GFXFillWireframe;

      GFXStateBlockRef sb = GFX->createStateBlock(stateFinal);
      rpd->mRenderStates[i] = sb;
   }   
}

U32 ProcessedMaterial::_getRenderStateIndex( const SceneRenderState *sceneState, 
                                             const SceneData &sgData )
{
   // Based on what the state of the world is, get our render state block
   U32 currState = 0;

   // NOTE: We should only use per-material or per-pass hints to
   // change the render state.  This is importaint because we 
   // only change the state blocks between material passes.
   //
   // For example sgData.visibility would be bad to use
   // in here without changing how RenderMeshMgr works.

   if ( sgData.binType == SceneData::GlowBin )
      currState |= RenderPassData::STATE_GLOW;

   if ( sceneState && sceneState->isReflectPass() )
      currState |= RenderPassData::STATE_REFLECT;

   if ( sgData.binType != SceneData::PrePassBin &&
        mMaterial->isTranslucent() )
      currState |= RenderPassData::STATE_TRANSLUCENT;

   if ( sgData.wireframe )
      currState |= RenderPassData::STATE_WIREFRAME;

   return currState;
}

void ProcessedMaterial::_setRenderState(  const SceneRenderState *state, 
                                          const SceneData& sgData, 
                                          U32 pass )
{   
   // Make sure we have the pass
   if ( pass >= mPasses.size() )
      return;

   U32 currState = _getRenderStateIndex( state, sgData );

   GFX->setStateBlock(mPasses[pass]->mRenderStates[currState]);   
}


void ProcessedMaterial::_setStageData()
{
   // Only do this once
   if ( mHasSetStageData ) 
      return;
   mHasSetStageData = true;

   U32 i;

   // Load up all the textures for every possible stage
   for( i=0; i<Material::MAX_STAGES; i++ )
   {
      // DiffuseMap
      if( mMaterial->mDiffuseMapFilename[i].isNotEmpty() )
      {
         mStages[i].setTex( MFT_DiffuseMap, _createTexture( mMaterial->mDiffuseMapFilename[i], &GFXDefaultStaticDiffuseProfile ) );
         if (!mStages[i].getTex( MFT_DiffuseMap ))
         {
            mMaterial->logError("Failed to load diffuse map %s for stage %i", _getTexturePath(mMaterial->mDiffuseMapFilename[i]).c_str(), i);
            
            // Load a debug texture to make it clear to the user 
            // that the texture for this stage was missing.
            mStages[i].setTex( MFT_DiffuseMap, _createTexture( GFXTextureManager::getMissingTexturePath().c_str(), &GFXDefaultStaticDiffuseProfile ) );
         }
      }

      // OverlayMap
      if( mMaterial->mOverlayMapFilename[i].isNotEmpty() )
      {
         mStages[i].setTex( MFT_OverlayMap, _createTexture( mMaterial->mOverlayMapFilename[i], &GFXDefaultStaticDiffuseProfile ) );
         if(!mStages[i].getTex( MFT_OverlayMap ))
            mMaterial->logError("Failed to load overlay map %s for stage %i", _getTexturePath(mMaterial->mOverlayMapFilename[i]).c_str(), i);
      }

      // LightMap
      if( mMaterial->mLightMapFilename[i].isNotEmpty() )
      {
         mStages[i].setTex( MFT_LightMap, _createTexture( mMaterial->mLightMapFilename[i], &GFXDefaultStaticDiffuseProfile ) );
         if(!mStages[i].getTex( MFT_LightMap ))
            mMaterial->logError("Failed to load light map %s for stage %i", _getTexturePath(mMaterial->mLightMapFilename[i]).c_str(), i);
      }

      // ToneMap
      if( mMaterial->mToneMapFilename[i].isNotEmpty() )
      {
         mStages[i].setTex( MFT_ToneMap, _createTexture( mMaterial->mToneMapFilename[i], &GFXDefaultStaticDiffuseProfile ) );
         if(!mStages[i].getTex( MFT_ToneMap ))
            mMaterial->logError("Failed to load tone map %s for stage %i", _getTexturePath(mMaterial->mToneMapFilename[i]).c_str(), i);
      }

      // DetailMap
      if( mMaterial->mDetailMapFilename[i].isNotEmpty() )
      {
         mStages[i].setTex( MFT_DetailMap, _createTexture( mMaterial->mDetailMapFilename[i], &GFXDefaultStaticDiffuseProfile ) );
         if(!mStages[i].getTex( MFT_DetailMap ))
            mMaterial->logError("Failed to load detail map %s for stage %i", _getTexturePath(mMaterial->mDetailMapFilename[i]).c_str(), i);
      }

      // NormalMap
      if( mMaterial->mNormalMapFilename[i].isNotEmpty() )
      {
         mStages[i].setTex( MFT_NormalMap, _createTexture( mMaterial->mNormalMapFilename[i], &GFXDefaultStaticNormalMapProfile ) );
         if(!mStages[i].getTex( MFT_NormalMap ))
            mMaterial->logError("Failed to load normal map %s for stage %i", _getTexturePath(mMaterial->mNormalMapFilename[i]).c_str(), i);
      }

      // Detail Normal Map
      if( mMaterial->mDetailNormalMapFilename[i].isNotEmpty() )
      {
         mStages[i].setTex( MFT_DetailNormalMap, _createTexture( mMaterial->mDetailNormalMapFilename[i], &GFXDefaultStaticNormalMapProfile ) );
         if(!mStages[i].getTex( MFT_DetailNormalMap ))
            mMaterial->logError("Failed to load normal map %s for stage %i", _getTexturePath(mMaterial->mDetailNormalMapFilename[i]).c_str(), i);
      }
      
      // SpecularMap
      if( mMaterial->mSpecularMapFilename[i].isNotEmpty() )
      {
         mStages[i].setTex( MFT_SpecularMap, _createTexture( mMaterial->mSpecularMapFilename[i], &GFXDefaultStaticDiffuseProfile ) );
         if(!mStages[i].getTex( MFT_SpecularMap ))
            mMaterial->logError("Failed to load specular map %s for stage %i", _getTexturePath(mMaterial->mSpecularMapFilename[i]).c_str(), i);
      }

      // EnironmentMap
      if( mMaterial->mEnvMapFilename[i].isNotEmpty() )
      {
         mStages[i].setTex( MFT_EnvMap, _createTexture( mMaterial->mEnvMapFilename[i], &GFXDefaultStaticDiffuseProfile ) );
         if(!mStages[i].getTex( MFT_EnvMap ))
            mMaterial->logError("Failed to load environment map %s for stage %i", _getTexturePath(mMaterial->mEnvMapFilename[i]).c_str(), i);
      }
   }

	mMaterial->mCubemapData = dynamic_cast<CubemapData*>(Sim::findObject( mMaterial->mCubemapName ));
	if( !mMaterial->mCubemapData )
		mMaterial->mCubemapData = NULL;
		
		
   // If we have a cubemap put it on stage 0 (cubemaps only supported on stage 0)
   if( mMaterial->mCubemapData )
   {
      mMaterial->mCubemapData->createMap();
      mStages[0].setCubemap( mMaterial->mCubemapData->mCubemap ); 
      if ( !mStages[0].getCubemap() )
         mMaterial->logError("Failed to load cubemap");
   }
}

