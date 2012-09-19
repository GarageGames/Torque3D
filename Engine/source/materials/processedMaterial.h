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

#ifndef _MATERIALS_PROCESSEDMATERIAL_H_
#define _MATERIALS_PROCESSEDMATERIAL_H_

#ifndef _MATERIALDEFINITION_H_
#include "materials/materialDefinition.h"
#endif
#ifndef _MATERIALFEATUREDATA_H_
#include "materials/materialFeatureData.h"
#endif
#ifndef _GFXSTATEBLOCK_H_
#include "gfx/gfxStateBlock.h"
#endif
#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif
#ifndef _MATSTATEHINT_H_
#include "materials/matStateHint.h"
#endif

class ShaderFeature;
class MaterialParameters;
class MaterialParameterHandle;
class SceneRenderState;
class GFXVertexBufferHandleBase;
class GFXPrimitiveBufferHandle;
class MatrixSet;


/// This contains the common data needed to render a pass.
struct RenderPassData
{
public:

   struct TexSlotT
   {
      /// This is the default type of texture which 
      /// is valid with most texture types.
      /// @see mTexType
      GFXTexHandle texObject;

      /// Only valid when the texture type is set 
      /// to Material::TexTarget.
      /// @see mTexType
     NamedTexTargetRef texTarget;

   } mTexSlot[Material::MAX_TEX_PER_PASS];

   U32 mTexType[Material::MAX_TEX_PER_PASS];

   /// The cubemap to use when the texture type is
   /// set to Material::Cube.
   /// @see mTexType
   GFXCubemap *mCubeMap;

   U32 mNumTex;

   U32 mNumTexReg;

   MaterialFeatureData mFeatureData;

   bool mGlow;

   Material::BlendOp mBlendOp;

   U32 mStageNum;

   /// State permutations, used to index into 
   /// the render states array.
   /// @see mRenderStates
   enum 
   {
      STATE_REFLECT = 1,
      STATE_TRANSLUCENT = 2,
      STATE_GLOW = 4,      
      STATE_WIREFRAME = 8,
      STATE_MAX = 16
   };

   ///
   GFXStateBlockRef mRenderStates[STATE_MAX];

   RenderPassData();

   virtual ~RenderPassData() { reset(); }

   virtual void reset();

   /// Creates and returns a unique description string.
   virtual String describeSelf() const;
};

/// This is an abstract base class which provides the external
/// interface all subclasses must implement. This interface
/// primarily consists of setting state.  Pass creation
/// is implementation specific, and internal, thus it is
/// not in this base class.
class ProcessedMaterial
{
public:
   ProcessedMaterial();
   virtual ~ProcessedMaterial();

   /// @name State setting functions
   ///
   /// @{

   ///
   virtual void addStateBlockDesc(const GFXStateBlockDesc& sb);

   ///
   virtual void updateStateBlocks() { _initRenderPassDataStateBlocks(); }

   /// Set the user defined shader macros.
   virtual void setShaderMacros( const Vector<GFXShaderMacro> &macros ) { mUserMacros = macros; }

   /// Sets the textures needed for rendering the current pass
   virtual void setTextureStages(SceneRenderState *, const SceneData &sgData, U32 pass ) = 0;

   /// Sets the transformation matrix, i.e. Model * View * Projection
   virtual void setTransforms(const MatrixSet &matrixSet, SceneRenderState *state, const U32 pass) = 0;
   
   /// Sets the scene info like lights for the given pass.
   virtual void setSceneInfo(SceneRenderState *, const SceneData& sgData, U32 pass) = 0;

   /// Sets the given vertex and primitive buffers so we can render geometry
   virtual void setBuffers(GFXVertexBufferHandleBase* vertBuffer, GFXPrimitiveBufferHandle* primBuffer); 
   
   /// @see BaseMatInstance::setUserObject
   virtual void setUserObject( SimObject *userObject ) { mUserObject = userObject; }

   /// 
   virtual bool stepInstance();

   /// @}

   /// Initializes us (eg. loads textures, creates passes, generates shaders)
   virtual bool init(   const FeatureSet& features, 
                        const GFXVertexFormat *vertexFormat,
                        const MatFeaturesDelegate &featuresDelegate ) = 0;

   /// Returns the state hint which can be used for 
   /// sorting and fast comparisions of the equality 
   /// of a material instance.
   virtual const MatStateHint& getStateHint() const { return mStateHint; }

   /// Sets up the given pass.  Returns true if the pass was set up, false if there was an error or if
   /// the specified pass is out of bounds.
   virtual bool setupPass(SceneRenderState *, const SceneData& sgData, U32 pass) = 0;

