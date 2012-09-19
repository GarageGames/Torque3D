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

#ifndef _SHADOWMAPMANAGER_H_
#define _SHADOWMAPMANAGER_H_

#ifndef _TSINGLETON_H_
#include "core/util/tSingleton.h"
#endif
#ifndef _SHADOWMANAGER_H_
#include "lighting/shadowManager.h"
#endif
#ifndef _GFXENUMS_H_
#include "gfx/gfxEnums.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _MPOINT4_H_
#include "math/mPoint4.h"
#endif

class LightShadowMap;
class ShadowMapPass;
class LightInfo;

class SceneManager;
class SceneRenderState;


class ShadowMapManager : public ShadowManager
{
   typedef ShadowManager Parent;

   friend class ShadowMapPass;

public:

   ShadowMapManager();
   virtual ~ShadowMapManager();

   /// Sets the current shadowmap (used in setLightInfo/setTextureStage calls)
   void setLightShadowMap( LightShadowMap *lm ) { mCurrentShadowMap = lm; }
   
   /// Looks up the shadow map for the light then sets it.
   void setLightShadowMapForLight( LightInfo *light );

   /// Return the current shadow map
   LightShadowMap* getCurrentShadowMap() const { return mCurrentShadowMap; }

   ShadowMapPass* getShadowMapPass() const { return mShadowMapPass; }

   // Shadow manager
   virtual void activate();
   virtual void deactivate();

   GFXTextureObject* getTapRotationTex();

   /// The shadow map deactivation signal.
   static Signal<void(void)> smShadowDeactivateSignal;

   static void updateShadowDisable();

protected:

   void _onTextureEvent( GFXTexCallbackCode code );

   void _onPreRender( SceneManager *sg, const SceneRenderState* state );

   ShadowMapPass *mShadowMapPass;
   LightShadowMap *mCurrentShadowMap;

   ///
   GFXTexHandle mTapRotationTex;

   bool mIsActive;

public:
   // For ManagedSingleton.
   static const char* getSingletonName() { return "ShadowMapManager"; }   
};


/// Returns the ShadowMapManager singleton.
#define SHADOWMGR ManagedSingleton<ShadowMapManager>::instance()

GFX_DeclareTextureProfile( ShadowMapTexProfile );

#endif // _SHADOWMAPMANAGER_H_