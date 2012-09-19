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

#ifndef _RENDERTERRAINMGR_H_
#define _RENDERTERRAINMGR_H_

#ifndef _RENDERBINMANAGER_H_
#include "renderInstance/renderBinManager.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif

class TerrCell;
class GFXTextureObject;
class TerrainCellMaterial;


/// The render instance for terrain cells.
struct TerrainRenderInst : public RenderInst
{
   GFXVertexBuffer *vertBuff;

   GFXPrimitiveBuffer *primBuff;

   GFXPrimitive prim;

   BaseMatInstance *mat;

   const MatrixF *objectToWorldXfm;

   TerrainCellMaterial *cellMat;

   /// The lights we pass to the material for 
   /// this cell in order light importance.
   LightInfo *lights[8];

   void clear()
   {
      dMemset( this, 0, sizeof( TerrainRenderInst ) );   
      type = RenderPassManager::RIT_Terrain;      
   }
};


///
class RenderTerrainMgr : public RenderBinManager
{
   typedef RenderBinManager Parent;

protected:

   Vector<TerrainRenderInst*> mInstVector;

   static bool smRenderWireframe;

   static S32 smCellsRendered;
   static S32 smOverrideCells;
   static S32 smDrawCalls;

   static bool _clearStats( GFXDevice::GFXDeviceEventType type );

   // RenderBinManager
   virtual void internalAddElement( RenderInst *inst );

public:

   RenderTerrainMgr();
   RenderTerrainMgr( F32 renderOrder, F32 processAddOrder );
   virtual ~RenderTerrainMgr();

   // ConsoleObject
   static void initPersistFields();
   DECLARE_CONOBJECT(RenderTerrainMgr);

   // RenderBinManager
   virtual void sort();
   virtual void render( SceneRenderState *state );
   virtual void clear();

};

#endif // _RENDERTERRAINMGR_H_
