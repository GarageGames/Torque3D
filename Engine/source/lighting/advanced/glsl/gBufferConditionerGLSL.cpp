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
#include "lighting/advanced/glsl/gBufferConditionerGLSL.h"

#include "shaderGen/featureMgr.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "materials/materialFeatureTypes.h"
#include "materials/materialFeatureData.h"


GBufferConditionerGLSL::GBufferConditionerGLSL( const GFXFormat bufferFormat ) :
      Parent( bufferFormat )
{
   // Figure out how we should store the normal data. These are the defaults.
   mCanWriteNegativeValues = false;
   mNormalStorageType = CartesianXYZ;

   // Note:  We clear to a depth 1 (the w component) so
   // that the unrendered parts of the scene end up 
   // farthest to the camera.

   switch(bufferFormat)
   {
      case GFXFormatR8G8B8A8:
         // TODO: Some kind of logic here. Spherical is better, but is more
         // expensive. 
         mNormalStorageType = Spherical;
         mBitsPerChannel = 8;
         break;

      case GFXFormatR16G16B16A16F:
         // Floating point buffers don't need to encode negative values
         mCanWriteNegativeValues = true;
         mNormalStorageType = Spherical;
         mBitsPerChannel = 16;
         break;

      // Store a 32bit depth with a sperical normal in the
      // integer 16 format.  This gives us perfect depth 
      // precision and high quality normals within a 64bit
      // buffer format.
      case GFXFormatR16G16B16A16:
         mNormalStorageType = Spherical;
         mBitsPerChannel = 16;
         break;

      case GFXFormatR32G32B32A32F:
         mCanWriteNegativeValues = true;
         mNormalStorageType = CartesianXYZ;
         mBitsPerChannel = 32;
         break;

      default:
         AssertFatal(false, "Unsupported G-Buffer format");
   }
}

GBufferConditionerGLSL::~GBufferConditionerGLSL()
{
}

void GBufferConditionerGLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                          const MaterialFeatureData &fd )
{
   output = NULL;

   if( !fd.features[MFT_NormalMap] )
   {
      // grab incoming vert normal
      Var *inNormal = (Var*) LangElement::find( "normal" );
      AssertFatal( inNormal, "Something went bad with ShaderGen. The normal should be already defined." );

      // grab output for gbuffer normal
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
      Var *outNormal = connectComp->getElement( RT_TEXCOORD );
      outNormal->setName( "gbNormal" );
      outNormal->setType( "vec3" );

      // create objToWorld variable
      Var *objToWorld = (Var*) LangElement::find( "objTrans" );
      if( !objToWorld )
      {
         objToWorld = new Var;
         objToWorld->setType( "mat4" );
         objToWorld->setName( "objTrans" );
         objToWorld->uniform = true;
         objToWorld->constSortPos = cspPrimitive;   
      }

      // Kick out the world-space normal
      LangElement *statement = new GenOp( "   @ = vec3(@ * vec4(normalize(@), 0.0));\r\n", outNormal, objToWorld, inNormal );
      output = statement;
   }
}

void GBufferConditionerGLSL::processPix(  Vector<ShaderComponent*> &componentList, 
                                          const MaterialFeatureData &fd )
{     
   // sanity
   AssertFatal( fd.features[MFT_EyeSpaceDepthOut], "No depth-out feature enabled! Bad news!" );

   MultiLine *meta = new MultiLine;

   // grab connector normal
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *gbNormal = (Var*) LangElement::find( "gbNormal" );
   if( !gbNormal )
   {
      gbNormal = connectComp->getElement( RT_TEXCOORD );
      gbNormal->setName( "gbNormal" );
      gbNormal->setType( "vec3" );
      gbNormal->mapsToSampler = false;
      gbNormal->uniform = false;
   }

   // find depth
   ShaderFeature *depthFeat = FEATUREMGR->getByType( MFT_EyeSpaceDepthOut );
   AssertFatal( depthFeat != NULL, "No eye space depth feature found!" );

   Var *depth = (Var*) LangElement::find(depthFeat->getOutputVarName());
   AssertFatal( depth, "Something went bad with ShaderGen. The depth should be already generated by the EyeSpaceDepthOut feature." );


   Var *unconditionedOut = new Var;
   unconditionedOut->setType("vec4");
   unconditionedOut->setName("normal_depth");

   LangElement *outputDecl = new DecOp( unconditionedOut );

   // NOTE: We renormalize the normal here as they
   // will not stay normalized during interpolation.
   meta->addStatement( new GenOp("   @ = @;", outputDecl, new GenOp( "vec4(normalize(@), @)", gbNormal, depth ) ) );
   meta->addStatement( assignOutput( unconditionedOut ) );

   output = meta;
}

