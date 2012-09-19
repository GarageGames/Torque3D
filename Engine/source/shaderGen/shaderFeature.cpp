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
#include "shaderGen/shaderFeature.h"

#include "shaderGen/langElement.h"
#include "shaderGen/shaderOp.h"


void ShaderFeature::addDependency( const ShaderDependency *dependsOn )
{
   for ( U32 i = 0; i < mDependencies.size(); i++ )
   {
      if ( *mDependencies[i] == *dependsOn )
         return;
   }

   mDependencies.push_back( dependsOn );
}

ShaderFeature::Resources ShaderFeature::getResources( const MaterialFeatureData &fd )
{
   Resources temp; 
   return temp; 
}

const char* ShaderFeature::getOutputTargetVarName( OutputTarget target ) const
{
   const char* targName = "col";
   if ( target != DefaultTarget )
   {
      targName = "col1";
      AssertFatal(target == RenderTarget1, "yeah Pat is lame and didn't want to do bit math stuff, TODO");
   }

   return targName;
}

Var* ShaderFeature::findOrCreateLocal( const char *name, 
                                       const char *type, 
                                       MultiLine *multi )
{
   Var *outVar = (Var*)LangElement::find( name );
   if ( !outVar )
   {
      outVar = new Var;
      outVar->setType( type );
      outVar->setName( name );
      multi->addStatement( new GenOp( "   @;\r\n", new DecOp( outVar ) ) );
   }

   return outVar;
}