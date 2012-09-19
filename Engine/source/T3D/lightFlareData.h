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

#ifndef _LIGHTFLAREDATA_H_
#define _LIGHTFLAREDATA_H_

#ifndef _SIMDATABLOCK_H_
#include "console/simDatablock.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _GFXSTATEBLOCK_H_
#include "gfx/gfxStateBlock.h"
#endif

class LightInfo;
struct ObjectRenderInst;
class SceneRenderState;
class BaseMatInstance;
class GFXOcclusionQuery;

struct LightFlareState
{  
   ~LightFlareState();
   void clear();

   /// Object calling LightFlareData::prepRender fills these in!
   F32 scale;              
   F32 fullBrightness;
   MatrixF lightMat;
   LightInfo *lightInfo;
   F32 worldRadius;

   /// Used internally by LightFlareData!
   U32 visChangedTime;
   bool visible;   
   F32 occlusion;
   GFXVertexBufferHandle<GFXVertexPCT> vertBuffer;   
   GFXOcclusionQuery *occlusionQuery;
   GFXOcclusionQuery *fullPixelQuery;   
};

class LightFlareData : public SimDataBlock
{
   typedef SimDataBlock Parent;
   
   #define MAX_ELEMENTS 20

public:

   LightFlareData();
   virtual ~LightFlareData();

   DECLARE_CONOBJECT( LightFlareData );

   static void initPersistFields();
   virtual void inspectPostApply();

   // SimDataBlock
   virtual bool preload( bool server, String &errorStr );
   virtual void packData( BitStream *stream );
   virtual void unpackData( BitStream *stream );

   /// Submits render instances for corona and flare effects.
   void prepRender( SceneRenderState *state, LightFlareState *flareState );

protected:

   bool _testVisibility(   const SceneRenderState *state, LightFlareState *flareState, 
                           U32 *outVisDelta, F32 *outOcclusionFade, Point3F *outLightPosSS );

   bool _preload( bool server, String &errorStr );
   void _makePrimBuffer( GFXPrimitiveBufferHandle *pb, U32 count );
   void _renderCorona( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

protected:
   
   static const U32 LosMask;
   static const U32 FadeOutTime = 20;
   static const U32 FadeInTime = 125;    
   static Point3F sBasePoints[4];

   // Fields...

   F32 mScale;
   bool mFlareEnabled;
   String mFlareTextureName;
   GFXTexHandle mFlareTexture;
   F32 mOcclusionRadius;
   bool mRenderReflectPass;

   RectF mElementRect[MAX_ELEMENTS];
   F32 mElementDist[MAX_ELEMENTS];
   F32 mElementScale[MAX_ELEMENTS];
   ColorF mElementTint[MAX_ELEMENTS];
   bool mElementRotate[MAX_ELEMENTS];
   bool mElementUseLightColor[MAX_ELEMENTS];   

protected:

   U32 mElementCount;   
   GFXPrimitiveBufferHandle mFlarePrimBuffer;   
};

#endif // _LIGHTFLAREDATA_H_