ShaderFeature::Resources GBufferConditionerGLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res;

   // Passing from VS->PS:
   // - world space normal (gbNormal)
   res.numTexReg = 1;

   return res; 
}

Var* GBufferConditionerGLSL::printMethodHeader( MethodType methodType, const String &methodName, Stream &stream, MultiLine *meta )
{
   const bool isCondition = ( methodType == ConditionerFeature::ConditionMethod );

   Var *retVal = NULL;

   // The uncondition method inputs are changed
   if( isCondition )
      retVal = Parent::printMethodHeader( methodType, methodName, stream, meta );
   else
   {
      Var *methodVar = new Var;
      methodVar->setName(methodName);
      methodVar->setType("vec4");
      DecOp *methodDecl = new DecOp(methodVar);

      Var *prepassSampler = new Var;
      prepassSampler->setName("prepassSamplerVar");
      prepassSampler->setType("sampler2D");
      DecOp *prepassSamplerDecl = new DecOp(prepassSampler);

      Var *screenUV = new Var;
      screenUV->setName("screenUVVar");
      screenUV->setType("vec2");
      DecOp *screenUVDecl = new DecOp(screenUV);

      Var *bufferSample = new Var;
      bufferSample->setName("bufferSample");
      bufferSample->setType("vec4");
      DecOp *bufferSampleDecl = new DecOp(bufferSample); 

      meta->addStatement( new GenOp( "@(@, @)\r\n", methodDecl, prepassSamplerDecl, screenUVDecl ) );

      meta->addStatement( new GenOp( "{\r\n" ) );

      meta->addStatement( new GenOp( "   // Sampler g-buffer\r\n" ) );

      // The gbuffer has no mipmaps, so use tex2dlod when 
      // so that the shader compiler can optimize.
      meta->addStatement( new GenOp( "   @ = texture2DLod(@, @, 0.0);\r\n", bufferSampleDecl, prepassSampler, screenUV ) );

      // We don't use this way of passing var's around, so this should cause a crash
      // if something uses this improperly
      retVal = bufferSample;
   }

   return retVal;
}

GenOp* GBufferConditionerGLSL::_posnegEncode( GenOp *val )
{
   return mCanWriteNegativeValues ? val : new GenOp("0.5 * (@ + 1.0)", val);
}

GenOp* GBufferConditionerGLSL::_posnegDecode( GenOp *val )
{
   return mCanWriteNegativeValues ? val : new GenOp("@ * 2.0 - 1.0", val);
}

Var* GBufferConditionerGLSL::_conditionOutput( Var *unconditionedOutput, MultiLine *meta )
{
   Var *retVar = new Var;
   retVar->setType("vec4");
   retVar->setName("_gbConditionedOutput");
   LangElement *outputDecl = new DecOp( retVar );

   switch(mNormalStorageType)
   {
      case CartesianXYZ:
         meta->addStatement( new GenOp( "   // g-buffer conditioner: vec4(normal.xyz, depth)\r\n" ) );
         meta->addStatement( new GenOp( "   @ = vec4(@, @.a);\r\n", outputDecl, 
            _posnegEncode(new GenOp("@.xyz", unconditionedOutput)), unconditionedOutput ) );
         break;

      case CartesianXY:
         meta->addStatement( new GenOp( "   // g-buffer conditioner: vec4(normal.xy, depth Hi + z-sign, depth Lo)\r\n" ) );
         meta->addStatement( new GenOp( "   @ = vec4(@, @.a);", outputDecl, 
            _posnegEncode(new GenOp("vec3(@.xy, sign(@.z))", unconditionedOutput, unconditionedOutput)), unconditionedOutput ) );
         break;

      case Spherical:
         meta->addStatement( new GenOp( "   // g-buffer conditioner: vec4(normal.theta, normal.phi, depth Hi, depth Lo)\r\n" ) );
         meta->addStatement( new GenOp( "   @ = vec4(@, 0.0, @.a);\r\n", outputDecl, 
            _posnegEncode(new GenOp("vec2(atan2(@.y, @.x) / 3.14159265358979323846f, @.z)", unconditionedOutput, unconditionedOutput, unconditionedOutput ) ), 
            unconditionedOutput ) );
         break;
   }

   // Encode depth into two channels
   if(mNormalStorageType != CartesianXYZ)
   {
      const U64 maxValPerChannel = 1 << mBitsPerChannel;
      const U64 extraVal = (maxValPerChannel * maxValPerChannel - 1) - (maxValPerChannel - 1) * 2;
      meta->addStatement( new GenOp( "   \r\n   // Encode depth into hi/lo\r\n" ) );
      meta->addStatement( new GenOp( avar( "   vec3 _tempDepth = fract(@.a * vec3(1.0, %llu.0, %llu.0));\r\n", maxValPerChannel - 1, extraVal ), 
         unconditionedOutput ) );
      meta->addStatement( new GenOp( avar( "   @.zw = _tempDepth.xy - _tempDepth.yz * vec2(1.0/%llu.0, 1.0/%llu.0);\r\n\r\n", maxValPerChannel - 1, maxValPerChannel - 1  ), 
         retVar ) );
   }

   AssertFatal( retVar != NULL, avar( "Cannot condition output to buffer format: %s", GFXStringTextureFormat[getBufferFormat()] ) );
   return retVar; 
}

