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
#include "renderInstance/forcedMaterialMeshMgr.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxPrimitiveBuffer.h"
#include "gfx/gfxDebugEvent.h"
#include "materials/sceneData.h"
#include "materials/materialManager.h"
#include "materials/materialDefinition.h"
#include "console/consoleTypes.h"
#include "math/util/matrixSet.h"

IMPLEMENT_CONOBJECT(ForcedMaterialMeshMgr);

ConsoleDocClass( ForcedMaterialMeshMgr,
   "@brief Basically the same as RenderMeshMgr, but will override the material "
   "of the instance. Exists for backwards compatibility, not currently used, soon to be deprecated\n\n"
   "@internal"
);

ForcedMaterialMeshMgr::ForcedMaterialMeshMgr()
{
   mOverrideInstance = NULL;
   mOverrideMaterial = NULL;
}

ForcedMaterialMeshMgr::ForcedMaterialMeshMgr(RenderInstType riType, F32 renderOrder, F32 processAddOrder, BaseMatInstance* overrideMaterial)
: RenderMeshMgr(riType, renderOrder, processAddOrder)
{
   mOverrideInstance = overrideMaterial;
   mOverrideMaterial = NULL;
}

void ForcedMaterialMeshMgr::setOverrideMaterial(BaseMatInstance* overrideMaterial)
{
   SAFE_DELETE(mOverrideInstance);
   mOverrideInstance = overrideMaterial;
}

ForcedMaterialMeshMgr::~ForcedMaterialMeshMgr()
{
   setOverrideMaterial(NULL);
}

void ForcedMaterialMeshMgr::initPersistFields()
{
   addProtectedField("material", TYPEID< Material >(), Offset(mOverrideMaterial, ForcedMaterialMeshMgr),
      &_setOverrideMat, &_getOverrideMat, "Material used to draw all meshes in the render bin.");

   Parent::initPersistFields();
}

void ForcedMaterialMeshMgr::render(SceneRenderState * state)
{
   PROFILE_SCOPE(ForcedMaterialMeshMgr_render);

   if(!mOverrideInstance && mOverrideMaterial.isValid())
   {
      mOverrideInstance = mOverrideMaterial->createMatInstance();
      mOverrideInstance->init( MATMGR->getDefaultFeatures(), getGFXVertexFormat<GFXVertexPNTBT>() );
   }

   // Early out if nothing to draw.
   if(!mElementList.size() || !mOverrideInstance)
      return;

   GFXDEBUGEVENT_SCOPE(ForcedMaterialMeshMgr_Render, ColorI::RED);

   // Automagically save & restore our viewport and transforms.
   GFXTransformSaver saver;

   // init loop data
   SceneData sgData;
   sgData.init( state );

   MeshRenderInst *ri = static_cast<MeshRenderInst*>(mElementList[0].inst);
   setupSGData( ri, sgData );

   while (mOverrideInstance->setupPass(state, sgData))
   {   
      for( U32 j=0; j<mElementList.size(); j++)
      {
         MeshRenderInst* passRI = static_cast<MeshRenderInst*>(mElementList[j].inst);
         if(passRI->primBuff->getPointer()->mPrimitiveArray[passRI->primBuffIndex].numVertices < 1)
            continue;

         getRenderPass()->getMatrixSet().setWorld(*passRI->objectToWorld);
         getRenderPass()->getMatrixSet().setView(*passRI->worldToCamera);
         getRenderPass()->getMatrixSet().setProjection(*passRI->projection);
         mOverrideInstance->setTransforms(getRenderPass()->getMatrixSet(), state);

         mOverrideInstance->setBuffers(passRI->vertBuff, passRI->primBuff);
         GFX->drawPrimitive( passRI->primBuffIndex );                  
      }
   }
}

const char* ForcedMaterialMeshMgr::_getOverrideMat( void *object, const char *data )
{
   ForcedMaterialMeshMgr &mgr = *reinterpret_cast<ForcedMaterialMeshMgr *>( object );
   if( mgr.mOverrideMaterial.isValid() )
      return mgr.mOverrideMaterial->getIdString();
   else
      return "0";
}

bool ForcedMaterialMeshMgr::_setOverrideMat( void *object, const char *index, const char *data )
{
   ForcedMaterialMeshMgr &mgr = *reinterpret_cast<ForcedMaterialMeshMgr *>( object );   
   BaseMatInstance* material;
   Sim::findObject( data, material );
   mgr.setOverrideMaterial( material );
   return false;
}

