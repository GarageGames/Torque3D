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
#include "renderOcclusionMgr.h"

#include "console/consoleTypes.h"
#include "scene/sceneObject.h"
#include "gfx/gfxOcclusionQuery.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"
#include "math/util/sphereMesh.h"
#include "gfx/gfxDebugEvent.h"


IMPLEMENT_CONOBJECT(RenderOcclusionMgr);

ConsoleDocClass( RenderOcclusionMgr, 
   "@brief A render bin which renders occlusion query requests.\n\n"
   "This render bin gathers occlusion query render instances and renders them. "
   "It is currently used by light flares and ShapeBase reflection cubemaps.\n\n"
   "You can type '$RenderOcclusionMgr::debugRender = true' in the console to "
   "see debug rendering of the occlusion geometry.\n\n"
   "@ingroup RenderBin\n" );


bool RenderOcclusionMgr::smDebugRender = false;

RenderOcclusionMgr::RenderOcclusionMgr()
: RenderBinManager(RenderPassManager::RIT_Occluder, 1.0f, 1.0f)
{
   mOverrideMat = NULL;
   mSpherePrimCount = 0;
}

RenderOcclusionMgr::RenderOcclusionMgr(RenderInstType riType, F32 renderOrder, F32 processAddOrder)
: RenderBinManager(riType, renderOrder, processAddOrder)
{  
   mOverrideMat = NULL;
}

static const Point3F cubePoints[8] = 
{
   Point3F(-0.5, -0.5, -0.5), Point3F(-0.5, -0.5,  0.5), Point3F(-0.5,  0.5, -0.5), Point3F(-0.5,  0.5,  0.5),
   Point3F( 0.5, -0.5, -0.5), Point3F( 0.5, -0.5,  0.5), Point3F( 0.5,  0.5, -0.5), Point3F( 0.5,  0.5,  0.5)
};

static const U32 cubeFaces[6][4] = 
{
   { 0, 4, 6, 2 }, { 0, 2, 3, 1 }, { 0, 1, 5, 4 },
   { 3, 2, 6, 7 }, { 7, 6, 4, 5 }, { 3, 7, 5, 1 }
};

void RenderOcclusionMgr::init()
{
   GFXStateBlockDesc d;

   d.setBlend( false );   
   d.cullDefined = true;
   d.cullMode = GFXCullCCW;
   d.setZReadWrite( true, false );   

   mDebugSB = GFX->createStateBlock(d);

   d.setColorWrites( false, false, false, false );

   mNormalSB = GFX->createStateBlock(d);      

   d.setZReadWrite( false, false );

   mTestSB = GFX->createStateBlock(d);

   mBoxBuff.set( GFX, 36, GFXBufferTypeStatic );
   GFXVertexPC *verts = mBoxBuff.lock();

   U32 vertexIndex = 0;
   U32 idx;
   for(int i = 0; i < 6; i++)
   {
      idx = cubeFaces[i][0];
      verts[vertexIndex].point = cubePoints[idx];
      verts[vertexIndex].color.set( 1,0,1,1 );
      vertexIndex++;

      idx = cubeFaces[i][1];
      verts[vertexIndex].point = cubePoints[idx];
      verts[vertexIndex].color.set( 1,0,1,1 );
      vertexIndex++;

      idx = cubeFaces[i][3];
      verts[vertexIndex].point = cubePoints[idx];
      verts[vertexIndex].color.set( 1,0,1,1 );
      vertexIndex++;

      idx = cubeFaces[i][1];
      verts[vertexIndex].point = cubePoints[idx];
      verts[vertexIndex].color.set( 1,0,1,1 );
      vertexIndex++;

      idx = cubeFaces[i][3];
      verts[vertexIndex].point = cubePoints[idx];
      verts[vertexIndex].color.set( 1,0,1,1 );
      vertexIndex++;

      idx = cubeFaces[i][2];
      verts[vertexIndex].point = cubePoints[idx];
      verts[vertexIndex].color.set( 1,0,1,1 );
      vertexIndex++;
   }

   mBoxBuff.unlock();

   SphereMesh sphere;
   const SphereMesh::TriangleMesh *sphereMesh = sphere.getMesh(1);
   mSpherePrimCount = sphereMesh->numPoly;
   mSphereBuff.set( GFX, mSpherePrimCount * 3, GFXBufferTypeStatic );
   verts = mSphereBuff.lock();
   vertexIndex = 0;

   for ( S32 i = 0; i < mSpherePrimCount; i++ )
   {      
      verts[vertexIndex].point = sphereMesh->poly[i].pnt[0];
      verts[vertexIndex].color.set( 1,0,1,1 );
      vertexIndex++;

      verts[vertexIndex].point = sphereMesh->poly[i].pnt[1];
      verts[vertexIndex].color.set( 1,0,1,1 );
      vertexIndex++;

      verts[vertexIndex].point = sphereMesh->poly[i].pnt[2];
      verts[vertexIndex].color.set( 1,0,1,1 );
      vertexIndex++;
   }
   mSphereBuff.unlock();
}

