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
#include "lighting/shadowMap/shadowMatHook.h"

#include "materials/materialManager.h"
#include "materials/customMaterialDefinition.h"
#include "materials/materialFeatureTypes.h"
#include "materials/materialFeatureData.h"
#include "shaderGen/featureType.h"
#include "shaderGen/featureMgr.h"
#include "scene/sceneRenderState.h"
#include "terrain/terrFeatureTypes.h"


const MatInstanceHookType ShadowMaterialHook::Type( "ShadowMap" );

ShadowMaterialHook::ShadowMaterialHook()
{
   dMemset( mShadowMat, 0, sizeof( mShadowMat ) );
}

ShadowMaterialHook::~ShadowMaterialHook()
{
   for ( U32 i = 0; i < ShadowType_Count; i++ )
      SAFE_DELETE( mShadowMat[i] );
}

void ShadowMaterialHook::init( BaseMatInstance *inMat )
{
   if( !inMat->isValid() )
      return;

   // Tweak the feature data to include just what we need.
   FeatureSet features;
   features.addFeature( MFT_VertTransform );
   features.addFeature( MFT_DiffuseMap );
   features.addFeature( MFT_TexAnim );
   features.addFeature( MFT_AlphaTest );
   features.addFeature( MFT_Visibility );

   // Actually we want to include features from the inMat
   // if they operate on the preTransform verts so things
   // like wind/deformation effects will also affect the shadow.
   const FeatureSet &inFeatures = inMat->getFeatures();
   for ( U32 i = 0; i < inFeatures.getCount(); i++ )
   {      
      const FeatureType& ft = inFeatures.getAt(i);
      
      if ( ft.getGroup() == MFG_PreTransform )
         features.addFeature( ft );
   }

   // Do instancing in shadows if we can.
   if ( inFeatures.hasFeature( MFT_UseInstancing ) )
      features.addFeature( MFT_UseInstancing );

   Material *shadowMat = (Material*)inMat->getMaterial();
   if ( dynamic_cast<CustomMaterial*>( shadowMat ) )
   {
      // This is a custom material... who knows what it really does, but
      // if it wasn't already filtered out of the shadow render then just
      // give it some default depth out material.
      shadowMat = MATMGR->getMaterialDefinitionByName( "AL_DefaultShadowMaterial" );
   }

   // By default we want to disable some states
   // that the material might enable for us.
   GFXStateBlockDesc forced;
   forced.setBlend( false );
   forced.setAlphaTest( false );

   // We should force on zwrite as the prepass
   // will disable it by default.
   forced.setZReadWrite( true, true );
   
   // TODO: Should we render backfaces for 
   // shadows or does the ESM take care of 
   // all our acne issues?
   //forced.setCullMode( GFXCullCW );

   // Vector, and spotlights use the same shadow material.
   BaseMatInstance *newMat = new ShadowMatInstance( shadowMat );
   newMat->setUserObject( inMat->getUserObject() );
   newMat->getFeaturesDelegate().bind( &ShadowMaterialHook::_overrideFeatures );
   newMat->addStateBlockDesc( forced );
   if( !newMat->init( features, inMat->getVertexFormat() ) )
   {
      SAFE_DELETE( newMat );
      newMat = MATMGR->createWarningMatInstance();
   }
   
   mShadowMat[ShadowType_Spot] = newMat;

   newMat = new ShadowMatInstance( shadowMat );
   newMat->setUserObject( inMat->getUserObject() );
   newMat->getFeaturesDelegate().bind( &ShadowMaterialHook::_overrideFeatures );
   forced.setCullMode( GFXCullCW );   
   newMat->addStateBlockDesc( forced );
   forced.cullDefined = false;
   newMat->addShaderMacro( "CUBE_SHADOW_MAP", "" );
   newMat->init( features, inMat->getVertexFormat() );
   mShadowMat[ShadowType_CubeMap] = newMat;
   
   // A dual paraboloid shadow rendered in a single draw call.
   features.addFeature( MFT_ParaboloidVertTransform );
   features.addFeature( MFT_IsSinglePassParaboloid );
   features.removeFeature( MFT_VertTransform );
   newMat = new ShadowMatInstance( shadowMat );
   newMat->setUserObject( inMat->getUserObject() );
   GFXStateBlockDesc noCull( forced );
   noCull.setCullMode( GFXCullNone );
   newMat->addStateBlockDesc( noCull );
   newMat->getFeaturesDelegate().bind( &ShadowMaterialHook::_overrideFeatures );
   newMat->init( features, inMat->getVertexFormat() );
   mShadowMat[ShadowType_DualParaboloidSinglePass] = newMat;

   // Regular dual paraboloid shadow.
   features.addFeature( MFT_ParaboloidVertTransform );
   features.removeFeature( MFT_IsSinglePassParaboloid );
   features.removeFeature( MFT_VertTransform );
   newMat = new ShadowMatInstance( shadowMat );
   newMat->setUserObject( inMat->getUserObject() );
   newMat->addStateBlockDesc( forced );
   newMat->getFeaturesDelegate().bind( &ShadowMaterialHook::_overrideFeatures );
   newMat->init( features, inMat->getVertexFormat() );
   mShadowMat[ShadowType_DualParaboloid] = newMat;

   /*
   // A single paraboloid shadow.
   newMat = new ShadowMatInstance( startMatInstance );
   GFXStateBlockDesc noCull;
   noCull.setCullMode( GFXCullNone );
   newMat->addStateBlockDesc( noCull );
   newMat->getFeaturesDelegate().bind( &ShadowMaterialHook::_overrideFeatures );
   newMat->init( features, globalFeatures, inMat->getVertexFormat() );
   mShadowMat[ShadowType_DualParaboloidSinglePass] = newMat;
   */
}

