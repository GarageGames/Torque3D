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
#include "shaderGen/conditionerFeature.h"

#include "shaderGen/shaderOp.h"
#include "shaderGen/featureMgr.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "core/stream/fileStream.h"
#include "materials/shaderData.h"
#include "core/util/safeDelete.h"

const String ConditionerFeature::ConditionerIncludeFileName = "autogenConditioners.h";

bool ConditionerFeature::smDirtyConditioners = true;

Vector<ConditionerFeature*> ConditionerFeature::smConditioners;


ConditionerFeature::ConditionerFeature( const GFXFormat bufferFormat )
   :  mBufferFormat(bufferFormat)
{
   dMemset( mMethodDependency, 0, sizeof( mMethodDependency ) );

   smConditioners.push_back( this );
   smDirtyConditioners = true;
}

ConditionerFeature::~ConditionerFeature()
{
   for( U32 i = 0; i < NumMethodTypes; i++ )
      SAFE_DELETE( mMethodDependency[i] );

   smConditioners.remove( this );
   smDirtyConditioners = true;
}

LangElement *ConditionerFeature::assignOutput( Var *unconditionedOutput, ShaderFeature::OutputTarget outputTarget /* = ShaderFeature::DefaultTarget*/ )
{
   LangElement *assign;
   MultiLine *meta = new MultiLine;

   meta->addStatement( new GenOp( avar( "\r\n\r\n   // output buffer format: %s\r\n", GFXStringTextureFormat[getBufferFormat()] ) ) );

   // condition the output
   Var *conditionedOutput = _conditionOutput( unconditionedOutput, meta );

   // search for color var
   Var *color = (Var*) LangElement::find( getOutputTargetVarName(outputTarget) );

   if ( !color )
   {
      // create color var
      color = new Var;

      if(GFX->getAdapterType() == OpenGL)
      {
         color->setName( getOutputTargetVarName(outputTarget) );
         color->setType( "vec4" );
         DecOp* colDecl = new DecOp(color);

         assign = new GenOp( "@ = vec4(@)", colDecl, conditionedOutput );
      }
      else
      {
         color->setType( "fragout" );
         color->setName( getOutputTargetVarName(outputTarget) );
         color->setStructName( "OUT" );

         assign = new GenOp( "@ = @", color, conditionedOutput );
      }
   }
   else
   {
      if (GFX->getAdapterType() == OpenGL)
         assign = new GenOp( "@ = vec4(@)", color, conditionedOutput);
      else
         assign = new GenOp( "@ = @", color, conditionedOutput );
   }

   meta->addStatement( new GenOp( "   @;\r\n", assign ) );

   return meta;
}

Var *ConditionerFeature::_conditionOutput( Var *unconditionedOutput, MultiLine *meta )
{
   meta->addStatement( new GenOp( "   // generic conditioner: no conditioning performed\r\n" ) );
   return unconditionedOutput; 
}

Var *ConditionerFeature::_unconditionInput( Var *conditionedInput, MultiLine *meta )
{
   meta->addStatement( new GenOp( "   // generic conditioner: no conditioning performed\r\n" ) );
   return conditionedInput; 
}

const String &ConditionerFeature::getShaderMethodName( MethodType methodType )
{
   if ( mConditionMethodName.isEmpty() )
   {
      const U32 hash = getName().getHashCaseInsensitive();
      mUnconditionMethodName = avar("autogen%s_%08x", "Uncondition", hash );
      mConditionMethodName = avar("autogen%s_%08x", "Condition", hash );
   }

   return methodType == UnconditionMethod ? mUnconditionMethodName : mConditionMethodName;
}

ConditionerMethodDependency* ConditionerFeature::getConditionerMethodDependency( MethodType methodType )
{
   if ( mMethodDependency[methodType] == NULL )
      mMethodDependency[methodType] = new ConditionerMethodDependency( this, methodType );

   return mMethodDependency[methodType];
}