void RenderOcclusionMgr::consoleInit()
{
   Con::addVariable( "$RenderOcclusionMgr::debugRender", TypeBool, &RenderOcclusionMgr::smDebugRender,
      "@brief A debugging feature which renders the occlusion volumes to the scene.\n"
      "@see RenderOcclusionMgr\n"
      "@ingroup RenderBin\n" );
}

void RenderOcclusionMgr::initPersistFields()
{
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// render objects
//-----------------------------------------------------------------------------
void RenderOcclusionMgr::render( SceneRenderState *state )
{
   PROFILE_SCOPE(RenderOcclusionMgr_render);

   // Early out if nothing to draw.
   if ( !mElementList.size() )
      return;
   
   GFXDEBUGEVENT_SCOPE(RenderOcclusionMgr_Render, ColorI::BLUE);

   if ( mNormalSB.isNull() )
      init();

   GFX->disableShaders();
   GFX->setupGenericShaders( GFXDevice::GSColor );  


   OccluderRenderInst *firstEl = static_cast<OccluderRenderInst*>(mElementList[0].inst);
   
   if ( firstEl->isSphere )   
      GFX->setVertexBuffer( mSphereBuff );
   else
      GFX->setVertexBuffer( mBoxBuff );

   bool wasSphere = firstEl->isSphere;   


   for( U32 i=0; i<mElementList.size(); i++ )
   {
      OccluderRenderInst *ri = static_cast<OccluderRenderInst*>(mElementList[i].inst);
      
      AssertFatal( ri->query != NULL, "RenderOcclusionMgr::render, OcclusionRenderInst has NULL GFXOcclusionQuery" );

      if ( ri->isSphere != wasSphere )
      {
         if ( ri->isSphere )
            GFX->setVertexBuffer( mSphereBuff );
         else
            GFX->setVertexBuffer( mBoxBuff );

         wasSphere = ri->isSphere;
      }

      GFX->pushWorldMatrix();
      
      MatrixF xfm( *ri->orientation );
      xfm.setPosition( ri->position );      
      xfm.scale( ri->scale );

      //GFXTransformSaver saver;      
      GFX->multWorld( xfm );

      if ( smDebugRender )
         GFX->setStateBlock( mDebugSB );
      else
         GFX->setStateBlock( mNormalSB );   

      ri->query->begin();
      
      if ( wasSphere )
         GFX->drawPrimitive( GFXTriangleList, 0, mSpherePrimCount );
      else
         GFX->drawPrimitive( GFXTriangleList, 0, 12 );

      ri->query->end();

      if ( ri->query2 )
      {
         GFX->setStateBlock( mTestSB );

         ri->query2->begin();

         if ( wasSphere )
            GFX->drawPrimitive( GFXTriangleList, 0, mSpherePrimCount );
         else
            GFX->drawPrimitive( GFXTriangleList, 0, 12 );

         ri->query2->end();
      }

      GFX->popWorldMatrix();      
   }
}