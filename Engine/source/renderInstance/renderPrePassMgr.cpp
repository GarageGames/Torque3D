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
#include "renderInstance/renderPrePassMgr.h"

#include "gfx/gfxTransformSaver.h"
#include "materials/sceneData.h"
#include "materials/materialManager.h"
#include "materials/materialFeatureTypes.h"
#include "core/util/safeDelete.h"
#include "shaderGen/featureMgr.h"
#include "shaderGen/HLSL/depthHLSL.h"
#include "shaderGen/GLSL/depthGLSL.h"
#include "shaderGen/conditionerFeature.h"
#include "shaderGen/shaderGenVars.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "gfx/gfxDebugEvent.h"
#include "materials/customMaterialDefinition.h"
#include "lighting/advanced/advancedLightManager.h"
#include "lighting/advanced/advancedLightBinManager.h"
#include "terrain/terrCell.h"
#include "renderInstance/renderTerrainMgr.h"
#include "terrain/terrCellMaterial.h"
#include "math/mathUtils.h"
#include "math/util/matrixSet.h"

const MatInstanceHookType PrePassMatInstanceHook::Type( "PrePass" );
const String RenderPrePassMgr::BufferName("prepass");
const RenderInstType RenderPrePassMgr::RIT_PrePass("PrePass");

IMPLEMENT_CONOBJECT(RenderPrePassMgr);

ConsoleDocClass( RenderPrePassMgr, 
   "@brief The render bin which performs a z+normals prepass used in Advanced Lighting.\n\n"
   "This render bin is used in Advanced Lighting to gather all opaque mesh render instances "
   "and render them to the g-buffer for use in lighting the scene and doing effects.\n\n"
   "PostEffect and other shaders can access the output of this bin by using the #prepass "
   "texture target name.  See the edge anti-aliasing post effect for an example.\n\n"
   "@see game/core/scripts/client/postFx/edgeAA.cs\n"
   "@ingroup RenderBin\n" );


RenderPrePassMgr::RenderSignal& RenderPrePassMgr::getRenderSignal()
{
   static RenderSignal theSignal;
   return theSignal;
}


RenderPrePassMgr::RenderPrePassMgr( bool gatherDepth,
                                    GFXFormat format )
   :  Parent(  RIT_PrePass,
               0.01f,
               0.01f,
               format,
               Point2I( Parent::DefaultTargetSize, Parent::DefaultTargetSize),
               gatherDepth ? Parent::DefaultTargetChainLength : 0 ),
      mPrePassMatInstance( NULL )
{
   notifyType( RenderPassManager::RIT_Decal );
   notifyType( RenderPassManager::RIT_Mesh );
   notifyType( RenderPassManager::RIT_Terrain );
   notifyType( RenderPassManager::RIT_Object );

   // We want a full-resolution buffer
   mTargetSizeType = RenderTexTargetBinManager::WindowSize;

   if(getTargetChainLength() > 0)
      GFXShader::addGlobalMacro( "TORQUE_LINEAR_DEPTH" );

   mNamedTarget.registerWithName( BufferName );

   _registerFeatures();
}

RenderPrePassMgr::~RenderPrePassMgr()
{
   GFXShader::removeGlobalMacro( "TORQUE_LINEAR_DEPTH" );

   _unregisterFeatures();
   SAFE_DELETE( mPrePassMatInstance );
}

void RenderPrePassMgr::_registerFeatures()
{
   ConditionerFeature *cond = new LinearEyeDepthConditioner( getTargetFormat() );
   FEATUREMGR->registerFeature( MFT_PrePassConditioner, cond );
   mNamedTarget.setConditioner( cond );
}

void RenderPrePassMgr::_unregisterFeatures()
{
   mNamedTarget.setConditioner( NULL );
   FEATUREMGR->unregisterFeature(MFT_PrePassConditioner);
}

bool RenderPrePassMgr::setTargetSize(const Point2I &newTargetSize)
{
   bool ret = Parent::setTargetSize( newTargetSize );
   mNamedTarget.setViewport( GFX->getViewport() );
   return ret;
}

