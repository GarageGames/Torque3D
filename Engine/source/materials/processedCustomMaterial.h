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

#ifndef _MATERIALS_PROCESSEDCUSTOMMATERIAL_H_
#define _MATERIALS_PROCESSEDCUSTOMMATERIAL_H_

#ifndef _MATERIALS_PROCESSEDSHADERMATERIAL_H_
#include "materials/processedShaderMaterial.h"
#endif
#ifndef _CUSTOMMATERIALDEFINITION_H_
#include "materials/customMaterialDefinition.h"
#endif


///
class ProcessedCustomMaterial : public ProcessedShaderMaterial
{
   typedef ProcessedShaderMaterial Parent;
public:
   ProcessedCustomMaterial(Material &mat);
   ~ProcessedCustomMaterial();

   virtual bool setupPass(SceneRenderState *, const SceneData& sgData, U32 pass);
   virtual bool init( const FeatureSet &features, const GFXVertexFormat *vertexFormat, const MatFeaturesDelegate &featuresDelegate );   
   virtual void setTextureStages(SceneRenderState *, const SceneData &sgData, U32 pass );
   virtual MaterialParameters* allocMaterialParameters();

protected:

   virtual void _setStageData();
   virtual bool _hasCubemap(U32 pass);
   void _initPassStateBlock( RenderPassData *rpd, GFXStateBlockDesc &result );
   virtual void _initPassStateBlocks();

private:

   CustomMaterial* mCustomMaterial;

   /// The conditioner macros passed to the 
   /// shader on construction.
   Vector<GFXShaderMacro> mConditionerMacros;

   /// How many texture slots are we using.
   U32 mMaxTex;
   
   template <typename T>
   void setMaterialParameter(MaterialParameters* param, MaterialParameterHandle* handle,
      const String& value);
   void setMatrixParameter(MaterialParameters* param, 
      MaterialParameterHandle* handle, const String& value, GFXShaderConstType matrixType);
};

#endif // _MATERIALS_PROCESSEDCUSTOMMATERIAL_H_
