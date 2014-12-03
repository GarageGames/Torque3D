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
#include "terrain/glsl/terrFeatureGLSL.h"

#include "terrain/terrFeatureTypes.h"
#include "materials/materialFeatureTypes.h"
#include "materials/materialFeatureData.h"
#include "gfx/gfxDevice.h"
#include "shaderGen/langElement.h"
#include "shaderGen/shaderOp.h"
#include "shaderGen/featureMgr.h"
#include "shaderGen/shaderGen.h"
#include "core/module.h"

namespace 
{
   void register_glsl_shader_features_for_terrain(GFXAdapterType type)
   {
      if(type != OpenGL)
         return;

      FEATUREMGR->registerFeature( MFT_TerrainBaseMap, new TerrainBaseMapFeatGLSL );
      FEATUREMGR->registerFeature( MFT_TerrainParallaxMap, new NamedFeatureGLSL( "Terrain Parallax Texture" ) );   
      FEATUREMGR->registerFeature( MFT_TerrainDetailMap, new TerrainDetailMapFeatGLSL );
      FEATUREMGR->registerFeature( MFT_TerrainNormalMap, new TerrainNormalMapFeatGLSL );
      FEATUREMGR->registerFeature( MFT_TerrainMacroMap, new TerrainMacroMapFeatGLSL );
      FEATUREMGR->registerFeature( MFT_TerrainLightMap, new TerrainLightMapFeatGLSL );
      FEATUREMGR->registerFeature( MFT_TerrainSideProject, new NamedFeatureGLSL( "Terrain Side Projection" ) );
      FEATUREMGR->registerFeature( MFT_TerrainAdditive, new TerrainAdditiveFeatGLSL );     
   }

};

MODULE_BEGIN( TerrainFeatGLSL )

   MODULE_INIT_AFTER( ShaderGen )

   MODULE_INIT
   {      
      SHADERGEN->getFeatureInitSignal().notify(&register_glsl_shader_features_for_terrain);
   }

MODULE_END;


Var* TerrainFeatGLSL::_getUniformVar( const char *name, const char *type, ConstantSortPosition csp )
{
   Var *theVar = (Var*)LangElement::find( name );
   if ( !theVar )
   {
      theVar = new Var;
      theVar->setType( type );
      theVar->setName( name );
      theVar->uniform = true;
      theVar->constSortPos = csp;
   }
   
   return theVar;
}

Var* TerrainFeatGLSL::_getInDetailCoord( Vector<ShaderComponent*> &componentList )
{
   String name( String::ToString( "detCoord%d", getProcessIndex() ) );
   Var *inDet = (Var*)LangElement::find( name );
   
   if ( !inDet )
   {
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
      
      inDet = connectComp->getElement( RT_TEXCOORD );
      inDet->setName( name );
      inDet->setStructName( "IN" );
      inDet->setType( "vec4" );
      inDet->mapsToSampler = true;
   }
   
   return inDet;
}

Var* TerrainFeatGLSL::_getInMacroCoord( Vector<ShaderComponent*> &componentList )
{
   String name( String::ToString( "macroCoord%d", getProcessIndex() ) );
   Var *inDet = (Var*)LangElement::find( name );

   if ( !inDet )
   {
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );

      inDet = connectComp->getElement( RT_TEXCOORD );
      inDet->setName( name );
      inDet->setStructName( "IN" );
      inDet->setType( "vec4" );
      inDet->mapsToSampler = true;
   }

   return inDet;
}

Var* TerrainFeatGLSL::_getNormalMapTex()
{
   String name( String::ToString( "normalMap%d", getProcessIndex() ) );
   Var *normalMap =  (Var*)LangElement::find( name );
   
   if ( !normalMap )
   {
      normalMap = new Var;
      normalMap->setType( "sampler2D" );
      normalMap->setName( name );
      normalMap->uniform = true;
      normalMap->sampler = true;
      normalMap->constNum = Var::getTexUnitNum();
   }
   
   return normalMap;
}

Var* TerrainFeatGLSL::_getDetailIdStrengthParallax()
{
   String name( String::ToString( "detailIdStrengthParallax%d", getProcessIndex() ) );
   
   Var *detailInfo = (Var*)LangElement::find( name );
   if ( !detailInfo )
   {
      detailInfo = new Var;
      detailInfo->setType( "vec3" );
      detailInfo->setName( name );
      detailInfo->uniform = true;
      detailInfo->constSortPos = cspPotentialPrimitive;
   }
   
   return detailInfo;
}