void ConditionerFeature::_print( Stream *stream )
{  
   _printMethod( ConditionMethod, getShaderMethodName( ConditionMethod ), *stream );
   LangElement::deleteElements();

   _printMethod( UnconditionMethod, getShaderMethodName( UnconditionMethod ), *stream );
   LangElement::deleteElements();
}

void ConditionerFeature::_updateConditioners()
{
   smDirtyConditioners = false;

   String includePath = "shadergen:/" + ConditionerIncludeFileName;

   FileStream stream;
   if ( !stream.open( includePath, Torque::FS::File::Write ) )
      return;

   for ( U32 i=0; i < smConditioners.size(); i++ )
      smConditioners[i]->_print( &stream );
}

Var *ConditionerFeature::printMethodHeader( MethodType methodType, const String &methodName, Stream &stream, MultiLine *meta )
{
   Var *methodVar = new Var;
   methodVar->setName(methodName);
   DecOp *methodDecl = new DecOp(methodVar);

   const bool isCondition = (methodType == ConditionerFeature::ConditionMethod);

   Var *paramVar = new Var;
   paramVar->setName(avar("%sconditioned%sput", isCondition ? "un" : "", isCondition ? "Out" : "In"));
   DecOp *paramDecl = new DecOp(paramVar);

   if(GFX->getAdapterType() == OpenGL)
   {
      methodVar->setType("vec4");
      paramVar->setType("vec4");
   }
   else
   {
      methodVar->setType("inline float4");
      paramVar->setType("in float4");
   }

   // Method header and opening bracket
   meta->addStatement( new GenOp( "@(@)\r\n", methodDecl, paramDecl ) );
   meta->addStatement( new GenOp( "{\r\n" ) );

   return paramVar;
}

void ConditionerFeature::printMethodFooter( MethodType methodType, Var *retVar, Stream &stream, MultiLine *meta )
{
   // Return and closing bracket
   meta->addStatement( new GenOp( "\r\n   return @;\r\n", retVar ) );
   meta->addStatement( new GenOp( "}\r\n" ) );
}

void ConditionerFeature::_printMethod( MethodType methodType, const String &methodName, Stream &stream )
{
   MultiLine *meta = new MultiLine;

   printHeaderComment( methodType, methodName, stream, meta );
   Var *paramVar = printMethodHeader( methodType, methodName, stream, meta );
   Var *unconditionedInput = NULL;
   if( methodType == UnconditionMethod )
      unconditionedInput = _unconditionInput( paramVar, meta );
   else
      unconditionedInput = _conditionOutput( paramVar, meta );

   printMethodFooter( methodType, unconditionedInput, stream, meta );
   printFooterComment( methodType, methodName, stream, meta );

   meta->print(stream);
}

void ConditionerFeature::printHeaderComment( MethodType methodType, const String &methodName, Stream &stream, MultiLine *meta )
{
   meta->addStatement( new GenOp( "//------------------------------------------------------------------------------\r\n" ) );
   meta->addStatement( new GenOp( avar( "// Autogenerated '%s' %s Method\r\n", getName().c_str(), 
      methodType == ConditionMethod ? "Condition" : "Uncondition" ) ) );
   meta->addStatement( new GenOp( "//------------------------------------------------------------------------------\r\n" ) );
}

void ConditionerFeature::printFooterComment( MethodType methodType, const String &methodName, Stream &stream, MultiLine *meta )
{
   meta->addStatement( new GenOp( "\r\n\r\n" ) );
}

void ConditionerMethodDependency::print( Stream &s ) const
{
   mConditioner->_printMethod(mMethodType, mConditioner->getShaderMethodName(mMethodType), s);
}

void ConditionerMethodDependency::createMethodMacro( const String &methodName, Vector<GFXShaderMacro> &macros )
{
   GFXShaderMacro conditionerMethodMacro;
   conditionerMethodMacro.name = methodName;
   conditionerMethodMacro.value = mConditioner->getShaderMethodName(mMethodType);
   macros.push_back(conditionerMethodMacro);
}
