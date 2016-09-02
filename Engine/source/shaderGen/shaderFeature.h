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
#ifndef _SHADERFEATURE_H_
#define _SHADERFEATURE_H_

#ifndef _MATERIALDEFINITION_H_
#include "materials/materialDefinition.h"
#endif
#ifndef _SHADERCOMP_H_
#include "shaderGen/shaderComp.h"
#endif
#ifndef _SHADER_DEPENDENCY_H_
#include "shaderGen/shaderDependency.h"
#endif

class MultiLine;
struct LangElement;
struct MaterialFeatureData;
class GFXShaderConstBuffer;
struct RenderPassData;
struct SceneData;
class SceneRenderState;
class GFXShader;
class GFXVertexFormat;


///
class ShaderFeatureConstHandles
{
public:

   virtual ~ShaderFeatureConstHandles() { }

   virtual void init( GFXShader *shader ) = 0;

   virtual void setConsts( SceneRenderState *state, 
                           const SceneData &sgData,
                           GFXShaderConstBuffer *buffer ) = 0;
};

//**************************************************************************
/*!
   The ShaderFeature class is the base class for every procedurally generated
   feature. Each feature the engine recognizes is part of the MaterialFeatureType 
   enum.  That structure is used to indicate which features are present in a shader
   to be generated.  This is useful as many ShaderFeatures will output different 
   code depending on what other features are going to be in the shader.

   Shaders are generated using the ShaderFeature interface, so all of the
   descendants interact pretty much the same way.

*/
//**************************************************************************


//**************************************************************************
// Shader Feature
//**************************************************************************
class ShaderFeature
{
public:

   // Bitfield which allows a shader feature to say which render targets it outputs
   // data to (could be more than one).
   enum OutputTarget
   {
      DefaultTarget =   1 << 0,
      RenderTarget1 =   1 << 1,
      RenderTarget2 =   1 << 2,
      RenderTarget3 =   1 << 3,
   };

protected:

   LangElement *output;

   /// The list of unique shader dependencies.
   Vector<const ShaderDependency *> mDependencies;

   ///
   S32 mProcessIndex;

public:

   // TODO: Make this protected and give it a proper API.
   const GFXVertexFormat *mVertexFormat;

   // TODO: Make this protected and give it a proper API.
   GFXVertexFormat *mInstancingFormat;

public:   

   //**************************************************************************
   /*!
      The Resources structure is used by ShaderFeature to indicate how many
      hardware "resources" it needs.  Resources are things such as how
      many textures it uses and how many texture registers it needs to pass
      information from the vertex to the pixel shader.

      The Resources data can change depending what hardware is available.  For
      instance, pixel 1.x level hardware may need more texture registers than
      pixel 2.0+ hardware because each texture register can only be used with
      its respective texture sampler.

      The data in Resources is used to determine how many features can be
      squeezed into a singe shader.  If a feature requires too many resources
      to fit into the current shader, it will be put into another pass.
   */
   //**************************************************************************
   struct Resources
   {
      U32 numTex;
      U32 numTexReg;

      Resources()
      {
         dMemset( this, 0, sizeof( Resources ) );
      }
   };


   //-----------------------------------------------------------------------
   // Base functions
   //-----------------------------------------------------------------------
   
   ShaderFeature()
      :  output( NULL ),
         mProcessIndex( 0 ),
         mInstancingFormat( NULL ),
         mVertexFormat( NULL )
   {
   }

   virtual ~ShaderFeature() {}

   /// returns output from a processed vertex or pixel shader
   LangElement* getOutput() const { return output; }
   
   ///
   void setProcessIndex( S32 index ) { mProcessIndex = index; }

   ///
   S32 getProcessIndex() const { return mProcessIndex; }

   //-----------------------------------------------------------------------
   // Virtual Functions
   //-----------------------------------------------------------------------
   
   /// Get the incoming base texture coords - useful for bumpmap and detail maps
   virtual Var* getVertTexCoord( const String &name ) = 0;

   /// Set up a texture space matrix - to pass into pixel shader
   virtual LangElement * setupTexSpaceMat(  Vector<ShaderComponent*> &componentList, 
      Var **texSpaceMat ) = 0;

   /// Expand and assign a normal map. This takes care of compressed normal maps as well.
   virtual LangElement * expandNormalMap( LangElement *sampleNormalOp, 
      LangElement *normalDecl, LangElement *normalVar, const MaterialFeatureData &fd ) = 0;

   /// Helper function for applying the color to shader output.
   ///
   /// @param elem         The rbg or rgba color to assign.
   ///
   /// @param blend        The type of blending to perform.
   ///
   /// @param lerpElem     The optional lerp parameter when doing a LerpAlpha blend, 
   ///                     if not set then the elem is used.
   ///
   virtual LangElement* assignColor(   LangElement *elem, 
                                       Material::BlendOp blend, 
                                       LangElement *lerpElem = NULL,
                                       ShaderFeature::OutputTarget outputTarget = ShaderFeature::DefaultTarget ) = 0;


