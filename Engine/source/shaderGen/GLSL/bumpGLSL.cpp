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
#include "shaderGen/GLSL/bumpGLSL.h"

#include "shaderGen/shaderOp.h"
#include "gfx/gfxDevice.h"
#include "materials/matInstance.h"
#include "materials/processedMaterial.h"
#include "materials/materialFeatureTypes.h"
#include "shaderGen/shaderGenVars.h"


void BumpFeatGLSL::processVert(  Vector<ShaderComponent*> &componentList, 
                                 const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;
	output = meta;
	
	const bool useTexAnim = fd.features[MFT_TexAnim];

   // Output the texture coord.
   getOutTexCoord(   "texCoord", 
                     "vec2", 
                     true, 
                     useTexAnim, 
                     meta, 
                     componentList );

	if ( fd.features.hasFeature( MFT_DetailNormalMap ) )
      addOutDetailTexCoord( componentList, 
										meta,
										useTexAnim );
	
   // Also output the worldToTanget transform which
   // we use to create the world space normal.
   getOutWorldToTangent( componentList, meta, fd );
}

void BumpFeatGLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                 const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;
	output = meta;

   // Get the texture coord.
   Var *texCoord = getInTexCoord( "out_texCoord", "vec2", true, componentList );

   // Sample the bumpmap.
   Var *bumpMap = getNormalMapTex();
   
	LangElement *texOp = NULL;
	
	//Handle atlased textures
	if(fd.features[MFT_NormalMapAtlas])
   {
		// This is a big block of code, so put a comment in the shader code
      meta->addStatement( new GenOp( "   // Atlased texture coordinate calculation (see BumpFeat*LSL for details)\r\n") );
		
      Var *atlasedTex = new Var;
      atlasedTex->setName("atlasedBumpCoord");
      atlasedTex->setType("vec2");
      LangElement *atDecl = new DecOp(atlasedTex);
		
      // Parameters of the texture atlas
      Var *atParams  = new Var;
      atParams->setType("vec4");
      atParams->setName("bumpAtlasParams");
      atParams->uniform = true;
      atParams->constSortPos = cspPotentialPrimitive;
		
      // Parameters of the texture (tile) this object is using in the atlas
      Var *tileParams  = new Var;
      tileParams->setType("vec4");
      tileParams->setName("bumpAtlasTileParams");
      tileParams->uniform = true;
      tileParams->constSortPos = cspPotentialPrimitive;
		
		const bool is_sm3 = (GFX->getPixelShaderVersion() > 2.0f);
		// getPixelShaderVersion() on Mac currently returns 2.0,
		// or 3.0 if Advanced Lighting is enabled
		if(is_sm3)
      {
         // Figure out the mip level
         meta->addStatement(new GenOp("   vec2 _dx_bump = dFdx(@ * @.z);\r\n", texCoord, atParams));
         meta->addStatement(new GenOp("   vec2 _dy_bump = dFdy(@ * @.z);\r\n", texCoord, atParams));
         meta->addStatement(new GenOp("   float mipLod_bump = 0.5 * log2(max(dot(_dx_bump, _dx_bump), dot(_dy_bump, _dy_bump)));\r\n"));
         meta->addStatement(new GenOp("   mipLod_bump = clamp(mipLod_bump, 0.0, @.w);\r\n", atParams));
			
         // And the size of the mip level
         meta->addStatement(new GenOp("   float mipPixSz_bump = pow(2.0, @.w - mipLod_bump);\r\n", atParams));
         meta->addStatement(new GenOp("   vec2 mipSz_bump = mipPixSz_bump / @.xy;\r\n", atParams));
      }
      else
      {
         meta->addStatement(new GenOp("   vec2 mipSz = float2(1.0, 1.0);\r\n"));
      }
		
		// Tiling mode
      // TODO: Select wrap or clamp somehow
      if( true ) // Wrap
         meta->addStatement(new GenOp("   @ = fract(@);\r\n", atDecl, texCoord));
      else       // Clamp
         meta->addStatement(new GenOp("   @ = saturate(@);\r\n", atDecl, texCoord));
		
      // Finally scale/offset, and correct for filtering
      meta->addStatement(new GenOp("   @ = @ * ((mipSz_bump * @.xy - 1.0) / mipSz_bump) + 0.5 / mipSz_bump + @.xy * @.xy;\r\n", 
											  atlasedTex, atlasedTex, atParams, atParams, tileParams));
		
      // Add a newline
      meta->addStatement(new GenOp( "\r\n"));
		
      if(is_sm3)
		{
			texOp = new GenOp( "texture2DLod(@, vec4(@, 0.0, mipLod_bump)", bumpMap, texCoord );
		}
		else
		{
			texOp = new GenOp( "texture2D(@, @)", bumpMap, texCoord );
		}
	}
	else
	{
		texOp = new GenOp( "texture2D(@, @)", bumpMap, texCoord );
	}
		
   Var *bumpNorm = new Var( "bumpNormal", "vec4" );
   meta->addStatement( expandNormalMap( texOp, new DecOp( bumpNorm ), bumpNorm, fd ) );

	// If we have a detail normal map we add the xy coords of
   // it to the base normal map.  This gives us the effect we
   // want with few instructions and minial artifacts.
   if ( fd.features.hasFeature( MFT_DetailNormalMap ) )
   {
      bumpMap = new Var;
      bumpMap->setType( "sampler2D" );
      bumpMap->setName( "detailBumpMap" );
      bumpMap->uniform = true;
      bumpMap->sampler = true;
      bumpMap->constNum = Var::getTexUnitNum();
		
      texCoord = getInTexCoord( "detCoord", "vec2", true, componentList );
      texOp = new GenOp( "texture2D(@, @)", bumpMap, texCoord );
		
      Var *detailBump = new Var;
      detailBump->setName( "detailBump" );
      detailBump->setType( "vec4" );
      meta->addStatement( expandNormalMap( texOp, new DecOp( detailBump ), detailBump, fd ) );
		
      Var *detailBumpScale = new Var;
      detailBumpScale->setType( "float" );
      detailBumpScale->setName( "detailBumpStrength" );
      detailBumpScale->uniform = true;
      detailBumpScale->constSortPos = cspPass;
      meta->addStatement( new GenOp( "   @.xy += @.xy * @;\r\n", bumpNorm, detailBump, detailBumpScale ) );
   }
	
	
   // We transform it into world space by reversing the 
   // multiplication by the worldToTanget transform.
   Var *wsNormal = new Var( "wsNormal", "vec3" );
   Var *worldToTanget = getInWorldToTangent( componentList );
   meta->addStatement( new GenOp( "   @ = normalize( vec3( @.xyz * @ ) );\r\n", new DecOp( wsNormal ), bumpNorm, worldToTanget ) );

}