Var* TerrainFeatGLSL::_getMacroIdStrengthParallax()
{
   String name( String::ToString( "macroIdStrengthParallax%d", getProcessIndex() ) );

   Var *detailInfo = (Var*)LangElement::find( name );
   if ( !detailInfo )
   {
      detailInfo = new Var;
      detailInfo->setType( "vec3" );
      detailInfo->setName( name );
      detailInfo->uniform = true;
      detailInfo->constSortPos = cspPotentialPrimitive;
   }

   return detailInfo;
}


void TerrainBaseMapFeatGLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                          const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;
   output = meta;

   // Generate the incoming texture var.
   Var *inTex;
   {
      Var *inPos = (Var*)LangElement::find( "inPosition" );
      if ( !inPos )
         inPos = (Var*)LangElement::find( "position" );

      inTex = new Var( "texCoord", "vec3" );

      Var *oneOverTerrainSize = _getUniformVar( "oneOverTerrainSize", "float", cspPass );

      // NOTE: The y coord here should be negative to have
      // the texture maps not end up flipped which also caused
      // normal and parallax mapping to be incorrect.
      //
      // This mistake early in development means that the layer
      // id bilinear blend depends on it being that way.
      //
      // So instead i fixed this by flipping the base and detail
      // coord y scale to compensate when rendering.
      //
      meta->addStatement( new GenOp( "   @ = @.xyz * float3( @, @, -@ );\r\n", 
         new DecOp( inTex ), inPos, oneOverTerrainSize, oneOverTerrainSize, oneOverTerrainSize ) );
   }

   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );

   // Pass the texture coord to the pixel shader.
   Var *outTex = connectComp->getElement( RT_TEXCOORD );
   outTex->setName( "outTexCoord" );
   outTex->setStructName( "OUT" );
   outTex->setType( "vec3" );
   outTex->mapsToSampler = true;
   meta->addStatement( new GenOp( "   @.xy = @.xy;\r\n", outTex, inTex ) );

   // If this shader has a side projected layer then we 
   // pass the dot product between the +Y and the normal
   // thru outTexCoord.z for use in blending the textures.
   if ( fd.features.hasFeature( MFT_TerrainSideProject ) )
   {
      Var *inNormal = (Var*)LangElement::find( "normal" );
      meta->addStatement( 
         new GenOp( "   @.z = pow( abs( dot( normalize( float3( @.x, @.y, 0 ) ), float3( 0, 1, 0 ) ) ), 10.0 );\r\n", 
            outTex, inNormal, inNormal ) );
   }
   else
      meta->addStatement( new GenOp( "   @.z = 0;\r\n", outTex ) );

   // HACK: This is sort of lazy... we generate the tanget
   // vector here so that we're sure it exists in the parallax
   // and normal features which will expect "T" to exist.
   //
   // If this shader doesn't use it the shader compiler will
   // optimize away this code.
   //
   Var *inTangentZ = getVertTexCoord( "tcTangentZ" );
   Var *inTanget = new Var( "T", "vec3" );
   Var *squareSize = _getUniformVar( "squareSize", "float", cspPass );
   meta->addStatement( new GenOp( "   @ = normalize( float3( @, 0, @ ) );\r\n", 
      new DecOp( inTanget ), squareSize, inTangentZ ) );  
}

void TerrainBaseMapFeatGLSL::processPix(  Vector<ShaderComponent*> &componentList, 
                                          const MaterialFeatureData &fd )
{
   // grab connector texcoord register
   Var *texCoord = getInTexCoord( "texCoord", "vec3", true, componentList );

   // We do nothing more if this is a prepass.
   if ( fd.features.hasFeature( MFT_PrePassConditioner ) )
      return;

   // create texture var
   Var *diffuseMap = new Var;
   diffuseMap->setType( "sampler2D" );
   diffuseMap->setName( "baseTexMap" );
   diffuseMap->uniform = true;
   diffuseMap->sampler = true;
   diffuseMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

   MultiLine *meta = new MultiLine;

   Var *baseColor = new Var;
   baseColor->setType( "vec4" );
   baseColor->setName( "baseColor" );
   meta->addStatement( new GenOp( "   @ = tex2D( @, @.xy );\r\n", new DecOp( baseColor ), diffuseMap, texCoord ) );
   meta->addStatement( new GenOp( "   @;\r\n", assignColor( baseColor, Material::Mul ) ) );

   output = meta;
}

ShaderFeature::Resources TerrainBaseMapFeatGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 
   res.numTexReg = 1;

   // We only sample from the base map during a diffuse pass.
   if ( !fd.features.hasFeature( MFT_PrePassConditioner ) )
      res.numTex = 1;

   return res;
}

TerrainDetailMapFeatGLSL::TerrainDetailMapFeatGLSL()
   :  mTorqueDep( "shaders/common/gl/torque.glsl" ),
      mTerrainDep( "shaders/common/terrain/terrain.glsl" )
      
