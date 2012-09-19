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

#ifndef _RENDERGLOWMGR_H_
#define _RENDERGLOWMGR_H_

#ifndef _TEXTARGETBIN_MGR_H_
#include "renderInstance/renderTexTargetBinManager.h"
#endif


class PostEffect;


///
class RenderGlowMgr : public RenderTexTargetBinManager
{  
   typedef RenderTexTargetBinManager Parent;

public:

   RenderGlowMgr();
   virtual ~RenderGlowMgr();

   /// Returns the glow post effect.
   PostEffect* getGlowEffect();

   /// Returns true if the glow post effect is
   /// enabled and the glow buffer should be updated.
   bool isGlowEnabled();

   // RenderBinManager
   virtual void addElement( RenderInst *inst );
   virtual void render( SceneRenderState *state );

   // ConsoleObject
   DECLARE_CONOBJECT( RenderGlowMgr );

protected:

   class GlowMaterialHook : public MatInstanceHook
   {
   public:

      GlowMaterialHook( BaseMatInstance *matInst );
      virtual ~GlowMaterialHook();

      virtual BaseMatInstance *getMatInstance() { return mGlowMatInst; }

      virtual const MatInstanceHookType& getType() const { return Type; }

      /// Our material hook type.
      static const MatInstanceHookType Type;

   protected:

      static void _overrideFeatures(   ProcessedMaterial *mat,
                                       U32 stageNum,
                                       MaterialFeatureData &fd, 
                                       const FeatureSet &features );

      BaseMatInstance *mGlowMatInst; 
   };

   SimObjectPtr<PostEffect> mGlowEffect;

};


#endif // _RENDERGLOWMGR_H_
