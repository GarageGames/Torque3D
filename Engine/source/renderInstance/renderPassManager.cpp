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
#include "renderInstance/renderPassManager.h"

#include "materials/sceneData.h"
#include "materials/matInstance.h"
#include "materials/customMaterialDefinition.h"
#include "materials/materialManager.h"
#include "scene/sceneManager.h"
#include "scene/sceneObject.h"
#include "gfx/primBuilder.h"
#include "platform/profiler.h"
#include "renderInstance/renderBinManager.h"
#include "renderInstance/renderObjectMgr.h"
#include "renderInstance/renderMeshMgr.h"
#include "renderInstance/renderTranslucentMgr.h"
#include "renderInstance/renderGlowMgr.h"
#include "renderInstance/renderTerrainMgr.h"
#include "core/util/safeDelete.h"
#include "math/util/matrixSet.h"
#include "console/engineAPI.h"


const RenderInstType RenderInstType::Invalid( "" );

const RenderInstType RenderPassManager::RIT_Mesh("Mesh");
const RenderInstType RenderPassManager::RIT_Shadow("Shadow");
const RenderInstType RenderPassManager::RIT_Sky("Sky");
const RenderInstType RenderPassManager::RIT_Terrain("Terrain");
const RenderInstType RenderPassManager::RIT_Object("Object");      
const RenderInstType RenderPassManager::RIT_ObjectTranslucent("ObjectTranslucent");
const RenderInstType RenderPassManager::RIT_Decal("Decal");
const RenderInstType RenderPassManager::RIT_Water("Water");
const RenderInstType RenderPassManager::RIT_Foliage("Foliage");
const RenderInstType RenderPassManager::RIT_Translucent("Translucent");
const RenderInstType RenderPassManager::RIT_Begin("Begin");
const RenderInstType RenderPassManager::RIT_Custom("Custom");
const RenderInstType RenderPassManager::RIT_Particle("Particle");
const RenderInstType RenderPassManager::RIT_Occluder("Occluder");
const RenderInstType RenderPassManager::RIT_Editor("Editor");


//*****************************************************************************
// RenderInstance
//*****************************************************************************

void RenderInst::clear()
{
   dMemset( this, 0, sizeof(RenderInst) );
}

void MeshRenderInst::clear()
{
   dMemset( this, 0, sizeof(MeshRenderInst) );
   visibility = 1.0f;
}

void ParticleRenderInst::clear()
{
   dMemset( this, 0, sizeof(ParticleRenderInst) );
}

void ObjectRenderInst::clear()
{
   userData = NULL;

   dMemset( this, 0, sizeof( ObjectRenderInst ) );

   // The memset here is kinda wrong... it clears the
   // state initialized by the delegate constructor.
   //
   // This fixes it... but we probably need to have a
   // real constructor for RenderInsts.
   renderDelegate.clear();
}

void OccluderRenderInst::clear()
{
   dMemset( this, 0, sizeof(OccluderRenderInst) );
}


IMPLEMENT_CONOBJECT(RenderPassManager);


ConsoleDocClass( RenderPassManager, 
   "@brief A grouping of render bin managers which forms a render pass.\n\n"
   "The render pass is used to order a set of RenderBinManager objects which are used "
   "when rendering a scene.  This class does little work itself other than managing "
   "its list of render bins.\n\n"
   "In 'core/scripts/client/renderManager.cs' you will find the DiffuseRenderPassManager "
   "which is used by the C++ engine to render the scene.\n\n"
   "@see RenderBinManager\n"
   "@ingroup RenderBin\n" );

RenderPassManager::RenderBinEventSignal& RenderPassManager::getRenderBinSignal()
{
   static RenderBinEventSignal theSignal;
   return theSignal;
}

void RenderPassManager::initPersistFields()
{
}

