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
#include "renderObjectMgr.h"
#include "console/consoleTypes.h"
#include "scene/sceneObject.h"
#include "materials/materialManager.h"
#include "scene/sceneRenderState.h"

IMPLEMENT_CONOBJECT(RenderObjectMgr);

ConsoleDocClass( RenderObjectMgr, 
   "@brief A render bin which uses object callbacks for rendering.\n\n"
   "This render bin gathers object render instances and calls its delegate "
   "method to perform rendering.  It is used infrequently for specialized "
   "scene objects which perform custom rendering.\n\n"
   "@ingroup RenderBin\n" );


RenderObjectMgr::RenderObjectMgr()
: RenderBinManager(RenderPassManager::RIT_Object, 1.0f, 1.0f)
{
   mOverrideMat = NULL;
}

RenderObjectMgr::RenderObjectMgr(RenderInstType riType, F32 renderOrder, F32 processAddOrder)
 : RenderBinManager(riType, renderOrder, processAddOrder)
{  
   mOverrideMat = NULL;
}

void RenderObjectMgr::initPersistFields()
{
   Parent::initPersistFields();
}

void RenderObjectMgr::setOverrideMaterial(BaseMatInstance* overrideMat)
{ 
   mOverrideMat = overrideMat; 
}

//-----------------------------------------------------------------------------
// render objects
//-----------------------------------------------------------------------------
void RenderObjectMgr::render( SceneRenderState *state )
{
   PROFILE_SCOPE(RenderObjectMgr_render);

   // Early out if nothing to draw.
   if(!mElementList.size())
      return;

   // Check if bin is disabled in advanced lighting.
   if ( MATMGR->getPrePassEnabled() && mBasicOnly )
      return;

   for( U32 i=0; i<mElementList.size(); i++ )
   {
      ObjectRenderInst *ri = static_cast<ObjectRenderInst*>(mElementList[i].inst);
      if ( ri->renderDelegate )
         ri->renderDelegate( ri, state, mOverrideMat );      
   }
}