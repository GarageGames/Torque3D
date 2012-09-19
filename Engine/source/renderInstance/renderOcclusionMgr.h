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
#ifndef _RENDEROCCLUSIONMGR_H_
#define _RENDEROCCLUSIONMGR_H_

#ifndef _RENDERBINMANAGER_H_
#include "renderInstance/renderBinManager.h"
#endif

//**************************************************************************
// RenderOcclusionMgr
//**************************************************************************
class RenderOcclusionMgr : public RenderBinManager
{
   typedef RenderBinManager Parent;
public:
   RenderOcclusionMgr();
   RenderOcclusionMgr(RenderInstType riType, F32 renderOrder, F32 processAddOrder);

   // RenderOcclusionMgr
   virtual void init();
   virtual void render(SceneRenderState * state);

   // ConsoleObject
   static void consoleInit();
   static void initPersistFields();
   DECLARE_CONOBJECT(RenderOcclusionMgr);

protected:
   BaseMatInstance* mOverrideMat;
   GFXStateBlockRef mNormalSB;
   GFXStateBlockRef mTestSB;
   
   GFXStateBlockRef mDebugSB;
   static bool smDebugRender;

   GFXVertexBufferHandle<GFXVertexPC> mBoxBuff;
   GFXVertexBufferHandle<GFXVertexPC> mSphereBuff;
   U32 mSpherePrimCount;
};

#endif // _RENDEROCCLUSIONMGR_H_