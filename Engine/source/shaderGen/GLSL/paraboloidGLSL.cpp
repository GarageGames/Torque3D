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
#include "shaderGen/GLSL/paraboloidGLSL.h"

#include "lighting/lightInfo.h"
#include "materials/sceneData.h"
#include "materials/materialFeatureTypes.h"
#include "materials/materialFeatureData.h"
#include "gfx/gfxShader.h"


void ParaboloidVertTransformGLSL::processVert(  Vector<ShaderComponent*> &componentList, 
                                                const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;

   // First check for an input position from a previous feature
   // then look for the default vertex position.
   Var *inPosition = (Var*)LangElement::find( "inPosition" );
   if ( !inPosition )
      inPosition = (Var*)LangElement::find( "position" );

   const bool isSinglePass = fd.features[ MFT_IsSinglePassParaboloid ];

   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );

   // Grab connector out position.
   Var *outPosition = connectComp->getElement( RT_POSITION );
   outPosition->setName( "gl_Position" );

   // Get the atlas scale.
   Var *atlasScale = new Var;
   atlasScale->setType( "vec2" );
   atlasScale->setName( "atlasScale" );
   atlasScale->uniform = true;
   atlasScale->constSortPos = cspPass;  

	// Transform into camera space
   Var *worldViewOnly = getWorldView( componentList, fd.features[MFT_UseInstancing], meta );
	
   // So what we're doing here is transforming into camera space, and
   // then directly manipulate into shadowmap space.
   //
   // http://www.gamedev.net/reference/articles/article2308.asp

   // Swizzle z and y post-transform
   meta->addStatement( new GenOp( "   @ = vec4(@ * vec4(@.xyz,1)).xzyw;\r\n", outPosition, worldViewOnly, inPosition ) );
   meta->addStatement( new GenOp( "   float L = length(@.xyz);\r\n", outPosition ) ); 

   if ( isSinglePass )
   {
      // Flip the z in the back case
      Var *outIsBack = connectComp->getElement( RT_TEXCOORD );
      outIsBack->setType( "float" );
      outIsBack->setName( "outIsBack" );

      meta->addStatement( new GenOp( "   bool isBack = @.z < 0.0;\r\n", outPosition ) ); 
      meta->addStatement( new GenOp( "   @ = isBack ? -1.0 : 1.0;\r\n", outIsBack ) ); 
      meta->addStatement( new GenOp( "   if ( isBack ) @.z = -@.z;\r\n", outPosition, outPosition ) );
   }

   meta->addStatement( new GenOp( "   @ /= L;\r\n", outPosition ) ); 
   meta->addStatement( new GenOp( "   @.z = @.z + 1.0;\r\n", outPosition, outPosition ) ); 
   meta->addStatement( new GenOp( "   @.xy /= @.z;\r\n", outPosition, outPosition ) ); 

   // Get the light parameters.
   Var *lightParams = new Var;
   lightParams->setType( "vec4" );
   lightParams->setName( "lightParams" );
   lightParams->uniform = true;
   lightParams->constSortPos = cspPass;  

   // TODO: If we change other shadow shaders to write out
   // linear depth, than fix this as well!
   //
   // (L - 1.0)/(lightParams.x - 1.0);
   //
   meta->addStatement( new GenOp( "   @.z = L / @.x;\r\n", outPosition, lightParams ) ); 
   meta->addStatement( new GenOp( "   @.w = 1.0;\r\n", outPosition ) ); 

   // Pass unmodified to pixel shader to allow it to clip properly.
   Var *outPosXY = connectComp->getElement( RT_TEXCOORD );
   outPosXY->setType( "vec2" );
   outPosXY->setName( "outPosXY" );
   meta->addStatement( new GenOp( "   @ = @.xy;\r\n", outPosXY, outPosition ) ); 

   // Scale and offset so it shows up in the atlas properly.
   meta->addStatement( new GenOp( "   @.xy *= @.xy;\r\n", outPosition, atlasScale ) ); 

   if ( isSinglePass )
      meta->addStatement( new GenOp( "   @.x += isBack ? 0.5 : -0.5;\r\n", outPosition ) );
   else
   {
      Var *atlasOffset = new Var;
      atlasOffset->setType( "vec2" );
      atlasOffset->setName( "atlasXOffset" );
      atlasOffset->uniform = true;
      atlasOffset->constSortPos = cspPass;  

      meta->addStatement( new GenOp( "   @.xy += @;\r\n", outPosition, atlasOffset ) );
   }

   output = meta;
}

void ParaboloidVertTransformGLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                                const MaterialFeatureData &fd )
{      
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );

   MultiLine *meta = new MultiLine;

   const bool isSinglePass = fd.features[ MFT_IsSinglePassParaboloid ];
   if ( isSinglePass )
   {
      // Cull things on the back side of the map.
      Var *isBack = connectComp->getElement( RT_TEXCOORD );
      isBack->setName( "outIsBack" );
      isBack->setType( "float" );
      meta->addStatement( new GenOp( "   if ( ( abs( @ ) - 0.999 ) < 0 ) discard;\r\n", isBack ) );
   }

   // Cull pixels outside of the valid paraboloid.
   Var *posXY = connectComp->getElement( RT_TEXCOORD );
   posXY->setName( "outPosXY" );
   posXY->setType( "vec2" );
   meta->addStatement( new GenOp( "   if ( ( 1.0 - length( @ ) ) < 0 ) discard;\r\n", posXY ) );

   output = meta;
}

ShaderFeature::Resources ParaboloidVertTransformGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources temp;
   temp.numTexReg = 2;
   return temp; 
}