ShaderFeature::Resources BumpFeatGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 

   // If we have no parallax then we bring on the normal tex.
   if ( !fd.features[MFT_Parallax] )
      res.numTex = 1;

   // Only the parallax or diffuse map will add texture
   // coords other than us.
   if (  !fd.features[MFT_Parallax] &&
         !fd.features[MFT_DiffuseMap] &&
         !fd.features[MFT_OverlayMap] &&
         !fd.features[MFT_DetailMap] )
      res.numTexReg++;

   // We pass the world to tanget space transform.
   res.numTexReg += 3;

	// Do we have detail normal mapping?
   if ( fd.features[MFT_DetailNormalMap] )
   {
      res.numTex++;
      if ( !fd.features[MFT_DetailMap] )
         res.numTexReg++;
   }
		
   return res;
}

void BumpFeatGLSL::setTexData(   Material::StageData &stageDat,
                                 const MaterialFeatureData &fd,
                                 RenderPassData &passData,
                                 U32 &texIndex )
{
   // If we had a parallax feature then it takes
   // care of hooking up the normal map texture.
   if ( fd.features[MFT_Parallax] )
      return;

   if ( fd.features[MFT_NormalMap] )
   {
      passData.mTexType[ texIndex ] = Material::Bump;
      passData.mTexSlot[ texIndex++ ].texObject = stageDat.getTex( MFT_NormalMap );
   }
	
	
   if ( fd.features[ MFT_DetailNormalMap ] )
   {
      passData.mTexType[ texIndex ] = Material::DetailBump;
      passData.mTexSlot[ texIndex++ ].texObject = stageDat.getTex( MFT_DetailNormalMap );
   }
}


