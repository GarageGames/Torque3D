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
#include "shaderGen/GLSL/pixSpecularGLSL.h"
#include "materials/processedMaterial.h"
#include "materials/materialFeatureTypes.h"
#include "shaderGen/shaderOp.h"
#include "shaderGen/shaderGenVars.h"
#include "gfx/gfxStructs.h"


PixelSpecularGLSL::PixelSpecularGLSL()
   : mDep( "shaders/common/gl/lighting.glsl" )
{
   addDependency( &mDep );
}

void PixelSpecularGLSL::processVert(   Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   /*
   AssertFatal( fd.features[MFT_RTLighting], 
      "PixelSpecularHLSL requires RTLighting to be enabled!" );

   MultiLine *meta = new MultiLine;

   // Get the eye world position.
   Var *eyePos = (Var*)LangElement::find( "eyePosWorld" );
   if( !eyePos )
   {
      eyePos = new Var;
      eyePos->setType( "float3" );
      eyePos->setName( "eyePosWorld" );
      eyePos->uniform = true;
      eyePos->constSortPos = cspPass;
   }

   // Grab a register for passing the 
   // world space view vector.
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *wsView = connectComp->getElement( RT_TEXCOORD );
   wsView->setName( "wsView" );
   wsView->setStructName( "OUT" );
   wsView->setType( "float3" );

   // Get the input position.
   Var *position = (Var*)LangElement::find( "inPosition" );
   if ( !position )
      position = (Var*)LangElement::find( "position" );
   
   // Get the object to world transform.
   Var *objTrans = (Var*) LangElement::find( "objTrans" );
   if ( !objTrans )
   {
      objTrans = new Var;
      objTrans->setType( "float4x4" );
      objTrans->setName( "objTrans" );
      objTrans->uniform = true;
      objTrans->constSortPos = cspPrimitive;      
   }

   meta->addStatement( new GenOp( "   @ = @ - mul( @, float4( @.xyz,1 ) ).xyz;\r\n", 
      wsView, eyePos, objTrans, position ) );

   output = meta;
   */
}

void PixelSpecularGLSL::processPix( Vector<ShaderComponent*> &componentList, 
                                    const MaterialFeatureData &fd )
{
   /*
   AssertFatal( fd.features[MFT_RTLighting], 
      "PixelSpecularHLSL requires RTLighting to be enabled!" );

   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );

   MultiLine *meta = new MultiLine;

   // Get the normal and light vectors from which the
   // RTLighting feature should have already setup.
   Var *wsNormal = (Var*)LangElement::find( "wsNormal" );
   Var *inLightVec = (Var*)LangElement::find( "inLightVec" );

   // Grab the world space position to eye vector.
   Var *wsView = connectComp->getElement( RT_TEXCOORD );
   wsView->setName( "wsView" );
   wsView->setStructName( "IN" );
   wsView->setType( "float3" );

   // Get the specular power and color.
   Var *specPow = new Var( "specularPower", "float" );
   specPow->uniform = true;
   specPow->constSortPos = cspPass;
   Var *specCol = (Var*)LangElement::find("specularColor");
   if(specCol == NULL)
   {
      specCol = new Var( "specularColor", "vec4" );
      specCol->uniform = true;
      specCol->constSortPos = cspPass;
   }

   // Calcuate the specular factor.
   Var *specular = new Var( "specular", "float" );
   meta->addStatement( new GenOp( "   @ = calcSpecular( -@, normalize( @ ), normalize( @ ), @ );\r\n", 
      new DecOp( specular ), inLightVec, wsNormal, wsView, specPow ) );

   LangElement *specMul = new GenOp( "float4(@.rgb,0) * @", specCol, specular );
   LangElement *final = specMul;
   
   // mask out with lightmap if present
   if( fd.features[MFT_LightMap] )
   {
      LangElement *lmColor = NULL;
      
      // find lightmap color
      lmColor = LangElement::find( "lmColor" );
      
      if ( !lmColor )
      {
         LangElement * lightMap = LangElement::find( "lightMap" );
         LangElement * lmCoord = LangElement::find( "texCoord2" );

         lmColor = new GenOp( "tex2D(@, @)", lightMap, lmCoord );
      }
   
      final = new GenOp( "@ * float4(@.rgb,0)", specMul, lmColor );
   }

   // We we have a normal map then mask the specular 
   if ( !fd.features[MFT_SpecularMap] && fd.features[MFT_NormalMap] )
   {
      Var *bumpColor = (Var*)LangElement::find( "bumpNormal" );
      final = new GenOp( "@ * @.a", final, bumpColor );
   }

   // Add the specular to the final color.
   meta->addStatement( new GenOp( "   @;\r\n", assignColor( final, Material::Add ) ) );

   output = meta;
   */
}

ShaderFeature::Resources PixelSpecularGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res;
   res.numTexReg = 1;
   return res;
}

void SpecularMapGLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // Get the texture coord.
   Var *texCoord = getInTexCoord( "out_texCoord", "vec2", true, componentList );

   // create texture var
   Var *specularMap = new Var;
   specularMap->setType( "sampler2D" );
   specularMap->setName( "specularMap" );
   specularMap->uniform = true;
   specularMap->sampler = true;
   specularMap->constNum = Var::getTexUnitNum();
   LangElement *texOp = new GenOp( "texture2D(@, @)", specularMap, texCoord );

   Var *specularColor = new Var( "specularColor", "vec4" );

   output = new GenOp( "   @ = @;\r\n", new DecOp( specularColor ), texOp );
}

ShaderFeature::Resources SpecularMapGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res;
   res.numTex = 1;
   return res;
}

void SpecularMapGLSL::setTexData( Material::StageData &stageDat,
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