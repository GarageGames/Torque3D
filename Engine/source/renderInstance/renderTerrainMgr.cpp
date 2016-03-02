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
#include "renderInstance/renderTerrainMgr.h"

#include "platform/profiler.h"
#include "scene/sceneManager.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxDebugEvent.h"
#include "materials/shaderData.h"
#include "materials/matInstance.h"
#include "scene/sceneRenderState.h"
#include "console/consoleTypes.h"
#include "terrain/terrCell.h"
#include "terrain/terrCellMaterial.h"
#include "math/util/matrixSet.h"
#include "materials/materialManager.h"

bool RenderTerrainMgr::smRenderWireframe = false;

S32 RenderTerrainMgr::smCellsRendered = 0;
S32 RenderTerrainMgr::smOverrideCells = 0;
S32 RenderTerrainMgr::smDrawCalls = 0;


IMPLEMENT_CONOBJECT(RenderTerrainMgr);


ConsoleDocClass( RenderTerrainMgr, 
   "@brief A render bin for terrain mesh rendering.\n\n"
   "This bin renders terrain render instances from a TerrainBlock. "
   "Normally a mesh would render via the RenderMeshMgr, but terrain uses "
   "a TerrainMaterial designed for multi-layered surfaces which this "
   "bin can processs.\n\n"
   "@ingroup RenderBin\n" );

RenderTerrainMgr::RenderTerrainMgr()
   :  RenderBinManager( RenderPassManager::RIT_Terrain, 1.0f, 1.0f )
{
}

RenderTerrainMgr::RenderTerrainMgr( F32 renderOrder, F32 processAddOrder )
   :  RenderBinManager( RenderPassManager::RIT_Terrain, renderOrder, processAddOrder )
{
}

RenderTerrainMgr::~RenderTerrainMgr()
{
}

void RenderTerrainMgr::initPersistFields()
{
   Con::addVariable( "RenderTerrainMgr::renderWireframe", TypeBool, &smRenderWireframe,
      "Used to enable wireframe rendering on terrain for debugging.\n"
      "@ingroup RenderBin\n" );

   // For stats.
   GFXDevice::getDeviceEventSignal().notify( &RenderTerrainMgr::_clearStats );
   Con::addVariable( "$TerrainBlock::cellsRendered", TypeS32, &smCellsRendered, "@internal" );
   Con::addVariable( "$TerrainBlock::overrideCells", TypeS32, &smOverrideCells, "@internal" );
   Con::addVariable( "$TerrainBlock::drawCalls", TypeS32, &smDrawCalls, "@internal" );

   Parent::initPersistFields();
}

bool RenderTerrainMgr::_clearStats( GFXDevice::GFXDeviceEventType type )
{
   if ( type == GFXDevice::deStartOfFrame )
   {
      smCellsRendered = 0;
      smOverrideCells = 0;
      smDrawCalls = 0;
   }

   return true;
}

void RenderTerrainMgr::internalAddElement( RenderInst *inst_ )
{
   TerrainRenderInst *inst = static_cast<TerrainRenderInst*>( inst_ );
   mInstVector.push_back( inst );
}

void RenderTerrainMgr::sort()
{
   // TODO: We could probably sort this in some
   // manner to improve terrain rendering perf.
}

void RenderTerrainMgr::clear()
{
   mInstVector.clear();
}

void RenderTerrainMgr::render( SceneRenderState *state )
{
   if ( mInstVector.empty() )
      return;

   // Check if bin is disabled in advanced lighting.
   if ( MATMGR->getPrePassEnabled() && mBasicOnly )
      return;

   PROFILE_SCOPE( RenderTerrainMgr_Render );

   GFXTransformSaver saver;

   // Prepare the common scene graph data.
   SceneData sgData;
   sgData.init( state );
   sgData.wireframe |= smRenderWireframe;

   // Restore transforms
   MatrixSet &matrixSet = getRenderPass()->getMatrixSet();
   matrixSet.restoreSceneViewProjection();

   GFXDEBUGEVENT_SCOPE( RenderTerrainMgr_Render, ColorI::GREEN );

   const MatrixF worldViewXfm = matrixSet.getWorldToCamera();
   const MatrixF &projXfm = matrixSet.getCameraToScreen();

   // HACKTASTIC!
   //
   // If this is a shadow pass then render the terrain
   // with the first material we get.
   //
   // In the near future terrains will operate using regular
   // MatInstance materials like any other mesh in which case
   // this will all be replaced by a normal mesh render bin.
   //
   if ( state->isShadowPass() )
   {      
      PROFILE_SCOPE( RenderTerrainMgr_Render_Shadow );

      Vector<TerrainRenderInst*>::iterator inst = mInstVector.begin();
      BaseMatInstance *overideMat = (*inst)->mat;
      while ( overideMat && overideMat->setupPass( state, sgData ) )
      {
         for ( ; inst != mInstVector.end(); inst++ )
         {
            smOverrideCells++;

            GFX->setPrimitiveBuffer( (*inst)->primBuff );
            GFX->setVertexBuffer( (*inst)->vertBuff );

            matrixSet.setWorld(*(*inst)->objectToWorldXfm);

            overideMat->setSceneInfo( state, sgData );
            overideMat->setTransforms( matrixSet, state );

            GFX->drawPrimitive( (*inst)->prim );
         }
      }

      return;
   }

   // Do the detail map passes.
   Vector<TerrainRenderInst*>::iterator inst = mInstVector.begin();
   for ( ; inst != mInstVector.end(); inst++ )
   {
      TerrainCellMaterial *mat = (*inst)->cellMat;

      GFX->setPrimitiveBuffer( (*inst)->primBuff );
      GFX->setVertexBuffer( (*inst)->vertBuff );

      ++smCellsRendered;

      mat->setTransformAndEye(   *(*inst)->objectToWorldXfm,
                                 worldViewXfm,
                                 projXfm,
                                 state->getFarPlane() );

      sgData.objTrans = (*inst)->objectToWorldXfm;
      dMemcpy( sgData.lights, (*inst)->lights, sizeof( sgData.lights ) );

      while ( mat->setupPass( state, sgData ) )
      {
         ++smDrawCalls;
         GFX->drawPrimitive( (*inst)->prim );
      }
   }
}