//
Var* ParallaxFeatGLSL::_getUniformVar( const char *name, const char *type )
{
   Var *theVar = (Var*)LangElement::find( name );
   if ( !theVar )
   {
      theVar = new Var;
      theVar->setType( type );
      theVar->setName( name );
      theVar->uniform = true;
      theVar->constSortPos = cspPass;
   }
	
   return theVar;
}

void ParallaxFeatGLSL::processVert( Vector<ShaderComponent*> &componentList, 
											  const MaterialFeatureData &fd )
{
   AssertFatal( GFX->getPixelShaderVersion() >= 2.0, 
					"ParallaxFeatGLSL::processVert - We don't support SM 1.x!" );
	
   MultiLine *meta = new MultiLine;
	
   // Add the texture coords.
   getOutTexCoord(   "texCoord", 
						"vec2", 
						true, 
						fd.features[MFT_TexAnim], 
						meta, 
						componentList );
	
   // Grab the input position.
   Var *inPos = (Var*)LangElement::find( "inPosition" );
   if ( !inPos )
      inPos = (Var*)LangElement::find( "position" );
	
   // Get the object space eye position and the world
   // to tangent transform.
   Var *eyePos = _getUniformVar( "eyePos", "vec3" );
   Var *objToTangentSpace = getOutObjToTangentSpace( componentList, meta, fd );
	
   // send transform to pixel shader
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *outViewTS = connectComp->getElement( RT_TEXCOORD, 1 );
   outViewTS->setName( "outViewTS" );
   outViewTS->setType( "vec3" );
   meta->addStatement( new GenOp( "   @ = ( @ - @.xyz ) * transpose( @ );\r\n", 
											outViewTS, inPos, eyePos, objToTangentSpace ) );
	
   output = meta;
}

void ParallaxFeatGLSL::processPix(  Vector<ShaderComponent*> &componentList, 
											 const MaterialFeatureData &fd )
{
   AssertFatal( GFX->getPixelShaderVersion() >= 2.0, 
					"ParallaxFeatGLSL::processPix - We don't support SM 1.x!" );
	
   MultiLine *meta = new MultiLine;
	
   // Order matters... get this first!
   Var *texCoord = getInTexCoord( "texCoord", "vec2", true, componentList );
	
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
	
   // We need the negative tangent space view vector
   // as in parallax mapping we step towards the camera.
   Var *negViewTS = (Var*)LangElement::find( "negViewTS" );
   if ( !negViewTS )
   {
      Var *inViewTS = (Var*)LangElement::find( "outViewTS" );
      if ( !inViewTS )
      {
         inViewTS = connectComp->getElement( RT_TEXCOORD, 1 );
         inViewTS->setName( "outViewTS" );
         inViewTS->setType( "vec3" );
      }
		
      negViewTS = new Var( "negViewTS", "vec3" );
      meta->addStatement( new GenOp( "   @ = -normalize( @ );\r\n", new DecOp( negViewTS ), inViewTS ) );
   }
	
   // Get the rest of our inputs.
   Var *parallaxInfo = _getUniformVar( "parallaxInfo", "float" );
   Var *normalMap = getNormalMapTex();
	
   // Do 3 parallax samples to get acceptable
   // quality without too much overhead.
   Var *pdepth = findOrCreateLocal( "pdepth", "float", meta );
   Var *poffset = findOrCreateLocal( "poffset", "vec2", meta );
   meta->addStatement( new GenOp( "   @ = texture2D( @, @.xy ).a;\r\n", pdepth, normalMap, texCoord ) );
   meta->addStatement( new GenOp( "   @ = @.xy * ( @ * @ );\r\n", poffset, negViewTS, pdepth, parallaxInfo ) );
	
   meta->addStatement( new GenOp( "   @ = ( @ + texture2D( @, @.xy + @ ).a ) * 0.5;\r\n", pdepth, pdepth, normalMap, texCoord, poffset ) );
   meta->addStatement( new GenOp( "   @ = @.xy * ( @ * @ );\r\n", poffset, negViewTS, pdepth, parallaxInfo ) );
	
   meta->addStatement( new GenOp( "   @ = ( @ + texture2D( @, @.xy + @ ).a ) * 0.5;\r\n", pdepth, pdepth, normalMap, texCoord, poffset ) );
   meta->addStatement( new GenOp( "   @ = @.xy * ( @ * @ );\r\n", poffset, negViewTS, pdepth, parallaxInfo ) );
	
   meta->addStatement( new GenOp( "   @.xy += @;\r\n", texCoord, poffset ) );
   
   // TODO: Fix second UV.
	
   output = meta;
}

