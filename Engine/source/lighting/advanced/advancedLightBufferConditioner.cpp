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
#include "lighting/advanced/advancedLightBufferConditioner.h"

#include "shaderGen/shaderOp.h"
#include "gfx/gfxDevice.h"
#include "core/util/safeDelete.h"


AdvancedLightBufferConditioner::~AdvancedLightBufferConditioner()
{
}

Var *AdvancedLightBufferConditioner::_conditionOutput( Var *unconditionedOutput, MultiLine *meta )
{
   Var *conditionedOutput = new Var;

   if(GFX->getAdapterType() == OpenGL)
      conditionedOutput->setType("vec4");
   else
      conditionedOutput->setType("float4");

   DecOp *outputDecl = new DecOp(conditionedOutput);
   if(mColorFormat == RGB)
   {
      conditionedOutput->setName("rgbLightInfoOut");

      // If this is a 16 bit integer format, scale up/down the values. All other
      // formats just write out the full 0..1
      if(getBufferFormat() == GFXFormatR16G16B16A16)
         meta->addStatement( new GenOp( "   @ = max(4.0, (float4(lightColor, specular) * NL_att + float4(bufferSample.rgb, 0.0)) / 4.0);\r\n", outputDecl ) );
      else
         meta->addStatement( new GenOp( "   @ = float4(lightColor, 0) * NL_att + float4(bufferSample.rgb, specular);\r\n", outputDecl ) );
   }
   else
   {
      // Input u'v' assumed to be scaled
      conditionedOutput->setName("luvLightInfoOut");
      meta->addStatement( new GenOp( "   @ = float4( lerp(bufferSample.xy, lightColor.xy, saturate(NL_att / bufferSample.z) * 0.5),\r\n", outputDecl ) );
      meta->addStatement( new GenOp( "               bufferSample.z + NL_att, bufferSample.w + saturate(specular * NL_att) );\r\n" ) );
   }

   return conditionedOutput;
}

Var *AdvancedLightBufferConditioner::_unconditionInput( Var *conditionedInput, MultiLine *meta )
{
   if(mColorFormat == RGB)
   {
      if(getBufferFormat() == GFXFormatR16G16B16A16)
         meta->addStatement( new GenOp( "   lightColor = @.rgb * 4.0;\r\n", conditionedInput ) );
      else
         meta->addStatement( new GenOp( "   lightColor = @.rgb;\r\n", conditionedInput ) );
      meta->addStatement( new GenOp( "   NL_att = dot(@.rgb, float3(0.3576, 0.7152, 0.1192));\r\n", conditionedInput ) );
   }
   else
   {
      meta->addStatement( new GenOp( "   // TODO: This clamps HDR values.\r\n" ) );
      meta->addStatement( new GenOp( "   NL_att = @.b;\r\n", conditionedInput ) );
      meta->addStatement( new GenOp( "   lightColor = DecodeLuv(float3(saturate(NL_att), @.rg * 0.62));\r\n", conditionedInput ) );
   }
   meta->addStatement( new GenOp( "   specular = @.a;\r\n", conditionedInput ) );

   return NULL;
}