{
   addDependency( &mTorqueDep );
   addDependency( &mTerrainDep );
}

void TerrainDetailMapFeatGLSL::processVert(  Vector<ShaderComponent*> &componentList, 
                                             const MaterialFeatureData &fd )
{
   const U32 detailIndex = getProcessIndex();

   // Grab incoming texture coords... the base map feature
   // made sure this was created.
   Var *inTex = (Var*)LangElement::find( "texCoord" );
   AssertFatal( inTex, "The texture coord is missing!" );

   // Grab the input position.
   Var *inPos = (Var*)LangElement::find( "inPosition" );
   if ( !inPos )
      inPos = (Var*)LangElement::find( "position" );

   // Get the object space eye position.
   Var *eyePos = _getUniformVar( "eyePos", "vec3", cspPotentialPrimitive );

   MultiLine *meta = new MultiLine;

   // If we have parallax mapping then make sure we've sent
   // the negative view vector to the pixel shader.
   if (  fd.features.hasFeature( MFT_TerrainParallaxMap ) &&
         !LangElement::find( "outNegViewTS" ) )
   {
      // Get the object to tangent transform which
      // will consume 3 output registers.
      Var *objToTangentSpace = getOutObjToTangentSpace( componentList, meta, fd );

      // Now use a single output register to send the negative
      // view vector in tangent space to the pixel shader.
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
      Var *outNegViewTS = connectComp->getElement( RT_TEXCOORD );
      outNegViewTS->setName( "outNegViewTS" );
      outNegViewTS->setStructName( "OUT" );
      outNegViewTS->setType( "vec3" );
      meta->addStatement( new GenOp( "   @ = tMul( @, float3( @ - @.xyz ) );\r\n", 
         outNegViewTS, objToTangentSpace, eyePos, inPos ) );
   }

   // Get the distance from the eye to this vertex.
   Var *dist = (Var*)LangElement::find( "dist" );
   if ( !dist )
   {
      dist = new Var;
      dist->setType( "float" );
      dist->setName( "dist" );  

      meta->addStatement( new GenOp( "   @ = distance( @.xyz, @ );\r\n", 
                                       new DecOp( dist ), inPos, eyePos ) );
   }

   // grab connector texcoord register
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *outTex = connectComp->getElement( RT_TEXCOORD );
   outTex->setName( String::ToString( "detCoord%d", detailIndex ) );
   outTex->setStructName( "OUT" );
   outTex->setType( "vec4" );
   outTex->mapsToSampler = true;

   // Get the detail scale and fade info.
   Var *detScaleAndFade = new Var;
   detScaleAndFade->setType( "vec4" );
   detScaleAndFade->setName( String::ToString( "detailScaleAndFade%d", detailIndex ) );
   detScaleAndFade->uniform = true;
   detScaleAndFade->constSortPos = cspPotentialPrimitive;

   // Setup the detail coord.
   //
   // NOTE: You see here we scale the texture coord by 'xyx'
   // to generate the detail coord.  This y is here because
   // its scale is flipped to correct for the non negative y
   // in texCoord.
   //
   // See TerrainBaseMapFeatGLSL::processVert().
   //
   meta->addStatement( new GenOp( "   @.xyz = @ * @.xyx;\r\n", outTex, inTex, detScaleAndFade ) );

   // And sneak the detail fade thru the w detailCoord.
   meta->addStatement( new GenOp( "   @.w = clamp( ( @.z - @ ) * @.w, 0.0, 1.0 );\r\n", 
                                    outTex, detScaleAndFade, dist, detScaleAndFade ) );   

   output = meta;
}

void TerrainDetailMapFeatGLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                             const MaterialFeatureData &fd )
{
   const U32 detailIndex = getProcessIndex();
   Var *inTex = getVertTexCoord( "texCoord" );

   MultiLine *meta = new MultiLine;

   // We need the negative tangent space view vector
   // as in parallax mapping we step towards the camera.
   Var *negViewTS = (Var*)LangElement::find( "negViewTS" );
   if (  !negViewTS &&
         fd.features.hasFeature( MFT_TerrainParallaxMap ) )
   {
      Var *inNegViewTS = (Var*)LangElement::find( "outNegViewTS" );
      if ( !inNegViewTS )
      {
         ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
         inNegViewTS = connectComp->getElement( RT_TEXCOORD );
         inNegViewTS->setName( "outNegViewTS" );
         inNegViewTS->setStructName( "IN" );
         inNegViewTS->setType( "vec3" );
      }
   
      negViewTS = new Var( "negViewTS", "vec3" );
      meta->addStatement( new GenOp( "   @ = normalize( @ );\r\n", new DecOp( negViewTS ), inNegViewTS ) );
   }

   // Get the layer samples.
   Var *layerSample = (Var*)LangElement::find( "layerSample" );
   if ( !layerSample )
   {
      layerSample = new Var;
      layerSample->setType( "vec4" );
      layerSample->setName( "layerSample" );

      // Get the layer texture var
      Var *layerTex = new Var;
      layerTex->setType( "sampler2D" );
      layerTex->setName( "layerTex" );
      layerTex->uniform = true;
      layerTex->sampler = true;
      layerTex->constNum = Var::getTexUnitNum();

      // Read the layer texture to get the samples.
      meta->addStatement( new GenOp( "   @ = round( tex2D( @, @.xy ) * 255.0f );\r\n", 
                                       new DecOp( layerSample ), layerTex, inTex ) );
   }

   Var *layerSize = (Var*)LangElement::find( "layerSize" );
   if ( !layerSize )
   {
      layerSize = new Var;
      layerSize->setType( "float" );
      layerSize->setName( "layerSize" );
      layerSize->uniform = true;
      layerSize->constSortPos = cspPass;
   }

   // Grab the incoming detail coord.
   Var *inDet = _getInDetailCoord( componentList );

   // Get the detail id.
   Var *detailInfo = _getDetailIdStrengthParallax();

   // Create the detail blend var.
   Var *detailBlend = new Var;
   detailBlend->setType( "float" );
   detailBlend->setName( String::ToString( "detailBlend%d", detailIndex ) );

   // Calculate the blend for this detail texture.
   meta->addStatement( new GenOp( "   @ = calcBlend( @.x, @.xy, @, @ );\r\n", 
                                    new DecOp( detailBlend ), detailInfo, inTex, layerSize, layerSample ) );

   // Get a var and accumulate the blend amount.
   Var *blendTotal = (Var*)LangElement::find( "blendTotal" );
   if ( !blendTotal )
   {
      blendTotal = new Var;
      blendTotal->setName( "blendTotal" );
      blendTotal->setType( "float" );
      meta->addStatement( new GenOp( "   @ = 0;\r\n", new DecOp( blendTotal ) ) );
   }

   // Add to the blend total.
   meta->addStatement( new GenOp( "   @ = max( @, @ );\r\n", blendTotal, blendTotal, detailBlend ) );

   // If we had a parallax feature... then factor in the parallax
   // amount so that it fades out with the layer blending.
   if ( fd.features.hasFeature( MFT_TerrainParallaxMap, detailIndex ) )
   {
      // Get the rest of our inputs.
      Var *normalMap = _getNormalMapTex();

      // Call the library function to do the rest.
      meta->addStatement( new GenOp( "   @.xy += parallaxOffset( @, @.xy, @, @.z * @ );\r\n", 
         inDet, normalMap, inDet, negViewTS, detailInfo, detailBlend ) );
   }

   // If this is a prepass then we skip color.
   if ( fd.features.hasFeature( MFT_PrePassConditioner ) )
   {
      // Check to see if we have a gbuffer normal.
      Var *gbNormal = (Var*)LangElement::find( "gbNormal" );

      // If we have a gbuffer normal and we don't have a
      // normal map feature then we need to lerp in a 
      // default normal else the normals below this layer
      // will show thru.
      if (  gbNormal && 
            !fd.features.hasFeature( MFT_TerrainNormalMap, detailIndex ) )
      {
         Var *viewToTangent = getInViewToTangent( componentList );

         meta->addStatement( new GenOp( "   @ = lerp( @, tGetMatrix3Row(@, 2), min( @, @.w ) );\r\n", 
            gbNormal, gbNormal, viewToTangent, detailBlend, inDet ) );
      }

      output = meta;
      return;
   }

   Var *detailColor = (Var*)LangElement::find( "detailColor" ); 
   if ( !detailColor )
   {
      detailColor = new Var;
      detailColor->setType( "vec4" );
      detailColor->setName( "detailColor" );
      meta->addStatement( new GenOp( "   @;\r\n", new DecOp( detailColor ) ) );
   }

   // Get the detail texture.
   Var *detailMap = new Var;
   detailMap->setType( "sampler2D" );
   detailMap->setName( String::ToString( "detailMap%d", detailIndex ) );
   detailMap->uniform = true;
   detailMap->sampler = true;
   detailMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

   // If we're using SM 3.0 then take advantage of 
   // dynamic branching to skip layers per-pixel.
   

   if ( GFX->getPixelShaderVersion() >= 3.0f )
      meta->addStatement( new GenOp( "   if ( @ > 0.0f )\r\n", detailBlend ) );

   meta->addStatement( new GenOp( "   {\r\n" ) );

   // Note that we're doing the standard greyscale detail 
   // map technique here which can darken and lighten the 
   // diffuse texture.
   //
   // We take two color samples and lerp between them for
   // side projection layers... else a single sample.
   //
   if ( fd.features.hasFeature( MFT_TerrainSideProject, detailIndex ) )
   {
      meta->addStatement( new GenOp( "      @ = ( lerp( tex2D( @, @.yz ), tex2D( @, @.xz ), @.z ) * 2.0 ) - 1.0;\r\n", 
                                                detailColor, detailMap, inDet, detailMap, inDet, inTex ) );
   }
   else
   {
      meta->addStatement( new GenOp( "      @ = ( tex2D( @, @.xy ) * 2.0 ) - 1.0;\r\n", 
                                       detailColor, detailMap, inDet ) );
   }

   meta->addStatement( new GenOp( "      @ *= @.y * @.w;\r\n",
                                    detailColor, detailInfo, inDet ) );

   Var *baseColor = (Var*)LangElement::find( "baseColor" );
   Var *outColor = (Var*)LangElement::find( "col" );

   meta->addStatement( new GenOp( "      @ = lerp( @, @ + @, @ );\r\n",
                                    outColor, outColor, baseColor, detailColor, detailBlend ) );

   meta->addStatement( new GenOp( "   }\r\n" ) );

   output = meta;
}