ShaderFeature::Resources ParallaxFeatGLSL::getResources( const MaterialFeatureData &fd )
{
   AssertFatal( GFX->getPixelShaderVersion() >= 2.0, 
					"ParallaxFeatGLSL::getResources - We don't support SM 1.x!" );
	
   Resources res;
	
   // We add the outViewTS to the outputstructure.
   res.numTexReg = 1;
	
   // If this isn't a prepass then we will be
   // creating the normal map here.
   if ( !fd.features.hasFeature( MFT_PrePassConditioner ) )
      res.numTex = 1;
	
   return res;
}

void ParallaxFeatGLSL::setTexData(  Material::StageData &stageDat,
											 const MaterialFeatureData &fd,
											 RenderPassData &passData,
											 U32 &texIndex )
{
   AssertFatal( GFX->getPixelShaderVersion() >= 2.0, 
					"ParallaxFeatGLSL::setTexData - We don't support SM 1.x!" );
	
   GFXTextureObject *tex = stageDat.getTex( MFT_NormalMap );
   if ( tex )
   {
      passData.mTexType[ texIndex ] = Material::Bump;
      passData.mTexSlot[ texIndex++ ].texObject = tex;
   }
}


//
void NormalsOutFeatGLSL::processVert(  Vector<ShaderComponent*> &componentList, 
												 const MaterialFeatureData &fd )
{
   // If we have normal maps then we can count
   // on it to generate the world space normal.
   if ( fd.features[MFT_NormalMap] )
      return;
	
   MultiLine *meta = new MultiLine;
   output = meta;
	
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
	
   Var *outNormal = connectComp->getElement( RT_TEXCOORD );
   outNormal->setName( "wsNormal" );
   outNormal->setType( "vec3" );
   outNormal->mapsToSampler = false;
	
   // Find the incoming vertex normal.
   Var *inNormal = (Var*)LangElement::find( "normal" );   
   if ( inNormal )
   {
      // Transform the normal to world space.
      Var *objTrans = getObjTrans( componentList, fd.features[MFT_UseInstancing], meta );
      meta->addStatement( new GenOp( "   @ = @ * normalize( @ );\r\n", outNormal, objTrans, inNormal ) );
   }
   else
   {
      // If we don't have a vertex normal... just pass the
      // camera facing normal to the pixel shader.
      meta->addStatement( new GenOp( "   @ = vec3( 0.0, 0.0, 1.0 );\r\n", outNormal ) );
   }
}

void NormalsOutFeatGLSL::processPix(   Vector<ShaderComponent*> &componentList, 
												const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;
   output = meta;
	
   Var *wsNormal = (Var*)LangElement::find( "wsNormal" );
   if ( !wsNormal )
   {
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
      wsNormal = connectComp->getElement( RT_TEXCOORD );
      wsNormal->setName( "wsNormal" );
      wsNormal->setType( "vec3" );
		
      // If we loaded the normal its our resposibility
      // to normalize it... the interpolators won't.
      //
      meta->addStatement( new GenOp( "   @ = normalize( @ );\r\n", wsNormal, wsNormal ) );      
   }
	
   LangElement *normalOut;
   Var *outColor = (Var*)LangElement::find( "col" );
   if ( outColor )
      normalOut = new GenOp( "vec4( ( -@ + 1 ) * 0.5, @.a )", wsNormal, outColor );
   else
      normalOut = new GenOp( "vec4( ( -@ + 1 ) * 0.5, 1 )", wsNormal );
	
   meta->addStatement( new GenOp( "   @;\r\n", 
											assignColor( normalOut, Material::None ) ) );
}