RenderPassManager::RenderPassManager()
{   
   mSceneManager = NULL;
   VECTOR_SET_ASSOCIATION( mRenderBins );

   mMatrixSet = reinterpret_cast<MatrixSet *>(dMalloc_aligned(sizeof(MatrixSet), 16));
   constructInPlace(mMatrixSet);
}

RenderPassManager::~RenderPassManager()
{
   dFree_aligned(mMatrixSet);

   // Any bins left need to be deleted.
   for ( U32 i=0; i<mRenderBins.size(); i++ )
   {
      RenderBinManager *bin = mRenderBins[i];

      // Clear the parent first, so that RenderBinManager::onRemove()
      // won't call removeManager() and break this loop.
      bin->setRenderPass( NULL );
      bin->deleteObject();
   }
}

void RenderPassManager::_insertSort(Vector<RenderBinManager*>& list, RenderBinManager* mgr, bool renderOrder)
{
   U32 i;
   for (i = 0; i < list.size(); i++)
   {
      bool renderCompare = mgr->getRenderOrder() < list[i]->getRenderOrder();
      bool processAddCompare = mgr->getProcessAddOrder() < list[i]->getProcessAddOrder();
      if ((renderOrder && renderCompare) || (!renderOrder && processAddCompare))      
      {
         list.insert(i);
         list[i] = mgr;
         return;
      }
   }

   list.push_back(mgr);
}

void RenderPassManager::addManager(RenderBinManager* mgr)
{
   if ( !mgr->isProperlyAdded() )
      mgr->registerObject();

   AssertFatal( mgr->getRenderPass() == NULL, "RenderPassManager::addManager() - Bin is still part of another pass manager!" );
   mgr->setRenderPass( this );

   _insertSort(mRenderBins, mgr, true);
}

void RenderPassManager::removeManager(RenderBinManager* mgr)
{
   AssertFatal( mgr->getRenderPass() == this, "RenderPassManager::removeManager() - We do not own this bin!" );

   mgr->setRenderPass( NULL );
   mRenderBins.remove( mgr );
}

RenderBinManager* RenderPassManager::getManager(S32 i) const
{
   if (i >= 0 && i < mRenderBins.size())
      return mRenderBins[i];
   else
      return NULL;
}

void RenderPassManager::addInst( RenderInst *inst )
{
   PROFILE_SCOPE( RenderPassManager_addInst );

   AssertFatal( inst != NULL, "RenderPassManager::addInst - Got null instance!" );

   AddInstTable::Iterator iter = mAddInstSignals.find( inst->type );
   if ( iter == mAddInstSignals.end() )
      return;

   iter->value.trigger( inst );
}

void RenderPassManager::sort()
{
   PROFILE_SCOPE( RenderPassManager_Sort );

   for (Vector<RenderBinManager *>::iterator itr = mRenderBins.begin();
      itr != mRenderBins.end(); itr++)
   {
      AssertFatal(*itr, "Render manager invalid!");
      (*itr)->sort();
   }
}

void RenderPassManager::clear()
{
   PROFILE_SCOPE( RenderPassManager_Clear );

   mChunker.clear();

   for (Vector<RenderBinManager *>::iterator itr = mRenderBins.begin();
      itr != mRenderBins.end(); itr++)
   {
      AssertFatal(*itr, "Invalid render manager!");
      (*itr)->clear();
   }
}

void RenderPassManager::render(SceneRenderState * state)
{
   PROFILE_SCOPE( RenderPassManager_Render );

   GFX->pushWorldMatrix();
   MatrixF proj = GFX->getProjectionMatrix();

   
   for (Vector<RenderBinManager *>::iterator itr = mRenderBins.begin();
      itr != mRenderBins.end(); itr++)
   {
      RenderBinManager *curBin = *itr;
      AssertFatal(curBin, "Invalid render manager!");
      getRenderBinSignal().trigger(curBin, state, true);
      curBin->render(state);
      getRenderBinSignal().trigger(curBin, state, false);
   }

   GFX->popWorldMatrix();
   GFX->setProjectionMatrix( proj );
      
   // Restore a clean state for subsequent rendering.
   GFX->disableShaders();
   for(S32 i = 0; i < GFX->getNumSamplers(); ++i)
      GFX->setTexture(i, NULL);
}