Var* GBufferConditionerGLSL::_unconditionInput( Var *conditionedInput, MultiLine *meta )
{
   Var *retVar = new Var;
   retVar->setType("vec4");
   retVar->setName("_gbUnconditionedInput");
   LangElement *outputDecl = new DecOp( retVar );

   switch(mNormalStorageType)
   {
      case CartesianXYZ:
         meta->addStatement( new GenOp( "   // g-buffer unconditioner: vec4(normal.xyz, depth)\r\n" ) );
         meta->addStatement( new GenOp( "   @ = vec4(@, @.a);\r\n", outputDecl, 
            _posnegDecode(new GenOp("@.xyz", conditionedInput)), conditionedInput ) );
         break;

      case CartesianXY:
         meta->addStatement( new GenOp( "   // g-buffer unconditioner: vec4(normal.xy, depth Hi + z-sign, depth Lo)\r\n" ) );
         meta->addStatement( new GenOp( "   @ = vec4(@, @.a);\r\n", outputDecl, 
            _posnegDecode(new GenOp("@.xyz", conditionedInput)), conditionedInput ) );
         meta->addStatement( new GenOp( "   @.z *= sqrt(1.0 - dot(@.xy, @.xy));\r\n", retVar, retVar, retVar ) );
         break;

      case Spherical:
         meta->addStatement( new GenOp( "   // g-buffer unconditioner: vec4(normal.theta, normal.phi, depth Hi, depth Lo)\r\n" ) );
         meta->addStatement( new GenOp( "   vec2 spGPUAngles = @;\r\n", _posnegDecode(new GenOp("@.xy", conditionedInput)) ) );
         meta->addStatement( new GenOp( "   vec2 sincosTheta;\r\n" ) );
         meta->addStatement( new GenOp( "   sincosTheta.x = sin(spGPUAngles.x * 3.14159265358979323846);\r\n" ) );
         meta->addStatement( new GenOp( "   sincosTheta.y = cos(spGPUAngles.x * 3.14159265358979323846);\r\n" ) );
         meta->addStatement( new GenOp( "   vec2 sincosPhi = vec2(sqrt(1.0 - spGPUAngles.y * spGPUAngles.y), spGPUAngles.y);\r\n" ) );
         meta->addStatement( new GenOp( "   @ = vec4(sincosTheta.y * sincosPhi.x, sincosTheta.x * sincosPhi.x, sincosPhi.y, @.a);\r\n", outputDecl, conditionedInput ) );
         break;
   }

   // Recover depth from encoding
   if(mNormalStorageType != CartesianXYZ)
   {
      const U64 maxValPerChannel = 1 << mBitsPerChannel;
      meta->addStatement( new GenOp( "   \r\n   // Decode depth\r\n" ) );
      meta->addStatement( new GenOp( avar( "   @.w = dot( @.zw, vec2(1.0, 1.0/%llu.0));\r\n", maxValPerChannel - 1 ), 
         retVar, conditionedInput ) );
   }


   AssertFatal( retVar != NULL, avar( "Cannot uncondition input from buffer format: %s", GFXStringTextureFormat[getBufferFormat()] ) );
   return retVar; 
}