Var *AdvancedLightBufferConditioner::printMethodHeader( MethodType methodType, const String &methodName, Stream &stream, MultiLine *meta )
{
   Var *methodVar = new Var;
   methodVar->setName(methodName);
   DecOp *methodDecl = new DecOp(methodVar);

   Var *lightColor = new Var;
   lightColor->setName("lightColor");
   DecOp *lightColorDecl = new DecOp(lightColor);

   Var *NLAtt = new Var;
   NLAtt->setName("NL_att");
   DecOp *NLAttDecl = new DecOp(NLAtt);

   Var *specular = new Var;
   specular->setName("specular");
   DecOp *specularDecl = new DecOp(specular);

   Var *bufferSample = new Var;
   bufferSample->setName("bufferSample");
   DecOp *bufferSampleDecl = new DecOp(bufferSample);

   const bool isCondition = ( methodType == ConditionerFeature::ConditionMethod );

   if(GFX->getAdapterType() == OpenGL)
   {
      methodVar->setType(avar("%s", isCondition ? "vec4" : "void"));
      lightColor->setType(avar("%s vec3", isCondition ? "in" : "out"));
      NLAtt->setType(avar("%s float", isCondition ? "in" : "out"));
      specular->setType(avar("%s float", isCondition ? "in" : "out"));
      bufferSample->setType("in vec4");
   }
   else
   {
      methodVar->setType(avar("inline %s", isCondition ? "float4" : "void"));
      lightColor->setType(avar("%s float3", isCondition ? "in" : "out"));
      NLAtt->setType(avar("%s float", isCondition ? "in" : "out"));
      specular->setType(avar("%s float", isCondition ? "in" : "out"));
      bufferSample->setType("in float4");
   }

   // If this is LUV, print methods to convert RGB<->LUV as needed
   if(mColorFormat == LUV)
   {
      if(!isCondition)
      {
         meta->addStatement( new GenOp( "float3 DecodeLuv(float3 Luv)\r\n{\r\n" ) );
         meta->addStatement( new GenOp( "   float2 xy = float2(9.0f, 4.0f) * Luv.yz / (dot(Luv.yz, float2(6.0f, -16.0f)) + 12.0f);\r\n" ) );
         meta->addStatement( new GenOp( "   float Ld = Luv.x;\r\n" ) );
         meta->addStatement( new GenOp( "   float3 XYZ = float3(xy.x, Ld, 1.0f - xy.x - xy.y);\r\n" ) );
         meta->addStatement( new GenOp( "   XYZ.xz = XYZ.xz * Ld / xy.y;\r\n" ) );
         meta->addStatement( new GenOp( "   const float3x3 XYZ2RGB =\r\n" ) );
         meta->addStatement( new GenOp( "   {\r\n" ) );
         meta->addStatement( new GenOp( "      2.5651f,    -1.1665f,   -0.3986f,\r\n" ) );
         meta->addStatement( new GenOp( "      -1.0217f,   1.9777f,    0.0439f,\r\n" ) );
         meta->addStatement( new GenOp( "      0.0753f,    -0.2543f,   1.1892f\r\n" ) );
         meta->addStatement( new GenOp( "   };\r\n" ) );
         meta->addStatement( new GenOp( "   return tMul(XYZ2RGB, XYZ);\r\n" ) );
         meta->addStatement( new GenOp( "}\r\n\r\n" ) );
      }
      else
      {
         // Shouldn't need this
      }
   }

   // Method header and opening bracket
   if(isCondition)
   {
      // All parameters are input parameters, and the return value is float4.
      // If this is an LUV buffer format, than the previous pixel value is needed
      // for interpolation.
      meta->addStatement( new GenOp( "@(@, @, @, @)\r\n", methodDecl, lightColorDecl, NLAttDecl, specularDecl, bufferSampleDecl ) );
   }
   else
   {
      // Sample as input, parameters as output. Void return.
      meta->addStatement( new GenOp( "@(@, @, @, @)\r\n", methodDecl, bufferSampleDecl, lightColorDecl, NLAttDecl, specularDecl ) );
   }

   meta->addStatement( new GenOp( "{\r\n" ) );

   // We don't use this way of passing var's around, so this should cause a crash
   // if something uses this improperly
   return ( isCondition ? NULL : bufferSample );
}

void AdvancedLightBufferConditioner::printMethodFooter( ConditionerFeature::MethodType methodType, Var *retVar, Stream &stream, MultiLine *meta )
{
   // Return and closing bracket
   if(methodType == ConditionerFeature::ConditionMethod)
      meta->addStatement( new GenOp( "\r\n   return @;\r\n", retVar ) );

   // Uncondition will assign output parameters 
   meta->addStatement( new GenOp( "}\r\n" ) );
}