   //-----------------------------------------------------------------------
   /*!
      Process vertex shader - This function is used by each feature to
      generate a list of LangElements that can be traversed and "printed"
      to generate the actual shader code.  The 'output' member is the head
      of that list.

      The componentList is used mostly for access to the "Connector"
      structure which is used to pass data from the vertex to the pixel
      shader.

      The MaterialFeatureData parameter is used to determine what other
      features are present for the shader being generated.
   */
   //-----------------------------------------------------------------------
   virtual void processVert( Vector<ShaderComponent*> &componentList,
                             const MaterialFeatureData &fd )
                             { output = NULL; }

   //-----------------------------------------------------------------------
   /*!
      Process pixel shader - This function is used by each feature to
      generate a list of LangElements that can be traversed and "printed"
      to generate the actual shader code.  The 'output' member is the head
      of that list.

      The componentList is used mostly for access to the "Connector"
      structure which is used to pass data from the vertex to the pixel
      shader.

      The MaterialFeatureData parameter is used to determine what other
      features are present for the shader being generated.
   */
   //-----------------------------------------------------------------------
   virtual void processPix( Vector<ShaderComponent*> &componentList, 
                            const MaterialFeatureData &fd ) 
                            { output = NULL; }

   /// Allows the feature to add macros to pixel shader compiles.
   virtual void processPixMacros( Vector<GFXShaderMacro> &macros, const MaterialFeatureData &fd  ) {};

   /// Allows the feature to add macros to vertex shader compiles.
   virtual void processVertMacros( Vector<GFXShaderMacro> &macros, const MaterialFeatureData &fd  ) {};

   /// Identifies what type of blending a feature uses.  This is used to
   /// group features with the same blend operation together in a multipass
   /// situation.
   virtual Material::BlendOp getBlendOp() { return Material::Add; }
   
   /// Returns the resource requirements of this feature based on what
   /// other features are present.  The "resources" are things such as
   /// texture units, and texture registers of which there can be
   /// very limited numbers.  The resources can vary depending on hardware
   /// and what other features are present.
   virtual Resources getResources( const MaterialFeatureData &fd );

   /// Fills texture related info in RenderPassData for this feature.  It
   /// takes into account the current pass (passData) as well as what other
   /// data is available to the material stage (stageDat).  
   ///
   /// For instance, ReflectCubeFeatHLSL would like to modulate its output 
   /// by the alpha channel of another texture.  If the current pass does
   /// not contain a diffuse or bump texture, but the Material does, then
   /// this function allows it to use one of those textures in the current
   /// pass.
   virtual void setTexData( Material::StageData &stageDat,
                            const MaterialFeatureData &fd,
                            RenderPassData &passData,
                            U32 &texIndex ){};
                            
   /// Returns the name of this feature.
   virtual String getName() = 0;

   /// Adds a dependency to this shader feature.
   virtual void addDependency( const ShaderDependency *depends );

   /// Gets the dependency list for this shader feature.
   virtual const Vector<const ShaderDependency *> &getDependencies() const { return mDependencies; }

   /// Returns the output variable name for this feature if it applies.
   virtual const char* getOutputVarName() const { return NULL; }

   /// Gets the render target this shader feature is assigning data to.
   virtual U32 getOutputTargets( const MaterialFeatureData &fd ) const { return DefaultTarget; }

   /// Returns the name of output targer var.
   const char* getOutputTargetVarName( OutputTarget target = DefaultTarget ) const;

   // Called from ProcessedShaderMaterial::determineFeatures to enable/disable features.
   virtual void determineFeature(   Material *material, 
                                    const GFXVertexFormat *vertexFormat,
                                    U32 stageNum,
                                    const FeatureType &type,
                                    const FeatureSet &features,
                                    MaterialFeatureData *outFeatureData ) { }

   //
   virtual ShaderFeatureConstHandles* createConstHandles( GFXShader *shader, SimObject *userObject ) { return NULL; }

   /// Called after processing the vertex and processing the pixel 
   /// to cleanup any temporary structures stored in the feature.
   virtual void reset() { output = NULL; mProcessIndex = 0; mInstancingFormat = NULL; mVertexFormat = NULL; }

   /// A simpler helper function which either finds
   /// the existing local var or creates one.
   static Var* findOrCreateLocal(   const char *name, 
                                    const char *type, 
                                    MultiLine *multi );
   // Set the instancing format
   void setInstancingFormat(GFXVertexFormat *format);
};

#endif // _SHADERFEATURE_H_
