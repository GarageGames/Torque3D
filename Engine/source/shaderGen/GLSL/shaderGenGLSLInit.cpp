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

#include "shaderGen/shaderGen.h"
#include "shaderGen/GLSL/shaderGenGLSL.h"
#include "shaderGen/GLSL/shaderFeatureGLSL.h"
#include "shaderGen/featureMgr.h"
#include "shaderGen/GLSL/bumpGLSL.h"
#include "shaderGen/GLSL/pixSpecularGLSL.h"
#include "shaderGen/GLSL/depthGLSL.h"
#include "shaderGen/GLSL/paraboloidGLSL.h"
#include "materials/materialFeatureTypes.h"
#include "core/module.h"
#include "shaderGen/GLSL/accuFeatureGLSL.h"

// Deferred Shading
#include "lighting/advanced/glsl/deferredShadingFeaturesGLSL.h"

static ShaderGen::ShaderGenInitDelegate sInitDelegate;

void _initShaderGenGLSL( ShaderGen *shaderGen )
{
   shaderGen->setPrinter( new ShaderGenPrinterGLSL );
   shaderGen->setComponentFactory( new ShaderGenComponentFactoryGLSL );
   shaderGen->setFileEnding( "glsl" );

   FEATUREMGR->registerFeature( MFT_VertTransform, new VertPositionGLSL );
   FEATUREMGR->registerFeature( MFT_RTLighting, new RTLightingFeatGLSL );
   FEATUREMGR->registerFeature( MFT_IsDXTnm, new NamedFeatureGLSL( "DXTnm" ) );
   FEATUREMGR->registerFeature( MFT_TexAnim, new TexAnimGLSL );
   FEATUREMGR->registerFeature( MFT_DiffuseMap, new DiffuseMapFeatGLSL );
   FEATUREMGR->registerFeature( MFT_OverlayMap, new OverlayTexFeatGLSL );
   FEATUREMGR->registerFeature( MFT_DiffuseColor, new DiffuseFeatureGLSL );
   FEATUREMGR->registerFeature( MFT_DiffuseVertColor, new DiffuseVertColorFeatureGLSL );
   FEATUREMGR->registerFeature( MFT_AlphaTest, new AlphaTestGLSL );
   FEATUREMGR->registerFeature( MFT_GlowMask, new GlowMaskGLSL );
   FEATUREMGR->registerFeature( MFT_LightMap, new LightmapFeatGLSL );
   FEATUREMGR->registerFeature( MFT_ToneMap, new TonemapFeatGLSL );
   FEATUREMGR->registerFeature( MFT_VertLit, new VertLitGLSL );
	FEATUREMGR->registerFeature( MFT_Parallax, new ParallaxFeatGLSL );
   FEATUREMGR->registerFeature( MFT_NormalMap, new BumpFeatGLSL );
	FEATUREMGR->registerFeature( MFT_DetailNormalMap, new NamedFeatureGLSL( "Detail Normal Map" ) );
   FEATUREMGR->registerFeature( MFT_DetailMap, new DetailFeatGLSL );
   FEATUREMGR->registerFeature( MFT_CubeMap, new ReflectCubeFeatGLSL );
   FEATUREMGR->registerFeature( MFT_PixSpecular, new PixelSpecularGLSL );
   FEATUREMGR->registerFeature( MFT_SpecularMap, new SpecularMapGLSL );
   FEATUREMGR->registerFeature( MFT_AccuMap, new AccuTexFeatGLSL );
   FEATUREMGR->registerFeature( MFT_GlossMap, new NamedFeatureGLSL( "Gloss Map" ) );
   FEATUREMGR->registerFeature( MFT_IsTranslucent, new NamedFeatureGLSL( "Translucent" ) );
   FEATUREMGR->registerFeature( MFT_IsTranslucentZWrite, new NamedFeatureGLSL( "Translucent ZWrite" ) );
   FEATUREMGR->registerFeature( MFT_Visibility, new VisibilityFeatGLSL );
   FEATUREMGR->registerFeature( MFT_Fog, new FogFeatGLSL );
   FEATUREMGR->registerFeature( MFT_LightbufferMRT, new NamedFeatureGLSL( "Lightbuffer MRT" ) );
   FEATUREMGR->registerFeature( MFT_RenderTarget1_Zero, new RenderTargetZeroGLSL( ShaderFeature::RenderTarget1 ) );
   FEATUREMGR->registerFeature( MFT_RenderTarget2_Zero, new RenderTargetZeroGLSL( ShaderFeature::RenderTarget2 ) );
   FEATUREMGR->registerFeature( MFT_RenderTarget3_Zero, new RenderTargetZeroGLSL( ShaderFeature::RenderTarget3 ) );
   FEATUREMGR->registerFeature( MFT_Imposter, new NamedFeatureGLSL( "Imposter" ) );

	FEATUREMGR->registerFeature( MFT_NormalsOut, new NormalsOutFeatGLSL );
	
   FEATUREMGR->registerFeature( MFT_DepthOut, new DepthOutGLSL );
   FEATUREMGR->registerFeature(MFT_EyeSpaceDepthOut, new EyeSpaceDepthOutGLSL());

	FEATUREMGR->registerFeature( MFT_HDROut, new HDROutGLSL );
	
   FEATUREMGR->registerFeature( MFT_ParaboloidVertTransform, new ParaboloidVertTransformGLSL );
   FEATUREMGR->registerFeature( MFT_IsSinglePassParaboloid, new NamedFeatureGLSL( "Single Pass Paraboloid" ) );
   FEATUREMGR->registerFeature( MFT_UseInstancing, new NamedFeatureGLSL( "Hardware Instancing" ) );
	
   FEATUREMGR->registerFeature( MFT_DiffuseMapAtlas, new NamedFeatureGLSL( "Diffuse Map Atlas" ) );
   FEATUREMGR->registerFeature( MFT_NormalMapAtlas, new NamedFeatureGLSL( "Normal Map Atlas" ) );

	FEATUREMGR->registerFeature( MFT_Foliage, new FoliageFeatureGLSL );
	
	FEATUREMGR->registerFeature( MFT_ParticleNormal, new ParticleNormalFeatureGLSL );
   FEATUREMGR->registerFeature( MFT_ForwardShading, new NamedFeatureGLSL( "Forward Shaded Material" ) );

   FEATUREMGR->registerFeature( MFT_ImposterVert, new ImposterVertFeatureGLSL );

   // Deferred Shading
   FEATUREMGR->registerFeature( MFT_isDeferred, new NamedFeatureGLSL( "Deferred Material" ) );
   FEATUREMGR->registerFeature( MFT_DeferredSpecMap, new DeferredSpecMapGLSL );
   FEATUREMGR->registerFeature( MFT_DeferredSpecVars, new DeferredSpecVarsGLSL );
   FEATUREMGR->registerFeature( MFT_DeferredMatInfoFlags, new DeferredMatInfoFlagsGLSL );
   FEATUREMGR->registerFeature( MFT_DeferredEmptySpec, new DeferredEmptySpecGLSL );
   FEATUREMGR->registerFeature( MFT_SkyBox, new NamedFeatureGLSL( "skybox" ) );
   FEATUREMGR->registerFeature( MFT_HardwareSkinning, new HardwareSkinningFeatureGLSL );
}

MODULE_BEGIN( ShaderGenGLSL )

   MODULE_INIT_AFTER( ShaderGen )
   MODULE_INIT_AFTER( ShaderGenFeatureMgr )
   
   MODULE_INIT
   {
      sInitDelegate.bind( &_initShaderGenGLSL );
      SHADERGEN->registerInitDelegate(OpenGL, sInitDelegate);   
   }
   
MODULE_END;
