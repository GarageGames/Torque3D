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
#include "renderInstance/renderBinManager.h"

#include "console/consoleTypes.h"
#include "materials/matInstance.h"
#include "scene/sceneManager.h"
#include "console/engineAPI.h"


IMPLEMENT_CONOBJECT(RenderBinManager);


RenderBinManager::RenderBinManager( const RenderInstType& ritype, F32 renderOrder, F32 processAddOrder ) :
   mRenderInstType( ritype ),
   mRenderOrder( renderOrder ),
   mProcessAddOrder( processAddOrder ),
   mRenderPass( NULL ),
   mBasicOnly ( false )
{
   VECTOR_SET_ASSOCIATION( mElementList );
   mElementList.reserve( 2048 );
}


ConsoleDocClass( RenderBinManager, 
   "@brief The abstract base for all render bins.\n\n"
   "The render bins are used by the engine as a high level method to order and batch rendering "
   "operations.\n"
   "@ingroup RenderBin\n" );

void RenderBinManager::initPersistFields()
{
   addField( "binType", TypeRealString, Offset(mRenderInstType.mName, RenderBinManager),
      "Sets the render bin type which limits what render instances are added to this bin." );

   addField("renderOrder", TypeF32, Offset(mRenderOrder, RenderBinManager),
      "Defines the order for rendering in relation to other bins." );

   addField("processAddOrder", TypeF32, Offset(mProcessAddOrder, RenderBinManager),
      "Defines the order for adding instances in relation to other bins." );

   addField( "basicOnly", TypeBool, Offset(mBasicOnly, RenderBinManager),
      "Limites the render bin to basic lighting only." );

   Parent::initPersistFields();
}

void RenderBinManager::onRemove()
{
   // Tell the render pass to remove us when 
   // we're being unregistered.
   if ( mRenderPass )
      mRenderPass->removeManager( this );

   Parent::onRemove();
}

void RenderBinManager::notifyType( const RenderInstType &type )
{
   // Avoid duplicate types.
   if (  !type.isValid() ||
         mRenderInstType == type ||
         mOtherTypes.contains( type ) )
      return;

   mOtherTypes.push_back( type );

   // Register for the signal if the pass
   // has already been assigned.
   if ( mRenderPass )
      mRenderPass->getAddSignal(type).notify( this, &RenderBinManager::addElement, mProcessAddOrder );
}

void RenderBinManager::setRenderPass( RenderPassManager *rpm )
{
   if ( mRenderPass )
   {
      if ( mRenderInstType.isValid() )
         mRenderPass->getAddSignal(mRenderInstType).remove( this, &RenderBinManager::addElement );

      for ( U32 i=0; i < mOtherTypes.size(); i++ )
         mRenderPass->getAddSignal(mOtherTypes[i]).remove( this, &RenderBinManager::addElement );         
   }

   mRenderPass = rpm;

   if ( mRenderPass )
   {
      if ( mRenderInstType.isValid() )
         mRenderPass->getAddSignal(mRenderInstType).notify( this, &RenderBinManager::addElement, mProcessAddOrder );

      for ( U32 i=0; i < mOtherTypes.size(); i++ )
         mRenderPass->getAddSignal(mOtherTypes[i]).notify( this, &RenderBinManager::addElement, mProcessAddOrder );
   }
}

void RenderBinManager::addElement( RenderInst *inst )
{   
   internalAddElement(inst);
}

void RenderBinManager::internalAddElement(RenderInst* inst)
{
   mElementList.increment();
   MainSortElem &elem = mElementList.last();
   elem.inst = inst;

   elem.key = inst->defaultKey;
   elem.key2 = inst->defaultKey2;
}

void RenderBinManager::clear()
{
   mElementList.clear();
}

void RenderBinManager::sort()
{
   dQsort( mElementList.address(), mElementList.size(), sizeof(MainSortElem), cmpKeyFunc);
}

S32 FN_CDECL RenderBinManager::cmpKeyFunc(const void* p1, const void* p2)
{
   const MainSortElem* mse1 = (const MainSortElem*) p1;
   const MainSortElem* mse2 = (const MainSortElem*) p2;

   S32 test1 = S32(mse2->key) - S32(mse1->key);

   return ( test1 == 0 ) ? S32(mse1->key2) - S32(mse2->key2) : test1;
}

void RenderBinManager::setupSGData( MeshRenderInst *ri, SceneData &data )
{
   PROFILE_SCOPE( RenderBinManager_setupSGData );

   // NOTE: We do not reset or clear the scene state 
   // here as the caller has initialized non-RI members 
   // himself and we must preserve them.
   //
   // It also saves a bunch of CPU as this is called for
   // every MeshRenderInst in every pass. 

   dMemcpy( data.lights, ri->lights, sizeof( data.lights ) );
   data.objTrans     = ri->objectToWorld;
   data.backBuffTex  = ri->backBuffTex;
   data.cubemap      = ri->cubemap;
   data.miscTex      = ri->miscTex;
   data.reflectTex   = ri->reflectTex;
   data.accuTex      = ri->accuTex;
   data.lightmap     = ri->lightmap;
   data.visibility   = ri->visibility;
   data.materialHint = ri->materialHint;
}

DefineEngineMethod( RenderBinManager, getBinType, const char*, (),,
   "Returns the bin type string." )
{
   return object->getRenderInstType().getName();
}
