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
#ifndef _CONDITIONER_BASE_H_
#define _CONDITIONER_BASE_H_

#ifndef _SHADERFEATURE_H_
#include "shaderGen/shaderFeature.h"
#endif
#ifndef _SHADER_DEPENDENCY_H_
#include "shaderGen/shaderDependency.h"
#endif

class MultiLine;
class ConditionerMethodDependency;


class ConditionerFeature : public ShaderFeature
{
   friend class ConditionerMethodDependency;

   typedef ShaderFeature Parent;

public:

   enum MethodType
   {
      ConditionMethod = 0, ///< Method used to take unconditioned data, and turn it into a format that can be written to the conditioned buffer
      UnconditionMethod, ///< Method used to take conditioned data from a buffer, and extract what is stored
      NumMethodTypes,
   };

   ConditionerFeature( const GFXFormat bufferFormat );
   virtual ~ConditionerFeature();

   virtual Material::BlendOp getBlendOp()
   { 
      return Material::None; 
   }

   virtual GFXFormat getBufferFormat() const { return mBufferFormat; }
   virtual bool setBufferFormat(const GFXFormat bufferFormat) { bool ret = mBufferFormat == bufferFormat; mBufferFormat = bufferFormat; return ret; }

   // zero-out these methods
   virtual Var* getVertTexCoord( const String &name ) { AssertFatal( false, "don't use this." ); return NULL; }
   virtual LangElement *setupTexSpaceMat(  Vector<ShaderComponent*> &componentList, Var **texSpaceMat ) { AssertFatal( false, "don't use this." );  return NULL; }
   virtual LangElement *expandNormalMap( LangElement *sampleNormalOp, LangElement *normalDecl, LangElement *normalVar, const MaterialFeatureData &fd ) { AssertFatal( false, "don't use this." );  return NULL; }
   virtual LangElement *assignColor( LangElement *elem, Material::BlendOp blend, LangElement *lerpElem = NULL, ShaderFeature::OutputTarget outputTarget = ShaderFeature::DefaultTarget ) { AssertFatal( false, "don't use this." ); return NULL; }

   // conditioned output
   virtual LangElement *assignOutput( Var *unconditionedOutput, ShaderFeature::OutputTarget outputTarget = ShaderFeature::DefaultTarget );

   // Get an HLSL/GLSL method name that will be available for the 
   // shader to read or write data to a conditioned buffer.
   virtual const String &getShaderMethodName( MethodType methodType );

   // Get the Method Dependency for ShaderGen, for this conditioner
   virtual ConditionerMethodDependency *getConditionerMethodDependency( MethodType methodType );

   static const String ConditionerIncludeFileName;

   static void updateConditioners() { if ( smDirtyConditioners ) _updateConditioners(); }

protected:

   static void _updateConditioners();

   static bool smDirtyConditioners;

   ConditionerMethodDependency *mMethodDependency[NumMethodTypes];

   static Vector<ConditionerFeature*> smConditioners;

   GFXFormat mBufferFormat;

   String mUnconditionMethodName;

   String mConditionMethodName;

   String mShaderIncludePath;

   void _print( Stream *stream );

   virtual Var *_conditionOutput( Var *unconditionedOutput, MultiLine *meta );
   virtual Var *_unconditionInput( Var *conditionedInput, MultiLine *meta );

   // Print method header, return primary parameter
   virtual Var *printMethodHeader( MethodType methodType, const String &methodName, Stream &stream, MultiLine *meta );
   virtual void printMethodFooter( MethodType methodType, Var *retVar, Stream &stream, MultiLine *meta );

   // Print comments
   virtual void printHeaderComment( MethodType methodType, const String &methodName, Stream &stream, MultiLine *meta );
   virtual void printFooterComment( MethodType methodType, const String &methodName, Stream &stream, MultiLine *meta );

   // print a HLSL/GLSL method to a stream, which can be used by a custom shader
   // to read conditioned data
   virtual void _printMethod( MethodType methodType, const String &methodName, Stream &stream );
};

//------------------------------------------------------------------------------

// ShaderDependancy that allows shadergen features to add a dependency on a conditioner method
class ConditionerMethodDependency : public ShaderDependency
{
protected:
   ConditionerFeature *mConditioner;
   ConditionerFeature::MethodType mMethodType;

public:
   ConditionerMethodDependency( ConditionerFeature *conditioner, const ConditionerFeature::MethodType methodType ) :
      mConditioner(conditioner), mMethodType(methodType) {}

   virtual void print( Stream &s ) const;

   // Auto insert information into a macro
   virtual void createMethodMacro( const String &methodName, Vector<GFXShaderMacro> &macros );
};

#endif // _CONDITIONER_BASE_H_