bool RenderPrePassMgr::_updateTargets()
{
   PROFILE_SCOPE(RenderPrePassMgr_updateTargets);

   bool ret = Parent::_updateTargets();

   // check for an output conditioner, and update it's format
   ConditionerFeature *outputConditioner = dynamic_cast<ConditionerFeature *>(FEATUREMGR->getByType(MFT_PrePassConditioner));
   if( outputConditioner && outputConditioner->setBufferFormat(mTargetFormat) )
   {
      // reload materials, the conditioner needs to alter the generated shaders
   }

   // Attach the light info buffer as a second render target, if there is
   // lightmapped geometry in the scene.
   AdvancedLightBinManager *lightBin;
   if (  Sim::findObject( "AL_LightBinMgr", lightBin ) &&
         lightBin->MRTLightmapsDuringPrePass() &&
         lightBin->isProperlyAdded() )
   {
      // Update the size of the light bin target here. This will call _updateTargets
      // on the light bin
      ret &= lightBin->setTargetSize( mTargetSize );
      if ( ret )
      {
         // Sanity check
         AssertFatal(lightBin->getTargetChainLength() == mTargetChainLength, "Target chain length mismatch");

         // Attach light info buffer to Color1 for each target in the chain
         for ( U32 i = 0; i < mTargetChainLength; i++ )
         {
            GFXTexHandle lightInfoTex = lightBin->getTargetTexture(0, i);
            mTargetChain[i]->attachTexture(GFXTextureTarget::Color1, lightInfoTex);
         }
      }
   }

   return ret;
}

void RenderPrePassMgr::_createPrePassMaterial()
{
   SAFE_DELETE(mPrePassMatInstance);

   const GFXVertexFormat *vertexFormat = getGFXVertexFormat<GFXVertexPNTTB>();

   MatInstance* prepassMat = static_cast<MatInstance*>(MATMGR->createMatInstance("AL_DefaultPrePassMaterial", vertexFormat));
   AssertFatal( prepassMat, "TODO: Handle this better." );
   mPrePassMatInstance = new PrePassMatInstance(prepassMat, this);
   mPrePassMatInstance->init( MATMGR->getDefaultFeatures(), vertexFormat);
   delete prepassMat;
}

void RenderPrePassMgr::setPrePassMaterial( PrePassMatInstance *mat )
{
   SAFE_DELETE(mPrePassMatInstance);
   mPrePassMatInstance = mat;
}

void RenderPrePassMgr::addElement( RenderInst *inst )
{
   PROFILE_SCOPE( RenderPrePassMgr_addElement )

   // Skip out if this bin is disabled.
   if (  gClientSceneGraph->getCurrentRenderState() &&
         gClientSceneGraph->getCurrentRenderState()->disableAdvancedLightingBins() )
      return;

   // First what type of render instance is it?
   const bool isDecalMeshInst = inst->type == RenderPassManager::RIT_Decal;

   const bool isMeshInst = inst->type == RenderPassManager::RIT_Mesh;

   const bool isTerrainInst = inst->type == RenderPassManager::RIT_Terrain;

   // Get the material if its a mesh.
   BaseMatInstance* matInst = NULL;
   if ( isMeshInst || isDecalMeshInst )
      matInst = static_cast<MeshRenderInst*>(inst)->matInst;

   // Skip decals if they don't have normal maps.
   if ( isDecalMeshInst && !matInst->hasNormalMap() )
      return;

   // If its a custom material and it refracts... skip it.
   if (  matInst && 
         matInst->isCustomMaterial() &&
         static_cast<CustomMaterial*>( matInst->getMaterial() )->mRefract )
      return;

   // Make sure we got a prepass material.
   if ( matInst )
   {
      matInst = getPrePassMaterial( matInst );
      if ( !matInst || !matInst->isValid() )
         return;
   }

   // We're gonna add it to the bin... get the right element list.
   Vector< MainSortElem > *elementList;
   if ( isMeshInst || isDecalMeshInst )
      elementList = &mElementList;
   else if ( isTerrainInst )
      elementList = &mTerrainElementList;
   else
      elementList = &mObjectElementList;

   elementList->increment();
   MainSortElem &elem = elementList->last();
   elem.inst = inst;

   // Store the original key... we might need it.
   U32 originalKey = elem.key;

   // Sort front-to-back first to get the most fillrate savings.
   const F32 invSortDistSq = F32_MAX - inst->sortDistSq;
   elem.key = *((U32*)&invSortDistSq);

   // Next sort by pre-pass material if its a mesh... use the original sort key.
   if ( isMeshInst )
      elem.key2 = matInst->getStateHint();
   else
      elem.key2 = originalKey;
}