void RenderPassManager::renderPass(SceneRenderState * state)
{
   PROFILE_SCOPE( RenderPassManager_RenderPass );
   sort();
   render(state);
   clear();
}

GFXTextureObject *RenderPassManager::getDepthTargetTexture()
{
   // If this is OpenGL, or something else has set the depth buffer, return the pointer
   if( mDepthBuff.isValid() )
   {
      // If this is OpenGL, make sure the depth target matches up
      // with the active render target.  Otherwise recreate.
      
      if( GFX->getAdapterType() == OpenGL )
      {
         GFXTarget* activeRT = GFX->getActiveRenderTarget();
         AssertFatal( activeRT, "Must be an active render target to call 'getDepthTargetTexture'" );
         
         Point2I activeRTSize = activeRT->getSize();
         if( mDepthBuff.getWidth() == activeRTSize.x &&
             mDepthBuff.getHeight() == activeRTSize.y )
            return mDepthBuff.getPointer();
      }
      else
         return mDepthBuff.getPointer();
   }

   if(GFX->getAdapterType() == OpenGL)
   {
      AssertFatal(GFX->getActiveRenderTarget(), "Must be an active render target to call 'getDepthTargetTexture'");

      const Point2I rtSize = GFX->getActiveRenderTarget()->getSize();
      mDepthBuff.set(rtSize.x, rtSize.y, GFXFormatD24S8, 
         &GFXDefaultZTargetProfile, avar("%s() - mDepthBuff (line %d)", __FUNCTION__, __LINE__));
      return mDepthBuff.getPointer();
   }

   // Default return value
   return GFXTextureTarget::sDefaultDepthStencil;
}

void RenderPassManager::setDepthTargetTexture( GFXTextureObject *zTarget )
{
   mDepthBuff = zTarget;
}

const MatrixF* RenderPassManager::allocSharedXform( SharedTransformType stt )
{
   AssertFatal(stt == View || stt == Projection, "Bad shared transform type");

   // Enable this to simulate non-shared transform performance
   //#define SIMULATE_NON_SHARED_TRANSFORMS
#ifdef SIMULATE_NON_SHARED_TRANSFORMS
   return allocUniqueXform(stt == View ? mMatrixSet->getWorldToCamera() : mMatrixSet->getCameraToScreen());
#else
   return &(stt == View ? mMatrixSet->getWorldToCamera() : mMatrixSet->getCameraToScreen());
#endif
}

void RenderPassManager::assignSharedXform( SharedTransformType stt, const MatrixF &xfm )
{
   AssertFatal(stt == View || stt == Projection, "Bad shared transform type");
   if(stt == View)
      mMatrixSet->setSceneView(xfm);
   else
      mMatrixSet->setSceneProjection(xfm);
}

// Script interface

DefineEngineMethod(RenderPassManager, getManagerCount, S32, (),, 
   "Returns the total number of bin managers." )
{
   return object->getManagerCount();
}

DefineEngineMethod( RenderPassManager, getManager, RenderBinManager*, ( S32 index ),,
   "Returns the render bin manager at the index or null if the index is out of range." )
{
   if(index < 0 || index >= object->getManagerCount())
      return NULL;

   return object->getManager(index);
}

DefineEngineMethod( RenderPassManager, addManager, void, ( RenderBinManager *renderBin ),, 
   "Add as a render bin manager to the pass." )
{
   if ( renderBin )
      object->addManager( renderBin );
}

DefineEngineMethod( RenderPassManager, removeManager, void, ( RenderBinManager *renderBin ),, 
   "Removes a render bin manager." )
{
   if ( renderBin )
      object->removeManager( renderBin );
}