BaseMatInstance* ShadowMaterialHook::getShadowMat( ShadowType type ) const
{ 
   AssertFatal( type < ShadowType_Count, "ShadowMaterialHook::getShadowMat() - Bad light type!" );

   // The cubemap and pssm shadows use the same
   // spotlight material for shadows.
   if (  type == ShadowType_Spot ||         
         type == ShadowType_PSSM )
      return mShadowMat[ShadowType_Spot];   

   // Get the specialized shadow material.
   return mShadowMat[type]; 
}

void ShadowMaterialHook::_overrideFeatures(  ProcessedMaterial *mat,
                                             U32 stageNum,
                                             MaterialFeatureData &fd, 
                                             const FeatureSet &features )
{
   if ( stageNum != 0 )
   {
      fd.features.clear();
      return;
   }

   // Disable the base texture if we don't 
   // have alpha test enabled.
   if ( !fd.features[ MFT_AlphaTest ] )
   {
      fd.features.removeFeature( MFT_TexAnim );
      fd.features.removeFeature( MFT_DiffuseMap );
   }

   // HACK: Need to figure out how to enable these 
   // suckers without this override call!

   fd.features.setFeature( MFT_ParaboloidVertTransform, 
      features.hasFeature( MFT_ParaboloidVertTransform ) );
   fd.features.setFeature( MFT_IsSinglePassParaboloid, 
      features.hasFeature( MFT_IsSinglePassParaboloid ) );
      
   // The paraboloid transform outputs linear depth, so
   // it needs to use the plain depth out feature.
   if ( fd.features.hasFeature( MFT_ParaboloidVertTransform ) ) 
      fd.features.addFeature( MFT_DepthOut );      
   else
      fd.features.addFeature( MFT_EyeSpaceDepthOut );
}

ShadowMatInstance::ShadowMatInstance( Material *mat ) 
 : MatInstance( *mat )
{
   mLightmappedMaterial = mMaterial->isLightmapped();
}

bool ShadowMatInstance::setupPass( SceneRenderState *state, const SceneData &sgData )
{
   // Respect SceneRenderState render flags
   if( (mLightmappedMaterial && !state->renderLightmappedMeshes()) ||
       (!mLightmappedMaterial && !state->renderNonLightmappedMeshes()) )
      return false;

   return Parent::setupPass(state, sgData);
}