void RenderPrePassMgr::sort()
{
   PROFILE_SCOPE( RenderPrePassMgr_sort );
   Parent::sort();
   dQsort( mTerrainElementList.address(), mTerrainElementList.size(), sizeof(MainSortElem), cmpKeyFunc);
   dQsort( mObjectElementList.address(), mObjectElementList.size(), sizeof(MainSortElem), cmpKeyFunc);
}

void RenderPrePassMgr::clear()
{
   Parent::clear();
   mTerrainElementList.clear();
   mObjectElementList.clear();
}

void RenderPrePassMgr::render( SceneRenderState *state )
{
   PROFILE_SCOPE(RenderPrePassMgr_render);

   // Take a look at the SceneRenderState and see if we should skip drawing the pre-pass
   if ( state->disableAdvancedLightingBins() )
      return;

   // NOTE: We don't early out here when the element list is
   // zero because we need the prepass to be cleared.

   // Automagically save & restore our viewport and transforms.
   GFXTransformSaver saver;

   GFXDEBUGEVENT_SCOPE( RenderPrePassMgr_Render, ColorI::RED );

   // Tell the superclass we're about to render
   const bool isRenderingToTarget = _onPreRender(state);

   // Clear all the buffers to white so that the
   // default depth is to the far plane.
   GFX->clear( GFXClearTarget | GFXClearZBuffer | GFXClearStencil, ColorI::WHITE, 1.0f, 0);

   // Restore transforms
   MatrixSet &matrixSet = getRenderPass()->getMatrixSet();
   matrixSet.restoreSceneViewProjection();
   const MatrixF worldViewXfm = GFX->getWorldMatrix();

   // Setup the default prepass material for object instances.
   if ( !mPrePassMatInstance )
      _createPrePassMaterial();
   if ( mPrePassMatInstance )
   {
      matrixSet.setWorld(MatrixF::Identity);
      mPrePassMatInstance->setTransforms(matrixSet, state);
   }

   // Signal start of pre-pass
   getRenderSignal().trigger( state, this, true );
   
   // First do a loop and render all the terrain... these are 
   // usually the big blockers in a scene and will save us fillrate
   // on the smaller meshes and objects.

   // The terrain doesn't need any scene graph data
   // in the the prepass... so just clear it.
   SceneData sgData;
   sgData.init( state, SceneData::PrePassBin );

   Vector< MainSortElem >::const_iterator itr = mTerrainElementList.begin();
   for ( ; itr != mTerrainElementList.end(); itr++ )
   {
      TerrainRenderInst *ri = static_cast<TerrainRenderInst*>( itr->inst );

      TerrainCellMaterial *mat = ri->cellMat->getPrePassMat();

      GFX->setPrimitiveBuffer( ri->primBuff );
      GFX->setVertexBuffer( ri->vertBuff );

      mat->setTransformAndEye(   *ri->objectToWorldXfm,
                                 worldViewXfm,
                                 GFX->getProjectionMatrix(),
                                 state->getFarPlane() );

      while ( mat->setupPass( state, sgData ) )
         GFX->drawPrimitive( ri->prim );
   }


   // Next render all the meshes.
   itr = mElementList.begin();
   for ( ; itr != mElementList.end(); )
   {
      MeshRenderInst *ri = static_cast<MeshRenderInst*>( itr->inst );

      // Get the prepass material.
      BaseMatInstance *mat = getPrePassMaterial( ri->matInst );

      // Set up SG data proper like and flag it 
      // as a pre-pass render
      setupSGData( ri, sgData );

      Vector< MainSortElem >::const_iterator meshItr, endOfBatchItr = itr;

      while ( mat->setupPass( state, sgData ) )
      {
         meshItr = itr;
         for ( ; meshItr != mElementList.end(); meshItr++ )
         {
            MeshRenderInst *passRI = static_cast<MeshRenderInst*>( meshItr->inst );

            // Check to see if we need to break this batch.
            //
            // NOTE: We're comparing the non-prepass materials 
            // here so we don't incur the cost of looking up the 
            // prepass hook on each inst.
            //
            if ( newPassNeeded( ri, passRI ) )
               break;

            // Set up SG data for this instance.
            setupSGData( passRI, sgData );

            matrixSet.setWorld(*passRI->objectToWorld);
            matrixSet.setView(*passRI->worldToCamera);
            matrixSet.setProjection(*passRI->projection);

            mat->setSceneInfo(state, sgData);
            mat->setTransforms(matrixSet, state);

            // If we're instanced then don't render yet.
            if ( mat->isInstanced() )
            {
               // Let the material increment the instance buffer, but
               // break the batch if it runs out of room for more.
               if ( !mat->stepInstance() )
               {
                  meshItr++;
                  break;
               }

               continue;
            }

            // Setup the vertex and index buffers.
            mat->setBuffers( passRI->vertBuff, passRI->primBuff );

            // Render this sucker.
            if ( passRI->prim )
               GFX->drawPrimitive( *passRI->prim );
            else
               GFX->drawPrimitive( passRI->primBuffIndex );
         }

         // Draw the instanced batch.
         if ( mat->isInstanced() )
         {
            // Sets the buffers including the instancing stream.
            mat->setBuffers( ri->vertBuff, ri->primBuff );

            if ( ri->prim )
               GFX->drawPrimitive( *ri->prim );
            else
               GFX->drawPrimitive( ri->primBuffIndex );
         }

         endOfBatchItr = meshItr;

      } // while( mat->setupPass(state, sgData) )

      // Force the increment if none happened, otherwise go to end of batch.
      itr = ( itr == endOfBatchItr ) ? itr + 1 : endOfBatchItr;
   }

   // The final loop is for object render instances.
   itr = mObjectElementList.begin();
   for ( ; itr != mObjectElementList.end(); itr++ )
   {
      ObjectRenderInst *ri = static_cast<ObjectRenderInst*>( itr->inst );
      if ( ri->renderDelegate )
         ri->renderDelegate( ri, state, mPrePassMatInstance );
   }

   // Signal end of pre-pass
   getRenderSignal().trigger( state, this, false );

   if(isRenderingToTarget)
      _onPostRender();
}

