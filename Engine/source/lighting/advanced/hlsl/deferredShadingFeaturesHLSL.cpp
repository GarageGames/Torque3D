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
#include "lighting/advanced/hlsl/deferredShadingFeaturesHLSL.h"

#include "lighting/advanced/advancedLightBinManager.h"
#include "shaderGen/langElement.h"
#include "shaderGen/shaderOp.h"
#include "shaderGen/conditionerFeature.h"
#include "renderInstance/renderPrePassMgr.h"
#include "materials/processedMaterial.h"
#include "materials/materialFeatureTypes.h"


//****************************************************************************
// Deferred Shading Features
//****************************************************************************

// Diffuse Map -> Color Buffer
void DeferredDiffuseMapHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // grab connector texcoord register
   Var *inTex = getInTexCoord( "texCoord", "float2", true, componentList );

   // create texture var
   Var *diffuseMap = new Var;
   diffuseMap->setType( "sampler2D" );
   diffuseMap->setName( "diffuseMap" );
   diffuseMap->uniform = true;
   diffuseMap->sampler = true;
   diffuseMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

   if (  fd.features[MFT_CubeMap] )
   {
      MultiLine * meta = new MultiLine;
      
      // create sample color
      Var *diffColor = new Var;
      diffColor->setType( "float4" );
      diffColor->setName( "diffuseColor" );
      LangElement *colorDecl = new DecOp( diffColor );
   
      meta->addStatement(  new GenOp( "   @ = tex2D(@, @);\r\n", 
                           colorDecl, 
                           diffuseMap, 
                           inTex ) );
      
      meta->addStatement( new GenOp( "   @;\r\n", assignColor( diffColor, Material::Mul , NULL, ShaderFeature::RenderTarget1 ) ) );
      output = meta;
   }
   else if(fd.features[MFT_DiffuseMapAtlas])
   {   
      // Handle atlased textures
      // http://www.infinity-universe.com/Infinity/index.php?option=com_content&task=view&id=65&Itemid=47
      MultiLine * meta = new MultiLine;
      output = meta;

      Var *atlasedTex = new Var;
      atlasedTex->setName("atlasedTexCoord");
      atlasedTex->setType("float2");
      LangElement *atDecl = new DecOp(atlasedTex);

      // Parameters of the texture atlas
      Var *atParams  = new Var;
      atParams->setType("float4");
      atParams->setName("diffuseAtlasParams");
      atParams->uniform = true;
      atParams->constSortPos = cspPotentialPrimitive;

      // Parameters of the texture (tile) this object is using in the atlas
      Var *tileParams  = new Var;
      tileParams->setType("float4");
      tileParams->setName("diffuseAtlasTileParams");
      tileParams->uniform = true;
      tileParams->constSortPos = cspPotentialPrimitive;

      const bool is_sm3 = (GFX->getPixelShaderVersion() > 2.0f);
      if(is_sm3)
      {
         // Figure out the mip level
         meta->addStatement(new GenOp("   float2 _dx = ddx(@ * @.z);\r\n", inTex, atParams));
         meta->addStatement(new GenOp("   float2 _dy = ddy(@ * @.z);\r\n", inTex, atParams));
         meta->addStatement(new GenOp("   float mipLod = 0.5 * log2(max(dot(_dx, _dx), dot(_dy, _dy)));\r\n"));
         meta->addStatement(new GenOp("   mipLod = clamp(mipLod, 0.0, @.w);\r\n", atParams));

         // And the size of the mip level
         meta->addStatement(new GenOp("   float mipPixSz = pow(2.0, @.w - mipLod);\r\n", atParams));
         meta->addStatement(new GenOp("   float2 mipSz = mipPixSz / @.xy;\r\n", atParams));
      }
      else
      {
         meta->addStatement(new GenOp("   float2 mipSz = float2(1.0, 1.0);\r\n"));
      }

      // Tiling mode
      // TODO: Select wrap or clamp somehow
      if( true ) // Wrap
         meta->addStatement(new GenOp("   @ = frac(@);\r\n", atDecl, inTex));
      else       // Clamp
         meta->addStatement(new GenOp("   @ = saturate(@);\r\n", atDecl, inTex));

      // Finally scale/offset, and correct for filtering
      meta->addStatement(new GenOp("   @ = @ * ((mipSz * @.xy - 1.0) / mipSz) + 0.5 / mipSz + @.xy * @.xy;\r\n", 
         atlasedTex, atlasedTex, atParams, atParams, tileParams));

      // Add a newline
      meta->addStatement(new GenOp( "\r\n"));

      // For the rest of the feature...
      inTex = atlasedTex;

      // create sample color var
      Var *diffColor = new Var;
      diffColor->setType("float4");
      diffColor->setName("diffuseColor");

      // To dump out UV coords...
//#define DEBUG_ATLASED_UV_COORDS
#ifdef DEBUG_ATLASED_UV_COORDS
      if(!fd.features[MFT_PrePassConditioner])
      {
         meta->addStatement(new GenOp("   @ = float4(@.xy, mipLod / @.w, 1.0);\r\n", new DecOp(diffColor), inTex, atParams));
         meta->addStatement(new GenOp("   @; return OUT;\r\n", assignColor(diffColor, Material::Mul)));
         return;
      }
#endif

      if(is_sm3)
      {
         meta->addStatement(new GenOp( "   @ = tex2Dlod(@, float4(@, 0.0, mipLod));\r\n", 
            new DecOp(diffColor), diffuseMap, inTex));
      }
      else
      {
         meta->addStatement(new GenOp( "   @ = tex2D(@, @);\r\n", 
            new DecOp(diffColor), diffuseMap, inTex));
      }

      meta->addStatement(new GenOp( "   @;\r\n", assignColor(diffColor, Material::Mul, NULL, ShaderFeature::RenderTarget1)));
   }
   else
   {
       LangElement *statement = new GenOp( "tex2D(@, @)", diffuseMap, inTex );
       output = new GenOp( "   @;\r\n", assignColor( statement, Material::None, NULL, ShaderFeature::RenderTarget1 ) );
   }
}