ShaderFeature::Resources TerrainDetailMapFeatGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res;

   if ( getProcessIndex() == 0 )
   {
      // If this is the first detail pass then we 
      // samples from the layer tex.
      res.numTex += 1;

      // If this material also does parallax then it
      // will generate the negative view vector and the
      // worldToTanget transform.
      if ( fd.features.hasFeature( MFT_TerrainParallaxMap ) )
         res.numTexReg += 4;
   }

   // If this isn't the prepass then we sample 
   // from the detail texture for diffuse coloring.
   if ( !fd.features.hasFeature( MFT_PrePassConditioner ) )
      res.numTex += 1;

   // If we have parallax for this layer then we'll also
   // be sampling the normal map for the parallax heightmap.
   if ( fd.features.hasFeature( MFT_TerrainParallaxMap, getProcessIndex() ) )
      res.numTex += 1;

   // Finally we always send the detail texture 
   // coord to the pixel shader.
   res.numTexReg += 1;

   return res;
}


TerrainMacroMapFeatGLSL::TerrainMacroMapFeatGLSL()
   :  mTorqueDep( "shaders/common/gl/torque.glsl" ),
      mTerrainDep( "shaders/common/terrain/terrain.glsl" )
      
{
   addDependency( &mTorqueDep );
   addDependency( &mTerrainDep );
}


void TerrainMacroMapFeatGLSL::processVert(  Vector<ShaderComponent*> &componentList, 
                                             const MaterialFeatureData &fd )
{
   const U32 detailIndex = getProcessIndex();

   // Grab incoming texture coords... the base map feature
   // made sure this was created.
   Var *inTex = (Var*)LangElement::find( "texCoord" );
   AssertFatal( inTex, "The texture coord is missing!" );

   // Grab the input position.
   Var *inPos = (Var*)LangElement::find( "inPosition" );
   if ( !inPos )
      inPos = (Var*)LangElement::find( "position" );

   // Get the object space eye position.
   Var *eyePos = _getUniformVar( "eyePos", "vec3", cspPotentialPrimitive );

   MultiLine *meta = new MultiLine;

   // Get the distance from the eye to this vertex.
   Var *dist = (Var*)LangElement::find( "macroDist" );
   if ( !dist )
   {
      dist = new Var;
      dist->setType( "float" );
      dist->setName( "macroDist" );  

      meta->addStatement( new GenOp( "   @ = distance( @.xyz, @ );\r\n", 
                                       new DecOp( dist ), inPos, eyePos ) );
   }

   // grab connector texcoord register
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *outTex = connectComp->getElement( RT_TEXCOORD );
   outTex->setName( String::ToString( "macroCoord%d", detailIndex ) );
   outTex->setStructName( "OUT" );
   outTex->setType( "vec4" );
   outTex->mapsToSampler = true;

   // Get the detail scale and fade info.
   Var *detScaleAndFade = new Var;
   detScaleAndFade->setType( "vec4" );
   detScaleAndFade->setName( String::ToString( "macroScaleAndFade%d", detailIndex ) );
   detScaleAndFade->uniform = true;
   detScaleAndFade->constSortPos = cspPotentialPrimitive;

   // Setup the detail coord.
   meta->addStatement( new GenOp( "   @.xyz = @ * @.xyx;\r\n", outTex, inTex, detScaleAndFade ) );

   // And sneak the detail fade thru the w detailCoord.
   meta->addStatement( new GenOp( "   @.w = clamp( ( @.z - @ ) * @.w, 0.0, 1.0 );\r\n", 
                                    outTex, detScaleAndFade, dist, detScaleAndFade ) );   

   output = meta;
}


void TerrainMacroMapFeatGLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                             const MaterialFeatureData &fd )
{
   const U32 detailIndex = getProcessIndex();
   Var *inTex = getVertTexCoord( "texCoord" );
   
   MultiLine *meta = new MultiLine;

   // We need the negative tangent space view vector
   // as in parallax mapping we step towards the camera.
   Var *negViewTS = (Var*)LangElement::find( "negViewTS" );
   if (  !negViewTS &&
         fd.features.hasFeature( MFT_TerrainParallaxMap ) )
   {
      Var *inNegViewTS = (Var*)LangElement::find( "outNegViewTS" );
      if ( !inNegViewTS )
      {
         ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
         inNegViewTS = connectComp->getElement( RT_TEXCOORD );
         inNegViewTS->setName( "outNegViewTS" );
         inNegViewTS->setStructName( "IN" );
         inNegViewTS->setType( "vec3" );
      }

      negViewTS = new Var( "negViewTS", "vec3" );
      meta->addStatement( new GenOp( "   @ = normalize( @ );\r\n", new DecOp( negViewTS ), inNegViewTS ) );
   }

   // Get the layer samples.
   Var *layerSample = (Var*)LangElement::find( "layerSample" );
   if ( !layerSample )
   {
      layerSample = new Var;
      layerSample->setType( "vec4" );
      layerSample->setName( "layerSample" );

      // Get the layer texture var
      Var *layerTex = new Var;
      layerTex->setType( "sampler2D" );
      layerTex->setName( "macrolayerTex" );
      layerTex->uniform = true;
      layerTex->sampler = true;
      layerTex->constNum = Var::getTexUnitNum();

      // Read the layer texture to get the samples.
      meta->addStatement( new GenOp( "   @ = round( tex2D( @, @.xy ) * 255.0f );\r\n", 
                                       new DecOp( layerSample ), layerTex, inTex ) );
   }

   Var *layerSize = (Var*)LangElement::find( "layerSize" );
   if ( !layerSize )
   {
      layerSize = new Var;
      layerSize->setType( "float" );
      layerSize->setName( "layerSize" );
      layerSize->uniform = true;
      layerSize->constSortPos = cspPass;
   }

   // Grab the incoming detail coord.
   Var *inDet = _getInMacroCoord( componentList );

   // Get the detail id.
   Var *detailInfo = _getMacroIdStrengthParallax();

   // Create the detail blend var.
   Var *detailBlend = new Var;
   detailBlend->setType( "float" );
   detailBlend->setName( String::ToString( "macroBlend%d", detailIndex ) );

   // Calculate the blend for this detail texture.
   meta->addStatement( new GenOp( "   @ = calcBlend( @.x, @.xy, @, @ );\r\n", 
                                    new DecOp( detailBlend ), detailInfo, inTex, layerSize, layerSample ) );

   // Get a var and accumulate the blend amount.
   Var *blendTotal = (Var*)LangElement::find( "blendTotal" );
   if ( !blendTotal )
   {
      blendTotal = new Var;
      //blendTotal->setName( "blendTotal" );
      blendTotal->setName( "blendTotal" );
      blendTotal->setType( "float" );
      meta->addStatement( new GenOp( "   @ = 0;\r\n", new DecOp( blendTotal ) ) );
   }

   // Add to the blend total.
   meta->addStatement( new GenOp( "   @ = max( @, @ );\r\n", blendTotal, blendTotal, detailBlend ) );

   // If this is a prepass then we skip color.
   if ( fd.features.hasFeature( MFT_PrePassConditioner ) )
   {
      // Check to see if we have a gbuffer normal.
      Var *gbNormal = (Var*)LangElement::find( "gbNormal" );

      // If we have a gbuffer normal and we don't have a
      // normal map feature then we need to lerp in a 
      // default normal else the normals below this layer
      // will show thru.
      if (  gbNormal && 
            !fd.features.hasFeature( MFT_TerrainNormalMap, detailIndex ) )
      {
         Var *viewToTangent = getInViewToTangent( componentList );

         meta->addStatement( new GenOp( "   @ = lerp( @, tGetMatrix3Row(@, 2), min( @, @.w ) );\r\n", 
            gbNormal, gbNormal, viewToTangent, detailBlend, inDet ) );
      }

      output = meta;
      return;
   }

   Var *detailColor = (Var*)LangElement::find( "macroColor" ); 
   if ( !detailColor )
   {
      detailColor = new Var;
      detailColor->setType( "vec4" );
      detailColor->setName( "macroColor" );
      meta->addStatement( new GenOp( "   @;\r\n", new DecOp( detailColor ) ) );
   }

   // Get the detail texture.
   Var *detailMap = new Var;
   detailMap->setType( "sampler2D" );
   detailMap->setName( String::ToString( "macroMap%d", detailIndex ) );
   detailMap->uniform = true;
   detailMap->sampler = true;
   detailMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

   // If we're using SM 3.0 then take advantage of 
   // dynamic branching to skip layers per-pixel.
   if ( GFX->getPixelShaderVersion() >= 3.0f )
      meta->addStatement( new GenOp( "   if ( @ > 0.0f )\r\n", detailBlend ) );

   meta->addStatement( new GenOp( "   {\r\n" ) );

   // Note that we're doing the standard greyscale detail 
   // map technique here which can darken and lighten the 
   // diffuse texture.
   //
   // We take two color samples and lerp between them for
   // side projection layers... else a single sample.
   //
   if ( fd.features.hasFeature( MFT_TerrainSideProject, detailIndex ) )
   {
      meta->addStatement( new GenOp( "      @ = ( lerp( tex2D( @, @.yz ), tex2D( @, @.xz ), @.z ) * 2.0 ) - 1.0;\r\n", 
                                                detailColor, detailMap, inDet, detailMap, inDet, inTex ) );
   }
   else
   {
      meta->addStatement( new GenOp( "      @ = ( tex2D( @, @.xy ) * 2.0 ) - 1.0;\r\n", 
                                       detailColor, detailMap, inDet ) );
   }

   meta->addStatement( new GenOp( "      @ *= @.y * @.w;\r\n",
                                    detailColor, detailInfo, inDet ) );

   Var *baseColor = (Var*)LangElement::find( "baseColor" );
   Var *outColor = (Var*)LangElement::find( "col" );

   meta->addStatement( new GenOp( "      @ = lerp( @, @ + @, @ );\r\n",
                                    outColor, outColor, outColor, detailColor, detailBlend ) );
   //outColor, outColor, baseColor, detailColor, detailBlend ) );

   meta->addStatement( new GenOp( "   }\r\n" ) );

   output = meta;
}