const GFXStateBlockDesc & RenderPrePassMgr::getOpaqueStenciWriteDesc( bool lightmappedGeometry /*= true*/ )
{
   static bool sbInit = false;
   static GFXStateBlockDesc sOpaqueStaticLitStencilWriteDesc;
   static GFXStateBlockDesc sOpaqueDynamicLitStencilWriteDesc;

   if(!sbInit)
   {
      sbInit = true;

      // Build the static opaque stencil write/test state block descriptions
      sOpaqueStaticLitStencilWriteDesc.stencilDefined = true;
      sOpaqueStaticLitStencilWriteDesc.stencilEnable = true;
      sOpaqueStaticLitStencilWriteDesc.stencilWriteMask = 0x03;
      sOpaqueStaticLitStencilWriteDesc.stencilMask = 0x03;
      sOpaqueStaticLitStencilWriteDesc.stencilRef = RenderPrePassMgr::OpaqueStaticLitMask;
      sOpaqueStaticLitStencilWriteDesc.stencilPassOp = GFXStencilOpReplace;
      sOpaqueStaticLitStencilWriteDesc.stencilFailOp = GFXStencilOpKeep;
      sOpaqueStaticLitStencilWriteDesc.stencilZFailOp = GFXStencilOpKeep;
      sOpaqueStaticLitStencilWriteDesc.stencilFunc = GFXCmpAlways;

      // Same only dynamic
      sOpaqueDynamicLitStencilWriteDesc = sOpaqueStaticLitStencilWriteDesc;
      sOpaqueDynamicLitStencilWriteDesc.stencilRef = RenderPrePassMgr::OpaqueDynamicLitMask;
   }

   return (lightmappedGeometry ? sOpaqueStaticLitStencilWriteDesc : sOpaqueDynamicLitStencilWriteDesc);
}