ShaderFeature::Resources DeferredDiffuseMapHLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 
   res.numTex = 1;
   res.numTexReg = 1;

   return res;
}

void DeferredDiffuseMapHLSL::setTexData(   Material::StageData &stageDat,
                                       const MaterialFeatureData &fd,
                                       RenderPassData &passData,
                                       U32 &texIndex )
{
   GFXTextureObject *tex = stageDat.getTex( MFT_DiffuseMap );
   if ( tex )
      passData.mTexSlot[ texIndex++ ].texObject = tex;
}

void DeferredDiffuseMapHLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;
   getOutTexCoord(   "texCoord", 
                     "float2", 
                     true, 
                     fd.features[MFT_TexAnim], 
                     meta, 
                     componentList );
   output = meta;
}

// Diffuse Color -> Color Buffer
void DeferredDiffuseColorHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   Var *diffuseMaterialColor  = new Var;
   diffuseMaterialColor->setType( "float4" );
   diffuseMaterialColor->setName( "diffuseMaterialColor" );
   diffuseMaterialColor->uniform = true;
   diffuseMaterialColor->constSortPos = cspPotentialPrimitive;

   MultiLine * meta = new MultiLine;
   meta->addStatement( new GenOp( "   @;\r\n", assignColor( diffuseMaterialColor, Material::Mul, NULL, ShaderFeature::RenderTarget1 ) ) );
   output = meta;
}

// Empty Color -> Color Buffer
void DeferredEmptyColorHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   output = new GenOp( "   @;\r\n", assignColor( new GenOp( "1.0" ), Material::None, NULL, ShaderFeature::RenderTarget1 ) );
}

// Specular Map -> Alpha of Color Buffer ( greyscaled )
void DeferredSpecMapHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // Get the texture coord.
   Var *texCoord = getInTexCoord( "texCoord", "float2", true, componentList );

   // search for color var
   Var *color = (Var*) LangElement::find( getOutputTargetVarName(ShaderFeature::RenderTarget1) );
   if ( !color )
   {
      // create color var
      color = new Var;
      color->setType( "fragout" );
      color->setName( getOutputTargetVarName(ShaderFeature::RenderTarget1) );
      color->setStructName( "OUT" );
   }

   // create texture var
   Var *specularMap = new Var;
   specularMap->setType( "sampler2D" );
   specularMap->setName( "specularMap" );
   specularMap->uniform = true;
   specularMap->sampler = true;
   specularMap->constNum = Var::getTexUnitNum();
   LangElement *texOp = new GenOp( "tex2D(@, @)", specularMap, texCoord );

   output = new GenOp( "   @.a = dot(tex2D(@, @).rgb, float3(0.3, 0.59, 0.11));\r\n", color, specularMap, texCoord );
}