ShaderFeature::Resources TerrainMacroMapFeatGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res;

   if ( getProcessIndex() == 0 )
   {
      // If this is the first detail pass then we 
      // samples from the layer tex.
         res.numTex += 1;
   }

   // If this isn't the prepass then we sample 
   // from the detail texture for diffuse coloring.
   if ( !fd.features.hasFeature( MFT_PrePassConditioner ) )
      res.numTex += 1;

   // Finally we always send the detail texture 
   // coord to the pixel shader.
   res.numTexReg += 1;

   return res;
}

void TerrainNormalMapFeatGLSL::processVert(  Vector<ShaderComponent*> &componentList, 
                                             const MaterialFeatureData &fd )
{
   // We only need to process normals during the prepass.
   if ( !fd.features.hasFeature( MFT_PrePassConditioner ) )
      return;

   MultiLine *meta = new MultiLine;

   // Make sure the world to tangent transform
   // is created and available for the pixel shader.
   getOutViewToTangent( componentList, meta, fd );

   output = meta;
}

void TerrainNormalMapFeatGLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                             const MaterialFeatureData &fd )
{
   // We only need to process normals during the prepass.
   if ( !fd.features.hasFeature( MFT_PrePassConditioner ) )
      return;

   MultiLine *meta = new MultiLine;

   Var *viewToTangent = getInViewToTangent( componentList );

   // This var is read from GBufferConditionerGLSL and 
   // used in the prepass output.
   Var *gbNormal = (Var*)LangElement::find( "gbNormal" );
   if ( !gbNormal )
   {
      gbNormal = new Var;
      gbNormal->setName( "gbNormal" );
      gbNormal->setType( "vec3" );
      meta->addStatement( new GenOp( "   @ = tGetMatrix3Row(@, 2);\r\n", new DecOp( gbNormal ), viewToTangent ) );
   }

   const U32 normalIndex = getProcessIndex();

   Var *detailBlend = (Var*)LangElement::find( String::ToString( "detailBlend%d", normalIndex ) );
   AssertFatal( detailBlend, "The detail blend is missing!" );

   // If we're using SM 3.0 then take advantage of 
   // dynamic branching to skip layers per-pixel.
   if ( GFX->getPixelShaderVersion() >= 3.0f )
      meta->addStatement( new GenOp( "   if ( @ > 0.0f )\r\n", detailBlend ) );

   meta->addStatement( new GenOp( "   {\r\n" ) );

   // Get the normal map texture.
   Var *normalMap = _getNormalMapTex();

   /// Get the texture coord.
   Var *inDet = _getInDetailCoord( componentList );
   Var *inTex = getVertTexCoord( "texCoord" );

   // Sample the normal map.
   //
   // We take two normal samples and lerp between them for
   // side projection layers... else a single sample.
   LangElement *texOp;
   if ( fd.features.hasFeature( MFT_TerrainSideProject, normalIndex ) )
   {
      texOp = new GenOp( "lerp( tex2D( @, @.yz ), tex2D( @, @.xz ), @.z )", 
         normalMap, inDet, normalMap, inDet, inTex );
   }
   else
      texOp = new GenOp( "tex2D(@, @.xy)", normalMap, inDet );

   // create bump normal
   Var *bumpNorm = new Var;
   bumpNorm->setName( "bumpNormal" );
   bumpNorm->setType( "vec4" );

   LangElement *bumpNormDecl = new DecOp( bumpNorm );
   meta->addStatement( expandNormalMap( texOp, bumpNormDecl, bumpNorm, fd ) );

   // Normalize is done later... 
   // Note: The reverse mul order is intentional. Affine matrix.
   meta->addStatement( new GenOp( "      @ = lerp( @, tMul( @.xyz, @ ), min( @, @.w ) );\r\n", 
      gbNormal, gbNormal, bumpNorm, viewToTangent, detailBlend, inDet ) );

   // End the conditional block.
   meta->addStatement( new GenOp( "   }\r\n" ) );

   // If this is the last normal map then we 
   // can test to see the total blend value
   // to see if we should clip the result.
   //if ( fd.features.getNextFeatureIndex( MFT_TerrainNormalMap, normalIndex ) == -1 )
      //meta->addStatement( new GenOp( "   clip( @ - 0.0001f );\r\n", blendTotal ) );

   output = meta;
}

