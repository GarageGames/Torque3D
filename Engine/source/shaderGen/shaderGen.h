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
#ifndef _SHADERGEN_H_
#define _SHADERGEN_H_

#ifndef _LANG_ELEMENT_H_
#include "shaderGen/langElement.h"
#endif
#ifndef _SHADERFEATURE_H_
#include "shaderGen/shaderFeature.h"
#endif
#ifndef _SHADERCOMP_H_
#include "shaderGen/shaderComp.h"
#endif
#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif
#ifndef _AUTOPTR_H_
#include "core/util/autoPtr.h"
#endif
#ifndef _TSINGLETON_H_
#include "core/util/tSingleton.h"
#endif
#ifndef _VOLUME_H_
#include "core/volume.h"
#endif
#ifndef _MATERIALFEATUREDATA_H_
#include "materials/materialFeatureData.h"
#endif


/// Base class used by shaderGen to be API agnostic.  Subclasses implement the various methods
/// in an API specific way.
class ShaderGenPrinter
{
public:
   virtual ~ShaderGenPrinter() {}
   
   /// Prints a simple header, including the engine name, language type, and
   /// the fact that the shader was procedurally generated
   virtual void printShaderHeader(Stream& stream) = 0;

   /// Prints a comment block specifying the beginning of the main() function (or equivalent)
   virtual void printMainComment(Stream& stream) = 0;

   /// Prints the final line of the vertex shader, e.g. return OUT; }, }, END
   virtual void printVertexShaderCloser(Stream& stream) = 0;

   /// Prints the output struct for the pixel shader.  Probably only used in HLSL/Cg.
   virtual void printPixelShaderOutputStruct(Stream& stream, const MaterialFeatureData &featureData) = 0;

   /// Prints the final line of the pixel shader.
   virtual void printPixelShaderCloser(Stream& stream) = 0;
   
   // Prints a line into the shader adding the proper terminator.
   virtual void printLine(Stream& stream, const String& line) = 0;
};

/// Abstract factory for created (and initializating, if necessary) shader components.
class ShaderGenComponentFactory
{
public:
   virtual ~ShaderGenComponentFactory() {}

   /// Creates and initializes a vertex input connector with the specified flags
   virtual ShaderComponent* createVertexInputConnector( const GFXVertexFormat &vertexFormat ) = 0;

   /// Creates and names a vertex/pixel connector
   virtual ShaderComponent* createVertexPixelConnector() = 0;

   /// Creates an instance of VertexParamsDef
   virtual ShaderComponent* createVertexParamsDef() = 0;

   /// Creates an instance of PixelParamsDef
   virtual ShaderComponent* createPixelParamsDef() = 0;
};

//**************************************************************************
/*!
   The ShaderGen class takes shader feature data (usually created by 
   MatInstance) and creates a vertex/pixel shader pair in text files
   to be later compiled by a shader manager.
   
   It accomplishes this task by creating a group of shader "components" and
   "features" that output bits of high level shader code.  Shader components
   translate to structures in HLSL that indicate incoming vertex data,
   data that is output from the vertex shader to the pixel shader, and data
   such as constants and textures that are passed directly to the shader
   from the app.

   Shader features are separable shader functions that can be turned on or
   off.  Examples would be bumpmapping and specular highlights.  See 
   MaterialFeatureData for the current list of features supported.

   ShaderGen processes all of the features that are present for a desired
   shader, and then prints them out to the respective vertex or pixel
   shader file.
   
   For more information on shader features and components see the 
   ShaderFeature and ShaderComponent classes.
*/
//**************************************************************************


//**************************************************************************
// Shader generator
//**************************************************************************
class ShaderGen
{
public:
   virtual ~ShaderGen();

   /// Parameter 1 is the ShaderGen instance to initialize.
   typedef Delegate<void (ShaderGen*)> ShaderGenInitDelegate;

   /// Register an initialization delegate for adapterType.  This should setPrinter/ComponentFactory/etc, and register
   /// shader features.
   void registerInitDelegate(GFXAdapterType adapterType, ShaderGenInitDelegate& initDelegate);

   /// Signal used to notify systems to register features.
   typedef Signal<void(GFXAdapterType type)> FeatureInitSignal;

   /// Returns the signal used to notify systems to register features.
   FeatureInitSignal& getFeatureInitSignal() { return mFeatureInitSignal; }

   /// vertFile and pixFile are filled in by this function.  They point to 
   /// the vertex and pixel shader files.  pixVersion is also filled in by
   /// this function.
   /// @param assignNum used to assign a specific number as the filename   
   void generateShader( const MaterialFeatureData &featureData,
                        char *vertFile, 
                        char *pixFile, 
                        F32 *pixVersion,
                        const GFXVertexFormat *vertexFormat,
                        const char* cacheName,
                        Vector<GFXShaderMacro> &macros );

   // Returns a shader that implements the features listed by dat.
   GFXShader* getShader( const MaterialFeatureData &dat, const GFXVertexFormat *vertexFormat, const Vector<GFXShaderMacro> *macros, const Vector<String> &samplers );

   // This will delete all of the procedural shaders that we have.  Used to regenerate shaders when
   // the ShaderFeatures have changed (due to lighting system change, or new plugin)
   virtual void flushProceduralShaders();

   void setPrinter(ShaderGenPrinter* printer) { mPrinter = printer; }
   void setComponentFactory(ShaderGenComponentFactory* factory) { mComponentFactory = factory; }
   void setFileEnding(String ending) { mFileEnding = ending; }

protected:   

   friend class ManagedSingleton<ShaderGen>;

   // Shader generation 
   MaterialFeatureData  mFeatureData;
   const GFXVertexFormat *mVertexFormat;
   
   Vector< ShaderComponent *> mComponents;

   AutoPtr<ShaderGenPrinter> mPrinter;
   AutoPtr<ShaderGenComponentFactory> mComponentFactory;

   String mFileEnding;

   /// The currently processing output.
   MultiLine *mOutput;
   GFXVertexFormat mInstancingFormat;

   /// Init 
   bool mInit;
   ShaderGenInitDelegate mInitDelegates[GFXAdapterType_Count];
   FeatureInitSignal mFeatureInitSignal;
   bool mRegisteredWithGFX;
   Torque::FS::FileSystemRef mMemFS;
   
   /// Map of cache string -> shaders
   typedef Map<String, GFXShaderRef> ShaderMap;
   ShaderMap mProcShaders;

   ShaderGen();

   bool _handleGFXEvent(GFXDevice::GFXDeviceEventType event);
   
   /// Causes the init delegate to be called.
   void initShaderGen();

   void _init();
   void _uninit();

   /// Creates all the various shader components that will be filled in when 
   /// the shader features are processed.
   void _createComponents();

   void _printFeatureList(Stream &stream);

   /// print out the processed features to the file stream
   void _printFeatures( Stream &stream );

   void _printDependencies( Stream &stream );

   void _processPixFeatures( Vector<GFXShaderMacro> &macros, bool macrosOnly = false );
   void _printPixShader( Stream &stream );

   void _processVertFeatures( Vector<GFXShaderMacro> &macros, bool macrosOnly = false );
   void _printVertShader( Stream &stream );

   // For ManagedSingleton.
   static const char* getSingletonName() { return "ShaderGen"; }   
};


/// Returns the ShaderGen singleton.
#define SHADERGEN ManagedSingleton<ShaderGen>::instance()

#endif // _SHADERGEN_H_