ShaderFeature::Resources DeferredSpecMapHLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 
   res.numTex = 1;
   res.numTexReg = 1;

   return res;
}

void DeferredSpecMapHLSL::setTexData(   Material::StageData &stageDat,
                                       const MaterialFeatureData &fd,
                                       RenderPassData &passData,
                                       U32 &texIndex )
{
   GFXTextureObject *tex = stageDat.getTex( MFT_SpecularMap );
   if ( tex )
   {
      passData.mTexType[ texIndex ] = Material::Standard;
      passData.mTexSlot[ texIndex++ ].texObject = tex;
   }
}

void DeferredSpecMapHLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;
   getOutTexCoord(   "texCoord", 
                     "float2", 
                     true, 
                     fd.features[MFT_TexAnim], 
                     meta, 
                     componentList );
   output = meta;
}

// Specular Color -> Alpha of Color Buffer ( greyscaled )
void DeferredSpecColorHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   Var *specularColor = (Var*)LangElement::find( "specularColor" );
   if ( !specularColor )
   {
      specularColor  = new Var( "specularColor", "float4" );
      specularColor->uniform = true;
      specularColor->constSortPos = cspPotentialPrimitive;
   }
   
   output = new GenOp( "   @;\r\n", assignColor( specularColor, Material::None, NULL,  ShaderFeature::RenderTarget1 ) );
}

// Black -> Alpha of Color Buffer (representing no specular)
void DeferredEmptySpecHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // Get the texture coord.
   Var *texCoord = getInTexCoord( "texCoord", "float2", true, componentList );

   // search for color var
   Var *color = (Var*) LangElement::find( getOutputTargetVarName(ShaderFeature::RenderTarget1) );
   if ( !color )
   {
      // create color var
      color = new Var;
      color->setType( "fragout" );
      color->setName( getOutputTargetVarName(ShaderFeature::RenderTarget1) );
      color->setStructName( "OUT" );
   }

   output = new GenOp( "   @.a = 0.0;\r\n", color );
}

// Gloss Map (Alpha Channel of Specular Map) -> Blue ( Spec Power ) of Material Info Buffer.
void DeferredGlossMapHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // Get the texture coord.
   Var *texCoord = getInTexCoord( "texCoord", "float2", true, componentList );

   // search for color var
   Var *color = (Var*) LangElement::find( getOutputTargetVarName(ShaderFeature::RenderTarget2) );
   if ( !color )
   {
      // create color var
      color = new Var;
      color->setType( "fragout" );
      color->setName( getOutputTargetVarName(ShaderFeature::RenderTarget2) );
      color->setStructName( "OUT" );
   }

   // create texture var
   Var *specularMap = (Var*)LangElement::find( "specularMap" );
   if (!specularMap)
   {
       specularMap->setType( "sampler2D" );
       specularMap->setName( "specularMap" );
       specularMap->uniform = true;
       specularMap->sampler = true;
       specularMap->constNum = Var::getTexUnitNum();
   }
   LangElement *texOp = new GenOp( "tex2D(@, @)", specularMap, texCoord );

   output = new GenOp( "   @.b = @.a;\r\n", color, texOp );
}

// Material Info Flags -> Red ( Flags ) of Material Info Buffer.
void DeferredMatInfoFlagsHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // search for color var
   Var *color = (Var*) LangElement::find( getOutputTargetVarName(ShaderFeature::RenderTarget2) );
   if ( !color )
   {
      // create color var
      color = new Var;
      color->setType( "fragout" );
      color->setName( getOutputTargetVarName(ShaderFeature::RenderTarget2) );
      color->setStructName( "OUT" );
   }

   Var *matInfoFlags = new Var;
   matInfoFlags->setType( "float" );
   matInfoFlags->setName( "matInfoFlags" );
   matInfoFlags->uniform = true;
   matInfoFlags->constSortPos = cspPotentialPrimitive;

   output = new GenOp( "   @.r = @;\r\n", color, matInfoFlags );
}