ShaderFeature::Resources TerrainNormalMapFeatGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res;

   // We only need to process normals during the prepass.
   if ( fd.features.hasFeature( MFT_PrePassConditioner ) )
   {
      // If this is the first normal map and there
      // are no parallax features then we will 
      // generate the worldToTanget transform.
      if (  !fd.features.hasFeature( MFT_TerrainParallaxMap ) &&
            ( getProcessIndex() == 0 || !fd.features.hasFeature( MFT_TerrainNormalMap, getProcessIndex() - 1 ) ) )
         res.numTexReg = 3;

      res.numTex = 1;
   }

   return res;
}

void TerrainLightMapFeatGLSL::processPix( Vector<ShaderComponent*> &componentList, 
                                          const MaterialFeatureData &fd )
{
   // grab connector texcoord register
   Var *inTex = (Var*)LangElement::find( "texCoord" );
   if ( !inTex )
      return;

   // Get the lightmap texture.
   Var *lightMap = new Var;
   lightMap->setType( "sampler2D" );
   lightMap->setName( "lightMapTex" );
   lightMap->uniform = true;
   lightMap->sampler = true;
   lightMap->constNum = Var::getTexUnitNum();

   MultiLine *meta = new MultiLine;

   // Find or create the lightMask value which is read by
   // RTLighting to mask out the lights.
   //
   // The first light is always the sunlight so we apply
   // the shadow mask to only the first channel.
   //
   Var *lightMask = (Var*)LangElement::find( "lightMask" );
   if ( !lightMask )
   {
      lightMask = new Var( "lightMask", "vec4" );
      meta->addStatement( new GenOp( "   @ = vec4(1);\r\n", new DecOp( lightMask ) ) );
   }

   meta->addStatement( new GenOp( "   @[0] = tex2D( @, @.xy ).r;\r\n", lightMask, lightMap, inTex ) );
   output = meta;
}

ShaderFeature::Resources TerrainLightMapFeatGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 
   res.numTex = 1;
   return res;
}


void TerrainAdditiveFeatGLSL::processPix( Vector<ShaderComponent*> &componentList, 
                                          const MaterialFeatureData &fd )
{
   Var *color = (Var*) LangElement::find( "col" );
   Var *blendTotal = (Var*)LangElement::find( "blendTotal" );
   if ( !color || !blendTotal )
      return;
   
   MultiLine *meta = new MultiLine;

   meta->addStatement( new GenOp( "   clip( @ - 0.0001 );\r\n", blendTotal ) );
   meta->addStatement( new GenOp( "   @.a = @;\r\n", color, blendTotal ) );

   output = meta;
}
