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

#ifndef _WATERBLOCK_H_
#define _WATERBLOCK_H_

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif
#ifndef _SCENEDATA_H_
#include "materials/sceneData.h"
#endif
#ifndef _MATINSTANCE_H_
#include "materials/matInstance.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _RENDERPASSMANAGER_H_
#include "renderInstance/renderPassManager.h"
#endif
#ifndef _WATEROBJECT_H_
#include "waterObject.h"
#endif


//*****************************************************************************
// WaterBlock
//*****************************************************************************
class WaterBlock : public WaterObject
{
   typedef WaterObject Parent;

public:

   // LEGACY support
   enum EWaterType
   {
      eWater            = 0,
      eOceanWater       = 1,
      eRiverWater       = 2,
      eStagnantWater    = 3,
      eLava             = 4,
      eHotLava          = 5,
      eCrustyLava       = 6,
      eQuicksand        = 7,
   };

private:

   enum MaskBits {
      UpdateMask =   Parent::NextFreeMask,
      NextFreeMask = Parent::NextFreeMask << 1
   };
   
   // vertex / index buffers
   Vector< GFXVertexBufferHandle<GFXWaterVertex>* > mVertBuffList;
   Vector<GFXPrimitiveBufferHandle*> mPrimBuffList;
   GFXVertexBufferHandle<GFXVertexPC> mRadialVertBuff;
   GFXPrimitiveBufferHandle mRadialPrimBuff;

   // misc
   F32            mGridElementSize;
   U32            mWidth;
   U32            mHeight;
   F32            mElapsedTime;   
   GFXTexHandle   mBumpTex;
   bool           mGenerateVB;

   // reflect plane
   //ReflectPlane   mReflectPlane;
   //Point3F        mReflectPlaneWorldPos;

   GFXTexHandle   mReflectTex;

   // Stateblocks
   GFXStateBlockRef mUnderwaterSB;
   
   void setupVertexBlock( U32 width, U32 height, U32 rowOffset );
   void setupPrimitiveBlock( U32 width, U32 height );
   void setMultiPassProjection();
   void clearVertBuffers();

   static bool setGridSizeProperty( void *object, const char *index, const char *data );
protected:

   //-------------------------------------------------------
   // Standard engine functions
   //-------------------------------------------------------
   bool onAdd();
   void onRemove();
   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn,           BitStream *stream);

   bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);

public:
   WaterBlock();
   virtual ~WaterBlock();

   DECLARE_CONOBJECT(WaterBlock);   

   static void initPersistFields();
   void onStaticModified( const char* slotName, const char*newValue = NULL );
   virtual void inspectPostApply();
   virtual void setTransform( const MatrixF & mat );
   virtual void setScale( const Point3F &scale );
   virtual bool buildPolyList( PolyListContext context, AbstractPolyList* polyList, const Box3F& box, const SphereF& sphere );

   // WaterObject
   virtual F32 getWaterCoverage( const Box3F &worldBox ) const;
   virtual F32 getSurfaceHeight( const Point2F &pos ) const;
   virtual bool isUnderwater( const Point3F &pnt ) const;

   // WaterBlock   
   bool isPointSubmerged ( const Point3F &pos, bool worldSpace = true ) const{ return true; }
   
   // SceneObject.
   virtual F32 distanceTo( const Point3F& pos ) const;

protected:

   // WaterObject
   virtual void setShaderParams( SceneRenderState *state, BaseMatInstance *mat, const WaterMatParams &paramHandles );
   virtual SceneData setupSceneGraphInfo( SceneRenderState *state );   
   virtual void setupVBIB();
   virtual void innerRender( SceneRenderState *state );
   virtual void _getWaterPlane( const Point3F &camPos, PlaneF &outPlane, Point3F &outPos );
};

#endif // _WATERBLOCK_H_