   // Material parameter methods
   virtual MaterialParameters* allocMaterialParameters() = 0;
   virtual MaterialParameters* getDefaultMaterialParameters() = 0;
   virtual void setMaterialParameters(MaterialParameters* param, S32 pass) { mCurrentParams = param; }; 
   virtual MaterialParameters* getMaterialParameters() { return mCurrentParams; }
   virtual MaterialParameterHandle* getMaterialParameterHandle(const String& name) = 0;

   /// Returns the pass data for the given pass.
   RenderPassData* getPass(U32 pass)
   {
      if(pass >= mPasses.size())
         return NULL;
      return mPasses[pass];
   }

   /// Returns the pass data for the given pass.
   const RenderPassData* getPass( U32 pass ) const { return mPasses[pass]; }

   /// Returns the number of stages we're rendering (not to be confused with the number of passes).
   virtual U32 getNumStages() = 0;

   /// Returns the number of passes we are rendering (not to be confused with the number of stages).
   U32 getNumPasses() const { return mPasses.size(); }

   /// Returns true if any pass glows
   bool hasGlow() const { return mHasGlow; }

   /// Gets the stage number for a pass
   U32 getStageFromPass(U32 pass) const
   {
      if(pass >= mPasses.size())
         return 0;
      return mPasses[pass]->mStageNum;
   }

   /// Returns the active features in use by this material.
   /// @see BaseMatInstance::getFeatures
   const FeatureSet& getFeatures() const { return mFeatures; }

   /// Dump shader info, or FF texture info?
   virtual void dumpMaterialInfo() { }

   /// Returns the source material.
   Material* getMaterial() const { return mMaterial; }

   /// Returns the texture used by a stage
   GFXTexHandle getStageTexture(U32 stage, const FeatureType &type)
   {
      return (stage < Material::MAX_STAGES) ? mStages[stage].getTex(type) : NULL;
   }

protected:

   /// Our passes.
   Vector<RenderPassData*> mPasses;

   /// The active features in use by this material.
   FeatureSet mFeatures;

   /// The material which we are processing.
   Material* mMaterial;

   MaterialParameters* mCurrentParams;

   /// Material::StageData is used here because the shader 
   /// generator throws a fit if it's passed anything else.
   Material::StageData mStages[Material::MAX_STAGES];

   /// If we've already loaded the stage data
   bool mHasSetStageData;

   /// If we glow
   bool mHasGlow;

   /// Number of stages (not to be confused with number of passes)
   U32 mMaxStages;

   /// The vertex format on which this material will render.
   const GFXVertexFormat *mVertexFormat;

   ///  Set by addStateBlockDesc, should be considered 
   /// when initPassStateBlock is called.
   GFXStateBlockDesc mUserDefined;   

   /// The user defined macros to pass to the 
   /// shader initialization.
   Vector<GFXShaderMacro> mUserMacros;

   /// The user defined object to pass to ShaderFeature::createConstHandles.
   SimObject *mUserObject;

   /// The state hint used for material sorting 
   /// and quick equality comparision.
   MatStateHint mStateHint;

   /// Loads all the textures for all of the stages in the Material
   virtual void _setStageData();

   /// Sets the blend state for rendering   
   void _setBlendState(Material::BlendOp blendOp, GFXStateBlockDesc& desc );

   /// Returns the path the material will attempt to load for a given texture filename.
   String _getTexturePath(const String& filename);

   /// Loads the texture located at _getTexturePath(filename) and gives it the specified profile
   GFXTexHandle _createTexture( const char *filename, GFXTextureProfile *profile );

   /// @name State blocks
   ///
   /// @{

   /// Creates the default state block templates, used by initStateBlocks.
   virtual void _initStateBlockTemplates(GFXStateBlockDesc& stateTranslucent, GFXStateBlockDesc& stateGlow, GFXStateBlockDesc& stateReflect);

   /// Does the base render state block setting, normally per pass.
   virtual void _initPassStateBlock( RenderPassData *rpd, GFXStateBlockDesc& result);

   /// Creates the default state blocks for a list of render states.
   virtual void _initRenderStateStateBlocks( RenderPassData *rpd );

   /// Creates the default state blocks for each RenderPassData item.
   virtual void _initRenderPassDataStateBlocks();

   /// This returns the index into the renderState array based on the sgData passed in.
   virtual U32 _getRenderStateIndex(   const SceneRenderState *state, 
                                       const SceneData &sgData );

   /// Activates the correct mPasses[currPass].renderState based on scene graph info
   virtual void _setRenderState( const SceneRenderState *state, 
                                 const SceneData &sgData, 
                                 U32 pass );
   /// @
};

#endif // _MATERIALS_PROCESSEDMATERIAL_H_
