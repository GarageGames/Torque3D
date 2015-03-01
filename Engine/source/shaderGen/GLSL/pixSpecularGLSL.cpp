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
   AssertFatal( fd.features[MFT_RTLighting], 
      "PixelSpecularHLSL requires RTLighting to be enabled!" );

   // Nothing to do here... MFT_RTLighting should have
   // taken care of passing everything to the pixel shader.
}

void PixelSpecularGLSL::processPix( Vector<ShaderComponent*> &componentList, 
                                    const MaterialFeatureData &fd )
{   
   AssertFatal( fd.features[MFT_RTLighting], 
      "PixelSpecularHLSL requires RTLighting to be enabled!" );

  // RTLighting should have spit out the 4 specular
   // powers for the 4 potential lights on this pass.
   // 
   // This can sometimes be NULL if RTLighting skips out
   // on us for lightmaps or missing normals.
   Var *specular = (Var*)LangElement::find( "specular" );
   if ( !specular )
      return;

   MultiLine *meta = new MultiLine;

   LangElement *specMul = new GenOp( "@", specular );
   LangElement *final = specMul;
   
   // mask out with lightmap if present
   if ( fd.features[MFT_LightMap] )
   {
      LangElement *lmColor = NULL;
      
      // find lightmap color
      lmColor = LangElement::find( "lmColor" );
      
      if ( !lmColor )
      {
         LangElement * lightMap = LangElement::find( "lightMap" );
         LangElement * lmCoord = LangElement::find( "texCoord2" );

         lmColor = new GenOp( "texture(@, @)", lightMap, lmCoord );
      }
   
      final = new GenOp( "@ * vec4(@.rgb,0)", specMul, lmColor );
   }

   // If we have a normal map then mask the specular 
   if ( fd.features[MFT_SpecularMap] )
   {
      Var *specularColor = (Var*)LangElement::find( "specularColor" );
      if (specularColor)
         final = new GenOp( "@ * @", final, specularColor );
   }
   else if ( fd.features[MFT_NormalMap] && !fd.features[MFT_IsDXTnm] )
   {
      Var *bumpColor = (Var*)LangElement::find( "bumpNormal" );
      final = new GenOp( "@ * @.a", final, bumpColor );
   }

   // Add the specular to the final color.   
   // search for color var
   Var *color = (Var*)LangElement::find( "col" );   
   meta->addStatement( new GenOp( "   @.rgb += ( @ ).rgb;\r\n", color, final ) );

   output = meta;
}

ShaderFeature::Resources PixelSpecularGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res;
   return res;
}

void SpecularMapGLSL::processVert(Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd)
{
   MultiLine *meta = new MultiLine;

   // Add the texture coords.
   getOutTexCoord("texCoord",
     "vec2",
      true,
      fd.features[MFT_TexAnim],
      meta,
      componentList);

   output = meta;
}

void SpecularMapGLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // Get the texture coord.
   Var *texCoord = getInTexCoord( "texCoord", "vec2", true, componentList );

   // create texture var
   Var *specularMap = new Var;
   specularMap->setType( "sampler2D" );
   specularMap->setName( "specularMap" );
   specularMap->uniform = true;
   specularMap->sampler = true;
   specularMap->constNum = Var::getTexUnitNum();
   LangElement *texOp = new GenOp( "texture(@, @)", specularMap, texCoord );

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
      passData.mSamplerNames[ texIndex ] = "specularMap";
      passData.mTexSlot[ texIndex++ ].texObject = tex;
   }
}
