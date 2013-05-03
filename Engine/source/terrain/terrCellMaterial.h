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

#ifndef _TERRCELLMATERIAL_H_
#define _TERRCELLMATERIAL_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _GFXSHADER_H_
#include "gfx/gfxShader.h"
#endif
#ifndef _GFXSTATEBLOCK_H_
#include "gfx/gfxStateBlock.h"
#endif


class SceneRenderState;
struct SceneData;
class TerrainMaterial;
class TerrainBlock;
class BaseMatInstance;


/// This is a complex material which holds one or more
/// optimized shaders for rendering a single cell.
class TerrainCellMaterial
{
protected:

   class MaterialInfo
   {
   public:

      MaterialInfo()
      {
      }

      ~MaterialInfo() 
      {
      }

      TerrainMaterial *mat;
      U32 layerId;

      GFXShaderConstHandle *detailTexConst;
      GFXTexHandle detailTex;

      GFXShaderConstHandle *macroTexConst;
      GFXTexHandle macroTex;

      GFXShaderConstHandle *normalTexConst;
      GFXTexHandle normalTex;

      GFXShaderConstHandle *detailInfoVConst;
      GFXShaderConstHandle *detailInfoPConst;

	  GFXShaderConstHandle *macroInfoVConst;
      GFXShaderConstHandle *macroInfoPConst;
   };

   class Pass
   {
   public:

      Pass() 
         :  shader( NULL )                     
      {
      }

      ~Pass() 
      {
         for ( U32 i=0; i < materials.size(); i++ )
            delete materials[i];
      }

      Vector<MaterialInfo*> materials;

      ///
      GFXShader *shader;

      GFXShaderConstBufferRef consts;

      GFXStateBlockRef stateBlock;
      GFXStateBlockRef wireframeStateBlock;

      GFXShaderConstHandle *modelViewProjConst;
      GFXShaderConstHandle *worldViewOnly;
      GFXShaderConstHandle *viewToObj;

      GFXShaderConstHandle *eyePosWorldConst;
      GFXShaderConstHandle *eyePosConst;

      GFXShaderConstHandle *objTransConst;
      GFXShaderConstHandle *worldToObjConst;
      GFXShaderConstHandle *vEyeConst;

      GFXShaderConstHandle *layerSizeConst;
      GFXShaderConstHandle *lightParamsConst;
      GFXShaderConstHandle *lightInfoBufferConst;

      GFXShaderConstHandle *baseTexMapConst;
      GFXShaderConstHandle *layerTexConst;

      GFXShaderConstHandle *lightMapTexConst;

      GFXShaderConstHandle *squareSize;
      GFXShaderConstHandle *oneOverTerrainSize;

      GFXShaderConstHandle *fogDataConst;
      GFXShaderConstHandle *fogColorConst;
   };

   TerrainBlock *mTerrain;

   U64 mMaterials;

   Vector<Pass> mPasses;

   U32 mCurrPass;

   GFXTexHandle mBaseMapTexture;

   GFXTexHandle mLayerMapTexture;

   NamedTexTargetRef mLightInfoTarget;

   /// The prepass material for this material.
   TerrainCellMaterial *mPrePassMat;

   /// The reflection material for this material.
   TerrainCellMaterial *mReflectMat;

   /// A vector of all terrain cell materials loaded in the system.
   static Vector<TerrainCellMaterial*> smAllMaterials;

   bool _createPass( Vector<MaterialInfo*> *materials, 
                     Pass *pass, 
                     bool firstPass,
                     bool prePassMat,
                     bool reflectMat,
                     bool baseOnly );

   void _updateMaterialConsts( Pass *pass );

public:
   
   TerrainCellMaterial();
   ~TerrainCellMaterial();

   void init(  TerrainBlock *block, 
               U64 activeMaterials,
               bool prePassMat = false,
               bool reflectMat = false,
               bool baseOnly = false );

   /// Returns a prepass material from this material.
   TerrainCellMaterial* getPrePassMat();

   /// Returns the reflection material from this material.
   TerrainCellMaterial* getReflectMat();

   void setTransformAndEye(   const MatrixF &modelXfm, 
                              const MatrixF &viewXfm,
                              const MatrixF &projectXfm,
                              F32 farPlane );

   ///
   bool setupPass(   const SceneRenderState *state,
                     const SceneData &sceneData );

   ///
   static BaseMatInstance* getShadowMat();

   /// 
   static void _updateDefaultAnisotropy();
};

#endif // _TERRCELLMATERIAL_H_

