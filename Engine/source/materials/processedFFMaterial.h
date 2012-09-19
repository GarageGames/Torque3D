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
#ifndef _MATERIALS_PROCESSEDFFMATERIAL_H_
#define _MATERIALS_PROCESSEDFFMATERIAL_H_

#ifndef _MATERIALS_PROCESSEDMATERIAL_H_
#include "materials/processedMaterial.h"
#endif

class LightInfo;
struct GFXShaderConstDesc;


/// Fixed function rendering.  Does not load or use shaders.  Does not rely on GFXMaterialFeatureData.
/// Tries very hard to not rely on anything possibly related to shaders. 
///
/// @note Does not always succeed.
class ProcessedFFMaterial : public ProcessedMaterial
{
   typedef ProcessedMaterial Parent;
public:
   ProcessedFFMaterial();
   ProcessedFFMaterial(Material &mat, const bool isLightingMaterial = false);
   ~ProcessedFFMaterial();
   /// @name Render state setting
   ///
   /// @{

   /// Sets necessary textures and texture ops for rendering
   virtual void setTextureStages(SceneRenderState *, const SceneData &sgData, U32 pass );

   virtual MaterialParameters* allocMaterialParameters();   
   virtual MaterialParameterHandle* getMaterialParameterHandle(const String& name);
   virtual MaterialParameters* getDefaultMaterialParameters();      
   
   virtual void setTransforms(const MatrixSet &matrixSet, SceneRenderState *state, const U32 pass);

   virtual void setSceneInfo(SceneRenderState *, const SceneData& sgData, U32 pass);

   /// @}

   virtual bool init(   const FeatureSet &features, 
                        const GFXVertexFormat *vertexFormat,
                        const MatFeaturesDelegate &featuresDelegate );

   /// Sets up the given pass
   ///
   /// @returns false if the pass could not be set up
   virtual bool setupPass(SceneRenderState *, const SceneData& sgData, U32 pass);

   /// Returns the number of stages we're using (not to be confused with the number of passes)
   virtual U32 getNumStages();

protected:

   MaterialParameterHandle* mDefaultHandle;
   MaterialParameters* mDefaultParameters;

   struct FixedFuncFeatureData
   {
      enum
      {
         DiffuseMap,
         LightMap,
         ToneMap,
         NumFeatures
      };
      bool features[NumFeatures];
   };

   bool mIsLightingMaterial;

   Vector<GFXShaderConstDesc> mParamDesc;   

   /// @name Internal functions
   ///
   /// @{

   /// Adds a pass for the given stage
   virtual void _addPass(U32 stageNum, FixedFuncFeatureData& featData);

   /// Chooses a blend op for the pass during pass creation
   virtual void _setPassBlendOp();

   /// Creates all necessary passes for the given stage
   void _createPasses( U32 stageNum, const FeatureSet &features );

   /// Determine what features we need
   void _determineFeatures(   U32 stageNum, 
                              FixedFuncFeatureData &featData, 
                              const FeatureSet &features);

   /// Sets light info for the first light
   virtual void _setPrimaryLightInfo(const MatrixF &objTrans, LightInfo* light, U32 pass);

   /// Sets light info for the second light
   virtual void _setSecondaryLightInfo(const MatrixF &objTrans, LightInfo* light);

   /// Does the base render state block setting, normally per pass
   virtual void _initPassStateBlock( RenderPassData *rpd, GFXStateBlockDesc &result );
   /// @}

   void _construct();
};

#endif
