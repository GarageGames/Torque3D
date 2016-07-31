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
#include "shaderGen/HLSL/shaderGenHLSL.h"
#include "shaderGen/HLSL/shaderFeatureHLSL.h"
#include "shaderGen/featureMgr.h"
#include "shaderGen/HLSL/bumpHLSL.h"
#include "shaderGen/HLSL/pixSpecularHLSL.h"
#include "shaderGen/HLSL/depthHLSL.h"
#include "shaderGen/HLSL/paraboloidHLSL.h"
#include "materials/materialFeatureTypes.h"
#include "core/module.h"
// Deferred Shading
#include "lighting/advanced/hlsl/deferredShadingFeaturesHLSL.h"
#include "shaderGen/HLSL/accuFeatureHLSL.h"

static ShaderGen::ShaderGenInitDelegate sInitDelegate;

void _initShaderGenHLSL( ShaderGen *shaderGen )
{
   shaderGen->setPrinter( new ShaderGenPrinterHLSL );
   shaderGen->setComponentFactory( new ShaderGenComponentFactoryHLSL );
   shaderGen->setFileEnding( "hlsl" );

   FEATUREMGR->registerFeature( MFT_VertTransform, new VertPositionHLSL );
   FEATUREMGR->registerFeature( MFT_RTLighting, new RTLightingFeatHLSL );
   FEATUREMGR->registerFeature( MFT_IsDXTnm, new NamedFeatureHLSL( "DXTnm" ) );
   FEATUREMGR->registerFeature( MFT_TexAnim, new TexAnimHLSL );
   FEATUREMGR->registerFeature( MFT_DiffuseMap, new DiffuseMapFeatHLSL );
   FEATUREMGR->registerFeature( MFT_OverlayMap, new OverlayTexFeatHLSL );
   FEATUREMGR->registerFeature( MFT_DiffuseColor, new DiffuseFeatureHLSL );
   FEATUREMGR->registerFeature( MFT_DiffuseVertColor, new DiffuseVertColorFeatureHLSL );
   FEATUREMGR->registerFeature( MFT_AlphaTest, new AlphaTestHLSL );
   FEATUREMGR->registerFeature( MFT_GlowMask, new GlowMaskHLSL );
   FEATUREMGR->registerFeature( MFT_LightMap, new LightmapFeatHLSL );
   FEATUREMGR->registerFeature( MFT_ToneMap, new TonemapFeatHLSL );
   FEATUREMGR->registerFeature( MFT_VertLit, new VertLitHLSL );
   FEATUREMGR->registerFeature( MFT_Parallax, new ParallaxFeatHLSL );
   FEATUREMGR->registerFeature( MFT_NormalMap, new BumpFeatHLSL );
   FEATUREMGR->registerFeature( MFT_DetailNormalMap, new NamedFeatureHLSL( "Detail Normal Map" ) );
   FEATUREMGR->registerFeature( MFT_DetailMap, new DetailFeatHLSL );
   FEATUREMGR->registerFeature( MFT_CubeMap, new ReflectCubeFeatHLSL );
   FEATUREMGR->registerFeature( MFT_PixSpecular, new PixelSpecularHLSL );
   FEATUREMGR->registerFeature( MFT_IsTranslucent, new NamedFeatureHLSL( "Translucent" ) );
   FEATUREMGR->registerFeature( MFT_IsTranslucentZWrite, new NamedFeatureHLSL( "Translucent ZWrite" ) );
   FEATUREMGR->registerFeature( MFT_Visibility, new VisibilityFeatHLSL );
   FEATUREMGR->registerFeature( MFT_Fog, new FogFeatHLSL );
   FEATUREMGR->registerFeature( MFT_SpecularMap, new SpecularMapHLSL );
   FEATUREMGR->registerFeature( MFT_AccuMap, new AccuTexFeatHLSL );
   FEATUREMGR->registerFeature( MFT_GlossMap, new NamedFeatureHLSL( "Gloss Map" ) );
   FEATUREMGR->registerFeature( MFT_LightbufferMRT, new NamedFeatureHLSL( "Lightbuffer MRT" ) );
   FEATUREMGR->registerFeature( MFT_RenderTarget1_Zero, new RenderTargetZeroHLSL( ShaderFeature::RenderTarget1 ) );
   FEATUREMGR->registerFeature( MFT_RenderTarget2_Zero, new RenderTargetZeroHLSL( ShaderFeature::RenderTarget2 ) );
   FEATUREMGR->registerFeature( MFT_RenderTarget3_Zero, new RenderTargetZeroHLSL( ShaderFeature::RenderTarget3 ) );
   FEATUREMGR->registerFeature( MFT_Imposter, new NamedFeatureHLSL( "Imposter" ) );

   FEATUREMGR->registerFeature( MFT_DiffuseMapAtlas, new NamedFeatureHLSL( "Diffuse Map Atlas" ) );
   FEATUREMGR->registerFeature( MFT_NormalMapAtlas, new NamedFeatureHLSL( "Normal Map Atlas" ) );

   FEATUREMGR->registerFeature( MFT_NormalsOut, new NormalsOutFeatHLSL );
   
   FEATUREMGR->registerFeature( MFT_DepthOut, new DepthOutHLSL );
   FEATUREMGR->registerFeature( MFT_EyeSpaceDepthOut, new EyeSpaceDepthOutHLSL() );

   FEATUREMGR->registerFeature( MFT_HDROut, new HDROutHLSL );

   FEATUREMGR->registerFeature( MFT_ParaboloidVertTransform, new ParaboloidVertTransformHLSL );
   FEATUREMGR->registerFeature( MFT_IsSinglePassParaboloid, new NamedFeatureHLSL( "Single Pass Paraboloid" ) );
   FEATUREMGR->registerFeature( MFT_UseInstancing, new NamedFeatureHLSL( "Hardware Instancing" ) );

   FEATUREMGR->registerFeature( MFT_Foliage, new FoliageFeatureHLSL );

   FEATUREMGR->registerFeature( MFT_ParticleNormal, new ParticleNormalFeatureHLSL );

   FEATUREMGR->registerFeature( MFT_InterlacedPrePass, new NamedFeatureHLSL( "Interlaced Pre Pass" ) );

   FEATUREMGR->registerFeature( MFT_ForwardShading, new NamedFeatureHLSL( "Forward Shaded Material" ) );

   FEATUREMGR->registerFeature( MFT_ImposterVert, new ImposterVertFeatureHLSL );

   // Deferred Shading
   FEATUREMGR->registerFeature( MFT_isDeferred, new NamedFeatureHLSL( "Deferred Material" ) );
   FEATUREMGR->registerFeature( MFT_DeferredSpecMap, new DeferredSpecMapHLSL );
   FEATUREMGR->registerFeature( MFT_DeferredSpecVars, new DeferredSpecVarsHLSL );
   FEATUREMGR->registerFeature( MFT_DeferredMatInfoFlags, new DeferredMatInfoFlagsHLSL );
   FEATUREMGR->registerFeature( MFT_DeferredEmptySpec, new DeferredEmptySpecHLSL );
   FEATUREMGR->registerFeature( MFT_SkyBox,  new NamedFeatureHLSL( "skybox" ) );
}

MODULE_BEGIN( ShaderGenHLSL )

   MODULE_INIT_AFTER( ShaderGen )
   MODULE_INIT_AFTER( ShaderGenFeatureMgr )
   
   MODULE_INIT
   {
      sInitDelegate.bind(_initShaderGenHLSL);
      SHADERGEN->registerInitDelegate(Direct3D9, sInitDelegate);
      SHADERGEN->registerInitDelegate(Direct3D9_360, sInitDelegate);
      SHADERGEN->registerInitDelegate(Direct3D11, sInitDelegate);
   }
   
MODULE_END;
