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

#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif

#ifndef _CUBEMAPDATA_H_
#include "gfx/sim/cubemapData.h"
#endif

#ifndef _MATERIALLIST_H_
#include "materials/materialList.h"
#endif

#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif

#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif


GFXDeclareVertexFormat( GFXSkyVertex )
{
   Point3F point;
   Point3F normal;
   GFXVertexColor color;
};


struct SkyMatParams
{
   void init( BaseMatInstance *matInst ) {};
};

class MatrixSet;

class SkyBox : public SceneObject
{
   typedef SceneObject Parent;

public:

   SkyBox();
   virtual ~SkyBox();

   DECLARE_CONOBJECT( SkyBox );

   // SimObject
   void onStaticModified( const char *slotName, const char *newValue );

   // ConsoleObject
   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();
   virtual void inspectPostApply();      

   // NetObject
   virtual U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
   virtual void unpackUpdate( NetConnection *conn, BitStream *stream );

   // SceneObject
   void prepRenderImage( SceneRenderState* state );

   /// Our render delegate.
   void _renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *mi );

   /// Prepares rendering structures and geometry.
   void _initRender();

protected:

   // Material 
   String mMatName;
   BaseMatInstance *mMatInstance;
   SkyMatParams mMatParamHandle;

   SimObjectPtr<Material> mMaterial;
   
   GFXVertexBufferHandle<GFXVertexPNT> mVB;

   GFXVertexBufferHandle<GFXVertexPC> mFogBandVB;
   Material *mFogBandMat;
   BaseMatInstance *mFogBandMatInst;

   ColorF mLastFogColor;

   bool mDrawBottom;
   bool mIsVBDirty;
   U32 mPrimCount;

   MatrixSet *mMatrixSet;

   F32 mFogBandHeight;   

   void _updateMaterial();
   void _initMaterial();

   BaseMatInstance* _getMaterialInstance();
};

#endif // _SKYBOX_H_