// Spec Strength -> Alpha Channel of Material Info Buffer.
void DeferredSpecStrengthHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // search for color var
   Var *color = (Var*) LangElement::find( getOutputTargetVarName(ShaderFeature::RenderTarget2) );
   if ( !color )
   {
      // create color var
      color = new Var;
      color->setType( "fragout" );
      color->setName( getOutputTargetVarName(ShaderFeature::RenderTarget2) );
      color->setStructName( "OUT" );
   }

   Var *specStrength = new Var;
   specStrength->setType( "float" );
   specStrength->setName( "specularStrength" );
   specStrength->uniform = true;
   specStrength->constSortPos = cspPotentialPrimitive;

   output = new GenOp( "   @.a = @ / 5.0;\r\n", color, specStrength );
}

// Spec Power -> Blue Channel ( of Material Info Buffer.
void DeferredSpecPowerHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // search for color var
   Var *color = (Var*) LangElement::find( getOutputTargetVarName(ShaderFeature::RenderTarget2) );
   if ( !color )
   {
      // create color var
      color = new Var;
      color->setType( "fragout" );
      color->setName( getOutputTargetVarName(ShaderFeature::RenderTarget2) );
      color->setStructName( "OUT" );
   }

   Var *specPower = new Var;
   specPower->setType( "float" );
   specPower->setName( "specularPower" );
   specPower->uniform = true;
   specPower->constSortPos = cspPotentialPrimitive;

   output = new GenOp( "   @ = float4(0.0, 0.0, @ / 128.0, 0.0);\r\n", color, specPower );
}

// Emissive -> Material Info Buffer.
void DeferredEmissiveHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   output = new GenOp( "   @;\r\n", assignColor( new GenOp( "float4(1.0, 0.0, 0.0, 0.0)" ), Material::None, NULL, ShaderFeature::RenderTarget2 ) );
}

// Tranlucency -> Green of Material Info Buffer.
void DeferredTranslucencyMapHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // Get the texture coord.
   Var *texCoord = getInTexCoord( "texCoord", "float2", true, componentList );

   // search for color var
   Var *color = (Var*) LangElement::find( getOutputTargetVarName(ShaderFeature::RenderTarget2) );
   if ( !color )
   {
      // create color var
      color = new Var;
      color->setType( "fragout" );
      color->setName( getOutputTargetVarName(ShaderFeature::RenderTarget2) );
      color->setStructName( "OUT" );
   }

   // create texture var
   Var *translucencyMap = new Var;
   translucencyMap->setType( "sampler2D" );
   translucencyMap->setName( "translucencyMap" );
   translucencyMap->uniform = true;
   translucencyMap->sampler = true;
   translucencyMap->constNum = Var::getTexUnitNum();

   output = new GenOp( "   @.g = dot(tex2D(@, @).rgb, float3(0.3, 0.59, 0.11));\r\n", color, translucencyMap, texCoord );
   
}

ShaderFeature::Resources DeferredTranslucencyMapHLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 
   res.numTex = 1;
   res.numTexReg = 1;

   return res;
}

void DeferredTranslucencyMapHLSL::setTexData(   Material::StageData &stageDat,
                                       const MaterialFeatureData &fd,
                                       RenderPassData &passData,
                                       U32 &texIndex )
{
   GFXTextureObject *tex = stageDat.getTex( MFT_TranslucencyMap );
   if ( tex )
   {
      passData.mTexType[ texIndex ] = Material::Standard;
      passData.mTexSlot[ texIndex++ ].texObject = tex;
   }
}

//****************************************************************************
// Vertex position
//****************************************************************************
void DeferredSkyHLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                    const MaterialFeatureData &fd )
{
   Var *outPosition = (Var*)LangElement::find( "hpos" );
   MultiLine *meta = new MultiLine;
   meta->addStatement( new GenOp( "   @.w = @.z;\r\n", outPosition, outPosition ) );

   output = meta;
}