const GFXStateBlockDesc & RenderPrePassMgr::getOpaqueStencilTestDesc()
{
   static bool sbInit = false;
   static GFXStateBlockDesc sOpaqueStencilTestDesc;

   if(!sbInit)
   {
      // Build opaque test
      sbInit = true;
      sOpaqueStencilTestDesc.stencilDefined = true;
      sOpaqueStencilTestDesc.stencilEnable = true;
      sOpaqueStencilTestDesc.stencilWriteMask = 0xFE;
      sOpaqueStencilTestDesc.stencilMask = 0x03;
      sOpaqueStencilTestDesc.stencilRef = 0;
      sOpaqueStencilTestDesc.stencilPassOp = GFXStencilOpKeep;
      sOpaqueStencilTestDesc.stencilFailOp = GFXStencilOpKeep;
      sOpaqueStencilTestDesc.stencilZFailOp = GFXStencilOpKeep;
      sOpaqueStencilTestDesc.stencilFunc = GFXCmpLess;
   }

   return sOpaqueStencilTestDesc;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


ProcessedPrePassMaterial::ProcessedPrePassMaterial( Material& mat, const RenderPrePassMgr *prePassMgr )
: Parent(mat), mPrePassMgr(prePassMgr)
{

}

void ProcessedPrePassMaterial::_determineFeatures( U32 stageNum,
                                                   MaterialFeatureData &fd,
                                                   const FeatureSet &features )
{
   Parent::_determineFeatures( stageNum, fd, features );

   // Find this for use down below...
   bool bEnableMRTLightmap = false;
   AdvancedLightBinManager *lightBin;
   if ( Sim::findObject( "AL_LightBinMgr", lightBin ) )
      bEnableMRTLightmap = lightBin->MRTLightmapsDuringPrePass();

   // If this material has a lightmap or tonemap (texture or baked vertex color),
   // it must be static. Otherwise it is dynamic.
   mIsLightmappedGeometry = ( fd.features.hasFeature( MFT_ToneMap ) ||
                              fd.features.hasFeature( MFT_LightMap ) ||
                              fd.features.hasFeature( MFT_VertLit ) ||
                              ( bEnableMRTLightmap && fd.features.hasFeature( MFT_IsTranslucent ) ||
                                                      fd.features.hasFeature( MFT_ForwardShading ) ||
                                                      fd.features.hasFeature( MFT_IsTranslucentZWrite ) ) );

   // Integrate proper opaque stencil write state
   mUserDefined.addDesc( mPrePassMgr->getOpaqueStenciWriteDesc( mIsLightmappedGeometry ) );

   FeatureSet newFeatures;

   // These are always on for prepass.
   newFeatures.addFeature( MFT_EyeSpaceDepthOut );
   newFeatures.addFeature( MFT_PrePassConditioner );

#ifndef TORQUE_DEDICATED

   for ( U32 i=0; i < fd.features.getCount(); i++ )
   {
      const FeatureType &type = fd.features.getAt( i );

      // Turn on the diffuse texture only if we
      // have alpha test.
      if ( type == MFT_AlphaTest )
      {
         newFeatures.addFeature( MFT_AlphaTest );
         newFeatures.addFeature( MFT_DiffuseMap );
      }

      else if ( type == MFT_IsTranslucentZWrite )
      {
         newFeatures.addFeature( MFT_IsTranslucentZWrite );
         newFeatures.addFeature( MFT_DiffuseMap );
      }

      // Always allow these.
      else if (   type == MFT_IsDXTnm ||
                  type == MFT_TexAnim ||
                  type == MFT_NormalMap ||
                  type == MFT_DetailNormalMap ||
                  type == MFT_AlphaTest ||
                  type == MFT_Parallax ||
                  type == MFT_InterlacedPrePass ||
                  type == MFT_Visibility ||
                  type == MFT_UseInstancing ||
                  type == MFT_DiffuseVertColor )
         newFeatures.addFeature( type );

      // Add any transform features.
      else if (   type.getGroup() == MFG_PreTransform ||
                  type.getGroup() == MFG_Transform ||
                  type.getGroup() == MFG_PostTransform )
         newFeatures.addFeature( type );
   }

   // If there is lightmapped geometry support, add the MRT light buffer features
   if(bEnableMRTLightmap)
   {
      // If this material has a lightmap, pass it through, and flag it to
      // send it's output to RenderTarget1
      if( fd.features.hasFeature( MFT_ToneMap ) )
      {
         newFeatures.addFeature( MFT_ToneMap );
         newFeatures.addFeature( MFT_LightbufferMRT );
      }
      else if( fd.features.hasFeature( MFT_LightMap ) )
      {
         newFeatures.addFeature( MFT_LightMap );
         newFeatures.addFeature( MFT_LightbufferMRT );
      }
      else if( fd.features.hasFeature( MFT_VertLit ) )
      {
         // Flag un-tone-map if necesasary
         if( fd.features.hasFeature( MFT_DiffuseMap ) )
            newFeatures.addFeature( MFT_VertLitTone );

         newFeatures.addFeature( MFT_VertLit );
         newFeatures.addFeature( MFT_LightbufferMRT );
      }
      else
      {
         // If this object isn't lightmapped, add a zero-output feature to it
         newFeatures.addFeature( MFT_RenderTarget1_Zero );
      }
   }

#endif

   // Set the new features.
   fd.features = newFeatures;
}

U32 ProcessedPrePassMaterial::getNumStages()
{
   // Return 1 stage so this material gets processed for sure
   return 1;
}

void ProcessedPrePassMaterial::addStateBlockDesc(const GFXStateBlockDesc& desc)
{
   GFXStateBlockDesc prePassStateBlock = desc;

   // Adjust color writes if this is a pure z-fill pass
   const bool pixelOutEnabled = mPrePassMgr->getTargetChainLength() > 0;
   if ( !pixelOutEnabled )
   {
      prePassStateBlock.colorWriteDefined = true;
      prePassStateBlock.colorWriteRed = pixelOutEnabled;
      prePassStateBlock.colorWriteGreen = pixelOutEnabled;
      prePassStateBlock.colorWriteBlue = pixelOutEnabled;
      prePassStateBlock.colorWriteAlpha = pixelOutEnabled;
   }

   // Never allow the alpha test state when rendering
   // the prepass as we use the alpha channel for the
   // depth information... MFT_AlphaTest will handle it.
   prePassStateBlock.alphaDefined = true;
   prePassStateBlock.alphaTestEnable = false;

   // If we're translucent then we're doing prepass blending
   // which never writes to the depth channels.
   const bool isTranslucent = getMaterial()->isTranslucent();
   if ( isTranslucent )
   {
      prePassStateBlock.setBlend( true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha );
      prePassStateBlock.setColorWrites( true, true, false, false );
   }

   // Enable z reads, but only enable zwrites if we're not translucent.
   prePassStateBlock.setZReadWrite( true, isTranslucent ? false : true );

   // Pass to parent
   Parent::addStateBlockDesc(prePassStateBlock);
}

PrePassMatInstance::PrePassMatInstance(MatInstance* root, const RenderPrePassMgr *prePassMgr)
: Parent(*root->getMaterial()), mPrePassMgr(prePassMgr)
{
   mFeatureList = root->getRequestedFeatures();
   mVertexFormat = root->getVertexFormat();
   mUserObject = root->getUserObject();
}

PrePassMatInstance::~PrePassMatInstance()
{
}

ProcessedMaterial* PrePassMatInstance::getShaderMaterial()
{
   return new ProcessedPrePassMaterial(*mMaterial, mPrePassMgr);
}

bool PrePassMatInstance::init( const FeatureSet &features,
                               const GFXVertexFormat *vertexFormat )
{
   return Parent::init( features, vertexFormat );
}

PrePassMatInstanceHook::PrePassMatInstanceHook( MatInstance *baseMatInst,
                                                const RenderPrePassMgr *prePassMgr )
   : mHookedPrePassMatInst(NULL), mPrePassManager(prePassMgr)
{
   // If the material is a custom material then
   // hope that using DefaultPrePassMaterial gives
   // them a good prepass.
   if ( baseMatInst->isCustomMaterial() )
   {
      MatInstance* dummyInst = static_cast<MatInstance*>( MATMGR->createMatInstance( "AL_DefaultPrePassMaterial", baseMatInst->getVertexFormat() ) );

      mHookedPrePassMatInst = new PrePassMatInstance( dummyInst, prePassMgr );
      mHookedPrePassMatInst->init( dummyInst->getRequestedFeatures(), baseMatInst->getVertexFormat());

      delete dummyInst;
      return;
   }

   // Create the prepass material instance.
   mHookedPrePassMatInst = new PrePassMatInstance(baseMatInst, prePassMgr);
   mHookedPrePassMatInst->getFeaturesDelegate() = baseMatInst->getFeaturesDelegate();

   // Get the features, but remove the instancing feature if the
   // original material didn't end up using it.
   FeatureSet features = baseMatInst->getRequestedFeatures();
   if ( !baseMatInst->isInstanced() )
      features.removeFeature( MFT_UseInstancing );

   // Initialize the material.
   mHookedPrePassMatInst->init(features, baseMatInst->getVertexFormat());
}

PrePassMatInstanceHook::~PrePassMatInstanceHook()
{
   SAFE_DELETE(mHookedPrePassMatInst);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void LinearEyeDepthConditioner::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // find depth
   ShaderFeature *depthFeat = FEATUREMGR->getByType( MFT_EyeSpaceDepthOut );
   AssertFatal( depthFeat != NULL, "No eye space depth feature found!" );

   Var *depth = (Var*) LangElement::find(depthFeat->getOutputVarName());
   AssertFatal( depth, "Something went bad with ShaderGen. The depth should be already generated by the EyeSpaceDepthOut feature." );

   MultiLine *meta = new MultiLine;

   meta->addStatement( assignOutput( depth ) );

   output = meta;
}

Var *LinearEyeDepthConditioner::_conditionOutput( Var *unconditionedOutput, MultiLine *meta )
{
   Var *retVar = NULL;

   String fracMethodName = (GFX->getAdapterType() == OpenGL) ? "fract" : "frac";

   switch(getBufferFormat())
   {
   case GFXFormatR8G8B8A8:
      retVar = new Var;
      retVar->setType("float4");
      retVar->setName("_ppDepth");
      meta->addStatement( new GenOp( "   // depth conditioner: packing to rgba\r\n" ) );
      meta->addStatement( new GenOp(
         avar( "   @ = %s(@ * (255.0/256) * float4(1, 255, 255 * 255, 255 * 255 * 255));\r\n", fracMethodName.c_str() ),
         new DecOp(retVar), unconditionedOutput ) );
      break;
   default:
      retVar = unconditionedOutput;
      meta->addStatement( new GenOp( "   // depth conditioner: no conditioning\r\n" ) );
      break;
   }

   AssertFatal( retVar != NULL, avar( "Cannot condition output to buffer format: %s", GFXStringTextureFormat[getBufferFormat()] ) );
   return retVar;
}

Var *LinearEyeDepthConditioner::_unconditionInput( Var *conditionedInput, MultiLine *meta )
{
   String float4Typename = (GFX->getAdapterType() == OpenGL) ? "vec4" : "float4";

   Var *retVar = conditionedInput;
   if(getBufferFormat() != GFXFormat_COUNT)
   {
      retVar = new Var;
      retVar->setType(float4Typename.c_str());
      retVar->setName("_ppDepth");
      meta->addStatement( new GenOp( avar( "   @ = %s(0, 0, 1, 1);\r\n", float4Typename.c_str() ), new DecOp(retVar) ) );

      switch(getBufferFormat())
      {
      case GFXFormatR32F:
      case GFXFormatR16F:
         meta->addStatement( new GenOp( "   // depth conditioner: float texture\r\n" ) );
         meta->addStatement( new GenOp( "   @.w = @.r;\r\n", retVar, conditionedInput ) );
         break;

      case GFXFormatR8G8B8A8:
         meta->addStatement( new GenOp( "   // depth conditioner: unpacking from rgba\r\n" ) );
         meta->addStatement( new GenOp(
            avar( "   @.w = dot(@ * (256.0/255), %s(1, 1 / 255, 1 / (255 * 255), 1 / (255 * 255 * 255)));\r\n", float4Typename.c_str() )
            , retVar, conditionedInput ) );
         break;
      default:
         AssertFatal(false, "LinearEyeDepthConditioner::_unconditionInput - Unrecognized buffer format");
      }
   }

   return retVar;
}

Var* LinearEyeDepthConditioner::printMethodHeader( MethodType methodType, const String &methodName, Stream &stream, MultiLine *meta )
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
      if (GFX->getAdapterType() == OpenGL)
         methodVar->setType("vec4");
      else
         methodVar->setType("inline float4");
      DecOp *methodDecl = new DecOp(methodVar);

      Var *prepassSampler = new Var;
      prepassSampler->setName("prepassSamplerVar");
      prepassSampler->setType("sampler2D");
      DecOp *prepassSamplerDecl = new DecOp(prepassSampler);

      Var *screenUV = new Var;
      screenUV->setName("screenUVVar");
      if (GFX->getAdapterType() == OpenGL)
         screenUV->setType("vec2");
      else
         screenUV->setType("float2");
      DecOp *screenUVDecl = new DecOp(screenUV);

      Var *bufferSample = new Var;
      bufferSample->setName("bufferSample");
      if (GFX->getAdapterType() == OpenGL)
         bufferSample->setType("vec4");
      else
         bufferSample->setType("float4");
      DecOp *bufferSampleDecl = new DecOp(bufferSample);

      meta->addStatement( new GenOp( "@(@, @)\r\n", methodDecl, prepassSamplerDecl, screenUVDecl ) );

      meta->addStatement( new GenOp( "{\r\n" ) );

      meta->addStatement( new GenOp( "   // Sampler g-buffer\r\n" ) );

      // The linear depth target has no mipmaps, so use tex2dlod when
      // possible so that the shader compiler can optimize.
      meta->addStatement( new GenOp( "   #if TORQUE_SM >= 30\r\n" ) );
      if (GFX->getAdapterType() == OpenGL)
         meta->addStatement( new GenOp( "    @ = texture2DLod(@, @, 0); \r\n", bufferSampleDecl, prepassSampler, screenUV) );
      else
         meta->addStatement( new GenOp( "      @ = tex2Dlod(@, float4(@,0,0));\r\n", bufferSampleDecl, prepassSampler, screenUV ) );
      meta->addStatement( new GenOp( "   #else\r\n" ) );
      if (GFX->getAdapterType() == OpenGL)
         meta->addStatement( new GenOp( "    @ = texture2D(@, @);\r\n", bufferSampleDecl, prepassSampler, screenUV) );
      else
         meta->addStatement( new GenOp( "      @ = tex2D(@, @);\r\n", bufferSampleDecl, prepassSampler, screenUV ) );
      meta->addStatement( new GenOp( "   #endif\r\n\r\n" ) );

      // We don't use this way of passing var's around, so this should cause a crash
      // if something uses this improperly
      retVal = bufferSample;
   }

   return retVal;
}
